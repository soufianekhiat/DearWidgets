#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 22 0
layout(binding = 0)
uniform sampler2D texture0_0;


#line 21
layout(binding = 1)
uniform sampler2D texture1_0;


#line 23
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


#line 86 0
vec3 viridis_0(float t_1)
{

#line 87
    float _S8 = saturate_0(t_1);

#line 95
    return saturate_1(vec3(0.27772733569145203, 0.00540734454989433, 0.33409979939460754) + _S8 * (vec3(0.10509303957223892, 1.40461349487304688, 1.38459014892578125) + _S8 * (vec3(-0.33086183667182922, 0.21484756469726562, 0.09509516507387161) + _S8 * (vec3(-4.63423061370849609, -5.79910087585449219, -19.33244132995605469) + _S8 * (vec3(6.22827005386352539, 14.17993354797363281, 56.6905517578125) + _S8 * (vec3(4.77638483047485352, -13.74514579772949219, -65.35303497314453125) + _S8 * vec3(-5.4354557991027832, 4.64585256576538086, 26.31243515014648438)))))));
}


#line 97
vec3 turbo_0(float t_2)
{

#line 98
    float _S9 = saturate_0(t_2);

#line 105
    float _S10 = _S9 * _S9;

#line 105
    float _S11 = _S10 * _S9;

#line 105
    vec4 v4_0 = vec4(1.0, _S9, _S10, _S11);
    vec2 v2_0 = vec2(_S10, _S11) * _S10;
    return saturate_1(vec3(dot(v4_0, vec4(0.13572138547897339, 4.61539268493652344, -42.6603240966796875, 132.1310882568359375)) + dot(v2_0, vec2(-152.9423980712890625, 59.28638076782226562)), dot(v4_0, vec4(0.09140261262655258, 2.19418835639953613, 4.84296655654907227, -14.18503379821777344)) + dot(v2_0, vec2(4.2772984504699707, 2.82956600189208984)), dot(v4_0, vec4(0.10667330026626587, 12.64194583892822266, -60.5820465087890625, 110.36277008056640625)) + dot(v2_0, vec2(-89.903106689453125, 27.34824943542480469))));
}


#line 109
vec3 magma_0(float t_3)
{

#line 110
    float _S12 = saturate_0(t_3);

#line 118
    return saturate_1(vec3(-0.00213648495264351, -0.00074965506792068, -0.00538612762466073) + _S12 * (vec3(0.25166055560112, 0.67752325534820557, 2.49402666091918945) + _S12 * (vec3(8.35371685028076172, -3.57771944999694824, 0.3144679069519043) + _S12 * (vec3(-27.66873359680175781, 14.26473045349121094, -13.64921283721923828) + _S12 * (vec3(52.17613983154296875, -27.94360542297363281, 12.94416904449462891) + _S12 * (vec3(-50.768524169921875, 29.04658317565917969, 4.23415279388427734) + _S12 * vec3(18.65570449829101562, -11.48977375030517578, -5.60196161270141602)))))));
}

vec3 ramp_0(float t_4, uint mode_0)
{

#line 122
    if(mode_0 == 1U)
    {

#line 122
        return viridis_0(t_4);
    }

#line 123
    if(mode_0 == 2U)
    {

#line 123
        return turbo_0(t_4);
    }

#line 124
    if(mode_0 == 3U)
    {

#line 124
        return magma_0(t_4);
    }

#line 125
    return vec3(t_4, t_4, t_4);
}


#line 125
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 125
layout(location = 0)
in vec4 input_col_0;


#line 125
layout(location = 1)
in vec2 input_uv_0;

void main()
{
    vec3 a_srgb_0 = (texture(texture0_0, (input_uv_0))).xyz;
    vec3 b_srgb_0 = (texture(texture1_0, (input_uv_0))).xyz;
    vec3 a_lin_0 = srgb_to_linear_0(a_srgb_0);
    vec3 b_lin_0 = srgb_to_linear_0(b_srgb_0);

    uint formula_0 = DeltaECompareParams_0.modePack_0.x;

#line 135
    float dE_0;

    if(formula_0 == 1U)
    {

#line 137
        dE_0 = length(linear_to_oklab_0(a_lin_0) - linear_to_oklab_0(b_lin_0)) * 100.0;

#line 137
    }
    else
    {

#line 147
        vec3 lab_a_0 = xyz_to_lab_0(linear_to_xyz_0(a_lin_0));
        vec3 lab_b_0 = xyz_to_lab_0(linear_to_xyz_0(b_lin_0));
        float dE_1 = length(lab_a_0 - lab_b_0);
        if(formula_0 == 2U)
        {

            float dL_0 = lab_a_0.x - lab_b_0.x;
            vec2 _S13 = lab_a_0.yz;

#line 154
            float _S14 = length(_S13);

#line 154
            vec2 _S15 = lab_b_0.yz;

#line 154
            float dC_0 = _S14 - length(_S15);
            vec2 _S16 = _S13 - _S15;

            float SC_0 = 1.0 + 0.04500000178813934 * _S14;
            float SH_0 = 1.0 + 0.01499999966472387 * _S14;

#line 158
            dE_0 = sqrt(dL_0 * dL_0 + dC_0 / SC_0 * (dC_0 / SC_0) + max(dot(_S16, _S16) - dC_0 * dC_0, 0.0) / (SH_0 * SH_0));

#line 150
        }
        else
        {

#line 161
            if(formula_0 == 3U)
            {

#line 161
                dE_0 = dE_1 * 0.85000002384185791;

#line 161
            }
            else
            {

#line 161
                dE_0 = dE_1;

#line 161
            }

#line 150
        }

#line 137
    }

#line 175
    vec3 final_0 = ramp_0(saturate_0((dE_0 + DeltaECompareParams_0.rangePack_0.w) * DeltaECompareParams_0.rangePack_0.z / max(DeltaECompareParams_0.rangePack_0.x, 0.00100000004749745)), DeltaECompareParams_0.modePack_0.y) * DeltaECompareParams_0.mixPack_0.z + a_srgb_0 * DeltaECompareParams_0.mixPack_0.x + b_srgb_0 * DeltaECompareParams_0.mixPack_0.y;

#line 175
    bool _S17;


    if((DeltaECompareParams_0.modePack_0.z) > 0U)
    {

#line 178
        _S17 = dE_0 > (DeltaECompareParams_0.rangePack_0.y);

#line 178
    }
    else
    {

#line 178
        _S17 = false;

#line 178
    }

#line 178
    vec3 final_1;

#line 178
    if(_S17)
    {

#line 178
        final_1 = mix(final_0, vec3(1.0, 0.0, 0.0), vec3(0.34999999403953552));

#line 178
    }
    else
    {

#line 178
        final_1 = final_0;

#line 178
    }

#line 178
    entryPointParam_main_ps_0 = vec4(saturate_1(final_1), 1.0) * input_col_0;

#line 178
    return;
}

