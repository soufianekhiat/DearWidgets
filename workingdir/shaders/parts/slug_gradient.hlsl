// Slug GPU font rendering, COLR v1 gradient variant (slug_gradient), written once as shader parts.
// Based on the reference implementation by Eric Lengyel.
// SPDX-License-Identifier: MIT OR Apache-2.0
// Copyright 2017, by Eric Lengyel.
//
// SlugGradientVertex: SlugVertex (parts/slug.hlsl) plus grd (gradient: dirX, dirY, scale, bias) and col2 (color 1);
// col is color 0. The gradient is evaluated in em space, the coverage as in parts/slug.hlsl.

//@inputs
float4 pos : POSITION;
float4 tex : TEXCOORD0;
float4 jac : TEXCOORD1;
float4 bnd : TEXCOORD2;
float4 col : COLOR0;
float4 grd : TEXCOORD3;
float4 col2 : COLOR1;

//@varyings
nointerpolation float4 banding;
nointerpolation int4 glyph;
nointerpolation float4 gradColor0;
nointerpolation float4 gradColor1;
nointerpolation float4 gradParams;  // dirX, dirY, scale, bias (radial: centerX, centerY, scale, bias)

//@textures
0 curveTexture
1 bandTexture

//@functions
//@include slug_core.inc

// Two gradient kinds share this variant, told apart by bit 24 of the packed glyph data (glyph.w bit 8), with the same
// (xy, scale, bias) layout:
//   linear: t = dot(em, dir) * scale + bias
//   radial: t = length(em - center) * scale + bias
// Center, radius and direction are pre-baked into the em space the band texture runs in.
float4 SlugGradientShade(float2 texcoord, float4 banding, int4 glyph, float4 color0, float4 color1, float4 params)
{
    float coverage = SlugRender(texcoord, banding, glyph);
    bool isRadial = (glyph.w & 0x100) != 0;
    float t = isRadial ? length(texcoord - params.xy) * params.z + params.w
                       : dot(texcoord, params.xy) * params.z + params.w;
    float4 gradColor = lerp(color0, color1, saturate(t));
    return float4(gradColor.rgb, gradColor.a * coverage);
}

//@vs
float4 m0 = IMP_MUL(float4(1.0, 0.0, 0.0, 0.0), ProjMtx);
float4 m1 = IMP_MUL(float4(0.0, 1.0, 0.0, 0.0), ProjMtx);
float4 m3 = IMP_MUL(float4(0.0, 0.0, 0.0, 1.0), ProjMtx);
float2 dim = float2(2.0f / m0.x, 2.0f / abs(m1.y));
float2 dilatedPos = float2(0.0, 0.0);
v.uv = SlugDilate(in_pos, in_tex, in_jac, m0, m1, m3, dim, dilatedPos);
position = IMP_MUL(ProjMtx, float4(dilatedPos, 0.0, 1.0));
v.gradColor0 = in_col;
v.gradColor1 = in_col2;
v.gradParams = in_grd;
SlugUnpack(in_tex, in_bnd, v.banding, v.glyph);

//@ps
col_out = SlugGradientShade(v.uv, v.banding, v.glyph, v.gradColor0, v.gradColor1, v.gradParams);
