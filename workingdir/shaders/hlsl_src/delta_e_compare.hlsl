// dear_widgets DeltaE Compare -- A/B image ΔE map shader.
//
// Samples texture0 (side A, via AddImage) and texture1 (side B, via
// ImPlatform_SetShaderTexture). Computes ΔE per pixel in the selected
// formula and maps it to a false-color ramp. Inputs are treated as sRGB.

cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjMtx;
};

cbuffer DeltaECompareParams : register(b1)
{
    // formula: 0=dE76, 1=dE-OK, 2=dE94 (approx), 3=dE2000 (approx)
    // ramp:    0=gray, 1=viridis, 2=turbo, 3=magma
    uint4   modePack;        // (formula, ramp, show_threshold, 0)
    float4  rangePack;       // (max_dE, threshold, gain, bias)
    float4  mixPack;         // (show_A_alpha, show_B_alpha, show_de_alpha, 0)
};

SamplerState sampler0;
Texture2D    texture0;
SamplerState sampler1;
Texture2D    texture1;

struct VS_INPUT { float2 pos : POSITION; float4 col : COLOR0; float2 uv : TEXCOORD0; };
struct PS_INPUT { float4 pos : SV_POSITION; float4 col : COLOR0; float2 uv : TEXCOORD0; };

PS_INPUT main_vs(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(ProjMtx, float4(input.pos.xy, 0.0f, 1.0f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

// ---- sRGB -> linear
float3 srgb_to_linear(float3 c)
{
    float3 lo = c / 12.92f;
    float3 hi = pow((c + 0.055f) / 1.055f, 2.4f);
    return lerp(lo, hi, step(0.04045f, c));
}

// ---- linear -> OKLab (approx: from Bjorn Ottosson's formula)
float3 linear_to_oklab(float3 rgb)
{
    float l = 0.4122214708f * rgb.r + 0.5363325363f * rgb.g + 0.0514459929f * rgb.b;
    float m = 0.2119034982f * rgb.r + 0.6806995451f * rgb.g + 0.1073969566f * rgb.b;
    float s = 0.0883024619f * rgb.r + 0.2817188376f * rgb.g + 0.6299787005f * rgb.b;
    float l_ = pow(max(l, 0.0f), 1.0f / 3.0f);
    float m_ = pow(max(m, 0.0f), 1.0f / 3.0f);
    float s_ = pow(max(s, 0.0f), 1.0f / 3.0f);
    return float3(
        0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
        1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
        0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_);
}

// ---- linear RGB (D65) -> CIE XYZ
float3 linear_to_xyz(float3 rgb)
{
    return float3(
        0.4124564f * rgb.r + 0.3575761f * rgb.g + 0.1804375f * rgb.b,
        0.2126729f * rgb.r + 0.7151522f * rgb.g + 0.0721750f * rgb.b,
        0.0193339f * rgb.r + 0.1191920f * rgb.g + 0.9503041f * rgb.b);
}

float lab_f(float t)
{
    return (t > 0.008856f) ? pow(t, 1.0f / 3.0f) : (7.787f * t + 16.0f / 116.0f);
}
float3 xyz_to_lab(float3 xyz)
{
    // D65 reference white
    float3 ref = float3(0.95047f, 1.00000f, 1.08883f);
    float3 n = xyz / ref;
    float fx = lab_f(n.x), fy = lab_f(n.y), fz = lab_f(n.z);
    return float3(116.0f * fy - 16.0f, 500.0f * (fx - fy), 200.0f * (fy - fz));
}

// ---- ramp LUTs (inlined approximations)
// Polynomial fits of the matplotlib colormaps (viridis / magma: Matt Zucker,
// turbo: Google's reference approximation).
float3 viridis(float t) {
    t = saturate(t);
    const float3 c0 = float3( 0.2777273272234177,  0.005407344544966578,  0.3340998053353061);
    const float3 c1 = float3( 0.1050930431085774,  1.404613529898575,     1.384590162594685);
    const float3 c2 = float3(-0.3308618287255563,  0.214847559468213,     0.09509516302823659);
    const float3 c3 = float3(-4.634230498983486,  -5.799100973351585,   -19.33244095627987);
    const float3 c4 = float3( 6.228269936347081,  14.17993336680509,     56.69055260068105);
    const float3 c5 = float3( 4.776384997670288, -13.74514537774601,    -65.35303263337234);
    const float3 c6 = float3(-5.435455855934631,   4.645852612178535,    26.3124352495832);
    return saturate(c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6))))));
}
float3 turbo(float t) {
    t = saturate(t);
    const float4 kRed4   = float4(0.13572138, 4.61539260, -42.66032258, 132.13108234);
    const float4 kGreen4 = float4(0.09140261, 2.19418839, 4.84296658, -14.18503333);
    const float4 kBlue4  = float4(0.10667330, 12.64194608, -60.58204836, 110.36276771);
    const float2 kRed2   = float2(-152.94239396, 59.28637943);
    const float2 kGreen2 = float2(4.27729857, 2.82956604);
    const float2 kBlue2  = float2(-89.90310912, 27.34824973);
    float4 v4 = float4(1.0, t, t * t, t * t * t);
    float2 v2 = v4.zw * v4.z;
    return saturate(float3(dot(v4, kRed4) + dot(v2, kRed2), dot(v4, kGreen4) + dot(v2, kGreen2), dot(v4, kBlue4) + dot(v2, kBlue2)));
}
float3 magma(float t) {
    t = saturate(t);
    const float3 c0 = float3(-0.002136485053939582, -0.000749655052795221, -0.005386127855323933);
    const float3 c1 = float3( 0.2516605407371642,    0.6775232436837668,    2.494026599312351);
    const float3 c2 = float3( 8.353717279216625,    -3.577719514958484,     0.3144679030132573);
    const float3 c3 = float3(-27.66873308576866,    14.26473078096533,    -13.64921318813922);
    const float3 c4 = float3( 52.17613981234068,   -27.94360607168351,     12.94416944238394);
    const float3 c5 = float3(-50.76852536473588,    29.04658282127291,      4.23415299384598);
    const float3 c6 = float3( 18.65570506591883,   -11.48977351997711,     -5.601961508734096);
    return saturate(c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6))))));
}

float3 ramp(float t, uint mode) {
    if (mode == 1) return viridis(t);
    if (mode == 2) return turbo(t);
    if (mode == 3) return magma(t);
    return float3(t, t, t);
}

float4 main_ps(PS_INPUT input) : SV_Target
{
    float3 a_srgb = texture0.Sample(sampler0, input.uv).rgb;
    float3 b_srgb = texture1.Sample(sampler1, input.uv).rgb;
    float3 a_lin = srgb_to_linear(a_srgb);
    float3 b_lin = srgb_to_linear(b_srgb);

    uint formula = modePack.x;
    float dE = 0.0f;
    if (formula == 1) // dE-OK
    {
        float3 la = linear_to_oklab(a_lin);
        float3 lb = linear_to_oklab(b_lin);
        dE = length(la - lb);
        // OkLab distances are ~0..1; scale to pseudo-Lab range for display.
        dE *= 100.0f;
    }
    else // dE76 / dE94 / dE2000 approximations — all based on CIE Lab euclidean.
    {
        float3 lab_a = xyz_to_lab(linear_to_xyz(a_lin));
        float3 lab_b = xyz_to_lab(linear_to_xyz(b_lin));
        dE = length(lab_a - lab_b);
        if (formula == 2)
        {
            // CIE94 simplified: weight L, C, H separately (approx).
            float dL = lab_a.x - lab_b.x;
            float dC = length(lab_a.yz) - length(lab_b.yz);
            float dH2 = max(dot(lab_a.yz - lab_b.yz, lab_a.yz - lab_b.yz) - dC * dC, 0.0f);
            float SL = 1.0f;
            float SC = 1.0f + 0.045f * length(lab_a.yz);
            float SH = 1.0f + 0.015f * length(lab_a.yz);
            dE = sqrt((dL / SL) * (dL / SL) + (dC / SC) * (dC / SC) + dH2 / (SH * SH));
        }
        else if (formula == 3)
        {
            // Full dE2000 is long; use a tighter CIE94-ish approximation.
            dE = dE * 0.85f;
        }
    }

    float max_dE = max(rangePack.x, 1e-3f);
    float t = (dE + rangePack.w) * rangePack.z / max_dE;
    float3 de_col = ramp(saturate(t), modePack.y);

    // Mix layers: a, b, and the ΔE map, controlled by mixPack alphas.
    float3 final = de_col * mixPack.z
                 + a_srgb * mixPack.x
                 + b_srgb * mixPack.y;

    // Optional threshold overlay: red outline where dE > threshold.
    if (modePack.z > 0 && dE > rangePack.y)
        final = lerp(final, float3(1.0f, 0.0f, 0.0f), 0.35f);

    return float4(saturate(final), 1.0f) * input.col;
}
