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

#line 70
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


#line 142
float join_dist_0(vec2 P_world_0, vec2 vertex_0, vec2 cur_dir_0, vec2 adj_dir_0, float dy_1, int jtype_0, float halfw_0, float mlimit_0)
{



    if(jtype_0 == 0)
    {

#line 148
        return length(P_world_0 - vertex_0);
    }

#line 154
    vec2 _S3 = P_world_0 - vertex_0;



    float d_0 = max(abs(dy_1), abs(dot(_S3, vec2(- adj_dir_0.y, adj_dir_0.x))));


    vec2 bisect_0 = cur_dir_0 + adj_dir_0;
    float bl2_0 = dot(bisect_0, bisect_0);

#line 162
    float d_1;
    if(bl2_0 > 0.00100000004749745)
    {
        vec2 bn_0 = bisect_0 * (inversesqrt((bl2_0)));

        float miter_d_0 = abs(dot(_S3, vec2(- bn_0.y, bn_0.x)));

        if(jtype_0 == 1)
        {

#line 169
            d_1 = max(d_0, miter_d_0 - mlimit_0 * halfw_0);

#line 169
        }
        else
        {

#line 169
            d_1 = max(d_0, miter_d_0);

#line 169
        }

#line 163
    }
    else
    {

#line 163
        d_1 = d_0;

#line 163
    }

#line 175
    return d_1;
}


#line 97
float eval_dash_sdf_0(float u_rel_0, float dash_len_0, int cap_type_0, float abs_dy_0, float halfw_1, float t_1)
{

#line 98
    float d_2;

    if(cap_type_0 == 5)
    {

#line 108
        if(u_rel_0 < halfw_1)
        {

#line 108
            d_2 = max(abs_dy_0, cap_dist_0(5, halfw_1 - u_rel_0, abs_dy_0, t_1));

#line 108
        }
        else
        {

#line 108
            d_2 = abs_dy_0;

#line 108
        }

        float _S4 = dash_len_0 - halfw_1;

#line 110
        if(u_rel_0 > _S4)
        {

#line 110
            d_2 = max(d_2, cap_dist_0(5, u_rel_0 - _S4, abs_dy_0, t_1));

#line 110
        }
        else
        {

#line 110
        }

        return d_2;
    }

#line 112
    bool _S5;

    if(cap_type_0 == 0)
    {

#line 114
        _S5 = true;

#line 114
    }
    else
    {

#line 114
        _S5 = cap_type_0 == 1;

#line 114
    }

#line 114
    if(_S5)
    {

#line 114
        d_2 = 0.0;

#line 114
    }
    else
    {

#line 114
        d_2 = halfw_1;

#line 114
    }
    if(u_rel_0 < d_2)
    {

#line 116
        return cap_dist_0(cap_type_0, d_2 - u_rel_0, abs_dy_0, t_1);
    }

#line 117
    float _S6 = dash_len_0 - d_2;

#line 117
    if(u_rel_0 > _S6)
    {

#line 118
        return cap_dist_0(cap_type_0, u_rel_0 - _S6, abs_dy_0, t_1);
    }

#line 119
    return abs_dy_0;
}


#line 8639 1
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 8639
layout(location = 1)
in vec2 input_uv_0;


#line 178 0
void main()
{
    vec2 P_0 = mix(PS_CONSTANT_BUFFER_0.rect_min_0, PS_CONSTANT_BUFFER_0.rect_max_0, input_uv_0.xy);



    float seg_len_0 = max(PS_CONSTANT_BUFFER_0.seg_end_0 - PS_CONSTANT_BUFFER_0.seg_start_0, 0.00000999999974738);

    vec2 ex_0 = (PS_CONSTANT_BUFFER_0.p1_0 - PS_CONSTANT_BUFFER_0.p0_0) * (1.0 / seg_len_0);
    float _S7 = ex_0.y;

#line 187
    float _S8 = ex_0.x;

#line 187
    vec2 ey_0 = vec2(- _S7, _S8);

#line 198
    float effective_thickness_0 = max(PS_CONSTANT_BUFFER_0.thickness_0, 1.0);
    vec4 effective_color_0 = PS_CONSTANT_BUFFER_0.color_0;
    if((PS_CONSTANT_BUFFER_0.thickness_0) < 1.0)
    {

#line 201
        effective_color_0[3] = min(effective_color_0.w, PS_CONSTANT_BUFFER_0.thickness_0);

#line 200
    }


    float halfw_2 = 0.5 * effective_thickness_0;
    float t_2 = halfw_2 - PS_CONSTANT_BUFFER_0.aa_0;

    float lx_0 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ex_0);
    float ly_0 = dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ey_0);
    float dx_1 = PS_CONSTANT_BUFFER_0.seg_start_0 + lx_0;


    bool has_prev_0 = (dot(PS_CONSTANT_BUFFER_0.prev_dir_0, PS_CONSTANT_BUFFER_0.prev_dir_0)) > 0.00009999999747379;
    bool has_next_0 = (dot(PS_CONSTANT_BUFFER_0.next_dir_0, PS_CONSTANT_BUFFER_0.next_dir_0)) > 0.00009999999747379;
    int cap_type_1 = int(PS_CONSTANT_BUFFER_0.cap_0);
    int jtype_1 = int(PS_CONSTANT_BUFFER_0.join_type_0);

#line 219
    int flags_0 = int(PS_CONSTANT_BUFFER_0.debug_joins_0);
    bool dbg_0 = (flags_0 & 1) != 0;
    bool is_first_of_loop_0 = (flags_0 & 2) != 0;
    bool is_last_of_loop_0 = (flags_0 & 4) != 0;

#line 229
    if(has_prev_0)
    {
        vec2 bisect_1 = PS_CONSTANT_BUFFER_0.prev_dir_0 + ex_0;
        if((dot(bisect_1, bisect_1)) > 0.00100000004749745)
        {
            if((dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, bisect_1)) <= 0.0)
            {

#line 234
                entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 234
                return;
            }

#line 232
        }

#line 229
    }

#line 238
    if(has_next_0)
    {
        vec2 bisect_2 = ex_0 + PS_CONSTANT_BUFFER_0.next_dir_0;
        if((dot(bisect_2, bisect_2)) > 0.00100000004749745)
        {
            if((dot(P_0 - PS_CONSTANT_BUFFER_0.p1_0, bisect_2)) > 0.0)
            {

#line 243
                entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 243
                return;
            }

#line 241
        }

#line 238
    }

#line 249
    float max_ext_0 = halfw_2 + PS_CONSTANT_BUFFER_0.aa_0;

#line 249
    float max_ext_1;
    if(jtype_1 == 1)
    {

#line 250
        max_ext_1 = max(max_ext_0, PS_CONSTANT_BUFFER_0.miter_limit_0 * halfw_2 + PS_CONSTANT_BUFFER_0.aa_0);

#line 250
    }
    else
    {

#line 250
        max_ext_1 = max_ext_0;

#line 250
    }
    bool _S9 = !has_prev_0;

#line 251
    bool wrap_active_0;

#line 251
    if(_S9)
    {

#line 251
        wrap_active_0 = dx_1 < (- max_ext_1);

#line 251
    }
    else
    {

#line 251
        wrap_active_0 = false;

#line 251
    }

#line 251
    if(wrap_active_0)
    {

#line 251
        entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 251
        return;
    }

#line 252
    bool _S10 = !has_next_0;

#line 252
    if(_S10)
    {

#line 252
        wrap_active_0 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0 + max_ext_1);

#line 252
    }
    else
    {

#line 252
        wrap_active_0 = false;

#line 252
    }

#line 252
    if(wrap_active_0)
    {

#line 252
        entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 252
        return;
    }

#line 266
    float dash_len_1 = PS_CONSTANT_BUFFER_0.dash_0.x;
    float gap_len_0 = PS_CONSTANT_BUFFER_0.dash_0.y;
    float period_0 = max(0.00000999999974738, dash_len_1 + gap_len_0);

#line 268
    float d_3;

#line 268
    int zone_0;

#line 274
    if(dash_len_1 > 1.0e+06)
    {
        float _S11 = abs(ly_0);

        if(_S9)
        {

#line 278
            wrap_active_0 = dx_1 < 0.0;

#line 278
        }
        else
        {

#line 278
            wrap_active_0 = false;

#line 278
        }

#line 278
        if(wrap_active_0)
        {
            float _S12 = cap_dist_0(cap_type_1, - dx_1, _S11, t_2);

#line 280
            zone_0 = 1;

#line 280
            d_3 = _S12;

#line 278
        }
        else
        {


            if(_S10)
            {

#line 283
                wrap_active_0 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);

#line 283
            }
            else
            {

#line 283
                wrap_active_0 = false;

#line 283
            }

#line 283
            if(wrap_active_0)
            {
                float _S13 = cap_dist_0(cap_type_1, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, _S11, t_2);

#line 285
                zone_0 = 1;

#line 285
                d_3 = _S13;

#line 283
            }
            else
            {


                if(has_prev_0)
                {

#line 288
                    wrap_active_0 = lx_0 < 0.0;

#line 288
                }
                else
                {

#line 288
                    wrap_active_0 = false;

#line 288
                }

#line 288
                if(wrap_active_0)
                {
                    float _S14 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_2, PS_CONSTANT_BUFFER_0.miter_limit_0);

#line 290
                    zone_0 = 2;

#line 290
                    d_3 = _S14;

#line 288
                }
                else
                {


                    if(has_next_0)
                    {

#line 293
                        wrap_active_0 = lx_0 > seg_len_0;

#line 293
                    }
                    else
                    {

#line 293
                        wrap_active_0 = false;

#line 293
                    }

#line 293
                    if(wrap_active_0)
                    {
                        float _S15 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_2, PS_CONSTANT_BUFFER_0.miter_limit_0);

#line 295
                        zone_0 = 3;

#line 295
                        d_3 = _S15;

#line 293
                    }
                    else
                    {

#line 293
                        zone_0 = 0;

#line 293
                        d_3 = _S11;

#line 293
                    }

#line 288
                }

#line 283
            }

#line 278
        }

#line 274
    }
    else
    {

#line 306
        if(is_first_of_loop_0)
        {

#line 306
            wrap_active_0 = has_prev_0;

#line 306
        }
        else
        {

#line 306
            wrap_active_0 = false;

#line 306
        }

#line 306
        if(wrap_active_0)
        {

#line 306
            wrap_active_0 = lx_0 < 0.0;

#line 306
        }
        else
        {

#line 306
            wrap_active_0 = false;

#line 306
        }

#line 306
        float dx_dash_0;

#line 306
        if(wrap_active_0)
        {

#line 306
            dx_dash_0 = PS_CONSTANT_BUFFER_0.total_length_0 + dx_1;

#line 306
        }
        else
        {

#line 306
            dx_dash_0 = dx_1;

#line 306
        }

        if(is_last_of_loop_0)
        {

#line 308
            wrap_active_0 = has_next_0;

#line 308
        }
        else
        {

#line 308
            wrap_active_0 = false;

#line 308
        }

#line 308
        if(wrap_active_0)
        {

#line 308
            wrap_active_0 = lx_0 > seg_len_0;

#line 308
        }
        else
        {

#line 308
            wrap_active_0 = false;

#line 308
        }

#line 308
        if(wrap_active_0)
        {

#line 308
            dx_dash_0 = dx_1 - PS_CONSTANT_BUFFER_0.total_length_0;

#line 308
        }
        else
        {

#line 308
        }


        float u_0 = dx_dash_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;

        float u_rel_1 = u_0 - floor(u_0 / period_0) * period_0;

#line 334
        if(is_first_of_loop_0)
        {

#line 334
            wrap_active_0 = lx_0 < 0.0;

#line 334
        }
        else
        {

#line 334
            wrap_active_0 = false;

#line 334
        }

#line 334
        if(wrap_active_0)
        {

#line 334
            wrap_active_0 = true;

#line 334
        }
        else
        {

#line 335
            if(is_last_of_loop_0)
            {

#line 335
                wrap_active_0 = lx_0 > seg_len_0;

#line 335
            }
            else
            {

#line 335
                wrap_active_0 = false;

#line 335
            }

#line 334
        }

#line 343
        if(!wrap_active_0)
        {

#line 343
            wrap_active_0 = gap_len_0 > 0.0;

#line 343
        }
        else
        {

#line 343
            wrap_active_0 = false;

#line 343
        }

#line 343
        if(wrap_active_0)
        {
            float dash_arc_start_0 = dx_dash_0 - u_rel_1;
            float dash_arc_end_0 = dash_arc_start_0 + dash_len_1;
            if(has_next_0)
            {



                if((abs((atan((_S8 * PS_CONSTANT_BUFFER_0.next_dir_0.y - _S7 * PS_CONSTANT_BUFFER_0.next_dir_0.x),(dot(ex_0, PS_CONSTANT_BUFFER_0.next_dir_0)))))) > 0.2617993950843811)
                {

#line 352
                    wrap_active_0 = dash_arc_start_0 > (PS_CONSTANT_BUFFER_0.seg_end_0);

#line 352
                }
                else
                {

#line 352
                    wrap_active_0 = false;

#line 352
                }

#line 352
                if(wrap_active_0)
                {

#line 352
                    entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 352
                    return;
                }

#line 347
            }

#line 355
            if(has_prev_0)
            {



                if((abs((atan((PS_CONSTANT_BUFFER_0.prev_dir_0.x * _S7 - PS_CONSTANT_BUFFER_0.prev_dir_0.y * _S8),(dot(PS_CONSTANT_BUFFER_0.prev_dir_0, ex_0)))))) > 0.2617993950843811)
                {

#line 360
                    wrap_active_0 = dash_arc_end_0 < (PS_CONSTANT_BUFFER_0.seg_start_0);

#line 360
                }
                else
                {

#line 360
                    wrap_active_0 = false;

#line 360
                }

#line 360
                if(wrap_active_0)
                {

#line 360
                    entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 360
                    return;
                }

#line 355
            }

#line 343
        }

#line 372
        float a_dy_0 = abs(ly_0);



        float _S16 = min(eval_dash_sdf_0(u_rel_1, dash_len_1, cap_type_1, a_dy_0, halfw_2, t_2), min(eval_dash_sdf_0(u_rel_1 + period_0, dash_len_1, cap_type_1, a_dy_0, halfw_2, t_2), eval_dash_sdf_0(u_rel_1 - period_0, dash_len_1, cap_type_1, a_dy_0, halfw_2, t_2)));


        if(_S9)
        {

#line 379
            wrap_active_0 = dx_1 < 0.0;

#line 379
        }
        else
        {

#line 379
            wrap_active_0 = false;

#line 379
        }

#line 379
        if(wrap_active_0)
        {

#line 379
            d_3 = cap_dist_0(cap_type_1, - dx_1, a_dy_0, t_2);

#line 379
        }
        else
        {

#line 381
            if(_S10)
            {

#line 381
                wrap_active_0 = dx_1 > (PS_CONSTANT_BUFFER_0.total_length_0);

#line 381
            }
            else
            {

#line 381
                wrap_active_0 = false;

#line 381
            }

#line 381
            if(wrap_active_0)
            {

#line 381
                d_3 = cap_dist_0(cap_type_1, dx_1 - PS_CONSTANT_BUFFER_0.total_length_0, a_dy_0, t_2);

#line 381
            }
            else
            {

#line 381
                d_3 = _S16;

#line 381
            }

#line 379
        }

#line 392
        if(has_prev_0)
        {

#line 392
            wrap_active_0 = lx_0 < 0.0;

#line 392
        }
        else
        {

#line 392
            wrap_active_0 = false;

#line 392
        }

#line 392
        bool vertex_in_dash_p_0;

#line 392
        if(wrap_active_0)
        {
            float jd_0 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, ex_0, PS_CONSTANT_BUFFER_0.prev_dir_0, ly_0, jtype_1, halfw_2, PS_CONSTANT_BUFFER_0.miter_limit_0);


            float angle_p_0 = (atan((PS_CONSTANT_BUFFER_0.prev_dir_0.x * _S7 - PS_CONSTANT_BUFFER_0.prev_dir_0.y * _S8),(dot(PS_CONSTANT_BUFFER_0.prev_dir_0, ex_0))));

#line 397
            float v_al_0;



            if(is_first_of_loop_0)
            {

#line 401
                v_al_0 = PS_CONSTANT_BUFFER_0.total_length_0;

#line 401
            }
            else
            {

#line 401
                v_al_0 = PS_CONSTANT_BUFFER_0.seg_start_0;

#line 401
            }
            float v_u_0 = v_al_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;

            if((v_u_0 - period_0 * floor(v_u_0 / period_0)) < dash_len_1)
            {

#line 404
                vertex_in_dash_p_0 = true;

#line 404
            }
            else
            {

#line 404
                vertex_in_dash_p_0 = gap_len_0 <= 0.0;

#line 404
            }
            if(vertex_in_dash_p_0)
            {

#line 405
                d_3 = jd_0;

#line 405
            }
            else
            {

#line 405
                d_3 = max(d_3, jd_0);

#line 405
            }

#line 411
            if((abs(angle_p_0)) < 1.57079625129699707)
            {
                if(vertex_in_dash_p_0)
                {

                    float a_0 = angle_p_0 + 1.57079625129699707;

#line 416
                    d_3 = max(abs(- lx_0 * cos(a_0) - ly_0 * sin(a_0)), d_3);

#line 413
                }
                else
                {

#line 420
                    if(cap_type_1 == 2)
                    {

#line 420
                        wrap_active_0 = true;

#line 420
                    }
                    else
                    {

#line 420
                        wrap_active_0 = cap_type_1 == 4;

#line 420
                    }

#line 420
                    if(wrap_active_0)
                    {

#line 420
                        wrap_active_0 = true;

#line 420
                    }
                    else
                    {

#line 420
                        wrap_active_0 = cap_type_1 == 5;

#line 420
                    }

#line 420
                    if(wrap_active_0)
                    {

                        float a_1 = angle_p_0 * 0.5;

                        if((- lx_0 * cos(a_1) - ly_0 * sin(a_1)) > 0.0)
                        {

#line 425
                            entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 425
                            return;
                        }

#line 420
                    }

#line 413
                }

#line 411
            }
            else
            {

#line 411
            }

#line 411
            zone_0 = 2;

#line 392
        }
        else
        {

#line 431
            if(has_next_0)
            {

#line 431
                wrap_active_0 = lx_0 > seg_len_0;

#line 431
            }
            else
            {

#line 431
                wrap_active_0 = false;

#line 431
            }

#line 431
            if(wrap_active_0)
            {
                float jd_1 = join_dist_0(P_0, PS_CONSTANT_BUFFER_0.p1_0, ex_0, PS_CONSTANT_BUFFER_0.next_dir_0, ly_0, jtype_1, halfw_2, PS_CONSTANT_BUFFER_0.miter_limit_0);


                float angle_n_0 = (atan((_S8 * PS_CONSTANT_BUFFER_0.next_dir_0.y - _S7 * PS_CONSTANT_BUFFER_0.next_dir_0.x),(dot(ex_0, PS_CONSTANT_BUFFER_0.next_dir_0))));

                float v_u_1 = PS_CONSTANT_BUFFER_0.seg_end_0 + PS_CONSTANT_BUFFER_0.dash_offset_0;

                if((v_u_1 - period_0 * floor(v_u_1 / period_0)) < dash_len_1)
                {

#line 440
                    vertex_in_dash_p_0 = true;

#line 440
                }
                else
                {

#line 440
                    vertex_in_dash_p_0 = gap_len_0 <= 0.0;

#line 440
                }
                if(vertex_in_dash_p_0)
                {

#line 441
                    d_3 = jd_1;

#line 441
                }
                else
                {

#line 441
                    d_3 = max(d_3, jd_1);

#line 441
                }

                if((abs(angle_n_0)) < 1.57079625129699707)
                {
                    if(vertex_in_dash_p_0)
                    {
                        float a_2 = angle_n_0 + 1.57079625129699707;

#line 447
                        d_3 = max(abs((lx_0 - seg_len_0) * cos(a_2) - ly_0 * sin(a_2)), d_3);

#line 445
                    }
                    else
                    {



                        if(cap_type_1 == 2)
                        {

#line 451
                            wrap_active_0 = true;

#line 451
                        }
                        else
                        {

#line 451
                            wrap_active_0 = cap_type_1 == 4;

#line 451
                        }

#line 451
                        if(wrap_active_0)
                        {

#line 451
                            wrap_active_0 = true;

#line 451
                        }
                        else
                        {

#line 451
                            wrap_active_0 = cap_type_1 == 5;

#line 451
                        }

#line 451
                        if(wrap_active_0)
                        {
                            float a_3 = angle_n_0 * 0.5;

                            if(((lx_0 - seg_len_0) * cos(a_3) - ly_0 * sin(a_3)) > 0.0)
                            {

#line 455
                                entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 455
                                return;
                            }

#line 451
                        }

#line 445
                    }

#line 443
                }
                else
                {

#line 443
                }

#line 443
                zone_0 = 3;

#line 431
            }
            else
            {

#line 431
                zone_0 = 0;

#line 431
            }

#line 392
        }

#line 274
    }

#line 464
    float d_4 = d_3 - t_2;

#line 464
    vec3 dc_0;
    if(d_4 < 0.0)
    {
        if(dbg_0)
        {

#line 467
            wrap_active_0 = zone_0 >= 2;

#line 467
        }
        else
        {

#line 467
            wrap_active_0 = false;

#line 467
        }

#line 467
        if(wrap_active_0)
        {

            if(jtype_1 == 0)
            {

#line 470
                dc_0 = vec3(0.0, 1.0, 0.0);

#line 470
            }
            else
            {

#line 470
                if(jtype_1 == 2)
                {

#line 470
                    dc_0 = vec3(0.0, 0.0, 1.0);

#line 470
                }
                else
                {

#line 470
                    dc_0 = vec3(1.0, 0.0, 0.0);

#line 470
                }

#line 470
            }

#line 470
            entryPointParam_main_ps_0 = vec4(dc_0, effective_color_0.w);

#line 470
            return;
        }

#line 470
        entryPointParam_main_ps_0 = vec4(effective_color_0.xyz, effective_color_0.w);

#line 470
        return;
    }
    else
    {



        float d_5 = d_4 / max(PS_CONSTANT_BUFFER_0.aa_0, 0.00000999999974738);
        float a_4 = exp(- d_5 * d_5) * effective_color_0.w;
        if(dbg_0)
        {

#line 479
            wrap_active_0 = zone_0 >= 2;

#line 479
        }
        else
        {

#line 479
            wrap_active_0 = false;

#line 479
        }

#line 479
        if(wrap_active_0)
        {
            if(jtype_1 == 0)
            {

#line 481
                dc_0 = vec3(0.0, 1.0, 0.0);

#line 481
            }
            else
            {

#line 481
                if(jtype_1 == 2)
                {

#line 481
                    dc_0 = vec3(0.0, 0.0, 1.0);

#line 481
                }
                else
                {

#line 481
                    dc_0 = vec3(1.0, 0.0, 0.0);

#line 481
                }

#line 481
            }

#line 481
            entryPointParam_main_ps_0 = vec4(dc_0, a_4);

#line 481
            return;
        }

#line 481
        entryPointParam_main_ps_0 = vec4(effective_color_0.xyz, a_4);

#line 481
        return;
    }

#line 481
}

