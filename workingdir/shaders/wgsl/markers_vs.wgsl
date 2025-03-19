struct _MatrixStorage_float4x4_ColMajorstd140_0
{
    @align(16) data_0 : array<vec4<f32>, i32(4)>,
};

struct SLANG_ParameterGroup_vertexBuffer_std140_0
{
    @align(16) ProjectionMatrix_0 : _MatrixStorage_float4x4_ColMajorstd140_0,
};

@binding(1) @group(0) var<uniform> vertexBuffer_0 : SLANG_ParameterGroup_vertexBuffer_std140_0;
fn unpackStorage_0( _S1 : _MatrixStorage_float4x4_ColMajorstd140_0) -> mat4x4<f32>
{
    return mat4x4<f32>(_S1.data_0[i32(0)][i32(0)], _S1.data_0[i32(1)][i32(0)], _S1.data_0[i32(2)][i32(0)], _S1.data_0[i32(3)][i32(0)], _S1.data_0[i32(0)][i32(1)], _S1.data_0[i32(1)][i32(1)], _S1.data_0[i32(2)][i32(1)], _S1.data_0[i32(3)][i32(1)], _S1.data_0[i32(0)][i32(2)], _S1.data_0[i32(1)][i32(2)], _S1.data_0[i32(2)][i32(2)], _S1.data_0[i32(3)][i32(2)], _S1.data_0[i32(0)][i32(3)], _S1.data_0[i32(1)][i32(3)], _S1.data_0[i32(2)][i32(3)], _S1.data_0[i32(3)][i32(3)]);
}

struct PS_INPUT_0
{
    @builtin(position) pos_0 : vec4<f32>,
    @location(0) col_0 : vec4<f32>,
    @location(1) uv_0 : vec2<f32>,
};

struct vertexInput_0
{
    @location(0) pos_1 : vec2<f32>,
    @location(1) col_1 : vec4<f32>,
    @location(2) uv_1 : vec2<f32>,
};

@vertex
fn main_vs( _S2 : vertexInput_0) -> PS_INPUT_0
{
    var output_0 : PS_INPUT_0;
    output_0.pos_0 = (((vec4<f32>(_S2.pos_1.xy, 0.0f, 1.0f)) * (unpackStorage_0(vertexBuffer_0.ProjectionMatrix_0))));
    output_0.col_0 = _S2.col_1;
    output_0.uv_0 = _S2.uv_1;
    return output_0;
}

