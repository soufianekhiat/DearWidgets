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


#line 39
struct SLANG_ParameterGroup_fillParams_std140_0
{
    vec4 fillColor0_0;
    vec4 fillColor1_0;
    vec4 fillBBox_0;
    vec4 fillGrad_0;
    vec4 fillUVStart_0;
    vec4 fillUVEnd_0;
};


#line 32
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_fillParams_std140_0
{
    vec4 fillColor0_0;
    vec4 fillColor1_0;
    vec4 fillBBox_0;
    vec4 fillGrad_0;
    vec4 fillUVStart_0;
    vec4 fillUVEnd_0;
}fillParams_0;

#line 57
layout(binding = 2)
uniform texture2D fillTexture_0;


#line 58
layout(binding = 0)
uniform sampler fillSampler_0;


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


#line 64
float sRGBToLinearCh_0(float x_1)
{

#line 64
    float _S45;
    if(x_1 <= 0.04044999927282333)
    {

#line 65
        _S45 = x_1 / 12.92000007629394531;

#line 65
    }
    else
    {

#line 65
        _S45 = pow((x_1 + 0.05499999970197678) / 1.0549999475479126, 2.40000009536743164);

#line 65
    }

#line 65
    return _S45;
}



vec3 sRGBToLinear3_0(vec3 c_0)
{

#line 71
    return vec3(sRGBToLinearCh_0(c_0.x), sRGBToLinearCh_0(c_0.y), sRGBToLinearCh_0(c_0.z));
}




vec3 sRGBToOkLab_0(vec3 c_1)
{

#line 78
    vec3 lin_0 = sRGBToLinear3_0(c_1);
    float _S46 = lin_0.x;

#line 79
    float _S47 = lin_0.y;

#line 79
    float _S48 = lin_0.z;

#line 79
    float l_0 = 0.41222146153450012 * _S46 + 0.53633254766464233 * _S47 + 0.05144599452614784 * _S48;
    float m_0 = 0.21190349757671356 * _S46 + 0.68069952726364136 * _S47 + 0.10739696025848389 * _S48;
    float s_0 = 0.08830246329307556 * _S46 + 0.28171885013580322 * _S47 + 0.6299787163734436 * _S48;
    float l_1 = float((int(sign((l_0))))) * pow(abs(l_0), 0.3333333432674408);
    float m_1 = float((int(sign((m_0))))) * pow(abs(m_0), 0.3333333432674408);
    float s_1 = float((int(sign((s_0))))) * pow(abs(s_0), 0.3333333432674408);
    return vec3(l_1 * 0.21045425534248352 + m_1 * 0.79361778497695923 + s_1 * -0.00407204683870077, l_1 * 1.97799849510192871 + m_1 * -2.42859220504760742 + s_1 * 0.45059370994567871, l_1 * 0.02590403705835342 + m_1 * 0.7827717661857605 + s_1 * -0.80867576599121094);
}


#line 102
vec3 sRGBToOkLch_0(vec3 c_2)
{

#line 103
    vec3 lab_0 = sRGBToOkLab_0(c_2);
    float _S49 = lab_0.y;

#line 104
    float _S50 = lab_0.z;

#line 104
    float C_0 = sqrt(_S49 * _S49 + _S50 * _S50);
    float h_0 = (atan((_S50),(_S49)));

#line 105
    float h_1;
    if(h_0 < 0.0)
    {

#line 106
        h_1 = h_0 + 6.28318548202514648;

#line 106
    }
    else
    {

#line 106
        h_1 = h_0;

#line 106
    }

    return vec3(lab_0.x, C_0, h_1 / 6.28318548202514648);
}


#line 116
vec3 sRGBToHSV_0(vec3 c_3)
{
    float r_2 = c_3.x;

#line 118
    float g_0 = c_3.y;

#line 118
    float b_2 = c_3.z;

#line 118
    float g_1;

#line 118
    float K_0;

#line 118
    float b_3;
    if(g_0 < b_2)
    {

#line 119
        g_1 = b_2;

#line 119
        K_0 = -1.0;

#line 119
        b_3 = g_0;

#line 119
    }
    else
    {

#line 119
        g_1 = g_0;

#line 119
        K_0 = 0.0;

#line 119
        b_3 = b_2;

#line 119
    }

#line 119
    float r_3;
    if(r_2 < g_1)
    {

#line 120
        float _S51 = -0.3333333432674408 - K_0;

#line 120
        r_3 = g_1;

#line 120
        g_1 = r_2;

#line 120
        K_0 = _S51;

#line 120
    }
    else
    {

#line 120
        r_3 = r_2;

#line 120
    }
    float chroma_0 = r_3 - min(g_1, b_3);


    return vec3(abs(K_0 + (g_1 - b_3) / (6.0 * chroma_0 + 9.99999968265522539e-21)), chroma_0 / (r_3 + 9.99999968265522539e-21), r_3);
}


#line 67
float linearToSRGBCh_0(float x_2)
{

#line 67
    float _S52;
    if(x_2 <= 0.00313080009073019)
    {

#line 68
        _S52 = 12.92000007629394531 * x_2;

#line 68
    }
    else
    {

#line 68
        _S52 = 1.0549999475479126 * pow(x_2, 0.4166666567325592) - 0.05499999970197678;

#line 68
    }

#line 68
    return _S52;
}


#line 12401 1
vec3 saturate_1(vec3 x_3)
{

#line 12409
    return clamp(x_3, vec3(0.0), vec3(1.0));
}


#line 73 0
vec3 linearToSRGB3_0(vec3 c_4)
{

#line 74
    return saturate_1(vec3(linearToSRGBCh_0(c_4.x), linearToSRGBCh_0(c_4.y), linearToSRGBCh_0(c_4.z)));
}


#line 90
vec3 okLabToSRGB_0(vec3 Lab_0)
{

#line 91
    float _S53 = Lab_0.x;

#line 91
    float _S54 = Lab_0.y;

#line 91
    float _S55 = Lab_0.z;

#line 91
    float l_2 = _S53 + _S54 * 0.39633777737617493 + _S55 * 0.21580375730991364;
    float m_2 = _S53 + _S54 * -0.10556134581565857 + _S55 * -0.06385417282581329;
    float s_2 = _S53 + _S54 * -0.08948417752981186 + _S55 * -1.29148554801940918;
    float l_3 = l_2 * l_2 * l_2;

#line 94
    float m_3 = m_2 * m_2 * m_2;

#line 94
    float s_3 = s_2 * s_2 * s_2;

#line 99
    return linearToSRGB3_0(vec3(l_3 * 4.07674169540405273 + m_3 * -3.30771160125732422 + s_3 * 0.23096993565559387, l_3 * -1.26843798160552979 + m_3 * 2.60975742340087891 + s_3 * -0.34131938219070435, l_3 * -0.0041960864327848 + m_3 * -0.70341861248016357 + s_3 * 1.70761466026306152));
}


#line 110
vec3 okLchToSRGB_0(vec3 lch_0)
{

#line 111
    float _S56 = lch_0.y;

#line 111
    float _S57 = lch_0.z * 6.28318548202514648;

    return okLabToSRGB_0(vec3(lch_0.x, _S56 * cos(_S57), _S56 * sin(_S57)));
}


#line 126
vec3 hsvToSRGB_0(vec3 c_5)
{

#line 127
    float h_2 = c_5.x;

#line 127
    float s_4 = c_5.y;

#line 127
    float v_0 = c_5.z;
    if(s_4 < 9.99999997475242708e-07)
    {

#line 128
        return vec3(v_0, v_0, v_0);
    }

#line 129
    float h_3 = ((((h_2) < 0.0) ? -mod(-(h_2),abs((1.0))) : mod((h_2),abs((1.0)))));

#line 129
    float h_4;

#line 129
    if(h_3 < 0.0)
    {

#line 129
        h_4 = h_3 + 1.0;

#line 129
    }
    else
    {

#line 129
        h_4 = h_3;

#line 129
    }
    float h_5 = h_4 * 6.0;
    int i_0 = int(floor(h_5));
    float f_0 = h_5 - float(i_0);
    float p_0 = v_0 * (1.0 - s_4);
    float q_0 = v_0 * (1.0 - s_4 * f_0);
    float t_0 = v_0 * (1.0 - s_4 * (1.0 - f_0));
    if(i_0 == 0)
    {

#line 136
        return vec3(v_0, t_0, p_0);
    }

#line 137
    if(i_0 == 1)
    {

#line 137
        return vec3(q_0, v_0, p_0);
    }

#line 138
    if(i_0 == 2)
    {

#line 138
        return vec3(p_0, v_0, t_0);
    }

#line 139
    if(i_0 == 3)
    {

#line 139
        return vec3(p_0, q_0, v_0);
    }

#line 140
    if(i_0 == 4)
    {

#line 140
        return vec3(t_0, p_0, v_0);
    }

#line 141
    return vec3(v_0, p_0, q_0);
}


vec4 lerpInColorSpace_0(vec4 c0_0, vec4 c1_0, float ft_0, float space_0)
{

#line 146
    vec3 a_2 = c0_0.xyz;

#line 146
    vec3 b_4 = c1_0.xyz;
    bool _S58 = space_0 < 0.5;

#line 147
    vec3 a_3;

#line 147
    vec3 b_5;

#line 147
    if(_S58)
    {

#line 147
        a_3 = a_2;

#line 147
        b_5 = b_4;

#line 147
    }
    else
    {

#line 148
        if(space_0 < 1.5)
        {

#line 148
            vec3 _S59 = sRGBToLinear3_0(b_4);

#line 148
            a_3 = sRGBToLinear3_0(a_2);

#line 148
            b_5 = _S59;

#line 148
        }
        else
        {

#line 149
            if(space_0 < 2.5)
            {

#line 149
                vec3 _S60 = sRGBToOkLab_0(b_4);

#line 149
                a_3 = sRGBToOkLab_0(a_2);

#line 149
                b_5 = _S60;

#line 149
            }
            else
            {

#line 150
                if(space_0 < 3.5)
                {

#line 150
                    vec3 _S61 = sRGBToOkLch_0(b_4);

#line 150
                    a_3 = sRGBToOkLch_0(a_2);

#line 150
                    b_5 = _S61;

#line 150
                }
                else
                {

#line 151
                    vec3 _S62 = sRGBToHSV_0(b_4);

#line 151
                    a_3 = sRGBToHSV_0(a_2);

#line 151
                    b_5 = _S62;

#line 150
                }

#line 149
            }

#line 148
        }

#line 147
    }

#line 152
    vec3 r_4 = mix(a_3, b_5, vec3(ft_0));

#line 152
    vec3 r_5;
    if(_S58)
    {

#line 153
        r_5 = r_4;

#line 153
    }
    else
    {

#line 154
        if(space_0 < 1.5)
        {

#line 154
            r_5 = linearToSRGB3_0(r_4);

#line 154
        }
        else
        {

#line 155
            if(space_0 < 2.5)
            {

#line 155
                r_5 = okLabToSRGB_0(r_4);

#line 155
            }
            else
            {

#line 156
                if(space_0 < 3.5)
                {

#line 156
                    r_5 = okLchToSRGB_0(r_4);

#line 156
                }
                else
                {

#line 156
                    r_5 = hsvToSRGB_0(r_4);

#line 156
                }

#line 155
            }

#line 154
        }

#line 153
    }

#line 158
    return vec4(saturate_1(r_5), mix(c0_0.w, c1_0.w, ft_0));
}


#line 1779 1
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 1779
layout(location = 0)
in vec4 input_color_0;


#line 1779
layout(location = 1)
in vec2 input_texcoord_0;


#line 1779
flat layout(location = 2)
in vec4 input_banding_0;


#line 1779
flat layout(location = 3)
in ivec4 input_glyph_0;


#line 422 0
void main()
{
    float coverage_0 = SlugRender_0(input_texcoord_0, input_banding_0, input_glyph_0);

#line 456
    vec2 _S63 = input_color_0.xy;
    vec2 uv_0 = (gl_FragCoord.xy - _S63) / max(input_color_0.zw - _S63, vec2(1.0, 1.0));

    if((fillParams_0.fillGrad_0.x) < 2.5)
    {

#line 459
        float ft_1;


        if((fillParams_0.fillGrad_0.x) < 0.5)
        {

#line 463
            vec2 dir_0 = fillParams_0.fillUVEnd_0.xy - fillParams_0.fillUVStart_0.xy;
            float denom_0 = dot(dir_0, dir_0);
            if(denom_0 > 9.99999993922529029e-09)
            {

#line 465
                ft_1 = dot(uv_0 - fillParams_0.fillUVStart_0.xy, dir_0) / denom_0;

#line 465
            }
            else
            {

#line 465
                ft_1 = 0.0;

#line 465
            }

#line 462
        }
        else
        {

            if((fillParams_0.fillGrad_0.x) < 1.5)
            {

#line 466
                ft_1 = length(uv_0 - fillParams_0.fillUVStart_0.xy) / max(length(fillParams_0.fillUVEnd_0.xy - fillParams_0.fillUVStart_0.xy), 0.00000999999974738);

#line 466
            }
            else
            {


                vec2 d_2 = (uv_0 - fillParams_0.fillUVStart_0.xy) / max(length(fillParams_0.fillUVEnd_0.xy - fillParams_0.fillUVStart_0.xy), 0.00000999999974738);

#line 471
                ft_1 = abs(d_2.x) + abs(d_2.y);

#line 466
            }

#line 462
        }

#line 475
        vec4 fillCol_0 = lerpInColorSpace_0(fillParams_0.fillColor0_0, fillParams_0.fillColor1_0, saturate_0(ft_1), fillParams_0.fillGrad_0.y);

#line 475
        entryPointParam_main_ps_0 = vec4(fillCol_0.xyz, fillCol_0.w * coverage_0);

#line 475
        return;
    }
    else
    {

#line 485
        vec4 texColor_0 = (texture(sampler2D(fillTexture_0,fillSampler_0), (uv_0 * fillParams_0.fillUVEnd_0.xy + fillParams_0.fillUVStart_0.xy)));

#line 485
        entryPointParam_main_ps_0 = vec4(mix(fillParams_0.fillColor0_0.xyz, texColor_0.xyz * fillParams_0.fillColor0_0.xyz, vec3(texColor_0.w)), fillParams_0.fillColor0_0.w * coverage_0);

#line 485
        return;
    }

#line 485
}

