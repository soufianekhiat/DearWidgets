// dear_widgets ImageInspector -- color-managed raw-buffer uber-shader.
//
// Decodes any of 11 sample types x 1..4 channels (44 combinations) from a
// tight-packed RGBA32F texture, then applies the full color pipeline:
//
//   raw bytes -> decode -> mosaic decode -> NaN check -> input transfer
//   -> input gamut -> exposure/black/white -> temp/tint -> tonemap
//   -> output gamut -> output transfer -> channel mask -> false color
//
// Single uber-shader, dynamic uniform branching only -- no preprocessor
// permutations. Cross-compiled by Slang to GLSL/MSL/WGSL/WGPU at build time.
//
// Bandwidth = (widget pixels) x (filter taps), independent of source image
// size. Subsample at footprint center for zoom-out (no mipmap chain).

// ============================================================================
// Vertex constant buffer (ImGui-provided)
// ============================================================================
cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjMtx;
};

// ============================================================================
// Pixel constant buffer (ImageInspectorParams)
// ============================================================================
// Field order matches the C++ ImageInspectorParams struct in
// dear_widgets_image_inspector.cpp. Uniforms are bound by name via
// ImPlatform_SetShaderUniform so layout matters less than naming.
cbuffer ImageInspectorParams : register(b1)
{
    // image extents and inverse extents
    float4 imgSize;          // (w, h, 1/w, 1/h)
    // pan/zoom in image space and the fit-to-widget scale
    float4 panZoom;          // (pan.x, pan.y, zoom, fitScale)
    // widget pixel extents
    float4 viewportPx;       // (widget_w, widget_h, 1/widget_w, 1/widget_h)
    // packed-texture geometry (in texels) and total byte count
    uint4  packedTexDims;    // (packedTexW, packedTexH, totalBytes, log2packedTexW)
    // on-GPU strides (bytes) for the tight-packed buffer
    uint4  layoutPack;       // (gpu_x_stride, gpu_y_stride, gpu_c_stride, 0)
    // sample-type / channel / filter / mosaic-pattern enums
    uint4  formatPack;       // (sample_type, channels, mosaic_pattern, filter)
    // exposure block: stops, black point, white point, gamma exponent
    float4 exposureParams;   // (exposure_stops, black, white, gamma)
    // temp/tint
    float4 tempTint;         // (temp_offset, tint, 0, 0)
    // input gamut -> working space (3x3 packed as 3 float4 rows; .w unused)
    float4 inGamut_r0;
    float4 inGamut_r1;
    float4 inGamut_r2;
    // working space -> output gamut (3x3 packed as 3 float4 rows)
    float4 outGamut_r0;
    float4 outGamut_r1;
    float4 outGamut_r2;
    // input transfer / output transfer / tonemap / false color enums
    uint4  pipelinePack;     // (input_transfer, output_transfer, tonemap, false_color)
    // per-channel mask
    float4 channelMask;      // (R, G, B, A weights)
    // NaN/Inf highlight color
    float4 nanColor;
    // mosaic mode (raw vs bilinear demosaic) and reserved
    uint4  modePack;         // (mosaic_mode, 0, 0, 0)
};

// ============================================================================
// ImGui-bound texture (the packed RGBA32F byte buffer)
// ============================================================================
// This is bound automatically by ImGui's pipeline because we pass the packed
// texture to AddImage(). Sampler is declared for binding compatibility but
// never used -- every fetch is a Load() (point fetch).
SamplerState sampler0;
Texture2D    texture0;

// ============================================================================
// Vertex / Pixel structs
// ============================================================================
struct VS_INPUT
{
    float2 pos : POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 col : COLOR0;
    float2 uv  : TEXCOORD0;
};

// ============================================================================
// Sample type constants (mirror ImSampleType in ImPlatform.h)
// ============================================================================
#define ST_U8   0u
#define ST_I8   1u
#define ST_U16  2u
#define ST_I16  3u
#define ST_U32  4u
#define ST_I32  5u
#define ST_U64  6u
#define ST_I64  7u
#define ST_F16  8u
#define ST_F32  9u
#define ST_F64 10u

// Mosaic pattern constants (mirror ImMosaicPattern in dear_widgets.h)
#define MP_NONE   0u
#define MP_RGGB   1u
#define MP_GRBG   2u
#define MP_GBRG   3u
#define MP_BGGR   4u
#define MP_XTRANS 5u

// Mosaic mode
#define MM_RAW       0u
#define MM_BILINEAR  1u

// Filter constants
#define FILT_NEAREST   0u
#define FILT_BILINEAR  1u
#define FILT_MITCHELL  2u
#define FILT_CATMULL   3u
#define FILT_LANCZOS2  4u
#define FILT_LANCZOS3  5u

// Input transfer constants
#define TX_LINEAR     0u
#define TX_GAMMA      1u
#define TX_SRGB       2u
#define TX_REC709     3u
#define TX_REC1886    4u
#define TX_CINEON     5u
#define TX_SLOG2      6u
#define TX_SLOG3      7u
#define TX_LOGC3      8u
#define TX_LOGC4      9u
#define TX_CANONLOG  10u
#define TX_CANONLOG2 11u
#define TX_CANONLOG3 12u
#define TX_VLOG      13u
#define TX_LOG3G10   14u
#define TX_BMFILMG5  15u
#define TX_APPLELOG  16u
#define TX_FLOG      17u
#define TX_DLOG      18u
#define TX_PQ        19u
#define TX_HLG       20u

// Output transfer constants
#define OTX_LINEAR  0u
#define OTX_GAMMA   1u
#define OTX_SRGB    2u
#define OTX_PQ      3u
#define OTX_HLG     4u

// Tonemap constants
#define TM_NONE        0u
#define TM_REINHARD    1u
#define TM_REINHARDX   2u
#define TM_ACES        3u
#define TM_AGX         4u
#define TM_PBRNEUTRAL  5u
#define TM_HABLE       6u

// False color constants
#define FC_OFF        0u
#define FC_VIRIDIS    1u
#define FC_MAGMA      2u
#define FC_INFERNO    3u
#define FC_PLASMA     4u
#define FC_CIVIDIS    5u
#define FC_TURBO      6u
#define FC_CINEMA     7u
#define FC_OOG        8u

// ============================================================================
// Sign-extension helpers (HLSL has no native int8/int16)
// ============================================================================
int SignExtend8(uint v)  { return (int(v) << 24) >> 24; }
int SignExtend16(uint v) { return (int(v) << 16) >> 16; }

// ============================================================================
// Byte-buffer fetch: load a 32-bit word at a given byte offset
// ============================================================================
// Maps byte offset -> (texel x, texel y, lane 0..3 within RGBA32F texel).
// Returns the 32-bit word at that lane, reinterpreted as uint.
uint LoadWord(uint byte_off)
{
    uint word_idx = byte_off >> 2u;          // 32-bit word index
    uint lane     = word_idx & 3u;            // 0..3 within texel
    uint texel    = word_idx >> 2u;           // texel index
    uint tw       = packedTexDims.x;
    int  tx       = int(texel & (tw - 1u));
    int  ty       = int(texel / tw);
    uint4 raw = asuint(texture0.Load(int3(tx, ty, 0)));
    return raw[lane];
}

// ============================================================================
// FetchSample(sx, sy, c)
// ============================================================================
// Decodes one channel of one source pixel as a float in working units.
// Integer types map to [0, 1] (unsigned) or [-1, 1] (signed) by dividing by
// max representable. Float types are returned raw (not normalized).
//
// 64-bit emulation: U64/I64 are reconstructed from two 32-bit halves with
// precision loss above ~2^24 in display. F64 -> F32 with truncated mantissa.
// CPU inspector uses the exact value, this is display-only.
//
// IMPORTANT: the type dispatch uses a [branch] switch instead of an if-else
// chain. fxc handles switches as jump tables and won't predicate all 11
// branches into one giant flattened expression -- crucial because this
// function is called many times per output pixel.
float FetchSample(int sx, int sy, int c)
{
    uint sample_type = formatPack.x;
    uint x_stride    = layoutPack.x;
    uint y_stride    = layoutPack.y;
    uint c_stride    = layoutPack.z;
    uint off         = uint(sy) * y_stride + uint(sx) * x_stride + uint(c) * c_stride;

    uint w0 = LoadWord(off);
    float result = 0.0f;

    [branch] switch (sample_type)
    {
    case ST_U8:
    {
        uint shift = (off & 3u) * 8u;
        result = float((w0 >> shift) & 0xFFu) / 255.0f;
        break;
    }
    case ST_I8:
    {
        uint shift = (off & 3u) * 8u;
        result = float(SignExtend8((w0 >> shift) & 0xFFu)) / 127.0f;
        break;
    }
    case ST_U16:
    {
        uint shift = (off & 3u) * 8u;
        result = float((w0 >> shift) & 0xFFFFu) / 65535.0f;
        break;
    }
    case ST_I16:
    {
        uint shift = (off & 3u) * 8u;
        result = float(SignExtend16((w0 >> shift) & 0xFFFFu)) / 32767.0f;
        break;
    }
    case ST_U32:
        result = float(w0) / 4294967295.0f;
        break;
    case ST_I32:
        result = float(asint(w0)) / 2147483647.0f;
        break;
    case ST_F16:
    {
        uint shift = (off & 2u) * 8u;
        result = f16tof32((w0 >> shift) & 0xFFFFu);
        break;
    }
    case ST_F32:
        result = asfloat(w0);
        break;
    case ST_U64:
    {
        uint lo = w0;
        uint hi = LoadWord(off + 4u);
        result = float(lo) + float(hi) * 4294967296.0f;
        break;
    }
    case ST_I64:
    {
        uint lo = w0;
        uint hi = LoadWord(off + 4u);
        [branch] if ((hi & 0x80000000u) != 0u)
        {
            uint nlo = ~lo + 1u;
            uint nhi = ~hi + (nlo == 0u ? 1u : 0u);
            result = -(float(nlo) + float(nhi) * 4294967296.0f);
        }
        else
        {
            result = float(lo) + float(hi) * 4294967296.0f;
        }
        break;
    }
    case ST_F64:
    {
        uint lo = w0;
        uint hi = LoadWord(off + 4u);
        uint sign    = hi & 0x80000000u;
        int  exp64   = int((hi >> 20u) & 0x7FFu) - 1023;
        uint mant_hi = hi & 0xFFFFFu;
        uint mant32  = (mant_hi << 3u) | (lo >> 29u);
        int  exp32   = exp64 + 127;
        [branch] if (exp32 <= 0)        result = asfloat(sign);
        else      if (exp32 >= 255)     result = asfloat(sign | 0x7F800000u);
        else                            result = asfloat(sign | (uint(exp32) << 23u) | mant32);
        break;
    }
    default:
        result = 0.0f;
        break;
    }
    return result;
}

// ============================================================================
// FetchPixel -- decode all channels of a source pixel into float4
// ============================================================================
// Single-channel images replicate Y to RGB; alpha is 1.
// 2-channel images put R/G in their slots, B = 0, A = 1.
// 3-channel images set A = 1.
float4 FetchPixel(int sx, int sy)
{
    int channels = int(formatPack.y);
    // Clamp to image bounds
    sx = clamp(sx, 0, int(imgSize.x) - 1);
    sy = clamp(sy, 0, int(imgSize.y) - 1);
    float4 v = float4(0.0f, 0.0f, 0.0f, 1.0f);
    if (channels == 1)
    {
        float y = FetchSample(sx, sy, 0);
        v = float4(y, y, y, 1.0f);
    }
    else if (channels == 2)
    {
        v.r = FetchSample(sx, sy, 0);
        v.g = FetchSample(sx, sy, 1);
    }
    else if (channels == 3)
    {
        v.r = FetchSample(sx, sy, 0);
        v.g = FetchSample(sx, sy, 1);
        v.b = FetchSample(sx, sy, 2);
    }
    else // 4
    {
        v.r = FetchSample(sx, sy, 0);
        v.g = FetchSample(sx, sy, 1);
        v.b = FetchSample(sx, sy, 2);
        v.a = FetchSample(sx, sy, 3);
    }
    return v;
}

// ============================================================================
// Mosaic decode (Bayer / X-Trans)
// ============================================================================
// For mosaic data the source is single-channel but logically tiled by a CFA.
// Raw mode shows the raw value replicated to RGB (gray with green tint).
// Bilinear mode does a cheap demosaic by averaging neighbors of the same color.
//
// This is called BEFORE filtering; the filter then runs on the demosaiced
// values. (For inspection workloads the filter is typically Nearest anyway.)
float4 MosaicSampleBayer(int sx, int sy, uint pattern)
{
    // Identify which color this pixel is in the 2x2 pattern
    uint xp = uint(sx & 1);
    uint yp = uint(sy & 1);
    // Pattern layouts (top-left, top-right, bot-left, bot-right):
    //   RGGB: R G / G B
    //   GRBG: G R / B G
    //   GBRG: G B / R G
    //   BGGR: B G / G R
    // Index = yp * 2 + xp; value 0=R, 1=G, 2=B
    int color;
    if (pattern == MP_RGGB)
        color = (yp == 0u) ? (xp == 0u ? 0 : 1) : (xp == 0u ? 1 : 2);
    else if (pattern == MP_GRBG)
        color = (yp == 0u) ? (xp == 0u ? 1 : 0) : (xp == 0u ? 2 : 1);
    else if (pattern == MP_GBRG)
        color = (yp == 0u) ? (xp == 0u ? 1 : 2) : (xp == 0u ? 0 : 1);
    else // BGGR
        color = (yp == 0u) ? (xp == 0u ? 2 : 1) : (xp == 0u ? 1 : 0);

    if (modePack.x == MM_RAW)
    {
        // Raw passthrough: replicate to RGB. Add a faint tint by color.
        float v = FetchSample(sx, sy, 0);
        float3 tint = float3(1.0f, 1.0f, 1.0f);
        if (color == 0) tint = float3(1.10f, 0.95f, 0.95f);
        if (color == 1) tint = float3(0.95f, 1.10f, 0.95f);
        if (color == 2) tint = float3(0.95f, 0.95f, 1.10f);
        return float4(v * tint, 1.0f);
    }

    // Bilinear demosaic -- for each channel, average the nearest pixels of that
    // color. This is the cheapest "good enough" method. [loop] forces fxc to
    // emit a runtime loop instead of auto-unrolling (body is too large).
    float r = 0.0f, g = 0.0f, b = 0.0f;
    int   nr = 0,    ng = 0,    nb = 0;
    [loop] for (int dy = -1; dy <= 1; ++dy)
    {
        [loop] for (int dx = -1; dx <= 1; ++dx)
        {
            int x2 = sx + dx;
            int y2 = sy + dy;
            uint xp2 = uint(x2 & 1);
            uint yp2 = uint(y2 & 1);
            int c2;
            if (pattern == MP_RGGB)
                c2 = (yp2 == 0u) ? (xp2 == 0u ? 0 : 1) : (xp2 == 0u ? 1 : 2);
            else if (pattern == MP_GRBG)
                c2 = (yp2 == 0u) ? (xp2 == 0u ? 1 : 0) : (xp2 == 0u ? 2 : 1);
            else if (pattern == MP_GBRG)
                c2 = (yp2 == 0u) ? (xp2 == 0u ? 1 : 2) : (xp2 == 0u ? 0 : 1);
            else
                c2 = (yp2 == 0u) ? (xp2 == 0u ? 2 : 1) : (xp2 == 0u ? 1 : 0);

            int xc = clamp(x2, 0, int(imgSize.x) - 1);
            int yc = clamp(y2, 0, int(imgSize.y) - 1);
            float v = FetchSample(xc, yc, 0);
            if (c2 == 0) { r += v; nr += 1; }
            if (c2 == 1) { g += v; ng += 1; }
            if (c2 == 2) { b += v; nb += 1; }
        }
    }
    if (nr > 0) r /= float(nr);
    if (ng > 0) g /= float(ng);
    if (nb > 0) b /= float(nb);
    return float4(r, g, b, 1.0f);
}

// X-Trans is rarer; v1 ships a "average within 6x6 cell" placeholder which
// gives a usable but desaturated preview. Real X-Trans demosaic is out of scope.
float4 MosaicSampleXTrans(int sx, int sy)
{
    if (modePack.x == MM_RAW)
    {
        float v = FetchSample(sx, sy, 0);
        return float4(v, v, v, 1.0f);
    }
    // Average 6x6 cell -- slow but correct intent for preview
    // sx/sy are clamped non-negative by FetchPixelMosaic before this call,
    // so the uint cast is safe and silences X3556 (signed-int modulus warning).
    float sum = 0.0f;
    int   n   = 0;
    int   x0  = sx - int(uint(sx) % 6u);
    int   y0  = sy - int(uint(sy) % 6u);
    [loop] for (int j = 0; j < 6; ++j)
    {
        [loop] for (int i = 0; i < 6; ++i)
        {
            int xc = clamp(x0 + i, 0, int(imgSize.x) - 1);
            int yc = clamp(y0 + j, 0, int(imgSize.y) - 1);
            sum += FetchSample(xc, yc, 0);
            n   += 1;
        }
    }
    float v = (n > 0) ? sum / float(n) : 0.0f;
    return float4(v, v, v, 1.0f);
}

// Top-level mosaic-aware fetch. Falls back to FetchPixel if mosaic == None.
// [branch] avoids flattening: the common case (None) bypasses the entire mosaic
// path and just calls FetchPixel directly.
float4 FetchPixelMosaic(int sx, int sy)
{
    uint pattern = formatPack.z;
    [branch] if (pattern == MP_NONE)
        return FetchPixel(sx, sy);
    sx = clamp(sx, 0, int(imgSize.x) - 1);
    sy = clamp(sy, 0, int(imgSize.y) - 1);
    [branch] if (pattern == MP_XTRANS)
        return MosaicSampleXTrans(sx, sy);
    return MosaicSampleBayer(sx, sy, pattern);
}

// ============================================================================
// Filter implementations (run on the demosaiced/decoded source values)
// ============================================================================
float4 FilterNearest(float2 src)
{
    int sx = int(floor(src.x + 0.5f));
    int sy = int(floor(src.y + 0.5f));
    return FetchPixelMosaic(sx, sy);
}

float4 FilterBilinear(float2 src)
{
    float fx = src.x - 0.5f;
    float fy = src.y - 0.5f;
    int   x0 = int(floor(fx));
    int   y0 = int(floor(fy));
    float tx = fx - float(x0);
    float ty = fy - float(y0);
    float4 c00 = FetchPixelMosaic(x0,     y0);
    float4 c10 = FetchPixelMosaic(x0 + 1, y0);
    float4 c01 = FetchPixelMosaic(x0,     y0 + 1);
    float4 c11 = FetchPixelMosaic(x0 + 1, y0 + 1);
    float4 c0  = lerp(c00, c10, tx);
    float4 c1  = lerp(c01, c11, tx);
    return lerp(c0, c1, ty);
}

// Mitchell-Netravali / Catmull-Rom kernel: BC family.
// Mitchell: B = 1/3, C = 1/3
// Catmull-Rom: B = 0, C = 1/2
float MitchellWeight(float x, float B, float C)
{
    float ax = abs(x);
    float ax2 = ax * ax;
    float ax3 = ax2 * ax;
    if (ax < 1.0f)
    {
        return ((12.0f - 9.0f*B - 6.0f*C) * ax3
              + (-18.0f + 12.0f*B + 6.0f*C) * ax2
              + (6.0f - 2.0f*B)) * (1.0f / 6.0f);
    }
    if (ax < 2.0f)
    {
        return ((-B - 6.0f*C) * ax3
              + (6.0f*B + 30.0f*C) * ax2
              + (-12.0f*B - 48.0f*C) * ax
              + (8.0f*B + 24.0f*C)) * (1.0f / 6.0f);
    }
    return 0.0f;
}

float4 FilterBicubic(float2 src, float B, float C)
{
    float fx = src.x - 0.5f;
    float fy = src.y - 0.5f;
    int   ix = int(floor(fx));
    int   iy = int(floor(fy));
    float dx = fx - float(ix);
    float dy = fy - float(iy);
    float4 sum = float4(0, 0, 0, 0);
    float  ws  = 0.0f;
    // [loop] forces fxc to emit a runtime loop instead of unrolling 16 copies
    // of FetchPixelMosaic -> FetchSample (which would explode the shader).
    [loop] for (int j = -1; j <= 2; ++j)
    {
        float wy = MitchellWeight(float(j) - dy, B, C);
        [loop] for (int i = -1; i <= 2; ++i)
        {
            float wx = MitchellWeight(float(i) - dx, B, C);
            float w  = wx * wy;
            sum += w * FetchPixelMosaic(ix + i, iy + j);
            ws  += w;
        }
    }
    return (ws != 0.0f) ? (sum / ws) : sum;
}

float Sinc(float x)
{
    if (abs(x) < 1e-6f) return 1.0f;
    float px = 3.14159265358979323846f * x;
    return sin(px) / px;
}

float LanczosWeight(float x, float a)
{
    float ax = abs(x);
    if (ax >= a) return 0.0f;
    return Sinc(x) * Sinc(x / a);
}

float4 FilterLanczos(float2 src, int a)
{
    float fx = src.x - 0.5f;
    float fy = src.y - 0.5f;
    int   ix = int(floor(fx));
    int   iy = int(floor(fy));
    float dx = fx - float(ix);
    float dy = fy - float(iy);
    float4 sum = float4(0, 0, 0, 0);
    float  ws  = 0.0f;
    // Always iterate the full Lanczos3 range (-2..3 = 6 taps each axis = 36 total).
    // LanczosWeight returns 0 for |x| >= a, so smaller `a` automatically zeros out
    // the outer taps. [loop] prevents fxc from unrolling.
    [loop] for (int j = -2; j <= 3; ++j)
    {
        float wy = LanczosWeight(float(j) - dy, float(a));
        [loop] for (int i = -2; i <= 3; ++i)
        {
            float wx = LanczosWeight(float(i) - dx, float(a));
            float w  = wx * wy;
            sum += w * FetchPixelMosaic(ix + i, iy + j);
            ws  += w;
        }
    }
    return (ws != 0.0f) ? (sum / ws) : sum;
}

// ============================================================================
// Filter dispatch + zoom-out subsample
// ============================================================================
float4 SampleSource(float2 src, float src_step)
{
    // Zoom-out: snap to footprint center (no integration over the footprint --
    // honest subsample, may alias for inspection workloads).
    if (src_step > 1.0f)
    {
        int cx = int(floor(src.x));
        int cy = int(floor(src.y));
        return FetchPixelMosaic(cx, cy);
    }

    uint filter = formatPack.w;
    float4 r = float4(0, 0, 0, 1);
    [branch] switch (filter)
    {
    case FILT_NEAREST:  r = FilterNearest(src);                              break;
    case FILT_BILINEAR: r = FilterBilinear(src);                             break;
    case FILT_MITCHELL: r = FilterBicubic(src, 1.0f / 3.0f, 1.0f / 3.0f);    break;
    case FILT_CATMULL:  r = FilterBicubic(src, 0.0f, 0.5f);                  break;
    case FILT_LANCZOS2: r = FilterLanczos(src, 2);                           break;
    case FILT_LANCZOS3: r = FilterLanczos(src, 3);                           break;
    default:            r = FilterBilinear(src);                             break;
    }
    return r;
}

// ============================================================================
// Input transfer (inverse OETF / EOTF) -- raw -> scene-linear
// ============================================================================
// All log curve constants from manufacturer specs / ACES IDT references.
// For brevity each curve is implemented per-channel via a small inverse function.

float SrgbToLinear(float v)
{
    if (v <= 0.04045f) return v / 12.92f;
    return pow((v + 0.055f) / 1.055f, 2.4f);
}

float LinearToSrgb(float v)
{
    if (v <= 0.0031308f) return v * 12.92f;
    return 1.055f * pow(max(v, 0.0f), 1.0f / 2.4f) - 0.055f;
}

float Rec709Inverse(float v)
{
    // BT.709 OETF inverse
    if (v < 0.081f) return v / 4.5f;
    return pow((v + 0.099f) / 1.099f, 1.0f / 0.45f);
}

float Rec1886Inverse(float v)
{
    // Pure 2.4 gamma
    return pow(max(v, 0.0f), 2.4f);
}

float CineonInverse(float v)
{
    // Cineon log -> linear (10-bit code value space implied; here normalized [0,1])
    float code = v * 1023.0f;
    return pow(10.0f, (code - 685.0f) * 0.002f) - 0.0108f;
}

float SLog2Inverse(float v)
{
    // Sony S-Log2
    if (v >= 0.030001222851889727f)
        return ((pow(10.0f, ((v - 0.616596f - 0.03f) / 0.432699f)) - 0.037584f) * 219.0f) / 155.0f;
    return ((v - 0.030001222851889727f) / 3.53881278538813f) * 219.0f / 155.0f;
}

float SLog3Inverse(float v)
{
    // Sony S-Log3
    if (v >= 171.2102946929f / 1023.0f)
        return (pow(10.0f, ((v * 1023.0f - 420.0f) / 261.5f)) * (0.18f + 0.01f) - 0.01f);
    return (v * 1023.0f - 95.0f) * 0.01125f / (171.2102946929f - 95.0f);
}

float LogC3Inverse(float v)
{
    // ARRI LogC3 EI 800
    const float cut  = 0.149658f;
    const float a    = 5.555556f;
    const float b    = 0.052272f;
    const float c    = 0.247190f;
    const float d    = 0.385537f;
    const float e    = 5.367655f;
    const float f    = 0.092809f;
    if (v > e * cut + f)
        return (pow(10.0f, (v - d) / c) - b) / a;
    return (v - f) / e;
}

float LogC4Inverse(float v)
{
    // ARRI LogC4
    const float a = (pow(2.0f, 18.0f) - 16.0f) / 117.45f;
    const float b = (1023.0f - 95.0f) / 1023.0f;
    const float c = 95.0f / 1023.0f;
    const float s = (7.0f * log(2.0f) * pow(2.0f, 7.0f - 14.0f * c / b)) / (a * b);
    const float t = (pow(2.0f, 14.0f * (-c / b) + 6.0f) - 64.0f) / a;
    if (v < 0.0f) return v * s + t;
    float p = (v - c) / b;
    return (pow(2.0f, 14.0f * p + 6.0f) - 64.0f) / a;
}

float CanonLogInverse(float v)
{
    // Canon Log (original)
    if (v < 0.12512248f)
        return -(pow(10.0f, (0.12512248f - v) / 0.45310179f) - 1.0f) / 10.1596f;
    return (pow(10.0f, (v - 0.12512248f) / 0.45310179f) - 1.0f) / 10.1596f;
}

float CanonLog2Inverse(float v)
{
    // Canon Log 2
    if (v < 0.092864125f)
        return -(pow(10.0f, (0.092864125f - v) / 0.24136077f) - 1.0f) / 87.099375f;
    return (pow(10.0f, (v - 0.092864125f) / 0.24136077f) - 1.0f) / 87.099375f;
}

float CanonLog3Inverse(float v)
{
    // Canon Log 3 (piecewise linear/log)
    if (v < 0.097465473f)
        return -(pow(10.0f, (0.12783901f - v) / 0.36726845f) - 1.0f) / 14.98325f;
    if (v <= 0.15277891f)
        return (v - 0.12512219f) / 1.9754798f;
    return (pow(10.0f, (v - 0.12240537f) / 0.36726845f) - 1.0f) / 14.98325f;
}

float VLogInverse(float v)
{
    // Panasonic V-Log
    const float cut2 = 0.181f;
    const float b    = 0.00873f;
    const float c    = 0.241514f;
    const float d    = 0.598206f;
    if (v < cut2)
        return (v - 0.125f) / 5.6f;
    return pow(10.0f, (v - d) / c) - b;
}

float Log3G10Inverse(float v)
{
    // RED Log3G10
    float x = v / 0.222497f;
    float sign_x = (x < 0.0f) ? -1.0f : 1.0f;
    return sign_x * (pow(10.0f, abs(x)) - 1.0f) * 0.01f;
}

float BMFilmGen5Inverse(float v)
{
    // Blackmagic Film Gen 5 (approximate inverse)
    const float a = 0.08692876065491224f;
    const float b = 0.005494072432257808f;
    const float c = 0.5300133392291939f;
    const float d = 8.283605932402494f;
    const float e = 0.09246575342465754f;
    if (v < e)
        return (v - 0.092864f) / d;
    return (pow(2.0f, (v - c) / a) - b);
}

float AppleLogInverse(float v)
{
    // Apple Log (from Apple white paper, approximate)
    const float R0 = -0.05641088f;
    const float Rt = 0.01f;
    const float c  = 47.28711236f;
    const float beta = 0.00964456f;
    const float gamma = 0.08550479f;
    const float delta = 0.69336945f;
    if (v < beta)
        return ((v / c) + R0);
    return pow(2.0f, (v - delta) / gamma) - Rt;
}

float FLogInverse(float v)
{
    // Fujifilm F-Log
    const float cut2 = 0.100537775223865f;
    const float a    = 0.555556f;
    const float b    = 0.009468f;
    const float c    = 0.344676f;
    const float d    = 0.790453f;
    const float e    = 8.735631f;
    const float f    = 0.092864f;
    if (v < cut2)
        return (v - f) / e;
    return (pow(10.0f, (v - d) / c) - b) / a;
}

float DLogInverse(float v)
{
    // DJI D-Log
    if (v <= 0.14f)
        return (v - 0.0929f) / 6.025f;
    return (pow(10.0f, (v - 0.807495f) / 0.2556207f) - 0.0108f) / 0.9892f;
}

float PQInverse(float v)
{
    // SMPTE ST.2084 (PQ) EOTF -- returns linear in [0, 10000] nits / 10000
    const float m1 = 2610.0f / 16384.0f;
    const float m2 = 2523.0f / 4096.0f * 128.0f;
    const float c1 = 3424.0f / 4096.0f;
    const float c2 = 2413.0f / 4096.0f * 32.0f;
    const float c3 = 2392.0f / 4096.0f * 32.0f;
    float vp = pow(max(v, 0.0f), 1.0f / m2);
    float num = max(vp - c1, 0.0f);
    float den = c2 - c3 * vp;
    return pow(num / den, 1.0f / m1);
}

float HLGInverse(float v)
{
    // BT.2100 HLG OETF inverse
    const float a = 0.17883277f;
    const float b = 0.28466892f;
    const float c = 0.55991073f;
    if (v <= 0.5f)
        return (v * v) / 3.0f;
    return (exp((v - c) / a) + b) / 12.0f;
}

float3 ApplyInputTransfer(float3 c)
{
    uint t = pipelinePack.x;
    float3 r = c;
    [branch] switch (t)
    {
    case TX_LINEAR:    r = c; break;
    case TX_GAMMA:     { float g = exposureParams.w;
                         r = float3(pow(max(c.r, 0.0f), g), pow(max(c.g, 0.0f), g), pow(max(c.b, 0.0f), g)); break; }
    case TX_SRGB:      r = float3(SrgbToLinear(c.r),    SrgbToLinear(c.g),    SrgbToLinear(c.b));    break;
    case TX_REC709:    r = float3(Rec709Inverse(c.r),   Rec709Inverse(c.g),   Rec709Inverse(c.b));   break;
    case TX_REC1886:   r = float3(Rec1886Inverse(c.r),  Rec1886Inverse(c.g),  Rec1886Inverse(c.b));  break;
    case TX_CINEON:    r = float3(CineonInverse(c.r),   CineonInverse(c.g),   CineonInverse(c.b));   break;
    case TX_SLOG2:     r = float3(SLog2Inverse(c.r),    SLog2Inverse(c.g),    SLog2Inverse(c.b));    break;
    case TX_SLOG3:     r = float3(SLog3Inverse(c.r),    SLog3Inverse(c.g),    SLog3Inverse(c.b));    break;
    case TX_LOGC3:     r = float3(LogC3Inverse(c.r),    LogC3Inverse(c.g),    LogC3Inverse(c.b));    break;
    case TX_LOGC4:     r = float3(LogC4Inverse(c.r),    LogC4Inverse(c.g),    LogC4Inverse(c.b));    break;
    case TX_CANONLOG:  r = float3(CanonLogInverse(c.r), CanonLogInverse(c.g), CanonLogInverse(c.b)); break;
    case TX_CANONLOG2: r = float3(CanonLog2Inverse(c.r),CanonLog2Inverse(c.g),CanonLog2Inverse(c.b));break;
    case TX_CANONLOG3: r = float3(CanonLog3Inverse(c.r),CanonLog3Inverse(c.g),CanonLog3Inverse(c.b));break;
    case TX_VLOG:      r = float3(VLogInverse(c.r),     VLogInverse(c.g),     VLogInverse(c.b));     break;
    case TX_LOG3G10:   r = float3(Log3G10Inverse(c.r),  Log3G10Inverse(c.g),  Log3G10Inverse(c.b));  break;
    case TX_BMFILMG5:  r = float3(BMFilmGen5Inverse(c.r),BMFilmGen5Inverse(c.g),BMFilmGen5Inverse(c.b));break;
    case TX_APPLELOG:  r = float3(AppleLogInverse(c.r), AppleLogInverse(c.g), AppleLogInverse(c.b)); break;
    case TX_FLOG:      r = float3(FLogInverse(c.r),     FLogInverse(c.g),     FLogInverse(c.b));     break;
    case TX_DLOG:      r = float3(DLogInverse(c.r),     DLogInverse(c.g),     DLogInverse(c.b));     break;
    case TX_PQ:        r = float3(PQInverse(c.r),       PQInverse(c.g),       PQInverse(c.b));       break;
    case TX_HLG:       r = float3(HLGInverse(c.r),      HLGInverse(c.g),      HLGInverse(c.b));      break;
    default:           r = c; break;
    }
    return r;
}

// ============================================================================
// Output transfer
// ============================================================================
float PQForward(float v)
{
    const float m1 = 2610.0f / 16384.0f;
    const float m2 = 2523.0f / 4096.0f * 128.0f;
    const float c1 = 3424.0f / 4096.0f;
    const float c2 = 2413.0f / 4096.0f * 32.0f;
    const float c3 = 2392.0f / 4096.0f * 32.0f;
    float lp = pow(max(v, 0.0f), m1);
    return pow((c1 + c2 * lp) / (1.0f + c3 * lp), m2);
}

float HLGForward(float v)
{
    const float a = 0.17883277f;
    const float b = 0.28466892f;
    const float c = 0.55991073f;
    if (v <= 1.0f / 12.0f)
        return sqrt(3.0f * v);
    return a * log(12.0f * v - b) + c;
}

float3 ApplyOutputTransfer(float3 c)
{
    uint t = pipelinePack.y;
    float3 r = c;
    [branch] switch (t)
    {
    case OTX_LINEAR: r = c; break;
    case OTX_GAMMA:  { float g = 1.0f / max(exposureParams.w, 0.001f);
                       r = float3(pow(max(c.r, 0.0f), g), pow(max(c.g, 0.0f), g), pow(max(c.b, 0.0f), g)); break; }
    case OTX_SRGB:   r = float3(LinearToSrgb(c.r), LinearToSrgb(c.g), LinearToSrgb(c.b)); break;
    case OTX_PQ:     r = float3(PQForward(c.r),    PQForward(c.g),    PQForward(c.b));    break;
    case OTX_HLG:    r = float3(HLGForward(c.r),   HLGForward(c.g),   HLGForward(c.b));   break;
    default:         r = c; break;
    }
    return r;
}

// ============================================================================
// Tonemap operators
// ============================================================================
float3 TonemapReinhard(float3 c)
{
    return c / (1.0f + c);
}

float3 TonemapReinhardExt(float3 c)
{
    float W = max(exposureParams.z, 0.001f);    // White point
    return (c * (1.0f + c / (W * W))) / (1.0f + c);
}

// Narkowicz ACES filmic
float3 TonemapACES(float3 c)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float d = 2.43f;
    const float e = 0.59f;
    const float f = 0.14f;
    return saturate((c * (a * c + b)) / (c * (d * c + e) + f));
}

// AGX (Troy Sobotka) -- simplified analytic fit
float3 TonemapAGX(float3 c)
{
    // Approximate AGX log encoding then 2-3-2 sigmoid
    float3 v = log2(max(c, 1e-10f));
    v = (v + 12.47393f) / (4.026069f + 12.47393f);
    v = saturate(v);
    // Polynomial sigmoid (Troy's approximation)
    float3 v2 = v * v;
    float3 v3 = v2 * v;
    float3 v4 = v2 * v2;
    return - 17.86f * v3 * v3
           + 78.01f * v4 * v
           - 126.7f * v4
           + 92.06f * v3
           - 28.72f * v2
           + 4.361f * v
           - 0.1718f;
}

// Khronos PBR Neutral
float3 TonemapPBRNeutral(float3 c)
{
    const float startCompression = 0.8f - 0.04f;
    const float desaturation     = 0.15f;
    float x  = min(c.r, min(c.g, c.b));
    float offset = (x < 0.08f) ? (x - 6.25f * x * x) : 0.04f;
    c -= offset;
    float peak = max(c.r, max(c.g, c.b));
    if (peak < startCompression) return c;
    float d = 1.0f - startCompression;
    float newPeak = 1.0f - d * d / (peak + d - startCompression);
    c *= newPeak / peak;
    float g = 1.0f - 1.0f / (desaturation * (peak - newPeak) + 1.0f);
    return lerp(c, newPeak.xxx, g);
}

float3 TonemapHable(float3 c)
{
    const float A = 0.15f;
    const float B = 0.50f;
    const float C = 0.10f;
    const float D = 0.20f;
    const float E = 0.02f;
    const float F = 0.30f;
    const float W = 11.2f;
    float3 num = ((c * (A * c + C * B) + D * E) / (c * (A * c + B) + D * F)) - E / F;
    float  den = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    return num / den;
}

float3 ApplyTonemap(float3 c)
{
    uint t = pipelinePack.z;
    float3 r = c;
    [branch] switch (t)
    {
    case TM_NONE:       r = c;                    break;
    case TM_REINHARD:   r = TonemapReinhard(c);   break;
    case TM_REINHARDX:  r = TonemapReinhardExt(c);break;
    case TM_ACES:       r = TonemapACES(c);       break;
    case TM_AGX:        r = TonemapAGX(c);        break;
    case TM_PBRNEUTRAL: r = TonemapPBRNeutral(c); break;
    case TM_HABLE:      r = TonemapHable(c);      break;
    default:            r = c;                    break;
    }
    return r;
}

// ============================================================================
// False color palettes -- analytic polynomial fits
// ============================================================================
// Viridis / Magma / Inferno / Plasma fits from Mike Bostock & Inigo Quilez
// (cubic polynomials matched to the original LUTs).
float3 PaletteViridis(float t)
{
    t = saturate(t);
    return float3(0.267f, 0.005f, 0.329f) +
        t * (float3(0.105f, 1.405f, 1.385f) +
        t * (float3(-0.330f, -0.317f, 0.214f) +
        t * (float3(6.228f, -2.520f, -2.665f) +
        t * (float3(-13.09f, 1.395f, 6.331f) +
        t * (float3(11.10f, 0.0f, -7.235f) +
        t *  float3(-3.658f, 0.0f, 2.093f))))));
}
float3 PaletteMagma(float t)
{
    t = saturate(t);
    return float3(-0.002f, -0.000f, -0.014f) +
        t * (float3(0.255f, 0.039f, 1.602f) +
        t * (float3(7.187f, 2.794f, 4.804f) +
        t * (float3(-25.95f, -7.701f, -23.39f) +
        t * (float3(38.21f, 8.502f, 38.30f) +
        t * (float3(-25.55f, -3.851f, -27.40f) +
        t *  float3(6.484f, 0.534f, 7.465f))))));
}
float3 PaletteInferno(float t)
{
    t = saturate(t);
    return float3(0.0002f, 0.0017f, -0.0193f) +
        t * (float3(0.106f, 0.561f, 3.987f) +
        t * (float3(11.602f, -3.972f, -15.94f) +
        t * (float3(-41.71f, 17.43f, 44.35f) +
        t * (float3(77.16f, -33.40f, -81.80f) +
        t * (float3(-71.32f, 32.63f, 73.21f) +
        t *  float3(25.13f, -12.24f, -23.07f))))));
}
float3 PalettePlasma(float t)
{
    t = saturate(t);
    return float3(0.0588f, 0.0297f, 0.5310f) +
        t * (float3(2.176f, 0.238f, 1.072f) +
        t * (float3(0.117f, 0.4806f, -3.029f) +
        t * (float3(-9.628f, -1.929f, 4.423f) +
        t * (float3(20.79f, 1.940f, -3.137f) +
        t * (float3(-17.06f, -0.733f, 0.965f) +
        t *  float3(4.688f, 0.0697f, -0.083f))))));
}
float3 PaletteCividis(float t)
{
    t = saturate(t);
    return float3(-0.0086f, 0.1322f, 0.3014f) +
        t * (float3(0.5444f, 0.7196f, 1.4523f) +
        t * (float3(-0.0908f, 0.1334f, -3.953f) +
        t * (float3(0.5563f, 0.0181f, 4.0846f) +
        t *  float3(-0.0017f, -0.0046f, -1.8857f))));
}
float3 PaletteTurbo(float t)
{
    // Google Turbo polynomial fit
    t = saturate(t);
    float3 r = float3(0.1357f, 0.0914f, 0.1067f);
    float3 a = float3(4.5974f, 2.1856f, 12.5925f);
    float3 b = float3(-42.660f, 4.8410f, -60.582f);
    float3 c = float3(132.13f, -14.185f, 110.51f);
    float3 d = float3(-152.94f, 4.2774f, -89.901f);
    float3 e = float3(59.286f, 2.7993f, 27.343f);
    return r + t*(a + t*(b + t*(c + t*(d + t*e))));
}

// Cinematographer scope: zones based on EV-like brightness.
float3 PaletteCinema(float v)
{
    if (v <  0.0f)  return float3(0.5f, 0.0f, 0.5f);  // Below black: purple
    if (v <  0.04f) return float3(0.0f, 0.0f, 1.0f);  // Deep shadow: blue
    if (v <  0.10f) return float3(0.0f, 0.5f, 1.0f);
    if (v <  0.18f) return float3(0.0f, 1.0f, 1.0f);
    if (v <  0.42f) return float3(0.5f, 1.0f, 0.0f);  // Mid grey: green
    if (v <  0.78f) return float3(1.0f, 1.0f, 0.0f);
    if (v <  0.95f) return float3(1.0f, 0.5f, 0.0f);
    if (v <= 1.0f)  return float3(1.0f, 0.0f, 0.0f);  // Near clip: red
    return float3(1.0f, 1.0f, 1.0f);                  // Above clip: white
}

float3 ApplyFalseColor(float3 c)
{
    uint mode = pipelinePack.w;
    float v = (c.r + c.g + c.b) * (1.0f / 3.0f);
    float3 r = c;
    [branch] switch (mode)
    {
    case FC_OFF:     r = c;                 break;
    case FC_VIRIDIS: r = PaletteViridis(v); break;
    case FC_MAGMA:   r = PaletteMagma(v);   break;
    case FC_INFERNO: r = PaletteInferno(v); break;
    case FC_PLASMA:  r = PalettePlasma(v);  break;
    case FC_CIVIDIS: r = PaletteCividis(v); break;
    case FC_TURBO:   r = PaletteTurbo(v);   break;
    case FC_CINEMA:  r = PaletteCinema(v);  break;
    case FC_OOG:
    {
        bool oog = (c.r < 0.0f) || (c.g < 0.0f) || (c.b < 0.0f) ||
                   (c.r > 1.0f) || (c.g > 1.0f) || (c.b > 1.0f);
        r = oog ? float3(1.0f, 1.0f, 0.0f) : c;
        break;
    }
    default: r = c; break;
    }
    return r;
}

// ============================================================================
// Gamut transform helpers
// ============================================================================
float3 MulMatrix(float4 r0, float4 r1, float4 r2, float3 v)
{
    return float3(dot(r0.xyz, v), dot(r1.xyz, v), dot(r2.xyz, v));
}

// ============================================================================
// Temperature/tint adjustment in a simplified LMS-like space
// ============================================================================
float3 ApplyTempTint(float3 c, float temp, float tint)
{
    // temp > 0 = warmer (boost R, reduce B); temp < 0 = cooler
    // tint > 0 = magenta (boost R+B, reduce G); tint < 0 = green
    float tw = temp * 0.1f;
    float tt = tint * 0.1f;
    c.r *= (1.0f + tw + tt);
    c.g *= (1.0f - tt);
    c.b *= (1.0f - tw + tt);
    return c;
}

// ============================================================================
// Vertex shader (standard ImGui vertex)
// ============================================================================
PS_INPUT main_vs(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(ProjMtx, float4(input.pos.xy, 0.0f, 1.0f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

// ============================================================================
// Pixel shader entry point -- wires the whole pipeline together
// ============================================================================
float4 main_ps(PS_INPUT input) : SV_Target
{
    // ---- 1. Coordinate mapping (output frag -> source pixel) ----
    // input.uv is the [0..1] UV across the widget rect.
    // Source coordinate in image-space pixels:
    //   src = (uv - 0.5) * (widgetSize / totalScale) + (imgSize/2 + pan)
    // where totalScale = fitScale * zoom.
    float totalScale = panZoom.z * panZoom.w;     // zoom * fitScale
    float2 widgetSz  = viewportPx.xy;
    float2 imgCenter = imgSize.xy * 0.5f + panZoom.xy;
    float2 src       = imgCenter + (input.uv - 0.5f) * widgetSz / totalScale;
    float  src_step  = 1.0f / max(totalScale, 1e-6f);

    // ---- 2/3. Filter dispatch + zoom-out subsample ----
    float4 sampled = SampleSource(src, src_step);
    float3 rgb = sampled.rgb;
    float  a   = sampled.a;

    // ---- 4. NaN / Inf highlight ----
    if (any(isnan(rgb)) || any(isinf(rgb)))
        return float4(nanColor.rgb, 1.0f);

    // ---- 5. Linearize via input transfer ----
    rgb = ApplyInputTransfer(rgb);

    // ---- 6. Input gamut -> working space ----
    rgb = MulMatrix(inGamut_r0, inGamut_r1, inGamut_r2, rgb);

    // ---- 7. Exposure / black / white ----
    float exposure_mul = exp2(exposureParams.x);
    rgb = (rgb - exposureParams.y) / max(exposureParams.z - exposureParams.y, 1e-6f);
    rgb *= exposure_mul;

    // ---- 8. Temperature / tint ----
    rgb = ApplyTempTint(rgb, tempTint.x, tempTint.y);

    // ---- 9. Tonemap ----
    rgb = ApplyTonemap(rgb);

    // ---- 10. Working -> output gamut ----
    rgb = MulMatrix(outGamut_r0, outGamut_r1, outGamut_r2, rgb);

    // ---- 11. Output transfer ----
    rgb = ApplyOutputTransfer(rgb);

    // ---- 12. Channel mask ----
    rgb *= channelMask.rgb;
    a   *= channelMask.a;

    // ---- 13. False color ----
    rgb = ApplyFalseColor(rgb);

    return float4(rgb, a) * input.col;
}
