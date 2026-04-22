#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 32 0
struct SLANG_ParameterGroup_VolumeViewerParams_std140_0
{
    vec4 camRight_0;
    vec4 camUp_0;
    vec4 camForward_0;
    vec4 camPos_0;
    vec4 winPack_0;
    uvec4 modePack_0;
    vec4 volDims_0;
    vec4 bgColor_0;
};


#line 20
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_VolumeViewerParams_std140_0
{
    vec4 camRight_0;
    vec4 camUp_0;
    vec4 camForward_0;
    vec4 camPos_0;
    vec4 winPack_0;
    uvec4 modePack_0;
    vec4 volDims_0;
    vec4 bgColor_0;
}VolumeViewerParams_0;

#line 38
layout(binding = 5)
uniform texture3D volume_0;


#line 37
layout(binding = 4)
uniform sampler volumeSampler_0;


#line 53
vec2 ray_box_0(vec3 ro_0, vec3 rd_0, vec3 bmin_0, vec3 bmax_0)
{
    vec3 inv_0 = 1.0 / rd_0;
    vec3 t0_0 = (bmin_0 - ro_0) * inv_0;
    vec3 t1_0 = (bmax_0 - ro_0) * inv_0;
    vec3 tmin3_0 = min(t0_0, t1_0);
    vec3 tmax3_0 = max(t0_0, t1_0);


    return vec2(max(max(tmin3_0.x, tmin3_0.y), tmin3_0.z), min(min(tmax3_0.x, tmax3_0.y), tmax3_0.z));
}

float sample_vol_0(vec3 uvw_0)
{

    return (textureLod(sampler3D(volume_0,volumeSampler_0), (uvw_0), (0.0))).x;
}


#line 12386 1
float saturate_0(float x_0)
{

#line 12394
    return clamp(x_0, 0.0, 1.0);
}


#line 71 0
float window_map_0(float v_0, float wmin_0, float wmax_0, float gamma_0)
{

    return pow(saturate_0((v_0 - wmin_0) / max(wmax_0 - wmin_0, 9.99999997475242708e-07)), 1.0 / max(gamma_0, 0.00009999999747379));
}


#line 12401 1
vec3 saturate_1(vec3 x_1)
{

#line 12409
    return clamp(x_1, vec3(0.0), vec3(1.0));
}


#line 78 0
vec3 apply_ramp_0(float t_0, uint mode_0)
{
    if(mode_0 == 1U)
    {
        float _S1 = saturate_0(t_0);

#line 87
        return saturate_1(vec3(0.2669999897480011, 0.00499999988824129, 0.32899999618530273) + _S1 * (vec3(0.15000000596046448, 1.38999998569488525, 0.80000001192092896) + _S1 * (vec3(2.40000009536743164, -0.69999998807907104, -2.20000004768371582) + _S1 * vec3(-1.89999997615814209, 0.44999998807907104, 1.5))));
    }
    return vec3(t_0, t_0, t_0);
}


vec3 gradient_0(vec3 uvw_1, vec3 inv_dims_0)
{
    vec3 _S2 = vec3(inv_dims_0.x, 0.0, 0.0);
    vec3 _S3 = vec3(0.0, inv_dims_0.y, 0.0);
    vec3 _S4 = vec3(0.0, 0.0, inv_dims_0.z);
    return vec3(sample_vol_0(uvw_1 + _S2) - sample_vol_0(uvw_1 - _S2), sample_vol_0(uvw_1 + _S3) - sample_vol_0(uvw_1 - _S3), sample_vol_0(uvw_1 + _S4) - sample_vol_0(uvw_1 - _S4));
}


#line 98
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 98
layout(location = 0)
in vec4 input_col_0;


#line 98
layout(location = 1)
in vec2 input_uv_0;

void main()
{

    vec2 ndc_0 = input_uv_0 * 2.0 - 1.0;
    ndc_0[0] = ndc_0[0] * VolumeViewerParams_0.camRight_0.w;



    ndc_0[1] = - ndc_0.y;
    float fov_tan_0 = VolumeViewerParams_0.camUp_0.w;
    vec3 ro_1 = VolumeViewerParams_0.camPos_0.xyz;
    vec3 rd_1 = normalize(VolumeViewerParams_0.camForward_0.xyz + VolumeViewerParams_0.camRight_0.xyz * ndc_0.x * fov_tan_0 + VolumeViewerParams_0.camUp_0.xyz * ndc_0.y * fov_tan_0);



    const vec3 _S5 = vec3(0.0, 0.0, 0.0);

#line 116
    const vec3 _S6 = vec3(1.0, 1.0, 1.0);

#line 116
    vec2 t_1 = ray_box_0(ro_1, rd_1, _S5, _S6);
    float _S7 = t_1.x;

#line 117
    float _S8 = t_1.y;

#line 117
    bool hit_0;

#line 117
    if(_S7 > _S8)
    {

#line 117
        hit_0 = true;

#line 117
    }
    else
    {

#line 117
        hit_0 = _S8 < 0.0;

#line 117
    }

#line 117
    if(hit_0)
    {

#line 117
        entryPointParam_main_ps_0 = VolumeViewerParams_0.bgColor_0 * input_col_0;

#line 117
        return;
    }

    float ts_0 = max(_S7, 0.0);

    float step_0 = max(VolumeViewerParams_0.camPos_0.w, 0.00009999999747379);

    uint _S9 = min(max(VolumeViewerParams_0.modePack_0.z, 1U), uint((_S8 - ts_0) / step_0 + 1.0));

    uint mode_1 = VolumeViewerParams_0.modePack_0.x;
    uint ramp_0 = VolumeViewerParams_0.modePack_0.y;
    float wmin_1 = VolumeViewerParams_0.winPack_0.x;
    float wmax_1 = VolumeViewerParams_0.winPack_0.y;
    float gamma_1 = VolumeViewerParams_0.winPack_0.z;
    float _S10 = VolumeViewerParams_0.winPack_0.w;
    vec3 inv_dims_1 = 1.0 / max(VolumeViewerParams_0.volDims_0.xyz, _S6);

    vec3 _S11 = VolumeViewerParams_0.bgColor_0.xyz;

#line 134
    uint i_0;

#line 134
    float m_0;

#line 134
    float final_a_0;

#line 134
    vec3 final_rgb_0;


    if(mode_1 == 1U)
    {

#line 137
        m_0 = 0.0;

#line 137
        i_0 = 0U;


        for(;;)
        {

#line 140
            if(i_0 < _S9)
            {
            }
            else
            {

#line 140
                break;
            }


            float _S12 = max(m_0, sample_vol_0(ro_1 + rd_1 * (ts_0 + float(i_0) * step_0)));

#line 140
            uint i_1 = i_0 + 1U;

#line 140
            m_0 = _S12;

#line 140
            i_0 = i_1;

#line 140
        }

#line 140
        final_rgb_0 = apply_ramp_0(window_map_0(m_0, wmin_1, wmax_1, gamma_1), ramp_0);

#line 140
        final_a_0 = 1.0;

#line 137
    }
    else
    {

#line 137
        vec3 hit_uvw_0;

#line 150
        if(mode_1 == 2U)
        {

#line 150
            m_0 = sample_vol_0(ro_1 + rd_1 * ts_0);

#line 150
            i_0 = 1U;

#line 155
            for(;;)
            {

#line 155
                if(i_0 < _S9)
                {
                }
                else
                {

#line 155
                    hit_0 = false;

#line 155
                    hit_uvw_0 = _S5;

#line 155
                    break;
                }
                float s_0 = ts_0 + float(i_0) * step_0;

                float v_1 = sample_vol_0(ro_1 + rd_1 * s_0);
                if(((m_0 - _S10) * (v_1 - _S10)) < 0.0)
                {

                    vec3 _S13 = ro_1 + rd_1 * (s_0 - step_0 + (_S10 - m_0) / max(v_1 - m_0, 9.99999997475242708e-07) * step_0);

#line 163
                    hit_0 = true;

#line 163
                    hit_uvw_0 = _S13;

                    break;
                }

#line 155
                uint i_2 = i_0 + 1U;

#line 155
                m_0 = v_1;

#line 155
                i_0 = i_2;

#line 155
            }

#line 169
            if(hit_0)
            {
                vec3 g_0 = gradient_0(hit_uvw_0, inv_dims_1);
                float gl_0 = length(g_0);
                if(gl_0 > 9.99999997475242708e-07)
                {

#line 173
                    final_rgb_0 = g_0 / gl_0;

#line 173
                }
                else
                {

#line 173
                    final_rgb_0 = vec3(0.0, 1.0, 0.0);

#line 173
                }

                if((dot(final_rgb_0, rd_1)) > 0.0)
                {

#line 175
                    final_rgb_0 = - final_rgb_0;

#line 175
                }
                else
                {

#line 175
                }

#line 175
                final_rgb_0 = apply_ramp_0(window_map_0(sample_vol_0(hit_uvw_0), wmin_1, wmax_1, gamma_1), ramp_0) * (0.20000000298023224 + 0.80000001192092896 * max(dot(final_rgb_0, normalize(vec3(0.40000000596046448, 0.69999998807907104, 0.5))), 0.0));

#line 175
                final_a_0 = 1.0;

#line 169
            }
            else
            {

#line 169
                final_rgb_0 = _S11;

#line 169
                final_a_0 = 0.0;

#line 169
            }

#line 150
        }
        else
        {

#line 150
            i_0 = 0U;

#line 150
            m_0 = 0.0;

#line 150
            hit_uvw_0 = _S5;

#line 186
            for(;;)
            {

#line 186
                if(i_0 < _S9)
                {
                }
                else
                {

#line 186
                    break;
                }


                float t01_0 = window_map_0(sample_vol_0(ro_1 + rd_1 * (ts_0 + float(i_0) * step_0)), wmin_1, wmax_1, gamma_1);


                float a_0 = saturate_0(t01_0 * step_0 * 4.0);
                float _S14 = 1.0 - m_0;

#line 194
                vec3 acc_rgb_0 = hit_uvw_0 + _S14 * apply_ramp_0(t01_0, ramp_0) * a_0;
                float acc_a_0 = m_0 + _S14 * a_0;
                if(acc_a_0 > 0.99000000953674316)
                {

#line 196
                    hit_uvw_0 = acc_rgb_0;

#line 196
                    m_0 = acc_a_0;

#line 196
                    break;
                }

#line 186
                i_0 = i_0 + 1U;

#line 186
                m_0 = acc_a_0;

#line 186
                hit_uvw_0 = acc_rgb_0;

#line 186
            }

#line 186
            final_rgb_0 = hit_uvw_0 + VolumeViewerParams_0.bgColor_0.xyz * (1.0 - m_0);

#line 186
            final_a_0 = 1.0;

#line 150
        }

#line 137
    }

#line 137
    entryPointParam_main_ps_0 = vec4(final_rgb_0, final_a_0) * input_col_0;

#line 137
    return;
}

