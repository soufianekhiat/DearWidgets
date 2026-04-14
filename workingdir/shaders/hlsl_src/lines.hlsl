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
    float  debug_joins;   //       bit flags: 1=debug joins, 2=first seg of closed polyline, 4=last seg of closed polyline
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
//
// Cap shapes (cap_dist returns the SDF "outside distance"; pixel is inside the
// stroke when (cap_dist - t) < 0).
//
// TriangleOut = arrow tip extending OUTWARD past the segment endpoint by halfw,
//   apex at (dx=t, dy=0). Inside region: dx + |dy| < t.
//
// TriangleIn  = V-notch carved INTO the stroke end (centerline missing, two
//   triangular wings extend outward to (t, ±t)). Inside region:
//   |dy| < t AND |dy| > dx.
//
// NOTE: these formulas were swapped relative to the enum names until recently.
// The matching CPU geometry in dear_widgets.cpp's draw_subpath emits a single
// outward apex for TriangleOut and a pentagon notch for TriangleIn — see
// switch (cap) at lines ~19044 and ~19075.
float cap_dist(int ctype, float dx, float dy, float t)
{
    dx = abs(dx);
    dy = abs(dy);
    if (ctype == 0) return 1e10;
    if (ctype == 1) return max(dx + t, dy);
    if (ctype == 2) return max(dx, dy);
    if (ctype == 3) return sqrt(dx * dx + dy * dy);
    if (ctype == 4) return (dx + dy);                     // TriangleOut: arrow tip
    if (ctype == 5) return max(dy, (t + dx - dy));        // TriangleIn:  V-notch
    return 1e10;
}

// Compute SDF distance in a join region.
// Both segments at a join compute the same value (symmetric SDF), so
// the bisector split produces no seam with transparent colours.
//
// INVARIANT: this function must only be invoked from the JOIN ZONES — i.e.
// when lx < 0 (pixel before segment start) or lx > seg_len (pixel past segment
// end). The bevel clip `max(d, miter_d)` unconditionally adds the bisector
// clip, which is correct in the join area but would wrongly narrow the body.
// The reference (solid-lines-2D.frag:85-87) guards its bevel with
// `(dx < segment.x || dx > segment.y)`; we rely on the CALLER calling this
// only from the join zones (see main_ps body-vs-join branching).
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
    // seg_len = seg_end - seg_start (pre-computed C++-side and carried in the
    // cbuffer) instead of recomputing length(p1-p0) per pixel.
    float seg_len = max(seg_end - seg_start, 1e-5);
    float inv_len = 1.0 / seg_len;
    float2 ex = ba_vec * inv_len;
    float2 ey = float2(-ex.y, ex.x);

    // Sub-pixel thickness: clamp to 1 px and modulate alpha (Rougier 2013).
    // Without this, halfw - aa goes negative for thin lines and the gaussian
    // fringe dims the body. Reference: solid-lines-2D.vert:136-139,
    // dash-lines-2D.vert:148-151 — which CLAMPS alpha to thickness via
    //     v_color.a = min(v_linewidth, v_color.a);
    //     v_linewidth = max(v_linewidth, 1.0);
    // i.e. for a translucent colour, alpha only drops if thickness < alpha.
    // The earlier `alpha *= saturate(thickness)` over-dimmed translucent
    // sub-pixel lines (0.5 alpha × 0.3 thickness = 0.15, reference = 0.3).
    float effective_thickness = max(thickness, 1.0);
    float4 effective_color    = color;
    if (thickness < 1.0)
        effective_color.a = min(effective_color.a, thickness);

    float halfw = 0.5 * effective_thickness;
    float t = halfw - aa;

    float lx = dot(P - p0, ex);
    float ly = dot(P - p0, ey);
    float dx = seg_start + lx;
    float dy = ly;

    bool has_prev = (dot(prev_dir, prev_dir) > 0.0001);
    bool has_next = (dot(next_dir, next_dir) > 0.0001);
    int cap_type = (int)cap;
    int jtype = (int)join_type;
    // Decode bit flags: 1=debug joins, 2=first segment of closed polyline,
    // 4=last segment of closed polyline (replaces the old seg_start<0.001
    // heuristic which could misfire on degenerate polylines whose first
    // segment has near-zero length).
    int flags = (int)debug_joins;
    bool dbg              = (flags & 1) != 0;
    bool is_first_of_loop = (flags & 2) != 0;
    bool is_last_of_loop  = (flags & 4) != 0;

    // --- Bisector clip: prevent overdraw between adjacent segments ---
    // Use <= on the prev side and > on the next side so a pixel exactly on
    // the bisector line is kept by exactly one neighbour (the "next" one).
    // Without the asymmetry, bisector-aligned pixels would be drawn by both
    // segments and double-alpha-blended (visible for translucent colours).
    if (has_prev)
    {
        float2 bisect = prev_dir + ex;
        if (dot(bisect, bisect) > 0.001)
        {
            if (dot(P - p0, bisect) <= 0.0)
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
        // For the closing vertex of closed polylines, wrap dx so the dash
        // pattern is continuous around the loop. The is_first_of_loop and
        // is_last_of_loop flags are set by the CPU emitter, replacing the old
        // seg_start<0.001 heuristic.
        float dx_dash = dx;
        if (is_first_of_loop && has_prev && lx < 0.0)
            dx_dash = total_length + dx;   // wrap negative → end of polyline
        if (is_last_of_loop && has_next && lx > seg_len)
            dx_dash = dx - total_length;   // wrap past-end → start of polyline

        float u = dx_dash + dash_offset;
        float m = u - period * floor(u / period);
        // Half-open dash interval [0, dash_len). m == dash_len counts as gap.
        // Matches CPU DW_BuildOnIntervals which advances state on
        // pos >= dashes[idx], so both paths agree at the boundary.
        bool in_dash = (m < dash_len);

        // Reference dash-lines-2D.frag:251-266: at "discontinuous" joins
        // (turn > 15°), discard pixels whose current dash is entirely outside
        // THIS segment's arc-length range. At sharp turns the two adjacent
        // segments' join quads overlap and each segment would otherwise render
        // a "phantom dash" that belongs to the other segment.
        //
        // GOTCHA: skip these discards when the closed-polyline wrap is active
        // for the pixel. In the wrap case (is_first_of_loop at lx<0 or
        // is_last_of_loop at lx>seg_len), dx_dash was shifted by ±total_length
        // to get a continuous phase across the seam — dash_arc_start would
        // then legitimately fall outside [seg_start, seg_end] in polyline
        // coords, but the dash IS relevant to this segment's seam join.
        {
            const float THETA = 0.2617994;                   // 15° in radians
            bool wrap_active = (is_first_of_loop && lx < 0.0) ||
                               (is_last_of_loop  && lx > seg_len);
            if (!wrap_active)
            {
                float dash_arc_start = dx_dash - m;
                float dash_arc_end   = dash_arc_start + dash_len;
                if (has_next)
                {
                    float cross_en = ex.x * next_dir.y - ex.y * next_dir.x;
                    float dot_en   = dot(ex, next_dir);
                    float angle_n  = atan2(cross_en, dot_en);
                    if (abs(angle_n) > THETA && dash_arc_start > seg_end)
                        return float4(0, 0, 0, 0);
                }
                if (has_prev)
                {
                    float cross_pe = prev_dir.x * ex.y - prev_dir.y * ex.x;
                    float dot_pe   = dot(prev_dir, ex);
                    float angle_pp = atan2(cross_pe, dot_pe);
                    if (abs(angle_pp) > THETA && dash_arc_end < seg_start)
                        return float4(0, 0, 0, 0);
                }
            }
        }

        if (in_dash)
        {
            // Dash body: rectangle of width 2*halfw, no per-dash cap shapes.
            // (The previous code applied a buggy cap_type==5 special case here
            // that carved out the centerline of long dashes; the reference uses
            // a dash atlas to do this properly. Caps still apply at polyline
            // endpoints, just not at every dash boundary.)
            d = abs(dy);
        }
        else
        {
            float to_prev_end = m - dash_len;
            float to_next_start = period - m;
            float d1 = cap_dist(cap_type, to_prev_end, abs(dy), t);
            float d2 = cap_dist(cap_type, to_next_start, abs(dy), t);
            d = min(d1, d2);
        }

        // Cap at polyline endpoints (open polylines only)
        if (!has_prev && dx < 0.0)
            d = cap_dist(cap_type, -dx, abs(dy), t);
        else if (!has_next && dx > total_length)
            d = cap_dist(cap_type, dx - total_length, abs(dy), t);

        // Join zone: when a dash covers the vertex, fill with the join
        // shape directly (like the solid path) so the two segments produce
        // a seamless join.  When the vertex falls in a gap, only clip
        // nearby dash caps to the join shape via max().
        //
        // Without the direct assignment, the per-pixel arc-length
        // projection differs between the two segments at acute angles,
        // causing premature dash caps on the diagonally-clipped side.
        if (has_prev && lx < 0.0)
        {
            float jd = join_dist(P, p0, ex, prev_dir, dy, jtype, halfw, miter_limit);
            float cross_pe = prev_dir.x * ex.y - prev_dir.y * ex.x;
            float dot_pe   = dot(prev_dir, ex);
            float angle_p  = atan2(cross_pe, dot_pe);

            // Dash state at the vertex (seg_start); wrap for closing vertex of
            // closed polylines (the first segment's p0 is also the polyline end).
            float v_al = is_first_of_loop ? total_length : seg_start;
            float v_u  = v_al + dash_offset;
            float v_m  = v_u - period * floor(v_u / period);
            bool vertex_in_dash_p = (v_m < dash_len);
            d = vertex_in_dash_p ? jd : max(d, jd);

            // AA and cap-transformation only apply for MODERATE turns. At sharp
            // turns the rotated formulas from the reference over-clip legitimate
            // stroke geometry (the phantom-dash discard above handles the real
            // sharp-turn issue: dashes entirely outside the segment's range).
            if (abs(angle_p) < 1.5707963)                        // < PI/2 = 90°
            {
                if (vertex_in_dash_p)
                {
                    // Reference dash-lines-2D.frag:352-356: rotated-bisector AA.
                    float a = angle_p + 1.5707963;               // + PI/2
                    float f = abs(-lx * cos(a) - dy * sin(a));
                    d       = max(f, d);
                }
                else if (cap_type == 2 || cap_type == 4 || cap_type == 5)
                {
                    // Reference cap transformation (dash-lines-2D.frag:301-313).
                    float a  = angle_p * 0.5;                    // half-angle
                    float xr = -lx * cos(a) - dy * sin(a);
                    if (xr > 0.0)
                        return float4(0, 0, 0, 0);
                }
            }
            zone = 2;
        }
        else if (has_next && lx > seg_len)
        {
            float jd = join_dist(P, p1, ex, next_dir, dy, jtype, halfw, miter_limit);
            float cross_en = ex.x * next_dir.y - ex.y * next_dir.x;
            float dot_en   = dot(ex, next_dir);
            float angle_n  = atan2(cross_en, dot_en);

            float v_u = seg_end + dash_offset;
            float v_m = v_u - period * floor(v_u / period);
            bool vertex_in_dash_n = (v_m < dash_len);
            d = vertex_in_dash_n ? jd : max(d, jd);

            if (abs(angle_n) < 1.5707963)
            {
                if (vertex_in_dash_n)
                {
                    float a = angle_n + 1.5707963;
                    float f = abs((lx - seg_len) * cos(a) - dy * sin(a));
                    d       = max(f, d);
                }
                else if (cap_type == 2 || cap_type == 4 || cap_type == 5)
                {
                    float a  = angle_n * 0.5;
                    float xr = (lx - seg_len) * cos(a) - dy * sin(a);
                    if (xr > 0.0)
                        return float4(0, 0, 0, 0);
                }
            }
            zone = 3;
        }
    }

    // --- Anti-aliasing ---
    d = d - t;
    if (d < 0.0)
    {
        if (dbg && zone >= 2)
        {
            // Debug: Green=Round(0), Red=Miter(1), Blue=Bevel(2)
            float3 dc = (jtype == 0) ? float3(0,1,0) : (jtype == 2) ? float3(0,0,1) : float3(1,0,0);
            return float4(dc, effective_color.a);
        }
        return float4(effective_color.rgb, effective_color.a);
    }
    else
    {
        d /= max(aa, 1e-5);
        float a = exp(-d * d) * effective_color.a;
        if (dbg && zone >= 2)
        {
            float3 dc = (jtype == 0) ? float3(0,1,0) : (jtype == 2) ? float3(0,0,1) : float3(1,0,0);
            return float4(dc, a);
        }
        return float4(effective_color.rgb, a);
    }
}
