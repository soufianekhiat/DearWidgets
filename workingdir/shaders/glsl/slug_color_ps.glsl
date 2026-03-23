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


#line 188 0
vec3 SolveCubicRoots_0(float a_1, float b_1, float c_0, float d_1)
{

    if((abs(a_1)) < 0.0000152587890625)
    {
        if((abs(b_1)) < 0.0000152587890625)
        {
            if((abs(c_0)) < 0.0000152587890625)
            {

#line 195
                return vec3(1.0e+09, 1.0e+09, 1.0e+09);
            }

#line 196
            return vec3(- d_1 / c_0, 1.0e+09, 1.0e+09);
        }
        float disc2_0 = c_0 * c_0 - 4.0 * b_1 * d_1;
        if(disc2_0 < 0.0)
        {

#line 199
            return vec3(1.0e+09, 1.0e+09, 1.0e+09);
        }

#line 200
        float sq_0 = sqrt(disc2_0);

#line 200
        float inv2b_0 = 0.5 / b_1;
        float _S14 = - c_0;

#line 201
        return vec3((_S14 - sq_0) * inv2b_0, (_S14 + sq_0) * inv2b_0, 1.0e+09);
    }

    float inv_a_0 = 1.0 / a_1;
    float B_0 = b_1 * inv_a_0;

#line 205
    float C_0 = c_0 * inv_a_0;

    float shift_0 = - B_0 / 3.0;
    float p_0 = C_0 - B_0 * B_0 / 3.0;
    float q_0 = d_1 * inv_a_0 + B_0 * (2.0 * B_0 * B_0 - 9.0 * C_0) / 27.0;
    float disc_0 = - (4.0 * p_0 * p_0 * p_0 + 27.0 * q_0 * q_0);
    if(disc_0 >= 0.0)
    {


        float m_0 = 2.0 * sqrt(max(- p_0 / 3.0, 0.0));

#line 215
        float arg_0;
        if(m_0 > 1.00000001168609742e-07)
        {

#line 216
            arg_0 = clamp(3.0 * q_0 / (p_0 * m_0), -1.0, 1.0);

#line 216
        }
        else
        {

#line 216
            arg_0 = 0.0;

#line 216
        }
        float phi_0 = acos(arg_0) / 3.0;
        return vec3(m_0 * cos(phi_0) + shift_0, m_0 * cos(phi_0 - 2.09439516067504883) + shift_0, m_0 * cos(phi_0 - 4.18879032135009766) + shift_0);
    }
    else
    {



        float sq_1 = sqrt(max(- disc_0 / 108.0, 0.0));
        float hq_0 = - q_0 * 0.5;
        float _S15 = hq_0 + sq_1;
        float _S16 = hq_0 - sq_1;
        return vec3(float((int(sign((_S15))))) * pow(abs(_S15), 0.3333333432674408) + float((int(sign((_S16))))) * pow(abs(_S16), 0.3333333432674408) + shift_0, 1.0e+09, 1.0e+09);
    }

#line 229
}


#line 236
void ApplyCubicRoot_0(float t_0, float x_em_0, float pxPerEm_0, float deriv_0, float sign_pos_0, inout float cov_0, inout float wgt_0)
{

#line 237
    bool _S17;

    if(t_0 < 0.0)
    {

#line 239
        _S17 = true;

#line 239
    }
    else
    {

#line 239
        _S17 = t_0 >= 1.0;

#line 239
    }

#line 239
    if(_S17)
    {

#line 239
        return;
    }

#line 240
    float r_0 = x_em_0 * pxPerEm_0;
    float contrib_0 = saturate_0(r_0 + 0.5);
    float w_0 = saturate_0(1.0 - abs(r_0) * 2.0);
    if(deriv_0 > 0.0)
    {

#line 243
        cov_0 = cov_0 + sign_pos_0 * contrib_0;

#line 243
        wgt_0 = max(wgt_0, w_0);

#line 243
    }
    else
    {

#line 244
        if(deriv_0 < 0.0)
        {

#line 244
            cov_0 = cov_0 - sign_pos_0 * contrib_0;

#line 244
            wgt_0 = max(wgt_0, w_0);

#line 244
        }

#line 243
    }

    return;
}


#line 172
vec2 SolveVertPoly_0(vec4 p12_1, vec2 p3_1)
{
    vec2 _S18 = p12_1.xy;

#line 174
    vec2 _S19 = p12_1.zw;

#line 174
    vec2 a_2 = _S18 - _S19 * 2.0 + p3_1;
    vec2 b_2 = _S18 - _S19;
    float _S20 = a_2.x;

#line 176
    float ra_1 = 1.0 / _S20;
    float _S21 = b_2.x;

#line 177
    float rb_1 = 0.5 / _S21;
    float _S22 = p12_1.x;

#line 178
    float d_2 = sqrt(max(_S21 * _S21 - _S20 * _S22, 0.0));
    float _S23 = (_S21 - d_2) * ra_1;
    float _S24 = (_S21 + d_2) * ra_1;

#line 180
    float t1_1;

#line 180
    float t2_2;
    if((abs(_S20)) < 0.0000152587890625)
    {

#line 181
        float t2_3 = _S22 * rb_1;

#line 181
        t1_1 = t2_3;

#line 181
        t2_2 = t2_3;

#line 181
    }
    else
    {

#line 181
        t1_1 = _S23;

#line 181
        t2_2 = _S24;

#line 181
    }
    float _S25 = a_2.y;

#line 182
    float _S26 = b_2.y * 2.0;

#line 182
    float _S27 = p12_1.y;

#line 182
    return vec2((_S25 * t1_1 - _S26) * t1_1 + _S27, (_S25 * t2_2 - _S26) * t2_2 + _S27);
}


#line 247
float CalcCoverage_0(float xcov_0, float ycov_0, float xwgt_0, float ywgt_0, int flags_0)
{

#line 269
    return saturate_0(max(abs(xcov_0 * xwgt_0 + ycov_0 * ywgt_0) / max(xwgt_0 + ywgt_0, 0.0000152587890625), min(abs(xcov_0), abs(ycov_0))));
}

float SlugRender_0(vec2 renderCoord_0, vec4 banding_0, ivec4 glyphData_0)
{

#line 272
    float maxY_0;



    vec2 _S28 = 1.0 / (abs(dFdx(renderCoord_0)) + abs(dFdy(renderCoord_0)));

    ivec2 glyphLoc_1 = glyphData_0.xy;
    ivec2 bandMax_0 = glyphData_0.zw;
    bandMax_0[1] = (bandMax_0[1]) & 255;

    ivec2 bandIndex_0 = clamp(ivec2(renderCoord_0 * banding_0.xy + banding_0.zw), ivec2(0, 0), bandMax_0);



    float xcov_1 = 0.0;

#line 286
    float xwgt_1 = 0.0;
    int _S29 = glyphLoc_1.x;

#line 287
    int _S30 = glyphLoc_1.y;

#line 287
    ivec2 _S31 = BandLoad_0(ivec2(_S29 + bandIndex_0.y, _S30));

#line 287
    int ci_0 = 0;

    for(;;)
    {

#line 289
        if(ci_0 < (_S31.x))
        {
        }
        else
        {

#line 289
            break;
        }

#line 309
        ivec2 ref_0 = BandLoad_0(CalcBandLoc_0(glyphLoc_1, uint(_S31.y) + uint(ci_0)));
        int _S32 = ref_0.x;

#line 310
        bool isCubic_0 = (_S32 & 4096) != 0;
        int _S33 = _S32 & 4095;

#line 311
        int _S34 = ref_0.y;
        vec4 p12_2 = CurveLoad_0(ivec2(_S33, _S34)) - vec4(renderCoord_0, renderCoord_0);
        vec4 texel1_0 = CurveLoad_0(ivec2(_S33 + 1, _S34));
        vec2 p3_2 = texel1_0.xy - renderCoord_0;
        vec2 p4_0 = texel1_0.zw - renderCoord_0;


        if(isCubic_0)
        {

#line 318
            maxY_0 = max(max(max(p12_2.x, p12_2.z), p3_2.x), p4_0.x);

#line 318
        }
        else
        {

#line 318
            maxY_0 = max(max(p12_2.x, p12_2.z), p3_2.x);

#line 318
        }

        float _S35 = _S28.x;

#line 320
        if((maxY_0 * _S35) < -0.5)
        {

#line 320
            break;
        }
        if(!isCubic_0)
        {
            uint code_0 = CalcRootCode_0(p12_2.y, p12_2.w, p3_2.y);
            if(code_0 != 0U)
            {
                vec2 r_1 = SolveHorizPoly_0(p12_2, p3_2) * _S35;
                if((code_0 & 1U) != 0U)
                {
                    float _S36 = r_1.x;

#line 330
                    xcov_1 = xcov_1 + saturate_0(_S36 + 0.5);
                    xwgt_1 = max(xwgt_1, saturate_0(1.0 - abs(_S36) * 2.0));

#line 328
                }

#line 333
                if(code_0 > 1U)
                {
                    float _S37 = r_1.y;

#line 335
                    xcov_1 = xcov_1 - saturate_0(_S37 + 0.5);
                    xwgt_1 = max(xwgt_1, saturate_0(1.0 - abs(_S37) * 2.0));

#line 333
                }

#line 325
            }

#line 322
        }
        else
        {

#line 343
            float _S38 = p12_2.y;

#line 343
            float _S39 = p12_2.w;

#line 343
            float _S40 = 3.0 * _S39;

#line 343
            float _S41 = 3.0 * p3_2.y;

#line 343
            float ay_0 = - _S38 + _S40 - _S41 + p4_0.y;
            float by_0 = 3.0 * _S38 - 6.0 * _S39 + _S41;
            float cy_0 = -3.0 * _S38 + _S40;

            float _S42 = p12_2.x;

#line 347
            float _S43 = p12_2.z;

#line 347
            float _S44 = 3.0 * _S43;

#line 347
            float _S45 = 3.0 * p3_2.x;

#line 347
            float ax_0 = - _S42 + _S44 - _S45 + p4_0.x;
            float bx_0 = 3.0 * _S42 - 6.0 * _S43 + _S45;
            float cx_0 = -3.0 * _S42 + _S44;

            vec3 ts_0 = SolveCubicRoots_0(ay_0, by_0, cy_0, _S38);

            float t0_0 = ts_0.x;
            float _S46 = 3.0 * ay_0;

#line 354
            float _S47 = 2.0 * by_0;

#line 354
            ApplyCubicRoot_0(t0_0, ((ax_0 * t0_0 + bx_0) * t0_0 + cx_0) * t0_0 + _S42, _S35, (_S46 * t0_0 + _S47) * t0_0 + cy_0, 1.0, xcov_1, xwgt_1);
            float t1_2 = ts_0.y;
            ApplyCubicRoot_0(t1_2, ((ax_0 * t1_2 + bx_0) * t1_2 + cx_0) * t1_2 + _S42, _S35, (_S46 * t1_2 + _S47) * t1_2 + cy_0, 1.0, xcov_1, xwgt_1);
            float t2_4 = ts_0.z;
            ApplyCubicRoot_0(t2_4, ((ax_0 * t2_4 + bx_0) * t2_4 + cx_0) * t2_4 + _S42, _S35, (_S46 * t2_4 + _S47) * t2_4 + cy_0, 1.0, xcov_1, xwgt_1);

#line 322
        }

#line 289
        ci_0 = ci_0 + 1;

#line 289
    }

#line 363
    float ycov_1 = 0.0;

#line 363
    float ywgt_1 = 0.0;
    ivec2 _S48 = BandLoad_0(ivec2(_S29 + bandMax_0.y + 1 + bandIndex_0.x, _S30));

#line 364
    int ci2_0 = 0;

    for(;;)
    {

#line 366
        if(ci2_0 < (_S48.x))
        {
        }
        else
        {

#line 366
            break;
        }

#line 372
        ivec2 ref_1 = BandLoad_0(CalcBandLoc_0(glyphLoc_1, uint(_S48.y) + uint(ci2_0)));
        int _S49 = ref_1.x;

#line 373
        bool isCubic_1 = (_S49 & 4096) != 0;
        int _S50 = _S49 & 4095;

#line 374
        int _S51 = ref_1.y;
        vec4 p12_3 = CurveLoad_0(ivec2(_S50, _S51)) - vec4(renderCoord_0, renderCoord_0);
        vec4 texel1_1 = CurveLoad_0(ivec2(_S50 + 1, _S51));
        vec2 p3_3 = texel1_1.xy - renderCoord_0;
        vec2 p4_1 = texel1_1.zw - renderCoord_0;


        if(isCubic_1)
        {

#line 381
            maxY_0 = max(max(max(p12_3.y, p12_3.w), p3_3.y), p4_1.y);

#line 381
        }
        else
        {

#line 381
            maxY_0 = max(max(p12_3.y, p12_3.w), p3_3.y);

#line 381
        }

        float _S52 = _S28.y;

#line 383
        if((maxY_0 * _S52) < -0.5)
        {

#line 383
            break;
        }
        if(!isCubic_1)
        {
            uint code_1 = CalcRootCode_0(p12_3.x, p12_3.z, p3_3.x);
            if(code_1 != 0U)
            {
                vec2 r_2 = SolveVertPoly_0(p12_3, p3_3) * _S52;
                if((code_1 & 1U) != 0U)
                {
                    float _S53 = r_2.x;

#line 393
                    ycov_1 = ycov_1 - saturate_0(_S53 + 0.5);
                    ywgt_1 = max(ywgt_1, saturate_0(1.0 - abs(_S53) * 2.0));

#line 391
                }

#line 396
                if(code_1 > 1U)
                {
                    float _S54 = r_2.y;

#line 398
                    ycov_1 = ycov_1 + saturate_0(_S54 + 0.5);
                    ywgt_1 = max(ywgt_1, saturate_0(1.0 - abs(_S54) * 2.0));

#line 396
                }

#line 388
            }

#line 385
        }
        else
        {

#line 406
            float _S55 = p12_3.x;

#line 406
            float _S56 = p12_3.z;

#line 406
            float _S57 = 3.0 * _S56;

#line 406
            float _S58 = 3.0 * p3_3.x;

#line 406
            float ax_1 = - _S55 + _S57 - _S58 + p4_1.x;
            float bx_1 = 3.0 * _S55 - 6.0 * _S56 + _S58;
            float cx_1 = -3.0 * _S55 + _S57;

            float _S59 = p12_3.y;

#line 410
            float _S60 = p12_3.w;

#line 410
            float _S61 = 3.0 * _S60;

#line 410
            float _S62 = 3.0 * p3_3.y;

#line 410
            float ay_1 = - _S59 + _S61 - _S62 + p4_1.y;
            float by_1 = 3.0 * _S59 - 6.0 * _S60 + _S62;
            float cy_1 = -3.0 * _S59 + _S61;

            vec3 ts_1 = SolveCubicRoots_0(ax_1, bx_1, cx_1, _S55);

            float t0_1 = ts_1.x;
            float _S63 = 3.0 * ax_1;

#line 417
            float _S64 = 2.0 * bx_1;

#line 417
            ApplyCubicRoot_0(t0_1, ((ay_1 * t0_1 + by_1) * t0_1 + cy_1) * t0_1 + _S59, _S52, (_S63 * t0_1 + _S64) * t0_1 + cx_1, -1.0, ycov_1, ywgt_1);
            float t1_3 = ts_1.y;
            ApplyCubicRoot_0(t1_3, ((ay_1 * t1_3 + by_1) * t1_3 + cy_1) * t1_3 + _S59, _S52, (_S63 * t1_3 + _S64) * t1_3 + cx_1, -1.0, ycov_1, ywgt_1);
            float t2_5 = ts_1.z;
            ApplyCubicRoot_0(t2_5, ((ay_1 * t2_5 + by_1) * t2_5 + cy_1) * t2_5 + _S59, _S52, (_S63 * t2_5 + _S64) * t2_5 + cx_1, -1.0, ycov_1, ywgt_1);

#line 385
        }

#line 366
        ci2_0 = ci2_0 + 1;

#line 366
    }

#line 425
    return CalcCoverage_0(xcov_1, ycov_1, xwgt_1, ywgt_1, glyphData_0.w);
}


#line 425
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 425
layout(location = 0)
in vec4 input_color_0;


#line 425
layout(location = 1)
in vec2 input_texcoord_0;


#line 425
flat layout(location = 2)
in vec4 input_banding_0;


#line 425
flat layout(location = 3)
in ivec4 input_glyph_0;

void main()
{

#line 428
    entryPointParam_main_ps_0 = vec4(input_color_0.xyz, input_color_0.w * SlugRender_0(input_texcoord_0, input_banding_0, input_glyph_0));

#line 428
    return;
}

