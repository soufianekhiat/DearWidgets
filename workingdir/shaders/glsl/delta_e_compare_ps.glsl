#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 22 0
layout(binding = 3)
uniform sampler2D texture0_0;


#line 21
layout(binding = 5)
uniform sampler2D texture1_0;


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


// Ramp fits mirrored from hlsl_src/delta_e_compare.hlsl (hand-kept: this file
// carries the OpenGL sampler fix, so it is not regenerated wholesale).
vec3 viridis_0(float t_1)
{
    float t = saturate_0(t_1);
    const vec3 c0 = vec3( 0.2777273272234177,  0.005407344544966578,  0.3340998053353061);
    const vec3 c1 = vec3( 0.1050930431085774,  1.404613529898575,     1.384590162594685);
    const vec3 c2 = vec3(-0.3308618287255563,  0.214847559468213,     0.09509516302823659);
    const vec3 c3 = vec3(-4.634230498983486,  -5.799100973351585,   -19.33244095627987);
    const vec3 c4 = vec3( 6.228269936347081,  14.17993336680509,     56.69055260068105);
    const vec3 c5 = vec3( 4.776384997670288, -13.74514537774601,    -65.35303263337234);
    const vec3 c6 = vec3(-5.435455855934631,   4.645852612178535,    26.3124352495832);
    return saturate_1(c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6))))));
}

vec3 turbo_0(float t_2)
{
    float t = saturate_0(t_2);
    const vec4 kRed4   = vec4(0.13572138, 4.61539260, -42.66032258, 132.13108234);
    const vec4 kGreen4 = vec4(0.09140261, 2.19418839, 4.84296658, -14.18503333);
    const vec4 kBlue4  = vec4(0.10667330, 12.64194608, -60.58204836, 110.36276771);
    const vec2 kRed2   = vec2(-152.94239396, 59.28637943);
    const vec2 kGreen2 = vec2(4.27729857, 2.82956604);
    const vec2 kBlue2  = vec2(-89.90310912, 27.34824973);
    vec4 v4 = vec4(1.0, t, t * t, t * t * t);
    vec2 v2 = v4.zw * v4.z;
    return saturate_1(vec3(dot(v4, kRed4) + dot(v2, kRed2), dot(v4, kGreen4) + dot(v2, kGreen2), dot(v4, kBlue4) + dot(v2, kBlue2)));
}

vec3 magma_0(float t_3)
{
    float t = saturate_0(t_3);
    const vec3 c0 = vec3(-0.002136485053939582, -0.000749655052795221, -0.005386127855323933);
    const vec3 c1 = vec3( 0.2516605407371642,    0.6775232436837668,    2.494026599312351);
    const vec3 c2 = vec3( 8.353717279216625,    -3.577719514958484,     0.3144679030132573);
    const vec3 c3 = vec3(-27.66873308576866,    14.26473078096533,    -13.64921318813922);
    const vec3 c4 = vec3( 52.17613981234068,   -27.94360607168351,     12.94416944238394);
    const vec3 c5 = vec3(-50.76852536473588,    29.04658282127291,      4.23415299384598);
    const vec3 c6 = vec3( 18.65570506591883,   -11.48977351997711,     -5.601961508734096);
    return saturate_1(c0 + t * (c1 + t * (c2 + t * (c3 + t * (c4 + t * (c5 + t * c6))))));
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
    vec3 a_srgb_0 = (texture(texture0_0, (input_uv_0))).xyz;
    vec3 b_srgb_0 = (texture(texture1_0, (input_uv_0))).xyz;
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

