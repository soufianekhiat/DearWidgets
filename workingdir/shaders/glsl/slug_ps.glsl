// Slug GPU Font Rendering - Pixel Shader (GLSL)
// Based on the Slug Algorithm by Eric Lengyel (public domain, 2026)
// https://sluglibrary.com
//
// Cross-platform design: per-glyph Slug data is passed via named uniforms
// (slugBandLoc, slugBanding) set by SlugSetGlyphUniforms callback.
#version 450
layout(row_major) uniform;
layout(row_major) buffer;

// Per-glyph Slug data uploaded via ImPlatform_SetUniform before each draw:
//   slugBandLoc.xy = glyphTexX, glyphTexY (band texture location as floats)
//   slugBandLoc.zw = bandMaxX,  bandMaxY  (band grid dimensions as floats)
//   slugBanding.xy = bandScaleX, bandScaleY
//   slugBanding.zw = bandOffsetX, bandOffsetY
uniform vec4 slugBandLoc;
uniform vec4 slugBanding;

// Glyph data textures (point-sampled, no mipmaps)
// curveTexture: RGBA32F - each curve uses 2 consecutive texels:
//   texel 0: (p1.x, p1.y, p2.x, p2.y)
//   texel 1: (p3.x, p3.y, 0, 0)
uniform sampler2D curveTexture;  // bound to t0 by AddImageQuad

// bandTexture: RGBA32F storing integers as exact floats
//   band headers: (.x=count, .y=offset, 0, 0)
//   curve refs:   (.x=curveTexX, .y=curveTexY, 0, 0)
uniform sampler2D bandTexture;   // bound to t1 by SlugSetGlyphUniforms

// Inputs from vertex shader
layout(location = 0) in vec4 v_color;
layout(location = 1) in vec2 v_renderCoord;  // em-space coordinate

layout(location = 0) out vec4 fragColor;

#define SLUG_BAND_TEXTURE_WIDTH  4096
#define SLUG_LOG_BAND_TEX_WIDTH  12   // log2(4096)

// ---- Helpers ----------------------------------------------------------------

ivec2 BandLoad(ivec2 coord)
{
    vec4 raw = texelFetch(bandTexture, coord, 0);
    return ivec2(int(raw.r + 0.5), int(raw.g + 0.5));
}

vec4 CurveLoad(ivec2 coord)
{
    return texelFetch(curveTexture, coord, 0);
}

ivec2 CalcBandLoc(ivec2 glyphLoc, int offset)
{
    ivec2 loc = ivec2(glyphLoc.x + offset, glyphLoc.y);
    loc.y += loc.x >> SLUG_LOG_BAND_TEX_WIDTH;
    loc.x &= SLUG_BAND_TEXTURE_WIDTH - 1;
    return loc;
}

// ---- Slug Algorithm ---------------------------------------------------------

uint CalcRootCode(float y1, float y2, float y3)
{
    uint i1 = floatBitsToUint(y1) >> 31u;
    uint i2 = floatBitsToUint(y2) >> 30u;
    uint i3 = floatBitsToUint(y3) >> 29u;
    uint shift = (i2 & 2u) | (i1 & ~2u);
    shift = (i3 & 4u) | (shift & ~4u);
    return (0x2E74u >> shift) & 0x0101u;
}

vec2 SolveHorizPoly(vec4 p12, vec2 p3)
{
    vec2 a = p12.xy - p12.zw * 2.0 + p3;
    vec2 b = p12.xy - p12.zw;
    float ra = 1.0 / a.y;
    float rb = 0.5  / b.y;
    float d  = sqrt(max(b.y * b.y - a.y * p12.y, 0.0));
    float t1 = (b.y - d) * ra;
    float t2 = (b.y + d) * ra;
    if (abs(a.y) < (1.0 / 65536.0)) { t1 = t2 = p12.y * rb; }
    return vec2((a.x * t1 - b.x * 2.0) * t1 + p12.x,
                (a.x * t2 - b.x * 2.0) * t2 + p12.x);
}

vec2 SolveVertPoly(vec4 p12, vec2 p3)
{
    vec2 a = p12.xy - p12.zw * 2.0 + p3;
    vec2 b = p12.xy - p12.zw;
    float ra = 1.0 / a.x;
    float rb = 0.5  / b.x;
    float d  = sqrt(max(b.x * b.x - a.x * p12.x, 0.0));
    float t1 = (b.x - d) * ra;
    float t2 = (b.x + d) * ra;
    if (abs(a.x) < (1.0 / 65536.0)) { t1 = t2 = p12.x * rb; }
    return vec2((a.y * t1 - b.y * 2.0) * t1 + p12.y,
                (a.y * t2 - b.y * 2.0) * t2 + p12.y);
}

float CalcCoverage(float xcov, float ycov, float xwgt, float ywgt)
{
    float wsum = max(xwgt + ywgt, 1.0 / 65536.0);
    float cov  = max(abs(xcov * xwgt + ycov * ywgt) / wsum,
                     min(abs(xcov), abs(ycov)));
    return clamp(cov, 0.0, 1.0);
}

// ---- Main -------------------------------------------------------------------

void main()
{
    vec2 renderCoord = v_renderCoord;

    // Decode per-glyph data from uniforms
    ivec2 glyphLoc = ivec2(int(slugBandLoc.x + 0.5), int(slugBandLoc.y + 0.5));
    ivec2 bandMax  = ivec2(int(slugBandLoc.z + 0.5), int(slugBandLoc.w + 0.5));

    vec2 emsPerPixel = fwidth(renderCoord);
    vec2 pixelsPerEm = 1.0 / emsPerPixel;

    ivec2 bandIndex = clamp(ivec2(renderCoord * slugBanding.xy + slugBanding.zw),
                            ivec2(0, 0), bandMax);

    // ----- Horizontal ray (cast leftward, accumulate x-crossings) -----
    float xcov = 0.0, xwgt = 0.0;

    ivec2 hBandData = BandLoad(ivec2(glyphLoc.x + bandIndex.y, glyphLoc.y));
    ivec2 hBandLoc  = CalcBandLoc(glyphLoc, hBandData.y);

    for (int ci = 0; ci < hBandData.x; ci++)
    {
        ivec2 curveRef = BandLoad(ivec2(hBandLoc.x + ci, hBandLoc.y));
        ivec2 curveLoc = ivec2(curveRef.x, curveRef.y);

        vec4 p12 = CurveLoad(curveLoc)                              - vec4(renderCoord, renderCoord);
        vec2 p3  = CurveLoad(ivec2(curveLoc.x + 1, curveLoc.y)).xy - renderCoord;

        if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5) break;

        uint code = CalcRootCode(p12.y, p12.w, p3.y);
        if (code != 0u)
        {
            vec2 r = SolveHorizPoly(p12, p3) * pixelsPerEm.x;
            if ((code & 1u) != 0u)
            {
                xcov += clamp(r.x + 0.5, 0.0, 1.0);
                xwgt  = max(xwgt, clamp(1.0 - abs(r.x) * 2.0, 0.0, 1.0));
            }
            if (code > 1u)
            {
                xcov -= clamp(r.y + 0.5, 0.0, 1.0);
                xwgt  = max(xwgt, clamp(1.0 - abs(r.y) * 2.0, 0.0, 1.0));
            }
        }
    }

    // ----- Vertical ray (cast upward, accumulate y-crossings) -----
    float ycov = 0.0, ywgt = 0.0;

    ivec2 vBandData = BandLoad(ivec2(glyphLoc.x + bandMax.y + 1 + bandIndex.x, glyphLoc.y));
    ivec2 vBandLoc  = CalcBandLoc(glyphLoc, vBandData.y);

    for (int ci = 0; ci < vBandData.x; ci++)
    {
        ivec2 curveRef = BandLoad(ivec2(vBandLoc.x + ci, vBandLoc.y));
        ivec2 curveLoc = ivec2(curveRef.x, curveRef.y);

        vec4 p12 = CurveLoad(curveLoc)                              - vec4(renderCoord, renderCoord);
        vec2 p3  = CurveLoad(ivec2(curveLoc.x + 1, curveLoc.y)).xy - renderCoord;

        if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5) break;

        uint code = CalcRootCode(p12.x, p12.z, p3.x);
        if (code != 0u)
        {
            vec2 r = SolveVertPoly(p12, p3) * pixelsPerEm.y;
            if ((code & 1u) != 0u)
            {
                ycov -= clamp(r.x + 0.5, 0.0, 1.0);
                ywgt  = max(ywgt, clamp(1.0 - abs(r.x) * 2.0, 0.0, 1.0));
            }
            if (code > 1u)
            {
                ycov += clamp(r.y + 0.5, 0.0, 1.0);
                ywgt  = max(ywgt, clamp(1.0 - abs(r.y) * 2.0, 0.0, 1.0));
            }
        }
    }

    float coverage = CalcCoverage(xcov, ycov, xwgt, ywgt);
    fragColor = v_color * coverage;
}
