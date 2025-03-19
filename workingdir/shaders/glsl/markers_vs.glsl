#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 1972 0
struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    vec4  data_0[4];
};


#line 1972
struct SLANG_ParameterGroup_vertexBuffer_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 ProjectionMatrix_0;
};


#line 16 1
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_vertexBuffer_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 ProjectionMatrix_0;
}vertexBuffer_0;

#line 16
mat4x4 unpackStorage_0(_MatrixStorage_float4x4_ColMajorstd140_0 _S1)
{

#line 16
    return mat4x4(_S1.data_0[0][0], _S1.data_0[1][0], _S1.data_0[2][0], _S1.data_0[3][0], _S1.data_0[0][1], _S1.data_0[1][1], _S1.data_0[2][1], _S1.data_0[3][1], _S1.data_0[0][2], _S1.data_0[1][2], _S1.data_0[2][2], _S1.data_0[3][2], _S1.data_0[0][3], _S1.data_0[1][3], _S1.data_0[2][3], _S1.data_0[3][3]);
}


#line 11470 2
layout(location = 0)
out vec4 entryPointParam_main_vs_col_0;


#line 21 1
layout(location = 1)
out vec2 entryPointParam_main_vs_uv_0;


#line 21
layout(location = 0)
in vec2 input_pos_0;


#line 21
layout(location = 1)
in vec4 input_col_0;


#line 21
layout(location = 2)
in vec2 input_uv_0;


#line 28
struct PS_INPUT_0
{
    vec4 pos_0;
    vec4 col_0;
    vec2 uv_0;
};


#line 353
void main()
{
    PS_INPUT_0 output_0;
    output_0.pos_0 = (((vec4(input_pos_0.xy, 0.0, 1.0)) * (unpackStorage_0(vertexBuffer_0.ProjectionMatrix_0))));
    output_0.col_0 = input_col_0;
    output_0.uv_0 = input_uv_0;
    PS_INPUT_0 _S2 = output_0;

#line 359
    gl_Position = output_0.pos_0;

#line 359
    entryPointParam_main_vs_col_0 = _S2.col_0;

#line 359
    entryPointParam_main_vs_uv_0 = _S2.uv_0;

#line 359
    return;
}

