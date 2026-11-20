// Slug GPU font rendering, written once as shader parts (ImPlatform_ParseShaderParts) for every backend.
// Based on the reference implementation by Eric Lengyel.
// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright 2017, by Eric Lengyel.
//
// Variants (DearWidgets maps the shader names to defines): slug (coverage * vertex color), slug_color (SLUG_COLOR:
// COLR v0 palette color, alpha modulated by coverage), slug_debug (SLUG_DEBUG: coverage as gray), slug_fill (SLUG_FILL:
// gradient or image fill from the parameters, over the text bbox carried in the vertex color).
// The COLR v1 gradient variant has its own vertex format: parts/slug_gradient.hlsl.
//
// Vertex format (SlugVertex, 80 bytes = 5 x float4):
//   pos  xy = screen-space position, zw = outward vertex normal
//   tex  xy = em-space UV (undilated), zw = packed glyph data (as uint: bits 0-15 band texture X, 16-31 band texture Y;
//        bits 0-15 bandMaxX, 16-23 bandMaxY, bit 28 even-odd)
//   jac  inverse Jacobian (00, 01, 10, 11): maps screen-delta to em-delta
//   bnd  band transform (scaleX, scaleY, offsetX, offsetY)
//   col  RGBA vertex color (SLUG_FILL: the text bbox minX, minY, maxX, maxY in screen pixels)

//@params
float4 fillColor0;   // RGBA gradient start color (or image tint)
float4 fillColor1;   // RGBA gradient end color (unused for image)
float4 fillBBox;     // reserved: the bbox comes from the vertex color
float4 fillGrad;     // x = type (0 linear, 1 radial, 2 diamond, 3 image), y = color space (0-4)
float4 fillUVStart;  // gradient UV start (or image uv_offset)
float4 fillUVEnd;    // gradient UV end (or image uv_scale)
float4 slugTexel;    // 1 / curve texture size (xy), 1 / band texture size (zw): shader model 3 only

//@defines
#ifdef IMP_SM3
#define SLUG_GLYPH float4
#else
#define SLUG_GLYPH int4
#endif

//@inputs
float4 pos : POSITION;
float4 tex : TEXCOORD0;
float4 jac : TEXCOORD1;
float4 bnd : TEXCOORD2;
float4 col : COLOR0;

//@varyings
nointerpolation float4 banding;  // band scale / offset, constant per glyph
nointerpolation SLUG_GLYPH glyph; // glyph location + band max + flags, constant per glyph
float2 screen;                   // SLUG_FILL: the pixel position in screen coordinates

//@textures
0 curveTexture
1 bandTexture
2 fillTexture

//@functions
//@include slug_core.inc

// ---- Color Space Conversion (SLUG_FILL only) --------------------------------
#ifdef SLUG_FILL

float sRGBToLinearCh(float x) {
    return (x <= 0.04045f) ? x / 12.92f : pow((x + 0.055f) / 1.055f, 2.4f);
}
float linearToSRGBCh(float x) {
    return (x <= 0.0031308f) ? 12.92f * x : 1.055f * pow(x, 1.0f / 2.4f) - 0.055f;
}
float3 sRGBToLinear3(float3 c) {
    return float3(sRGBToLinearCh(c.x), sRGBToLinearCh(c.y), sRGBToLinearCh(c.z));
}
float3 linearToSRGB3(float3 c) {
    return saturate(float3(linearToSRGBCh(c.x), linearToSRGBCh(c.y), linearToSRGBCh(c.z)));
}

float3 sRGBToOkLab(float3 c) {
    float3 lin = sRGBToLinear3(c);
    float l = 0.4122214708f * lin.x + 0.5363325363f * lin.y + 0.0514459929f * lin.z;
    float m = 0.2119034982f * lin.x + 0.6806995451f * lin.y + 0.1073969566f * lin.z;
    float s = 0.0883024619f * lin.x + 0.2817188376f * lin.y + 0.6299787005f * lin.z;
    l = sign(l) * pow(abs(l), 1.0f / 3.0f);
    m = sign(m) * pow(abs(m), 1.0f / 3.0f);
    s = sign(s) * pow(abs(s), 1.0f / 3.0f);
    return float3(
        l * 0.2104542553f + m * 0.7936177850f + s * -0.0040720468f,
        l * 1.9779984951f + m * -2.4285922050f + s * 0.4505937099f,
        l * 0.0259040371f + m * 0.7827717662f + s * -0.8086757660f);
}
float3 okLabToSRGB(float3 Lab) {
    float l = Lab.x + Lab.y * 0.3963377774f + Lab.z * 0.2158037573f;
    float m = Lab.x + Lab.y * -0.1055613458f + Lab.z * -0.0638541728f;
    float s = Lab.x + Lab.y * -0.0894841775f + Lab.z * -1.2914855480f;
    l = l * l * l; m = m * m * m; s = s * s * s;
    float3 rgb = float3(
        l * 4.0767416621f + m * -3.3077115913f + s * 0.2309699292f,
        l * -1.2684380046f + m * 2.6097574011f + s * -0.3413193965f,
        l * -0.0041960863f + m * -0.7034186147f + s * 1.7076147010f);
    return linearToSRGB3(rgb);
}

float3 sRGBToOkLch(float3 c) {
    float3 lab = sRGBToOkLab(c);
    float C = sqrt(lab.y * lab.y + lab.z * lab.z);
    float h = atan2(lab.z, lab.y);
    if (h < 0.0f) h += 6.28318530718f;
    h /= 6.28318530718f;
    return float3(lab.x, C, h);
}
float3 okLchToSRGB(float3 lch) {
    float a = lch.y * cos(lch.z * 6.28318530718f);
    float b = lch.y * sin(lch.z * 6.28318530718f);
    return okLabToSRGB(float3(lch.x, a, b));
}

float3 sRGBToHSV(float3 c) {
    float K = 0.0f;
    float r = c.x, g = c.y, b = c.z;
    if (g < b) { float t = g; g = b; b = t; K = -1.0f; }
    if (r < g) { float t = r; r = g; g = t; K = -2.0f / 6.0f - K; }
    float chroma = r - min(g, b);
    float h = abs(K + (g - b) / (6.0f * chroma + 1e-20f));
    float s = chroma / (r + 1e-20f);
    return float3(h, s, r);
}
float3 hsvToSRGB(float3 c) {
    float h = c.x, s = c.y, v = c.z;
    if (s < 1e-6f) return float3(v, v, v);
    h = fmod(h, 1.0f); if (h < 0.0f) h += 1.0f;
    h *= 6.0f;
    int i = IMP_CAST(int, floor(h));
    float f = h - IMP_CAST(float, i);
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));
    if (i == 0) return float3(v, t, p);
    if (i == 1) return float3(q, v, p);
    if (i == 2) return float3(p, v, t);
    if (i == 3) return float3(p, q, v);
    if (i == 4) return float3(t, p, v);
    return float3(v, p, q);
}

// Interpolate two colors in a chosen color space. space: 0=sRGB, 1=linear, 2=OkLab, 3=OkLch, 4=HSV
float4 lerpInColorSpace(float4 c0, float4 c1, float ft, float space) {
    float3 a = c0.rgb, b = c1.rgb;
    if      (space < 0.5f) { /* sRGB - identity */ }
    else if (space < 1.5f) { a = sRGBToLinear3(a); b = sRGBToLinear3(b); }
    else if (space < 2.5f) { a = sRGBToOkLab(a);   b = sRGBToOkLab(b); }
    else if (space < 3.5f) { a = sRGBToOkLch(a);   b = sRGBToOkLch(b); }
    else                   { a = sRGBToHSV(a);     b = sRGBToHSV(b); }
    float3 r = lerp(a, b, ft);
    if      (space < 0.5f) { }
    else if (space < 1.5f) { r = linearToSRGB3(r); }
    else if (space < 2.5f) { r = okLabToSRGB(r); }
    else if (space < 3.5f) { r = okLchToSRGB(r); }
    else                   { r = hsvToSRGB(r); }
    return float4(saturate(r), lerp(c0.a, c1.a, ft));
}

#endif // SLUG_FILL

// The whole pixel stage: a function, so it can return early (a body cannot). texcoord: em-space sample coordinate.
float4 SlugShade(float2 texcoord, float4 color, float4 banding, SLUG_GLYPH glyph, float2 screen)
{
    float coverage = SlugRender(texcoord, banding, glyph);
#if defined(SLUG_DEBUG)
    return float4(coverage, coverage, coverage, max(coverage, 0.25f));
#elif defined(SLUG_FILL)
    // Fill: bbox from the vertex color (per-char and whole-text in one batch).
    float4 bbox = color; // (minX, minY, maxX, maxY) in screen pixels
    float2 fillSize = max(bbox.zw - bbox.xy, float2(1.0f, 1.0f));
    float2 uv = (screen - bbox.xy) / fillSize;

    if (fillGrad.x < 2.5f) {
        // Gradient (0=linear, 1=radial, 2=diamond) with color space interpolation
        float ft;
        if (fillGrad.x < 0.5f) {
            float2 dir = fillUVEnd.xy - fillUVStart.xy;
            float denom = dot(dir, dir);
            ft = (denom > 1e-8f) ? dot(uv - fillUVStart.xy, dir) / denom : 0.0f;
        } else if (fillGrad.x < 1.5f) {
            float radius = max(length(fillUVEnd.xy - fillUVStart.xy), 1e-5f);
            ft = length(uv - fillUVStart.xy) / radius;
        } else {
            float radius = max(length(fillUVEnd.xy - fillUVStart.xy), 1e-5f);
            float2 d = (uv - fillUVStart.xy) / radius;
            ft = abs(d.x) + abs(d.y);
        }
        ft = saturate(ft);
        float4 fillCol = lerpInColorSpace(fillColor0, fillColor1, ft, fillGrad.y);
        return float4(fillCol.rgb, fillCol.a * coverage);
    } else {
        // Image fill (fillGrad.x >= 3): sample user texture.
        // Composite the image OVER the tint color instead of multiplying the
        // glyph alpha by the image alpha: images with transparency (cut-out
        // PNGs) were erasing the glyph wherever the image was transparent,
        // leaving letters with holes / near-invisible. The glyph silhouette
        // must always stay solid -- only its RGB content comes from the image.
        float2 imgUV = uv * fillUVEnd.xy + fillUVStart.xy;
        float4 texColor = IMP_SAMPLE(fillTexture, imgUV);
        float3 rgb = lerp(fillColor0.rgb, texColor.rgb * fillColor0.rgb, texColor.a);
        return float4(rgb, fillColor0.a * coverage);
    }
#elif defined(SLUG_COLOR)
    return float4(color.rgb, color.a * coverage);
#else
    return color * coverage;
#endif
}

//@vs
// Dynamic vertex dilation from the projection rows, as the reference implementation.
float4 m0 = IMP_MUL(float4(1.0, 0.0, 0.0, 0.0), ProjMtx);
float4 m1 = IMP_MUL(float4(0.0, 1.0, 0.0, 0.0), ProjMtx);
float4 m3 = IMP_MUL(float4(0.0, 0.0, 0.0, 1.0), ProjMtx);
float2 dim = float2(2.0f / m0.x, 2.0f / abs(m1.y));
float2 dilatedPos = float2(0.0, 0.0);
v.uv = SlugDilate(in_pos, in_tex, in_jac, m0, m1, m3, dim, dilatedPos);
position = IMP_MUL(ProjMtx, float4(dilatedPos, 0.0, 1.0));
v.col = in_col;
v.screen = dilatedPos;
SlugUnpack(in_tex, in_bnd, v.banding, v.glyph);

//@ps
col_out = SlugShade(v.uv, v.col, v.banding, v.glyph, v.screen);
