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
// Dilation is performed in the vertex shader (SlugDilate), matching the reference exactly.
// The Jacobian is (1/sz, 0, 0, -1/sz) for an axis-aligned glyph at pixel size sz.
// Viewport dimensions are derived from the projection matrix: dim = (2/P[0][0], 2/|P[1][1]|).
//
// Entry points: main_vs, main_ps

// ---- Constant Buffers -------------------------------------------------------

// b0: ImGui orthographic projection matrix (set by ImPlatform_BeginCustomShader)
cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjectionMatrix;
};

// ---- Textures ---------------------------------------------------------------

// curveTexture (t0): RGBA32F, each quadratic Bezier uses 2 consecutive texels
//   texel 0: (p1.x, p1.y, p2.x, p2.y)
//   texel 1: (p3.x, p3.y, 0, 0)
Texture2D<float4> curveTexture : register(t0);

// bandTexture (t1): RGBA32F storing integer values as exact floats
//   band headers: (.r=count, .g=offset, 0, 0)
//   curve refs:   (.r=curveTexX, .g=curveTexY, 0, 0)
Texture2D<float4> bandTexture : register(t1);

// ---- Structs ----------------------------------------------------------------

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
// Pushes each vertex along its outward normal by exactly 0.5 screen pixels,
// accounting for the full MVP transform (handles rotation, scale, perspective).
// Also updates the em-space texcoord via the inverse Jacobian.
//   pos:  xy = object-space position, zw = outward normal direction
//   tex:  xy = em-space UV (undilated)
//   jac:  inverse Jacobian (maps object-space offset → em-space offset)
//   m0, m1, m3: rows 0, 1, 3 of the MVP matrix
//   dim:  viewport dimensions in pixels
//   vpos: [out] dilated object-space position
// Returns: dilated em-space texcoord
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

    // Derive viewport dimensions from the orthographic projection matrix.
    // P[0][0] = 2/W  =>  W = 2/P[0][0]
    // P[1][1] = ±2/H =>  H = 2/|P[1][1]|
    float4 m0  = ProjectionMatrix[0];
    float4 m1  = ProjectionMatrix[1];
    float4 m3  = ProjectionMatrix[3];
    float2 dim = float2(2.0f / m0.x, 2.0f / abs(m1.y));

    float2 dilatedPos;
    output.texcoord = SlugDilate(input.pos, input.tex, input.jac, m0, m1, m3, dim, dilatedPos);
    output.position = mul(ProjectionMatrix, float4(dilatedPos, 0.f, 1.f));
    output.color    = input.col;
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

float CalcCoverage(float xcov, float ycov, float xwgt, float ywgt, int flags)
{
    float coverage = max(abs(xcov * xwgt + ycov * ywgt) / max(xwgt + ywgt, 1.0f / 65536.0f),
                         min(abs(xcov), abs(ycov)));

#if defined(SLUG_EVENODD)
    if ((flags & 0x1000) == 0)
    {
#endif
        coverage = saturate(coverage);
#if defined(SLUG_EVENODD)
    }
    else
    {
        coverage = 1.0f - abs(1.0f - frac(coverage * 0.5f) * 2.0f);
    }
#endif

#if defined(SLUG_WEIGHT)
    coverage = sqrt(coverage);
#endif

    return coverage;
}

float SlugRender(float2 renderCoord, float4 banding, int4 glyphData)
{
    // fwidth expanded for Slang/Metal compatibility: abs(ddx) + abs(ddy)
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
    int2 hLoc  = CalcBandLoc(glyphLoc, uint(hData.y));

    for (int ci = 0; ci < hData.x; ci++)
    {
        int2   ref      = BandLoad(int2(hLoc.x + ci, hLoc.y));
        int2   curveLoc = int2(ref.x, ref.y);
        float4 p12 = CurveLoad(curveLoc)                            - float4(renderCoord, renderCoord);
        float2 p3  = CurveLoad(int2(curveLoc.x + 1, curveLoc.y)).xy - renderCoord;

        if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5f) break;

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
    int2 vLoc  = CalcBandLoc(glyphLoc, uint(vData.y));

    for (int ci2 = 0; ci2 < vData.x; ci2++)
    {
        int2   ref      = BandLoad(int2(vLoc.x + ci2, vLoc.y));
        int2   curveLoc = int2(ref.x, ref.y);
        float4 p12 = CurveLoad(curveLoc)                            - float4(renderCoord, renderCoord);
        float2 p3  = CurveLoad(int2(curveLoc.x + 1, curveLoc.y)).xy - renderCoord;

        if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5f) break;

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

    return CalcCoverage(xcov, ycov, xwgt, ywgt, glyphData.w);
}

float4 main_ps(PS_INPUT input) : SV_Target
{
    float coverage = SlugRender(input.texcoord, input.banding, input.glyph);
    return input.color * coverage;
}
