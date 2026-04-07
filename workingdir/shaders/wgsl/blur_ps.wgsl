struct SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    @align(16) texel_size_0 : vec2<f32>,
    @align(8) mode_0 : f32,
    @align(4) param0_0 : f32,
    @align(16) param1_0 : f32,
    @align(4) param2_0 : f32,
    @align(8) win_center_0 : vec2<f32>,
    @align(16) win_half_px_0 : vec2<f32>,
    @align(8) win_rounding_0 : f32,
    @align(4) pad0_0 : f32,
    @align(16) mouse_uv_0 : vec2<f32>,
    @align(8) pad1_0 : f32,
    @align(4) pad2_0 : f32,
};

@binding(1) @group(0) var<uniform> PS_CONSTANT_BUFFER_0 : SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0;
@binding(0) @group(0) var sceneTexture_0 : texture_2d<f32>;

@binding(0) @group(0) var sceneSampler_0 : sampler;

fn uvToPixel_0( uv_0 : vec2<f32>) -> vec2<f32>
{
    return (uv_0 - PS_CONSTANT_BUFFER_0.win_center_0) / PS_CONSTANT_BUFFER_0.texel_size_0;
}

fn sdRoundedBox_0( p_0 : vec2<f32>,  b_0 : vec2<f32>,  r_0 : f32) -> f32
{
    var q_0 : vec2<f32> = abs(p_0) - b_0 + vec2<f32>(r_0);
    return length(max(q_0, vec2<f32>(0.0f))) + min(max(q_0.x, q_0.y), 0.0f) - r_0;
}

fn sdfWithNormal_0( uv_1 : vec2<f32>,  extraRadius_0 : f32,  normal_0 : ptr<function, vec2<f32>>) -> f32
{
    var p_1 : vec2<f32> = uvToPixel_0(uv_1);
    var r_1 : f32 = PS_CONSTANT_BUFFER_0.win_rounding_0 + extraRadius_0;
    var d_0 : f32 = sdRoundedBox_0(p_1, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1);
    const _S1 : vec2<f32> = vec2<f32>(1.0f, 0.0f);
    const _S2 : vec2<f32> = vec2<f32>(0.0f, 1.0f);
    (*normal_0) = normalize(vec2<f32>(sdRoundedBox_0(p_1 + _S1, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1) - sdRoundedBox_0(p_1 - _S1, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1), sdRoundedBox_0(p_1 + _S2, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1) - sdRoundedBox_0(p_1 - _S2, PS_CONSTANT_BUFFER_0.win_half_px_0, r_1)) + vec2<f32>(0.00009999999747379f, 0.0f));
    return d_0;
}

fn sampleClamped_0( uv_2 : vec2<f32>) -> vec4<f32>
{
    var _S3 : vec2<f32> = vec2<f32>(0.5f);
    return (textureSample((sceneTexture_0), (sceneSampler_0), (clamp(uv_2, PS_CONSTANT_BUFFER_0.texel_size_0 * _S3, vec2<f32>(1.0f) - PS_CONSTANT_BUFFER_0.texel_size_0 * _S3))));
}

fn effect_glass_refract_0( uv_3 : vec2<f32>) -> vec4<f32>
{
    var eta_0 : f32 = 1.0f / max(PS_CONSTANT_BUFFER_0.param1_0, 1.00999999046325684f);
    var bevelPx_0 : f32 = max(PS_CONSTANT_BUFFER_0.param0_0, 0.00999999977648258f) * min(PS_CONSTANT_BUFFER_0.win_half_px_0.x, PS_CONSTANT_BUFFER_0.win_half_px_0.y);
    var normal_1 : vec2<f32>;
    var d_1 : f32 = sdfWithNormal_0(uv_3, 0.0f, &(normal_1));
    var curvature_0 : f32 = saturate(1.0f + d_1 / bevelPx_0);
    var ruv_0 : vec2<f32> = uv_3 + normal_1 * vec2<f32>(curvature_0) * vec2<f32>((1.0f - eta_0)) * vec2<f32>(bevelPx_0) * PS_CONSTANT_BUFFER_0.texel_size_0;
    var _S4 : vec4<f32> = sampleClamped_0(ruv_0) * vec4<f32>(0.5f);
    var col_0 : vec4<f32> = _S4;
    const _S5 : vec2<f32> = vec2<f32>(1.0f, 0.0f);
    var _S6 : vec4<f32> = vec4<f32>(0.125f);
    var _S7 : vec4<f32> = _S4 + sampleClamped_0(ruv_0 + PS_CONSTANT_BUFFER_0.texel_size_0 * _S5) * _S6;
    col_0 = _S7;
    var _S8 : vec4<f32> = _S7 + sampleClamped_0(ruv_0 - PS_CONSTANT_BUFFER_0.texel_size_0 * _S5) * _S6;
    col_0 = _S8;
    const _S9 : vec2<f32> = vec2<f32>(0.0f, 1.0f);
    var _S10 : vec4<f32> = _S8 + sampleClamped_0(ruv_0 + PS_CONSTANT_BUFFER_0.texel_size_0 * _S9) * _S6;
    col_0 = _S10;
    var _S11 : vec4<f32> = _S10 + sampleClamped_0(ruv_0 - PS_CONSTANT_BUFFER_0.texel_size_0 * _S9) * _S6;
    col_0 = _S11;
    var _S12 : vec3<f32> = _S11.xyz + vec3<f32>((pow(curvature_0, 2.0f) * 0.30000001192092896f));
    col_0.x = _S12.x;
    col_0.y = _S12.y;
    col_0.z = _S12.z;
    return col_0;
}

fn applyParallax_0( uv_4 : vec2<f32>,  strength_0 : f32) -> vec2<f32>
{
    var _S13 : vec2<f32> = vec2<f32>(2.0f);
    return uv_4 + (uv_4 - PS_CONSTANT_BUFFER_0.win_center_0) / max(PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0 * _S13, vec2<f32>(0.00100000004749745f, 0.00100000004749745f)) * vec2<f32>(strength_0) * PS_CONSTANT_BUFFER_0.texel_size_0 * _S13;
}

fn hash12_0( p_2 : vec2<f32>) -> f32
{
    var p3_0 : vec3<f32> = fract(p_2.xyx * vec3<f32>(0.1031000018119812f));
    var p3_1 : vec3<f32> = p3_0 + vec3<f32>(dot(p3_0, p3_0.yzx + vec3<f32>(33.3300018310546875f)));
    return fract((p3_1.x + p3_1.y) * p3_1.z);
}

fn effect_frosted_0( uv_5 : vec2<f32>) -> vec4<f32>
{
    var _S14 : vec2<f32> = applyParallax_0(uv_5, PS_CONSTANT_BUFFER_0.param0_0);
    var noise_uv_0 : vec2<f32> = _S14 / PS_CONSTANT_BUFFER_0.texel_size_0;
    var off_0 : vec2<f32> = PS_CONSTANT_BUFFER_0.texel_size_0 * vec2<f32>(PS_CONSTANT_BUFFER_0.param0_0) * vec2<f32>(0.5f);
    var suv_0 : vec2<f32> = _S14 + vec2<f32>(hash12_0(floor(noise_uv_0 * vec2<f32>(PS_CONSTANT_BUFFER_0.param1_0))) * 2.0f - 1.0f, hash12_0(floor(noise_uv_0 * vec2<f32>(PS_CONSTANT_BUFFER_0.param1_0)) + vec2<f32>(7.13000011444091797f, 3.71000003814697266f)) * 2.0f - 1.0f) * vec2<f32>(PS_CONSTANT_BUFFER_0.param0_0) * PS_CONSTANT_BUFFER_0.texel_size_0;
    var _S15 : f32 = off_0.x;
    var _S16 : f32 = - _S15;
    var _S17 : f32 = off_0.y;
    var _S18 : f32 = - _S17;
    return (sampleClamped_0(suv_0) + sampleClamped_0(suv_0 + vec2<f32>(_S16, _S17)) + sampleClamped_0(suv_0 + vec2<f32>(_S15, _S17)) + sampleClamped_0(suv_0 + vec2<f32>(_S15, _S18)) + sampleClamped_0(suv_0 + vec2<f32>(_S16, _S18))) * vec4<f32>(0.20000000298023224f);
}

fn effect_pixelate_0( uv_6 : vec2<f32>) -> vec4<f32>
{
    var block_0 : vec2<f32> = PS_CONSTANT_BUFFER_0.texel_size_0 * vec2<f32>(max(PS_CONSTANT_BUFFER_0.param0_0, 1.0f));
    return sampleClamped_0(floor(applyParallax_0(uv_6, PS_CONSTANT_BUFFER_0.param0_0) / block_0) * block_0 + block_0 * vec2<f32>(0.5f));
}

fn wavelengthToRGB_0( lambda_0 : f32) -> vec3<f32>
{
    var _S19 : f32 = lambda_0 - 442.0f;
    var _S20 : f32;
    if(lambda_0 < 442.0f)
    {
        _S20 = 0.06239999830722809f;
    }
    else
    {
        _S20 = 0.03739999979734421f;
    }
    var t1_0 : f32 = _S19 * _S20;
    var _S21 : f32 = lambda_0 - 599.79998779296875f;
    if(lambda_0 < 599.79998779296875f)
    {
        _S20 = 0.02639999985694885f;
    }
    else
    {
        _S20 = 0.03229999914765358f;
    }
    var t2_0 : f32 = _S21 * _S20;
    var _S22 : f32 = lambda_0 - 501.100006103515625f;
    if(lambda_0 < 501.100006103515625f)
    {
        _S20 = 0.04899999871850014f;
    }
    else
    {
        _S20 = 0.03819999843835831f;
    }
    var t3_0 : f32 = _S22 * _S20;
    var x_0 : f32 = 0.3619999885559082f * exp(-0.5f * t1_0 * t1_0) + 1.0559999942779541f * exp(-0.5f * t2_0 * t2_0) - 0.06499999761581421f * exp(-0.5f * t3_0 * t3_0);
    var _S23 : f32 = lambda_0 - 568.79998779296875f;
    if(lambda_0 < 568.79998779296875f)
    {
        _S20 = 0.02129999920725822f;
    }
    else
    {
        _S20 = 0.02470000088214874f;
    }
    var t4_0 : f32 = _S23 * _S20;
    var _S24 : f32 = lambda_0 - 530.9000244140625f;
    if(lambda_0 < 530.9000244140625f)
    {
        _S20 = 0.06129999831318855f;
    }
    else
    {
        _S20 = 0.03220000118017197f;
    }
    var t5_0 : f32 = _S24 * _S20;
    var y_0 : f32 = 0.82099997997283936f * exp(-0.5f * t4_0 * t4_0) + 0.28600001335144043f * exp(-0.5f * t5_0 * t5_0);
    var _S25 : f32 = lambda_0 - 437.0f;
    if(lambda_0 < 437.0f)
    {
        _S20 = 0.08449999988079071f;
    }
    else
    {
        _S20 = 0.02779999934136868f;
    }
    var t6_0 : f32 = _S25 * _S20;
    var _S26 : f32 = lambda_0 - 459.0f;
    if(lambda_0 < 459.0f)
    {
        _S20 = 0.03849999979138374f;
    }
    else
    {
        _S20 = 0.07249999791383743f;
    }
    var t7_0 : f32 = _S26 * _S20;
    var z_0 : f32 = 1.21700000762939453f * exp(-0.5f * t6_0 * t6_0) + 0.6809999942779541f * exp(-0.5f * t7_0 * t7_0);
    var rgb_0 : vec3<f32>;
    rgb_0[i32(0)] = 3.2406001091003418f * x_0 - 1.53719997406005859f * y_0 - 0.49860000610351562f * z_0;
    rgb_0[i32(1)] = -0.96890002489089966f * x_0 + 1.87580001354217529f * y_0 + 0.04149999842047691f * z_0;
    rgb_0[i32(2)] = 0.05570000037550926f * x_0 - 0.20399999618530273f * y_0 + 1.05700004100799561f * z_0;
    return max(rgb_0, vec3<f32>(0.0f));
}

fn effect_chromatic_0( uv_7 : vec2<f32>) -> vec4<f32>
{
    var toUV_0 : vec2<f32> = uv_7 - PS_CONSTANT_BUFFER_0.win_center_0;
    var _S27 : vec2<f32> = toUV_0 * vec2<f32>(length(toUV_0 / max(PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0, vec2<f32>(0.00100000004749745f, 0.00100000004749745f)))) * vec2<f32>(PS_CONSTANT_BUFFER_0.param0_0) * PS_CONSTANT_BUFFER_0.texel_size_0 / vec2<f32>(max(length(toUV_0), 0.00009999999747379f));
    var N_0 : i32 = clamp(i32(PS_CONSTANT_BUFFER_0.param1_0), i32(4), i32(16));
    var N_1 : i32;
    if(N_0 < i32(4))
    {
        N_1 = i32(8);
    }
    else
    {
        N_1 = N_0;
    }
    const _S28 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    var _S29 : f32 = f32(N_1 - i32(1));
    const _S30 : vec3<f32> = vec3<f32>(0.00100000004749745f, 0.00100000004749745f, 0.00100000004749745f);
    var i_0 : i32 = i32(0);
    var col_1 : vec3<f32> = _S28;
    var totalWeight_0 : vec3<f32> = _S28;
    for(;;)
    {
        if(i_0 < N_1)
        {
        }
        else
        {
            break;
        }
        var lambda_1 : f32 = mix(380.0f, 780.0f, f32(i_0) / _S29);
        var w_0 : vec3<f32> = wavelengthToRGB_0(lambda_1);
        var col_2 : vec3<f32> = col_1 + sampleClamped_0(uv_7 + _S27 * vec2<f32>((3.025e+05f / (lambda_1 * lambda_1) - 1.0f))).xyz * w_0;
        var totalWeight_1 : vec3<f32> = totalWeight_0 + w_0;
        i_0 = i_0 + i32(1);
        col_1 = col_2;
        totalWeight_0 = totalWeight_1;
    }
    return vec4<f32>(col_1 / max(totalWeight_0, _S30), 1.0f);
}

fn effect_liquid_glass_0( uv_8 : vec2<f32>) -> vec4<f32>
{
    var strength_1 : f32 = max(PS_CONSTANT_BUFFER_0.param0_0, 0.00999999977648258f);
    var bevelPx_1 : f32 = max(PS_CONSTANT_BUFFER_0.param1_0, 0.00999999977648258f) * min(PS_CONSTANT_BUFFER_0.win_half_px_0.x, PS_CONSTANT_BUFFER_0.win_half_px_0.y);
    var normal_2 : vec2<f32>;
    var d_2 : f32 = sdfWithNormal_0(uv_8, 0.0f, &(normal_2));
    var inside_0 : f32 = saturate(- d_2 / bevelPx_1);
    var curvature_1 : f32 = 1.0f - inside_0;
    var ruv_1 : vec2<f32> = uv_8 + normal_2 * vec2<f32>(curvature_1) * vec2<f32>(strength_1) * vec2<f32>(bevelPx_1) * PS_CONSTANT_BUFFER_0.texel_size_0;
    var col_3 : vec4<f32> = sampleClamped_0(ruv_1);
    var dOff_0 : vec2<f32> = normal_2 * vec2<f32>((curvature_1 * strength_1 * bevelPx_1 * 0.30000001192092896f * PS_CONSTANT_BUFFER_0.texel_size_0.x));
    col_3[i32(0)] = sampleClamped_0(ruv_1 + dOff_0).x;
    col_3[i32(2)] = sampleClamped_0(ruv_1 - dOff_0).z;
    var _S31 : vec3<f32> = col_3.xyz + vec3<f32>((pow(saturate(1.0f - abs(d_2 + bevelPx_1 * 0.30000001192092896f) / (bevelPx_1 * 0.15000000596046448f)), 4.0f) * 0.40000000596046448f));
    col_3.x = _S31.x;
    col_3.y = _S31.y;
    col_3.z = _S31.z;
    var _S32 : vec3<f32> = col_3.xyz + vec3<f32>((pow(curvature_1, 3.0f) * 0.25f));
    col_3.x = _S32.x;
    col_3.y = _S32.y;
    col_3.z = _S32.z;
    var _S33 : vec3<f32> = col_3.xyz * vec3<f32>(mix(0.85000002384185791f, 1.0f, inside_0));
    col_3.x = _S33.x;
    col_3.y = _S33.y;
    col_3.z = _S33.z;
    return col_3;
}

fn effect_heat_haze_0( uv_9 : vec2<f32>) -> vec4<f32>
{
    var amplitude_0 : f32 = max(PS_CONSTANT_BUFFER_0.param0_0, 0.5f);
    var freq_0 : f32 = max(PS_CONSTANT_BUFFER_0.param1_0, 1.0f);
    var px_0 : vec2<f32> = uv_9 / PS_CONSTANT_BUFFER_0.texel_size_0;
    var _S34 : f32 = px_0.y * freq_0;
    var _S35 : f32 = px_0.x * freq_0;
    var wave1_0 : f32 = sin(_S34 * 0.05000000074505806f + PS_CONSTANT_BUFFER_0.param2_0 * 2.29999995231628418f) * cos(_S35 * 0.02999999932944775f + PS_CONSTANT_BUFFER_0.param2_0 * 1.70000004768371582f);
    var wave2_0 : f32 = sin(_S34 * 0.07999999821186066f - PS_CONSTANT_BUFFER_0.param2_0 * 1.89999997615814209f + 1.5f) * cos(_S35 * 0.03999999910593033f - PS_CONSTANT_BUFFER_0.param2_0 * 2.09999990463256836f);
    var distort_0 : vec2<f32>;
    distort_0[i32(0)] = (wave1_0 * 0.60000002384185791f + wave2_0 * 0.40000000596046448f) * amplitude_0 * PS_CONSTANT_BUFFER_0.texel_size_0.x;
    distort_0[i32(1)] = (wave2_0 * 0.60000002384185791f + wave1_0 * 0.40000000596046448f) * amplitude_0 * PS_CONSTANT_BUFFER_0.texel_size_0.y * 0.5f;
    var _S36 : vec4<f32> = sampleClamped_0(uv_9 + distort_0);
    var col_4 : vec4<f32> = _S36;
    var _S37 : vec3<f32> = _S36.xyz * vec3<f32>((1.0f + wave1_0 * wave2_0 * 0.02999999932944775f));
    col_4.x = _S37.x;
    col_4.y = _S37.y;
    col_4.z = _S37.z;
    return col_4;
}

fn hash22_0( p_3 : vec2<f32>) -> vec2<f32>
{
    var p3_2 : vec3<f32> = fract(p_3.xyx * vec3<f32>(0.1031000018119812f, 0.10300000011920929f, 0.09730000048875809f));
    var p3_3 : vec3<f32> = p3_2 + vec3<f32>(dot(p3_2, p3_2.yzx + vec3<f32>(33.3300018310546875f)));
    return fract((p3_3.xx + p3_3.yz) * p3_3.zy);
}

fn effect_voronoi_0( uv_10 : vec2<f32>) -> vec4<f32>
{
    var edgeW_0 : f32 = max(PS_CONSTANT_BUFFER_0.param1_0, 0.5f);
    var px_1 : vec2<f32> = uv_10 / PS_CONSTANT_BUFFER_0.texel_size_0;
    var cellSize_0 : f32 = min(PS_CONSTANT_BUFFER_0.win_half_px_0.x, PS_CONSTANT_BUFFER_0.win_half_px_0.y) * 2.0f / max(PS_CONSTANT_BUFFER_0.param0_0, 2.0f);
    var _S38 : vec2<f32> = vec2<f32>(cellSize_0);
    var cell_0 : vec2<f32> = floor(px_1 / _S38);
    const _S39 : vec2<f32> = vec2<f32>(3.17000007629394531f, 1.9299999475479126f);
    var _S40 : f32 = 1.0f - 1.0f / max(max(PS_CONSTANT_BUFFER_0.param2_0, 1.0f), 1.00999999046325684f);
    var _S41 : f32 = _S40 * cellSize_0;
    var _S42 : f32 = _S41 * 0.20000000298023224f;
    var dispersion_0 : f32 = _S41 * 0.05999999865889549f;
    const _S43 : vec3<f32> = vec3<f32>(1.0f, 1.0f, 1.0f);
    var _S44 : f32 = edgeW_0 * 3.0f;
    var minDist_0 : f32 = 1.0e+10f;
    var secondDist_0 : f32 = 1.0e+10f;
    var closestCell_0 : vec2<f32> = cell_0;
    var y_1 : i32 = i32(-1);
    var _S45 : vec2<f32> = vec2<f32>(7.30999994277954102f);
    var _S46 : vec2<f32> = vec2<f32>(2.0f);
    var _S47 : vec2<f32> = vec2<f32>(1.0f);
    var _S48 : vec2<f32> = vec2<f32>(_S40);
    var _S49 : vec2<f32> = vec2<f32>(0.40000000596046448f);
    var _S50 : vec2<f32> = vec2<f32>(dispersion_0);
    for(;;)
    {
        if(y_1 <= i32(1))
        {
        }
        else
        {
            break;
        }
        var _S51 : f32 = f32(y_1);
        var minDist_1 : f32 = minDist_0;
        var secondDist_1 : f32 = secondDist_0;
        var closestCell_1 : vec2<f32> = closestCell_0;
        var x_1 : i32 = i32(-1);
        for(;;)
        {
            if(x_1 <= i32(1))
            {
            }
            else
            {
                break;
            }
            var neighbor_0 : vec2<f32> = cell_0 + vec2<f32>(f32(x_1), _S51);
            var d_3 : f32 = length(px_1 - (neighbor_0 + hash22_0(neighbor_0)) * _S38);
            if(d_3 < minDist_1)
            {
                var _S52 : f32 = minDist_1;
                minDist_1 = d_3;
                secondDist_1 = _S52;
                closestCell_1 = neighbor_0;
            }
            else
            {
                var secondDist_2 : f32;
                if(d_3 < secondDist_1)
                {
                    secondDist_2 = d_3;
                }
                else
                {
                    secondDist_2 = secondDist_1;
                }
                secondDist_1 = secondDist_2;
            }
            x_1 = x_1 + i32(1);
        }
        var y_2 : i32 = y_1 + i32(1);
        minDist_0 = minDist_1;
        secondDist_0 = secondDist_1;
        closestCell_0 = closestCell_1;
        y_1 = y_2;
    }
    var cellTilt_0 : vec2<f32> = hash22_0(closestCell_0 * _S45 + _S39) * _S46 - _S47;
    var sampleUV_0 : vec2<f32> = applyParallax_0(uv_10 + cellTilt_0 * _S48 * _S38 * _S49 * PS_CONSTANT_BUFFER_0.texel_size_0, _S42);
    var col_5 : vec4<f32> = sampleClamped_0(sampleUV_0);
    var dOff_1 : vec2<f32> = cellTilt_0 * _S50 * PS_CONSTANT_BUFFER_0.texel_size_0;
    col_5[i32(0)] = sampleClamped_0(sampleUV_0 + dOff_1).x;
    col_5[i32(2)] = sampleClamped_0(sampleUV_0 - dOff_1).z;
    var edgeDist_0 : f32 = secondDist_0 - minDist_0;
    var _S53 : vec3<f32> = mix(col_5.xyz, _S43, vec3<f32>((pow(1.0f - saturate(edgeDist_0 / edgeW_0), 2.0f) * 0.5f)));
    col_5.x = _S53.x;
    col_5.y = _S53.y;
    col_5.z = _S53.z;
    var _S54 : vec3<f32> = col_5.xyz * vec3<f32>(mix(0.69999998807907104f, 1.0f, saturate(edgeDist_0 / _S44)));
    col_5.x = _S54.x;
    col_5.y = _S54.y;
    col_5.z = _S54.z;
    return col_5;
}

fn effect_edge_glow_0( uv_11 : vec2<f32>) -> vec4<f32>
{
    var _S55 : f32 = PS_CONSTANT_BUFFER_0.texel_size_0.x;
    var _S56 : f32 = - _S55;
    var _S57 : f32 = PS_CONSTANT_BUFFER_0.texel_size_0.y;
    var _S58 : f32 = - _S57;
    const _S59 : vec3<f32> = vec3<f32>(0.29899999499320984f, 0.58700001239776611f, 0.11400000005960464f);
    var tr_0 : f32 = dot(sampleClamped_0(uv_11 + vec2<f32>(_S55, _S58)).xyz, _S59);
    var bl_0 : f32 = dot(sampleClamped_0(uv_11 + vec2<f32>(_S56, _S57)).xyz, _S59);
    var br_0 : f32 = dot(sampleClamped_0(uv_11 + vec2<f32>(_S55, _S57)).xyz, _S59);
    var _S60 : f32 = - dot(sampleClamped_0(uv_11 + vec2<f32>(_S56, _S58)).xyz, _S59);
    var gx_0 : f32 = _S60 - 2.0f * dot(sampleClamped_0(uv_11 + vec2<f32>(_S56, 0.0f)).xyz, _S59) - bl_0 + tr_0 + 2.0f * dot(sampleClamped_0(uv_11 + vec2<f32>(_S55, 0.0f)).xyz, _S59) + br_0;
    var gy_0 : f32 = _S60 - 2.0f * dot(sampleClamped_0(uv_11 + vec2<f32>(0.0f, _S58)).xyz, _S59) - tr_0 + bl_0 + 2.0f * dot(sampleClamped_0(uv_11 + vec2<f32>(0.0f, _S57)).xyz, _S59) + br_0;
    var hue_0 : f32 = atan2(gy_0, gx_0) / 6.28318548202514648f + 0.5f;
    return vec4<f32>(sampleClamped_0(uv_11).xyz * vec3<f32>(0.15000000596046448f) + saturate(abs(fract(vec3<f32>(hue_0, hue_0 - 0.3333333432674408f, hue_0 + 0.3333333432674408f)) * vec3<f32>(6.0f) - vec3<f32>(3.0f)) - vec3<f32>(1.0f)) * vec3<f32>(saturate(sqrt(gx_0 * gx_0 + gy_0 * gy_0) * max(PS_CONSTANT_BUFFER_0.param0_0, 0.5f))), 1.0f);
}

fn halftone_dot_ex_0( px_2 : vec2<f32>,  spacing_0 : f32,  angle_0 : f32,  intensity_0 : f32,  sharpness_0 : f32,  cellCenterPx_0 : ptr<function, vec2<f32>>) -> f32
{
    var ca_0 : f32 = cos(angle_0);
    var sa_0 : f32 = sin(angle_0);
    var _S61 : f32 = px_2.x;
    var _S62 : f32 = px_2.y;
    var rotated_0 : vec2<f32> = vec2<f32>(_S61 * ca_0 + _S62 * sa_0, - _S61 * sa_0 + _S62 * ca_0);
    var _S63 : vec2<f32> = vec2<f32>(spacing_0);
    var _S64 : f32 = spacing_0 * 0.5f;
    var cell_1 : vec2<f32> = floor(rotated_0 / _S63) * _S63 + vec2<f32>(_S64);
    var _S65 : f32 = cell_1.x;
    var _S66 : f32 = cell_1.y;
    (*cellCenterPx_0) = vec2<f32>(_S65 * ca_0 - _S66 * sa_0, _S65 * sa_0 + _S66 * ca_0);
    return saturate((intensity_0 * _S64 - length(rotated_0 - cell_1)) * (sharpness_0 / max(_S64 * 0.10000000149011612f, 0.5f)));
}

fn effect_halftone_0( uv_12 : vec2<f32>) -> vec4<f32>
{
    var spacing_1 : f32 = max(PS_CONSTANT_BUFFER_0.param0_0, 4.0f);
    var sharpness_1 : f32 = max(PS_CONSTANT_BUFFER_0.param1_0, 0.5f);
    var px_3 : vec2<f32> = uv_12 / PS_CONSTANT_BUFFER_0.texel_size_0;
    var cc_0 : vec2<f32>;
    var _S67 : f32 = halftone_dot_ex_0(px_3, spacing_1, 0.26179999113082886f, 0.0f, sharpness_1, &(cc_0));
    var dc_0 : f32 = halftone_dot_ex_0(px_3, spacing_1, 0.26179999113082886f, 1.0f - sampleClamped_0(cc_0 * PS_CONSTANT_BUFFER_0.texel_size_0).x, sharpness_1, &(cc_0));
    var _S68 : f32 = halftone_dot_ex_0(px_3, spacing_1, 1.30900001525878906f, 0.0f, sharpness_1, &(cc_0));
    var dm_0 : f32 = halftone_dot_ex_0(px_3, spacing_1, 1.30900001525878906f, 1.0f - sampleClamped_0(cc_0 * PS_CONSTANT_BUFFER_0.texel_size_0).y, sharpness_1, &(cc_0));
    var _S69 : f32 = halftone_dot_ex_0(px_3, spacing_1, 0.0f, 0.0f, sharpness_1, &(cc_0));
    var dy_0 : f32 = halftone_dot_ex_0(px_3, spacing_1, 0.0f, 1.0f - sampleClamped_0(cc_0 * PS_CONSTANT_BUFFER_0.texel_size_0).z, sharpness_1, &(cc_0));
    var _S70 : f32 = halftone_dot_ex_0(px_3, spacing_1, 0.78539997339248657f, 0.0f, sharpness_1, &(cc_0));
    var srcK_0 : vec4<f32> = sampleClamped_0(cc_0 * PS_CONSTANT_BUFFER_0.texel_size_0);
    var dk_0 : f32 = halftone_dot_ex_0(px_3, spacing_1, 0.78539997339248657f, min(1.0f - srcK_0.x, min(1.0f - srcK_0.y, 1.0f - srcK_0.z)) * 0.5f, sharpness_1, &(cc_0));
    return vec4<f32>(saturate(vec3<f32>(1.0f, 1.0f, 1.0f) - vec3<f32>(dc_0, 0.0f, 0.0f) - vec3<f32>(0.0f, dm_0, 0.0f) - vec3<f32>(0.0f, 0.0f, dy_0) - vec3<f32>(dk_0, dk_0, dk_0)), 1.0f);
}

fn effect_mouse_edge_0( uv_13 : vec2<f32>) -> vec4<f32>
{
    var _S71 : f32 = PS_CONSTANT_BUFFER_0.texel_size_0.x;
    var _S72 : f32 = - _S71;
    var _S73 : f32 = PS_CONSTANT_BUFFER_0.texel_size_0.y;
    var _S74 : f32 = - _S73;
    const _S75 : vec3<f32> = vec3<f32>(0.29899999499320984f, 0.58700001239776611f, 0.11400000005960464f);
    var tr_1 : f32 = dot(sampleClamped_0(uv_13 + vec2<f32>(_S71, _S74)).xyz, _S75);
    var bl_1 : f32 = dot(sampleClamped_0(uv_13 + vec2<f32>(_S72, _S73)).xyz, _S75);
    var br_1 : f32 = dot(sampleClamped_0(uv_13 + vec2<f32>(_S71, _S73)).xyz, _S75);
    var _S76 : f32 = - dot(sampleClamped_0(uv_13 + vec2<f32>(_S72, _S74)).xyz, _S75);
    var gx_1 : f32 = _S76 - 2.0f * dot(sampleClamped_0(uv_13 + vec2<f32>(_S72, 0.0f)).xyz, _S75) - bl_1 + tr_1 + 2.0f * dot(sampleClamped_0(uv_13 + vec2<f32>(_S71, 0.0f)).xyz, _S75) + br_1;
    var gy_1 : f32 = _S76 - 2.0f * dot(sampleClamped_0(uv_13 + vec2<f32>(0.0f, _S74)).xyz, _S75) - tr_1 + bl_1 + 2.0f * dot(sampleClamped_0(uv_13 + vec2<f32>(0.0f, _S73)).xyz, _S75) + br_1;
    var falloff_0 : f32 = saturate(1.0f - length(uv_13 / PS_CONSTANT_BUFFER_0.texel_size_0 - PS_CONSTANT_BUFFER_0.mouse_uv_0 / PS_CONSTANT_BUFFER_0.texel_size_0) / max(PS_CONSTANT_BUFFER_0.param0_0, 20.0f));
    var dir_0 : vec2<f32> = normalize(uv_13 - PS_CONSTANT_BUFFER_0.mouse_uv_0 + vec2<f32>(0.00009999999747379f, 0.0f));
    var hue_1 : f32 = atan2(dir_0.y, dir_0.x) / 6.28318548202514648f + 0.5f;
    var highlight_0 : vec3<f32> = saturate(abs(fract(vec3<f32>(hue_1, hue_1 - 0.3333333432674408f, hue_1 + 0.3333333432674408f)) * vec3<f32>(6.0f) - vec3<f32>(3.0f)) - vec3<f32>(1.0f));
    var glow_0 : f32 = saturate(sqrt(gx_1 * gx_1 + gy_1 * gy_1) * max(PS_CONSTANT_BUFFER_0.param1_0, 1.0f)) * (falloff_0 * falloff_0);
    var _S77 : vec4<f32> = sampleClamped_0(uv_13);
    var col_6 : vec4<f32> = _S77;
    var _S78 : vec3<f32> = _S77.xyz + highlight_0 * vec3<f32>(glow_0);
    col_6.x = _S78.x;
    col_6.y = _S78.y;
    col_6.z = _S78.z;
    return col_6;
}

fn effect_crt_0( uv_14 : vec2<f32>) -> vec4<f32>
{
    var lineThick_0 : f32 = max(PS_CONSTANT_BUFFER_0.param0_0, 1.0f);
    var centered_0 : vec2<f32> = (uv_14 - PS_CONSTANT_BUFFER_0.win_center_0) / (PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0);
    var r2_0 : f32 = dot(centered_0, centered_0);
    var distorted_0 : vec2<f32> = uv_14 + centered_0 * vec2<f32>(r2_0) * vec2<f32>(PS_CONSTANT_BUFFER_0.param1_0) * PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0 * vec2<f32>(0.10000000149011612f);
    var px_4 : vec2<f32> = distorted_0 / PS_CONSTANT_BUFFER_0.texel_size_0;
    var subpixel_0 : f32 = fract(px_4.x / 3.0f) * 3.0f;
    var phosphor_0 : vec3<f32>;
    phosphor_0[i32(0)] = saturate(1.0f - abs(subpixel_0 - 0.5f) * 1.5f);
    phosphor_0[i32(1)] = saturate(1.0f - abs(subpixel_0 - 1.5f) * 1.5f);
    phosphor_0[i32(2)] = saturate(1.0f - abs(subpixel_0 - 2.5f) * 1.5f);
    var _S79 : vec3<f32> = mix(vec3<f32>(1.0f, 1.0f, 1.0f), phosphor_0, vec3<f32>(0.60000002384185791f));
    phosphor_0 = _S79;
    var src_0 : vec3<f32> = sampleClamped_0(distorted_0).xyz;
    var linePhase_0 : f32 = fract(px_4.y / (lineThick_0 * 2.0f));
    return vec4<f32>(saturate(src_0 * _S79 * vec3<f32>(mix(mix(0.15000000596046448f, 1.0f, smoothstep(0.0f, 0.15000000596046448f, linePhase_0) * (1.0f - smoothstep(0.34999999403953552f, 0.5f, linePhase_0))), 1.0f, dot(src_0, vec3<f32>(0.29899999499320984f, 0.58700001239776611f, 0.11400000005960464f)) * 0.30000001192092896f)) * vec3<f32>((1.0f - r2_0 * 0.40000000596046448f))), 1.0f);
}

fn effect_dot_matrix_0( uv_15 : vec2<f32>) -> vec4<f32>
{
    var cellPx_0 : f32 = max(PS_CONSTANT_BUFFER_0.param0_0, 3.0f);
    var px_5 : vec2<f32> = uv_15 / PS_CONSTANT_BUFFER_0.texel_size_0;
    var _S80 : vec2<f32> = vec2<f32>(cellPx_0);
    var cellCenter_0 : vec2<f32> = (floor(px_5 / _S80) + vec2<f32>(0.5f)) * _S80;
    var inCell_0 : vec2<f32> = (px_5 - cellCenter_0) / vec2<f32>((cellPx_0 * 0.5f));
    var dist_0 : f32 = mix(max(abs(inCell_0.x), abs(inCell_0.y)), length(inCell_0), saturate(PS_CONSTANT_BUFFER_0.param1_0));
    var _S81 : vec3<f32> = sampleClamped_0(cellCenter_0 * PS_CONSTANT_BUFFER_0.texel_size_0).xyz;
    var lum_0 : f32 = dot(_S81, vec3<f32>(0.29899999499320984f, 0.58700001239776611f, 0.11400000005960464f));
    var dotSize_0 : f32 = mix(0.5f, 0.94999998807907104f, lum_0);
    return vec4<f32>(_S81 * vec3<f32>(saturate((dotSize_0 - dist_0) * cellPx_0 * 0.5f)) + vec3<f32>((saturate(1.0f - dist_0 / dotSize_0) * 0.15000000596046448f * lum_0)), 1.0f);
}

fn effect_glitch_0( uv_16 : vec2<f32>) -> vec4<f32>
{
    var intensity_1 : f32 = max(PS_CONSTANT_BUFFER_0.param0_0, 0.10000000149011612f);
    var blockSize_0 : f32 = max(PS_CONSTANT_BUFFER_0.param1_0, 4.0f);
    var px_6 : vec2<f32> = uv_16 / PS_CONSTANT_BUFFER_0.texel_size_0;
    var timeSeed_0 : f32 = floor(PS_CONSTANT_BUFFER_0.param2_0 * 8.0f);
    var lineBlock_0 : f32 = floor(px_6.y / blockSize_0);
    var lineShift_0 : f32;
    if((hash12_0(vec2<f32>(lineBlock_0, timeSeed_0))) > (1.0f - intensity_1 * 0.30000001192092896f))
    {
        lineShift_0 = (hash12_0(vec2<f32>(lineBlock_0 + 0.5f, timeSeed_0)) * 2.0f - 1.0f) * intensity_1 * 40.0f;
    }
    else
    {
        lineShift_0 = 0.0f;
    }
    var _S82 : vec2<f32> = floor(px_6 / vec2<f32>((blockSize_0 * 4.0f))) + vec2<f32>(timeSeed_0);
    var blockHash_0 : f32 = hash12_0(_S82);
    var blockShift_0 : vec2<f32> = vec2<f32>(0.0f, 0.0f);
    if(blockHash_0 > (1.0f - intensity_1 * 0.10000000149011612f))
    {
        blockShift_0[i32(0)] = (hash12_0(_S82 + vec2<f32>(1.0f)) * 2.0f - 1.0f) * intensity_1 * 60.0f;
        blockShift_0[i32(1)] = (hash12_0(_S82 + vec2<f32>(2.0f)) * 2.0f - 1.0f) * intensity_1 * 20.0f;
    }
    var shiftedUV_0 : vec2<f32> = (px_6 + vec2<f32>(lineShift_0, 0.0f) + blockShift_0) * PS_CONSTANT_BUFFER_0.texel_size_0;
    var col_7 : vec3<f32>;
    var _S83 : vec2<f32> = vec2<f32>(intensity_1 * 3.0f * PS_CONSTANT_BUFFER_0.texel_size_0.x * hash12_0(vec2<f32>(timeSeed_0, 0.76999998092651367f)), 0.0f);
    col_7[i32(0)] = sampleClamped_0(shiftedUV_0 + _S83).x;
    col_7[i32(1)] = sampleClamped_0(shiftedUV_0).y;
    col_7[i32(2)] = sampleClamped_0(shiftedUV_0 - _S83).z;
    if((hash12_0(vec2<f32>(lineBlock_0 + 3.0f, timeSeed_0))) > (1.0f - intensity_1 * 0.05000000074505806f))
    {
        col_7 = vec3<f32>(1.0f) - col_7;
    }
    return vec4<f32>(saturate(col_7), 1.0f);
}

fn effect_stained_glass_0( uv_17 : vec2<f32>) -> vec4<f32>
{
    var leadW_0 : f32 = max(PS_CONSTANT_BUFFER_0.param1_0, 1.0f);
    var px_7 : vec2<f32> = uv_17 / PS_CONSTANT_BUFFER_0.texel_size_0;
    var cellSize_1 : f32 = min(PS_CONSTANT_BUFFER_0.win_half_px_0.x, PS_CONSTANT_BUFFER_0.win_half_px_0.y) * 2.0f / max(PS_CONSTANT_BUFFER_0.param0_0, 2.0f);
    var _S84 : vec2<f32> = vec2<f32>(cellSize_1);
    var _S85 : vec2<f32> = floor(px_7 / _S84);
    var _S86 : f32 = cellSize_1 * 0.10000000149011612f;
    const _S87 : vec3<f32> = vec3<f32>(0.29899999499320984f, 0.58700001239776611f, 0.11400000005960464f);
    var _S88 : vec3<f32> = vec3<f32>(1.5f);
    var _S89 : f32 = cellSize_1 * 0.40000000596046448f;
    const _S90 : vec3<f32> = vec3<f32>(0.05000000074505806f, 0.05000000074505806f, 0.05000000074505806f);
    var minDist_2 : f32 = 1.0e+10f;
    var secondDist_3 : f32 = 1.0e+10f;
    var closestPt_0 : vec2<f32> = px_7;
    var y_3 : i32 = i32(-1);
    var _S91 : vec2<f32> = vec2<f32>(2.0f);
    var _S92 : vec3<f32> = vec3<f32>(0.20000000298023224f);
    var _S93 : vec3<f32> = vec3<f32>(0.89999997615814209f);
    for(;;)
    {
        if(y_3 <= i32(1))
        {
        }
        else
        {
            break;
        }
        var _S94 : f32 = f32(y_3);
        var minDist_3 : f32 = minDist_2;
        var secondDist_4 : f32 = secondDist_3;
        var closestPt_1 : vec2<f32> = closestPt_0;
        var x_2 : i32 = i32(-1);
        for(;;)
        {
            if(x_2 <= i32(1))
            {
            }
            else
            {
                break;
            }
            var neighbor_1 : vec2<f32> = _S85 + vec2<f32>(f32(x_2), _S94);
            var pt_0 : vec2<f32> = (neighbor_1 + hash22_0(neighbor_1)) * _S84;
            var d_4 : f32 = length(px_7 - pt_0);
            if(d_4 < minDist_3)
            {
                var _S95 : f32 = minDist_3;
                minDist_3 = d_4;
                secondDist_4 = _S95;
                closestPt_1 = pt_0;
            }
            else
            {
                var secondDist_5 : f32;
                if(d_4 < secondDist_4)
                {
                    secondDist_5 = d_4;
                }
                else
                {
                    secondDist_5 = secondDist_4;
                }
                secondDist_4 = secondDist_5;
            }
            x_2 = x_2 + i32(1);
        }
        var y_4 : i32 = y_3 + i32(1);
        minDist_2 = minDist_3;
        secondDist_3 = secondDist_4;
        closestPt_0 = closestPt_1;
        y_3 = y_4;
    }
    var centroidUV_0 : vec2<f32> = applyParallax_0(closestPt_0 * PS_CONSTANT_BUFFER_0.texel_size_0, _S86);
    var ts2_0 : vec2<f32> = PS_CONSTANT_BUFFER_0.texel_size_0 * _S91;
    var _S96 : vec2<f32> = vec2<f32>(ts2_0.x, 0.0f);
    var _S97 : vec2<f32> = vec2<f32>(0.0f, ts2_0.y);
    var col_8 : vec3<f32> = (sampleClamped_0(centroidUV_0).xyz + sampleClamped_0(centroidUV_0 + _S96).xyz + sampleClamped_0(centroidUV_0 - _S96).xyz + sampleClamped_0(centroidUV_0 + _S97).xyz + sampleClamped_0(centroidUV_0 - _S97).xyz) * _S92;
    var lum_1 : f32 = dot(col_8, _S87);
    var lead_0 : f32 = saturate((secondDist_3 - minDist_2) / leadW_0);
    var _S98 : vec3<f32> = vec3<f32>((lead_0 * lead_0));
    return vec4<f32>(mix(_S90, saturate(mix(vec3<f32>(lum_1, lum_1, lum_1), col_8, _S88)) * _S98 * _S93 + vec3<f32>((pow(saturate(1.0f - minDist_2 / _S89), 8.0f) * 0.20000000298023224f)), _S98), 1.0f);
}

fn N13_0( p_4 : f32) -> vec3<f32>
{
    var p3_4 : vec3<f32> = fract(vec3<f32>(p_4, p_4, p_4) * vec3<f32>(0.1031000018119812f, 0.11368999630212784f, 0.13786999881267548f));
    var p3_5 : vec3<f32> = p3_4 + vec3<f32>(dot(p3_4, p3_4.yzx + vec3<f32>(19.19000053405761719f)));
    var _S99 : f32 = p3_5.x;
    var _S100 : f32 = p3_5.y;
    var _S101 : f32 = p3_5.z;
    return fract(vec3<f32>((_S99 + _S100) * _S101, (_S99 + _S101) * _S100, (_S100 + _S101) * _S99));
}

fn S_0( a_0 : f32,  b_1 : f32,  t_0 : f32) -> f32
{
    var _S102 : f32;
    if(a_0 < b_1)
    {
        _S102 = smoothstep(a_0, b_1, t_0);
    }
    else
    {
        _S102 = 1.0f - smoothstep(b_1, a_0, t_0);
    }
    return _S102;
}

fn Saw_0( b_2 : f32,  t_1 : f32) -> f32
{
    return S_0(0.0f, b_2, t_1) * S_0(1.0f, b_2, t_1);
}

fn StaticDrops_0( uv_18 : vec2<f32>,  time_0 : f32) -> f32
{
    var _S103 : vec2<f32> = uv_18 * vec2<f32>(40.0f);
    var id_0 : vec2<f32> = floor(_S103);
    var _S104 : vec2<f32> = vec2<f32>(0.5f);
    var n_0 : vec3<f32> = N13_0(id_0.x * 107.4499969482421875f + id_0.y * 3543.654052734375f);
    var _S105 : f32 = n_0.z;
    return S_0(0.30000001192092896f, 0.0f, length(fract(_S103) - _S104 - (n_0.xy - _S104) * vec2<f32>(0.69999998807907104f))) * fract(_S105 * 10.0f) * Saw_0(0.02500000037252903f, fract(time_0 + _S105));
}

fn DropLayer_0( uv_19 : vec2<f32>,  time_1 : f32) -> vec2<f32>
{
    var _S106 : vec2<f32> = uv_19;
    var _S107 : f32 = _S106[i32(1)] + time_1 * 0.75f;
    _S106[i32(1)] = _S107;
    var grid_0 : vec2<f32> = vec2<f32>(6.0f, 1.0f) * vec2<f32>(2.0f);
    _S106[i32(1)] = _S107 + fract(sin(floor(_S106 * grid_0).x * 12345.564453125f) * 7658.759765625f);
    var id_1 : vec2<f32> = floor(_S106 * grid_0);
    var n_1 : vec3<f32> = N13_0(id_1.x * 35.20000076293945312f + id_1.y * 2376.10009765625f);
    var st_0 : vec2<f32> = fract(_S106 * grid_0) - vec2<f32>(0.5f, 0.0f);
    var x_3 : f32 = n_1.x - 0.5f;
    var _S108 : f32 = uv_19.y;
    var y_5 : f32 = _S108 * 20.0f;
    var _S109 : f32 = n_1.z;
    var x_4 : f32 = (x_3 + sin(y_5 + sin(y_5)) * (0.5f - abs(x_3)) * (_S109 - 0.5f)) * 0.69999998807907104f;
    var y_6 : f32 = (Saw_0(0.85000002384185791f, fract(time_1 + _S109)) - 0.5f) * 0.89999997615814209f + 0.5f;
    var _S110 : f32 = st_0.y;
    var r_2 : f32 = sqrt(S_0(1.0f, y_6, _S110));
    var trailFront_0 : f32 = S_0(-0.01999999955296516f, 0.01999999955296516f, _S110 - y_6);
    return vec2<f32>(S_0(0.40000000596046448f, 0.0f, length((st_0 - vec2<f32>(x_4, y_6)) * vec2<f32>(1.0f, 6.0f))) + S_0(0.30000001192092896f, 0.0f, length(st_0 - vec2<f32>(x_4, fract(_S108 * 10.0f) + (_S110 - 0.5f)))) * r_2 * trailFront_0, S_0(0.23000000417232513f * r_2, 0.15000000596046448f * r_2 * r_2, abs(st_0.x - x_4)) * (trailFront_0 * r_2 * r_2));
}

fn RainDrops_0( uv_20 : vec2<f32>,  time_2 : f32,  rainAmount_0 : f32) -> vec2<f32>
{
    var t_2 : f32 = - time_2;
    var l0_0 : f32 = smoothstep(0.0f, 0.5f, rainAmount_0);
    var l1_0 : f32 = smoothstep(0.25f, 0.75f, rainAmount_0);
    var m1_0 : vec2<f32> = DropLayer_0(uv_20, t_2) * vec2<f32>(l1_0);
    var m2_0 : vec2<f32> = DropLayer_0(uv_20 * vec2<f32>(1.85000002384185791f), t_2) * vec2<f32>(l0_0);
    return vec2<f32>(smoothstep(0.30000001192092896f, 1.0f, StaticDrops_0(uv_20, t_2) * l0_0 + m1_0.x + m2_0.x), max(m1_0.y * l0_0, m2_0.y * l1_0));
}

fn srgbToLinear_0( c_0 : f32) -> f32
{
    var _S111 : f32;
    if(c_0 <= 0.04044999927282333f)
    {
        _S111 = c_0 / 12.92000007629394531f;
    }
    else
    {
        _S111 = pow((c_0 + 0.05499999970197678f) / 1.0549999475479126f, 2.40000009536743164f);
    }
    return _S111;
}

fn srgbToLinear3_0( c_1 : vec3<f32>) -> vec3<f32>
{
    return vec3<f32>(srgbToLinear_0(c_1.x), srgbToLinear_0(c_1.y), srgbToLinear_0(c_1.z));
}

fn linearToSrgb_0( c_2 : f32) -> f32
{
    var _S112 : f32;
    if(c_2 <= 0.00313080009073019f)
    {
        _S112 = c_2 * 12.92000007629394531f;
    }
    else
    {
        _S112 = 1.0549999475479126f * pow(c_2, 0.4166666567325592f) - 0.05499999970197678f;
    }
    return _S112;
}

fn linearToSrgb3_0( c_3 : vec3<f32>) -> vec3<f32>
{
    return vec3<f32>(linearToSrgb_0(c_3.x), linearToSrgb_0(c_3.y), linearToSrgb_0(c_3.z));
}

fn effect_rain_0( uv_21 : vec2<f32>) -> vec4<f32>
{
    var rainAmount_1 : f32 = saturate(PS_CONSTANT_BUFFER_0.param0_0);
    var fogBlur_0 : f32 = max(PS_CONSTANT_BUFFER_0.param1_0, 0.0f);
    var time_3 : f32 = PS_CONSTANT_BUFFER_0.param2_0;
    var _S113 : vec2<f32> = vec2<f32>(0.5f);
    var winUV_0 : vec2<f32> = (uv_21 - PS_CONSTANT_BUFFER_0.win_center_0) / (PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0) * _S113 + _S113;
    winUV_0[i32(0)] = winUV_0[i32(0)] * (PS_CONSTANT_BUFFER_0.win_half_px_0.x / max(PS_CONSTANT_BUFFER_0.win_half_px_0.y, 1.0f));
    var c_4 : vec2<f32> = RainDrops_0(winUV_0, time_3, rainAmount_1);
    var _S114 : f32 = c_4.x;
    var focus_0 : f32 = mix(mix(fogBlur_0, 0.0f, smoothstep(0.10000000149011612f, 0.20000000298023224f, _S114)), fogBlur_0 * 0.5f, c_4.y);
    var refractUV_0 : vec2<f32> = uv_21 + vec2<f32>(RainDrops_0(winUV_0 + vec2<f32>(0.0020000000949949f, 0.0f), time_3, rainAmount_1).x - _S114, RainDrops_0(winUV_0 + vec2<f32>(0.0f, 0.0020000000949949f), time_3, rainAmount_1).x - _S114) * PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0 * _S113;
    var R_0 : i32 = clamp(i32(focus_0 * 2.0f + 0.5f), i32(0), i32(4));
    var blurred_0 : vec3<f32>;
    if(R_0 > i32(0))
    {
        const _S115 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
        var _S116 : f32 = max(focus_0 * 0.5f, 1.0f);
        var _S117 : i32 = - R_0;
        var ky_0 : i32 = _S117;
        blurred_0 = _S115;
        var count_0 : f32 = 0.0f;
        var _S118 : vec2<f32> = vec2<f32>(_S116);
        for(;;)
        {
            if(ky_0 <= R_0)
            {
            }
            else
            {
                break;
            }
            var _S119 : f32 = f32(ky_0);
            var kx_0 : i32 = _S117;
            for(;;)
            {
                if(kx_0 <= R_0)
                {
                }
                else
                {
                    break;
                }
                var acc_0 : vec3<f32> = blurred_0 + srgbToLinear3_0(sampleClamped_0(refractUV_0 + vec2<f32>(f32(kx_0), _S119) * _S118 * PS_CONSTANT_BUFFER_0.texel_size_0).xyz);
                var count_1 : f32 = count_0 + 1.0f;
                kx_0 = kx_0 + i32(1);
                blurred_0 = acc_0;
                count_0 = count_1;
            }
            ky_0 = ky_0 + i32(1);
        }
        blurred_0 = linearToSrgb3_0(blurred_0 / vec3<f32>(count_0));
    }
    else
    {
        blurred_0 = sampleClamped_0(refractUV_0).xyz;
    }
    var _S120 : vec3<f32> = mix(blurred_0, sampleClamped_0(refractUV_0).xyz, vec3<f32>(smoothstep(0.10000000149011612f, 0.30000001192092896f, _S114)));
    var col_9 : vec4<f32>;
    col_9.x = _S120.x;
    col_9.y = _S120.y;
    col_9.z = _S120.z;
    col_9[i32(3)] = 1.0f;
    return col_9;
}

fn effect_kaleidoscope_0( uv_22 : vec2<f32>) -> vec4<f32>
{
    var rotation_0 : f32 = PS_CONSTANT_BUFFER_0.param1_0 + PS_CONSTANT_BUFFER_0.param2_0 * 0.30000001192092896f;
    var centered_1 : vec2<f32> = (uv_22 - PS_CONSTANT_BUFFER_0.win_center_0) / (PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0);
    var ca_1 : f32 = cos(rotation_0);
    var sa_1 : f32 = sin(rotation_0);
    var _S121 : f32 = centered_1.x;
    var _S122 : f32 = centered_1.y;
    var _S123 : f32 = _S121 * ca_1 + _S122 * sa_1;
    var _S124 : f32 = - _S121 * sa_1 + _S122 * ca_1;
    var angle_1 : f32 = atan2(_S124, _S123);
    var segAngle_0 : f32 = 6.28318548202514648f / max(PS_CONSTANT_BUFFER_0.param0_0, 2.0f);
    var _S125 : f32 = segAngle_0 * 0.5f;
    var angle_2 : f32 = abs(((((angle_1)) % ((segAngle_0)))) - _S125);
    var _S126 : vec4<f32> = sampleClamped_0(vec2<f32>(cos(angle_2), sin(angle_2)) * vec2<f32>(length(vec2<f32>(_S123, _S124))) * PS_CONSTANT_BUFFER_0.win_half_px_0 * PS_CONSTANT_BUFFER_0.texel_size_0 + PS_CONSTANT_BUFFER_0.win_center_0);
    var col_10 : vec4<f32> = _S126;
    var _S127 : vec3<f32> = _S126.xyz + vec3<f32>((pow(saturate(1.0f - abs(((((angle_1 + 3.14159274101257324f)) % ((segAngle_0)))) - _S125) / (segAngle_0 * 0.05000000074505806f)), 4.0f) * 0.15000000596046448f));
    col_10.x = _S127.x;
    col_10.y = _S127.y;
    col_10.z = _S127.z;
    return col_10;
}

fn sampleLinear_0( uv_23 : vec2<f32>) -> vec4<f32>
{
    var _S128 : vec4<f32> = sampleClamped_0(uv_23);
    var c_5 : vec4<f32> = _S128;
    var _S129 : vec3<f32> = srgbToLinear3_0(_S128.xyz);
    c_5.x = _S129.x;
    c_5.y = _S129.y;
    c_5.z = _S129.z;
    return c_5;
}

fn effect_blur_0( uv_24 : vec2<f32>) -> vec4<f32>
{
    var radius_0 : f32 = PS_CONSTANT_BUFFER_0.param0_0;
    var _S130 : vec2<f32>;
    if((PS_CONSTANT_BUFFER_0.param1_0) < 0.5f)
    {
        _S130 = vec2<f32>(PS_CONSTANT_BUFFER_0.texel_size_0.x, 0.0f);
    }
    else
    {
        _S130 = vec2<f32>(0.0f, PS_CONSTANT_BUFFER_0.texel_size_0.y);
    }
    var bevelPx_2 : f32 = max(radius_0 * 2.0f, 8.0f);
    var normal_3 : vec2<f32>;
    var d_5 : f32 = sdfWithNormal_0(uv_24, 0.0f, &(normal_3));
    var _S131 : vec2<f32> = uv_24 + normal_3 * vec2<f32>((saturate(1.0f - saturate(- d_5 / bevelPx_2)) * radius_0 * 0.5f)) * PS_CONSTANT_BUFFER_0.texel_size_0;
    var edgeHighlight_0 : f32 = pow(saturate(1.0f - abs(d_5) / (bevelPx_2 * 0.5f)), 3.0f) * 0.15000000596046448f;
    var _S132 : f32 = radius_0 / 6.0f;
    var w_1 : array<f32, i32(7)> = array<f32, i32(7)>( 0.19648249447345734f, 0.17489039897918701f, 0.1225150004029274f, 0.06753169745206833f, 0.0292431004345417f, 0.00995950028300285f, 0.00266269990243018f );
    var _S133 : vec3<f32> = sampleLinear_0(_S131).xyz * vec3<f32>(0.19648249447345734f);
    var i_1 : i32 = i32(1);
    var lin_0 : vec3<f32> = _S133;
    var _S134 : vec3<f32> = vec3<f32>(edgeHighlight_0);
    for(;;)
    {
        if(i_1 < i32(7))
        {
        }
        else
        {
            break;
        }
        var off_1 : vec2<f32> = _S130 * vec2<f32>((f32(i_1) * _S132));
        var _S135 : vec3<f32> = vec3<f32>(w_1[i_1]);
        var lin_1 : vec3<f32> = lin_0 + sampleLinear_0(_S131 + off_1).xyz * _S135 + sampleLinear_0(_S131 - off_1).xyz * _S135;
        i_1 = i_1 + i32(1);
        lin_0 = lin_1;
    }
    var _S136 : vec3<f32> = linearToSrgb3_0(lin_0 + _S134);
    var col_11 : vec4<f32>;
    col_11.x = _S136.x;
    col_11.y = _S136.y;
    col_11.z = _S136.z;
    col_11[i32(3)] = 1.0f;
    return col_11;
}

struct pixelOutput_0
{
    @location(0) output_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) col_12 : vec4<f32>,
    @location(1) uv_25 : vec2<f32>,
};

@fragment
fn main_ps( _S137 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    var col_13 : vec4<f32>;
    var m_0 : i32 = i32(PS_CONSTANT_BUFFER_0.mode_0);
    if(m_0 == i32(1))
    {
        col_13 = effect_glass_refract_0(_S137.uv_25);
    }
    else
    {
        if(m_0 == i32(2))
        {
            col_13 = effect_frosted_0(_S137.uv_25);
        }
        else
        {
            if(m_0 == i32(3))
            {
                col_13 = effect_pixelate_0(_S137.uv_25);
            }
            else
            {
                if(m_0 == i32(4))
                {
                    col_13 = effect_chromatic_0(_S137.uv_25);
                }
                else
                {
                    if(m_0 == i32(5))
                    {
                        col_13 = effect_liquid_glass_0(_S137.uv_25);
                    }
                    else
                    {
                        if(m_0 == i32(6))
                        {
                            col_13 = effect_heat_haze_0(_S137.uv_25);
                        }
                        else
                        {
                            if(m_0 == i32(7))
                            {
                                col_13 = effect_voronoi_0(_S137.uv_25);
                            }
                            else
                            {
                                if(m_0 == i32(8))
                                {
                                    col_13 = effect_edge_glow_0(_S137.uv_25);
                                }
                                else
                                {
                                    if(m_0 == i32(9))
                                    {
                                        col_13 = effect_halftone_0(_S137.uv_25);
                                    }
                                    else
                                    {
                                        if(m_0 == i32(10))
                                        {
                                            col_13 = effect_mouse_edge_0(_S137.uv_25);
                                        }
                                        else
                                        {
                                            if(m_0 == i32(11))
                                            {
                                                col_13 = effect_crt_0(_S137.uv_25);
                                            }
                                            else
                                            {
                                                if(m_0 == i32(12))
                                                {
                                                    col_13 = effect_dot_matrix_0(_S137.uv_25);
                                                }
                                                else
                                                {
                                                    if(m_0 == i32(13))
                                                    {
                                                        col_13 = effect_glitch_0(_S137.uv_25);
                                                    }
                                                    else
                                                    {
                                                        if(m_0 == i32(14))
                                                        {
                                                            col_13 = effect_stained_glass_0(_S137.uv_25);
                                                        }
                                                        else
                                                        {
                                                            if(m_0 == i32(15))
                                                            {
                                                                col_13 = effect_rain_0(_S137.uv_25);
                                                            }
                                                            else
                                                            {
                                                                if(m_0 == i32(16))
                                                                {
                                                                    col_13 = effect_kaleidoscope_0(_S137.uv_25);
                                                                }
                                                                else
                                                                {
                                                                    col_13 = effect_blur_0(_S137.uv_25);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    col_13[i32(3)] = 1.0f;
    var _S138 : pixelOutput_0 = pixelOutput_0( col_13 * _S137.col_12 );
    return _S138;
}

