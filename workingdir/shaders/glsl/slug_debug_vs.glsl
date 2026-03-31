#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 1500 0
struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    vec4  data_0[4];
};


#line 1500
struct SLANG_ParameterGroup_vertexBuffer_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 ProjectionMatrix_0;
};


#line 25 1
layout(binding = 0)
layout(std140) uniform block_SLANG_ParameterGroup_vertexBuffer_std140_0
{
    _MatrixStorage_float4x4_ColMajorstd140_0 ProjectionMatrix_0;
}vertexBuffer_0;

#line 25
mat4x4 unpackStorage_0(_MatrixStorage_float4x4_ColMajorstd140_0 _S1)
{

#line 25
    return mat4x4(_S1.data_0[0][0], _S1.data_0[1][0], _S1.data_0[2][0], _S1.data_0[3][0], _S1.data_0[0][1], _S1.data_0[1][1], _S1.data_0[2][1], _S1.data_0[3][1], _S1.data_0[0][2], _S1.data_0[1][2], _S1.data_0[2][2], _S1.data_0[3][2], _S1.data_0[0][3], _S1.data_0[1][3], _S1.data_0[2][3], _S1.data_0[3][3]);
}


#line 220
vec2 SlugDilate_0(vec4 pos_0, vec4 tex_0, vec4 jac_0, vec4 m0_0, vec4 m1_0, vec4 m3_0, vec2 dim_0, out vec2 vpos_0)
{
    vec2 _S2 = pos_0.zw;

#line 222
    vec2 n_0 = normalize(_S2);
    vec2 _S3 = m3_0.xy;

#line 223
    vec2 _S4 = pos_0.xy;

#line 223
    float s_0 = dot(_S3, _S4) + m3_0.w;
    float t_0 = dot(_S3, n_0);

    vec2 _S5 = m0_0.xy;

#line 226
    float u_0 = (s_0 * dot(_S5, n_0) - t_0 * (dot(_S5, _S4) + m0_0.w)) * dim_0.x;
    vec2 _S6 = m1_0.xy;

#line 227
    float v_0 = (s_0 * dot(_S6, n_0) - t_0 * (dot(_S6, _S4) + m1_0.w)) * dim_0.y;


    float st_0 = s_0 * t_0;
    float uv_0 = u_0 * u_0 + v_0 * v_0;
    vec2 d_0 = _S2 * (s_0 * s_0 * (st_0 + sqrt(uv_0)) / (uv_0 - st_0 * st_0));

    vpos_0 = _S4 + d_0;
    return vec2(tex_0.x + dot(d_0, jac_0.xy), tex_0.y + dot(d_0, jac_0.zw));
}


#line 212
void SlugUnpack_0(vec4 tex_1, vec4 bnd_0, out vec4 vbnd_0, out ivec4 vgly_0)
{
    uvec2 g_0 = floatBitsToUint(tex_1.zw);
    uint _S7 = g_0.x;

#line 215
    uint _S8 = g_0.y;

#line 215
    vgly_0 = ivec4(int(_S7 & 65535U), int(_S7 >> 16U), int(_S8 & 65535U), int(_S8 >> 16U));
    vbnd_0 = bnd_0;
    return;
}


#line 217
layout(location = 0)
out vec4 entryPointParam_main_vs_color_0;


#line 217
layout(location = 1)
out vec2 entryPointParam_main_vs_texcoord_0;


#line 217
flat layout(location = 2)
out vec4 entryPointParam_main_vs_banding_0;


#line 217
flat layout(location = 3)
out ivec4 entryPointParam_main_vs_glyph_0;


#line 217
layout(location = 0)
in vec4 input_pos_0;


#line 217
layout(location = 1)
in vec4 input_tex_0;


#line 217
layout(location = 2)
in vec4 input_jac_0;


#line 217
layout(location = 3)
in vec4 input_bnd_0;


#line 217
layout(location = 4)
in vec4 input_col_0;


#line 197
struct PS_INPUT_0
{
    vec4 position_0;
    vec4 color_0;
    vec2 texcoord_0;
    vec4 banding_0;
    ivec4 glyph_0;
};


#line 238
void main()
{
    PS_INPUT_0 output_0;

#line 247
    vec2 dilatedPos_0;
    vec2 _S9 = SlugDilate_0(input_pos_0, input_tex_0, input_jac_0, vec4(vertexBuffer_0.ProjectionMatrix_0.data_0[0][0], vertexBuffer_0.ProjectionMatrix_0.data_0[1][0], vertexBuffer_0.ProjectionMatrix_0.data_0[2][0], vertexBuffer_0.ProjectionMatrix_0.data_0[3][0]), vec4(vertexBuffer_0.ProjectionMatrix_0.data_0[0][1], vertexBuffer_0.ProjectionMatrix_0.data_0[1][1], vertexBuffer_0.ProjectionMatrix_0.data_0[2][1], vertexBuffer_0.ProjectionMatrix_0.data_0[3][1]), vec4(vertexBuffer_0.ProjectionMatrix_0.data_0[0][3], vertexBuffer_0.ProjectionMatrix_0.data_0[1][3], vertexBuffer_0.ProjectionMatrix_0.data_0[2][3], vertexBuffer_0.ProjectionMatrix_0.data_0[3][3]), vec2(2.0 / vertexBuffer_0.ProjectionMatrix_0.data_0[0][0], 2.0 / abs(vertexBuffer_0.ProjectionMatrix_0.data_0[1][1])), dilatedPos_0);

#line 248
    output_0.texcoord_0 = _S9;
    output_0.position_0 = (((vec4(dilatedPos_0, 0.0, 1.0)) * (unpackStorage_0(vertexBuffer_0.ProjectionMatrix_0))));

#line 255
    output_0.color_0 = input_col_0;

    SlugUnpack_0(input_tex_0, input_bnd_0, output_0.banding_0, output_0.glyph_0);
    PS_INPUT_0 _S10 = output_0;

#line 258
    gl_Position = output_0.position_0;

#line 258
    entryPointParam_main_vs_color_0 = _S10.color_0;

#line 258
    entryPointParam_main_vs_texcoord_0 = _S10.texcoord_0;

#line 258
    entryPointParam_main_vs_banding_0 = _S10.banding_0;

#line 258
    entryPointParam_main_vs_glyph_0 = _S10.glyph_0;

#line 258
    return;
}

