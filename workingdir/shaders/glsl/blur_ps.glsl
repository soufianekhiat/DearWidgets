#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 27 0
struct SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec2 texel_size_0;
    float mode_0;
    float param0_0;
    float param1_0;
    float param2_0;
    vec2 win_center_0;
    vec2 win_half_px_0;
    float win_rounding_0;
    float pad0_0;
    vec2 mouse_uv_0;
    float pad1_0;
    float pad2_0;
};


#line 14
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec2 texel_size_0;
    float mode_0;
    float param0_0;
    float param1_0;
    float param2_0;
    vec2 win_center_0;
    vec2 win_half_px_0;
    float win_rounding_0;
    float pad0_0;
    vec2 mouse_uv_0;
    float pad1_0;
    float pad2_0;
}PS_CONSTANT_BUFFER_0;

#line 69
layout(binding = 0)
uniform texture2D sceneTexture_0;


#line 70
layout(binding = 0)
uniform sampler sceneSampler_0;


#line 115
vec2 uvToPixel_0(vec2 uv_0)
{
    return (uv_0 - PS_CONSTANT_BUFFER_0.win_center_0) / PS_CONSTANT_BUFFER_0.texel_size_0;
}


#line 109
float sdRoundedBox_0(vec2 p_0, vec2 b_0, float r_0)
{
    vec2 q_0 = abs(p_0) - b_0 + r_0;
    return length(max(q_0, vec2(0.0))) + min(max(q_0.x, q_0.y), 0.0) - r_0;
}


#line 120
float sdfWithNormal_0(vec2 uv_1, float extraRadius_0, out vec2 normal_0)
{
    vec2 p_1 = uvToPixel_0(uv_1);

    float r_1 = PS_CONSTANT_BUFFER_0.win_rounding_0 + extraRadius_0;
    float d_0 = sdRoundedBox_0(p_1, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1);

    const vec2 _S1 = vec2(1.0, 0.0);
    const vec2 _S2 = vec2(0.0, 1.0);
    normal_0 = normalize(vec2(sdRoundedBox_0(p_1 + _S1, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1) - sdRoundedBox_0(p_1 - _S1, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1), sdRoundedBox_0(p_1 + _S2, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1) - sdRoundedBox_0(p_1 - _S2, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1)) + vec2(0.00009999999747379, 0.0));
    return d_0;
}


#line 12386 1
float saturate_0(float x_0)
{

#line 12394
    return clamp(x_0, 0.0, 1.0);
}


#line 74 0
vec4 sampleClamped_0(vec2 uv_2)
{
    return (texture(sampler2D(sceneTexture_0,sceneSampler_0), (clamp(uv_2, PS_CONSTANT_BUFFER_0.texel_size_0 * 0.5, 1.0 - PS_CONSTANT_BUFFER_0.texel_size_0 * 0.5))));
}


float srgbToLinear_0(float c_0)
{

#line 80
    float _S3;

    if(c_0 <= 0.04044999927282333)
    {

#line 82
        _S3 = c_0 / 12.92000007629394531;

#line 82
    }
    else
    {

#line 82
        _S3 = pow((c_0 + 0.05499999970197678) / 1.0549999475479126, 2.40000009536743164);

#line 82
    }

#line 82
    return _S3;
}




vec3 srgbToLinear3_0(vec3 c_1)
{

#line 88
    return vec3(srgbToLinear_0(c_1.x), srgbToLinear_0(c_1.y), srgbToLinear_0(c_1.z));
}


vec4 sampleLinear_0(vec2 uv_3)
{
    vec4 _S4 = sampleClamped_0(uv_3);

#line 94
    vec4 c_2 = _S4;
    c_2.xyz = srgbToLinear3_0(_S4.xyz);
    return c_2;
}


#line 84
float linearToSrgb_0(float c_3)
{

#line 84
    float _S5;

    if(c_3 <= 0.00313080009073019)
    {

#line 86
        _S5 = c_3 * 12.92000007629394531;

#line 86
    }
    else
    {

#line 86
        _S5 = 1.0549999475479126 * pow(c_3, 0.4166666567325592) - 0.05499999970197678;

#line 86
    }

#line 86
    return _S5;
}

vec3 linearToSrgb3_0(vec3 c_4)
{

#line 89
    return vec3(linearToSrgb_0(c_4.x), linearToSrgb_0(c_4.y), linearToSrgb_0(c_4.z));
}


#line 163
vec4 effect_blur_0(vec2 uv_4)
{
    float radius_0 = PS_CONSTANT_BUFFER_0.param0_0;

#line 165
    vec2 _S6;
    if((PS_CONSTANT_BUFFER_0.param1_0) < 0.5)
    {

#line 166
        _S6 = vec2(PS_CONSTANT_BUFFER_0.texel_size_0.x, 0.0);

#line 166
    }
    else
    {

#line 166
        _S6 = vec2(0.0, PS_CONSTANT_BUFFER_0.texel_size_0.y);

#line 166
    }

    float bevelPx_0 = max(radius_0 * 2.0, 8.0);
    vec2 normal_1;
    float d_1 = sdfWithNormal_0(uv_4, 0.0, normal_1);


    vec2 _S7 = uv_4 + normal_1 * (saturate_0(1.0 - saturate_0(- d_1 / bevelPx_0)) * radius_0 * 0.5) * PS_CONSTANT_BUFFER_0.texel_size_0;
    float edgeHighlight_0 = pow(saturate_0(1.0 - abs(d_1) / (bevelPx_0 * 0.5)), 3.0) * 0.15000000596046448;

    float _S8 = radius_0 / 6.0;
    const float  w_0[7] = { 0.19648249447345734, 0.17489039897918701, 0.1225150004029274, 0.06753169745206833, 0.0292431004345417, 0.00995950028300285, 0.00266269990243018 };


    vec3 _S9 = sampleLinear_0(_S7).xyz * 0.19648249447345734;

#line 180
    int i_0 = 1;

#line 180
    vec3 lin_0 = _S9;
    for(;;)
    {

#line 181
        if(i_0 < 7)
        {
        }
        else
        {

#line 181
            break;
        }
        vec2 off_0 = _S6 * (float(i_0) * _S8);

        vec3 lin_1 = lin_0 + sampleLinear_0(_S7 + off_0).xyz * w_0[i_0] + sampleLinear_0(_S7 - off_0).xyz * w_0[i_0];

#line 181
        i_0 = i_0 + 1;

#line 181
        lin_0 = lin_1;

#line 181
    }

#line 189
    vec4 col_0;
    col_0.xyz = linearToSrgb3_0(lin_0 + edgeHighlight_0);
    col_0[3] = 1.0;
    return col_0;
}


#line 192
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 192
layout(location = 0)
in vec4 input_col_0;


#line 192
layout(location = 1)
in vec2 input_uv_0;


#line 927
void main()
{

    vec4 col_1 = effect_blur_0(input_uv_0);

#line 968
    col_1[3] = 1.0;

#line 968
    entryPointParam_main_ps_0 = col_1 * input_col_0;

#line 968
    return;
}

