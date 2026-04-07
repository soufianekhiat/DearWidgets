// Stroke fill shader — winding number evaluation per pixel.
// Matches the Linebender GPU stroke expansion paper (HPG 2024):
// CPU produces "line soup" (directed line segments), pixel shader
// evaluates winding number to determine inside/outside.

// Vertex shader constant buffer (ImGui projection matrix)
cbuffer vertexBuffer : register(b0)
{
    float4x4 ProjMtx;
};

// Pixel shader constant buffer
// Layout: params + color + bounds + segments[N]
// Total max size: 48 + 1024*16 = 16432 bytes
cbuffer strokeBuffer : register(b1)
{
    float4 params;          // x=num_segments, y=unused, z=aa_width, w=unused
    float4 strokeColor;     // RGBA
    float4 bounds;          // xy=rect_min, zw=rect_max (screen space)
    float4 segments[1024];  // xy=p0, zw=p1 for each directed segment
};

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
    output.pos = mul(ProjMtx, float4(input.pos.xy, 0.f, 1.f));
    output.col = input.col;
    output.uv  = input.uv;
    return output;
}

float4 main_ps(PS_INPUT input) : SV_Target
{
    // Map UV [0,1] to screen-space position within bounding box
    float2 P = lerp(bounds.xy, bounds.zw, input.uv.xy);
    int num = (int)params.x;
    float aa = params.z;

    // --- Winding number evaluation ---
    // Standard ray-casting: horizontal ray from P to +infinity.
    // For each segment (p0→p1), check if it crosses the ray.
    int winding = 0;
    float min_dist_sq = 1e10;

    for (int i = 0; i < num; i++)
    {
        float2 p0 = segments[i].xy;
        float2 p1 = segments[i].zw;

        // Winding number contribution
        if (p0.y <= P.y)
        {
            if (p1.y > P.y)
            {
                float cross_val = (p1.x - p0.x) * (P.y - p0.y) - (P.x - p0.x) * (p1.y - p0.y);
                if (cross_val > 0.0) winding++;
            }
        }
        else
        {
            if (p1.y <= P.y)
            {
                float cross_val = (p1.x - p0.x) * (P.y - p0.y) - (P.x - p0.x) * (p1.y - p0.y);
                if (cross_val < 0.0) winding--;
            }
        }

        // Minimum distance to segment (for anti-aliasing)
        float2 d = p1 - p0;
        float len_sq = dot(d, d);
        float t = (len_sq > 1e-8) ? saturate(dot(P - p0, d) / len_sq) : 0.0;
        float2 proj = p0 + d * t;
        float2 diff = P - proj;
        float dist_sq = dot(diff, diff);
        min_dist_sq = min(min_dist_sq, dist_sq);
    }

    // --- Anti-aliased output ---
    float min_dist = sqrt(min_dist_sq);

    if (winding == 0)
    {
        // Outside: blend using distance to nearest edge for AA
        if (aa > 0.0 && min_dist < aa)
        {
            float alpha = 1.0 - min_dist / aa;
            return float4(strokeColor.rgb, strokeColor.a * alpha);
        }
        discard;
        return float4(0, 0, 0, 0);
    }
    else
    {
        // Inside: always full alpha (no edge reduction)
        return strokeColor;
    }
}
