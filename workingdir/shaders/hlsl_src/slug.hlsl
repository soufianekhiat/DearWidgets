// Slug GPU Font Rendering - HLSL Shader (VS + PS)
// Based on the Slug Algorithm by Eric Lengyel (public domain, 2026)
// https://sluglibrary.com
//
// Single draw call design: all glyphs in a DrawText call share one geometry
// draw command. Per-glyph Slug data is stored in a cbuffer array (slugGlyphData),
// indexed by glyph index encoded in the vertex color (R=low byte, G=high byte).
// Entry points: main_vs (vertex), main_ps (pixel)

// ---- Constant Buffers -------------------------------------------------------

// b0: ImGui's standard projection matrix (set by ImGui DX11 backend)
cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjectionMatrix;
};

// b1: per-batch Slug data, set via ImPlatform_SetShaderUniform before each draw
// Layout (must match SlugBatchCBData trailing float array in C++):
//   slugTextColor       : RGBA text color as float4
//   slugGlyphData[0]    : bandLoc for glyph 0  (xy=bandTexX/Y, zw=bandMaxX/Y)
//   slugGlyphData[1]    : banding for glyph 0  (xy=bandScale,  zw=bandOffset)
//   slugGlyphData[2]    : bandLoc for glyph 1
//   slugGlyphData[3]    : banding for glyph 1
//   ... (2 float4s per glyph, up to 256 glyphs = 512 float4s)
cbuffer slugBatch : register(b1)
{
    float4 slugTextColor;
    float4 slugGlyphData[512];   // supports up to 256 glyphs per DrawText call
};

// ---- Textures ---------------------------------------------------------------

// curveTexture: each quadratic Bezier curve uses 2 consecutive texels
//   texel 0: (p1.x, p1.y, p2.x, p2.y)
//   texel 1: (p3.x, p3.y, 0, 0)
Texture2D<float4> curveTexture : register(t0);

// bandTexture: integers stored as exact floats
//   band headers: (count, offset, 0, 0)
//   curve refs:   (curveTexX, curveTexY, 0, 0)
Texture2D<float4> bandTexture  : register(t1);

// ---- Structs ----------------------------------------------------------------

// Matches ImDrawVert: pos(float2) + uv(float2) + col(RGBA8 UNORM)
struct VS_INPUT
{
    float2 pos : POSITION;   // screen-space position
    float2 uv  : TEXCOORD0;  // em-space render coordinate (Slug renderCoord)
    float4 col : COLOR0;     // glyph index encoded as UNORM (r=low byte, g=high byte)
};

struct PS_INPUT
{
    float4 pos                      : SV_POSITION;
    float2 renderCoord              : TEXCOORD0;  // em-space coord, interpolated
    nointerpolation uint glyphIndex : TEXCOORD1;  // constant across the primitive
};

// ---- Vertex Shader ----------------------------------------------------------

PS_INPUT main_vs(VS_INPUT input)
{
    PS_INPUT output;
    output.pos         = mul(ProjectionMatrix, float4(input.pos, 0.f, 1.f));
    output.renderCoord = input.uv;
    // Decode glyph index from UNORM vertex color: R=low byte, G=high byte
    // IM_COL32(gi & 0xFF, (gi >> 8) & 0xFF, 0, 255) → ABGR in memory → col.r=R, col.g=G
    output.glyphIndex  = (uint)(input.col.r * 255.0f + 0.5f)
                       | ((uint)(input.col.g * 255.0f + 0.5f) << 8);
    return output;
}

// ---- Pixel Shader -----------------------------------------------------------

#define SLUG_BAND_TEXTURE_WIDTH  4096
#define SLUG_LOG_BAND_TEX_WIDTH  12

int2 BandLoad(int2 coord)
{
    float4 raw = bandTexture.Load(int3(coord, 0));
    return int2(int(raw.r + 0.5f), int(raw.g + 0.5f));
}

float4 CurveLoad(int2 coord)
{
    return curveTexture.Load(int3(coord, 0));
}

int2 CalcBandLoc(int2 glyphLoc, int offset)
{
    int2 loc = int2(glyphLoc.x + offset, glyphLoc.y);
    loc.y += loc.x >> SLUG_LOG_BAND_TEX_WIDTH;
    loc.x &= SLUG_BAND_TEXTURE_WIDTH - 1;
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
    float wsum = max(xwgt + ywgt, 1.0f / 65536.0f);
    float cov  = max(abs(xcov * xwgt + ycov * ywgt) / wsum,
                     min(abs(xcov), abs(ycov)));
    return saturate(cov);
}

float4 main_ps(PS_INPUT input) : SV_Target
{
    float2 renderCoord = input.renderCoord;

    // Fetch per-glyph data from the batch cbuffer using the flat-shaded glyph index
    uint gi = input.glyphIndex;
    float4 bandLoc = slugGlyphData[gi * 2u + 0u];
    float4 banding = slugGlyphData[gi * 2u + 1u];

    int2 glyphLoc = int2(int(bandLoc.x + 0.5f), int(bandLoc.y + 0.5f));
    int2 bandMax  = int2(int(bandLoc.z + 0.5f), int(bandLoc.w + 0.5f));

    float2 emsPerPixel = fwidth(renderCoord);
    float2 pixelsPerEm = 1.0f / emsPerPixel;

    int2 bandIndex = clamp(int2(renderCoord * banding.xy + banding.zw),
                           int2(0, 0), bandMax);

    // ----- Horizontal bands -----
    float xcov = 0.0f, xwgt = 0.0f;
    int2 hData = BandLoad(int2(glyphLoc.x + bandIndex.y, glyphLoc.y));
    int2 hLoc  = CalcBandLoc(glyphLoc, hData.y);

    for (int ci = 0; ci < hData.x; ci++)
    {
        int2   ref      = BandLoad(int2(hLoc.x + ci, hLoc.y));
        int2   curveLoc = int2(ref.x, ref.y);
        float4 p12 = CurveLoad(curveLoc)                         - float4(renderCoord, renderCoord);
        float2 p3  = CurveLoad(int2(curveLoc.x+1, curveLoc.y)).xy - renderCoord;

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

    // ----- Vertical bands -----
    float ycov = 0.0f, ywgt = 0.0f;
    int2 vData = BandLoad(int2(glyphLoc.x + bandMax.y + 1 + bandIndex.x, glyphLoc.y));
    int2 vLoc  = CalcBandLoc(glyphLoc, vData.y);

    for (int ci = 0; ci < vData.x; ci++)
    {
        int2   ref      = BandLoad(int2(vLoc.x + ci, vLoc.y));
        int2   curveLoc = int2(ref.x, ref.y);
        float4 p12 = CurveLoad(curveLoc)                         - float4(renderCoord, renderCoord);
        float2 p3  = CurveLoad(int2(curveLoc.x+1, curveLoc.y)).xy - renderCoord;

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

    float coverage = CalcCoverage(xcov, ycov, xwgt, ywgt);
    return slugTextColor * coverage;
}
