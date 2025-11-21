cbuffer PS_CONSTANT_BUFFER : register(b0)
{
    float2 p0;            // screen-space start
    float2 p1;            // screen-space end
    float  thickness;     // stroke width in pixels
    float  aa;            // AA fringe in pixels
    float2 dash;          // x=dash length, y=gap length
    float  dash_offset;   // offset in pixels
    float  cap;           // 0=butt, 1=square, 2=round
    float  join;          // reserved
    float  miter_limit;   // reserved
    float  pad0;          // padding
    float2 rect_min;      // quad min in screen space
    float2 rect_max;      // quad max in screen space
    float4 color;         // RGBA
};

// Vertex shader constant buffer at register b0
// DirectX: ImGui backend automatically provides this
// OpenGL: Converted to "uniform mat4 ProjMtx" in GLSL (manual edit required)
cbuffer vertexBuffer : register(b0)
{
	float4x4 ProjMtx;
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

// Distance to oriented rectangle centered at c, with half extents b, local basis ex (unit), ey (unit)
float sdOrientedBox(float2 p, float2 c, float2 ex, float2 ey, float2 b)
{
    float2 rel = p - c;
    float2 q = float2(dot(rel, ex), dot(rel, ey));
    float2 d = abs(q) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float sdCapsule(float2 p, float2 a, float2 b, float r)
{
    float2 pa = p - a, ba = b - a;
    float h = saturate(dot(pa, ba) / dot(ba, ba));
    return length(pa - ba * h) - r;
}

float2 perp(float2 v) { return float2(-v.y, v.x); }

float main_dash_mask(float s, float dash_len, float gap_len)
{
    float period = max(1e-5, dash_len + gap_len);
    float m = frac(s / period) * period;
    return (m <= dash_len) ? 1.0f : 0.0f;
}

// Signed distance for an isosceles triangle oriented along +x with base centered on x=0
// q.x = half base width, q.y = height (apex at (q.y, 0), base from (-q.x,0) to (q.x,0))
float sdTriangleIsoscelesX(float2 p, float2 q)
{
    // Map to canonical triangle with apex along +x: swap axes from the usual +y formulation
    // We want apex on +x, base on x=0. Use the standard +y function with swapped coords.
    // Standard isosceles SDF (apex along +y):
    float2 ps = float2(p.y, p.x);
    float2 qs = float2(q.x, q.y);

    ps.x = abs(ps.x);
    float2 a = ps - qs * saturate(dot(ps, qs) / dot(qs, qs));
    float2 b = ps - qs * float2(saturate(ps.x / qs.x), 1.0);
    float s = -sign(qs.y);
    float d2 = min(dot(a, a), dot(b, b));
    float xsgn = s * (ps.x * qs.y - ps.y * qs.x);
    return sqrt(d2) * sign(xsgn);
}

float4 main_ps(PS_INPUT input) : SV_Target
{
    // Reconstruct pixel position in screen space from UV + rect
    float2 P = lerp(rect_min, rect_max, input.uv.xy);
    float2 ba = p1 - p0;
    float len = max(length(ba), 1e-5);
    float2 ex = ba / len;
    float2 ey = perp(ex);
    float halfw = 0.5 * thickness;

    // Signed distance to stroke shape (caps handled)
    float d;
    // ImWidgetsCap: 0=None,1=Butt,2=Square,3=Round,4=TriangleOut,5=TriangleIn
    if (cap < 1.5) // None/Butt -> butt
    {
        float2 c = 0.5 * (p0 + p1);
        float2 b = float2(0.5 * len, halfw);
        d = sdOrientedBox(P, c, ex, ey, b);
    }
    else if (cap < 2.5) // square
    {
        float2 c = 0.5 * (p0 + p1);
        float2 b = float2(0.5 * len + halfw, halfw);
        d = sdOrientedBox(P, c, ex, ey, b);
    }
    else if (cap < 3.5) // round
    {
        d = sdCapsule(P, p0, p1, halfw);
    }
    else if (cap < 4.5) // TriangleOut -> union of butt rect and outward isosceles at both ends
    {
        float2 c = 0.5 * (p0 + p1);
        float2 b = float2(0.5 * len, halfw);
        float d_rect = sdOrientedBox(P, c, ex, ey, b);

        // Triangle at start (points outward, along -ex)
        float2 local0 = float2(dot(P - p0, -ex), dot(P - p0, ey)); // height along +x in local
        float d_tri0 = sdTriangleIsoscelesX(local0, float2(halfw, halfw));

        // Triangle at end (points outward, along +ex)
        float2 local1 = float2(dot(P - p1, ex), dot(P - p1, ey));
        float d_tri1 = sdTriangleIsoscelesX(local1, float2(halfw, halfw));

        // Union: min of distances
        d = min(d_rect, min(d_tri0, d_tri1));
    }
    else // TriangleIn -> subtract inward isosceles at both ends from butt rect
    {
        float2 c = 0.5 * (p0 + p1);
        float2 b = float2(0.5 * len, halfw);
        float d_rect = sdOrientedBox(P, c, ex, ey, b);

        // Inward triangles with height = halfw inside segment
        float2 local0 = float2(dot(P - p0, ex), dot(P - p0, ey));   // +x toward inside of segment
        float2 local1 = float2(dot(P - p1, -ex), dot(P - p1, ey)); // +x toward inside from end
        float d_tri0 = sdTriangleIsoscelesX(local0, float2(halfw, halfw));
        float d_tri1 = sdTriangleIsoscelesX(local1, float2(halfw, halfw));

        // Subtract triangles: A \ B = max(A, -B)
        d = max(d_rect, -min(d_tri0, d_tri1));
    }

    // Anti-aliased edge
    float alpha_edge = saturate(0.5 - d / max(aa, 1e-5));

    // Dash mask: project onto axis along segment (unclamped)
    float s = dot(P - p0, ex) + dash_offset;
    float m = main_dash_mask(s, dash.x, dash.y);

    float vis = alpha_edge * m;
    float4 out_col = color;
    out_col.rgb *= vis;   // also modulate RGB so dashes are obvious even if blend state is atypical
    out_col.a   *= vis;
    return out_col;
}
