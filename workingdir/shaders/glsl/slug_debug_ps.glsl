#version 450
#extension GL_EXT_samplerless_texture_functions : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 53 0
layout(binding = 1)
uniform texture2D bandTexture_0;


#line 48
layout(binding = 0)
uniform texture2D curveTexture_0;


#line 265
ivec2 BandLoad_0(ivec2 coord_0)
{
    ivec3 _S1 = ivec3(coord_0, 0);

#line 267
    vec4 raw_0 = (texelFetch((bandTexture_0), ((_S1)).xy, ((_S1)).z));
    return ivec2(int(raw_0.x + 0.5), int(raw_0.y + 0.5));
}


#line 276
ivec2 CalcBandLoc_0(ivec2 glyphLoc_0, uint offset_0)
{
    int _S2 = glyphLoc_0.x + int(offset_0);

#line 278
    ivec2 loc_0 = ivec2(_S2, glyphLoc_0.y);
    loc_0[1] = loc_0[1] + (_S2 >> 12);
    loc_0[0] = (loc_0[0]) & 4095;
    return loc_0;
}


#line 271
vec4 CurveLoad_0(ivec2 coord_1)
{
    ivec3 _S3 = ivec3(coord_1, 0);

#line 273
    return (texelFetch((curveTexture_0), ((_S3)).xy, ((_S3)).z));
}


#line 284
uint CalcRootCode_0(float y1_0, float y2_0, float y3_0)
{

#line 291
    return (11892U >> ((((floatBitsToUint(y3_0)) >> 29U) & 4U) | (((((floatBitsToUint(y2_0)) >> 30U) & 2U) | (((floatBitsToUint(y1_0)) >> 31U) & 4294967293U)) & 4294967291U))) & 257U;
}

vec2 SolveHorizPoly_0(vec4 p12_0, vec2 p3_0)
{
    vec2 _S4 = p12_0.xy;

#line 296
    vec2 _S5 = p12_0.zw;

#line 296
    vec2 a_0 = _S4 - _S5 * 2.0 + p3_0;
    vec2 b_0 = _S4 - _S5;
    float _S6 = a_0.y;

#line 298
    float ra_0 = 1.0 / _S6;
    float _S7 = b_0.y;

#line 299
    float rb_0 = 0.5 / _S7;
    float _S8 = p12_0.y;

#line 300
    float d_0 = sqrt(max(_S7 * _S7 - _S6 * _S8, 0.0));
    float _S9 = (_S7 - d_0) * ra_0;
    float _S10 = (_S7 + d_0) * ra_0;

#line 302
    float t1_0;

#line 302
    float t2_0;
    if((abs(_S6)) < 0.0000152587890625)
    {

#line 303
        float t2_1 = _S8 * rb_0;

#line 303
        t1_0 = t2_1;

#line 303
        t2_0 = t2_1;

#line 303
    }
    else
    {

#line 303
        t1_0 = _S9;

#line 303
        t2_0 = _S10;

#line 303
    }
    float _S11 = a_0.x;

#line 304
    float _S12 = b_0.x * 2.0;

#line 304
    float _S13 = p12_0.x;

#line 304
    return vec2((_S11 * t1_0 - _S12) * t1_0 + _S13, (_S11 * t2_0 - _S12) * t2_0 + _S13);
}


#line 12386 1
float saturate_0(float x_0)
{

#line 12394
    return clamp(x_0, 0.0, 1.0);
}


#line 308 0
vec2 SolveVertPoly_0(vec4 p12_1, vec2 p3_1)
{
    vec2 _S14 = p12_1.xy;

#line 310
    vec2 _S15 = p12_1.zw;

#line 310
    vec2 a_1 = _S14 - _S15 * 2.0 + p3_1;
    vec2 b_1 = _S14 - _S15;
    float _S16 = a_1.x;

#line 312
    float ra_1 = 1.0 / _S16;
    float _S17 = b_1.x;

#line 313
    float rb_1 = 0.5 / _S17;
    float _S18 = p12_1.x;

#line 314
    float d_1 = sqrt(max(_S17 * _S17 - _S16 * _S18, 0.0));
    float _S19 = (_S17 - d_1) * ra_1;
    float _S20 = (_S17 + d_1) * ra_1;

#line 316
    float t1_1;

#line 316
    float t2_2;
    if((abs(_S16)) < 0.0000152587890625)
    {

#line 317
        float t2_3 = _S18 * rb_1;

#line 317
        t1_1 = t2_3;

#line 317
        t2_2 = t2_3;

#line 317
    }
    else
    {

#line 317
        t1_1 = _S19;

#line 317
        t2_2 = _S20;

#line 317
    }
    float _S21 = a_1.y;

#line 318
    float _S22 = b_1.y * 2.0;

#line 318
    float _S23 = p12_1.y;

#line 318
    return vec2((_S21 * t1_1 - _S22) * t1_1 + _S23, (_S21 * t2_2 - _S22) * t2_2 + _S23);
}


float CalcCoverage_0(float xcov_0, float ycov_0, float xwgt_0, float ywgt_0)
{



    return saturate_0(max(abs(xcov_0 * xwgt_0 + ycov_0 * ywgt_0) / max(xwgt_0 + ywgt_0, 0.0000152587890625), min(abs(xcov_0), abs(ycov_0))));
}

float SlugRender_0(vec2 renderCoord_0, vec4 banding_0, ivec4 glyphData_0)
{

#line 330
    float ycov_1;

#line 330
    float ywgt_1;

#line 330
    float ycov_2;

#line 330
    float ywgt_2;


    vec2 _S24 = 1.0 / (abs(dFdx(renderCoord_0)) + abs(dFdy(renderCoord_0)));

    ivec2 glyphLoc_1 = glyphData_0.xy;
    ivec2 bandMax_0 = glyphData_0.zw;
    bandMax_0[1] = (bandMax_0[1]) & 255;

    ivec2 bandIndex_0 = clamp(ivec2(renderCoord_0 * banding_0.xy + banding_0.zw), ivec2(0, 0), bandMax_0);

#line 344
    int _S25 = glyphLoc_1.x;

#line 344
    int _S26 = glyphLoc_1.y;

#line 344
    ivec2 _S27 = BandLoad_0(ivec2(_S25 + bandIndex_0.y, _S26));

    int _S28 = _S27.x;

#line 377
    int _S29 = bandIndex_0.x;

#line 348
    uint _S30 = uint(_S27.y);

    vec4 _S31 = vec4(renderCoord_0, renderCoord_0);

#line 356
    float _S32 = _S24.x;

#line 389
    float _S33 = _S24.y;

#line 389
    float xwgt_1 = 0.0;

#line 389
    int ci_0 = 0;

#line 389
    float xcov_1 = 0.0;

#line 346
    for(;;)
    {

#line 346
        if(ci_0 < _S28)
        {
        }
        else
        {

#line 346
            break;
        }
        ivec2 ref_0 = BandLoad_0(CalcBandLoc_0(glyphLoc_1, _S30 + uint(ci_0)));
        int _S34 = (ref_0.x) & 4095;

#line 349
        int _S35 = ref_0.y;
        vec4 p12_2 = CurveLoad_0(ivec2(_S34, _S35)) - _S31;

        vec2 p3_2 = CurveLoad_0(ivec2(_S34 + 1, _S35)).xy - renderCoord_0;



        if((max(max(p12_2.x, p12_2.z), p3_2.x) * _S32) < -0.5)
        {

#line 356
            break;
        }
        uint code_0 = CalcRootCode_0(p12_2.y, p12_2.w, p3_2.y);
        if(code_0 != 0U)
        {
            vec2 r_0 = SolveHorizPoly_0(p12_2, p3_2) * _S32;
            if((code_0 & 1U) != 0U)
            {
                float _S36 = r_0.x;

#line 364
                float xcov_2 = xcov_1 + saturate_0(_S36 + 0.5);

#line 364
                ywgt_2 = max(xwgt_1, saturate_0(1.0 - abs(_S36) * 2.0));

#line 364
                ycov_2 = xcov_2;

#line 362
            }
            else
            {

#line 362
                ywgt_2 = xwgt_1;

#line 362
                ycov_2 = xcov_1;

#line 362
            }

#line 367
            if(code_0 > 1U)
            {
                float _S37 = r_0.y;

#line 369
                float xcov_3 = ycov_2 - saturate_0(_S37 + 0.5);

#line 369
                ywgt_1 = max(ywgt_2, saturate_0(1.0 - abs(_S37) * 2.0));

#line 369
                ycov_1 = xcov_3;

#line 367
            }
            else
            {

#line 367
                ywgt_1 = ywgt_2;

#line 367
                ycov_1 = ycov_2;

#line 367
            }

#line 367
            xwgt_1 = ywgt_1;

#line 367
            xcov_1 = ycov_1;

#line 359
        }

#line 346
        ci_0 = ci_0 + 1;

#line 346
    }

#line 377
    ivec2 _S38 = BandLoad_0(ivec2(_S25 + bandMax_0.y + 1 + _S29, _S26));

    int _S39 = _S38.x;

    uint _S40 = uint(_S38.y);

#line 381
    ywgt_2 = 0.0;

#line 381
    int ci2_0 = 0;

#line 381
    ycov_2 = 0.0;

#line 379
    for(;;)
    {

#line 379
        if(ci2_0 < _S39)
        {
        }
        else
        {

#line 379
            break;
        }
        ivec2 ref_1 = BandLoad_0(CalcBandLoc_0(glyphLoc_1, _S40 + uint(ci2_0)));
        int _S41 = (ref_1.x) & 4095;

#line 382
        int _S42 = ref_1.y;
        vec4 p12_3 = CurveLoad_0(ivec2(_S41, _S42)) - _S31;

        vec2 p3_3 = CurveLoad_0(ivec2(_S41 + 1, _S42)).xy - renderCoord_0;



        if((max(max(p12_3.y, p12_3.w), p3_3.y) * _S33) < -0.5)
        {

#line 389
            break;
        }
        uint code_1 = CalcRootCode_0(p12_3.x, p12_3.z, p3_3.x);
        if(code_1 != 0U)
        {
            vec2 r_1 = SolveVertPoly_0(p12_3, p3_3) * _S33;
            if((code_1 & 1U) != 0U)
            {
                float _S43 = r_1.x;

#line 397
                float ycov_3 = ycov_2 - saturate_0(_S43 + 0.5);

#line 397
                ywgt_1 = max(ywgt_2, saturate_0(1.0 - abs(_S43) * 2.0));

#line 397
                ycov_1 = ycov_3;

#line 395
            }
            else
            {

#line 395
                ywgt_1 = ywgt_2;

#line 395
                ycov_1 = ycov_2;

#line 395
            }

#line 395
            float ywgt_3;

#line 395
            float ycov_4;

#line 400
            if(code_1 > 1U)
            {
                float _S44 = r_1.y;

#line 402
                float ycov_5 = ycov_1 + saturate_0(_S44 + 0.5);

#line 402
                ywgt_3 = max(ywgt_1, saturate_0(1.0 - abs(_S44) * 2.0));

#line 402
                ycov_4 = ycov_5;

#line 400
            }
            else
            {

#line 400
                ywgt_3 = ywgt_1;

#line 400
                ycov_4 = ycov_1;

#line 400
            }

#line 400
            ywgt_2 = ywgt_3;

#line 400
            ycov_2 = ycov_4;

#line 392
        }

#line 379
        ci2_0 = ci2_0 + 1;

#line 379
    }

#line 408
    return CalcCoverage_0(xcov_1, ycov_2, xwgt_1, ywgt_2);
}



vec4 SlugRenderDebug_0(vec2 renderCoord_1, vec4 banding_1, ivec4 glyphData_1)
{


    float coverage_0 = SlugRender_0(renderCoord_1, banding_1, glyphData_1);
    return vec4(coverage_0, coverage_0, coverage_0, max(coverage_0, 0.25));
}


#line 418
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 418
layout(location = 1)
in vec2 input_texcoord_0;


#line 418
flat layout(location = 2)
in vec4 input_banding_0;


#line 418
flat layout(location = 3)
in ivec4 input_glyph_0;


void main()
{

#line 422
    entryPointParam_main_ps_0 = SlugRenderDebug_0(input_texcoord_0, input_banding_0, input_glyph_0);

#line 422
    return;
}

