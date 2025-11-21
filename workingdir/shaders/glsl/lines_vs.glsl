#version 450
layout(row_major) uniform;
layout(row_major) buffer;

// ImPlatform expects a simple uniform named ProjMtx
uniform mat4 ProjMtx;


#line 11470 2
layout(location = 0)
out vec4 entryPointParam_main_vs_col_0;


#line 24 1
layout(location = 1)
out vec2 entryPointParam_main_vs_uv_0;


#line 24
layout(location = 0)
in vec2 input_pos_0;


#line 24
layout(location = 1)
in vec4 input_col_0;


#line 24
layout(location = 2)
in vec2 input_uv_0;


#line 31
struct PS_INPUT_0
{
    vec4 pos_0;
    vec4 col_0;
    vec2 uv_0;
};

void main()
{
    PS_INPUT_0 output_0;
    output_0.pos_0 = ProjMtx * vec4(input_pos_0.xy, 0.0, 1.0);
    output_0.col_0 = input_col_0;
    output_0.uv_0 = input_uv_0;
    PS_INPUT_0 _S2 = output_0;

#line 44
    gl_Position = output_0.pos_0;

#line 44
    entryPointParam_main_vs_col_0 = _S2.col_0;

#line 44
    entryPointParam_main_vs_uv_0 = _S2.uv_0;

#line 44
    return;
}

