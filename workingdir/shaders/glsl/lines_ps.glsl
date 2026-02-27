#version 450
layout(row_major) uniform;
layout(row_major) buffer;

// Layout: 80 bytes, must match ImWidgetsDashedLineBuffer in dear_widgets.h
// std140 packing: vec2 aligned to 8, vec4 aligned to 16
struct SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec2 p0_0;            // offset 0
    vec2 p1_0;            // offset 8
    float thickness_0;    // offset 16
    float aa_0;           // offset 20
    vec2 dash_0;          // offset 24
    float dash_offset_0;  // offset 32
    float cap_0;          // offset 36
    float join_0;         // offset 40
    float miter_limit_0;  // offset 44
    vec2 rect_min_0;      // offset 48
    vec2 rect_max_0;      // offset 56
    vec4 color_0;         // offset 64
};

layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec2 p0_0;            // offset 0
    vec2 p1_0;            // offset 8
    float thickness_0;    // offset 16
    float aa_0;           // offset 20
    vec2 dash_0;          // offset 24
    float dash_offset_0;  // offset 32
    float cap_0;          // offset 36
    float join_0;         // offset 40
    float miter_limit_0;  // offset 44
    vec2 rect_min_0;      // offset 48
    vec2 rect_max_0;      // offset 56
    vec4 color_0;         // offset 64
}PS_CONSTANT_BUFFER_0;

#line 63
vec2 perp_0(vec2 v_0)
{

#line 63
    return vec2(- v_0.y, v_0.x);
}


#line 48
float sdOrientedBox_0(vec2 p_0, vec2 c_0, vec2 ex_0, vec2 ey_0, vec2 b_0)
{
    vec2 rel_0 = p_0 - c_0;

    vec2 d_0 = abs(vec2(dot(rel_0, ex_0), dot(rel_0, ey_0))) - b_0;
    return length(max(d_0, vec2(0.0))) + min(max(d_0.x, d_0.y), 0.0);
}


#line 12386 2
float saturate_0(float x_0)
{

#line 12394
    return clamp(x_0, 0.0, 1.0);
}


#line 56 1
float sdCapsule_0(vec2 p_1, vec2 a_0, vec2 b_1, float r_0)
{
    vec2 pa_0 = p_1 - a_0;

#line 58
    vec2 ba_0 = b_1 - a_0;

    return length(pa_0 - ba_0 * saturate_0(dot(pa_0, ba_0) / dot(ba_0, ba_0))) - r_0;
}


#line 74
float sdTriangleIsoscelesX_0(vec2 p_2, vec2 q_0)
{



    float _S1 = p_2.y;

#line 79
    vec2 ps_0 = vec2(_S1, p_2.x);
    float _S2 = q_0.x;

#line 80
    float _S3 = q_0.y;

#line 80
    vec2 qs_0 = vec2(_S2, _S3);

    ps_0[0] = abs(_S1);
    vec2 a_1 = ps_0 - qs_0 * saturate_0(dot(ps_0, qs_0) / dot(qs_0, qs_0));
    vec2 b_2 = ps_0 - qs_0 * vec2(saturate_0(ps_0.x / _S2), 1.0);



    return sqrt(min(dot(a_1, a_1), dot(b_2, b_2))) * float((int(sign((float(- (int(sign((_S3))))) * (ps_0.x * _S3 - ps_0.y * _S2))))));
}


#line 65
float main_dash_mask_0(float s_0, float dash_len_0, float gap_len_0)
{
    float period_0 = max(0.00000999999974738, dash_len_0 + gap_len_0);

#line 67
    float _S4;

    if((fract(s_0 / period_0) * period_0) <= dash_len_0)
    {

#line 69
        _S4 = 1.0;

#line 69
    }
    else
    {

#line 69
        _S4 = 0.0;

#line 69
    }

#line 69
    return _S4;
}


#line 69
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 69
layout(location = 1)
in vec2 input_uv_0;


#line 91
void main()
{

    vec2 P_0 = mix(PS_CONSTANT_BUFFER_0.rect_min_0, PS_CONSTANT_BUFFER_0.rect_max_0, input_uv_0.xy);
    vec2 ba_1 = PS_CONSTANT_BUFFER_0.p1_0 - PS_CONSTANT_BUFFER_0.p0_0;
    float len_0 = max(length(ba_1), 0.00000999999974738);
    vec2 ex_1 = ba_1 / len_0;
    vec2 ey_1 = perp_0(ex_1);
    float halfw_0 = 0.5 * PS_CONSTANT_BUFFER_0.thickness_0;

#line 99
    float d_1;

#line 104
    if((PS_CONSTANT_BUFFER_0.cap_0) < 1.5)
    {

#line 104
        d_1 = sdOrientedBox_0(P_0, 0.5 * (PS_CONSTANT_BUFFER_0.p0_0 + PS_CONSTANT_BUFFER_0.p1_0), ex_1, ey_1, vec2(0.5 * len_0, halfw_0));

#line 104
    }
    else
    {



        if((PS_CONSTANT_BUFFER_0.cap_0) < 2.5)
        {

#line 110
            d_1 = sdOrientedBox_0(P_0, 0.5 * (PS_CONSTANT_BUFFER_0.p0_0 + PS_CONSTANT_BUFFER_0.p1_0), ex_1, ey_1, vec2(0.5 * len_0 + halfw_0, halfw_0));

#line 110
        }
        else
        {



            if((PS_CONSTANT_BUFFER_0.cap_0) < 3.5)
            {

#line 116
                d_1 = sdCapsule_0(P_0, PS_CONSTANT_BUFFER_0.p0_0, PS_CONSTANT_BUFFER_0.p1_0, halfw_0);

#line 116
            }
            else
            {

                if((PS_CONSTANT_BUFFER_0.cap_0) < 4.5)
                {

#line 128
                    vec2 _S5 = vec2(halfw_0, halfw_0);

#line 128
                    d_1 = min(sdOrientedBox_0(P_0, 0.5 * (PS_CONSTANT_BUFFER_0.p0_0 + PS_CONSTANT_BUFFER_0.p1_0), ex_1, ey_1, vec2(0.5 * len_0, halfw_0)), min(sdTriangleIsoscelesX_0(vec2(dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, - ex_1), dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ey_1)), _S5), sdTriangleIsoscelesX_0(vec2(dot(P_0 - PS_CONSTANT_BUFFER_0.p1_0, ex_1), dot(P_0 - PS_CONSTANT_BUFFER_0.p1_0, ey_1)), _S5)));

#line 120
                }
                else
                {

#line 146
                    vec2 _S6 = vec2(halfw_0, halfw_0);

#line 146
                    d_1 = max(sdOrientedBox_0(P_0, 0.5 * (PS_CONSTANT_BUFFER_0.p0_0 + PS_CONSTANT_BUFFER_0.p1_0), ex_1, ey_1, vec2(0.5 * len_0, halfw_0)), - min(sdTriangleIsoscelesX_0(vec2(dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ex_1), dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ey_1)), _S6), sdTriangleIsoscelesX_0(vec2(dot(P_0 - PS_CONSTANT_BUFFER_0.p1_0, - ex_1), dot(P_0 - PS_CONSTANT_BUFFER_0.p1_0, ey_1)), _S6)));

#line 120
                }

#line 116
            }

#line 110
        }

#line 104
    }

#line 160
    float vis_0 = saturate_0(0.5 - d_1 / max(PS_CONSTANT_BUFFER_0.aa_0, 0.00000999999974738)) * main_dash_mask_0(dot(P_0 - PS_CONSTANT_BUFFER_0.p0_0, ex_1) + PS_CONSTANT_BUFFER_0.dash_offset_0, PS_CONSTANT_BUFFER_0.dash_0.x, PS_CONSTANT_BUFFER_0.dash_0.y);
    vec4 _S7 = PS_CONSTANT_BUFFER_0.color_0;

#line 161
    vec4 out_col_0 = PS_CONSTANT_BUFFER_0.color_0;
    out_col_0.xyz = _S7.xyz * vis_0;
    out_col_0[3] = out_col_0[3] * vis_0;

#line 163
    entryPointParam_main_ps_0 = out_col_0;

#line 163
    return;
}

