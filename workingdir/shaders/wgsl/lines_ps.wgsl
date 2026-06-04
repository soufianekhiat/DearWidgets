struct SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    @align(16) p0_0 : vec2<f32>,
    @align(8) p1_0 : vec2<f32>,
    @align(16) thickness_0 : f32,
    @align(4) aa_0 : f32,
    @align(8) dash_0 : vec2<f32>,
    @align(16) dash_offset_0 : f32,
    @align(4) cap_0 : f32,
    @align(8) join_type_0 : f32,
    @align(4) miter_limit_0 : f32,
    @align(16) rect_min_0 : vec2<f32>,
    @align(8) rect_max_0 : vec2<f32>,
    @align(16) color_0 : vec4<f32>,
    @align(16) prev_dir_0 : vec2<f32>,
    @align(8) next_dir_0 : vec2<f32>,
    @align(16) seg_start_0 : f32,
    @align(4) seg_end_0 : f32,
    @align(8) total_length_0 : f32,
    @align(4) debug_joins_0 : f32,
};

@binding(1) @group(0) var<uniform> PS_CONSTANT_BUFFER_0 : SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0;
fn cap_dist_0( ctype_0 : i32,  dx_0 : f32,  dy_0 : f32,  t_0 : f32) -> f32
{
    var _S1 : f32 = abs(dx_0);
    var _S2 : f32 = abs(dy_0);
    if(ctype_0 == i32(0))
    {
        return 1.0e+10f;
    }
    if(ctype_0 == i32(1))
    {
        return max(_S1 + t_0, _S2);
    }
    if(ctype_0 == i32(2))
    {
        return max(_S1, _S2);
    }
    if(ctype_0 == i32(3))
    {
        return sqrt(_S1 * _S1 + _S2 * _S2);
    }
    if(ctype_0 == i32(4))
    {
        return _S1 + _S2;
    }
    if(ctype_0 == i32(5))
    {
        return max(_S2, t_0 + _S1 - _S2);
    }
    return 1.0e+10f;
}

fn rsqrt_0( x_0 : f32) -> f32
{
    return 1.0f / sqrt(x_0);
}

fn join_dist_0( P_world_0 : vec2<f32>,  vertex_0 : vec2<f32>,  cur_dir_0 : vec2<f32>,  adj_dir_0 : vec2<f32>,  dy_1 : f32,  jtype_0 : i32,  halfw_0 : f32,  mlimit_0 : f32) -> f32
{
    if(jtype_0 == i32(0))
    {
        return length(P_world_0 - vertex_0);
    }
    var _S3 : vec2<f32> = P_world_0 - vertex_0;
    var d_0 : f32 = max(abs(dy_1), abs(dot(_S3, vec2<f32>(- adj_dir_0.y, adj_dir_0.x))));
    var bisect_0 : vec2<f32> = cur_dir_0 + adj_dir_0;
    var bl2_0 : f32 = dot(bisect_0, bisect_0);
    var d_1 : f32;
    if(bl2_0 > 0.00100000004749745f)
    {
        var bn_0 : vec2<f32> = bisect_0 * vec2<f32>(rsqrt_0(bl2_0));
        var miter_d_0 : f32 = abs(dot(_S3, vec2<f32>(- bn_0.y, bn_0.x)));
        if(jtype_0 == i32(1))
        {
            d_1 = max(d_0, miter_d_0 - mlimit_0 * halfw_0);
        }
        else
        {
            d_1 = max(d_0, miter_d_0);
        }
    }
    else
    {
        d_1 = d_0;
    }
    return d_1;
}

fn eval_dash_sdf_0( u_rel_0 : f32,  dash_len_0 : f32,  cap_type_0 : i32,  abs_dy_0 : f32,  halfw_1 : f32,  t_1 : f32) -> f32
{
    var d_2 : f32;
    if(cap_type_0 == i32(5))
    {
        if(u_rel_0 < halfw_1)
        {
            d_2 = max(abs_dy_0, cap_dist_0(i32(5), halfw_1 - u_rel_0, abs_dy_0, t_1));
        }
        else
        {
            d_2 = abs_dy_0;
        }
        var _S4 : f32 = dash_len_0 - halfw_1;
        if(u_rel_0 > _S4)
        {
            d_2 = max(d_2, cap_dist_0(i32(5), u_rel_0 - _S4, abs_dy_0, t_1));
        }
        else
        {
        }
        return d_2;
    }
    var _S5 : bool;
    if(cap_type_0 == i32(0))
    {
        _S5 = true;
    }
    else
    {
        _S5 = cap_type_0 == i32(1);
    }
    if(_S5)
    {
        d_2 = 0.0f;
    }
    else
    {
        d_2 = halfw_1;
    }
    if(u_rel_0 < d_2)
    {
        return cap_dist_0(cap_type_0, d_2 - u_rel_0, abs_dy_0, t_1);
    }
    var _S6 : f32 = dash_len_0 - d_2;
    if(u_rel_0 > _S6)
    {
        return cap_dist_0(cap_type_0, u_rel_0 - _S6, abs_dy_0, t_1);
    }
    return abs_dy_0;
}

struct pixelOutput_0
{
    @location(0) output_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) col_0 : vec4<f32>,
    @location(1) uv_0 : vec2<f32>,
};

@fragment
fn main_ps( _S7 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    var P_0 : vec2<f32> = mix(PS_CONSTANT_BUFFER_0.rect_min_0, PS_CONSTANT_BUFFER_0.rect_max_0, _S7.uv_0.xy);
    var seg_len_0 : f32 = max(PS_CONSTANT_BUFFER_0.seg_end_0 - PS_CONSTANT_BUFFER_0.seg_start_0, 0.00000999999974738f);
    var ex_0 : vec2<f32> = (PS_CONSTANT_BUFFER_0.p1_0 - PS_CONSTANT_BUFFER_0.p0_0) * vec2<f32>((1.0f / seg_len_0));
    var _S8 : f32 = ex_0.y;
    var _S9 : f32 = ex_0.x;
    var ey_0 : vec2<f32> = vec2<f32>(- _S8, _S9);
    var effective_thickness_0 : f32 = max(PS_CONSTANT_BUFFER_0.thickness_0, 1.0f);
    var effective_color_0 : vec4<f32> = PS_CONSTANT_BUFFER_0.color_0;
    if((PS_CONSTANT_BUFFER_0.thickness_0) < 1.0f)
    {
        effective_color_0[i32(3)] = min(effective_color_0.w, PS_CONSTANT_BUFFER_0.thickness_0);
    }
    var halfw_2 : f32 = 0.5f * effective_thickness_0;
    var t_2 : f32 = halfw_2 - PS_CONSTANT_BUFFER_0.aa_0;
    var lx_0 : f32 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ex_0);
    var ly_0 : f32 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ey_0);
    var dx_1 : f32 = PS_CONSTANT_BUFFER_0.seg_start_0 + lx_0;
    var has_prev_0 : bool = (dot(PS_CONSTANT_BUFFER_0.prev_dir_0, PS_CONSTANT_BUFFER_0.prev_dir_0)) > 0.00009999999747379f;
    var has_next_0 : bool = (dot(PS_CONSTANT_BUFFER_0.next_dir_0, PS_CONSTANT_BUFFER_0.next_dir_0)) > 0.00009999999747379f;
    var cap_type_1 : i32 = i32(PS_CONSTANT_BUFFER_0.cap_0);
    var jtype_1 : i32 = i32(PS_CONSTANT_BUFFER_0.join_type_0);
    var flags_0 : i32 = i32(PS_CONSTANT_BUFFER_0.debug_joins_0);
    var dbg_0 : bool = ((flags_0 & (i32(1)))) != i32(0);
    var is_first_of_loop_0 : bool = ((flags_0 & (i32(2)))) != i32(0);
    var is_last_of_loop_0 : bool = ((flags_0 & (i32(4)))) != i32(0);
    if(has_prev_0)
    {
        var bisect_1 : vec2<f32> = PS_CONSTANT_BUFFER_0.prev_dir_0 + ex_0;
        if((dot(bisect_1, bisect_1)) > 0.00100000004749745f)
        {
            if((dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, bisect_1)) <= 0.0f)
            {
                var _S10 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                return _S10;
            }
        }
    }
    if(has_next_0)
    {
        var bisect_2 : vec2<f32> = ex_0 + PS_CONSTANT_BUFFER_0.next_dir_0;
        if((dot(bisect_2, bisect_2)) > 0.00100000004749745f)
        {
            if((dot(P_0 - PS_CONSTANT_BUFFER_0.p1_0, bisect_2)) > 0.0f)
            {
                var _S11 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                return _S11;
            }
        }
    }
    var max_ext_0 : f32 = halfw_2 + PS_CONSTANT_BUFFER_0.aa_0;
    var max_ext_1 : f32;
    if(jtype_1 == i32(1))
    {
        max_ext_1 = max(max_ext_0, PS_CONSTANT_BUFFER_0.miter_limit_0 * halfw_2 + PS_CONSTANT_BUFFER_0.aa_0);
    }
    else
    {
        max_ext_1 = max_ext_0;
    }
    var _S12 : bool = !has_prev_0;
    var wrap_active_0 : bool;
    if(_S12)
    {
        wrap_active_0 = dx_1 < (- max_ext_1);
    }
    else
    {
        wrap_active_0 = false;
    }
    if(wrap_active_0)
    {
        var _S13 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
        return _S13;
    }
    var _S14 : bool = !has_next_0;
    if(_S14)
    {
        wrap_active_0 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0 + max_ext_1);
    }
    else
    {
        wrap_active_0 = false;
    }
    if(wrap_active_0)
    {
        var _S15 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
        return _S15;
    }
    var dash_len_1 : f32 = PS_CONSTANT_BUFFER_0.dash_0.x;
    var gap_len_0 : f32 = PS_CONSTANT_BUFFER_0.dash_0.y;
    var period_0 : f32 = max(0.00000999999974738f, dash_len_1 + gap_len_0);
    var d_3 : f32;
    var zone_0 : i32;
    if(dash_len_1 > 1.0e+06f)
    {
        var _S16 : f32 = abs(ly_0);
        if(_S12)
        {
            wrap_active_0 = dx_1 < 0.0f;
        }
        else
        {
            wrap_active_0 = false;
        }
        if(wrap_active_0)
        {
            var _S17 : f32 = cap_dist_0(cap_type_1, - dx_1, _S16, t_2);
            zone_0 = i32(1);
            d_3 = _S17;
        }
        else
        {
            if(_S14)
            {
                wrap_active_0 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);
            }
            else
            {
                wrap_active_0 = false;
            }
            if(wrap_active_0)
            {
                var _S18 : f32 = cap_dist_0(cap_type_1, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, _S16, t_2);
                zone_0 = i32(1);
                d_3 = _S18;
            }
            else
            {
                if(has_prev_0)
                {
                    wrap_active_0 = lx_0 < 0.0f;
                }
                else
                {
                    wrap_active_0 = false;
                }
                if(wrap_active_0)
                {
                    var _S19 : f32 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_2, PS_CONSTANT_BUFFER_0.miter_limit_0);
                    zone_0 = i32(2);
                    d_3 = _S19;
                }
                else
                {
                    if(has_next_0)
                    {
                        wrap_active_0 = lx_0 > seg_len_0;
                    }
                    else
                    {
                        wrap_active_0 = false;
                    }
                    if(wrap_active_0)
                    {
                        var _S20 : f32 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_2, PS_CONSTANT_BUFFER_0.miter_limit_0);
                        zone_0 = i32(3);
                        d_3 = _S20;
                    }
                    else
                    {
                        zone_0 = i32(0);
                        d_3 = _S16;
                    }
                }
            }
        }
    }
    else
    {
        if(is_first_of_loop_0)
        {
            wrap_active_0 = has_prev_0;
        }
        else
        {
            wrap_active_0 = false;
        }
        if(wrap_active_0)
        {
            wrap_active_0 = lx_0 < 0.0f;
        }
        else
        {
            wrap_active_0 = false;
        }
        var dx_dash_0 : f32;
        if(wrap_active_0)
        {
            dx_dash_0 = PS_CONSTANT_BUFFER_0.total_length_0 + dx_1;
        }
        else
        {
            dx_dash_0 = dx_1;
        }
        if(is_last_of_loop_0)
        {
            wrap_active_0 = has_next_0;
        }
        else
        {
            wrap_active_0 = false;
        }
        if(wrap_active_0)
        {
            wrap_active_0 = lx_0 > seg_len_0;
        }
        else
        {
            wrap_active_0 = false;
        }
        if(wrap_active_0)
        {
            dx_dash_0 = dx_1 - PS_CONSTANT_BUFFER_0.total_length_0;
        }
        else
        {
        }
        var u_0 : f32 = dx_dash_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;
        var u_rel_1 : f32 = u_0 - floor(u_0 / period_0) * period_0;
        if(is_first_of_loop_0)
        {
            wrap_active_0 = lx_0 < 0.0f;
        }
        else
        {
            wrap_active_0 = false;
        }
        if(wrap_active_0)
        {
            wrap_active_0 = true;
        }
        else
        {
            if(is_last_of_loop_0)
            {
                wrap_active_0 = lx_0 > seg_len_0;
            }
            else
            {
                wrap_active_0 = false;
            }
        }
        if(!wrap_active_0)
        {
            wrap_active_0 = gap_len_0 > 0.0f;
        }
        else
        {
            wrap_active_0 = false;
        }
        if(wrap_active_0)
        {
            var dash_arc_start_0 : f32 = dx_dash_0 - u_rel_1;
            var dash_arc_end_0 : f32 = dash_arc_start_0 + dash_len_1;
            if(has_next_0)
            {
                if((abs(atan2(_S9 * PS_CONSTANT_BUFFER_0.next_dir_0.y - _S8 * PS_CONSTANT_BUFFER_0.next_dir_0.x, dot(ex_0, PS_CONSTANT_BUFFER_0.next_dir_0)))) > 0.2617993950843811f)
                {
                    wrap_active_0 = dash_arc_start_0 > (PS_CONSTANT_BUFFER_0.seg_end_0);
                }
                else
                {
                    wrap_active_0 = false;
                }
                if(wrap_active_0)
                {
                    var _S21 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                    return _S21;
                }
            }
            if(has_prev_0)
            {
                if((abs(atan2(PS_CONSTANT_BUFFER_0.prev_dir_0.x * _S8 - PS_CONSTANT_BUFFER_0.prev_dir_0.y * _S9, dot(PS_CONSTANT_BUFFER_0.prev_dir_0, ex_0)))) > 0.2617993950843811f)
                {
                    wrap_active_0 = dash_arc_end_0 < (PS_CONSTANT_BUFFER_0.seg_start_0);
                }
                else
                {
                    wrap_active_0 = false;
                }
                if(wrap_active_0)
                {
                    var _S22 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                    return _S22;
                }
            }
        }
        var a_dy_0 : f32 = abs(ly_0);
        var _S23 : f32 = min(eval_dash_sdf_0(u_rel_1, dash_len_1, cap_type_1, a_dy_0, halfw_2, t_2), min(eval_dash_sdf_0(u_rel_1 + period_0, dash_len_1, cap_type_1, a_dy_0, halfw_2, t_2), eval_dash_sdf_0(u_rel_1 - period_0, dash_len_1, cap_type_1, a_dy_0, halfw_2, t_2)));
        if(_S12)
        {
            wrap_active_0 = dx_1 < 0.0f;
        }
        else
        {
            wrap_active_0 = false;
        }
        if(wrap_active_0)
        {
            d_3 = cap_dist_0(cap_type_1, - dx_1, a_dy_0, t_2);
        }
        else
        {
            if(_S14)
            {
                wrap_active_0 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);
            }
            else
            {
                wrap_active_0 = false;
            }
            if(wrap_active_0)
            {
                d_3 = cap_dist_0(cap_type_1, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, a_dy_0, t_2);
            }
            else
            {
                d_3 = _S23;
            }
        }
        if(has_prev_0)
        {
            wrap_active_0 = lx_0 < 0.0f;
        }
        else
        {
            wrap_active_0 = false;
        }
        var vertex_in_dash_p_0 : bool;
        if(wrap_active_0)
        {
            var jd_0 : f32 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_2, PS_CONSTANT_BUFFER_0.miter_limit_0);
            var angle_p_0 : f32 = atan2(PS_CONSTANT_BUFFER_0.prev_dir_0.x * _S8 - PS_CONSTANT_BUFFER_0.prev_dir_0.y * _S9, dot(PS_CONSTANT_BUFFER_0.prev_dir_0, ex_0));
            var v_al_0 : f32;
            if(is_first_of_loop_0)
            {
                v_al_0 = PS_CONSTANT_BUFFER_0.total_length_0;
            }
            else
            {
                v_al_0 = PS_CONSTANT_BUFFER_0.seg_start_0;
            }
            var v_u_0 : f32 = v_al_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;
            if((v_u_0 - period_0 * floor(v_u_0 / period_0)) < dash_len_1)
            {
                vertex_in_dash_p_0 = true;
            }
            else
            {
                vertex_in_dash_p_0 = gap_len_0 <= 0.0f;
            }
            if(vertex_in_dash_p_0)
            {
                d_3 = jd_0;
            }
            else
            {
                d_3 = max(d_3, jd_0);
            }
            if((abs(angle_p_0)) < 1.57079625129699707f)
            {
                if(vertex_in_dash_p_0)
                {
                    var a_0 : f32 = angle_p_0 + 1.57079625129699707f;
                    d_3 = max(abs(- lx_0 * cos(a_0) - ly_0 * sin(a_0)), d_3);
                }
                else
                {
                    if(cap_type_1 == i32(2))
                    {
                        wrap_active_0 = true;
                    }
                    else
                    {
                        wrap_active_0 = cap_type_1 == i32(4);
                    }
                    if(wrap_active_0)
                    {
                        wrap_active_0 = true;
                    }
                    else
                    {
                        wrap_active_0 = cap_type_1 == i32(5);
                    }
                    if(wrap_active_0)
                    {
                        var a_1 : f32 = angle_p_0 * 0.5f;
                        if((- lx_0 * cos(a_1) - ly_0 * sin(a_1)) > 0.0f)
                        {
                            var _S24 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                            return _S24;
                        }
                    }
                }
            }
            else
            {
            }
            zone_0 = i32(2);
        }
        else
        {
            if(has_next_0)
            {
                wrap_active_0 = lx_0 > seg_len_0;
            }
            else
            {
                wrap_active_0 = false;
            }
            if(wrap_active_0)
            {
                var jd_1 : f32 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_2, PS_CONSTANT_BUFFER_0.miter_limit_0);
                var angle_n_0 : f32 = atan2(_S9 * PS_CONSTANT_BUFFER_0.next_dir_0.y - _S8 * PS_CONSTANT_BUFFER_0.next_dir_0.x, dot(ex_0, PS_CONSTANT_BUFFER_0.next_dir_0));
                var v_u_1 : f32 = PS_CONSTANT_BUFFER_0.seg_end_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;
                if((v_u_1 - period_0 * floor(v_u_1 / period_0)) < dash_len_1)
                {
                    vertex_in_dash_p_0 = true;
                }
                else
                {
                    vertex_in_dash_p_0 = gap_len_0 <= 0.0f;
                }
                if(vertex_in_dash_p_0)
                {
                    d_3 = jd_1;
                }
                else
                {
                    d_3 = max(d_3, jd_1);
                }
                if((abs(angle_n_0)) < 1.57079625129699707f)
                {
                    if(vertex_in_dash_p_0)
                    {
                        var a_2 : f32 = angle_n_0 + 1.57079625129699707f;
                        d_3 = max(abs((lx_0 - seg_len_0) * cos(a_2) - ly_0 * sin(a_2)), d_3);
                    }
                    else
                    {
                        if(cap_type_1 == i32(2))
                        {
                            wrap_active_0 = true;
                        }
                        else
                        {
                            wrap_active_0 = cap_type_1 == i32(4);
                        }
                        if(wrap_active_0)
                        {
                            wrap_active_0 = true;
                        }
                        else
                        {
                            wrap_active_0 = cap_type_1 == i32(5);
                        }
                        if(wrap_active_0)
                        {
                            var a_3 : f32 = angle_n_0 * 0.5f;
                            if(((lx_0 - seg_len_0) * cos(a_3) - ly_0 * sin(a_3)) > 0.0f)
                            {
                                var _S25 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                                return _S25;
                            }
                        }
                    }
                }
                else
                {
                }
                zone_0 = i32(3);
            }
            else
            {
                zone_0 = i32(0);
            }
        }
    }
    var d_4 : f32 = d_3 - t_2;
    var dc_0 : vec3<f32>;
    if(d_4 < 0.0f)
    {
        if(dbg_0)
        {
            wrap_active_0 = zone_0 >= i32(2);
        }
        else
        {
            wrap_active_0 = false;
        }
        if(wrap_active_0)
        {
            if(jtype_1 == i32(0))
            {
                dc_0 = vec3<f32>(0.0f, 1.0f, 0.0f);
            }
            else
            {
                if(jtype_1 == i32(2))
                {
                    dc_0 = vec3<f32>(0.0f, 0.0f, 1.0f);
                }
                else
                {
                    dc_0 = vec3<f32>(1.0f, 0.0f, 0.0f);
                }
            }
            var _S26 : pixelOutput_0 = pixelOutput_0( vec4<f32>(dc_0, effective_color_0.w) );
            return _S26;
        }
        var _S27 : pixelOutput_0 = pixelOutput_0( vec4<f32>(effective_color_0.xyz, effective_color_0.w) );
        return _S27;
    }
    else
    {
        var d_5 : f32 = d_4 / max(PS_CONSTANT_BUFFER_0.aa_0, 0.00000999999974738f);
        var a_4 : f32 = exp(- d_5 * d_5) * effective_color_0.w;
        if(dbg_0)
        {
            wrap_active_0 = zone_0 >= i32(2);
        }
        else
        {
            wrap_active_0 = false;
        }
        if(wrap_active_0)
        {
            if(jtype_1 == i32(0))
            {
                dc_0 = vec3<f32>(0.0f, 1.0f, 0.0f);
            }
            else
            {
                if(jtype_1 == i32(2))
                {
                    dc_0 = vec3<f32>(0.0f, 0.0f, 1.0f);
                }
                else
                {
                    dc_0 = vec3<f32>(1.0f, 0.0f, 0.0f);
                }
            }
            var _S28 : pixelOutput_0 = pixelOutput_0( vec4<f32>(dc_0, a_4) );
            return _S28;
        }
        var _S29 : pixelOutput_0 = pixelOutput_0( vec4<f32>(effective_color_0.xyz, a_4) );
        return _S29;
    }
}

