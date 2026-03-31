// ===================================================
// Slug GPU Font Rendering - DearWidgets HLSL Shader
// Based on the reference implementation by Eric Lengyel
// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright 2017, by Eric Lengyel.
// ===================================================
//
// Vertex format (SlugVertex, 80 bytes = 5 x float4):
//   POSITION  float4  (offset  0) - xy = screen-space position, zw = outward vertex normal
//   TEXCOORD0 float4  (offset 16) - xy = em-space UV (undilated), zw = packed glyph data
//   TEXCOORD1 float4  (offset 32) - inverse Jacobian (00, 01, 10, 11): maps screen-delta → em-delta
//   TEXCOORD2 float4  (offset 48) - band transform (scaleX, scaleY, offsetX, offsetY)
//   COLOR0    float4  (offset 64) - RGBA vertex color
//
// tex.z interpreted as uint: bits 0-15 = bandTexX, bits 16-31 = bandTexY
// tex.w interpreted as uint: bits 0-15 = bandMaxX, bits 16-23 = bandMaxY, bit 28 = E (even-odd)
//
// All curves are quadratic Bezier (cubics are converted to quadratics on the CPU).
//
// Entry points: main_vs, main_ps

// ---- Constant Buffers -------------------------------------------------------

// b0: ImGui orthographic projection matrix (set by ImPlatform_BeginCustomShader)
cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjectionMatrix;
};

// b1: Fill parameters (only used by SLUG_FILL permutation)
#ifdef SLUG_FILL
cbuffer fillParams : register(b1)
{
    float4 fillColor0;   // RGBA gradient start color (or image tint)
    float4 fillColor1;   // RGBA gradient end color (unused for image)
    float4 fillBBox;     // (reserved — bbox now from vertex color)
    float4 fillGrad;     // x=type (0=linear,1=radial,2=diamond,3=image), y=colorSpace (0-4), z,w=unused
    float4 fillUVStart;  // gradient UV start (or image uv_offset)
    float4 fillUVEnd;    // gradient UV end (or image uv_scale)
};
#endif

// ---- Textures ---------------------------------------------------------------

// curveTexture (t0): RGBA32F, each quadratic Bezier uses 2 consecutive texels
//   texel 0: (p1.x, p1.y, p2.x, p2.y)
//   texel 1: (p3.x, p3.y, 0, 0)
Texture2D<float4> curveTexture : register(t0);

// bandTexture (t1): RGBA32F storing integer values as exact floats
//   band headers: (.r=count, .g=offset, 0, 0)
//   curve refs:   (.r=curveTexX, .g=curveTexY, 0, 0)
Texture2D<float4> bandTexture : register(t1);

// fillTexture (t2): user image texture (only used by SLUG_FILL image mode)
#ifdef SLUG_FILL
Texture2D<float4> fillTexture : register(t2);
SamplerState fillSampler : register(s0);  // ImGui DX11 binds linear-clamp to s0
#endif

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
    int i = (int)floor(h);
    float f = h - (float)i;
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
    if      (space < 0.5f) { /* sRGB — identity */ }
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

// ---- Structs ----------------------------------------------------------------

#ifdef SLUG_GRADIENT
struct VS_INPUT
{
    float4 pos  : POSITION;
    float4 tex  : TEXCOORD0;
    float4 jac  : TEXCOORD1;
    float4 bnd  : TEXCOORD2;
    float4 col  : COLOR0;     // gradient color 0
    float4 grd  : TEXCOORD3;  // gradient: dirX, dirY, scale, bias
    float4 col2 : COLOR1;     // gradient color 1
};

struct PS_INPUT
{
    float4 position                     : SV_Position;
    float2 texcoord                     : TEXCOORD0;
    nointerpolation float4 banding      : TEXCOORD1;
    nointerpolation int4 glyph          : TEXCOORD2;
    nointerpolation float4 gradColor0   : COLOR0;
    nointerpolation float4 gradColor1   : COLOR1;
    nointerpolation float4 gradParams   : TEXCOORD3;  // dirX, dirY, scale, bias
};
#else
struct VS_INPUT
{
    float4 pos  : POSITION;   // xy = screen-space position, zw = outward vertex normal
    float4 tex  : TEXCOORD0;  // xy = em UV (undilated), zw = packed glyph location + band max
    float4 jac  : TEXCOORD1;  // inverse Jacobian (00, 01, 10, 11)
    float4 bnd  : TEXCOORD2;  // band transform: scaleX, scaleY, offsetX, offsetY
    float4 col  : COLOR0;     // RGBA vertex color (float)
};

struct PS_INPUT
{
    float4 position                 : SV_Position;
    float4 color                    : COLOR0;
    float2 texcoord                 : TEXCOORD0;  // em-space sample coord (dilated, interpolated)
    nointerpolation float4 banding  : TEXCOORD1;  // band scale/offset, constant per glyph
    nointerpolation int4 glyph      : TEXCOORD2;  // glyph loc + band max + flags, constant per glyph
};
#endif

// ---- Vertex Shader ----------------------------------------------------------

// Decode packed per-glyph data from tex.zw and pass through bnd.
// tex.z (as uint): bits  0-15 = glyph band texture X, bits 16-31 = glyph band texture Y
// tex.w (as uint): bits  0-15 = bandMaxX,             bits 16-31 = bandMaxY | (E << 12)
void SlugUnpack(float4 tex, float4 bnd, out float4 vbnd, out int4 vgly)
{
    uint2 g  = asuint(tex.zw);
    vgly = int4(int(g.x & 0xFFFFU), int(g.x >> 16U), int(g.y & 0xFFFFU), int(g.y >> 16U));
    vbnd = bnd;
}

// Dynamic vertex dilation — matches the reference implementation exactly.
float2 SlugDilate(float4 pos, float4 tex, float4 jac, float4 m0, float4 m1, float4 m3, float2 dim, out float2 vpos)
{
    float2 n  = normalize(pos.zw);
    float  s  = dot(m3.xy, pos.xy) + m3.w;
    float  t  = dot(m3.xy, n);

    float  u  = (s * dot(m0.xy, n) - t * (dot(m0.xy, pos.xy) + m0.w)) * dim.x;
    float  v  = (s * dot(m1.xy, n) - t * (dot(m1.xy, pos.xy) + m1.w)) * dim.y;

    float  s2 = s * s;
    float  st = s * t;
    float  uv = u * u + v * v;
    float2 d  = pos.zw * (s2 * (st + sqrt(uv)) / (uv - st * st));

    vpos = pos.xy + d;
    return float2(tex.x + dot(d, jac.xy), tex.y + dot(d, jac.zw));
}

PS_INPUT main_vs(VS_INPUT input)
{
    PS_INPUT output;

    float4 m0  = ProjectionMatrix[0];
    float4 m1  = ProjectionMatrix[1];
    float4 m3  = ProjectionMatrix[3];
    float2 dim = float2(2.0f / m0.x, 2.0f / abs(m1.y));

    float2 dilatedPos;
    output.texcoord = SlugDilate(input.pos, input.tex, input.jac, m0, m1, m3, dim, dilatedPos);
    output.position = mul(ProjectionMatrix, float4(dilatedPos, 0.f, 1.f));
#ifdef SLUG_GRADIENT
    output.gradColor0 = input.col;
    output.gradColor1 = input.col2;
    output.gradParams = input.grd;
#else
    output.color    = input.col;
#endif
    SlugUnpack(input.tex, input.bnd, output.banding, output.glyph);
    return output;
}

// ---- Pixel Shader -----------------------------------------------------------

#define kLogBandTextureWidth 12

int2 BandLoad(int2 coord)
{
    float4 raw = bandTexture.Load(int3(coord, 0));
    return int2(int(raw.r + 0.5f), int(raw.g + 0.5f));
}

float4 CurveLoad(int2 coord)
{
    return curveTexture.Load(int3(coord, 0));
}

int2 CalcBandLoc(int2 glyphLoc, uint offset)
{
    int2 loc = int2(glyphLoc.x + int(offset), glyphLoc.y);
    loc.y   += loc.x >> kLogBandTextureWidth;
    loc.x   &= (1 << kLogBandTextureWidth) - 1;
    return loc;
}

uint CalcRootCode(float y1, float y2, float y3)
{
    uint i1 = asuint(y1) >> 31U;
    uint i2 = asuint(y2) >> 30U;
    uint i3 = asuint(y3) >> 29U;
    uint shift = (i2 & 2U) | (i1 & ~2U);
    shift = (i3 & 4U) | (shift & ~4U);
    return (0x2E74U >> shift) & 0x0101U;
}

float2 SolveHorizPoly(float4 p12, float2 p3)
{
    float2 a = p12.xy - p12.zw * 2.0f + p3;
    float2 b = p12.xy - p12.zw;
    float ra = 1.0f / a.y;
    float rb = 0.5f  / b.y;
    float d  = sqrt(max(b.y * b.y - a.y * p12.y, 0.0f));
    float t1 = (b.y - d) * ra;
    float t2 = (b.y + d) * ra;
    if (abs(a.y) < 1.0f / 65536.0f) { t1 = t2 = p12.y * rb; }
    return float2((a.x * t1 - b.x * 2.0f) * t1 + p12.x,
                  (a.x * t2 - b.x * 2.0f) * t2 + p12.x);
}

float2 SolveVertPoly(float4 p12, float2 p3)
{
    float2 a = p12.xy - p12.zw * 2.0f + p3;
    float2 b = p12.xy - p12.zw;
    float ra = 1.0f / a.x;
    float rb = 0.5f  / b.x;
    float d  = sqrt(max(b.x * b.x - a.x * p12.x, 0.0f));
    float t1 = (b.x - d) * ra;
    float t2 = (b.x + d) * ra;
    if (abs(a.x) < 1.0f / 65536.0f) { t1 = t2 = p12.x * rb; }
    return float2((a.y * t1 - b.y * 2.0f) * t1 + p12.y,
                  (a.y * t2 - b.y * 2.0f) * t2 + p12.y);
}

float CalcCoverage(float xcov, float ycov, float xwgt, float ywgt)
{
    float coverage = max(abs(xcov * xwgt + ycov * ywgt) / max(xwgt + ywgt, 1.0f / 65536.0f),
                         min(abs(xcov), abs(ycov)));
    coverage = saturate(coverage);
    return coverage;
}

float SlugRender(float2 renderCoord, float4 banding, int4 glyphData)
{
    float2 emsPerPixel = abs(ddx(renderCoord)) + abs(ddy(renderCoord));
    float2 pixelsPerEm = 1.0f / emsPerPixel;

    int2 glyphLoc = glyphData.xy;
    int2 bandMax  = glyphData.zw;
    bandMax.y    &= 0x00FF;

    int2 bandIndex = clamp(int2(renderCoord * banding.xy + banding.zw),
                           int2(0, 0), bandMax);

    // ----- Horizontal band (cast ray leftward) -----
    float xcov = 0.0f, xwgt = 0.0f;
    int2 hData = BandLoad(int2(glyphLoc.x + bandIndex.y, glyphLoc.y));

    for (int ci = 0; ci < hData.x; ci++)
    {
        int2   ref      = BandLoad(CalcBandLoc(glyphLoc, uint(hData.y) + uint(ci)));
        int2   curveLoc = int2(ref.x & 0x0FFF, ref.y);
        float4 p12    = CurveLoad(curveLoc) - float4(renderCoord, renderCoord);
        float4 texel1 = CurveLoad(int2(curveLoc.x + 1, curveLoc.y));
        float2 p3     = texel1.xy - renderCoord;

        // Early exit: curves sorted by descending maxX
        float maxX = max(max(p12.x, p12.z), p3.x);
        if (maxX * pixelsPerEm.x < -0.5f) break;

        uint code = CalcRootCode(p12.y, p12.w, p3.y);
        if (code != 0U)
        {
            float2 r = SolveHorizPoly(p12, p3) * pixelsPerEm.x;
            if ((code & 1U) != 0U)
            {
                xcov += saturate(r.x + 0.5f);
                xwgt  = max(xwgt, saturate(1.0f - abs(r.x) * 2.0f));
            }
            if (code > 1U)
            {
                xcov -= saturate(r.y + 0.5f);
                xwgt  = max(xwgt, saturate(1.0f - abs(r.y) * 2.0f));
            }
        }
    }

    // ----- Vertical band (cast ray upward) -----
    float ycov = 0.0f, ywgt = 0.0f;
    int2 vData = BandLoad(int2(glyphLoc.x + bandMax.y + 1 + bandIndex.x, glyphLoc.y));

    for (int ci2 = 0; ci2 < vData.x; ci2++)
    {
        int2   ref      = BandLoad(CalcBandLoc(glyphLoc, uint(vData.y) + uint(ci2)));
        int2   curveLoc = int2(ref.x & 0x0FFF, ref.y);
        float4 p12    = CurveLoad(curveLoc) - float4(renderCoord, renderCoord);
        float4 texel1 = CurveLoad(int2(curveLoc.x + 1, curveLoc.y));
        float2 p3     = texel1.xy - renderCoord;

        // Early exit: curves sorted by descending maxY
        float maxY = max(max(p12.y, p12.w), p3.y);
        if (maxY * pixelsPerEm.y < -0.5f) break;

        uint code = CalcRootCode(p12.x, p12.z, p3.x);
        if (code != 0U)
        {
            float2 r = SolveVertPoly(p12, p3) * pixelsPerEm.y;
            if ((code & 1U) != 0U)
            {
                ycov -= saturate(r.x + 0.5f);
                ywgt  = max(ywgt, saturate(1.0f - abs(r.x) * 2.0f));
            }
            if (code > 1U)
            {
                ycov += saturate(r.y + 0.5f);
                ywgt  = max(ywgt, saturate(1.0f - abs(r.y) * 2.0f));
            }
        }
    }

    return CalcCoverage(xcov, ycov, xwgt, ywgt);
}

#ifdef SLUG_DEBUG
// Debug variant: output xcov/ycov/coverage as visible colors
float4 SlugRenderDebug(float2 renderCoord, float4 banding, int4 glyphData)
{
    // Re-use the main render to get coverage, then also compute xcov/ycov for visualization.
    // (Since we removed cubics, SlugRender is the single authoritative path.)
    float coverage = SlugRender(renderCoord, banding, glyphData);
    return float4(coverage, coverage, coverage, max(coverage, 0.25f));
}
#endif

float4 main_ps(PS_INPUT input) : SV_Target
{
    float coverage = SlugRender(input.texcoord, input.banding, input.glyph);
#ifdef SLUG_DEBUG
    return SlugRenderDebug(input.texcoord, input.banding, input.glyph);
#elif defined(SLUG_GRADIENT)
    // Linear gradient: compute t from em-space coordinate projected onto gradient line
    float t = dot(input.texcoord, input.gradParams.xy) * input.gradParams.z + input.gradParams.w;
    t = saturate(t);
    float4 gradColor = lerp(input.gradColor0, input.gradColor1, t);
    return float4(gradColor.rgb, gradColor.a * coverage);
#elif defined(SLUG_FILL)
    // Fill: bbox from vertex color (supports per-char and whole-text in one batch).
    float4 bbox = input.color; // (minX, minY, maxX, maxY) in screen pixels
    float2 fillSize = max(bbox.zw - bbox.xy, float2(1, 1));
    float2 uv = (input.position.xy - bbox.xy) / fillSize;

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
        // Image fill (fillGrad.x >= 3): sample user texture
        float2 imgUV = uv * fillUVEnd.xy + fillUVStart.xy;
        float4 texColor = fillTexture.Sample(fillSampler, imgUV);
        texColor *= fillColor0; // tint
        return float4(texColor.rgb, texColor.a * coverage);
    }
#elif defined(SLUG_COLOR)
    return float4(input.color.rgb, input.color.a * coverage);
#else
    return input.color * coverage;
#endif
}
