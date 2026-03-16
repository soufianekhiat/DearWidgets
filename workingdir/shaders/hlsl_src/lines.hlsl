// Vertex shader constant buffer at register b0
cbuffer vertexBuffer : register(b0)
{
	float4x4 ProjMtx;
};

// Pixel shader constant buffer at register b1
// Layout: 7 registers (112 bytes), must match ImWidgetsDashedLineBuffer in dear_widgets.h
cbuffer PS_CONSTANT_BUFFER : register(b1)
{
    float2 p0;            // reg0: screen-space start
    float2 p1;            //       screen-space end
    float  thickness;     // reg1: stroke width in pixels
    float  aa;            //       AA fringe in pixels
    float2 dash;          //       x=dash length, y=gap length
    float  dash_offset;   // reg2: offset in pixels
    float  cap;           //       ImWidgetsCap_
    float  join_type;     //       ImWidgetsJoin_
    float  miter_limit;   //       miter limit ratio
    float2 rect_min;      // reg3: quad min in screen space
    float2 rect_max;      //       quad max in screen space
    float4 color;         // reg4: RGBA
    float2 prev_dir;      // reg5: tangent of previous segment (0,0 = cap)
    float2 next_dir;      //       tangent of next segment (0,0 = cap)
    float  seg_start;     // reg6: cumulative arc-length at p0
    float  seg_end;       //       cumulative arc-length at p1
    float  total_length;  //       total polyline length
    float  debug_joins;   //       1.0 = debug join visualization
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

// ImWidgetsCap: 0=None, 1=Butt, 2=Square, 3=Round, 4=TriangleOut, 5=TriangleIn
float cap_dist(int ctype, float dx, float dy, float t)
{
    dx = abs(dx);
    dy = abs(dy);
    if (ctype == 0) return 1e10;
    if (ctype == 1) return max(dx + t, dy);
    if (ctype == 2) return max(dx, dy);
    if (ctype == 3) return sqrt(dx * dx + dy * dy);
    if (ctype == 4) return max(dy, (t + dx - dy));
    if (ctype == 5) return (dx + dy);
    return 1e10;
}

// Compute SDF distance in a join region.
// Both segments at a join compute the same value (symmetric SDF), so
// the bisector split produces no seam with transparent colours.
//
// P_world:   pixel position in screen space
// vertex:    join vertex position in screen space
// cur_dir:   tangent of current segment (ex)
// adj_dir:   tangent of adjacent segment (prev_dir or next_dir)
// dy:        perpendicular distance to current segment axis
// jtype:     0=Round, 1=Miter, 2=Bevel
// halfw:     half stroke width
// mlimit:    miter limit ratio
float join_dist(float2 P_world, float2 vertex,
                float2 cur_dir, float2 adj_dir,
                float dy, int jtype, float halfw, float mlimit)
{
    // Round join: circle centered at vertex (already symmetric)
    if (jtype == 0)
        return length(P_world - vertex);

    // Distance to current segment axis
    float d_cur = abs(dy);
    // Distance to adjacent segment axis
    float2 adj_perp = float2(-adj_dir.y, adj_dir.x);
    float d_adj = abs(dot(P_world - vertex, adj_perp));

    // Miter diamond: intersection of both stroke half-planes.
    // max(d_cur, d_adj) is identical from either segment's perspective.
    float d = max(d_cur, d_adj);

    // Bisector perpendicular distance (shared by miter limit & bevel)
    float2 bisect = cur_dir + adj_dir;
    float bl2 = dot(bisect, bisect);
    if (bl2 > 0.001)
    {
        float2 bn = bisect * rsqrt(bl2);
        float2 bp = float2(-bn.y, bn.x);
        float miter_d = abs(dot(P_world - vertex, bp));

        if (jtype == 1) // Miter: allow extension, clip at limit
            d = max(d, miter_d - mlimit * halfw);
        else            // Bevel: clip at bisector line
            d = max(d, miter_d);
    }

    return d;
}

float4 main_ps(PS_INPUT input) : SV_Target
{
    float2 P = lerp(rect_min, rect_max, input.uv.xy);
    float2 ba_vec = p1 - p0;
    float seg_len = max(length(ba_vec), 1e-5);
    float2 ex = ba_vec / seg_len;
    float2 ey = float2(-ex.y, ex.x);
    float halfw = 0.5 * thickness;
    float t = halfw - aa;

    float lx = dot(P - p0, ex);
    float ly = dot(P - p0, ey);
    float dx = seg_start + lx;
    float dy = ly;

    bool has_prev = (dot(prev_dir, prev_dir) > 0.0001);
    bool has_next = (dot(next_dir, next_dir) > 0.0001);
    int cap_type = (int)cap;
    int jtype = (int)join_type;
    bool dbg = (debug_joins > 0.5);

    // --- Bisector clip: prevent overdraw between adjacent segments ---
    if (has_prev)
    {
        float2 bisect = prev_dir + ex;
        if (dot(bisect, bisect) > 0.001)
        {
            if (dot(P - p0, bisect) < 0.0)
                return float4(0, 0, 0, 0);
        }
    }
    if (has_next)
    {
        float2 bisect = ex + next_dir;
        if (dot(bisect, bisect) > 0.001)
        {
            if (dot(P - p1, bisect) > 0.0)
                return float4(0, 0, 0, 0);
        }
    }

    // --- Early discard for pixels far from the line ---
    float max_ext = halfw + aa;
    if (jtype == 1) max_ext = max(max_ext, miter_limit * halfw + aa);
    if (!has_prev && dx < -max_ext) return float4(0, 0, 0, 0);
    if (!has_next && dx > total_length + max_ext) return float4(0, 0, 0, 0);

    // --- Dash pattern ---
    float dash_len = dash.x;
    float gap_len  = dash.y;
    float period = max(1e-5, dash_len + gap_len);
    bool solid = (gap_len < 0.5);

    float d = 0.0;
    int zone = 0; // 0=body, 1=cap, 2=join_p0, 3=join_p1

    if (solid)
    {
        d = abs(dy);

        if (!has_prev && dx < 0.0)
        {
            d = cap_dist(cap_type, -dx, abs(dy), t);
            zone = 1;
        }
        else if (!has_next && dx > total_length)
        {
            d = cap_dist(cap_type, dx - total_length, abs(dy), t);
            zone = 1;
        }
        else if (has_prev && lx < 0.0)
        {
            d = join_dist(P, p0, ex, prev_dir, dy, jtype, halfw, miter_limit);
            zone = 2;
        }
        else if (has_next && lx > seg_len)
        {
            d = join_dist(P, p1, ex, next_dir, dy, jtype, halfw, miter_limit);
            zone = 3;
        }
    }
    else
    {
        float u = dx + dash_offset;
        float m = u - period * floor(u / period);
        bool in_dash = (m < dash_len);

        if (in_dash)
        {
            d = abs(dy);
            float to_start = m;
            float to_end = dash_len - m;
            float d_start = cap_dist(cap_type, to_start, abs(dy), t);
            float d_end = cap_dist(cap_type, to_end, abs(dy), t);
            if (cap_type == 5)
                d = max(d, min(d_start, d_end));
        }
        else
        {
            float to_prev_end = m - dash_len;
            float to_next_start = period - m;
            float d1 = cap_dist(cap_type, to_prev_end, abs(dy), t);
            float d2 = cap_dist(cap_type, to_next_start, abs(dy), t);
            d = min(d1, d2);
        }

        // Cap at polyline endpoints
        if (!has_prev && dx < 0.0)
            d = cap_dist(cap_type, -dx, abs(dy), t);
        else if (!has_next && dx > total_length)
            d = cap_dist(cap_type, dx - total_length, abs(dy), t);

        // Apply join shaping at interior joins so round/bevel/miter
        // are respected even with dashed lines.
        if (has_prev && lx < 0.0)
        {
            d = max(d, join_dist(P, p0, ex, prev_dir, dy, jtype, halfw, miter_limit));
            zone = 2;
        }
        else if (has_next && lx > seg_len)
        {
            d = max(d, join_dist(P, p1, ex, next_dir, dy, jtype, halfw, miter_limit));
            zone = 3;
        }
    }

    // --- Anti-aliasing ---
    d = d - t;
    if (d < 0.0)
    {
        if (dbg && zone >= 2)
            return float4(1, 0, 0, color.a);
        return float4(color.rgb, color.a);
    }
    else
    {
        d /= max(aa, 1e-5);
        float a = exp(-d * d) * color.a;
        if (dbg && zone >= 2)
            return float4(1, 0, 0, a);
        return float4(color.rgb, a);
    }
}
