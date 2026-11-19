// Slug GPU font rendering: the code shared by parts/slug.hlsl and parts/slug_gradient.hlsl (their //@functions).
// Based on the reference implementation by Eric Lengyel.
// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright 2017, by Eric Lengyel.
//
// curveTexture (t0): RGBA32F, each quadratic Bezier uses 2 consecutive texels: (p1.x, p1.y, p2.x, p2.y), (p3.x, p3.y, 0, 0).
// bandTexture (t1): RGBA32F storing integer values as exact floats: band headers (.r = count, .g = offset),
// curve refs (.r = curveTexX, .g = curveTexY).
// All curves are quadratic Bezier (cubics are converted to quadratics on the CPU).

// Decode packed per-glyph data from tex.zw and pass through bnd.
// tex.z (as uint): bits  0-15 = glyph band texture X, bits 16-31 = glyph band texture Y
// tex.w (as uint): bits  0-15 = bandMaxX,             bits 16-31 = bandMaxY | (E << 12)
void SlugUnpack(float4 tex, float4 bnd, IMP_OUT(float4) vbnd, IMP_OUT(int4) vgly)
{
    uint2 g  = asuint(tex.zw);
    vgly = int4(int(g.x & 0xFFFFU), int(g.x >> 16U), int(g.y & 0xFFFFU), int(g.y >> 16U));
    vbnd = bnd;
}

// Dynamic vertex dilation - matches the reference implementation exactly.
float2 SlugDilate(float4 pos, float4 tex, float4 jac, float4 m0, float4 m1, float4 m3, float2 dim, IMP_OUT(float2) vpos)
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

#define kLogBandTextureWidth 12

int2 BandLoad(int2 coord)
{
    float4 raw = IMP_LOAD(bandTexture, coord);
    return int2(int(raw.r + 0.5f), int(raw.g + 0.5f));
}

float4 CurveLoad(int2 coord)
{
    return IMP_LOAD(curveTexture, coord);
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

