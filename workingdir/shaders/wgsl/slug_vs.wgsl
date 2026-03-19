struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct SLANG_ParameterGroup_vertexBuffer_std140_0
{
    @align(16) ProjectionMatrix_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
};

@binding(0) @group(0) var<uniform> vertexBuffer_0 : SLANG_ParameterGroup_vertexBuffer_std140_0;
fn SlugDilate_0( pos_0 : vec4<f32>,  tex_0 : vec4<f32>,  jac_0 : vec4<f32>,  m0_0 : vec4<f32>,  m1_0 : vec4<f32>,  m3_0 : vec4<f32>,  dim_0 : vec2<f32>,  vpos_0 : ptr<function, vec2<f32>>) -> vec2<f32>
{
    var _S1 : vec2<f32> = pos_0.zw;
    var n_0 : vec2<f32> = normalize(_S1);
    var _S2 : vec2<f32> = m3_0.xy;
    var _S3 : vec2<f32> = pos_0.xy;
    var s_0 : f32 = dot(_S2, _S3) + m3_0.w;
    var t_0 : f32 = dot(_S2, n_0);
    var _S4 : vec2<f32> = m0_0.xy;
    var u_0 : f32 = (s_0 * dot(_S4, n_0) - t_0 * (dot(_S4, _S3) + m0_0.w)) * dim_0.x;
    var _S5 : vec2<f32> = m1_0.xy;
    var v_0 : f32 = (s_0 * dot(_S5, n_0) - t_0 * (dot(_S5, _S3) + m1_0.w)) * dim_0.y;
    var st_0 : f32 = s_0 * t_0;
    var uv_0 : f32 = u_0 * u_0 + v_0 * v_0;
    var d_0 : vec2<f32> = _S1 * vec2<f32>((s_0 * s_0 * (st_0 + sqrt(uv_0)) / (uv_0 - st_0 * st_0)));
    (*vpos_0) = _S3 + d_0;
    return vec2<f32>(tex_0.x + dot(d_0, jac_0.xy), tex_0.y + dot(d_0, jac_0.zw));
}

fn SlugUnpack_0( tex_1 : vec4<f32>,  bnd_0 : vec4<f32>,  vbnd_0 : ptr<function, vec4<f32>>,  vgly_0 : ptr<function, vec4<i32>>)
{
    var g_0 : vec2<u32> = (bitcast<vec2<u32>>((tex_1.zw)));
    var _S6 : u32 = g_0.x;
    var _S7 : u32 = g_0.y;
    (*vgly_0) = vec4<i32>(i32((_S6 & (u32(65535)))), i32((_S6 >> (u32(16)))), i32((_S7 & (u32(65535)))), i32((_S7 >> (u32(16)))));
    (*vbnd_0) = bnd_0;
    return;
}

struct PS_INPUT_0
{
    @builtin(position) position_0 : vec4<f32>,
    @location(0) color_0 : vec4<f32>,
    @location(3) texcoord_0 : vec2<f32>,
    @interpolate(flat) @location(1) banding_0 : vec4<f32>,
    @interpolate(flat) @location(2) glyph_0 : vec4<i32>,
};

struct vertexInput_0
{
    @location(0) pos_1 : vec4<f32>,
    @location(3) tex_2 : vec4<f32>,
    @location(1) jac_1 : vec4<f32>,
    @location(2) bnd_1 : vec4<f32>,
    @location(4) col_0 : vec4<f32>,
};

@vertex
fn main_vs( _S8 : vertexInput_0) -> PS_INPUT_0
{
    var output_0 : PS_INPUT_0;
    var dilatedPos_0 : vec2<f32>;
    var _S9 : vec2<f32> = SlugDilate_0(_S8.pos_1, _S8.tex_2, _S8.jac_1, vec4<f32>(vertexBuffer_0.ProjectionMatrix_0.data_0[i32(0)][i32(0)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(1)][i32(0)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(2)][i32(0)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(3)][i32(0)]), vec4<f32>(vertexBuffer_0.ProjectionMatrix_0.data_0[i32(0)][i32(1)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(1)][i32(1)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(2)][i32(1)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(3)][i32(1)]), vec4<f32>(vertexBuffer_0.ProjectionMatrix_0.data_0[i32(0)][i32(3)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(1)][i32(3)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(2)][i32(3)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(3)][i32(3)]), vec2<f32>(2.0f / vertexBuffer_0.ProjectionMatrix_0.data_0[i32(0)][i32(0)], 2.0f / abs(vertexBuffer_0.ProjectionMatrix_0.data_0[i32(1)][i32(1)])), &(dilatedPos_0));
    output_0.texcoord_0 = _S9;
    output_0.position_0 = (((vec4<f32>(dilatedPos_0, 0.0f, 1.0f)) * (mat4x4<f32>(vertexBuffer_0.ProjectionMatrix_0.data_0[i32(0)][i32(0)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(1)][i32(0)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(2)][i32(0)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(3)][i32(0)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(0)][i32(1)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(1)][i32(1)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(2)][i32(1)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(3)][i32(1)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(0)][i32(2)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(1)][i32(2)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(2)][i32(2)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(3)][i32(2)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(0)][i32(3)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(1)][i32(3)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(2)][i32(3)], vertexBuffer_0.ProjectionMatrix_0.data_0[i32(3)][i32(3)]))));
    output_0.color_0 = _S8.col_0;
    var _S10 : vec4<f32> = output_0.banding_0;
    var _S11 : vec4<i32> = output_0.glyph_0;
    SlugUnpack_0(_S8.tex_2, _S8.bnd_1, &(_S10), &(_S11));
    output_0.banding_0 = _S10;
    output_0.glyph_0 = _S11;
    return output_0;
}

