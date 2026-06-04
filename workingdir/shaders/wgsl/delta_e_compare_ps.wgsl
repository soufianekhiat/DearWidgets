@binding(3) @group(0) var texture0_0 : texture_2d<f32>;

@binding(2) @group(0) var sampler0_0 : sampler;

@binding(5) @group(0) var texture1_0 : texture_2d<f32>;

@binding(4) @group(0) var sampler1_0 : sampler;

struct SLANG_ParameterGroup_DeltaECompareParams_std140_0
{
    @align(16) modePack_0 : vec4<u32>,
    @align(16) rangePack_0 : vec4<f32>,
    @align(16) mixPack_0 : vec4<f32>,
};

@binding(1) @group(0) var<uniform> DeltaECompareParams_0 : SLANG_ParameterGroup_DeltaECompareParams_std140_0;
fn srgb_to_linear_0( c_0 : vec3<f32>) -> vec3<f32>
{
    return mix(c_0 / vec3<f32>(12.92000007629394531f), pow((c_0 + vec3<f32>(0.05499999970197678f)) / vec3<f32>(1.0549999475479126f), vec3<f32>(2.40000009536743164f)), step(vec3<f32>(0.04044999927282333f), c_0));
}

fn linear_to_oklab_0( rgb_0 : vec3<f32>) -> vec3<f32>
{
    var _S1 : f32 = rgb_0.x;
    var _S2 : f32 = rgb_0.y;
    var _S3 : f32 = rgb_0.z;
    var l_0 : f32 = pow(max(0.41222146153450012f * _S1 + 0.53633254766464233f * _S2 + 0.05144599452614784f * _S3, 0.0f), 0.3333333432674408f);
    var m_0 : f32 = pow(max(0.21190349757671356f * _S1 + 0.68069952726364136f * _S2 + 0.10739696025848389f * _S3, 0.0f), 0.3333333432674408f);
    var s_0 : f32 = pow(max(0.08830246329307556f * _S1 + 0.28171885013580322f * _S2 + 0.6299787163734436f * _S3, 0.0f), 0.3333333432674408f);
    return vec3<f32>(0.21045425534248352f * l_0 + 0.79361778497695923f * m_0 - 0.00407204683870077f * s_0, 1.97799849510192871f * l_0 - 2.42859220504760742f * m_0 + 0.45059370994567871f * s_0, 0.02590403705835342f * l_0 + 0.7827717661857605f * m_0 - 0.80867576599121094f * s_0);
}

fn linear_to_xyz_0( rgb_1 : vec3<f32>) -> vec3<f32>
{
    var _S4 : f32 = rgb_1.x;
    var _S5 : f32 = rgb_1.y;
    var _S6 : f32 = rgb_1.z;
    return vec3<f32>(0.41245639324188232f * _S4 + 0.35757610201835632f * _S5 + 0.18043750524520874f * _S6, 0.21267290413379669f * _S4 + 0.71515220403671265f * _S5 + 0.07217500358819962f * _S6, 0.01933390088379383f * _S4 + 0.11919199675321579f * _S5 + 0.95030409097671509f * _S6);
}

fn lab_f_0( t_0 : f32) -> f32
{
    var _S7 : f32;
    if(t_0 > 0.00885600037872791f)
    {
        _S7 = pow(t_0, 0.3333333432674408f);
    }
    else
    {
        _S7 = 7.78700017929077148f * t_0 + 0.13793103396892548f;
    }
    return _S7;
}

fn xyz_to_lab_0( xyz_0 : vec3<f32>) -> vec3<f32>
{
    var n_0 : vec3<f32> = xyz_0 / vec3<f32>(0.950469970703125f, 1.0f, 1.08882999420166016f);
    var fy_0 : f32 = lab_f_0(n_0.y);
    return vec3<f32>(116.0f * fy_0 - 16.0f, 500.0f * (lab_f_0(n_0.x) - fy_0), 200.0f * (fy_0 - lab_f_0(n_0.z)));
}

fn viridis_0( t_1 : f32) -> vec3<f32>
{
    var _S8 : vec3<f32> = vec3<f32>(saturate(t_1));
    return saturate(vec3<f32>(0.26700401306152344f, 0.0048739998601377f, 0.32941499352455139f) + _S8 * (vec3<f32>(1.02600002288818359f, 1.52300000190734863f, 0.35499998927116394f) + _S8 * (vec3<f32>(1.18299996852874756f, -1.28299999237060547f, -3.0150001049041748f) + _S8 * vec3<f32>(-3.18000006675720215f, 0.6940000057220459f, 2.54699993133544922f))));
}

fn turbo_0( t_2 : f32) -> vec3<f32>
{
    var _S9 : f32 = saturate(t_2);
    return saturate(vec3<f32>(34.6100006103515625f + _S9 * (-1565.949951171875f + _S9 * (23299.0390625f + _S9 * (-7.3095421875e+04f + _S9 * (1.080520234375e+05f - _S9 * 58346.3515625f)))) * 9.99999997475242708e-07f, 23.30999946594238281f + _S9 * (557.33001708984375f + _S9 * (1225.3299560546875f + _S9 * (-3574.9599609375f + _S9 * (1073.77001953125f + _S9 * 707.55999755859375f)))) * 0.00100000004749745f, 27.20000076293945312f + _S9 * (3211.10009765625f + _S9 * (-15327.9697265625f + _S9 * (27814.0f + _S9 * (-22569.1796875f + _S9 * 6838.66015625f)))) * 0.00000999999974738f));
}

fn magma_0( t_3 : f32) -> vec3<f32>
{
    var _S10 : vec3<f32> = vec3<f32>(saturate(t_3));
    return saturate(vec3<f32>(-0.0020000000949949f, -0.0020000000949949f, -0.0130000002682209f) + _S10 * (vec3<f32>(0.25f, 0.15000000596046448f, 0.69999998807907104f) + _S10 * (vec3<f32>(1.60000002384185791f, 0.34999999403953552f, 0.60000002384185791f) + _S10 * vec3<f32>(-1.10000002384185791f, -0.20000000298023224f, -1.89999997615814209f))));
}

fn ramp_0( t_4 : f32,  mode_0 : u32) -> vec3<f32>
{
    if(mode_0 == u32(1))
    {
        return viridis_0(t_4);
    }
    if(mode_0 == u32(2))
    {
        return turbo_0(t_4);
    }
    if(mode_0 == u32(3))
    {
        return magma_0(t_4);
    }
    return vec3<f32>(t_4, t_4, t_4);
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
fn main_ps( _S11 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    var a_srgb_0 : vec3<f32> = (textureSample((texture0_0), (sampler0_0), (_S11.uv_0))).xyz;
    var b_srgb_0 : vec3<f32> = (textureSample((texture1_0), (sampler1_0), (_S11.uv_0))).xyz;
    var a_lin_0 : vec3<f32> = srgb_to_linear_0(a_srgb_0);
    var b_lin_0 : vec3<f32> = srgb_to_linear_0(b_srgb_0);
    var formula_0 : u32 = DeltaECompareParams_0.modePack_0.x;
    var dE_0 : f32;
    if(formula_0 == u32(1))
    {
        dE_0 = length(linear_to_oklab_0(a_lin_0) - linear_to_oklab_0(b_lin_0)) * 100.0f;
    }
    else
    {
        var lab_a_0 : vec3<f32> = xyz_to_lab_0(linear_to_xyz_0(a_lin_0));
        var lab_b_0 : vec3<f32> = xyz_to_lab_0(linear_to_xyz_0(b_lin_0));
        var dE_1 : f32 = length(lab_a_0 - lab_b_0);
        if(formula_0 == u32(2))
        {
            var dL_0 : f32 = lab_a_0.x - lab_b_0.x;
            var _S12 : vec2<f32> = lab_a_0.yz;
            var _S13 : f32 = length(_S12);
            var _S14 : vec2<f32> = lab_b_0.yz;
            var dC_0 : f32 = _S13 - length(_S14);
            var _S15 : vec2<f32> = _S12 - _S14;
            var SC_0 : f32 = 1.0f + 0.04500000178813934f * _S13;
            var SH_0 : f32 = 1.0f + 0.01499999966472387f * _S13;
            dE_0 = sqrt(dL_0 * dL_0 + dC_0 / SC_0 * (dC_0 / SC_0) + max(dot(_S15, _S15) - dC_0 * dC_0, 0.0f) / (SH_0 * SH_0));
        }
        else
        {
            if(formula_0 == u32(3))
            {
                dE_0 = dE_1 * 0.85000002384185791f;
            }
            else
            {
                dE_0 = dE_1;
            }
        }
    }
    var final_0 : vec3<f32> = ramp_0(saturate((dE_0 + DeltaECompareParams_0.rangePack_0.w) * DeltaECompareParams_0.rangePack_0.z / max(DeltaECompareParams_0.rangePack_0.x, 0.00100000004749745f)), DeltaECompareParams_0.modePack_0.y) * vec3<f32>(DeltaECompareParams_0.mixPack_0.z) + a_srgb_0 * vec3<f32>(DeltaECompareParams_0.mixPack_0.x) + b_srgb_0 * vec3<f32>(DeltaECompareParams_0.mixPack_0.y);
    var _S16 : bool;
    if((DeltaECompareParams_0.modePack_0.z) > u32(0))
    {
        _S16 = dE_0 > (DeltaECompareParams_0.rangePack_0.y);
    }
    else
    {
        _S16 = false;
    }
    var final_1 : vec3<f32>;
    if(_S16)
    {
        final_1 = mix(final_0, vec3<f32>(1.0f, 0.0f, 0.0f), vec3<f32>(0.34999999403953552f));
    }
    else
    {
        final_1 = final_0;
    }
    var _S17 : pixelOutput_0 = pixelOutput_0( vec4<f32>(saturate(final_1), 1.0f) * _S11.col_0 );
    return _S17;
}

