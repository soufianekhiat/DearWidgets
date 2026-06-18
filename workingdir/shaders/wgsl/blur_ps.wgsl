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

fn srgbToLinear_0( c_0 : f32) -> f32
{
    var _S4 : f32;
    if(c_0 <= 0.04044999927282333f)
    {
        _S4 = c_0 / 12.92000007629394531f;
    }
    else
    {
        _S4 = pow((c_0 + 0.05499999970197678f) / 1.0549999475479126f, 2.40000009536743164f);
    }
    return _S4;
}

fn srgbToLinear3_0( c_1 : vec3<f32>) -> vec3<f32>
{
    return vec3<f32>(srgbToLinear_0(c_1.x), srgbToLinear_0(c_1.y), srgbToLinear_0(c_1.z));
}

fn sampleLinear_0( uv_3 : vec2<f32>) -> vec4<f32>
{
    var _S5 : vec4<f32> = sampleClamped_0(uv_3);
    var c_2 : vec4<f32> = _S5;
    var _S6 : vec3<f32> = srgbToLinear3_0(_S5.xyz);
    c_2.x = _S6.x;
    c_2.y = _S6.y;
    c_2.z = _S6.z;
    return c_2;
}

fn linearToSrgb_0( c_3 : f32) -> f32
{
    var _S7 : f32;
    if(c_3 <= 0.00313080009073019f)
    {
        _S7 = c_3 * 12.92000007629394531f;
    }
    else
    {
        _S7 = 1.0549999475479126f * pow(c_3, 0.4166666567325592f) - 0.05499999970197678f;
    }
    return _S7;
}

fn linearToSrgb3_0( c_4 : vec3<f32>) -> vec3<f32>
{
    return vec3<f32>(linearToSrgb_0(c_4.x), linearToSrgb_0(c_4.y), linearToSrgb_0(c_4.z));
}

fn effect_blur_0( uv_4 : vec2<f32>) -> vec4<f32>
{
    var radius_0 : f32 = PS_CONSTANT_BUFFER_0.param0_0;
    var _S8 : vec2<f32>;
    if((PS_CONSTANT_BUFFER_0.param1_0) < 0.5f)
    {
        _S8 = vec2<f32>(PS_CONSTANT_BUFFER_0.texel_size_0.x, 0.0f);
    }
    else
    {
        _S8 = vec2<f32>(0.0f, PS_CONSTANT_BUFFER_0.texel_size_0.y);
    }
    var bevelPx_0 : f32 = max(radius_0 * 2.0f, 8.0f);
    var normal_1 : vec2<f32>;
    var d_1 : f32 = sdfWithNormal_0(uv_4, 0.0f, &(normal_1));
    var _S9 : vec2<f32> = uv_4 + normal_1 * vec2<f32>((saturate(1.0f - saturate(- d_1 / bevelPx_0)) * radius_0 * 0.5f)) * PS_CONSTANT_BUFFER_0.texel_size_0;
    var edgeHighlight_0 : f32 = pow(saturate(1.0f - abs(d_1) / (bevelPx_0 * 0.5f)), 3.0f) * 0.15000000596046448f;
    var _S10 : f32 = radius_0 / 6.0f;
    var w_0 : array<f32, i32(7)> = array<f32, i32(7)>( 0.19648249447345734f, 0.17489039897918701f, 0.1225150004029274f, 0.06753169745206833f, 0.0292431004345417f, 0.00995950028300285f, 0.00266269990243018f );
    var _S11 : vec3<f32> = sampleLinear_0(_S9).xyz * vec3<f32>(0.19648249447345734f);
    var i_0 : i32 = i32(1);
    var lin_0 : vec3<f32> = _S11;
    var _S12 : vec3<f32> = vec3<f32>(edgeHighlight_0);
    for(;;)
    {
        if(i_0 < i32(7))
        {
        }
        else
        {
            break;
        }
        var off_0 : vec2<f32> = _S8 * vec2<f32>((f32(i_0) * _S10));
        var _S13 : vec3<f32> = vec3<f32>(w_0[i_0]);
        var lin_1 : vec3<f32> = lin_0 + sampleLinear_0(_S9 + off_0).xyz * _S13 + sampleLinear_0(_S9 - off_0).xyz * _S13;
        i_0 = i_0 + i32(1);
        lin_0 = lin_1;
    }
    var _S14 : vec3<f32> = linearToSrgb3_0(lin_0 + _S12);
    var col_0 : vec4<f32>;
    col_0.x = _S14.x;
    col_0.y = _S14.y;
    col_0.z = _S14.z;
    col_0[i32(3)] = 1.0f;
    return col_0;
}

struct pixelOutput_0
{
    @location(0) output_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) col_1 : vec4<f32>,
    @location(1) uv_5 : vec2<f32>,
};

@fragment
fn main_ps( _S15 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    var col_2 : vec4<f32> = effect_blur_0(_S15.uv_5);
    col_2[i32(3)] = 1.0f;
    var _S16 : pixelOutput_0 = pixelOutput_0( col_2 * _S15.col_1 );
    return _S16;
}

