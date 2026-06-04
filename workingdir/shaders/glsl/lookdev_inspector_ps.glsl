#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 21 0
struct SLANG_ParameterGroup_LookDevInspectorParams_std140_0
{
    vec4 sideA_0;
    vec4 sideB_0;
    vec4 divider_0;
    vec4 canvas_0;
};


#line 13
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_LookDevInspectorParams_std140_0
{
    vec4 sideA_0;
    vec4 sideB_0;
    vec4 divider_0;
    vec4 canvas_0;
}LookDevInspectorParams_0;

#line 27
layout(binding = 5)
uniform texture2D texture1_0;


#line 26
layout(binding = 4)
uniform sampler sampler1_0;


#line 25
layout(binding = 3)
uniform texture2D texture0_0;


#line 24
layout(binding = 2)
uniform sampler sampler0_0;


#line 52
vec3 apply_tone_0(vec3 rgb_0, vec4 params_0)
{



    float _S1 = params_0.y;



    return pow(max((rgb_0 * (exp2((params_0.x))) - _S1) / max(params_0.z - _S1, 9.99999997475242708e-07), vec3(0.0)), vec3(1.0 / max(params_0.w, 0.00009999999747379)));
}


#line 12401 1
vec3 saturate_0(vec3 x_0)
{

#line 12409
    return clamp(x_0, vec3(0.0), vec3(1.0));
}


#line 12409
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 12409
layout(location = 0)
in vec4 input_col_0;


#line 12409
layout(location = 1)
in vec2 input_uv_0;


#line 64 0
void main()
{



    float aspect_0 = LookDevInspectorParams_0.canvas_0.x / max(LookDevInspectorParams_0.canvas_0.y, 1.0);

#line 75
    bool onB_0 = ((input_uv_0.x * aspect_0 - LookDevInspectorParams_0.divider_0.x * aspect_0) * cos(LookDevInspectorParams_0.divider_0.z) + (input_uv_0.y - LookDevInspectorParams_0.divider_0.y) * sin(LookDevInspectorParams_0.divider_0.z)) < 0.0;

#line 75
    bool onB_1;
    if((LookDevInspectorParams_0.divider_0.w) > 0.5)
    {

#line 76
        onB_1 = !onB_0;

#line 76
    }
    else
    {

#line 76
        onB_1 = onB_0;

#line 76
    }

    vec4 c_0;
    if(onB_1)
    {
        vec4 _S2 = (texture(sampler2D(texture1_0,sampler1_0), (input_uv_0)));

#line 81
        c_0 = _S2;
        c_0.xyz = apply_tone_0(_S2.xyz, LookDevInspectorParams_0.sideB_0);

#line 79
    }
    else
    {

#line 86
        vec4 _S3 = (texture(sampler2D(texture0_0,sampler0_0), (input_uv_0)));

#line 86
        c_0 = _S3;
        c_0.xyz = apply_tone_0(_S3.xyz, LookDevInspectorParams_0.sideA_0);

#line 79
    }

#line 89
    c_0.xyz = saturate_0(c_0.xyz);

#line 89
    entryPointParam_main_ps_0 = c_0 * input_col_0;

#line 89
    return;
}

