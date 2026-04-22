struct SLANG_ParameterGroup_LookDevInspectorParams_std140_0
{
    @align(16) sideA_0 : vec4<f32>,
    @align(16) sideB_0 : vec4<f32>,
    @align(16) divider_0 : vec4<f32>,
    @align(16) canvas_0 : vec4<f32>,
};

@binding(1) @group(0) var<uniform> LookDevInspectorParams_0 : SLANG_ParameterGroup_LookDevInspectorParams_std140_0;
@binding(5) @group(0) var texture1_0 : texture_2d<f32>;

@binding(4) @group(0) var sampler1_0 : sampler;

@binding(3) @group(0) var texture0_0 : texture_2d<f32>;

@binding(2) @group(0) var sampler0_0 : sampler;

fn apply_tone_0( rgb_0 : vec3<f32>,  params_0 : vec4<f32>) -> vec3<f32>
{
    var _S1 : f32 = params_0.y;
    return pow(max((rgb_0 * vec3<f32>(exp2(params_0.x)) - vec3<f32>(_S1)) / vec3<f32>(max(params_0.z - _S1, 9.99999997475242708e-07f)), vec3<f32>(0.0f)), vec3<f32>((1.0f / max(params_0.w, 0.00009999999747379f))));
}

struct pixelOutput_0
{
    @location(0) output_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) col_0 : vec4<f32>,
    @location(1) uv_0 : vec2<f32>,
};

@fragment
fn main_ps( _S2 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    var aspect_0 : f32 = LookDevInspectorParams_0.canvas_0.x / max(LookDevInspectorParams_0.canvas_0.y, 1.0f);
    var onB_0 : bool = ((_S2.uv_0.x * aspect_0 - LookDevInspectorParams_0.divider_0.x * aspect_0) * cos(LookDevInspectorParams_0.divider_0.z) + (_S2.uv_0.y - LookDevInspectorParams_0.divider_0.y) * sin(LookDevInspectorParams_0.divider_0.z)) < 0.0f;
    var onB_1 : bool;
    if((LookDevInspectorParams_0.divider_0.w) > 0.5f)
    {
        onB_1 = !onB_0;
    }
    else
    {
        onB_1 = onB_0;
    }
    var c_0 : vec4<f32>;
    if(onB_1)
    {
        var _S3 : vec4<f32> = (textureSample((texture1_0), (sampler1_0), (_S2.uv_0)));
        c_0 = _S3;
        var _S4 : vec3<f32> = apply_tone_0(_S3.xyz, LookDevInspectorParams_0.sideB_0);
        c_0.x = _S4.x;
        c_0.y = _S4.y;
        c_0.z = _S4.z;
    }
    else
    {
        var _S5 : vec4<f32> = (textureSample((texture0_0), (sampler0_0), (_S2.uv_0)));
        c_0 = _S5;
        var _S6 : vec3<f32> = apply_tone_0(_S5.xyz, LookDevInspectorParams_0.sideA_0);
        c_0.x = _S6.x;
        c_0.y = _S6.y;
        c_0.z = _S6.z;
    }
    var _S7 : vec3<f32> = saturate(c_0.xyz);
    c_0.x = _S7.x;
    c_0.y = _S7.y;
    c_0.z = _S7.z;
    var _S8 : pixelOutput_0 = pixelOutput_0( c_0 * _S2.col_0 );
    return _S8;
}

