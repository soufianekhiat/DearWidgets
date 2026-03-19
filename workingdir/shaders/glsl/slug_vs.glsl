#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 12078 0
struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    vec4  data_0[4];
};


#line 12078
struct SLANG_ParameterGroup_vertexBuffer_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 ProjectionMatrix_0;
};


#line 27 1
layout(binding = 0)
layout(std140) uniform block_SLANG_ParameterGroup_vertexBuffer_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 ProjectionMatrix_0;
}vertexBuffer_0;

#line 87
vec2 SlugDilate_0(vec4 pos_0, vec4 tex_0, vec4 jac_0, vec4 m0_0, vec4 m1_0, vec4 m3_0, vec2 dim_0, out vec2 vpos_0)
{
    vec2 _S1 = pos_0.zw;

#line 89
    vec2 n_0 = normalize(_S1);
    vec2 _S2 = m3_0.xy;

#line 90
    vec2 _S3 = pos_0.xy;

#line 90
    float s_0 = dot(_S2, _S3) + m3_0.w;
    float t_0 = dot(_S2, n_0);

    vec2 _S4 = m0_0.xy;

#line 93
    float u_0 = (s_0 * dot(_S4, n_0) - t_0 * (dot(_S4, _S3) + m0_0.w)) * dim_0.x;
    vec2 _S5 = m1_0.xy;

#line 94
    float v_0 = (s_0 * dot(_S5, n_0) - t_0 * (dot(_S5, _S3) + m1_0.w)) * dim_0.y;


    float st_0 = s_0 * t_0;
    float uv_0 = u_0 * u_0 + v_0 * v_0;
    vec2 d_0 = _S1 * (s_0 * s_0 * (st_0 + sqrt(uv_0)) / (uv_0 - st_0 * st_0));

    vpos_0 = _S3 + d_0;
    return vec2(tex_0.x + dot(d_0, jac_0.xy), tex_0.y + dot(d_0, jac_0.zw));
}


#line 69
void SlugUnpack_0(vec4 tex_1, vec4 bnd_0, out vec4 vbnd_0, out ivec4 vgly_0)
{
    uvec2 g_0 = floatBitsToUint(tex_1.zw);
    uint _S6 = g_0.x;

#line 72
    uint _S7 = g_0.y;

#line 72
    vgly_0 = ivec4(int(_S6 & 65535U), int(_S6 >> 16U), int(_S7 & 65535U), int(_S7 >> 16U));
    vbnd_0 = bnd_0;
    return;
}


#line 74
layout(location = 0)
out vec4 entryPointParam_main_vs_color_0;


#line 74
layout(location = 1)
out vec2 entryPointParam_main_vs_texcoord_0;


#line 74
flat layout(location = 2)
out vec4 entryPointParam_main_vs_banding_0;


#line 74
flat layout(location = 3)
out ivec4 entryPointParam_main_vs_glyph_0;


#line 74
layout(location = 0)
in vec4 input_pos_0;


#line 74
layout(location = 1)
in vec4 input_tex_0;


#line 74
layout(location = 2)
in vec4 input_jac_0;


#line 74
layout(location = 3)
in vec4 input_bnd_0;


#line 74
layout(location = 4)
in vec4 input_col_0;


#line 55
struct PS_INPUT_0
{
    vec4 position_0;
    vec4 color_0;
    vec2 texcoord_0;
    vec4 banding_0;
    ivec4 glyph_0;
};


#line 105
void main()
{
    PS_INPUT_0 output_0;

#line 117
    vec2 dilatedPos_0;
    vec2 _S8 = SlugDilate_0(input_pos_0, input_tex_0, input_jac_0, vec4(vertexBuffer_0.ProjectionMatrix_0.data_0[0][0], vertexBuffer_0.ProjectionMatrix_0.data_0[1][0], vertexBuffer_0.ProjectionMatrix_0.data_0[2][0], vertexBuffer_0.ProjectionMatrix_0.data_0[3][0]), vec4(vertexBuffer_0.ProjectionMatrix_0.data_0[0][1], vertexBuffer_0.ProjectionMatrix_0.data_0[1][1], vertexBuffer_0.ProjectionMatrix_0.data_0[2][1], vertexBuffer_0.ProjectionMatrix_0.data_0[3][1]), vec4(vertexBuffer_0.ProjectionMatrix_0.data_0[0][3], vertexBuffer_0.ProjectionMatrix_0.data_0[1][3], vertexBuffer_0.ProjectionMatrix_0.data_0[2][3], vertexBuffer_0.ProjectionMatrix_0.data_0[3][3]), vec2(2.0 / vertexBuffer_0.ProjectionMatrix_0.data_0[0][0], 2.0 / abs(vertexBuffer_0.ProjectionMatrix_0.data_0[1][1])), dilatedPos_0);

#line 118
    output_0.texcoord_0 = _S8;
    output_0.position_0 = (((vec4(dilatedPos_0, 0.0, 1.0)) * (mat4x4(vertexBuffer_0.ProjectionMatrix_0.data_0[0][0], vertexBuffer_0.ProjectionMatrix_0.data_0[1][0], vertexBuffer_0.ProjectionMatrix_0.data_0[2][0], vertexBuffer_0.ProjectionMatrix_0.data_0[3][0], vertexBuffer_0.ProjectionMatrix_0.data_0[0][1], vertexBuffer_0.ProjectionMatrix_0.data_0[1][1], vertexBuffer_0.ProjectionMatrix_0.data_0[2][1], vertexBuffer_0.ProjectionMatrix_0.data_0[3][1], vertexBuffer_0.ProjectionMatrix_0.data_0[0][2], vertexBuffer_0.ProjectionMatrix_0.data_0[1][2], vertexBuffer_0.ProjectionMatrix_0.data_0[2][2], vertexBuffer_0.ProjectionMatrix_0.data_0[3][2], vertexBuffer_0.ProjectionMatrix_0.data_0[0][3], vertexBuffer_0.ProjectionMatrix_0.data_0[1][3], vertexBuffer_0.ProjectionMatrix_0.data_0[2][3], vertexBuffer_0.ProjectionMatrix_0.data_0[3][3]))));
    output_0.color_0 = input_col_0;
    SlugUnpack_0(input_tex_0, input_bnd_0, output_0.banding_0, output_0.glyph_0);
    PS_INPUT_0 _S9 = output_0;

#line 122
    gl_Position = output_0.position_0;

#line 122
    entryPointParam_main_vs_color_0 = _S9.color_0;

#line 122
    entryPointParam_main_vs_texcoord_0 = _S9.texcoord_0;

#line 122
    entryPointParam_main_vs_banding_0 = _S9.banding_0;

#line 122
    entryPointParam_main_vs_glyph_0 = _S9.glyph_0;

#line 122
    return;
}

