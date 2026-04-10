#version 450
#extension GL_EXT_samplerless_texture_functions : require
#extension GL_EXT_control_flow_attributes : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 63 0
struct SLANG_ParameterGroup_ImageInspectorParams_std140_0
{
    vec4 imgSize_0;
    vec4 panZoom_0;
    vec4 viewportPx_0;
    uvec4 packedTexDims_0;
    uvec4 layoutPack_0;
    uvec4 formatPack_0;
    vec4 exposureParams_0;
    vec4 tempTint_0;
    vec4 inGamut_r0_0;
    vec4 inGamut_r1_0;
    vec4 inGamut_r2_0;
    vec4 outGamut_r0_0;
    vec4 outGamut_r1_0;
    vec4 outGamut_r2_0;
    uvec4 pipelinePack_0;
    vec4 channelMask_0;
    vec4 nanColor_0;
    uvec4 modePack_0;
};


#line 30
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_ImageInspectorParams_std140_0
{
    vec4 imgSize_0;
    vec4 panZoom_0;
    vec4 viewportPx_0;
    uvec4 packedTexDims_0;
    uvec4 layoutPack_0;
    uvec4 formatPack_0;
    vec4 exposureParams_0;
    vec4 tempTint_0;
    vec4 inGamut_r0_0;
    vec4 inGamut_r1_0;
    vec4 inGamut_r2_0;
    vec4 outGamut_r0_0;
    vec4 outGamut_r1_0;
    vec4 outGamut_r2_0;
    uvec4 pipelinePack_0;
    vec4 channelMask_0;
    vec4 nanColor_0;
    uvec4 modePack_0;
}ImageInspectorParams_0;

#line 73
layout(binding = 3)
uniform texture2D texture0_0;


#line 188
uint LoadWord_0(uint byte_off_0)
{
    uint word_idx_0 = byte_off_0 >> 2U;
    uint lane_0 = word_idx_0 & 3U;
    uint texel_0 = word_idx_0 >> 2U;
    uint tw_0 = ImageInspectorParams_0.packedTexDims_0.x;
    int tx_0 = int(texel_0 & (tw_0 - 1U));
    uint _S1 = texel_0 / tw_0;
    ivec3 _S2 = ivec3(tx_0, int(_S1), 0);
    return floatBitsToUint((texelFetch((texture0_0), ((_S2)).xy, ((_S2)).z)))[lane_0];
}


#line 180
int SignExtend8_0(uint v_0)
{

#line 180
    return (int(v_0) << 24) >> 24;
}


#line 181
int SignExtend16_0(uint v_1)
{

#line 181
    return (int(v_1) << 16) >> 16;
}


#line 215
float FetchSample_0(int sx_0, int sy_0, int c_0)
{
    uint sample_type_0 = ImageInspectorParams_0.formatPack_0.x;



    uint off_0 = uint(sy_0) * ImageInspectorParams_0.layoutPack_0.y + uint(sx_0) * ImageInspectorParams_0.layoutPack_0.x + uint(c_0) * ImageInspectorParams_0.layoutPack_0.z;

    uint w0_0 = LoadWord_0(off_0);

#line 223
    float result_0;


    switch(sample_type_0)
    {
    case 0U:
        {

#line 226
            result_0 = float((w0_0 >> ((off_0 & 3U) * 8U)) & 255U) / 255.0;

#line 232
            break;
        }
    case 1U:
        {

#line 232
            result_0 = float(SignExtend8_0((w0_0 >> ((off_0 & 3U) * 8U)) & 255U)) / 127.0;

#line 238
            break;
        }
    case 2U:
        {

#line 238
            result_0 = float((w0_0 >> ((off_0 & 3U) * 8U)) & 65535U) / 65535.0;

#line 244
            break;
        }
    case 3U:
        {

#line 244
            result_0 = float(SignExtend16_0((w0_0 >> ((off_0 & 3U) * 8U)) & 65535U)) / 32767.0;

#line 250
            break;
        }
    case 4U:
        {

#line 250
            result_0 = float(w0_0) / 4.294967296e+09;



            break;
        }
    case 5U:
        {

#line 254
            result_0 = float((int((w0_0)))) / 2.147483648e+09;


            break;
        }
    case 8U:
        {

#line 257
            result_0 = (unpackHalf2x16(((w0_0 >> ((off_0 & 2U) * 8U)) & 65535U)).x);

#line 262
            break;
        }
    case 9U:
        {

#line 262
            result_0 = uintBitsToFloat(w0_0);



            break;
        }
    case 6U:
        {
            uint hi_0 = LoadWord_0(off_0 + 4U);

#line 270
            result_0 = float(w0_0) + float(hi_0) * 4.294967296e+09;

            break;
        }
    case 7U:
        {

            uint hi_1 = LoadWord_0(off_0 + 4U);
            if((hi_1 & 2147483648U) != 0U)
            {
                uint nlo_0 = ~w0_0 + 1U;
                uint _S3 = ~hi_1;

#line 281
                uint _S4;

#line 281
                if(nlo_0 == 0U)
                {

#line 281
                    _S4 = 1U;

#line 281
                }
                else
                {

#line 281
                    _S4 = 0U;

#line 281
                }

#line 281
                result_0 = - (float(nlo_0) + float(_S3 + _S4) * 4.294967296e+09);

#line 278
            }
            else
            {

#line 278
                result_0 = float(w0_0) + float(hi_1) * 4.294967296e+09;

#line 278
            }

#line 288
            break;
        }
    case 10U:
        {

            uint hi_2 = LoadWord_0(off_0 + 4U);
            uint sign_0 = hi_2 & 2147483648U;


            uint mant32_0 = ((hi_2 & 1048575U) << 3U) | (w0_0 >> 29U);
            int exp32_0 = int((hi_2 >> 20U) & 2047U) - 1023 + 127;
            if(exp32_0 <= 0)
            {

#line 299
                result_0 = uintBitsToFloat(sign_0);

#line 299
            }
            else
            {

#line 300
                if(exp32_0 >= 255)
                {

#line 300
                    result_0 = uintBitsToFloat(sign_0 | 2139095040U);

#line 300
                }
                else
                {

#line 300
                    result_0 = uintBitsToFloat((sign_0 | (uint(exp32_0) << 23U)) | mant32_0);

#line 300
                }

#line 299
            }


            break;
        }
    default:
        {

#line 302
            result_0 = 0.0;



            break;
        }
    }

#line 308
    return result_0;
}


#line 317
vec4 FetchPixel_0(int sx_1, int sy_1)
{
    int channels_0 = int(ImageInspectorParams_0.formatPack_0.y);

    int _S5 = clamp(sx_1, 0, int(ImageInspectorParams_0.imgSize_0.x) - 1);
    int _S6 = clamp(sy_1, 0, int(ImageInspectorParams_0.imgSize_0.y) - 1);
    vec4 v_2 = vec4(0.0, 0.0, 0.0, 1.0);
    if(channels_0 == 1)
    {
        float y_0 = FetchSample_0(_S5, _S6, 0);
        v_2 = vec4(y_0, y_0, y_0, 1.0);

#line 324
    }
    else
    {


        if(channels_0 == 2)
        {
            float _S7 = FetchSample_0(_S5, _S6, 0);

#line 331
            v_2[0] = _S7;
            float _S8 = FetchSample_0(_S5, _S6, 1);

#line 332
            v_2[1] = _S8;

#line 329
        }
        else
        {


            if(channels_0 == 3)
            {
                float _S9 = FetchSample_0(_S5, _S6, 0);

#line 336
                v_2[0] = _S9;
                float _S10 = FetchSample_0(_S5, _S6, 1);

#line 337
                v_2[1] = _S10;
                float _S11 = FetchSample_0(_S5, _S6, 2);

#line 338
                v_2[2] = _S11;

#line 334
            }
            else
            {

#line 342
                float _S12 = FetchSample_0(_S5, _S6, 0);

#line 342
                v_2[0] = _S12;
                float _S13 = FetchSample_0(_S5, _S6, 1);

#line 343
                v_2[1] = _S13;
                float _S14 = FetchSample_0(_S5, _S6, 2);

#line 344
                v_2[2] = _S14;
                float _S15 = FetchSample_0(_S5, _S6, 3);

#line 345
                v_2[3] = _S15;

#line 334
            }

#line 329
        }

#line 324
    }

#line 347
    return v_2;
}


#line 430
vec4 MosaicSampleXTrans_0(int sx_2, int sy_2)
{
    if((ImageInspectorParams_0.modePack_0.x) == 0U)
    {
        float v_3 = FetchSample_0(sx_2, sy_2, 0);
        return vec4(v_3, v_3, v_3, 1.0);
    }

#line 442
    int _S16 = sx_2 - int(uint(sx_2) % 6U);
    int _S17 = sy_2 - int(uint(sy_2) % 6U);

#line 443
    int j_0 = 0;

#line 443
    float sum_0 = 0.0;

#line 443
    int n_0 = 0;
    [[dont_unroll]]
    for(;;)
    {

#line 444
        if(j_0 < 6)
        {
        }
        else
        {

#line 444
            break;
        }



        int _S18 = _S17 + j_0;

#line 449
        int i_0 = 0;

#line 446
        [[dont_unroll]]
        for(;;)
        {

#line 446
            if(i_0 < 6)
            {
            }
            else
            {

#line 446
                break;
            }


            float _S19 = FetchSample_0(clamp(_S16 + i_0, 0, int(ImageInspectorParams_0.imgSize_0.x) - 1), clamp(_S18, 0, int(ImageInspectorParams_0.imgSize_0.y) - 1), 0);

#line 450
            float sum_1 = sum_0 + _S19;
            int n_1 = n_0 + 1;

#line 446
            i_0 = i_0 + 1;

#line 446
            sum_0 = sum_1;

#line 446
            n_0 = n_1;

#line 446
        }

#line 444
        j_0 = j_0 + 1;

#line 444
    }

#line 444
    float v_4;

#line 454
    if(n_0 > 0)
    {

#line 454
        v_4 = sum_0 / float(n_0);

#line 454
    }
    else
    {

#line 454
        v_4 = 0.0;

#line 454
    }
    return vec4(v_4, v_4, v_4, 1.0);
}


#line 359
vec4 MosaicSampleBayer_0(int sx_3, int sy_3, uint pattern_0)
{

    uint xp_0 = uint(sx_3 & 1);
    uint yp_0 = uint(sy_3 & 1);

#line 371
    bool _S20 = pattern_0 == 1U;

#line 371
    int color_0;

#line 371
    if(_S20)
    {

#line 372
        if(yp_0 == 0U)
        {

#line 372
            if(xp_0 == 0U)
            {

#line 372
                color_0 = 0;

#line 372
            }
            else
            {

#line 372
                color_0 = 1;

#line 372
            }

#line 372
        }
        else
        {

#line 372
            if(xp_0 == 0U)
            {

#line 372
                color_0 = 1;

#line 372
            }
            else
            {

#line 372
                color_0 = 2;

#line 372
            }

#line 372
        }

#line 371
    }
    else
    {

#line 373
        if(pattern_0 == 2U)
        {

#line 374
            if(yp_0 == 0U)
            {

#line 374
                if(xp_0 == 0U)
                {

#line 374
                    color_0 = 1;

#line 374
                }
                else
                {

#line 374
                    color_0 = 0;

#line 374
                }

#line 374
            }
            else
            {

#line 374
                if(xp_0 == 0U)
                {

#line 374
                    color_0 = 2;

#line 374
                }
                else
                {

#line 374
                    color_0 = 1;

#line 374
                }

#line 374
            }

#line 373
        }
        else
        {

#line 375
            if(pattern_0 == 3U)
            {

#line 376
                if(yp_0 == 0U)
                {

#line 376
                    if(xp_0 == 0U)
                    {

#line 376
                        color_0 = 1;

#line 376
                    }
                    else
                    {

#line 376
                        color_0 = 2;

#line 376
                    }

#line 376
                }
                else
                {

#line 376
                    if(xp_0 == 0U)
                    {

#line 376
                        color_0 = 0;

#line 376
                    }
                    else
                    {

#line 376
                        color_0 = 1;

#line 376
                    }

#line 376
                }

#line 375
            }
            else
            {
                if(yp_0 == 0U)
                {

#line 378
                    if(xp_0 == 0U)
                    {

#line 378
                        color_0 = 2;

#line 378
                    }
                    else
                    {

#line 378
                        color_0 = 1;

#line 378
                    }

#line 378
                }
                else
                {

#line 378
                    if(xp_0 == 0U)
                    {

#line 378
                        color_0 = 1;

#line 378
                    }
                    else
                    {

#line 378
                        color_0 = 0;

#line 378
                    }

#line 378
                }

#line 375
            }

#line 373
        }

#line 371
    }

#line 380
    if((ImageInspectorParams_0.modePack_0.x) == 0U)
    {

        float v_5 = FetchSample_0(sx_3, sy_3, 0);
        const vec3 _S21 = vec3(1.0, 1.0, 1.0);

#line 384
        vec3 tint_0;
        if(color_0 == 0)
        {

#line 385
            tint_0 = vec3(1.10000002384185791, 0.94999998807907104, 0.94999998807907104);

#line 385
        }
        else
        {

#line 385
            tint_0 = _S21;

#line 385
        }
        if(color_0 == 1)
        {

#line 386
            tint_0 = vec3(0.94999998807907104, 1.10000002384185791, 0.94999998807907104);

#line 386
        }
        if(color_0 == 2)
        {

#line 387
            tint_0 = vec3(0.94999998807907104, 0.94999998807907104, 1.10000002384185791);

#line 387
        }
        return vec4(v_5 * tint_0, 1.0);
    }

#line 407
    bool _S22 = pattern_0 == 2U;

    bool _S23 = pattern_0 == 3U;

#line 409
    int dy_0 = -1;

#line 409
    float r_0 = 0.0;

#line 409
    int nr_0 = 0;

#line 409
    float g_0 = 0.0;

#line 409
    int ng_0 = 0;

#line 409
    float b_0 = 0.0;

#line 409
    int nb_0 = 0;

#line 396
    [[dont_unroll]]
    for(;;)
    {

#line 396
        if(dy_0 <= 1)
        {
        }
        else
        {

#line 396
            break;
        }



        int y2_0 = sy_3 + dy_0;

#line 406
        bool _S24 = uint(y2_0 & 1) == 0U;

#line 406
        int dx_0 = -1;

#line 406
        float r_1 = r_0;

#line 406
        int nr_1 = nr_0;

#line 406
        float g_1 = g_0;

#line 406
        int ng_1 = ng_0;

#line 406
        float b_1 = b_0;

#line 406
        int nb_1 = nb_0;

#line 398
        [[dont_unroll]]
        for(;;)
        {

#line 398
            if(dx_0 <= 1)
            {
            }
            else
            {

#line 398
                break;
            }
            int x2_0 = sx_3 + dx_0;

            uint xp2_0 = uint(x2_0 & 1);

#line 402
            int c2_0;


            if(_S20)
            {

#line 406
                if(_S24)
                {

#line 406
                    if(xp2_0 == 0U)
                    {

#line 406
                        color_0 = 0;

#line 406
                    }
                    else
                    {

#line 406
                        color_0 = 1;

#line 406
                    }

#line 406
                }
                else
                {

#line 406
                    if(xp2_0 == 0U)
                    {

#line 406
                        color_0 = 1;

#line 406
                    }
                    else
                    {

#line 406
                        color_0 = 2;

#line 406
                    }

#line 406
                }

#line 406
                c2_0 = color_0;

#line 405
            }
            else
            {

#line 407
                if(_S22)
                {

#line 408
                    if(_S24)
                    {

#line 408
                        if(xp2_0 == 0U)
                        {

#line 408
                            color_0 = 1;

#line 408
                        }
                        else
                        {

#line 408
                            color_0 = 0;

#line 408
                        }

#line 408
                    }
                    else
                    {

#line 408
                        if(xp2_0 == 0U)
                        {

#line 408
                            color_0 = 2;

#line 408
                        }
                        else
                        {

#line 408
                            color_0 = 1;

#line 408
                        }

#line 408
                    }

#line 408
                    c2_0 = color_0;

#line 407
                }
                else
                {

#line 409
                    if(_S23)
                    {

#line 410
                        if(_S24)
                        {

#line 410
                            if(xp2_0 == 0U)
                            {

#line 410
                                color_0 = 1;

#line 410
                            }
                            else
                            {

#line 410
                                color_0 = 2;

#line 410
                            }

#line 410
                        }
                        else
                        {

#line 410
                            if(xp2_0 == 0U)
                            {

#line 410
                                color_0 = 0;

#line 410
                            }
                            else
                            {

#line 410
                                color_0 = 1;

#line 410
                            }

#line 410
                        }

#line 410
                        c2_0 = color_0;

#line 409
                    }
                    else
                    {
                        if(_S24)
                        {

#line 412
                            if(xp2_0 == 0U)
                            {

#line 412
                                color_0 = 2;

#line 412
                            }
                            else
                            {

#line 412
                                color_0 = 1;

#line 412
                            }

#line 412
                        }
                        else
                        {

#line 412
                            if(xp2_0 == 0U)
                            {

#line 412
                                color_0 = 1;

#line 412
                            }
                            else
                            {

#line 412
                                color_0 = 0;

#line 412
                            }

#line 412
                        }

#line 412
                        c2_0 = color_0;

#line 409
                    }

#line 407
                }

#line 405
            }

#line 416
            float v_6 = FetchSample_0(clamp(x2_0, 0, int(ImageInspectorParams_0.imgSize_0.x) - 1), clamp(y2_0, 0, int(ImageInspectorParams_0.imgSize_0.y) - 1), 0);
            if(c2_0 == 0)
            {

#line 417
                int nr_2 = nr_1 + 1;

#line 417
                r_1 = r_1 + v_6;

#line 417
                nr_1 = nr_2;

#line 417
            }
            if(c2_0 == 1)
            {

#line 418
                int ng_2 = ng_1 + 1;

#line 418
                g_1 = g_1 + v_6;

#line 418
                ng_1 = ng_2;

#line 418
            }
            if(c2_0 == 2)
            {

#line 419
                int nb_2 = nb_1 + 1;

#line 419
                b_1 = b_1 + v_6;

#line 419
                nb_1 = nb_2;

#line 419
            }

#line 398
            dx_0 = dx_0 + 1;

#line 398
        }

#line 396
        dy_0 = dy_0 + 1;

#line 396
        r_0 = r_1;

#line 396
        nr_0 = nr_1;

#line 396
        g_0 = g_1;

#line 396
        ng_0 = ng_1;

#line 396
        b_0 = b_1;

#line 396
        nb_0 = nb_1;

#line 396
    }

#line 422
    if(nr_0 > 0)
    {

#line 422
        r_0 = r_0 / float(nr_0);

#line 422
    }
    if(ng_0 > 0)
    {

#line 423
        g_0 = g_0 / float(ng_0);

#line 423
    }
    if(nb_0 > 0)
    {

#line 424
        b_0 = b_0 / float(nb_0);

#line 424
    }
    return vec4(r_0, g_0, b_0, 1.0);
}


#line 461
vec4 FetchPixelMosaic_0(int sx_4, int sy_4)
{
    uint pattern_1 = ImageInspectorParams_0.formatPack_0.z;
    if(pattern_1 == 0U)
    {

#line 465
        vec4 _S25 = FetchPixel_0(sx_4, sy_4);

#line 465
        return _S25;
    }

#line 466
    int _S26 = clamp(sx_4, 0, int(ImageInspectorParams_0.imgSize_0.x) - 1);
    int _S27 = clamp(sy_4, 0, int(ImageInspectorParams_0.imgSize_0.y) - 1);
    if(pattern_1 == 5U)
    {

#line 469
        vec4 _S28 = MosaicSampleXTrans_0(_S26, _S27);

#line 469
        return _S28;
    }

#line 470
    vec4 _S29 = MosaicSampleBayer_0(_S26, _S27, pattern_1);

#line 470
    return _S29;
}




vec4 FilterNearest_0(vec2 src_0)
{


    vec4 _S30 = FetchPixelMosaic_0(int(floor(src_0.x + 0.5)), int(floor(src_0.y + 0.5)));

#line 480
    return _S30;
}

vec4 FilterBilinear_0(vec2 src_1)
{
    float fx_0 = src_1.x - 0.5;
    float fy_0 = src_1.y - 0.5;
    int x0_0 = int(floor(fx_0));
    int y0_0 = int(floor(fy_0));
    float tx_1 = fx_0 - float(x0_0);
    float ty_0 = fy_0 - float(y0_0);
    vec4 c00_0 = FetchPixelMosaic_0(x0_0, y0_0);
    int _S31 = x0_0 + 1;

#line 492
    vec4 c10_0 = FetchPixelMosaic_0(_S31, y0_0);
    int _S32 = y0_0 + 1;

#line 493
    vec4 c01_0 = FetchPixelMosaic_0(x0_0, _S32);
    vec4 c11_0 = FetchPixelMosaic_0(_S31, _S32);
    vec4 _S33 = vec4(tx_1);

    return mix(mix(c00_0, c10_0, _S33), mix(c01_0, c11_0, _S33), vec4(ty_0));
}




float MitchellWeight_0(float x_0, float B_0, float C_0)
{
    float ax_0 = abs(x_0);
    float ax2_0 = ax_0 * ax_0;
    float ax3_0 = ax2_0 * ax_0;
    if(ax_0 < 1.0)
    {
        float _S34 = 6.0 * C_0;

#line 510
        return ((12.0 - 9.0 * B_0 - _S34) * ax3_0 + (-18.0 + 12.0 * B_0 + _S34) * ax2_0 + (6.0 - 2.0 * B_0)) * 0.1666666716337204;
    }


    if(ax_0 < 2.0)
    {
        return ((- B_0 - 6.0 * C_0) * ax3_0 + (6.0 * B_0 + 30.0 * C_0) * ax2_0 + (-12.0 * B_0 - 48.0 * C_0) * ax_0 + (8.0 * B_0 + 24.0 * C_0)) * 0.1666666716337204;
    }



    return 0.0;
}

vec4 FilterBicubic_0(vec2 src_2, float B_1, float C_1)
{
    float fx_1 = src_2.x - 0.5;
    float fy_1 = src_2.y - 0.5;
    int ix_0 = int(floor(fx_1));
    int iy_0 = int(floor(fy_1));
    float _S35 = fx_1 - float(ix_0);
    float _S36 = fy_1 - float(iy_0);
    const vec4 _S37 = vec4(0.0, 0.0, 0.0, 0.0);

#line 532
    int j_1 = -1;

#line 532
    vec4 sum_2 = _S37;

#line 532
    float ws_0 = 0.0;



    [[dont_unroll]]
    for(;;)
    {

#line 536
        if(j_1 <= 2)
        {
        }
        else
        {

#line 536
            break;
        }
        float _S38 = MitchellWeight_0(float(j_1) - _S36, B_1, C_1);

#line 543
        int _S39 = iy_0 + j_1;

#line 543
        int i_1 = -1;

#line 539
        [[dont_unroll]]
        for(;;)
        {

#line 539
            if(i_1 <= 2)
            {
            }
            else
            {

#line 539
                break;
            }

            float w_0 = MitchellWeight_0(float(i_1) - _S35, B_1, C_1) * _S38;
            vec4 _S40 = FetchPixelMosaic_0(ix_0 + i_1, _S39);

#line 543
            vec4 sum_3 = sum_2 + w_0 * _S40;
            float ws_1 = ws_0 + w_0;

#line 539
            i_1 = i_1 + 1;

#line 539
            sum_2 = sum_3;

#line 539
            ws_0 = ws_1;

#line 539
        }

#line 536
        j_1 = j_1 + 1;

#line 536
    }

#line 547
    if(ws_0 != 0.0)
    {

#line 547
        sum_2 = sum_2 / ws_0;

#line 547
    }

#line 547
    return sum_2;
}

float Sinc_0(float x_1)
{
    if((abs(x_1)) < 9.99999997475242708e-07)
    {

#line 552
        return 1.0;
    }

#line 553
    float px_0 = 3.14159274101257324 * x_1;
    return sin(px_0) / px_0;
}

float LanczosWeight_0(float x_2, float a_0)
{

    if((abs(x_2)) >= a_0)
    {

#line 560
        return 0.0;
    }

#line 561
    return Sinc_0(x_2) * Sinc_0(x_2 / a_0);
}

vec4 FilterLanczos_0(vec2 src_3, int a_1)
{
    float fx_2 = src_3.x - 0.5;
    float fy_2 = src_3.y - 0.5;
    int ix_1 = int(floor(fx_2));
    int iy_1 = int(floor(fy_2));
    float _S41 = fx_2 - float(ix_1);
    float _S42 = fy_2 - float(iy_1);
    const vec4 _S43 = vec4(0.0, 0.0, 0.0, 0.0);

#line 579
    float _S44 = float(a_1);

#line 579
    int j_2 = -2;

#line 579
    vec4 sum_4 = _S43;

#line 579
    float ws_2 = 0.0;

#line 577
    [[dont_unroll]]
    for(;;)
    {

#line 577
        if(j_2 <= 3)
        {
        }
        else
        {

#line 577
            break;
        }
        float _S45 = LanczosWeight_0(float(j_2) - _S42, _S44);

#line 584
        int _S46 = iy_1 + j_2;

#line 584
        int i_2 = -2;

#line 580
        [[dont_unroll]]
        for(;;)
        {

#line 580
            if(i_2 <= 3)
            {
            }
            else
            {

#line 580
                break;
            }

            float w_1 = LanczosWeight_0(float(i_2) - _S41, _S44) * _S45;
            vec4 _S47 = FetchPixelMosaic_0(ix_1 + i_2, _S46);

#line 584
            vec4 sum_5 = sum_4 + w_1 * _S47;
            float ws_3 = ws_2 + w_1;

#line 580
            i_2 = i_2 + 1;

#line 580
            sum_4 = sum_5;

#line 580
            ws_2 = ws_3;

#line 580
        }

#line 577
        j_2 = j_2 + 1;

#line 577
    }

#line 588
    if(ws_2 != 0.0)
    {

#line 588
        sum_4 = sum_4 / ws_2;

#line 588
    }

#line 588
    return sum_4;
}




vec4 SampleSource_0(vec2 src_4, float src_step_0)
{


    if(src_step_0 > 1.0)
    {


        vec4 _S48 = FetchPixelMosaic_0(int(floor(src_4.x)), int(floor(src_4.y)));

#line 602
        return _S48;
    }

#line 602
    vec4 r_2;

#line 607
    switch(ImageInspectorParams_0.formatPack_0.w)
    {
    case 0U:
        {

#line 609
            vec4 _S49 = FilterNearest_0(src_4);

#line 609
            r_2 = _S49;

#line 609
            break;
        }
    case 1U:
        {

#line 610
            vec4 _S50 = FilterBilinear_0(src_4);

#line 610
            r_2 = _S50;

#line 610
            break;
        }
    case 2U:
        {

#line 611
            vec4 _S51 = FilterBicubic_0(src_4, 0.3333333432674408, 0.3333333432674408);

#line 611
            r_2 = _S51;

#line 611
            break;
        }
    case 3U:
        {

#line 612
            vec4 _S52 = FilterBicubic_0(src_4, 0.0, 0.5);

#line 612
            r_2 = _S52;

#line 612
            break;
        }
    case 4U:
        {

#line 613
            vec4 _S53 = FilterLanczos_0(src_4, 2);

#line 613
            r_2 = _S53;

#line 613
            break;
        }
    case 5U:
        {

#line 614
            vec4 _S54 = FilterLanczos_0(src_4, 3);

#line 614
            r_2 = _S54;

#line 614
            break;
        }
    default:
        {

#line 615
            vec4 _S55 = FilterBilinear_0(src_4);

#line 615
            r_2 = _S55;

#line 615
            break;
        }
    }

#line 617
    return r_2;
}


#line 626
float SrgbToLinear_0(float v_7)
{
    if(v_7 <= 0.04044999927282333)
    {

#line 628
        return v_7 / 12.92000007629394531;
    }

#line 629
    return pow((v_7 + 0.05499999970197678) / 1.0549999475479126, 2.40000009536743164);
}


#line 638
float Rec709Inverse_0(float v_8)
{

    if(v_8 < 0.08100000023841858)
    {

#line 641
        return v_8 / 4.5;
    }

#line 642
    return pow((v_8 + 0.0989999994635582) / 1.09899997711181641, 2.22222232818603516);
}

float Rec1886Inverse_0(float v_9)
{

    return pow(max(v_9, 0.0), 2.40000009536743164);
}

float CineonInverse_0(float v_10)
{


    return pow(10.0, (v_10 * 1023.0 - 685.0) * 0.0020000000949949) - 0.01080000028014183;
}

float SLog2Inverse_0(float v_11)
{

    if(v_11 >= 0.03000122308731079)
    {

#line 662
        return (pow(10.0, (v_11 - 0.61659598350524902 - 0.02999999932944775) / 0.43269899487495422) - 0.03758399933576584) * 219.0 / 155.0;
    }

#line 663
    return (v_11 - 0.03000122308731079) / 3.53881287574768066 * 219.0 / 155.0;
}

float SLog3Inverse_0(float v_12)
{

    if(v_12 >= 0.16736099123954773)
    {

#line 670
        return pow(10.0, (v_12 * 1023.0 - 420.0) / 261.5) * 0.1900000125169754 - 0.00999999977648258;
    }

#line 671
    return (v_12 * 1023.0 - 95.0) * 0.01125000044703484 / 76.210296630859375;
}

float LogC3Inverse_0(float v_13)
{

#line 684
    if(v_13 > 0.89612150192260742)
    {

#line 685
        return (pow(10.0, (v_13 - 0.38553699851036072) / 0.24718999862670898) - 0.0522719994187355) / 5.55555582046508789;
    }

#line 686
    return (v_13 - 0.09280899912118912) / 5.36765480041503906;
}

float LogC4Inverse_0(float v_14)
{

    float a_2 = (pow(2.0, 18.0) - 16.0) / 117.4499969482421875;


    float s_0 = 7.0 * log(2.0) * pow(2.0, 5.56681060791015625) / (a_2 * 0.90713590383529663);
    float t_0 = (pow(2.0, 4.56681060791015625) - 64.0) / a_2;
    if(v_14 < 0.0)
    {

#line 697
        return v_14 * s_0 + t_0;
    }
    return (pow(2.0, 14.0 * ((v_14 - 0.09286412596702576) / 0.90713590383529663) + 6.0) - 64.0) / a_2;
}

float CanonLogInverse_0(float v_15)
{

    if(v_15 < 0.12512247264385223)
    {

#line 706
        return - (pow(10.0, (0.12512247264385223 - v_15) / 0.45310178399085999) - 1.0) / 10.15960025787353516;
    }

#line 707
    return (pow(10.0, (v_15 - 0.12512247264385223) / 0.45310178399085999) - 1.0) / 10.15960025787353516;
}

float CanonLog2Inverse_0(float v_16)
{

    if(v_16 < 0.09286412596702576)
    {

#line 714
        return - (pow(10.0, (0.09286412596702576 - v_16) / 0.24136076867580414) - 1.0) / 87.09937286376953125;
    }

#line 715
    return (pow(10.0, (v_16 - 0.09286412596702576) / 0.24136076867580414) - 1.0) / 87.09937286376953125;
}

float CanonLog3Inverse_0(float v_17)
{

    if(v_17 < 0.09746547043323517)
    {

#line 722
        return - (pow(10.0, (0.12783901393413544 - v_17) / 0.36726844310760498) - 1.0) / 14.98324966430664062;
    }

#line 723
    if(v_17 <= 0.15277890861034393)
    {

#line 724
        return (v_17 - 0.12512218952178955) / 1.9754798412322998;
    }

#line 725
    return (pow(10.0, (v_17 - 0.12240537256002426) / 0.36726844310760498) - 1.0) / 14.98324966430664062;
}

float VLogInverse_0(float v_18)
{

#line 735
    if(v_18 < 0.1809999942779541)
    {

#line 736
        return (v_18 - 0.125) / 5.59999990463256836;
    }

#line 737
    return pow(10.0, (v_18 - 0.59820598363876343) / 0.24151399731636047) - 0.00872999988496304;
}

float Log3G10Inverse_0(float v_19)
{

    float x_3 = v_19 / 0.22249700129032135;

#line 743
    float sign_x_0;
    if(x_3 < 0.0)
    {

#line 744
        sign_x_0 = -1.0;

#line 744
    }
    else
    {

#line 744
        sign_x_0 = 1.0;

#line 744
    }
    return sign_x_0 * (pow(10.0, abs(x_3)) - 1.0) * 0.00999999977648258;
}

float BMFilmGen5Inverse_0(float v_20)
{

#line 756
    if(v_20 < 0.09246575087308884)
    {

#line 757
        return (v_20 - 0.09286399930715561) / 8.28360557556152344;
    }

#line 758
    return pow(2.0, (v_20 - 0.5300133228302002) / 0.08692876249551773) - 0.00549407256767154;
}

float AppleLogInverse_0(float v_21)
{

#line 770
    if(v_21 < 0.00964455958455801)
    {

#line 771
        return v_21 / 47.28711318969726562 + -0.05641087889671326;
    }

#line 772
    return pow(2.0, (v_21 - 0.69336944818496704) / 0.08550479263067245) - 0.00999999977648258;
}

float FLogInverse_0(float v_22)
{

#line 785
    if(v_22 < 0.10053777694702148)
    {

#line 786
        return (v_22 - 0.09286399930715561) / 8.73563098907470703;
    }

#line 787
    return (pow(10.0, (v_22 - 0.79045301675796509) / 0.34467598795890808) - 0.00946800038218498) / 0.55555599927902222;
}

float DLogInverse_0(float v_23)
{

    if(v_23 <= 0.14000000059604645)
    {

#line 794
        return (v_23 - 0.09290000051259995) / 6.02500009536743164;
    }

#line 795
    return (pow(10.0, (v_23 - 0.80749499797821045) / 0.25562068819999695) - 0.01080000028014183) / 0.98919999599456787;
}

float PQInverse_0(float v_24)
{

#line 806
    float vp_0 = pow(max(v_24, 0.0), 0.01268331333994865);


    return pow(max(vp_0 - 0.8359375, 0.0) / (18.8515625 - 18.6875 * vp_0), 6.27739477157592773);
}

float HLGInverse_0(float v_25)
{

#line 818
    if(v_25 <= 0.5)
    {

#line 819
        return v_25 * v_25 / 3.0;
    }

#line 820
    return (exp((v_25 - 0.55991071462631226) / 0.1788327693939209) + 0.28466892242431641) / 12.0;
}

vec3 ApplyInputTransfer_0(vec3 c_1)
{

#line 823
    vec3 r_3;



    switch(ImageInspectorParams_0.pipelinePack_0.x)
    {
    case 0U:
        {

#line 827
            r_3 = c_1;

            break;
        }
    case 1U:
        {

#line 830
            float g_2 = ImageInspectorParams_0.exposureParams_0.w;

#line 830
            r_3 = vec3(pow(max(c_1.x, 0.0), g_2), pow(max(c_1.y, 0.0), g_2), pow(max(c_1.z, 0.0), g_2));
            break;
        }
    case 2U:
        {

#line 831
            r_3 = vec3(SrgbToLinear_0(c_1.x), SrgbToLinear_0(c_1.y), SrgbToLinear_0(c_1.z));
            break;
        }
    case 3U:
        {

#line 832
            r_3 = vec3(Rec709Inverse_0(c_1.x), Rec709Inverse_0(c_1.y), Rec709Inverse_0(c_1.z));
            break;
        }
    case 4U:
        {

#line 833
            r_3 = vec3(Rec1886Inverse_0(c_1.x), Rec1886Inverse_0(c_1.y), Rec1886Inverse_0(c_1.z));
            break;
        }
    case 5U:
        {

#line 834
            r_3 = vec3(CineonInverse_0(c_1.x), CineonInverse_0(c_1.y), CineonInverse_0(c_1.z));
            break;
        }
    case 6U:
        {

#line 835
            r_3 = vec3(SLog2Inverse_0(c_1.x), SLog2Inverse_0(c_1.y), SLog2Inverse_0(c_1.z));
            break;
        }
    case 7U:
        {

#line 836
            r_3 = vec3(SLog3Inverse_0(c_1.x), SLog3Inverse_0(c_1.y), SLog3Inverse_0(c_1.z));
            break;
        }
    case 8U:
        {

#line 837
            r_3 = vec3(LogC3Inverse_0(c_1.x), LogC3Inverse_0(c_1.y), LogC3Inverse_0(c_1.z));
            break;
        }
    case 9U:
        {

#line 838
            r_3 = vec3(LogC4Inverse_0(c_1.x), LogC4Inverse_0(c_1.y), LogC4Inverse_0(c_1.z));
            break;
        }
    case 10U:
        {

#line 839
            r_3 = vec3(CanonLogInverse_0(c_1.x), CanonLogInverse_0(c_1.y), CanonLogInverse_0(c_1.z));
            break;
        }
    case 11U:
        {

#line 840
            r_3 = vec3(CanonLog2Inverse_0(c_1.x), CanonLog2Inverse_0(c_1.y), CanonLog2Inverse_0(c_1.z));
            break;
        }
    case 12U:
        {

#line 841
            r_3 = vec3(CanonLog3Inverse_0(c_1.x), CanonLog3Inverse_0(c_1.y), CanonLog3Inverse_0(c_1.z));
            break;
        }
    case 13U:
        {

#line 842
            r_3 = vec3(VLogInverse_0(c_1.x), VLogInverse_0(c_1.y), VLogInverse_0(c_1.z));
            break;
        }
    case 14U:
        {

#line 843
            r_3 = vec3(Log3G10Inverse_0(c_1.x), Log3G10Inverse_0(c_1.y), Log3G10Inverse_0(c_1.z));
            break;
        }
    case 15U:
        {

#line 844
            r_3 = vec3(BMFilmGen5Inverse_0(c_1.x), BMFilmGen5Inverse_0(c_1.y), BMFilmGen5Inverse_0(c_1.z));
            break;
        }
    case 16U:
        {

#line 845
            r_3 = vec3(AppleLogInverse_0(c_1.x), AppleLogInverse_0(c_1.y), AppleLogInverse_0(c_1.z));
            break;
        }
    case 17U:
        {

#line 846
            r_3 = vec3(FLogInverse_0(c_1.x), FLogInverse_0(c_1.y), FLogInverse_0(c_1.z));
            break;
        }
    case 18U:
        {

#line 847
            r_3 = vec3(DLogInverse_0(c_1.x), DLogInverse_0(c_1.y), DLogInverse_0(c_1.z));
            break;
        }
    case 19U:
        {

#line 848
            r_3 = vec3(PQInverse_0(c_1.x), PQInverse_0(c_1.y), PQInverse_0(c_1.z));
            break;
        }
    case 20U:
        {

#line 849
            r_3 = vec3(HLGInverse_0(c_1.x), HLGInverse_0(c_1.y), HLGInverse_0(c_1.z));
            break;
        }
    default:
        {

#line 850
            r_3 = c_1;
            break;
        }
    }

#line 853
    return r_3;
}


#line 1106
vec3 MulMatrix_0(vec4 r0_0, vec4 r1_0, vec4 r2_0, vec3 v_26)
{
    return vec3(dot(r0_0.xyz, v_26), dot(r1_0.xyz, v_26), dot(r2_0.xyz, v_26));
}




vec3 ApplyTempTint_0(vec3 c_2, float temp_0, float tint_1)
{

#line 1114
    vec3 _S56 = c_2;



    float tw_1 = temp_0 * 0.10000000149011612;
    float tt_0 = tint_1 * 0.10000000149011612;
    _S56[0] = _S56[0] * (1.0 + tw_1 + tt_0);
    _S56[1] = _S56[1] * (1.0 - tt_0);
    _S56[2] = _S56[2] * (1.0 - tw_1 + tt_0);
    return _S56;
}


#line 900
vec3 TonemapReinhard_0(vec3 c_3)
{
    return c_3 / (1.0 + c_3);
}

vec3 TonemapReinhardExt_0(vec3 c_4)
{
    float W_0 = max(ImageInspectorParams_0.exposureParams_0.z, 0.00100000004749745);
    return c_4 * (1.0 + c_4 / (W_0 * W_0)) / (1.0 + c_4);
}


#line 12401 1
vec3 saturate_0(vec3 x_4)
{

#line 12409
    return clamp(x_4, vec3(0.0), vec3(1.0));
}


#line 912 0
vec3 TonemapACES_0(vec3 c_5)
{

#line 919
    return saturate_0(c_5 * (2.50999999046325684 * c_5 + 0.02999999932944775) / (c_5 * (2.43000006675720215 * c_5 + 0.5899999737739563) + 0.14000000059604645));
}


vec3 TonemapAGX_0(vec3 c_6)
{



    vec3 v_27 = saturate_0((log2(max(c_6, vec3(1.00000001335143196e-10))) + 12.47393035888671875) / 16.5);

    vec3 v2_0 = v_27 * v_27;
    vec3 v3_0 = v2_0 * v_27;
    vec3 v4_0 = v2_0 * v2_0;
    return -17.8600006103515625 * v3_0 * v3_0 + 78.01000213623046875 * v4_0 * v_27 - 126.6999969482421875 * v4_0 + 92.05999755859375 * v3_0 - 28.71999931335449219 * v2_0 + 4.36100006103515625 * v_27 - 0.17180000245571136;
}


#line 943
vec3 TonemapPBRNeutral_0(vec3 c_7)
{


    float x_5 = min(c_7.x, min(c_7.y, c_7.z));

#line 947
    float offset_0;
    if(x_5 < 0.07999999821186066)
    {

#line 948
        offset_0 = x_5 - 6.25 * x_5 * x_5;

#line 948
    }
    else
    {

#line 948
        offset_0 = 0.03999999910593033;

#line 948
    }
    vec3 _S57 = c_7 - offset_0;
    float peak_0 = max(_S57.x, max(_S57.y, _S57.z));
    if(peak_0 < 0.75999999046325684)
    {

#line 951
        return _S57;
    }
    float newPeak_0 = 1.0 - 0.0576000027358532 / (peak_0 + 0.24000000953674316 - 0.75999999046325684);


    return mix(_S57 * (newPeak_0 / peak_0), vec3(newPeak_0), vec3(1.0 - 1.0 / (0.15000000596046448 * (peak_0 - newPeak_0) + 1.0)));
}

vec3 TonemapHable_0(vec3 c_8)
{

#line 968
    vec3 _S58 = 0.15000000596046448 * c_8;

    return ((c_8 * (_S58 + 0.05000000074505806) + 0.00400000018998981) / (c_8 * (_S58 + 0.5) + 0.06000000238418579) - 0.06666666269302368) / 0.72512936592102051;
}

vec3 ApplyTonemap_0(vec3 c_9)
{

#line 973
    vec3 r_4;



    switch(ImageInspectorParams_0.pipelinePack_0.z)
    {
    case 0U:
        {

#line 977
            r_4 = c_9;

            break;
        }
    case 1U:
        {

#line 979
            r_4 = TonemapReinhard_0(c_9);
            break;
        }
    case 2U:
        {

#line 980
            r_4 = TonemapReinhardExt_0(c_9);
            break;
        }
    case 3U:
        {

#line 981
            r_4 = TonemapACES_0(c_9);
            break;
        }
    case 4U:
        {

#line 982
            r_4 = TonemapAGX_0(c_9);
            break;
        }
    case 5U:
        {

#line 983
            r_4 = TonemapPBRNeutral_0(c_9);
            break;
        }
    case 6U:
        {

#line 984
            r_4 = TonemapHable_0(c_9);
            break;
        }
    default:
        {

#line 985
            r_4 = c_9;
            break;
        }
    }

#line 988
    return r_4;
}


#line 632
float LinearToSrgb_0(float v_28)
{
    if(v_28 <= 0.00313080009073019)
    {

#line 634
        return v_28 * 12.92000007629394531;
    }

#line 635
    return 1.0549999475479126 * pow(max(v_28, 0.0), 0.4166666567325592) - 0.05499999970197678;
}


#line 859
float PQForward_0(float v_29)
{

#line 866
    float lp_0 = pow(max(v_29, 0.0), 0.1593017578125);
    return pow((0.8359375 + 18.8515625 * lp_0) / (1.0 + 18.6875 * lp_0), 78.84375);
}

float HLGForward_0(float v_30)
{



    if(v_30 <= 0.0833333358168602)
    {

#line 876
        return sqrt(3.0 * v_30);
    }

#line 877
    return 0.1788327693939209 * log(12.0 * v_30 - 0.28466892242431641) + 0.55991071462631226;
}

vec3 ApplyOutputTransfer_0(vec3 c_10)
{

#line 880
    vec3 r_5;



    switch(ImageInspectorParams_0.pipelinePack_0.y)
    {
    case 0U:
        {

#line 884
            r_5 = c_10;

            break;
        }
    case 1U:
        {

#line 887
            float g_3 = 1.0 / max(ImageInspectorParams_0.exposureParams_0.w, 0.00100000004749745);

#line 887
            r_5 = vec3(pow(max(c_10.x, 0.0), g_3), pow(max(c_10.y, 0.0), g_3), pow(max(c_10.z, 0.0), g_3));
            break;
        }
    case 2U:
        {

#line 888
            r_5 = vec3(LinearToSrgb_0(c_10.x), LinearToSrgb_0(c_10.y), LinearToSrgb_0(c_10.z));
            break;
        }
    case 3U:
        {

#line 889
            r_5 = vec3(PQForward_0(c_10.x), PQForward_0(c_10.y), PQForward_0(c_10.z));
            break;
        }
    case 4U:
        {

#line 890
            r_5 = vec3(HLGForward_0(c_10.x), HLGForward_0(c_10.y), HLGForward_0(c_10.z));
            break;
        }
    default:
        {

#line 891
            r_5 = c_10;
            break;
        }
    }

#line 894
    return r_5;
}


#line 12386 1
float saturate_1(float x_6)
{

#line 12394
    return clamp(x_6, 0.0, 1.0);
}


#line 996 0
vec3 PaletteViridis_0(float t_1)
{
    float _S59 = saturate_1(t_1);
    return vec3(0.2669999897480011, 0.00499999988824129, 0.32899999618530273) + _S59 * (vec3(0.10499999672174454, 1.40499997138977051, 1.38499999046325684) + _S59 * (vec3(-0.33000001311302185, -0.31700000166893005, 0.21400000154972076) + _S59 * (vec3(6.22800016403198242, -2.51999998092651367, -2.66499996185302734) + _S59 * (vec3(-13.09000015258789062, 1.39499998092651367, 6.33099985122680664) + _S59 * (vec3(11.10000038146972656, 0.0, -7.2350001335144043) + _S59 * vec3(-3.65799999237060547, 0.0, 2.09299993515014648))))));
}


#line 1007
vec3 PaletteMagma_0(float t_2)
{
    float _S60 = saturate_1(t_2);
    return vec3(-0.0020000000949949, -0.0, -0.01400000043213367) + _S60 * (vec3(0.25499999523162842, 0.0390000008046627, 1.60199999809265137) + _S60 * (vec3(7.18699979782104492, 2.79399991035461426, 4.80399990081787109) + _S60 * (vec3(-25.95000076293945312, -7.70100021362304688, -23.3899993896484375) + _S60 * (vec3(38.20999908447265625, 8.50199985504150391, 38.29999923706054688) + _S60 * (vec3(-25.54999923706054688, -3.85100007057189941, -27.39999961853027344) + _S60 * vec3(6.48400020599365234, 0.5339999794960022, 7.46500015258789062))))));
}


#line 1018
vec3 PaletteInferno_0(float t_3)
{
    float _S61 = saturate_1(t_3);
    return vec3(0.00019999999494758, 0.00170000002253801, -0.01930000074207783) + _S61 * (vec3(0.10599999874830246, 0.56099998950958252, 3.9869999885559082) + _S61 * (vec3(11.60200023651123047, -3.9719998836517334, -15.93999958038330078) + _S61 * (vec3(-41.70999908447265625, 17.43000030517578125, 44.34999847412109375) + _S61 * (vec3(77.160003662109375, -33.40000152587890625, -81.8000030517578125) + _S61 * (vec3(-71.31999969482421875, 32.63000106811523438, 73.20999908447265625) + _S61 * vec3(25.12999916076660156, -12.23999977111816406, -23.06999969482421875))))));
}


#line 1029
vec3 PalettePlasma_0(float t_4)
{
    float _S62 = saturate_1(t_4);
    return vec3(0.05880000069737434, 0.02969999983906746, 0.53100001811981201) + _S62 * (vec3(2.17600011825561523, 0.23800000548362732, 1.07200002670288086) + _S62 * (vec3(0.11699999868869781, 0.48059999942779541, -3.02900004386901855) + _S62 * (vec3(-9.62800025939941406, -1.92900002002716064, 4.42299985885620117) + _S62 * (vec3(20.79000091552734375, 1.94000005722045898, -3.13700008392333984) + _S62 * (vec3(-17.05999946594238281, -0.73299998044967651, 0.9649999737739563) + _S62 * vec3(4.68800020217895508, 0.06970000267028809, -0.08299999684095383))))));
}


#line 1040
vec3 PaletteCividis_0(float t_5)
{
    float _S63 = saturate_1(t_5);
    return vec3(-0.00860000029206276, 0.13220000267028809, 0.30140000581741333) + _S63 * (vec3(0.54439997673034668, 0.71960002183914185, 1.45229995250701904) + _S63 * (vec3(-0.09080000221729279, 0.13339999318122864, -3.95300006866455078) + _S63 * (vec3(0.55629998445510864, 0.01810000091791153, 4.08459997177124023) + _S63 * vec3(-0.00170000002253801, -0.00460000010207295, -1.88569998741149902))));
}




vec3 PaletteTurbo_0(float t_6)
{

    float _S64 = saturate_1(t_6);

#line 1059
    return vec3(0.13570000231266022, 0.09139999747276306, 0.10670000314712524) + _S64 * (vec3(4.59740018844604492, 2.18560004234313965, 12.59249973297119141) + _S64 * (vec3(-42.65999984741210938, 4.84100008010864258, -60.582000732421875) + _S64 * (vec3(132.1300048828125, -14.18500041961669922, 110.51000213623046875) + _S64 * (vec3(-152.94000244140625, 4.27740001678466797, -89.9010009765625) + _S64 * vec3(59.28599929809570312, 2.79929995536804199, 27.34300041198730469)))));
}


vec3 PaletteCinema_0(float v_31)
{
    if(v_31 < 0.0)
    {

#line 1065
        return vec3(0.5, 0.0, 0.5);
    }

#line 1066
    if(v_31 < 0.03999999910593033)
    {

#line 1066
        return vec3(0.0, 0.0, 1.0);
    }

#line 1067
    if(v_31 < 0.10000000149011612)
    {

#line 1067
        return vec3(0.0, 0.5, 1.0);
    }

#line 1068
    if(v_31 < 0.18000000715255737)
    {

#line 1068
        return vec3(0.0, 1.0, 1.0);
    }

#line 1069
    if(v_31 < 0.41999998688697815)
    {

#line 1069
        return vec3(0.5, 1.0, 0.0);
    }

#line 1070
    if(v_31 < 0.77999997138977051)
    {

#line 1070
        return vec3(1.0, 1.0, 0.0);
    }

#line 1071
    if(v_31 < 0.94999998807907104)
    {

#line 1071
        return vec3(1.0, 0.5, 0.0);
    }

#line 1072
    if(v_31 <= 1.0)
    {

#line 1072
        return vec3(1.0, 0.0, 0.0);
    }

#line 1073
    return vec3(1.0, 1.0, 1.0);
}

vec3 ApplyFalseColor_0(vec3 c_11)
{

    float _S65 = c_11.x;

#line 1079
    float _S66 = c_11.y;

#line 1079
    float _S67 = c_11.z;

#line 1079
    float v_32 = (_S65 + _S66 + _S67) * 0.3333333432674408;

#line 1079
    vec3 r_6;

    switch(ImageInspectorParams_0.pipelinePack_0.w)
    {
    case 0U:
        {

#line 1081
            r_6 = c_11;

            break;
        }
    case 1U:
        {

#line 1083
            r_6 = PaletteViridis_0(v_32);
            break;
        }
    case 2U:
        {

#line 1084
            r_6 = PaletteMagma_0(v_32);
            break;
        }
    case 3U:
        {

#line 1085
            r_6 = PaletteInferno_0(v_32);
            break;
        }
    case 4U:
        {

#line 1086
            r_6 = PalettePlasma_0(v_32);
            break;
        }
    case 5U:
        {

#line 1087
            r_6 = PaletteCividis_0(v_32);
            break;
        }
    case 6U:
        {

#line 1088
            r_6 = PaletteTurbo_0(v_32);
            break;
        }
    case 7U:
        {

#line 1089
            r_6 = PaletteCinema_0(v_32);
            break;
        }
    case 8U:
        {

#line 1090
            bool oog_0;


            if(_S65 < 0.0)
            {

#line 1093
                oog_0 = true;

#line 1093
            }
            else
            {

#line 1093
                oog_0 = _S66 < 0.0;

#line 1093
            }

#line 1093
            if(oog_0)
            {

#line 1093
                oog_0 = true;

#line 1093
            }
            else
            {

#line 1093
                oog_0 = _S67 < 0.0;

#line 1093
            }

#line 1093
            if(oog_0)
            {

#line 1093
                oog_0 = true;

#line 1093
            }
            else
            {

#line 1093
                oog_0 = _S65 > 1.0;

#line 1093
            }
            if(oog_0)
            {

#line 1094
                oog_0 = true;

#line 1094
            }
            else
            {

#line 1094
                oog_0 = _S66 > 1.0;

#line 1094
            }

#line 1094
            if(oog_0)
            {

#line 1094
                oog_0 = true;

#line 1094
            }
            else
            {

#line 1094
                oog_0 = _S67 > 1.0;

#line 1094
            }
            if(oog_0)
            {

#line 1095
                r_6 = vec3(1.0, 1.0, 0.0);

#line 1095
            }
            else
            {

#line 1095
                r_6 = c_11;

#line 1095
            }
            break;
        }
    default:
        {

#line 1096
            r_6 = c_11;

            break;
        }
    }

#line 1100
    return r_6;
}


#line 1100
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 1100
layout(location = 0)
in vec4 input_col_0;


#line 1100
layout(location = 1)
in vec2 input_uv_0;


#line 1141
void main()
{

#line 1148
    float totalScale_0 = ImageInspectorParams_0.panZoom_0.z * ImageInspectorParams_0.panZoom_0.w;

#line 1155
    vec4 sampled_0 = SampleSource_0(ImageInspectorParams_0.imgSize_0.xy * 0.5 + ImageInspectorParams_0.panZoom_0.xy + (input_uv_0 - 0.5) * ImageInspectorParams_0.viewportPx_0.xy / totalScale_0, 1.0 / max(totalScale_0, 9.99999997475242708e-07));
    vec3 rgb_0 = sampled_0.xyz;
    float a_3 = sampled_0.w;

#line 1157
    bool _S68;


    if((any(bvec3((isnan(rgb_0))))))
    {

#line 1160
        _S68 = true;

#line 1160
    }
    else
    {

#line 1160
        _S68 = (any(bvec3((isinf(rgb_0)))));

#line 1160
    }

#line 1160
    if(_S68)
    {

#line 1160
        entryPointParam_main_ps_0 = vec4(ImageInspectorParams_0.nanColor_0.xyz, 1.0);

#line 1160
        return;
    }

#line 1160
    entryPointParam_main_ps_0 = vec4(ApplyFalseColor_0(ApplyOutputTransfer_0(MulMatrix_0(ImageInspectorParams_0.outGamut_r0_0, ImageInspectorParams_0.outGamut_r1_0, ImageInspectorParams_0.outGamut_r2_0, ApplyTonemap_0(ApplyTempTint_0((MulMatrix_0(ImageInspectorParams_0.inGamut_r0_0, ImageInspectorParams_0.inGamut_r1_0, ImageInspectorParams_0.inGamut_r2_0, ApplyInputTransfer_0(rgb_0)) - ImageInspectorParams_0.exposureParams_0.y) / max(ImageInspectorParams_0.exposureParams_0.z - ImageInspectorParams_0.exposureParams_0.y, 9.99999997475242708e-07) * (exp2((ImageInspectorParams_0.exposureParams_0.x))), ImageInspectorParams_0.tempTint_0.x, ImageInspectorParams_0.tempTint_0.y)))) * ImageInspectorParams_0.channelMask_0.xyz), a_3 * ImageInspectorParams_0.channelMask_0.w) * input_col_0;

#line 1160
    return;
}

