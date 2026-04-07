#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 28 0
struct SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec2 texel_size_0;
    float mode_0;
    float param0_0;
    float param1_0;
    float param2_0;
    vec2 win_center_0;
    vec2 win_half_px_0;
    float win_rounding_0;
    float pad0_0;
    vec2 mouse_uv_0;
    float pad1_0;
    float pad2_0;
};


#line 15
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    vec2 texel_size_0;
    float mode_0;
    float param0_0;
    float param1_0;
    float param2_0;
    vec2 win_center_0;
    vec2 win_half_px_0;
    float win_rounding_0;
    float pad0_0;
    vec2 mouse_uv_0;
    float pad1_0;
    float pad2_0;
}PS_CONSTANT_BUFFER_0;

#line 65
layout(binding = 0)
uniform texture2D sceneTexture_0;


#line 66
layout(binding = 0)
uniform sampler sceneSampler_0;


#line 111
vec2 uvToPixel_0(vec2 uv_0)
{
    return (uv_0 - PS_CONSTANT_BUFFER_0.win_center_0) / PS_CONSTANT_BUFFER_0.texel_size_0;
}


#line 105
float sdRoundedBox_0(vec2 p_0, vec2 b_0, float r_0)
{
    vec2 q_0 = abs(p_0) - b_0 + r_0;
    return length(max(q_0, vec2(0.0))) + min(max(q_0.x, q_0.y), 0.0) - r_0;
}


#line 116
float sdfWithNormal_0(vec2 uv_1, float extraRadius_0, out vec2 normal_0)
{
    vec2 p_1 = uvToPixel_0(uv_1);

    float r_1 = PS_CONSTANT_BUFFER_0.win_rounding_0 + extraRadius_0;
    float d_0 = sdRoundedBox_0(p_1, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1);

    const vec2 _S1 = vec2(1.0, 0.0);
    const vec2 _S2 = vec2(0.0, 1.0);
    normal_0 = normalize(vec2(sdRoundedBox_0(p_1 + _S1, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1) - sdRoundedBox_0(p_1 - _S1, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1), sdRoundedBox_0(p_1 + _S2, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1) - sdRoundedBox_0(p_1 - _S2, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1)) + vec2(0.00009999999747379, 0.0));
    return d_0;
}


#line 12386 1
float saturate_0(float x_0)
{

#line 12394
    return clamp(x_0, 0.0, 1.0);
}


#line 70 0
vec4 sampleClamped_0(vec2 uv_2)
{
    return (texture(sampler2D(sceneTexture_0,sceneSampler_0), (clamp(uv_2, PS_CONSTANT_BUFFER_0.texel_size_0 * 0.5, 1.0 - PS_CONSTANT_BUFFER_0.texel_size_0 * 0.5))));
}


#line 183
vec4 effect_glass_refract_0(vec2 uv_3)
{

    float eta_0 = 1.0 / max(PS_CONSTANT_BUFFER_0.param1_0, 1.00999999046325684);
    float bevelPx_0 = max(PS_CONSTANT_BUFFER_0.param0_0, 0.00999999977648258) * min(PS_CONSTANT_BUFFER_0.win_half_px_0.x, PS_CONSTANT_BUFFER_0.win_half_px_0.y);

    vec2 normal_1;
    float d_1 = sdfWithNormal_0(uv_3, 0.0, normal_1);
    float curvature_0 = saturate_0(1.0 + d_1 / bevelPx_0);

    vec2 ruv_0 = uv_3 + normal_1 * curvature_0 * (1.0 - eta_0) * bevelPx_0 * PS_CONSTANT_BUFFER_0.texel_size_0;

    vec4 _S3 = sampleClamped_0(ruv_0) * 0.5;

#line 195
    vec4 col_0 = _S3;
    const vec2 _S4 = vec2(1.0, 0.0);

#line 196
    vec4 _S5 = _S3 + sampleClamped_0(ruv_0 + PS_CONSTANT_BUFFER_0.texel_size_0 * _S4) * 0.125;

#line 196
    col_0 = _S5;
    vec4 _S6 = _S5 + sampleClamped_0(ruv_0 - PS_CONSTANT_BUFFER_0.texel_size_0 * _S4) * 0.125;

#line 197
    col_0 = _S6;
    const vec2 _S7 = vec2(0.0, 1.0);

#line 198
    vec4 _S8 = _S6 + sampleClamped_0(ruv_0 + PS_CONSTANT_BUFFER_0.texel_size_0 * _S7) * 0.125;

#line 198
    col_0 = _S8;
    vec4 _S9 = _S8 + sampleClamped_0(ruv_0 - PS_CONSTANT_BUFFER_0.texel_size_0 * _S7) * 0.125;

#line 199
    col_0 = _S9;
    col_0.xyz = _S9.xyz + pow(curvature_0, 2.0) * 0.30000001192092896;
    return col_0;
}


#line 96
vec2 applyParallax_0(vec2 uv_4, float strength_0)
{


    return uv_4 + (uv_4 - PS_CONSTANT_BUFFER_0.win_center_0) / max(PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0 * 2.0, vec2(0.00100000004749745, 0.00100000004749745)) * strength_0 * PS_CONSTANT_BUFFER_0.texel_size_0 * 2.0;
}


#line 131
float hash12_0(vec2 p_2)
{
    vec3 p3_0 = fract(p_2.xyx * 0.1031000018119812);
    vec3 p3_1 = p3_0 + dot(p3_0, p3_0.yzx + 33.3300018310546875);
    return fract((p3_1.x + p3_1.y) * p3_1.z);
}


#line 206
vec4 effect_frosted_0(vec2 uv_5)
{
    vec2 _S10 = applyParallax_0(uv_5, PS_CONSTANT_BUFFER_0.param0_0);
    vec2 noise_uv_0 = _S10 / PS_CONSTANT_BUFFER_0.texel_size_0;

#line 214
    vec2 off_0 = PS_CONSTANT_BUFFER_0.texel_size_0 * PS_CONSTANT_BUFFER_0.param0_0 * 0.5;
    vec2 suv_0 = _S10 + vec2(hash12_0(floor(noise_uv_0 * PS_CONSTANT_BUFFER_0.param1_0)) * 2.0 - 1.0, hash12_0(floor(noise_uv_0 * PS_CONSTANT_BUFFER_0.param1_0) + vec2(7.13000011444091797, 3.71000003814697266)) * 2.0 - 1.0) * PS_CONSTANT_BUFFER_0.param0_0 * PS_CONSTANT_BUFFER_0.texel_size_0;

    float _S11 = off_0.x;

#line 217
    float _S12 = - _S11;

#line 217
    float _S13 = off_0.y;

    float _S14 = - _S13;

    return (sampleClamped_0(suv_0) + sampleClamped_0(suv_0 + vec2(_S12, _S13)) + sampleClamped_0(suv_0 + vec2(_S11, _S13)) + sampleClamped_0(suv_0 + vec2(_S11, _S14)) + sampleClamped_0(suv_0 + vec2(_S12, _S14))) * 0.20000000298023224;
}



vec4 effect_pixelate_0(vec2 uv_6)
{

    vec2 block_0 = PS_CONSTANT_BUFFER_0.texel_size_0 * max(PS_CONSTANT_BUFFER_0.param0_0, 1.0);

    return sampleClamped_0(floor(applyParallax_0(uv_6, PS_CONSTANT_BUFFER_0.param0_0) / block_0) * block_0 + block_0 * 0.5);
}


#line 238
vec3 wavelengthToRGB_0(float lambda_0)
{

    float _S15 = lambda_0 - 442.0;

#line 241
    float _S16;

#line 241
    if(lambda_0 < 442.0)
    {

#line 241
        _S16 = 0.06239999830722809;

#line 241
    }
    else
    {

#line 241
        _S16 = 0.03739999979734421;

#line 241
    }

#line 241
    float t1_0 = _S15 * _S16;
    float _S17 = lambda_0 - 599.79998779296875;

#line 242
    if(lambda_0 < 599.79998779296875)
    {

#line 242
        _S16 = 0.02639999985694885;

#line 242
    }
    else
    {

#line 242
        _S16 = 0.03229999914765358;

#line 242
    }

#line 242
    float t2_0 = _S17 * _S16;
    float _S18 = lambda_0 - 501.100006103515625;

#line 243
    if(lambda_0 < 501.100006103515625)
    {

#line 243
        _S16 = 0.04899999871850014;

#line 243
    }
    else
    {

#line 243
        _S16 = 0.03819999843835831;

#line 243
    }

#line 243
    float t3_0 = _S18 * _S16;



    float x_1 = 0.3619999885559082 * exp(-0.5 * t1_0 * t1_0) + 1.0559999942779541 * exp(-0.5 * t2_0 * t2_0) - 0.06499999761581421 * exp(-0.5 * t3_0 * t3_0);

    float _S19 = lambda_0 - 568.79998779296875;

#line 249
    if(lambda_0 < 568.79998779296875)
    {

#line 249
        _S16 = 0.02129999920725822;

#line 249
    }
    else
    {

#line 249
        _S16 = 0.02470000088214874;

#line 249
    }

#line 249
    float t4_0 = _S19 * _S16;
    float _S20 = lambda_0 - 530.9000244140625;

#line 250
    if(lambda_0 < 530.9000244140625)
    {

#line 250
        _S16 = 0.06129999831318855;

#line 250
    }
    else
    {

#line 250
        _S16 = 0.03220000118017197;

#line 250
    }

#line 250
    float t5_0 = _S20 * _S16;


    float y_0 = 0.82099997997283936 * exp(-0.5 * t4_0 * t4_0) + 0.28600001335144043 * exp(-0.5 * t5_0 * t5_0);

    float _S21 = lambda_0 - 437.0;

#line 255
    if(lambda_0 < 437.0)
    {

#line 255
        _S16 = 0.08449999988079071;

#line 255
    }
    else
    {

#line 255
        _S16 = 0.02779999934136868;

#line 255
    }

#line 255
    float t6_0 = _S21 * _S16;
    float _S22 = lambda_0 - 459.0;

#line 256
    if(lambda_0 < 459.0)
    {

#line 256
        _S16 = 0.03849999979138374;

#line 256
    }
    else
    {

#line 256
        _S16 = 0.07249999791383743;

#line 256
    }

#line 256
    float t7_0 = _S22 * _S16;


    float z_0 = 1.21700000762939453 * exp(-0.5 * t6_0 * t6_0) + 0.6809999942779541 * exp(-0.5 * t7_0 * t7_0);


    vec3 rgb_0;
    rgb_0[0] = 3.2406001091003418 * x_1 - 1.53719997406005859 * y_0 - 0.49860000610351562 * z_0;
    rgb_0[1] = -0.96890002489089966 * x_1 + 1.87580001354217529 * y_0 + 0.04149999842047691 * z_0;
    rgb_0[2] = 0.05570000037550926 * x_1 - 0.20399999618530273 * y_0 + 1.05700004100799561 * z_0;
    return max(rgb_0, vec3(0.0));
}


#line 273
vec4 effect_chromatic_0(vec2 uv_7)
{

    vec2 toUV_0 = uv_7 - PS_CONSTANT_BUFFER_0.win_center_0;


    vec2 _S23 = toUV_0 * length(toUV_0 / max(PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0, vec2(0.00100000004749745, 0.00100000004749745))) * PS_CONSTANT_BUFFER_0.param0_0 * PS_CONSTANT_BUFFER_0.texel_size_0 / max(length(toUV_0), 0.00009999999747379);

    int N_0 = clamp(int(PS_CONSTANT_BUFFER_0.param1_0), 4, 16);

#line 281
    int N_1;
    if(N_0 < 4)
    {

#line 282
        N_1 = 8;

#line 282
    }
    else
    {

#line 282
        N_1 = N_0;

#line 282
    }

    const vec3 _S24 = vec3(0.0, 0.0, 0.0);



    float _S25 = float(N_1 - 1);

#line 306
    const vec3 _S26 = vec3(0.00100000004749745, 0.00100000004749745, 0.00100000004749745);

#line 306
    int i_0 = 0;

#line 306
    vec3 col_1 = _S24;

#line 306
    vec3 totalWeight_0 = _S24;

#line 286
    for(;;)
    {

#line 286
        if(i_0 < N_1)
        {
        }
        else
        {

#line 286
            break;
        }



        float lambda_1 = mix(380.0, 780.0, float(i_0) / _S25);

#line 301
        vec3 w_0 = wavelengthToRGB_0(lambda_1);

        vec3 col_2 = col_1 + sampleClamped_0(uv_7 + _S23 * (3.025e+05 / (lambda_1 * lambda_1) - 1.0)).xyz * w_0;
        vec3 totalWeight_1 = totalWeight_0 + w_0;

#line 286
        i_0 = i_0 + 1;

#line 286
        col_1 = col_2;

#line 286
        totalWeight_0 = totalWeight_1;

#line 286
    }

#line 307
    return vec4(col_1 / max(totalWeight_0, _S26), 1.0);
}



vec4 effect_liquid_glass_0(vec2 uv_8)
{
    float strength_1 = max(PS_CONSTANT_BUFFER_0.param0_0, 0.00999999977648258);

    float bevelPx_1 = max(PS_CONSTANT_BUFFER_0.param1_0, 0.00999999977648258) * min(PS_CONSTANT_BUFFER_0.win_half_px_0.x, PS_CONSTANT_BUFFER_0.win_half_px_0.y);

    vec2 normal_2;
    float d_2 = sdfWithNormal_0(uv_8, 0.0, normal_2);
    float inside_0 = saturate_0(- d_2 / bevelPx_1);
    float curvature_1 = 1.0 - inside_0;


    vec2 ruv_1 = uv_8 + normal_2 * curvature_1 * strength_1 * bevelPx_1 * PS_CONSTANT_BUFFER_0.texel_size_0;

    vec4 col_3 = sampleClamped_0(ruv_1);



    vec2 dOff_0 = normal_2 * (curvature_1 * strength_1 * bevelPx_1 * 0.30000001192092896 * PS_CONSTANT_BUFFER_0.texel_size_0.x);
    col_3[0] = sampleClamped_0(ruv_1 + dOff_0).x;
    col_3[2] = sampleClamped_0(ruv_1 - dOff_0).z;

#line 337
    col_3.xyz = col_3.xyz + pow(saturate_0(1.0 - abs(d_2 + bevelPx_1 * 0.30000001192092896) / (bevelPx_1 * 0.15000000596046448)), 4.0) * 0.40000000596046448;


    col_3.xyz = col_3.xyz + pow(curvature_1, 3.0) * 0.25;
    col_3.xyz = col_3.xyz * mix(0.85000002384185791, 1.0, inside_0);
    return col_3;
}



vec4 effect_heat_haze_0(vec2 uv_9)
{
    float amplitude_0 = max(PS_CONSTANT_BUFFER_0.param0_0, 0.5);
    float freq_0 = max(PS_CONSTANT_BUFFER_0.param1_0, 1.0);



    vec2 px_0 = uv_9 / PS_CONSTANT_BUFFER_0.texel_size_0;
    float _S27 = px_0.y * freq_0;

#line 355
    float _S28 = px_0.x * freq_0;

#line 355
    float wave1_0 = sin(_S27 * 0.05000000074505806 + PS_CONSTANT_BUFFER_0.param2_0 * 2.29999995231628418) * cos(_S28 * 0.02999999932944775 + PS_CONSTANT_BUFFER_0.param2_0 * 1.70000004768371582);
    float wave2_0 = sin(_S27 * 0.07999999821186066 - PS_CONSTANT_BUFFER_0.param2_0 * 1.89999997615814209 + 1.5) * cos(_S28 * 0.03999999910593033 - PS_CONSTANT_BUFFER_0.param2_0 * 2.09999990463256836);

    vec2 distort_0;
    distort_0[0] = (wave1_0 * 0.60000002384185791 + wave2_0 * 0.40000000596046448) * amplitude_0 * PS_CONSTANT_BUFFER_0.texel_size_0.x;
    distort_0[1] = (wave2_0 * 0.60000002384185791 + wave1_0 * 0.40000000596046448) * amplitude_0 * PS_CONSTANT_BUFFER_0.texel_size_0.y * 0.5;

    vec4 _S29 = sampleClamped_0(uv_9 + distort_0);

#line 362
    vec4 col_4 = _S29;



    col_4.xyz = _S29.xyz * (1.0 + wave1_0 * wave2_0 * 0.02999999932944775);

    return col_4;
}



vec2 hash22_0(vec2 p_3)
{
    vec3 p3_2 = fract(p_3.xyx * vec3(0.1031000018119812, 0.10300000011920929, 0.09730000048875809));
    vec3 p3_3 = p3_2 + dot(p3_2, p3_2.yzx + 33.3300018310546875);
    return fract((p3_3.xx + p3_3.yz) * p3_3.zy);
}


#line 384
vec4 effect_voronoi_0(vec2 uv_10)
{

    float edgeW_0 = max(PS_CONSTANT_BUFFER_0.param1_0, 0.5);



    vec2 px_1 = uv_10 / PS_CONSTANT_BUFFER_0.texel_size_0;
    float cellSize_0 = min(PS_CONSTANT_BUFFER_0.win_half_px_0.x, PS_CONSTANT_BUFFER_0.win_half_px_0.y) * 2.0 / max(PS_CONSTANT_BUFFER_0.param0_0, 2.0);
    vec2 cell_0 = floor(px_1 / cellSize_0);

#line 410
    const vec2 _S30 = vec2(3.17000007629394531, 1.9299999475479126);



    float _S31 = 1.0 - 1.0 / max(max(PS_CONSTANT_BUFFER_0.param2_0, 1.0), 1.00999999046325684);
    float _S32 = _S31 * cellSize_0;

#line 415
    float _S33 = _S32 * 0.20000000298023224;

#line 421
    float dispersion_0 = _S32 * 0.05999999865889549;

#line 430
    const vec3 _S34 = vec3(1.0, 1.0, 1.0);


    float _S35 = edgeW_0 * 3.0;

#line 433
    float minDist_0 = 1.0e+10;

#line 433
    float secondDist_0 = 1.0e+10;

#line 433
    vec2 closestCell_0 = cell_0;

#line 433
    int y_1 = -1;

#line 399
    for(;;)
    {

#line 399
        if(y_1 <= 1)
        {
        }
        else
        {

#line 399
            break;
        }

        float _S36 = float(y_1);

#line 402
        float minDist_1 = minDist_0;

#line 402
        float secondDist_1 = secondDist_0;

#line 402
        vec2 closestCell_1 = closestCell_0;

#line 402
        int x_2 = -1;

#line 400
        for(;;)
        {

#line 400
            if(x_2 <= 1)
            {
            }
            else
            {

#line 400
                break;
            }
            vec2 neighbor_0 = cell_0 + vec2(float(x_2), _S36);

            float d_3 = length(px_1 - (neighbor_0 + hash22_0(neighbor_0)) * cellSize_0);
            if(d_3 < minDist_1)
            {

#line 395
                float _S37 = minDist_1;

#line 395
                minDist_1 = d_3;

#line 395
                secondDist_1 = _S37;

#line 395
                closestCell_1 = neighbor_0;

#line 405
            }
            else
            {

#line 405
                float secondDist_2;
                if(d_3 < secondDist_1)
                {

#line 406
                    secondDist_2 = d_3;

#line 406
                }
                else
                {

#line 406
                    secondDist_2 = secondDist_1;

#line 406
                }

#line 406
                secondDist_1 = secondDist_2;

#line 405
            }

#line 400
            x_2 = x_2 + 1;

#line 400
        }

#line 399
        int y_2 = y_1 + 1;

#line 399
        minDist_0 = minDist_1;

#line 399
        secondDist_0 = secondDist_1;

#line 399
        closestCell_0 = closestCell_1;

#line 399
        y_1 = y_2;

#line 399
    }

#line 411
    vec2 cellTilt_0 = hash22_0(closestCell_0 * 7.30999994277954102 + _S30) * 2.0 - 1.0;



    vec2 sampleUV_0 = applyParallax_0(uv_10 + cellTilt_0 * _S31 * cellSize_0 * 0.40000000596046448 * PS_CONSTANT_BUFFER_0.texel_size_0, _S33);


    vec4 col_5 = sampleClamped_0(sampleUV_0);



    vec2 dOff_1 = cellTilt_0 * dispersion_0 * PS_CONSTANT_BUFFER_0.texel_size_0;
    col_5[0] = sampleClamped_0(sampleUV_0 + dOff_1).x;
    col_5[2] = sampleClamped_0(sampleUV_0 - dOff_1).z;


    float edgeDist_0 = secondDist_0 - minDist_0;


    col_5.xyz = mix(col_5.xyz, _S34, vec3(pow(1.0 - saturate_0(edgeDist_0 / edgeW_0), 2.0) * 0.5));



    col_5.xyz = col_5.xyz * mix(0.69999998807907104, 1.0, saturate_0(edgeDist_0 / _S35));

    return col_5;
}


#line 12401 1
vec3 saturate_1(vec3 x_3)
{

#line 12409
    return clamp(x_3, vec3(0.0), vec3(1.0));
}


#line 441 0
vec4 effect_edge_glow_0(vec2 uv_11)
{

#line 447
    float _S38 = PS_CONSTANT_BUFFER_0.texel_size_0.x;

#line 447
    float _S39 = - _S38;

#line 447
    float _S40 = PS_CONSTANT_BUFFER_0.texel_size_0.y;

#line 447
    float _S41 = - _S40;

#line 447
    const vec3 _S42 = vec3(0.29899999499320984, 0.58700001239776611, 0.11400000005960464);

    float tr_0 = dot(sampleClamped_0(uv_11 + vec2(_S38, _S41)).xyz, _S42);


    float bl_0 = dot(sampleClamped_0(uv_11 + vec2(_S39, _S40)).xyz, _S42);

    float br_0 = dot(sampleClamped_0(uv_11 + vec2(_S38, _S40)).xyz, _S42);

    float _S43 = - dot(sampleClamped_0(uv_11 + vec2(_S39, _S41)).xyz, _S42);

#line 456
    float gx_0 = _S43 - 2.0 * dot(sampleClamped_0(uv_11 + vec2(_S39, 0.0)).xyz, _S42) - bl_0 + tr_0 + 2.0 * dot(sampleClamped_0(uv_11 + vec2(_S38, 0.0)).xyz, _S42) + br_0;
    float gy_0 = _S43 - 2.0 * dot(sampleClamped_0(uv_11 + vec2(0.0, _S41)).xyz, _S42) - tr_0 + bl_0 + 2.0 * dot(sampleClamped_0(uv_11 + vec2(0.0, _S40)).xyz, _S42) + br_0;

#line 462
    float hue_0 = (atan((gy_0),(gx_0))) / 6.28318548202514648 + 0.5;

#line 469
    return vec4(sampleClamped_0(uv_11).xyz * 0.15000000596046448 + saturate_1(abs(fract(vec3(hue_0, hue_0 - 0.3333333432674408, hue_0 + 0.3333333432674408)) * 6.0 - 3.0) - 1.0) * saturate_0(sqrt(gx_0 * gx_0 + gy_0 * gy_0) * max(PS_CONSTANT_BUFFER_0.param0_0, 0.5)), 1.0);
}


#line 477
float halftone_dot_ex_0(vec2 px_2, float spacing_0, float angle_0, float intensity_0, float sharpness_0, out vec2 cellCenterPx_0)
{
    float ca_0 = cos(angle_0);

#line 479
    float sa_0 = sin(angle_0);
    float _S44 = px_2.x;

#line 480
    float _S45 = px_2.y;

#line 480
    vec2 rotated_0 = vec2(_S44 * ca_0 + _S45 * sa_0, - _S44 * sa_0 + _S45 * ca_0);
    float _S46 = spacing_0 * 0.5;

#line 481
    vec2 cell_1 = floor(rotated_0 / spacing_0) * spacing_0 + _S46;

    float _S47 = cell_1.x;

#line 483
    float _S48 = cell_1.y;

#line 483
    cellCenterPx_0 = vec2(_S47 * ca_0 - _S48 * sa_0, _S47 * sa_0 + _S48 * ca_0);

#line 489
    return saturate_0((intensity_0 * _S46 - length(rotated_0 - cell_1)) * (sharpness_0 / max(_S46 * 0.10000000149011612, 0.5)));
}

vec4 effect_halftone_0(vec2 uv_12)
{
    float spacing_1 = max(PS_CONSTANT_BUFFER_0.param0_0, 4.0);
    float sharpness_1 = max(PS_CONSTANT_BUFFER_0.param1_0, 0.5);

    vec2 px_3 = uv_12 / PS_CONSTANT_BUFFER_0.texel_size_0;



    vec2 cc_0;



    float _S49 = halftone_dot_ex_0(px_3, spacing_1, 0.26179999113082886, 0.0, sharpness_1, cc_0);


    float dc_0 = halftone_dot_ex_0(px_3, spacing_1, 0.26179999113082886, 1.0 - sampleClamped_0(cc_0 * PS_CONSTANT_BUFFER_0.texel_size_0).x, sharpness_1, cc_0);


    float _S50 = halftone_dot_ex_0(px_3, spacing_1, 1.30900001525878906, 0.0, sharpness_1, cc_0);


    float dm_0 = halftone_dot_ex_0(px_3, spacing_1, 1.30900001525878906, 1.0 - sampleClamped_0(cc_0 * PS_CONSTANT_BUFFER_0.texel_size_0).y, sharpness_1, cc_0);


    float _S51 = halftone_dot_ex_0(px_3, spacing_1, 0.0, 0.0, sharpness_1, cc_0);


    float dy_0 = halftone_dot_ex_0(px_3, spacing_1, 0.0, 1.0 - sampleClamped_0(cc_0 * PS_CONSTANT_BUFFER_0.texel_size_0).z, sharpness_1, cc_0);


    float _S52 = halftone_dot_ex_0(px_3, spacing_1, 0.78539997339248657, 0.0, sharpness_1, cc_0);
    vec4 srcK_0 = sampleClamped_0(cc_0 * PS_CONSTANT_BUFFER_0.texel_size_0);

    float dk_0 = halftone_dot_ex_0(px_3, spacing_1, 0.78539997339248657, min(1.0 - srcK_0.x, min(1.0 - srcK_0.y, 1.0 - srcK_0.z)) * 0.5, sharpness_1, cc_0);

#line 534
    return vec4(saturate_1(vec3(1.0, 1.0, 1.0) - vec3(dc_0, 0.0, 0.0) - vec3(0.0, dm_0, 0.0) - vec3(0.0, 0.0, dy_0) - vec3(dk_0, dk_0, dk_0)), 1.0);
}




vec4 effect_mouse_edge_0(vec2 uv_13)
{

#line 547
    float _S53 = PS_CONSTANT_BUFFER_0.texel_size_0.x;

#line 547
    float _S54 = - _S53;

#line 547
    float _S55 = PS_CONSTANT_BUFFER_0.texel_size_0.y;

#line 547
    float _S56 = - _S55;

#line 547
    const vec3 _S57 = vec3(0.29899999499320984, 0.58700001239776611, 0.11400000005960464);

    float tr_1 = dot(sampleClamped_0(uv_13 + vec2(_S53, _S56)).xyz, _S57);


    float bl_1 = dot(sampleClamped_0(uv_13 + vec2(_S54, _S55)).xyz, _S57);

    float br_1 = dot(sampleClamped_0(uv_13 + vec2(_S53, _S55)).xyz, _S57);

    float _S58 = - dot(sampleClamped_0(uv_13 + vec2(_S54, _S56)).xyz, _S57);

#line 556
    float gx_1 = _S58 - 2.0 * dot(sampleClamped_0(uv_13 + vec2(_S54, 0.0)).xyz, _S57) - bl_1 + tr_1 + 2.0 * dot(sampleClamped_0(uv_13 + vec2(_S53, 0.0)).xyz, _S57) + br_1;
    float gy_1 = _S58 - 2.0 * dot(sampleClamped_0(uv_13 + vec2(0.0, _S56)).xyz, _S57) - tr_1 + bl_1 + 2.0 * dot(sampleClamped_0(uv_13 + vec2(0.0, _S55)).xyz, _S57) + br_1;

#line 566
    float falloff_0 = saturate_0(1.0 - length(uv_13 / PS_CONSTANT_BUFFER_0.texel_size_0 - PS_CONSTANT_BUFFER_0.mouse_uv_0 / PS_CONSTANT_BUFFER_0.texel_size_0) / max(PS_CONSTANT_BUFFER_0.param0_0, 20.0));



    vec2 dir_0 = normalize(uv_13 - PS_CONSTANT_BUFFER_0.mouse_uv_0 + vec2(0.00009999999747379, 0.0));
    float hue_1 = (atan((dir_0.y),(dir_0.x))) / 6.28318548202514648 + 0.5;

    vec3 highlight_0 = saturate_1(abs(fract(vec3(hue_1, hue_1 - 0.3333333432674408, hue_1 + 0.3333333432674408)) * 6.0 - 3.0) - 1.0);

    float glow_0 = saturate_0(sqrt(gx_1 * gx_1 + gy_1 * gy_1) * max(PS_CONSTANT_BUFFER_0.param1_0, 1.0)) * (falloff_0 * falloff_0);

    vec4 _S59 = sampleClamped_0(uv_13);

#line 577
    vec4 col_6 = _S59;
    col_6.xyz = _S59.xyz + highlight_0 * glow_0;
    return col_6;
}



vec4 effect_crt_0(vec2 uv_14)
{
    float lineThick_0 = max(PS_CONSTANT_BUFFER_0.param0_0, 1.0);



    vec2 centered_0 = (uv_14 - PS_CONSTANT_BUFFER_0.win_center_0) / (PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0);
    float r2_0 = dot(centered_0, centered_0);
    vec2 distorted_0 = uv_14 + centered_0 * r2_0 * PS_CONSTANT_BUFFER_0.param1_0 * PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0 * 0.10000000149011612;


    vec2 px_4 = distorted_0 / PS_CONSTANT_BUFFER_0.texel_size_0;
    float subpixel_0 = fract(px_4.x / 3.0) * 3.0;
    vec3 phosphor_0;
    phosphor_0[0] = saturate_0(1.0 - abs(subpixel_0 - 0.5) * 1.5);
    phosphor_0[1] = saturate_0(1.0 - abs(subpixel_0 - 1.5) * 1.5);
    phosphor_0[2] = saturate_0(1.0 - abs(subpixel_0 - 2.5) * 1.5);
    vec3 _S60 = mix(vec3(1.0, 1.0, 1.0), phosphor_0, vec3(0.60000002384185791));

#line 601
    phosphor_0 = _S60;

    vec3 src_0 = sampleClamped_0(distorted_0).xyz;



    float linePhase_0 = fract(px_4.y / (lineThick_0 * 2.0));

#line 620
    return vec4(saturate_1(src_0 * _S60 * mix(mix(0.15000000596046448, 1.0, smoothstep(0.0, 0.15000000596046448, linePhase_0) * (1.0 - smoothstep(0.34999999403953552, 0.5, linePhase_0))), 1.0, dot(src_0, vec3(0.29899999499320984, 0.58700001239776611, 0.11400000005960464)) * 0.30000001192092896) * (1.0 - r2_0 * 0.40000000596046448)), 1.0);
}



vec4 effect_dot_matrix_0(vec2 uv_15)
{
    float cellPx_0 = max(PS_CONSTANT_BUFFER_0.param0_0, 3.0);


    vec2 px_5 = uv_15 / PS_CONSTANT_BUFFER_0.texel_size_0;

    vec2 cellCenter_0 = (floor(px_5 / cellPx_0) + 0.5) * cellPx_0;
    vec2 inCell_0 = (px_5 - cellCenter_0) / (cellPx_0 * 0.5);

#line 641
    float dist_0 = mix(max(abs(inCell_0.x), abs(inCell_0.y)), length(inCell_0), saturate_0(PS_CONSTANT_BUFFER_0.param1_0));


    vec3 _S61 = sampleClamped_0(cellCenter_0 * PS_CONSTANT_BUFFER_0.texel_size_0).xyz;

#line 644
    float lum_0 = dot(_S61, vec3(0.29899999499320984, 0.58700001239776611, 0.11400000005960464));
    float dotSize_0 = mix(0.5, 0.94999998807907104, lum_0);

#line 655
    return vec4(_S61 * saturate_0((dotSize_0 - dist_0) * cellPx_0 * 0.5) + saturate_0(1.0 - dist_0 / dotSize_0) * 0.15000000596046448 * lum_0, 1.0);
}



vec4 effect_glitch_0(vec2 uv_16)
{
    float intensity_1 = max(PS_CONSTANT_BUFFER_0.param0_0, 0.10000000149011612);
    float blockSize_0 = max(PS_CONSTANT_BUFFER_0.param1_0, 4.0);


    vec2 px_6 = uv_16 / PS_CONSTANT_BUFFER_0.texel_size_0;


    float timeSeed_0 = floor(PS_CONSTANT_BUFFER_0.param2_0 * 8.0);


    float lineBlock_0 = floor(px_6.y / blockSize_0);

#line 672
    float lineShift_0;


    if((hash12_0(vec2(lineBlock_0, timeSeed_0))) > (1.0 - intensity_1 * 0.30000001192092896))
    {

#line 675
        lineShift_0 = (hash12_0(vec2(lineBlock_0 + 0.5, timeSeed_0)) * 2.0 - 1.0) * intensity_1 * 40.0;

#line 675
    }
    else
    {

#line 675
        lineShift_0 = 0.0;

#line 675
    }

#line 680
    vec2 _S62 = floor(px_6 / (blockSize_0 * 4.0)) + timeSeed_0;

#line 680
    float blockHash_0 = hash12_0(_S62);
    vec2 blockShift_0 = vec2(0.0, 0.0);
    if(blockHash_0 > (1.0 - intensity_1 * 0.10000000149011612))
    {
        blockShift_0[0] = (hash12_0(_S62 + 1.0) * 2.0 - 1.0) * intensity_1 * 60.0;
        blockShift_0[1] = (hash12_0(_S62 + 2.0) * 2.0 - 1.0) * intensity_1 * 20.0;

#line 682
    }

#line 688
    vec2 shiftedUV_0 = (px_6 + vec2(lineShift_0, 0.0) + blockShift_0) * PS_CONSTANT_BUFFER_0.texel_size_0;

#line 693
    vec3 col_7;
    vec2 _S63 = vec2(intensity_1 * 3.0 * PS_CONSTANT_BUFFER_0.texel_size_0.x * hash12_0(vec2(timeSeed_0, 0.76999998092651367)), 0.0);

#line 694
    col_7[0] = sampleClamped_0(shiftedUV_0 + _S63).x;
    col_7[1] = sampleClamped_0(shiftedUV_0).y;
    col_7[2] = sampleClamped_0(shiftedUV_0 - _S63).z;



    if((hash12_0(vec2(lineBlock_0 + 3.0, timeSeed_0))) > (1.0 - intensity_1 * 0.05000000074505806))
    {

#line 701
        col_7 = 1.0 - col_7;

#line 700
    }


    return vec4(saturate_1(col_7), 1.0);
}




vec4 effect_stained_glass_0(vec2 uv_17)
{

    float leadW_0 = max(PS_CONSTANT_BUFFER_0.param1_0, 1.0);

    vec2 px_7 = uv_17 / PS_CONSTANT_BUFFER_0.texel_size_0;
    float cellSize_1 = min(PS_CONSTANT_BUFFER_0.win_half_px_0.x, PS_CONSTANT_BUFFER_0.win_half_px_0.y) * 2.0 / max(PS_CONSTANT_BUFFER_0.param0_0, 2.0);
    vec2 _S64 = floor(px_7 / cellSize_1);

#line 733
    float _S65 = cellSize_1 * 0.10000000149011612;

#line 744
    const vec3 _S66 = vec3(0.29899999499320984, 0.58700001239776611, 0.11400000005960464);
    const vec3 _S67 = vec3(1.5);

#line 754
    float _S68 = cellSize_1 * 0.40000000596046448;



    const vec3 _S69 = vec3(0.05000000074505806, 0.05000000074505806, 0.05000000074505806);

#line 758
    float minDist_2 = 1.0e+10;

#line 758
    float secondDist_3 = 1.0e+10;

#line 758
    vec2 closestPt_0 = px_7;

#line 758
    int y_3 = -1;

#line 722
    for(;;)
    {

#line 722
        if(y_3 <= 1)
        {
        }
        else
        {

#line 722
            break;
        }

        float _S70 = float(y_3);

#line 725
        float minDist_3 = minDist_2;

#line 725
        float secondDist_4 = secondDist_3;

#line 725
        vec2 closestPt_1 = closestPt_0;

#line 725
        int x_4 = -1;

#line 723
        for(;;)
        {

#line 723
            if(x_4 <= 1)
            {
            }
            else
            {

#line 723
                break;
            }
            vec2 neighbor_1 = _S64 + vec2(float(x_4), _S70);
            vec2 pt_0 = (neighbor_1 + hash22_0(neighbor_1)) * cellSize_1;
            float d_4 = length(px_7 - pt_0);
            if(d_4 < minDist_3)
            {

#line 718
                float _S71 = minDist_3;

#line 718
                minDist_3 = d_4;

#line 718
                secondDist_4 = _S71;

#line 718
                closestPt_1 = pt_0;

#line 728
            }
            else
            {

#line 728
                float secondDist_5;
                if(d_4 < secondDist_4)
                {

#line 729
                    secondDist_5 = d_4;

#line 729
                }
                else
                {

#line 729
                    secondDist_5 = secondDist_4;

#line 729
                }

#line 729
                secondDist_4 = secondDist_5;

#line 728
            }

#line 723
            x_4 = x_4 + 1;

#line 723
        }

#line 722
        int y_4 = y_3 + 1;

#line 722
        minDist_2 = minDist_3;

#line 722
        secondDist_3 = secondDist_4;

#line 722
        closestPt_0 = closestPt_1;

#line 722
        y_3 = y_4;

#line 722
    }

#line 733
    vec2 centroidUV_0 = applyParallax_0(closestPt_0 * PS_CONSTANT_BUFFER_0.texel_size_0, _S65);

    vec2 ts2_0 = PS_CONSTANT_BUFFER_0.texel_size_0 * 2.0;

    vec2 _S72 = vec2(ts2_0.x, 0.0);

    vec2 _S73 = vec2(0.0, ts2_0.y);

    vec3 col_8 = (sampleClamped_0(centroidUV_0).xyz + sampleClamped_0(centroidUV_0 + _S72).xyz + sampleClamped_0(centroidUV_0 - _S72).xyz + sampleClamped_0(centroidUV_0 + _S73).xyz + sampleClamped_0(centroidUV_0 - _S73).xyz) * 0.20000000298023224;


    float lum_1 = dot(col_8, _S66);

#line 750
    float lead_0 = saturate_0((secondDist_3 - minDist_2) / leadW_0);
    float lead_1 = lead_0 * lead_0;

#line 760
    return vec4(mix(_S69, saturate_1(mix(vec3(lum_1, lum_1, lum_1), col_8, _S67)) * lead_1 * 0.89999997615814209 + pow(saturate_0(1.0 - minDist_2 / _S68), 8.0) * 0.20000000298023224, vec3(lead_1)), 1.0);
}


#line 779
vec3 N13_0(float p_4)
{
    vec3 p3_4 = fract(vec3(p_4, p_4, p_4) * vec3(0.1031000018119812, 0.11368999630212784, 0.13786999881267548));
    vec3 p3_5 = p3_4 + dot(p3_4, p3_4.yzx + 19.19000053405761719);
    float _S74 = p3_5.x;

#line 783
    float _S75 = p3_5.y;

#line 783
    float _S76 = p3_5.z;

#line 783
    return fract(vec3((_S74 + _S75) * _S76, (_S74 + _S76) * _S75, (_S75 + _S76) * _S74));
}


#line 774
float S_0(float a_0, float b_1, float t_0)
{

#line 774
    float _S77;

    if(a_0 < b_1)
    {

#line 776
        _S77 = smoothstep(a_0, b_1, t_0);

#line 776
    }
    else
    {

#line 776
        _S77 = 1.0 - smoothstep(b_1, a_0, t_0);

#line 776
    }

#line 776
    return _S77;
}


#line 786
float Saw_0(float b_2, float t_1)
{
    return S_0(0.0, b_2, t_1) * S_0(1.0, b_2, t_1);
}


#line 846
float StaticDrops_0(vec2 uv_18, float time_0)
{
    vec2 _S78 = uv_18 * 40.0;
    vec2 id_0 = floor(_S78);

    vec3 n_0 = N13_0(id_0.x * 107.4499969482421875 + id_0.y * 3543.654052734375);


    float _S79 = n_0.z;
    return S_0(0.30000001192092896, 0.0, length(fract(_S78) - 0.5 - (n_0.xy - 0.5) * 0.69999998807907104)) * fract(_S79 * 10.0) * Saw_0(0.02500000037252903, fract(time_0 + _S79));
}


#line 794
vec2 DropLayer_0(vec2 uv_19, float time_1)
{

#line 794
    vec2 _S80 = uv_19;



    float _S81 = _S80[1] + time_1 * 0.75;

#line 798
    _S80[1] = _S81;

    const vec2 grid_0 = vec2(6.0, 1.0) * 2.0;

#line 805
    _S80[1] = _S81 + fract(sin(floor(_S80 * grid_0).x * 12345.564453125) * 7658.759765625);

    vec2 id_1 = floor(_S80 * grid_0);
    vec3 n_1 = N13_0(id_1.x * 35.20000076293945312 + id_1.y * 2376.10009765625);
    vec2 st_0 = fract(_S80 * grid_0) - vec2(0.5, 0.0);


    float x_5 = n_1.x - 0.5;
    float _S82 = uv_19.y;

#line 813
    float y_5 = _S82 * 20.0;

    float _S83 = n_1.z;
    float x_6 = (x_5 + sin(y_5 + sin(y_5)) * (0.5 - abs(x_5)) * (_S83 - 0.5)) * 0.69999998807907104;



    float y_6 = (Saw_0(0.85000002384185791, fract(time_1 + _S83)) - 0.5) * 0.89999997615814209 + 0.5;

#line 829
    float _S84 = st_0.y;

#line 829
    float r_2 = sqrt(S_0(1.0, y_6, _S84));


    float trailFront_0 = S_0(-0.01999999955296516, 0.01999999955296516, _S84 - y_6);

#line 842
    return vec2(S_0(0.40000000596046448, 0.0, length((st_0 - vec2(x_6, y_6)) * vec2(1.0, 6.0))) + S_0(0.30000001192092896, 0.0, length(st_0 - vec2(x_6, fract(_S82 * 10.0) + (_S84 - 0.5)))) * r_2 * trailFront_0, S_0(0.23000000417232513 * r_2, 0.15000000596046448 * r_2 * r_2, abs(st_0.x - x_6)) * (trailFront_0 * r_2 * r_2));
}


#line 859
vec2 RainDrops_0(vec2 uv_20, float time_2, float rainAmount_0)
{

    float t_2 = - time_2;

    float l0_0 = smoothstep(0.0, 0.5, rainAmount_0);
    float l1_0 = smoothstep(0.25, 0.75, rainAmount_0);



    vec2 m1_0 = DropLayer_0(uv_20, t_2) * l1_0;
    vec2 m2_0 = DropLayer_0(uv_20 * 1.85000002384185791, t_2) * l0_0;

#line 875
    return vec2(smoothstep(0.30000001192092896, 1.0, StaticDrops_0(uv_20, t_2) * l0_0 + m1_0.x + m2_0.x), max(m1_0.y * l0_0, m2_0.y * l1_0));
}


#line 76
float srgbToLinear_0(float c_0)
{

#line 76
    float _S85;

    if(c_0 <= 0.04044999927282333)
    {

#line 78
        _S85 = c_0 / 12.92000007629394531;

#line 78
    }
    else
    {

#line 78
        _S85 = pow((c_0 + 0.05499999970197678) / 1.0549999475479126, 2.40000009536743164);

#line 78
    }

#line 78
    return _S85;
}




vec3 srgbToLinear3_0(vec3 c_1)
{

#line 84
    return vec3(srgbToLinear_0(c_1.x), srgbToLinear_0(c_1.y), srgbToLinear_0(c_1.z));
}


#line 80
float linearToSrgb_0(float c_2)
{

#line 80
    float _S86;

    if(c_2 <= 0.00313080009073019)
    {

#line 82
        _S86 = c_2 * 12.92000007629394531;

#line 82
    }
    else
    {

#line 82
        _S86 = 1.0549999475479126 * pow(c_2, 0.4166666567325592) - 0.05499999970197678;

#line 82
    }

#line 82
    return _S86;
}

vec3 linearToSrgb3_0(vec3 c_3)
{

#line 85
    return vec3(linearToSrgb_0(c_3.x), linearToSrgb_0(c_3.y), linearToSrgb_0(c_3.z));
}


#line 878
vec4 effect_rain_0(vec2 uv_21)
{
    float rainAmount_1 = saturate_0(PS_CONSTANT_BUFFER_0.param0_0);
    float fogBlur_0 = max(PS_CONSTANT_BUFFER_0.param1_0, 0.0);
    float time_3 = PS_CONSTANT_BUFFER_0.param2_0;



    vec2 winUV_0 = (uv_21 - PS_CONSTANT_BUFFER_0.win_center_0) / (PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0) * 0.5 + 0.5;

    winUV_0[0] = winUV_0[0] * (PS_CONSTANT_BUFFER_0.win_half_px_0.x / max(PS_CONSTANT_BUFFER_0.win_half_px_0.y, 1.0));

    vec2 c_4 = RainDrops_0(winUV_0, time_3, rainAmount_1);

#line 896
    float _S87 = c_4.x;



    float focus_0 = mix(mix(fogBlur_0, 0.0, smoothstep(0.10000000149011612, 0.20000000298023224, _S87)), fogBlur_0 * 0.5, c_4.y);


    vec2 refractUV_0 = uv_21 + vec2(RainDrops_0(winUV_0 + vec2(0.0020000000949949, 0.0), time_3, rainAmount_1).x - _S87, RainDrops_0(winUV_0 + vec2(0.0, 0.0020000000949949), time_3, rainAmount_1).x - _S87) * PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0 * 0.5;


    int R_0 = clamp(int(focus_0 * 2.0 + 0.5), 0, 4);

#line 906
    vec3 blurred_0;


    if(R_0 > 0)
    {
        const vec3 _S88 = vec3(0.0, 0.0, 0.0);

        float _S89 = max(focus_0 * 0.5, 1.0);
        int _S90 = - R_0;

#line 914
        int ky_0 = _S90;

#line 914
        blurred_0 = _S88;

#line 914
        float count_0 = 0.0;

#line 914
        for(;;)
        {

#line 914
            if(ky_0 <= R_0)
            {
            }
            else
            {

#line 914
                break;
            }

            float _S91 = float(ky_0);

#line 917
            int kx_0 = _S90;

#line 915
            for(;;)
            {

#line 915
                if(kx_0 <= R_0)
                {
                }
                else
                {

#line 915
                    break;
                }

                vec3 acc_0 = blurred_0 + srgbToLinear3_0(sampleClamped_0(refractUV_0 + vec2(float(kx_0), _S91) * _S89 * PS_CONSTANT_BUFFER_0.texel_size_0).xyz);
                float count_1 = count_0 + 1.0;

#line 915
                kx_0 = kx_0 + 1;

#line 915
                blurred_0 = acc_0;

#line 915
                count_0 = count_1;

#line 915
            }

#line 914
            ky_0 = ky_0 + 1;

#line 914
        }

#line 914
        blurred_0 = linearToSrgb3_0(blurred_0 / count_0);

#line 909
    }
    else
    {

#line 909
        blurred_0 = sampleClamped_0(refractUV_0).xyz;

#line 909
    }

#line 933
    vec4 col_9;
    col_9.xyz = mix(blurred_0, sampleClamped_0(refractUV_0).xyz, vec3(smoothstep(0.10000000149011612, 0.30000001192092896, _S87)));
    col_9[3] = 1.0;

    return col_9;
}



vec4 effect_kaleidoscope_0(vec2 uv_22)
{

    float rotation_0 = PS_CONSTANT_BUFFER_0.param1_0 + PS_CONSTANT_BUFFER_0.param2_0 * 0.30000001192092896;


    vec2 centered_1 = (uv_22 - PS_CONSTANT_BUFFER_0.win_center_0) / (PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0);


    float ca_1 = cos(rotation_0);

#line 951
    float sa_1 = sin(rotation_0);
    float _S92 = centered_1.x;

#line 952
    float _S93 = centered_1.y;

#line 952
    float _S94 = _S92 * ca_1 + _S93 * sa_1;

#line 952
    float _S95 = - _S92 * sa_1 + _S93 * ca_1;


    float angle_1 = (atan((_S95),(_S94)));



    float segAngle_0 = 6.28318548202514648 / max(PS_CONSTANT_BUFFER_0.param0_0, 2.0);
    float _S96 = segAngle_0 * 0.5;

#line 960
    float angle_2 = abs(((((angle_1) < 0.0) ? -mod(-(angle_1),abs((segAngle_0))) : mod((angle_1),abs((segAngle_0))))) - _S96);

#line 967
    vec4 _S97 = sampleClamped_0(vec2(cos(angle_2), sin(angle_2)) * length(vec2(_S94, _S95)) * PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0 + PS_CONSTANT_BUFFER_0.win_center_0);

#line 967
    vec4 col_10 = _S97;


    float _S98 = angle_1 + 3.14159274101257324;

    col_10.xyz = _S97.xyz + pow(saturate_0(1.0 - abs(((((_S98) < 0.0) ? -mod(-(_S98),abs((segAngle_0))) : mod((_S98),abs((segAngle_0))))) - _S96) / (segAngle_0 * 0.05000000074505806)), 4.0) * 0.15000000596046448;

    return col_10;
}


#line 88
vec4 sampleLinear_0(vec2 uv_23)
{
    vec4 _S99 = sampleClamped_0(uv_23);

#line 90
    vec4 c_5 = _S99;
    c_5.xyz = srgbToLinear3_0(_S99.xyz);
    return c_5;
}


#line 149
vec4 effect_blur_0(vec2 uv_24)
{
    float radius_0 = PS_CONSTANT_BUFFER_0.param0_0;

#line 151
    vec2 _S100;
    if((PS_CONSTANT_BUFFER_0.param1_0) < 0.5)
    {

#line 152
        _S100 = vec2(PS_CONSTANT_BUFFER_0.texel_size_0.x, 0.0);

#line 152
    }
    else
    {

#line 152
        _S100 = vec2(0.0, PS_CONSTANT_BUFFER_0.texel_size_0.y);

#line 152
    }

    float bevelPx_2 = max(radius_0 * 2.0, 8.0);
    vec2 normal_3;
    float d_5 = sdfWithNormal_0(uv_24, 0.0, normal_3);


    vec2 _S101 = uv_24 + normal_3 * (saturate_0(1.0 - saturate_0(- d_5 / bevelPx_2)) * radius_0 * 0.5) * PS_CONSTANT_BUFFER_0.texel_size_0;
    float edgeHighlight_0 = pow(saturate_0(1.0 - abs(d_5) / (bevelPx_2 * 0.5)), 3.0) * 0.15000000596046448;

    float _S102 = radius_0 / 6.0;
    const float  w_1[7] = { 0.19648249447345734, 0.17489039897918701, 0.1225150004029274, 0.06753169745206833, 0.0292431004345417, 0.00995950028300285, 0.00266269990243018 };


    vec3 _S103 = sampleLinear_0(_S101).xyz * 0.19648249447345734;

#line 166
    int i_1 = 1;

#line 166
    vec3 lin_0 = _S103;
    for(;;)
    {

#line 167
        if(i_1 < 7)
        {
        }
        else
        {

#line 167
            break;
        }
        vec2 off_1 = _S100 * (float(i_1) * _S102);

        vec3 lin_1 = lin_0 + sampleLinear_0(_S101 + off_1).xyz * w_1[i_1] + sampleLinear_0(_S101 - off_1).xyz * w_1[i_1];

#line 167
        i_1 = i_1 + 1;

#line 167
        lin_0 = lin_1;

#line 167
    }

#line 175
    vec4 col_11;
    col_11.xyz = linearToSrgb3_0(lin_0 + edgeHighlight_0);
    col_11[3] = 1.0;
    return col_11;
}


#line 178
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 178
layout(location = 0)
in vec4 input_col_0;


#line 178
layout(location = 1)
in vec2 input_uv_0;


#line 977
void main()
{

    vec4 col_12;

    int m_0 = int(PS_CONSTANT_BUFFER_0.mode_0);
    if(m_0 == 1)
    {

#line 983
        col_12 = effect_glass_refract_0(input_uv_0);

#line 983
    }
    else
    {

#line 984
        if(m_0 == 2)
        {

#line 984
            col_12 = effect_frosted_0(input_uv_0);

#line 984
        }
        else
        {

#line 985
            if(m_0 == 3)
            {

#line 985
                col_12 = effect_pixelate_0(input_uv_0);

#line 985
            }
            else
            {

#line 986
                if(m_0 == 4)
                {

#line 986
                    col_12 = effect_chromatic_0(input_uv_0);

#line 986
                }
                else
                {

#line 987
                    if(m_0 == 5)
                    {

#line 987
                        col_12 = effect_liquid_glass_0(input_uv_0);

#line 987
                    }
                    else
                    {

#line 988
                        if(m_0 == 6)
                        {

#line 988
                            col_12 = effect_heat_haze_0(input_uv_0);

#line 988
                        }
                        else
                        {

#line 989
                            if(m_0 == 7)
                            {

#line 989
                                col_12 = effect_voronoi_0(input_uv_0);

#line 989
                            }
                            else
                            {

#line 990
                                if(m_0 == 8)
                                {

#line 990
                                    col_12 = effect_edge_glow_0(input_uv_0);

#line 990
                                }
                                else
                                {

#line 991
                                    if(m_0 == 9)
                                    {

#line 991
                                        col_12 = effect_halftone_0(input_uv_0);

#line 991
                                    }
                                    else
                                    {

#line 992
                                        if(m_0 == 10)
                                        {

#line 992
                                            col_12 = effect_mouse_edge_0(input_uv_0);

#line 992
                                        }
                                        else
                                        {

#line 993
                                            if(m_0 == 11)
                                            {

#line 993
                                                col_12 = effect_crt_0(input_uv_0);

#line 993
                                            }
                                            else
                                            {

#line 994
                                                if(m_0 == 12)
                                                {

#line 994
                                                    col_12 = effect_dot_matrix_0(input_uv_0);

#line 994
                                                }
                                                else
                                                {

#line 995
                                                    if(m_0 == 13)
                                                    {

#line 995
                                                        col_12 = effect_glitch_0(input_uv_0);

#line 995
                                                    }
                                                    else
                                                    {

#line 996
                                                        if(m_0 == 14)
                                                        {

#line 996
                                                            col_12 = effect_stained_glass_0(input_uv_0);

#line 996
                                                        }
                                                        else
                                                        {

#line 997
                                                            if(m_0 == 15)
                                                            {

#line 997
                                                                col_12 = effect_rain_0(input_uv_0);

#line 997
                                                            }
                                                            else
                                                            {

#line 998
                                                                if(m_0 == 16)
                                                                {

#line 998
                                                                    col_12 = effect_kaleidoscope_0(input_uv_0);

#line 998
                                                                }
                                                                else
                                                                {

#line 999
                                                                    col_12 = effect_blur_0(input_uv_0);

#line 998
                                                                }

#line 997
                                                            }

#line 996
                                                        }

#line 995
                                                    }

#line 994
                                                }

#line 993
                                            }

#line 992
                                        }

#line 991
                                    }

#line 990
                                }

#line 989
                            }

#line 988
                        }

#line 987
                    }

#line 986
                }

#line 985
            }

#line 984
        }

#line 983
    }

#line 1001
    col_12[3] = 1.0;

#line 1001
    entryPointParam_main_ps_0 = col_12 * input_col_0;

#line 1001
    return;
}

