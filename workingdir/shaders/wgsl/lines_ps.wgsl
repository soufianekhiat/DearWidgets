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
        return max(_S2, t_0 + _S1 - _S2);
    }
    if(ctype_0 == i32(5))
    {
        return _S1 + _S2;
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
fn main_ps( _S4 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    var P_0 : vec2<f32> = mix(PS_CONSTANT_BUFFER_0.rect_min_0, PS_CONSTANT_BUFFER_0.rect_max_0, _S4.uv_0.xy);
    var ba_vec_0 : vec2<f32> = PS_CONSTANT_BUFFER_0.p1_0 - PS_CONSTANT_BUFFER_0.p0_0;
    var seg_len_0 : f32 = max(length(ba_vec_0), 0.00000999999974738f);
    var ex_0 : vec2<f32> = ba_vec_0 / vec2<f32>(seg_len_0);
    var halfw_1 : f32 = 0.5f * PS_CONSTANT_BUFFER_0.thickness_0;
    var t_1 : f32 = halfw_1 - PS_CONSTANT_BUFFER_0.aa_0;
    var lx_0 : f32 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ex_0);
    var ly_0 : f32 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, vec2<f32>(- ex_0.y, ex_0.x));
    var dx_1 : f32 = PS_CONSTANT_BUFFER_0.seg_start_0 + lx_0;
    var has_prev_0 : bool = (dot(PS_CONSTANT_BUFFER_0.prev_dir_0, PS_CONSTANT_BUFFER_0.prev_dir_0)) > 0.00009999999747379f;
    var has_next_0 : bool = (dot(PS_CONSTANT_BUFFER_0.next_dir_0, PS_CONSTANT_BUFFER_0.next_dir_0)) > 0.00009999999747379f;
    var cap_type_0 : i32 = i32(PS_CONSTANT_BUFFER_0.cap_0);
    var jtype_1 : i32 = i32(PS_CONSTANT_BUFFER_0.join_type_0);
    var dbg_0 : bool = (PS_CONSTANT_BUFFER_0.debug_joins_0) > 0.5f;
    if(has_prev_0)
    {
        var bisect_1 : vec2<f32> = PS_CONSTANT_BUFFER_0.prev_dir_0 + ex_0;
        if((dot(bisect_1, bisect_1)) > 0.00100000004749745f)
        {
            if((dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, bisect_1)) < 0.0f)
            {
                var _S5 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                return _S5;
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
                var _S6 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
                return _S6;
            }
        }
    }
    var max_ext_0 : f32 = halfw_1 + PS_CONSTANT_BUFFER_0.aa_0;
    var max_ext_1 : f32;
    if(jtype_1 == i32(1))
    {
        max_ext_1 = max(max_ext_0, PS_CONSTANT_BUFFER_0.miter_limit_0 * halfw_1 + PS_CONSTANT_BUFFER_0.aa_0);
    }
    else
    {
        max_ext_1 = max_ext_0;
    }
    var _S7 : bool = !has_prev_0;
    var _S8 : bool;
    if(_S7)
    {
        _S8 = dx_1 < (- max_ext_1);
    }
    else
    {
        _S8 = false;
    }
    if(_S8)
    {
        var _S9 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
        return _S9;
    }
    var _S10 : bool = !has_next_0;
    if(_S10)
    {
        _S8 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0 + max_ext_1);
    }
    else
    {
        _S8 = false;
    }
    if(_S8)
    {
        var _S11 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
        return _S11;
    }
    var dash_len_0 : f32 = PS_CONSTANT_BUFFER_0.dash_0.x;
    var gap_len_0 : f32 = PS_CONSTANT_BUFFER_0.dash_0.y;
    var period_0 : f32 = max(0.00000999999974738f, dash_len_0 + gap_len_0);
    var d_2 : f32;
    var zone_0 : i32;
    if(gap_len_0 < 0.5f)
    {
        var _S12 : f32 = abs(ly_0);
        if(_S7)
        {
            _S8 = dx_1 < 0.0f;
        }
        else
        {
            _S8 = false;
        }
        if(_S8)
        {
            var _S13 : f32 = cap_dist_0(cap_type_0, - dx_1, _S12, t_1);
            zone_0 = i32(1);
            d_2 = _S13;
        }
        else
        {
            if(_S10)
            {
                _S8 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);
            }
            else
            {
                _S8 = false;
            }
            if(_S8)
            {
                var _S14 : f32 = cap_dist_0(cap_type_0, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, _S12, t_1);
                zone_0 = i32(1);
                d_2 = _S14;
            }
            else
            {
                if(has_prev_0)
                {
                    _S8 = lx_0 < 0.0f;
                }
                else
                {
                    _S8 = false;
                }
                if(_S8)
                {
                    var _S15 : f32 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);
                    zone_0 = i32(2);
                    d_2 = _S15;
                }
                else
                {
                    if(has_next_0)
                    {
                        _S8 = lx_0 > seg_len_0;
                    }
                    else
                    {
                        _S8 = false;
                    }
                    if(_S8)
                    {
                        var _S16 : f32 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);
                        zone_0 = i32(3);
                        d_2 = _S16;
                    }
                    else
                    {
                        zone_0 = i32(0);
                        d_2 = _S12;
                    }
                }
            }
        }
    }
    else
    {
        var u_0 : f32 = dx_1 + PS_CONSTANT_BUFFER_0.dash_offset_0;
        var m_0 : f32 = u_0 - period_0 * floor(u_0 / period_0);
        if(m_0 < dash_len_0)
        {
            var d_3 : f32 = abs(ly_0);
            var d_start_0 : f32 = cap_dist_0(cap_type_0, m_0, d_3, t_1);
            var d_end_0 : f32 = cap_dist_0(cap_type_0, dash_len_0 - m_0, d_3, t_1);
            if(cap_type_0 == i32(5))
            {
                d_2 = max(d_3, min(d_start_0, d_end_0));
            }
            else
            {
                d_2 = d_3;
            }
        }
        else
        {
            var _S17 : f32 = abs(ly_0);
            d_2 = min(cap_dist_0(cap_type_0, m_0 - dash_len_0, _S17, t_1), cap_dist_0(cap_type_0, period_0 - m_0, _S17, t_1));
        }
        if(_S7)
        {
            _S8 = dx_1 < 0.0f;
        }
        else
        {
            _S8 = false;
        }
        if(_S8)
        {
            d_2 = cap_dist_0(cap_type_0, - dx_1, abs(ly_0), t_1);
        }
        else
        {
            if(_S10)
            {
                _S8 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);
            }
            else
            {
                _S8 = false;
            }
            if(_S8)
            {
                d_2 = cap_dist_0(cap_type_0, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, abs(ly_0), t_1);
            }
            else
            {
            }
        }
        if(has_prev_0)
        {
            _S8 = lx_0 < 0.0f;
        }
        else
        {
            _S8 = false;
        }
        if(_S8)
        {
            var _S18 : f32 = min(d_2, join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0));
            zone_0 = i32(2);
            d_2 = _S18;
        }
        else
        {
            if(has_next_0)
            {
                _S8 = lx_0 > seg_len_0;
            }
            else
            {
                _S8 = false;
            }
            if(_S8)
            {
                var _S19 : f32 = min(d_2, join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0));
                zone_0 = i32(3);
                d_2 = _S19;
            }
            else
            {
                zone_0 = i32(0);
            }
        }
    }
    var d_4 : f32 = d_2 - t_1;
    if(d_4 < 0.0f)
    {
        if(dbg_0)
        {
            _S8 = zone_0 >= i32(2);
        }
        else
        {
            _S8 = false;
        }
        if(_S8)
        {
            var _S20 : pixelOutput_0 = pixelOutput_0( vec4<f32>(1.0f, 0.0f, 0.0f, PS_CONSTANT_BUFFER_0.color_0.w) );
            return _S20;
        }
        var _S21 : pixelOutput_0 = pixelOutput_0( vec4<f32>(PS_CONSTANT_BUFFER_0.color_0.xyz, PS_CONSTANT_BUFFER_0.color_0.w) );
        return _S21;
    }
    else
    {
        var d_5 : f32 = d_4 / max(PS_CONSTANT_BUFFER_0.aa_0, 0.00000999999974738f);
        var a_0 : f32 = exp(- d_5 * d_5) * PS_CONSTANT_BUFFER_0.color_0.w;
        if(dbg_0)
        {
            _S8 = zone_0 >= i32(2);
        }
        else
        {
            _S8 = false;
        }
        if(_S8)
        {
            var _S22 : pixelOutput_0 = pixelOutput_0( vec4<f32>(1.0f, 0.0f, 0.0f, a_0) );
            return _S22;
        }
        var _S23 : pixelOutput_0 = pixelOutput_0( vec4<f32>(PS_CONSTANT_BUFFER_0.color_0.xyz, a_0) );
        return _S23;
    }
}

