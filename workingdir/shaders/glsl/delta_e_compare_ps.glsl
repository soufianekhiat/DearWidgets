#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 22 0
layout(binding = 3)
uniform texture2D texture0_0;


#line 21
layout(binding = 2)
uniform sampler sampler0_0;

layout(binding = 5)
uniform texture2D texture1_0;


#line 23
layout(binding = 4)
uniform sampler sampler1_0;


#line 18
struct SLANG_ParameterGroup_DeltaECompareParams_std140_0
{
    uvec4 modePack_0;
    vec4 rangePack_0;
    vec4 mixPack_0;
};


#line 12
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_DeltaECompareParams_std140_0
{
    uvec4 modePack_0;
    vec4 rangePack_0;
    vec4 mixPack_0;
}DeltaECompareParams_0;

#line 39
vec3 srgb_to_linear_0(vec3 c_0)
{


    return mix(c_0 / 12.92000007629394531, pow((c_0 + 0.05499999970197678) / 1.0549999475479126, vec3(2.40000009536743164)), step(vec3(0.04044999927282333), c_0));
}


vec3 linear_to_oklab_0(vec3 rgb_0)
{
    float _S1 = rgb_0.x;

#line 49
    float _S2 = rgb_0.y;

#line 49
    float _S3 = rgb_0.z;


    float l_0 = pow(max(0.41222146153450012 * _S1 + 0.53633254766464233 * _S2 + 0.05144599452614784 * _S3, 0.0), 0.3333333432674408);
    float m_0 = pow(max(0.21190349757671356 * _S1 + 0.68069952726364136 * _S2 + 0.10739696025848389 * _S3, 0.0), 0.3333333432674408);
    float s_0 = pow(max(0.08830246329307556 * _S1 + 0.28171885013580322 * _S2 + 0.6299787163734436 * _S3, 0.0), 0.3333333432674408);
    return vec3(0.21045425534248352 * l_0 + 0.79361778497695923 * m_0 - 0.00407204683870077 * s_0, 1.97799849510192871 * l_0 - 2.42859220504760742 * m_0 + 0.45059370994567871 * s_0, 0.02590403705835342 * l_0 + 0.7827717661857605 * m_0 - 0.80867576599121094 * s_0);
}


#line 62
vec3 linear_to_xyz_0(vec3 rgb_1)
{

    float _S4 = rgb_1.x;

#line 65
    float _S5 = rgb_1.y;

#line 65
    float _S6 = rgb_1.z;

#line 64
    return vec3(0.41245639324188232 * _S4 + 0.35757610201835632 * _S5 + 0.18043750524520874 * _S6, 0.21267290413379669 * _S4 + 0.71515220403671265 * _S5 + 0.07217500358819962 * _S6, 0.01933390088379383 * _S4 + 0.11919199675321579 * _S5 + 0.95030409097671509 * _S6);
}




float lab_f_0(float t_0)
{

#line 70
    float _S7;

    if(t_0 > 0.00885600037872791)
    {

#line 72
        _S7 = pow(t_0, 0.3333333432674408);

#line 72
    }
    else
    {

#line 72
        _S7 = 7.78700017929077148 * t_0 + 0.13793103396892548;

#line 72
    }

#line 72
    return _S7;
}


#line 74
vec3 xyz_to_lab_0(vec3 xyz_0)
{


    vec3 n_0 = xyz_0 / vec3(0.950469970703125, 1.0, 1.08882999420166016);
    float fy_0 = lab_f_0(n_0.y);
    return vec3(116.0 * fy_0 - 16.0, 500.0 * (lab_f_0(n_0.x) - fy_0), 200.0 * (fy_0 - lab_f_0(n_0.z)));
}


#line 12386 1
float saturate_0(float x_0)
{

#line 12394
    return clamp(x_0, 0.0, 1.0);
}


#line 12401
vec3 saturate_1(vec3 x_1)
{

#line 12409
    return clamp(x_1, vec3(0.0), vec3(1.0));
}


#line 84 0
vec3 viridis_0(float t_1)
{

#line 85
    float _S8 = saturate_0(t_1);

#line 90
    return saturate_1(vec3(0.26700401306152344, 0.0048739998601377, 0.32941499352455139) + _S8 * (vec3(1.02600002288818359, 1.52300000190734863, 0.35499998927116394) + _S8 * (vec3(1.18299996852874756, -1.28299999237060547, -3.0150001049041748) + _S8 * vec3(-3.18000006675720215, 0.6940000057220459, 2.54699993133544922))));
}


#line 92
vec3 turbo_0(float t_2)
{

#line 93
    float _S9 = saturate_0(t_2);
    return saturate_1(vec3(34.6100006103515625 + _S9 * (-1565.949951171875 + _S9 * (23299.0390625 + _S9 * (-7.3095421875e+04 + _S9 * (1.080520234375e+05 - _S9 * 58346.3515625)))) * 9.99999997475242708e-07, 23.30999946594238281 + _S9 * (557.33001708984375 + _S9 * (1225.3299560546875 + _S9 * (-3574.9599609375 + _S9 * (1073.77001953125 + _S9 * 707.55999755859375)))) * 0.00100000004749745, 27.20000076293945312 + _S9 * (3211.10009765625 + _S9 * (-15327.9697265625 + _S9 * (27814.0 + _S9 * (-22569.1796875 + _S9 * 6838.66015625)))) * 0.00000999999974738));
}



vec3 magma_0(float t_3)
{

#line 100
    float _S10 = saturate_0(t_3);

#line 105
    return saturate_1(vec3(-0.0020000000949949, -0.0020000000949949, -0.0130000002682209) + _S10 * (vec3(0.25, 0.15000000596046448, 0.69999998807907104) + _S10 * (vec3(1.60000002384185791, 0.34999999403953552, 0.60000002384185791) + _S10 * vec3(-1.10000002384185791, -0.20000000298023224, -1.89999997615814209))));
}

vec3 ramp_0(float t_4, uint mode_0)
{

#line 109
    if(mode_0 == 1U)
    {

#line 109
        return viridis_0(t_4);
    }

#line 110
    if(mode_0 == 2U)
    {

#line 110
        return turbo_0(t_4);
    }

#line 111
    if(mode_0 == 3U)
    {

#line 111
        return magma_0(t_4);
    }

#line 112
    return vec3(t_4, t_4, t_4);
}


#line 112
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 112
layout(location = 0)
in vec4 input_col_0;


#line 112
layout(location = 1)
in vec2 input_uv_0;

void main()
{
    vec3 a_srgb_0 = (texture(sampler2D(texture0_0,sampler0_0), (input_uv_0))).xyz;
    vec3 b_srgb_0 = (texture(sampler2D(texture1_0,sampler1_0), (input_uv_0))).xyz;
    vec3 a_lin_0 = srgb_to_linear_0(a_srgb_0);
    vec3 b_lin_0 = srgb_to_linear_0(b_srgb_0);

    uint formula_0 = DeltaECompareParams_0.modePack_0.x;

#line 122
    float dE_0;

    if(formula_0 == 1U)
    {

#line 124
        dE_0 = length(linear_to_oklab_0(a_lin_0) - linear_to_oklab_0(b_lin_0)) * 100.0;

#line 124
    }
    else
    {

#line 134
        vec3 lab_a_0 = xyz_to_lab_0(linear_to_xyz_0(a_lin_0));
        vec3 lab_b_0 = xyz_to_lab_0(linear_to_xyz_0(b_lin_0));
        float dE_1 = length(lab_a_0 - lab_b_0);
        if(formula_0 == 2U)
        {

            float dL_0 = lab_a_0.x - lab_b_0.x;
            vec2 _S11 = lab_a_0.yz;

#line 141
            float _S12 = length(_S11);

#line 141
            vec2 _S13 = lab_b_0.yz;

#line 141
            float dC_0 = _S12 - length(_S13);
            vec2 _S14 = _S11 - _S13;

            float SC_0 = 1.0 + 0.04500000178813934 * _S12;
            float SH_0 = 1.0 + 0.01499999966472387 * _S12;

#line 145
            dE_0 = sqrt(dL_0 * dL_0 + dC_0 / SC_0 * (dC_0 / SC_0) + max(dot(_S14, _S14) - dC_0 * dC_0, 0.0) / (SH_0 * SH_0));

#line 137
        }
        else
        {

#line 148
            if(formula_0 == 3U)
            {

#line 148
                dE_0 = dE_1 * 0.85000002384185791;

#line 148
            }
            else
            {

#line 148
                dE_0 = dE_1;

#line 148
            }

#line 137
        }

#line 124
    }

#line 162
    vec3 final_0 = ramp_0(saturate_0((dE_0 + DeltaECompareParams_0.rangePack_0.w) * DeltaECompareParams_0.rangePack_0.z / max(DeltaECompareParams_0.rangePack_0.x, 0.00100000004749745)), DeltaECompareParams_0.modePack_0.y) * DeltaECompareParams_0.mixPack_0.z + a_srgb_0 * DeltaECompareParams_0.mixPack_0.x + b_srgb_0 * DeltaECompareParams_0.mixPack_0.y;

#line 162
    bool _S15;


    if((DeltaECompareParams_0.modePack_0.z) > 0U)
    {

#line 165
        _S15 = dE_0 > (DeltaECompareParams_0.rangePack_0.y);

#line 165
    }
    else
    {

#line 165
        _S15 = false;

#line 165
    }

#line 165
    vec3 final_1;

#line 165
    if(_S15)
    {

#line 165
        final_1 = mix(final_0, vec3(1.0, 0.0, 0.0), vec3(0.34999999403953552));

#line 165
    }
    else
    {

#line 165
        final_1 = final_0;

#line 165
    }

#line 165
    entryPointParam_main_ps_0 = vec4(saturate_1(final_1), 1.0) * input_col_0;

#line 165
    return;
}

