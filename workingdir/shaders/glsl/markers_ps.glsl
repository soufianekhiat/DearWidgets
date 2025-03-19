#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 13 0
struct SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec4 fg_color_0;
    vec4 bg_color_0;
    vec2 rotation_0;
    float linewidth_0;
    float size_0;
    float type_0;
    float antialiasing_0;
    float draw_type_0;
    float pad0_0;
};


#line 1
layout(binding = 0)
layout(std140) uniform block_SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec4 fg_color_0;
    vec4 bg_color_0;
    vec2 rotation_0;
    float linewidth_0;
    float size_0;
    float type_0;
    float antialiasing_0;
    float draw_type_0;
    float pad0_0;
}PS_CONSTANT_BUFFER_0;

#line 39
float disc_0(vec2 P_0, float size_1)
{
    return length(P_0) - size_1 * 0.5;
}


float square_0(vec2 P_1, float size_2)
{
    return max(abs(P_1.x), abs(P_1.y)) - size_2 / 2.82842707633972168;
}


float triangle2_0(vec2 P_2, float size_3)
{
    float _S1 = P_2.x;

#line 53
    float _S2 = P_2.y;



    return max(max(abs(0.70710676908493042 * (_S1 - _S2)), abs(0.70710676908493042 * (_S1 + _S2))) - size_3 / 2.82842707633972168, _S2);
}


float diamond_0(vec2 P_3, float size_4)
{
    float _S3 = P_3.x;

#line 63
    float _S4 = P_3.y;

    return max(abs(0.70710676908493042 * (_S3 - _S4)), abs(0.70710676908493042 * (_S3 + _S4))) - size_4 / 2.82842707633972168;
}


float heart_0(vec2 P_4, float size_5)
{
    float _S5 = P_4.x;

#line 71
    float _S6 = P_4.y;

#line 76
    return min(min(max(abs(0.70710676908493042 * (_S5 - _S6)), abs(0.70710676908493042 * (_S5 + _S6))) - size_5 / 3.5, length(P_4 - 0.70710676908493042 * vec2(1.0, -1.0) * size_5 / 3.5) - size_5 / 3.5), length(P_4 - 0.70710676908493042 * vec2(-1.0, -1.0) * size_5 / 3.5) - size_5 / 3.5);
}


float spade_0(vec2 P_5, float size_6)
{

    float s_0 = size_6 * 0.85000002384185791 / 3.5;
    float _S7 = P_5.x;

#line 84
    float _S8 = P_5.y;

#line 84
    float _S9 = 0.40000000596046448 * s_0;

#line 100
    return min(min(min(max(abs(0.70710676908493042 * (_S7 + _S8) + _S9), abs(0.70710676908493042 * (_S7 - _S8) - _S9)) - s_0, length(P_5 - 0.70710676908493042 * vec2(1.0, 0.20000000298023224) * s_0) - s_0), length(P_5 - 0.70710676908493042 * vec2(-1.0, 0.20000000298023224) * s_0) - s_0), max(- min(length(P_5 - vec2(0.64999997615814209, 0.125) * size_6) - size_6 / 1.60000002384185791, length(P_5 - vec2(-0.64999997615814209, 0.125) * size_6) - size_6 / 1.60000002384185791), max(_S8 - 0.5 * size_6, 0.10000000149011612 * size_6 - _S8)));
}


float club_0(vec2 P_6, float size_7)
{

#line 123
    float _S10 = P_6.y;



    return min(min(min(length(P_6 - 0.22499999403953552 * vec2(cos(-1.57079637050628662), sin(-1.57079637050628662)) * size_7) - size_7 / 4.25, length(P_6 - 0.22499999403953552 * vec2(cos(0.52359879016876221), sin(0.52359879016876221)) * size_7) - size_7 / 4.25), length(P_6 - 0.22499999403953552 * vec2(cos(2.61799383163452148), sin(2.61799383163452148)) * size_7) - size_7 / 4.25), max(- min(length(P_6 - vec2(0.64999997615814209, 0.125) * size_7) - size_7 / 1.60000002384185791, length(P_6 - vec2(-0.64999997615814209, 0.125) * size_7) - size_7 / 1.60000002384185791), max(_S10 - 0.5 * size_7, 0.20000000298023224 * size_7 - _S10)));
}


float chevron_0(vec2 P_7, float size_8)
{
    float _S11 = P_7.x;

#line 133
    float _S12 = P_7.y;

#line 133
    float x_0 = 0.70710676908493042 * (_S11 - _S12);
    float y_0 = 0.70710676908493042 * (_S11 + _S12);


    return max(max(abs(x_0), abs(y_0)) - size_8 / 3.0, - (max(abs(x_0 - size_8 / 3.0), abs(y_0 - size_8 / 3.0)) - size_8 / 3.0));
}


float clover_0(vec2 P_8, float size_9)
{

#line 153
    return min(min(length(P_8 - 0.25 * vec2(cos(-1.57079637050628662), sin(-1.57079637050628662)) * size_9) - size_9 / 3.5, length(P_8 - 0.25 * vec2(cos(0.52359879016876221), sin(0.52359879016876221)) * size_9) - size_9 / 3.5), length(P_8 - 0.25 * vec2(cos(2.61799383163452148), sin(2.61799383163452148)) * size_9) - size_9 / 3.5);
}


float ring_0(vec2 P_9, float size_10)
{
    float _S13 = length(P_9);

    return max(_S13 - size_10 / 2.0, - (_S13 - size_10 / 4.0));
}


float tag_0(vec2 P_10, float size_11)
{
    float _S14 = P_10.x;

#line 167
    float _S15 = abs(P_10.y);

    return max(max(abs(_S14) - size_11 / 2.0, _S15 - size_11 / 6.0), 0.75 * (abs(_S14 - size_11 / 1.5) + _S15 - size_11));
}


float cross_0(vec2 P_11, float size_12)
{
    float _S16 = P_11.x;

#line 175
    float _S17 = P_11.y;

#line 175
    float x_1 = 0.70710676908493042 * (_S16 - _S17);
    float y_1 = 0.70710676908493042 * (_S16 + _S17);

#line 182
    return max(min(max(abs(x_1 - size_12 / 3.0), abs(x_1 + size_12 / 3.0)), max(abs(y_1 - size_12 / 3.0), abs(y_1 + size_12 / 3.0))), max(abs(x_1), abs(y_1))) - size_12 / 2.0;
}


float asterisk_0(vec2 P_12, float size_13)
{
    float _S18 = P_12.x;

#line 188
    float _S19 = P_12.y;

    float _S20 = abs(0.70710676908493042 * (_S18 - _S19));

#line 190
    float _S21 = abs(0.70710676908493042 * (_S18 + _S19));

    float _S22 = abs(_S18);

#line 192
    float _S23 = abs(_S19);

    return min(min(max(_S20 - size_13 / 2.0, _S21 - size_13 / 10.0), max(_S21 - size_13 / 2.0, _S20 - size_13 / 10.0)), min(max(_S22 - size_13 / 2.0, _S23 - size_13 / 10.0), max(_S23 - size_13 / 2.0, _S22 - size_13 / 10.0)));
}


float infinity_0(vec2 P_13, float size_14)
{


    float _S24 = length(P_13 - vec2(0.21250000596046448, 0.0) * size_14);

    float _S25 = length(P_13 - vec2(-0.21250000596046448, 0.0) * size_14);

    return min(max(_S24 - size_14 / 3.5, - (_S24 - size_14 / 7.5)), max(_S25 - size_14 / 3.5, - (_S25 - size_14 / 7.5)));
}


float pin_0(vec2 P_14, float size_15)
{

    float _S26 = length(P_14 - vec2(0.0, -0.15000000596046448) * size_15);

    float _S27 = 2.0 * size_15;



    return max(min(_S26 - size_15 / 2.67499995231628418, max(max(length(P_14 - vec2(1.49000000953674316, -0.80000001192092896) * size_15) - _S27, length(P_14 - vec2(-1.49000000953674316, -0.80000001192092896) * size_15) - _S27), - P_14.y)), - (_S26 - size_15 / 5.0));
}


float arrow_0(vec2 P_15, float size_16)
{
    float _S28 = P_15.x;

#line 225
    float _S29 = abs(P_15.y);


    return min(max(abs(_S28 - size_16 / 6.0) - size_16 / 4.0, _S29 - size_16 / 4.0), max(0.75 * (abs(_S28) + _S29 - size_16 / 2.0), max(abs(_S28 + size_16 / 2.0), _S29) - size_16 / 2.0));
}




float ellipse_0(vec2 P_16, float size_17)
{
    float _S30 = size_17 / 3.0;

#line 236
    float _S31 = size_17 / 2.0;

#line 236
    vec2 ab_0 = vec2(_S30, _S31);
    vec2 p_0 = abs(P_16);

#line 237
    vec2 ab_1;

#line 237
    vec2 p_1;
    if((p_0.x) > (p_0.y))
    {
        vec2 _S32 = p_0.yx;

#line 240
        ab_1 = vec2(_S31, _S30);

#line 240
        p_1 = _S32;

#line 238
    }
    else
    {

#line 238
        ab_1 = ab_0;

#line 238
        p_1 = p_0;

#line 238
    }

#line 243
    float _S33 = ab_1.y;

#line 243
    float _S34 = ab_1.x;

#line 243
    float l_0 = _S33 * _S33 - _S34 * _S34;
    float m_0 = _S34 * p_1.x / l_0;
    float _S35 = p_1.y;

#line 245
    float n_0 = _S33 * _S35 / l_0;
    float m2_0 = m_0 * m_0;
    float n2_0 = n_0 * n_0;

    float c_0 = (m2_0 + n2_0 - 1.0) / 3.0;
    float c3_0 = c_0 * c_0 * c_0;

    float _S36 = m2_0 * n2_0;

#line 252
    float q_0 = c3_0 + _S36 * 2.0;
    float d_0 = c3_0 + _S36;
    float g_0 = m_0 + m_0 * n2_0;

#line 254
    float co_0;



    if(d_0 < 0.0)
    {
        float p_2 = acos(q_0 / c3_0) / 3.0;
        float s_1 = cos(p_2);
        float t_0 = sin(p_2) * sqrt(3.0);
        float _S37 = - c_0;

#line 263
        float rx_0 = sqrt(_S37 * (s_1 + t_0 + 2.0) + m2_0);
        float ry_0 = sqrt(_S37 * (s_1 - t_0 + 2.0) + m2_0);

#line 264
        co_0 = (ry_0 + float((int(sign((l_0))))) * rx_0 + abs(g_0) / (rx_0 * ry_0) - m_0) / 2.0;

#line 258
    }
    else
    {

#line 269
        float h_0 = 2.0 * m_0 * n_0 * sqrt(d_0);
        float _S38 = q_0 + h_0;

#line 270
        float s_2 = float((int(sign((_S38))))) * pow(abs(_S38), 0.3333333432674408);
        float _S39 = q_0 - h_0;

#line 271
        float u_0 = float((int(sign((_S39))))) * pow(abs(_S39), 0.3333333432674408);
        float rx_1 = - s_2 - u_0 - c_0 * 4.0 + 2.0 * m2_0;
        float ry_1 = (s_2 - u_0) * sqrt(3.0);
        float rm_0 = sqrt(rx_1 * rx_1 + ry_1 * ry_1);

#line 274
        co_0 = (ry_1 / sqrt(rm_0 - rx_1) + 2.0 * g_0 / rm_0 - m_0) / 2.0;

#line 258
    }

#line 280
    float _S40 = _S33 * sqrt(1.0 - co_0 * co_0);
    return length(vec2(_S34 * co_0, _S40) - p_1) * float((int(sign((_S35 - _S40)))));
}

float ellipse_fast_0(vec2 P_17, float size_18)
{

#line 290
    float f_0 = length(P_17 * vec2(1.0, 3.0));

    return f_0 * (f_0 - 0.89999997615814209) / length(P_17 * vec2(1.0, 9.0));
}


#line 312
vec4 filled_0(float distance_0, float linewidth_1, float antialias_0, vec4 fill_0)
{



    float border_distance_0 = abs(distance_0) - (linewidth_1 / 2.0 - antialias_0);
    float alpha_0 = border_distance_0 / antialias_0;
    float alpha_1 = exp(- alpha_0 * alpha_0);

#line 319
    vec4 frag_color_0;

    if(border_distance_0 < 0.0)
    {

#line 321
        frag_color_0 = fill_0;

#line 321
    }
    else
    {

#line 323
        if(distance_0 < 0.0)
        {

#line 323
            frag_color_0 = fill_0;

#line 323
        }
        else
        {

#line 323
            frag_color_0 = vec4(fill_0.xyz, alpha_1 * fill_0.w);

#line 323
        }

#line 321
    }

#line 328
    return frag_color_0;
}


#line 295
vec4 stroke_0(float distance_1, float linewidth_2, float antialias_1, vec4 stroke_1)
{



    float border_distance_1 = abs(distance_1) - (linewidth_2 / 2.0 - antialias_1);
    float alpha_2 = border_distance_1 / antialias_1;
    float alpha_3 = exp(- alpha_2 * alpha_2);

#line 302
    vec4 frag_color_1;

    if(border_distance_1 < 0.0)
    {

#line 304
        frag_color_1 = stroke_1;

#line 304
    }
    else
    {

#line 304
        frag_color_1 = vec4(stroke_1.xyz, stroke_1.w * alpha_3);

#line 304
    }

#line 309
    return frag_color_1;
}


#line 331
vec4 outline_0(float distance_2, float linewidth_3, float antialias_2, vec4 stroke_2, vec4 fill_1)
{



    float border_distance_2 = abs(distance_2) - (linewidth_3 / 2.0 - antialias_2);
    float alpha_4 = border_distance_2 / antialias_2;
    float alpha_5 = exp(- alpha_4 * alpha_4);

#line 338
    vec4 frag_color_2;

    if(border_distance_2 < 0.0)
    {

#line 340
        frag_color_2 = stroke_2;

#line 340
    }
    else
    {

#line 342
        if(distance_2 < 0.0)
        {

#line 342
            frag_color_2 = mix(fill_1, stroke_2, vec4(sqrt(alpha_5)));

#line 342
        }
        else
        {

#line 342
            frag_color_2 = vec4(stroke_2.xyz, stroke_2.w * alpha_5);

#line 342
        }

#line 340
    }

#line 347
    return frag_color_2;
}


#line 347
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 347
layout(location = 1)
in vec2 input_uv_0;


#line 362
void main()
{


    const vec4 _S41 = vec4(1.0, 1.0, 1.0, 1.0);

    vec2 _S42 = input_uv_0.xy - 0.5;

#line 368
    vec2 P_18 = _S42;
    P_18[1] = - _S42.y;
    P_18 = vec2(PS_CONSTANT_BUFFER_0.rotation_0.x * P_18.x - PS_CONSTANT_BUFFER_0.rotation_0.y * P_18.y, PS_CONSTANT_BUFFER_0.rotation_0.y * P_18.x + PS_CONSTANT_BUFFER_0.rotation_0.x * P_18.y);


    float antialias_3 = PS_CONSTANT_BUFFER_0.antialiasing_0;
    float point_size_0 = 1.41421353816986084 * PS_CONSTANT_BUFFER_0.size_0 + 2.0 * (PS_CONSTANT_BUFFER_0.linewidth_0 + 1.5 * PS_CONSTANT_BUFFER_0.antialiasing_0);

#line 374
    float distance_3;

    if((PS_CONSTANT_BUFFER_0.type_0) == 0.0)
    {

#line 376
        distance_3 = disc_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 376
    }
    else
    {

#line 378
        if((PS_CONSTANT_BUFFER_0.type_0) == 1.0)
        {

#line 378
            distance_3 = square_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 378
        }
        else
        {

#line 380
            if((PS_CONSTANT_BUFFER_0.type_0) == 2.0)
            {

#line 380
                distance_3 = triangle2_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 380
            }
            else
            {

#line 382
                if((PS_CONSTANT_BUFFER_0.type_0) == 3.0)
                {

#line 382
                    distance_3 = diamond_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 382
                }
                else
                {

#line 384
                    if((PS_CONSTANT_BUFFER_0.type_0) == 4.0)
                    {

#line 384
                        distance_3 = heart_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 384
                    }
                    else
                    {

#line 386
                        if((PS_CONSTANT_BUFFER_0.type_0) == 5.0)
                        {

#line 386
                            distance_3 = spade_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 386
                        }
                        else
                        {

#line 388
                            if((PS_CONSTANT_BUFFER_0.type_0) == 6.0)
                            {

#line 388
                                distance_3 = club_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 388
                            }
                            else
                            {

#line 390
                                if((PS_CONSTANT_BUFFER_0.type_0) == 7.0)
                                {

#line 390
                                    distance_3 = chevron_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 390
                                }
                                else
                                {

#line 392
                                    if((PS_CONSTANT_BUFFER_0.type_0) == 8.0)
                                    {

#line 392
                                        distance_3 = clover_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 392
                                    }
                                    else
                                    {

#line 394
                                        if((PS_CONSTANT_BUFFER_0.type_0) == 9.0)
                                        {

#line 394
                                            distance_3 = ring_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 394
                                        }
                                        else
                                        {

#line 396
                                            if((PS_CONSTANT_BUFFER_0.type_0) == 10.0)
                                            {

#line 396
                                                distance_3 = tag_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 396
                                            }
                                            else
                                            {

#line 398
                                                if((PS_CONSTANT_BUFFER_0.type_0) == 11.0)
                                                {

#line 398
                                                    distance_3 = cross_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 398
                                                }
                                                else
                                                {

#line 400
                                                    if((PS_CONSTANT_BUFFER_0.type_0) == 12.0)
                                                    {

#line 400
                                                        distance_3 = asterisk_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 400
                                                    }
                                                    else
                                                    {

#line 402
                                                        if((PS_CONSTANT_BUFFER_0.type_0) == 13.0)
                                                        {

#line 402
                                                            distance_3 = infinity_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 402
                                                        }
                                                        else
                                                        {

#line 404
                                                            if((PS_CONSTANT_BUFFER_0.type_0) == 14.0)
                                                            {

#line 404
                                                                distance_3 = pin_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 404
                                                            }
                                                            else
                                                            {

#line 406
                                                                if((PS_CONSTANT_BUFFER_0.type_0) == 15.0)
                                                                {

#line 406
                                                                    distance_3 = arrow_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 406
                                                                }
                                                                else
                                                                {

#line 408
                                                                    if((PS_CONSTANT_BUFFER_0.type_0) == 16.0)
                                                                    {

#line 408
                                                                        distance_3 = ellipse_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 408
                                                                    }
                                                                    else
                                                                    {

#line 410
                                                                        if((PS_CONSTANT_BUFFER_0.type_0) == 17.0)
                                                                        {

#line 410
                                                                            distance_3 = ellipse_fast_0(P_18 * point_size_0, PS_CONSTANT_BUFFER_0.size_0);

#line 410
                                                                        }

#line 408
                                                                    }

#line 406
                                                                }

#line 404
                                                            }

#line 402
                                                        }

#line 400
                                                    }

#line 398
                                                }

#line 396
                                            }

#line 394
                                        }

#line 392
                                    }

#line 390
                                }

#line 388
                            }

#line 386
                        }

#line 384
                    }

#line 382
                }

#line 380
            }

#line 378
        }

#line 376
    }

#line 376
    vec4 col_out_0;

#line 413
    if((PS_CONSTANT_BUFFER_0.draw_type_0) == 0.0)
    {

#line 413
        col_out_0 = filled_0(distance_3, PS_CONSTANT_BUFFER_0.linewidth_0, antialias_3, PS_CONSTANT_BUFFER_0.fg_color_0);

#line 413
    }
    else
    {

#line 415
        if((PS_CONSTANT_BUFFER_0.draw_type_0) == 1.0)
        {

#line 415
            col_out_0 = stroke_0(distance_3, PS_CONSTANT_BUFFER_0.linewidth_0, antialias_3, PS_CONSTANT_BUFFER_0.fg_color_0);

#line 415
        }
        else
        {

#line 417
            if((PS_CONSTANT_BUFFER_0.draw_type_0) == 2.0)
            {

#line 417
                col_out_0 = outline_0(distance_3, PS_CONSTANT_BUFFER_0.linewidth_0, antialias_3, PS_CONSTANT_BUFFER_0.fg_color_0, PS_CONSTANT_BUFFER_0.bg_color_0);

#line 417
            }
            else
            {

#line 419
                if((PS_CONSTANT_BUFFER_0.draw_type_0) == 3.0)
                {

#line 419
                    col_out_0 = vec4(vec3(pow(abs(distance_3), 0.45454543828964233)), 1.0);

#line 419
                }
                else
                {

#line 421
                    if((PS_CONSTANT_BUFFER_0.draw_type_0) == 4.0)
                    {

#line 422
                        vec4 _S43 = PS_CONSTANT_BUFFER_0.fg_color_0;

#line 422
                        vec4 _S44 = PS_CONSTANT_BUFFER_0.bg_color_0;

#line 422
                        if(distance_3 > 0.0)
                        {

#line 422
                            col_out_0 = vec4(1.0);

#line 422
                        }
                        else
                        {

#line 422
                            col_out_0 = vec4(0.0);

#line 422
                        }

#line 422
                        col_out_0 = mix(_S43, _S44, col_out_0);

#line 421
                    }
                    else
                    {

#line 421
                        col_out_0 = _S41;

#line 421
                    }

#line 419
                }

#line 417
            }

#line 415
        }

#line 413
    }

#line 413
    entryPointParam_main_ps_0 = col_out_0;

#line 413
    return;
}

