// dear_widgets LookDevInspector -- two-image A/B compare shader.
//
// Samples texture0 (side A, bound via ImGui's AddImage) and texture1 (side B,
// bound via ImPlatform_SetShaderTexture). A divider line defined by pivot + angle
// selects which side is shown at each pixel. Per-side exposure / black / white /
// gamma controls applied independently.

cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjMtx;
};

cbuffer LookDevInspectorParams : register(b1)
{
    // Per-side controls: exposure stops, black point, white point, gamma.
    float4 sideA;           // (exposure, black, white, gamma)
    float4 sideB;
    // Divider.
    float4 divider;         // (pivot_u, pivot_v, angle_rad, swap)
    // Canvas info.
    float4 canvas;          // (w_px, h_px, 1/w_px, 1/h_px)
};

SamplerState sampler0;
Texture2D    texture0;     // side A (always bound via AddImage)
SamplerState sampler1;
Texture2D    texture1;     // side B (bound via ImPlatform_SetShaderTexture at "texture1" slot 1)

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

PS_INPUT main_vs(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(ProjMtx, float4(input.pos.xy, 0.0f, 1.0f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

float3 apply_tone(float3 rgb, float4 params)
{
    // exposure in stops
    rgb *= exp2(params.x);
    // black / white
    rgb = (rgb - params.y) / max(params.z - params.y, 1e-6f);
    rgb = max(rgb, 0.0f);
    // gamma
    rgb = pow(rgb, 1.0f / max(params.w, 1e-4f));
    return rgb;
}

float4 main_ps(PS_INPUT input) : SV_Target
{
    // Side test must be in the same coordinate space as the widget's visual
    // gizmo (pixel space). Scale UV.x by aspect so a 45-degree line in the
    // widget matches a 45-degree split on-screen regardless of widget shape.
    float aspect = canvas.x / max(canvas.y, 1.0);
    float cs = cos(divider.z);
    float sn = sin(divider.z);
    float2 p     = float2(input.uv.x * aspect, input.uv.y);
    float2 pivot = float2(divider.x  * aspect, divider.y);
    float d = (p.x - pivot.x) * cs + (p.y - pivot.y) * sn;
    bool onB = (d < 0.0f);
    if (divider.w > 0.5f) onB = !onB;  // swap

    float4 c;
    if (onB)
    {
        c = texture1.Sample(sampler1, input.uv);
        c.rgb = apply_tone(c.rgb, sideB);
    }
    else
    {
        c = texture0.Sample(sampler0, input.uv);
        c.rgb = apply_tone(c.rgb, sideA);
    }
    c.rgb = saturate(c.rgb);
    return c * input.col;
}
