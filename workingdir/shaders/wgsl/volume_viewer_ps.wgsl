struct SLANG_ParameterGroup_VolumeViewerParams_std140_0
{
    @align(16) camRight_0 : vec4<f32>,
    @align(16) camUp_0 : vec4<f32>,
    @align(16) camForward_0 : vec4<f32>,
    @align(16) camPos_0 : vec4<f32>,
    @align(16) winPack_0 : vec4<f32>,
    @align(16) modePack_0 : vec4<u32>,
    @align(16) volDims_0 : vec4<f32>,
    @align(16) bgColor_0 : vec4<f32>,
};

@binding(1) @group(0) var<uniform> VolumeViewerParams_0 : SLANG_ParameterGroup_VolumeViewerParams_std140_0;
@binding(5) @group(0) var volume_0 : texture_3d<f32>;

@binding(4) @group(0) var volumeSampler_0 : sampler;

fn ray_box_0( ro_0 : vec3<f32>,  rd_0 : vec3<f32>,  bmin_0 : vec3<f32>,  bmax_0 : vec3<f32>) -> vec2<f32>
{
    var inv_0 : vec3<f32> = vec3<f32>(1.0f) / rd_0;
    var t0_0 : vec3<f32> = (bmin_0 - ro_0) * inv_0;
    var t1_0 : vec3<f32> = (bmax_0 - ro_0) * inv_0;
    var tmin3_0 : vec3<f32> = min(t0_0, t1_0);
    var tmax3_0 : vec3<f32> = max(t0_0, t1_0);
    return vec2<f32>(max(max(tmin3_0.x, tmin3_0.y), tmin3_0.z), min(min(tmax3_0.x, tmax3_0.y), tmax3_0.z));
}

fn sample_vol_0( uvw_0 : vec3<f32>) -> f32
{
    return (textureSampleLevel((volume_0), (volumeSampler_0), (uvw_0), (0.0f))).x;
}

fn window_map_0( v_0 : f32,  wmin_0 : f32,  wmax_0 : f32,  gamma_0 : f32) -> f32
{
    return pow(saturate((v_0 - wmin_0) / max(wmax_0 - wmin_0, 9.99999997475242708e-07f)), 1.0f / max(gamma_0, 0.00009999999747379f));
}

fn apply_ramp_0( t_0 : f32,  mode_0 : u32) -> vec3<f32>
{
    if(mode_0 == u32(1))
    {
        var _S1 : vec3<f32> = vec3<f32>(saturate(t_0));
        return saturate(vec3<f32>(0.2669999897480011f, 0.00499999988824129f, 0.32899999618530273f) + _S1 * (vec3<f32>(0.15000000596046448f, 1.38999998569488525f, 0.80000001192092896f) + _S1 * (vec3<f32>(2.40000009536743164f, -0.69999998807907104f, -2.20000004768371582f) + _S1 * vec3<f32>(-1.89999997615814209f, 0.44999998807907104f, 1.5f))));
    }
    return vec3<f32>(t_0, t_0, t_0);
}

fn gradient_0( uvw_1 : vec3<f32>,  inv_dims_0 : vec3<f32>) -> vec3<f32>
{
    var _S2 : vec3<f32> = vec3<f32>(inv_dims_0.x, 0.0f, 0.0f);
    var _S3 : vec3<f32> = vec3<f32>(0.0f, inv_dims_0.y, 0.0f);
    var _S4 : vec3<f32> = vec3<f32>(0.0f, 0.0f, inv_dims_0.z);
    return vec3<f32>(sample_vol_0(uvw_1 + _S2) - sample_vol_0(uvw_1 - _S2), sample_vol_0(uvw_1 + _S3) - sample_vol_0(uvw_1 - _S3), sample_vol_0(uvw_1 + _S4) - sample_vol_0(uvw_1 - _S4));
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
fn main_ps( _S5 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    var ndc_0 : vec2<f32> = _S5.uv_0 * vec2<f32>(2.0f) - vec2<f32>(1.0f);
    ndc_0[i32(0)] = ndc_0[i32(0)] * VolumeViewerParams_0.camRight_0.w;
    ndc_0[i32(1)] = - ndc_0.y;
    var ro_1 : vec3<f32> = VolumeViewerParams_0.camPos_0.xyz;
    var _S6 : vec3<f32> = vec3<f32>(VolumeViewerParams_0.camUp_0.w);
    var rd_1 : vec3<f32> = normalize(VolumeViewerParams_0.camForward_0.xyz + VolumeViewerParams_0.camRight_0.xyz * vec3<f32>(ndc_0.x) * _S6 + VolumeViewerParams_0.camUp_0.xyz * vec3<f32>(ndc_0.y) * _S6);
    const _S7 : vec3<f32> = vec3<f32>(0.0f, 0.0f, 0.0f);
    const _S8 : vec3<f32> = vec3<f32>(1.0f, 1.0f, 1.0f);
    var t_1 : vec2<f32> = ray_box_0(ro_1, rd_1, _S7, _S8);
    var _S9 : f32 = t_1.x;
    var _S10 : f32 = t_1.y;
    var hit_0 : bool;
    if(_S9 > _S10)
    {
        hit_0 = true;
    }
    else
    {
        hit_0 = _S10 < 0.0f;
    }
    if(hit_0)
    {
        var _S11 : pixelOutput_0 = pixelOutput_0( VolumeViewerParams_0.bgColor_0 * _S5.col_0 );
        return _S11;
    }
    var ts_0 : f32 = max(_S9, 0.0f);
    var step_0 : f32 = max(VolumeViewerParams_0.camPos_0.w, 0.00009999999747379f);
    var _S12 : u32 = min(max(VolumeViewerParams_0.modePack_0.z, u32(1)), u32((_S10 - ts_0) / step_0 + 1.0f));
    var mode_1 : u32 = VolumeViewerParams_0.modePack_0.x;
    var ramp_0 : u32 = VolumeViewerParams_0.modePack_0.y;
    var wmin_1 : f32 = VolumeViewerParams_0.winPack_0.x;
    var wmax_1 : f32 = VolumeViewerParams_0.winPack_0.y;
    var gamma_1 : f32 = VolumeViewerParams_0.winPack_0.z;
    var _S13 : f32 = VolumeViewerParams_0.winPack_0.w;
    var inv_dims_1 : vec3<f32> = vec3<f32>(1.0f) / max(VolumeViewerParams_0.volDims_0.xyz, _S8);
    var _S14 : vec3<f32> = VolumeViewerParams_0.bgColor_0.xyz;
    var i_0 : u32;
    var m_0 : f32;
    var final_a_0 : f32;
    var final_rgb_0 : vec3<f32>;
    if(mode_1 == u32(1))
    {
        m_0 = 0.0f;
        i_0 = u32(0);
        for(;;)
        {
            if(i_0 < _S12)
            {
            }
            else
            {
                break;
            }
            var _S15 : f32 = max(m_0, sample_vol_0(ro_1 + rd_1 * vec3<f32>((ts_0 + f32(i_0) * step_0))));
            var i_1 : u32 = i_0 + u32(1);
            m_0 = _S15;
            i_0 = i_1;
        }
        final_rgb_0 = apply_ramp_0(window_map_0(m_0, wmin_1, wmax_1, gamma_1), ramp_0);
        final_a_0 = 1.0f;
    }
    else
    {
        var hit_uvw_0 : vec3<f32>;
        if(mode_1 == u32(2))
        {
            m_0 = sample_vol_0(ro_1 + rd_1 * vec3<f32>(ts_0));
            i_0 = u32(1);
            for(;;)
            {
                if(i_0 < _S12)
                {
                }
                else
                {
                    hit_0 = false;
                    hit_uvw_0 = _S7;
                    break;
                }
                var s_0 : f32 = ts_0 + f32(i_0) * step_0;
                var v_1 : f32 = sample_vol_0(ro_1 + rd_1 * vec3<f32>(s_0));
                if(((m_0 - _S13) * (v_1 - _S13)) < 0.0f)
                {
                    var _S16 : vec3<f32> = ro_1 + rd_1 * vec3<f32>((s_0 - step_0 + (_S13 - m_0) / max(v_1 - m_0, 9.99999997475242708e-07f) * step_0));
                    hit_0 = true;
                    hit_uvw_0 = _S16;
                    break;
                }
                var i_2 : u32 = i_0 + u32(1);
                m_0 = v_1;
                i_0 = i_2;
            }
            if(hit_0)
            {
                var g_0 : vec3<f32> = gradient_0(hit_uvw_0, inv_dims_1);
                var gl_0 : f32 = length(g_0);
                if(gl_0 > 9.99999997475242708e-07f)
                {
                    final_rgb_0 = g_0 / vec3<f32>(gl_0);
                }
                else
                {
                    final_rgb_0 = vec3<f32>(0.0f, 1.0f, 0.0f);
                }
                if((dot(final_rgb_0, rd_1)) > 0.0f)
                {
                    final_rgb_0 = - final_rgb_0;
                }
                final_rgb_0 = apply_ramp_0(window_map_0(sample_vol_0(hit_uvw_0), wmin_1, wmax_1, gamma_1), ramp_0) * vec3<f32>((0.20000000298023224f + 0.80000001192092896f * max(dot(final_rgb_0, normalize(vec3<f32>(0.40000000596046448f, 0.69999998807907104f, 0.5f))), 0.0f)));
                final_a_0 = 1.0f;
            }
            else
            {
                final_rgb_0 = _S14;
                final_a_0 = 0.0f;
            }
        }
        else
        {
            i_0 = u32(0);
            m_0 = 0.0f;
            hit_uvw_0 = _S7;
            for(;;)
            {
                if(i_0 < _S12)
                {
                }
                else
                {
                    break;
                }
                var t01_0 : f32 = window_map_0(sample_vol_0(ro_1 + rd_1 * vec3<f32>((ts_0 + f32(i_0) * step_0))), wmin_1, wmax_1, gamma_1);
                var a_0 : f32 = saturate(t01_0 * step_0 * 4.0f);
                var _S17 : f32 = 1.0f - m_0;
                var acc_rgb_0 : vec3<f32> = hit_uvw_0 + vec3<f32>(_S17) * apply_ramp_0(t01_0, ramp_0) * vec3<f32>(a_0);
                var acc_a_0 : f32 = m_0 + _S17 * a_0;
                if(acc_a_0 > 0.99000000953674316f)
                {
                    hit_uvw_0 = acc_rgb_0;
                    m_0 = acc_a_0;
                    break;
                }
                i_0 = i_0 + u32(1);
                m_0 = acc_a_0;
                hit_uvw_0 = acc_rgb_0;
            }
            final_rgb_0 = hit_uvw_0 + VolumeViewerParams_0.bgColor_0.xyz * vec3<f32>((1.0f - m_0));
            final_a_0 = 1.0f;
        }
    }
    var _S18 : pixelOutput_0 = pixelOutput_0( vec4<f32>(final_rgb_0, final_a_0) * _S5.col_0 );
    return _S18;
}

