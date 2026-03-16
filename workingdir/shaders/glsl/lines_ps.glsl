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

#line 55
float cap_dist_0(int ctype_0, float dx_0, float dy_0, float t_0)
{
    float _S1 = abs(dx_0);
    float _S2 = abs(dy_0);
    if(ctype_0 == 0)
    {

#line 59
        return 1.0e+10;
    }

#line 60
    if(ctype_0 == 1)
    {

#line 60
        return max(_S1 + t_0, _S2);
    }

#line 61
    if(ctype_0 == 2)
    {

#line 61
        return max(_S1, _S2);
    }

#line 62
    if(ctype_0 == 3)
    {

#line 62
        return sqrt(_S1 * _S1 + _S2 * _S2);
    }

#line 63
    if(ctype_0 == 4)
    {

#line 63
        return max(_S2, t_0 + _S1 - _S2);
    }

#line 64
    if(ctype_0 == 5)
    {

#line 64
        return _S1 + _S2;
    }

#line 65
    return 1.0e+10;
}


#line 80
float join_dist_0(vec2 P_world_0, vec2 vertex_0, vec2 cur_dir_0, vec2 adj_dir_0, float dy_1, int jtype_0, float halfw_0, float mlimit_0)
{



    if(jtype_0 == 0)
    {

#line 86
        return length(P_world_0 - vertex_0);
    }

#line 92
    vec2 _S3 = P_world_0 - vertex_0;



    float d_0 = max(abs(dy_1), abs(dot(_S3, vec2(- adj_dir_0.y, adj_dir_0.x))));


    vec2 bisect_0 = cur_dir_0 + adj_dir_0;
    float bl2_0 = dot(bisect_0, bisect_0);

#line 100
    float d_1;
    if(bl2_0 > 0.00100000004749745)
    {
        vec2 bn_0 = bisect_0 * (inversesqrt((bl2_0)));

        float miter_d_0 = abs(dot(_S3, vec2(- bn_0.y, bn_0.x)));

        if(jtype_0 == 1)
        {

#line 107
            d_1 = max(d_0, miter_d_0 - mlimit_0 * halfw_0);

#line 107
        }
        else
        {

#line 107
            d_1 = max(d_0, miter_d_0);

#line 107
        }

#line 101
    }
    else
    {

#line 101
        d_1 = d_0;

#line 101
    }

#line 113
    return d_1;
}


#line 8639 1
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 8639
layout(location = 1)
in vec2 input_uv_0;


#line 116 0
void main()
{
    vec2 P_0 = mix(PS_CONSTANT_BUFFER_0.rect_min_0, PS_CONSTANT_BUFFER_0.rect_max_0, input_uv_0.xy);
    vec2 ba_vec_0 = PS_CONSTANT_BUFFER_0.p1_0 - PS_CONSTANT_BUFFER_0.p0_0;
    float seg_len_0 = max(length(ba_vec_0), 0.00000999999974738);
    vec2 ex_0 = ba_vec_0 / seg_len_0;

    float halfw_1 = 0.5 * PS_CONSTANT_BUFFER_0.thickness_0;
    float t_1 = halfw_1 - PS_CONSTANT_BUFFER_0.aa_0;

    float lx_0 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ex_0);
    float ly_0 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, vec2(- ex_0.y, ex_0.x));
    float dx_1 = PS_CONSTANT_BUFFER_0.seg_start_0 + lx_0;


    bool has_prev_0 = (dot(PS_CONSTANT_BUFFER_0.prev_dir_0, PS_CONSTANT_BUFFER_0.prev_dir_0)) > 0.00009999999747379;
    bool has_next_0 = (dot(PS_CONSTANT_BUFFER_0.next_dir_0, PS_CONSTANT_BUFFER_0.next_dir_0)) > 0.00009999999747379;
    int cap_type_0 = int(PS_CONSTANT_BUFFER_0.cap_0);
    int jtype_1 = int(PS_CONSTANT_BUFFER_0.join_type_0);
    bool dbg_0 = (PS_CONSTANT_BUFFER_0.debug_joins_0) > 0.5;


    if(has_prev_0)
    {
        vec2 bisect_1 = PS_CONSTANT_BUFFER_0.prev_dir_0 + ex_0;
        if((dot(bisect_1, bisect_1)) > 0.00100000004749745)
        {
            if((dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, bisect_1)) < 0.0)
            {

#line 143
                entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 143
                return;
            }

#line 141
        }

#line 138
    }

#line 147
    if(has_next_0)
    {
        vec2 bisect_2 = ex_0 + PS_CONSTANT_BUFFER_0.next_dir_0;
        if((dot(bisect_2, bisect_2)) > 0.00100000004749745)
        {
            if((dot(P_0 - PS_CONSTANT_BUFFER_0.p1_0, bisect_2)) > 0.0)
            {

#line 152
                entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 152
                return;
            }

#line 150
        }

#line 147
    }

#line 158
    float max_ext_0 = halfw_1 + PS_CONSTANT_BUFFER_0.aa_0;

#line 158
    float max_ext_1;
    if(jtype_1 == 1)
    {

#line 159
        max_ext_1 = max(max_ext_0, PS_CONSTANT_BUFFER_0.miter_limit_0 * halfw_1 + PS_CONSTANT_BUFFER_0.aa_0);

#line 159
    }
    else
    {

#line 159
        max_ext_1 = max_ext_0;

#line 159
    }
    bool _S4 = !has_prev_0;

#line 160
    bool _S5;

#line 160
    if(_S4)
    {

#line 160
        _S5 = dx_1 < (- max_ext_1);

#line 160
    }
    else
    {

#line 160
        _S5 = false;

#line 160
    }

#line 160
    if(_S5)
    {

#line 160
        entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 160
        return;
    }

#line 161
    bool _S6 = !has_next_0;

#line 161
    if(_S6)
    {

#line 161
        _S5 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0 + max_ext_1);

#line 161
    }
    else
    {

#line 161
        _S5 = false;

#line 161
    }

#line 161
    if(_S5)
    {

#line 161
        entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 161
        return;
    }

    float dash_len_0 = PS_CONSTANT_BUFFER_0.dash_0.x;
    float gap_len_0 = PS_CONSTANT_BUFFER_0.dash_0.y;
    float period_0 = max(0.00000999999974738, dash_len_0 + gap_len_0);

#line 166
    float d_2;

#line 166
    int zone_0;

#line 172
    if(gap_len_0 < 0.5)
    {
        float _S7 = abs(ly_0);

        if(_S4)
        {

#line 176
            _S5 = dx_1 < 0.0;

#line 176
        }
        else
        {

#line 176
            _S5 = false;

#line 176
        }

#line 176
        if(_S5)
        {
            float _S8 = cap_dist_0(cap_type_0, - dx_1, _S7, t_1);

#line 178
            zone_0 = 1;

#line 178
            d_2 = _S8;

#line 176
        }
        else
        {


            if(_S6)
            {

#line 181
                _S5 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);

#line 181
            }
            else
            {

#line 181
                _S5 = false;

#line 181
            }

#line 181
            if(_S5)
            {
                float _S9 = cap_dist_0(cap_type_0, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, _S7, t_1);

#line 183
                zone_0 = 1;

#line 183
                d_2 = _S9;

#line 181
            }
            else
            {


                if(has_prev_0)
                {

#line 186
                    _S5 = lx_0 < 0.0;

#line 186
                }
                else
                {

#line 186
                    _S5 = false;

#line 186
                }

#line 186
                if(_S5)
                {
                    float _S10 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);

#line 188
                    zone_0 = 2;

#line 188
                    d_2 = _S10;

#line 186
                }
                else
                {


                    if(has_next_0)
                    {

#line 191
                        _S5 = lx_0 > seg_len_0;

#line 191
                    }
                    else
                    {

#line 191
                        _S5 = false;

#line 191
                    }

#line 191
                    if(_S5)
                    {
                        float _S11 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);

#line 193
                        zone_0 = 3;

#line 193
                        d_2 = _S11;

#line 191
                    }
                    else
                    {

#line 191
                        zone_0 = 0;

#line 191
                        d_2 = _S7;

#line 191
                    }

#line 186
                }

#line 181
            }

#line 176
        }

#line 172
    }
    else
    {

#line 204
        if(has_prev_0)
        {

#line 204
            _S5 = (PS_CONSTANT_BUFFER_0.seg_start_0) < 0.00100000004749745;

#line 204
        }
        else
        {

#line 204
            _S5 = false;

#line 204
        }

#line 204
        if(_S5)
        {

#line 204
            _S5 = lx_0 < 0.0;

#line 204
        }
        else
        {

#line 204
            _S5 = false;

#line 204
        }

#line 204
        float dx_dash_0;

#line 204
        if(_S5)
        {

#line 204
            dx_dash_0 = PS_CONSTANT_BUFFER_0.total_length_0 + dx_1;

#line 204
        }
        else
        {

#line 204
            dx_dash_0 = dx_1;

#line 204
        }

        if(has_next_0)
        {

#line 206
            _S5 = (PS_CONSTANT_BUFFER_0.seg_end_0) > (PS_CONSTANT_BUFFER_0.total_length_0 - 0.00100000004749745);

#line 206
        }
        else
        {

#line 206
            _S5 = false;

#line 206
        }

#line 206
        if(_S5)
        {

#line 206
            _S5 = lx_0 > seg_len_0;

#line 206
        }
        else
        {

#line 206
            _S5 = false;

#line 206
        }

#line 206
        if(_S5)
        {

#line 206
            dx_dash_0 = dx_1 - PS_CONSTANT_BUFFER_0.total_length_0;

#line 206
        }
        else
        {

#line 206
        }


        float u_0 = dx_dash_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;
        float m_0 = u_0 - period_0 * floor(u_0 / period_0);


        if(m_0 < dash_len_0)
        {
            float d_3 = abs(ly_0);


            float d_start_0 = cap_dist_0(cap_type_0, m_0, d_3, t_1);
            float d_end_0 = cap_dist_0(cap_type_0, dash_len_0 - m_0, d_3, t_1);
            if(cap_type_0 == 5)
            {

#line 220
                d_2 = max(d_3, min(d_start_0, d_end_0));

#line 220
            }
            else
            {

#line 220
                d_2 = d_3;

#line 220
            }

#line 213
        }
        else
        {

#line 227
            float _S12 = abs(ly_0);

#line 227
            d_2 = min(cap_dist_0(cap_type_0, m_0 - dash_len_0, _S12, t_1), cap_dist_0(cap_type_0, period_0 - m_0, _S12, t_1));

#line 213
        }

#line 233
        if(_S4)
        {

#line 233
            _S5 = dx_1 < 0.0;

#line 233
        }
        else
        {

#line 233
            _S5 = false;

#line 233
        }

#line 233
        if(_S5)
        {

#line 233
            d_2 = cap_dist_0(cap_type_0, - dx_1, abs(ly_0), t_1);

#line 233
        }
        else
        {

#line 235
            if(_S6)
            {

#line 235
                _S5 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);

#line 235
            }
            else
            {

#line 235
                _S5 = false;

#line 235
            }

#line 235
            if(_S5)
            {

#line 235
                d_2 = cap_dist_0(cap_type_0, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, abs(ly_0), t_1);

#line 235
            }
            else
            {

#line 235
            }

#line 233
        }

#line 246
        if(has_prev_0)
        {

#line 246
            _S5 = lx_0 < 0.0;

#line 246
        }
        else
        {

#line 246
            _S5 = false;

#line 246
        }

#line 246
        if(_S5)
        {
            float jd_0 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);

#line 248
            float v_al_0;

            if((PS_CONSTANT_BUFFER_0.seg_start_0) < 0.00100000004749745)
            {

#line 250
                v_al_0 = PS_CONSTANT_BUFFER_0.total_length_0;

#line 250
            }
            else
            {

#line 250
                v_al_0 = PS_CONSTANT_BUFFER_0.seg_start_0;

#line 250
            }
            float v_u_0 = v_al_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;

            if((v_u_0 - period_0 * floor(v_u_0 / period_0)) < dash_len_0)
            {

#line 253
                max_ext_1 = jd_0;

#line 253
            }
            else
            {

#line 253
                max_ext_1 = max(d_2, jd_0);

#line 253
            }

#line 253
            zone_0 = 2;

#line 253
            d_2 = max_ext_1;

#line 246
        }
        else
        {

#line 256
            if(has_next_0)
            {

#line 256
                _S5 = lx_0 > seg_len_0;

#line 256
            }
            else
            {

#line 256
                _S5 = false;

#line 256
            }

#line 256
            if(_S5)
            {
                float jd_1 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_1, PS_CONSTANT_BUFFER_0.miter_limit_0);

                float v_u_1 = PS_CONSTANT_BUFFER_0.seg_end_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;

                if((v_u_1 - period_0 * floor(v_u_1 / period_0)) < dash_len_0)
                {

#line 262
                    max_ext_1 = jd_1;

#line 262
                }
                else
                {

#line 262
                    max_ext_1 = max(d_2, jd_1);

#line 262
                }

#line 262
                zone_0 = 3;

#line 262
                d_2 = max_ext_1;

#line 256
            }
            else
            {

#line 256
                zone_0 = 0;

#line 256
            }

#line 246
        }

#line 172
    }

#line 268
    float d_4 = d_2 - t_1;

#line 268
    vec3 dc_0;
    if(d_4 < 0.0)
    {
        if(dbg_0)
        {

#line 271
            _S5 = zone_0 >= 2;

#line 271
        }
        else
        {

#line 271
            _S5 = false;

#line 271
        }

#line 271
        if(_S5)
        {

            if(jtype_1 == 0)
            {

#line 274
                dc_0 = vec3(0.0, 1.0, 0.0);

#line 274
            }
            else
            {

#line 274
                if(jtype_1 == 2)
                {

#line 274
                    dc_0 = vec3(0.0, 0.0, 1.0);

#line 274
                }
                else
                {

#line 274
                    dc_0 = vec3(1.0, 0.0, 0.0);

#line 274
                }

#line 274
            }

#line 274
            entryPointParam_main_ps_0 = vec4(dc_0, PS_CONSTANT_BUFFER_0.color_0.w);

#line 274
            return;
        }

#line 274
        entryPointParam_main_ps_0 = vec4(PS_CONSTANT_BUFFER_0.color_0.xyz, PS_CONSTANT_BUFFER_0.color_0.w);

#line 274
        return;
    }
    else
    {



        float d_5 = d_4 / max(PS_CONSTANT_BUFFER_0.aa_0, 0.00000999999974738);
        float a_0 = exp(- d_5 * d_5) * PS_CONSTANT_BUFFER_0.color_0.w;
        if(dbg_0)
        {

#line 283
            _S5 = zone_0 >= 2;

#line 283
        }
        else
        {

#line 283
            _S5 = false;

#line 283
        }

#line 283
        if(_S5)
        {
            if(jtype_1 == 0)
            {

#line 285
                dc_0 = vec3(0.0, 1.0, 0.0);

#line 285
            }
            else
            {

#line 285
                if(jtype_1 == 2)
                {

#line 285
                    dc_0 = vec3(0.0, 0.0, 1.0);

#line 285
                }
                else
                {

#line 285
                    dc_0 = vec3(1.0, 0.0, 0.0);

#line 285
                }

#line 285
            }

#line 285
            entryPointParam_main_ps_0 = vec4(dc_0, a_0);

#line 285
            return;
        }

#line 285
        entryPointParam_main_ps_0 = vec4(PS_CONSTANT_BUFFER_0.color_0.xyz, a_0);

#line 285
        return;
    }

#line 285
}

