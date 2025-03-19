#ifndef __IM_SHADER_H__
#define __IM_SHADER_H__

#if defined(IM_SHADER_HLSL)
#define IMS_IN        in
#define IMS_OUT       out
#define IMS_INOUT     inout
#define IMS_UNIFORM   uniform
#define IMS_CBUFFER   cbuffer
#elif defined(IM_SHADER_GLSL)
#define IMS_IN        in
#define IMS_OUT       out
#define IMS_INOUT     inout
#define IMS_UNIFORM   const
#define IMS_CBUFFER   uniform
#endif

#if defined(IM_SHADER_HLSL)

#define Mat44f matrix<float, 4, 4>
#define Mat33f matrix<float, 3, 3>

#endif

#if defined(IM_SHADER_GLSL)

#define Mat44f   mat4
#define Mat33f   mat3

#define float2   vec2
#define float3   vec3
#define float4   vec4
#define uint2    uvec2
#define uint3    uvec3
#define uint4    uvec4
#define int2     ivec2
#define int3     ivec3
#define int4     ivec4

#define float4x4 mat4
#define float3x3 mat3
#define float2x2 mat2

#endif
#endif
IMS_CBUFFER vertexBuffer
{
	float4x4 ProjectionMatrix;
};
struct VS_INPUT
{
	float2 pos : POSITION;
	float4 col : COLOR0;
	float2 uv  : TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 col : COLOR0;
	float2 uv  : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));
	output.col = input.col;
	output.uv  = input.uv;
	return output;
}

