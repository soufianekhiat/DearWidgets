#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 28 0
struct SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec2 p0_0;
    vec2 p1_0;
    float thickness_0;
    float aa_0;
    vec2 dash_0;
    float dash_offset_0;
    float cap_0;
    float join_type_0;
    float miter_limit_0;
    vec2 rect_min_0;
    vec2 rect_max_0;
    vec4 color_0;
    vec2 prev_dir_0;
    vec2 next_dir_0;
    float seg_start_0;
    float seg_end_0;
    float total_length_0;
    float debug_joins_0;
};


#line 9
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec2 p0_0;
    vec2 p1_0;
    float thickness_0;
    float aa_0;
    vec2 dash_0;
    float dash_offset_0;
    float cap_0;
    float join_type_0;
    float miter_limit_0;
    vec2 rect_min_0;
    vec2 rect_max_0;
    vec4 color_0;
    vec2 prev_dir_0;
    vec2 next_dir_0;
    float seg_start_0;
    float seg_end_0;
    float total_length_0;
    float debug_joins_0;
}PS_CONSTANT_BUFFER_0;

#line 12386 1
float saturate_0(float x_0)
{

#line 12394
    return clamp(x_0, 0.0, 1.0);
}


#line 70 0
float cap_dist_0(int ctype_0, float dx_0, float dy_0, float t_0)
{
    float _S1 = abs(dx_0);
    float _S2 = abs(dy_0);
    if(ctype_0 == 0)
    {

#line 74
        return 1.0e+10;
    }

#line 75
    if(ctype_0 == 1)
    {

#line 75
        return max(_S1 + t_0, _S2);
    }

#line 76
    if(ctype_0 == 2)
    {

#line 76
        return max(_S1, _S2);
    }

#line 77
    if(ctype_0 == 3)
    {

#line 77
        return sqrt(_S1 * _S1 + _S2 * _S2);
    }

#line 78
    if(ctype_0 == 4)
    {

#line 78
        return _S1 + _S2;
    }

#line 79
    if(ctype_0 == 5)
    {

#line 79
        return max(_S2, t_0 + _S1 - _S2);
    }

#line 80
    return 1.0e+10;
}


#line 95
float join_dist_0(vec2 P_world_0, vec2 vertex_0, vec2 cur_dir_0, vec2 adj_dir_0, float dy_1, int jtype_0, float halfw_0, float mlimit_0)
{



    if(jtype_0 == 0)
    {

#line 101
        return length(P_world_0 - vertex_0);
    }

#line 107
    vec2 _S3 = P_world_0 - vertex_0;



    float d_0 = max(abs(dy_1), abs(dot(_S3, vec2(- adj_dir_0.y, adj_dir_0.x))));


    vec2 bisect_0 = cur_dir_0 + adj_dir_0;
    float bl2_0 = dot(bisect_0, bisect_0);

#line 115
    float d_1;
    if(bl2_0 > 0.00100000004749745)
    {
        vec2 bn_0 = bisect_0 * (inversesqrt((bl2_0)));

        float miter_d_0 = abs(dot(_S3, vec2(- bn_0.y, bn_0.x)));

        if(jtype_0 == 1)
        {

#line 122
            d_1 = max(d_0, miter_d_0 - mlimit_0 * halfw_0);

#line 122
        }
        else
        {

#line 122
            d_1 = max(d_0, miter_d_0);

#line 122
        }

#line 116
    }
    else
    {

#line 116
        d_1 = d_0;

#line 116
    }

#line 128
    return d_1;
}


#line 8639 1
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 8639
layout(location = 1)
in vec2 input_uv_0;


#line 131 0
void main()
{
    vec2 P_0 = mix(PS_CONSTANT_BUFFER_0.rect_min_0, PS_CONSTANT_BUFFER_0.rect_max_0, input_uv_0.xy);
    vec2 ba_vec_0 = PS_CONSTANT_BUFFER_0.p1_0 - PS_CONSTANT_BUFFER_0.p0_0;
    float seg_len_0 = max(length(ba_vec_0), 0.00000999999974738);
    vec2 ex_0 = ba_vec_0 / seg_len_0;
    vec2 ey_0 = vec2(- ex_0.y, ex_0.x);

#line 143
    float effective_thickness_0 = max(PS_CONSTANT_BUFFER_0.thickness_0, 1.0);
    vec4 effective_color_0 = PS_CONSTANT_BUFFER_0.color_0;
    effective_color_0[3] = effective_color_0[3] * saturate_0(PS_CONSTANT_BUFFER_0.thickness_0);

    float halfw_1 = 0.5 * effective_thickness_0;
    float t_1 = halfw_1 - PS_CONSTANT_BUFFER_0.aa_0;

    float lx_0 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ex_0);
    float ly_0 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ey_0);
    float dx_1 = PS_CONSTANT_BUFFER_0.seg_start_0 + lx_0;


    bool has_prev_0 = (dot(PS_CONSTANT_BUFFER_0.prev_dir_0, PS_CONSTANT_BUFFER_0.prev_dir_0)) > 0.00009999999747379;
    bool has_next_0 = (dot(PS_CONSTANT_BUFFER_0.next_dir_0, PS_CONSTANT_BUFFER_0.next_dir_0)) > 0.00009999999747379;
    int cap_type_0 = int(PS_CONSTANT_BUFFER_0.cap_0);
    int jtype_1 = int(PS_CONSTANT_BUFFER_0.join_type_0);

#line 163
    int flags_0 = int(PS_CONSTANT_BUFFER_0.debug_joins_0);
    bool dbg_0 = (flags_0 & 1) != 0;
    bool is_first_of_loop_0 = (flags_0 & 2) != 0;
    bool is_last_of_loop_0 = (flags_0 & 4) != 0;

#line 173
    if(has_prev_0)
    {
        vec2 bisect_1 = PS_CONSTANT_BUFFER_0.prev_dir_0 + ex_0;
        if((dot(bisect_1, bisect_1)) > 0.00100000004749745)
        {
            if((dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, bisect_1)) <= 0.0)
            {

#line 178
                entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 178
                return;
            }

#line 176
        }

#line 173
    }

#line 182
    if(has_next_0)
    {
        vec2 bisect_2 = ex_0 + PS_CONSTANT_BUFFER_0.next_dir_0;
        if((dot(bisect_2, bisect_2)) > 0.00100000004749745)
        {
            if((dot(P_0 - PS_CONSTANT_BUFFER_0.p1_0, bisect_2)) > 0.0)
            {

#line 187
                entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 187
                return;
            }

#line 185
        }

#line 182
    }

#line 193
    float max_ext_0 = halfw_1 + PS_CONSTANT_BUFFER_0.aa_0;

#line 193
    float max_ext_1;
    if(jtype_1 == 1)
    {

#line 194
        max_ext_1 = max(max_ext_0, PS_CONSTANT_BUFFER_0.miter_limit_0 * halfw_1 + PS_CONSTANT_BUFFER_0.aa_0);

#line 194
    }
    else
    {

#line 194
        max_ext_1 = max_ext_0;

#line 194
    }
    bool _S4 = !has_prev_0;

#line 195
    bool _S5;

#line 195
    if(_S4)
    {

#line 195
        _S5 = dx_1 < (- max_ext_1);

#line 195
    }
    else
    {

#line 195
        _S5 = false;

#line 195
    }

#line 195
    if(_S5)
    {

#line 195
        entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 195
        return;
    }

#line 196
    bool _S6 = !has_next_0;

#line 196
    if(_S6)
    {

#line 196
        _S5 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0 + max_ext_1);

#line 196
    }
    else
    {

#line 196
        _S5 = false;

#line 196
    }

#line 196
    if(_S5)
    {

#line 196
        entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 196
        return;
    }

    float dash_len_0 = PS_CONSTANT_BUFFER_0.dash_0.x;
    float gap_len_0 = PS_CONSTANT_BUFFER_0.dash_0.y;
    float period_0 = max(0.00000999999974738, dash_len_0 + gap_len_0);

#line 201
    float d_2;

#line 201
    int zone_0;

#line 207
    if(gap_len_0 < 0.5)
    {
        float _S7 = abs(ly_0);

        if(_S4)
        {

#line 211
            _S5 = dx_1 < 0.0;

#line 211
        }
        else
        {

#line 211
            _S5 = false;

#line 211
        }

#line 211
        if(_S5)
        {
            float _S8 = cap_dist_0(cap_type_0, - dx_1, _S7, t_1);

#line 213
            zone_0 = 1;

#line 213
            d_2 = _S8;

#line 211
        }
        else
        {


            if(_S6)
            {

#line 216
                _S5 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);

#line 216
            }
            else
            {

#line 216
                _S5 = false;

#line 216
            }

#line 216
            if(_S5)
            {
                float _S9 = cap_dist_0(cap_type_0, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, _S7, t_1);

#line 218
                zone_0 = 1;

#line 218
                d_2 = _S9;

#line 216
            }
            else
            {


                if(has_prev_0)
                {

#line 221
                    _S5 = lx_0 < 0.0;

#line 221
                }
                else
                {

#line 221
                    _S5 = false;

#line 221
                }

#line 221
                if(_S5)
                {
                    float _S10 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);

#line 223
                    zone_0 = 2;

#line 223
                    d_2 = _S10;

#line 221
                }
                else
                {


                    if(has_next_0)
                    {

#line 226
                        _S5 = lx_0 > seg_len_0;

#line 226
                    }
                    else
                    {

#line 226
                        _S5 = false;

#line 226
                    }

#line 226
                    if(_S5)
                    {
                        float _S11 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);

#line 228
                        zone_0 = 3;

#line 228
                        d_2 = _S11;

#line 226
                    }
                    else
                    {

#line 226
                        zone_0 = 0;

#line 226
                        d_2 = _S7;

#line 226
                    }

#line 221
                }

#line 216
            }

#line 211
        }

#line 207
    }
    else
    {

#line 239
        if(is_first_of_loop_0)
        {

#line 239
            _S5 = has_prev_0;

#line 239
        }
        else
        {

#line 239
            _S5 = false;

#line 239
        }

#line 239
        if(_S5)
        {

#line 239
            _S5 = lx_0 < 0.0;

#line 239
        }
        else
        {

#line 239
            _S5 = false;

#line 239
        }

#line 239
        float dx_dash_0;

#line 239
        if(_S5)
        {

#line 239
            dx_dash_0 = PS_CONSTANT_BUFFER_0.total_length_0 + dx_1;

#line 239
        }
        else
        {

#line 239
            dx_dash_0 = dx_1;

#line 239
        }

        if(is_last_of_loop_0)
        {

#line 241
            _S5 = has_next_0;

#line 241
        }
        else
        {

#line 241
            _S5 = false;

#line 241
        }

#line 241
        if(_S5)
        {

#line 241
            _S5 = lx_0 > seg_len_0;

#line 241
        }
        else
        {

#line 241
            _S5 = false;

#line 241
        }

#line 241
        if(_S5)
        {

#line 241
            dx_dash_0 = dx_1 - PS_CONSTANT_BUFFER_0.total_length_0;

#line 241
        }
        else
        {

#line 241
        }


        float u_0 = dx_dash_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;
        float m_0 = u_0 - period_0 * floor(u_0 / period_0);


        if(m_0 < dash_len_0)
        {

#line 248
            d_2 = abs(ly_0);

#line 248
        }
        else
        {

#line 261
            float _S12 = abs(ly_0);

#line 261
            d_2 = min(cap_dist_0(cap_type_0, m_0 - dash_len_0, _S12, t_1), cap_dist_0(cap_type_0, period_0 - m_0, _S12, t_1));

#line 248
        }

#line 267
        if(_S4)
        {

#line 267
            _S5 = dx_1 < 0.0;

#line 267
        }
        else
        {

#line 267
            _S5 = false;

#line 267
        }

#line 267
        if(_S5)
        {

#line 267
            d_2 = cap_dist_0(cap_type_0, - dx_1, abs(ly_0), t_1);

#line 267
        }
        else
        {

#line 269
            if(_S6)
            {

#line 269
                _S5 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);

#line 269
            }
            else
            {

#line 269
                _S5 = false;

#line 269
            }

#line 269
            if(_S5)
            {

#line 269
                d_2 = cap_dist_0(cap_type_0, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, abs(ly_0), t_1);

#line 269
            }
            else
            {

#line 269
            }

#line 267
        }

#line 280
        if(has_prev_0)
        {

#line 280
            _S5 = lx_0 < 0.0;

#line 280
        }
        else
        {

#line 280
            _S5 = false;

#line 280
        }

#line 280
        if(_S5)
        {
            float jd_0 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);

#line 282
            float v_al_0;


            if(is_first_of_loop_0)
            {

#line 285
                v_al_0 = PS_CONSTANT_BUFFER_0.total_length_0;

#line 285
            }
            else
            {

#line 285
                v_al_0 = PS_CONSTANT_BUFFER_0.seg_start_0;

#line 285
            }
            float v_u_0 = v_al_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;

            if((v_u_0 - period_0 * floor(v_u_0 / period_0)) < dash_len_0)
            {

#line 288
                max_ext_1 = jd_0;

#line 288
            }
            else
            {

#line 288
                max_ext_1 = max(d_2, jd_0);

#line 288
            }

#line 288
            zone_0 = 2;

#line 288
            d_2 = max_ext_1;

#line 280
        }
        else
        {

#line 291
            if(has_next_0)
            {

#line 291
                _S5 = lx_0 > seg_len_0;

#line 291
            }
            else
            {

#line 291
                _S5 = false;

#line 291
            }

#line 291
            if(_S5)
            {
                float jd_1 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);

                float v_u_1 = PS_CONSTANT_BUFFER_0.seg_end_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;

                if((v_u_1 - period_0 * floor(v_u_1 / period_0)) < dash_len_0)
                {

#line 297
                    max_ext_1 = jd_1;

#line 297
                }
                else
                {

#line 297
                    max_ext_1 = max(d_2, jd_1);

#line 297
                }

#line 297
                zone_0 = 3;

#line 297
                d_2 = max_ext_1;

#line 291
            }
            else
            {

#line 291
                zone_0 = 0;

#line 291
            }

#line 280
        }

#line 207
    }

#line 303
    float d_3 = d_2 - t_1;

#line 303
    vec3 dc_0;
    if(d_3 < 0.0)
    {
        if(dbg_0)
        {

#line 306
            _S5 = zone_0 >= 2;

#line 306
        }
        else
        {

#line 306
            _S5 = false;

#line 306
        }

#line 306
        if(_S5)
        {

            if(jtype_1 == 0)
            {

#line 309
                dc_0 = vec3(0.0, 1.0, 0.0);

#line 309
            }
            else
            {

#line 309
                if(jtype_1 == 2)
                {

#line 309
                    dc_0 = vec3(0.0, 0.0, 1.0);

#line 309
                }
                else
                {

#line 309
                    dc_0 = vec3(1.0, 0.0, 0.0);

#line 309
                }

#line 309
            }

#line 309
            entryPointParam_main_ps_0 = vec4(dc_0, effective_color_0.w);

#line 309
            return;
        }

#line 309
        entryPointParam_main_ps_0 = vec4(effective_color_0.xyz, effective_color_0.w);

#line 309
        return;
    }
    else
    {



        float d_4 = d_3 / max(PS_CONSTANT_BUFFER_0.aa_0, 0.00000999999974738);
        float a_0 = exp(- d_4 * d_4) * effective_color_0.w;
        if(dbg_0)
        {

#line 318
            _S5 = zone_0 >= 2;

#line 318
        }
        else
        {

#line 318
            _S5 = false;

#line 318
        }

#line 318
        if(_S5)
        {
            if(jtype_1 == 0)
            {

#line 320
                dc_0 = vec3(0.0, 1.0, 0.0);

#line 320
            }
            else
            {

#line 320
                if(jtype_1 == 2)
                {

#line 320
                    dc_0 = vec3(0.0, 0.0, 1.0);

#line 320
                }
                else
                {

#line 320
                    dc_0 = vec3(1.0, 0.0, 0.0);

#line 320
                }

#line 320
            }

#line 320
            entryPointParam_main_ps_0 = vec4(dc_0, a_0);

#line 320
            return;
        }

#line 320
        entryPointParam_main_ps_0 = vec4(effective_color_0.xyz, a_0);

#line 320
        return;
    }

#line 320
}

