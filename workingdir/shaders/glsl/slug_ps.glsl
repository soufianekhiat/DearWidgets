#version 450
#extension GL_EXT_samplerless_texture_functions : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 42 0
layout(binding = 1)
uniform texture2D bandTexture_0;


#line 37
layout(binding = 0)
uniform texture2D curveTexture_0;


#line 129
ivec2 BandLoad_0(ivec2 coord_0)
{
    ivec3 _S1 = ivec3(coord_0, 0);

#line 131
    vec4 raw_0 = (texelFetch((bandTexture_0), ((_S1)).xy, ((_S1)).z));
    return ivec2(int(raw_0.x + 0.5), int(raw_0.y + 0.5));
}


#line 140
ivec2 CalcBandLoc_0(ivec2 glyphLoc_0, uint offset_0)
{
    int _S2 = glyphLoc_0.x + int(offset_0);

#line 142
    ivec2 loc_0 = ivec2(_S2, glyphLoc_0.y);
    loc_0[1] = loc_0[1] + (_S2 >> 12);
    loc_0[0] = (loc_0[0]) & 4095;
    return loc_0;
}


#line 135
vec4 CurveLoad_0(ivec2 coord_1)
{
    ivec3 _S3 = ivec3(coord_1, 0);

#line 137
    return (texelFetch((curveTexture_0), ((_S3)).xy, ((_S3)).z));
}


#line 148
uint CalcRootCode_0(float y1_0, float y2_0, float y3_0)
{

#line 155
    return (11892U >> ((((floatBitsToUint(y3_0)) >> 29U) & 4U) | (((((floatBitsToUint(y2_0)) >> 30U) & 2U) | (((floatBitsToUint(y1_0)) >> 31U) & 4294967293U)) & 4294967291U))) & 257U;
}

vec2 SolveHorizPoly_0(vec4 p12_0, vec2 p3_0)
{
    vec2 _S4 = p12_0.xy;

#line 160
    vec2 _S5 = p12_0.zw;

#line 160
    vec2 a_0 = _S4 - _S5 * 2.0 + p3_0;
    vec2 b_0 = _S4 - _S5;
    float _S6 = a_0.y;

#line 162
    float ra_0 = 1.0 / _S6;
    float _S7 = b_0.y;

#line 163
    float rb_0 = 0.5 / _S7;
    float _S8 = p12_0.y;

#line 164
    float d_0 = sqrt(max(_S7 * _S7 - _S6 * _S8, 0.0));
    float _S9 = (_S7 - d_0) * ra_0;
    float _S10 = (_S7 + d_0) * ra_0;

#line 166
    float t1_0;

#line 166
    float t2_0;
    if((abs(_S6)) < 0.0000152587890625)
    {

#line 167
        float t2_1 = _S8 * rb_0;

#line 167
        t1_0 = t2_1;

#line 167
        t2_0 = t2_1;

#line 167
    }
    else
    {

#line 167
        t1_0 = _S9;

#line 167
        t2_0 = _S10;

#line 167
    }
    float _S11 = a_0.x;

#line 168
    float _S12 = b_0.x * 2.0;

#line 168
    float _S13 = p12_0.x;

#line 168
    return vec2((_S11 * t1_0 - _S12) * t1_0 + _S13, (_S11 * t2_0 - _S12) * t2_0 + _S13);
}


#line 12977 1
float saturate_0(float x_0)
{

#line 12985
    return clamp(x_0, 0.0, 1.0);
}


#line 172 0
vec2 SolveVertPoly_0(vec4 p12_1, vec2 p3_1)
{
    vec2 _S14 = p12_1.xy;

#line 174
    vec2 _S15 = p12_1.zw;

#line 174
    vec2 a_1 = _S14 - _S15 * 2.0 + p3_1;
    vec2 b_1 = _S14 - _S15;
    float _S16 = a_1.x;

#line 176
    float ra_1 = 1.0 / _S16;
    float _S17 = b_1.x;

#line 177
    float rb_1 = 0.5 / _S17;
    float _S18 = p12_1.x;

#line 178
    float d_1 = sqrt(max(_S17 * _S17 - _S16 * _S18, 0.0));
    float _S19 = (_S17 - d_1) * ra_1;
    float _S20 = (_S17 + d_1) * ra_1;

#line 180
    float t1_1;

#line 180
    float t2_2;
    if((abs(_S16)) < 0.0000152587890625)
    {

#line 181
        float t2_3 = _S18 * rb_1;

#line 181
        t1_1 = t2_3;

#line 181
        t2_2 = t2_3;

#line 181
    }
    else
    {

#line 181
        t1_1 = _S19;

#line 181
        t2_2 = _S20;

#line 181
    }
    float _S21 = a_1.y;

#line 182
    float _S22 = b_1.y * 2.0;

#line 182
    float _S23 = p12_1.y;

#line 182
    return vec2((_S21 * t1_1 - _S22) * t1_1 + _S23, (_S21 * t2_2 - _S22) * t2_2 + _S23);
}


float CalcCoverage_0(float xcov_0, float ycov_0, float xwgt_0, float ywgt_0, int flags_0)
{

#line 208
    return saturate_0(max(abs(xcov_0 * xwgt_0 + ycov_0 * ywgt_0) / max(xwgt_0 + ywgt_0, 0.0000152587890625), min(abs(xcov_0), abs(ycov_0))));
}

float SlugRender_0(vec2 renderCoord_0, vec4 banding_0, ivec4 glyphData_0)
{

#line 211
    float ycov_1;

#line 211
    float ywgt_1;

#line 211
    float ycov_2;

#line 211
    float ywgt_2;



    vec2 _S24 = 1.0 / (abs(dFdx(renderCoord_0)) + abs(dFdy(renderCoord_0)));

    ivec2 glyphLoc_1 = glyphData_0.xy;
    ivec2 bandMax_0 = glyphData_0.zw;
    bandMax_0[1] = (bandMax_0[1]) & 255;

    ivec2 bandIndex_0 = clamp(ivec2(renderCoord_0 * banding_0.xy + banding_0.zw), ivec2(0, 0), bandMax_0);

#line 226
    int _S25 = glyphLoc_1.x;

#line 226
    int _S26 = glyphLoc_1.y;

#line 226
    ivec2 hData_0 = BandLoad_0(ivec2(_S25 + bandIndex_0.y, _S26));
    ivec2 _S27 = CalcBandLoc_0(glyphLoc_1, uint(hData_0.y));

#line 227
    float xwgt_1 = 0.0;

#line 227
    int ci_0 = 0;

#line 227
    float xcov_1 = 0.0;

    for(;;)
    {

#line 229
        if(ci_0 < (hData_0.x))
        {
        }
        else
        {

#line 229
            break;
        }
        ivec2 ref_0 = BandLoad_0(ivec2(_S27.x + ci_0, _S27.y));
        int _S28 = ref_0.x;

#line 232
        int _S29 = ref_0.y;
        vec4 p12_2 = CurveLoad_0(ivec2(_S28, _S29)) - vec4(renderCoord_0, renderCoord_0);
        vec2 p3_2 = CurveLoad_0(ivec2(_S28 + 1, _S29)).xy - renderCoord_0;

        float _S30 = _S24.x;

#line 236
        if((max(max(p12_2.x, p12_2.z), p3_2.x) * _S30) < -0.5)
        {

#line 236
            break;
        }
        uint code_0 = CalcRootCode_0(p12_2.y, p12_2.w, p3_2.y);
        if(code_0 != 0U)
        {
            vec2 r_0 = SolveHorizPoly_0(p12_2, p3_2) * _S30;
            if((code_0 & 1U) != 0U)
            {
                float _S31 = r_0.x;

#line 244
                float xcov_2 = xcov_1 + saturate_0(_S31 + 0.5);

#line 244
                ywgt_2 = max(xwgt_1, saturate_0(1.0 - abs(_S31) * 2.0));

#line 244
                ycov_2 = xcov_2;

#line 242
            }
            else
            {

#line 242
                ywgt_2 = xwgt_1;

#line 242
                ycov_2 = xcov_1;

#line 242
            }

#line 247
            if(code_0 > 1U)
            {
                float _S32 = r_0.y;

#line 249
                float xcov_3 = ycov_2 - saturate_0(_S32 + 0.5);

#line 249
                ywgt_1 = max(ywgt_2, saturate_0(1.0 - abs(_S32) * 2.0));

#line 249
                ycov_1 = xcov_3;

#line 247
            }
            else
            {

#line 247
                ywgt_1 = ywgt_2;

#line 247
                ycov_1 = ycov_2;

#line 247
            }

#line 247
            xwgt_1 = ywgt_1;

#line 247
            xcov_1 = ycov_1;

#line 239
        }

#line 229
        ci_0 = ci_0 + 1;

#line 229
    }

#line 257
    ivec2 vData_0 = BandLoad_0(ivec2(_S25 + bandMax_0.y + 1 + bandIndex_0.x, _S26));
    ivec2 _S33 = CalcBandLoc_0(glyphLoc_1, uint(vData_0.y));

#line 258
    ywgt_2 = 0.0;

#line 258
    int ci2_0 = 0;

#line 258
    ycov_2 = 0.0;

    for(;;)
    {

#line 260
        if(ci2_0 < (vData_0.x))
        {
        }
        else
        {

#line 260
            break;
        }
        ivec2 ref_1 = BandLoad_0(ivec2(_S33.x + ci2_0, _S33.y));
        int _S34 = ref_1.x;

#line 263
        int _S35 = ref_1.y;
        vec4 p12_3 = CurveLoad_0(ivec2(_S34, _S35)) - vec4(renderCoord_0, renderCoord_0);
        vec2 p3_3 = CurveLoad_0(ivec2(_S34 + 1, _S35)).xy - renderCoord_0;

        float _S36 = _S24.y;

#line 267
        if((max(max(p12_3.y, p12_3.w), p3_3.y) * _S36) < -0.5)
        {

#line 267
            break;
        }
        uint code_1 = CalcRootCode_0(p12_3.x, p12_3.z, p3_3.x);
        if(code_1 != 0U)
        {
            vec2 r_1 = SolveVertPoly_0(p12_3, p3_3) * _S36;
            if((code_1 & 1U) != 0U)
            {
                float _S37 = r_1.x;

#line 275
                float ycov_3 = ycov_2 - saturate_0(_S37 + 0.5);

#line 275
                ywgt_1 = max(ywgt_2, saturate_0(1.0 - abs(_S37) * 2.0));

#line 275
                ycov_1 = ycov_3;

#line 273
            }
            else
            {

#line 273
                ywgt_1 = ywgt_2;

#line 273
                ycov_1 = ycov_2;

#line 273
            }

#line 273
            float ywgt_3;

#line 273
            float ycov_4;

#line 278
            if(code_1 > 1U)
            {
                float _S38 = r_1.y;

#line 280
                float ycov_5 = ycov_1 + saturate_0(_S38 + 0.5);

#line 280
                ywgt_3 = max(ywgt_1, saturate_0(1.0 - abs(_S38) * 2.0));

#line 280
                ycov_4 = ycov_5;

#line 278
            }
            else
            {

#line 278
                ywgt_3 = ywgt_1;

#line 278
                ycov_4 = ycov_1;

#line 278
            }

#line 278
            ywgt_2 = ywgt_3;

#line 278
            ycov_2 = ycov_4;

#line 270
        }

#line 260
        ci2_0 = ci2_0 + 1;

#line 260
    }

#line 286
    return CalcCoverage_0(xcov_1, ycov_2, xwgt_1, ywgt_2, glyphData_0.w);
}


#line 286
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 286
layout(location = 0)
in vec4 input_color_0;


#line 286
layout(location = 1)
in vec2 input_texcoord_0;


#line 286
flat layout(location = 2)
in vec4 input_banding_0;


#line 286
flat layout(location = 3)
in ivec4 input_glyph_0;

void main()
{

#line 289
    entryPointParam_main_ps_0 = input_color_0 * SlugRender_0(input_texcoord_0, input_banding_0, input_glyph_0);

#line 289
    return;
}

