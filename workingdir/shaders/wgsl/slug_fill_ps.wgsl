@binding(1) @group(0) var bandTexture_0 : texture_2d<f32>;

@binding(0) @group(0) var curveTexture_0 : texture_2d<f32>;

struct SLANG_ParameterGroup_fillParams_std140_0
{
    @align(16) fillColor0_0 : vec4<f32>,
    @align(16) fillColor1_0 : vec4<f32>,
    @align(16) fillBBox_0 : vec4<f32>,
    @align(16) fillGrad_0 : vec4<f32>,
    @align(16) fillUVStart_0 : vec4<f32>,
    @align(16) fillUVEnd_0 : vec4<f32>,
};

@binding(1) @group(0) var<uniform> fillParams_0 : SLANG_ParameterGroup_fillParams_std140_0;
@binding(2) @group(0) var fillTexture_0 : texture_2d<f32>;

@binding(0) @group(0) var fillSampler_0 : sampler;

fn BandLoad_0( coord_0 : vec2<i32>) -> vec2<i32>
{
    var _S1 : vec3<i32> = vec3<i32>(coord_0, i32(0));
    var raw_0 : vec4<f32> = (textureLoad((bandTexture_0), ((_S1)).xy, ((_S1)).z));
    return vec2<i32>(i32(raw_0.x + 0.5f), i32(raw_0.y + 0.5f));
}

fn CalcBandLoc_0( glyphLoc_0 : vec2<i32>,  offset_0 : u32) -> vec2<i32>
{
    var _S2 : i32 = glyphLoc_0.x + i32(offset_0);
    var loc_0 : vec2<i32> = vec2<i32>(_S2, glyphLoc_0.y);
    loc_0[i32(1)] = loc_0[i32(1)] + ((_S2 >> (u32(12))));
    loc_0[i32(0)] = ((loc_0[i32(0)]) & (i32(4095)));
    return loc_0;
}

fn CurveLoad_0( coord_1 : vec2<i32>) -> vec4<f32>
{
    var _S3 : vec3<i32> = vec3<i32>(coord_1, i32(0));
    return (textureLoad((curveTexture_0), ((_S3)).xy, ((_S3)).z));
}

fn CalcRootCode_0( y1_0 : f32,  y2_0 : f32,  y3_0 : f32) -> u32
{
    return (((u32(11892) >> ((((((((bitcast<u32>((y3_0))) >> (u32(29)))) & (u32(4)))) | ((((((((((bitcast<u32>((y2_0))) >> (u32(30)))) & (u32(2)))) | ((((((bitcast<u32>((y1_0))) >> (u32(31)))) & (u32(4294967293))))))) & (u32(4294967291)))))))))) & (u32(257)));
}

fn SolveHorizPoly_0( p12_0 : vec4<f32>,  p3_0 : vec2<f32>) -> vec2<f32>
{
    var _S4 : vec2<f32> = p12_0.xy;
    var _S5 : vec2<f32> = p12_0.zw;
    var a_0 : vec2<f32> = _S4 - _S5 * vec2<f32>(2.0f) + p3_0;
    var b_0 : vec2<f32> = _S4 - _S5;
    var _S6 : f32 = a_0.y;
    var ra_0 : f32 = 1.0f / _S6;
    var _S7 : f32 = b_0.y;
    var rb_0 : f32 = 0.5f / _S7;
    var _S8 : f32 = p12_0.y;
    var d_0 : f32 = sqrt(max(_S7 * _S7 - _S6 * _S8, 0.0f));
    var _S9 : f32 = (_S7 - d_0) * ra_0;
    var _S10 : f32 = (_S7 + d_0) * ra_0;
    var t1_0 : f32;
    var t2_0 : f32;
    if((abs(_S6)) < 0.0000152587890625f)
    {
        var t2_1 : f32 = _S8 * rb_0;
        t1_0 = t2_1;
        t2_0 = t2_1;
    }
    else
    {
        t1_0 = _S9;
        t2_0 = _S10;
    }
    var _S11 : f32 = a_0.x;
    var _S12 : f32 = b_0.x * 2.0f;
    var _S13 : f32 = p12_0.x;
    return vec2<f32>((_S11 * t1_0 - _S12) * t1_0 + _S13, (_S11 * t2_0 - _S12) * t2_0 + _S13);
}

fn SolveVertPoly_0( p12_1 : vec4<f32>,  p3_1 : vec2<f32>) -> vec2<f32>
{
    var _S14 : vec2<f32> = p12_1.xy;
    var _S15 : vec2<f32> = p12_1.zw;
    var a_1 : vec2<f32> = _S14 - _S15 * vec2<f32>(2.0f) + p3_1;
    var b_1 : vec2<f32> = _S14 - _S15;
    var _S16 : f32 = a_1.x;
    var ra_1 : f32 = 1.0f / _S16;
    var _S17 : f32 = b_1.x;
    var rb_1 : f32 = 0.5f / _S17;
    var _S18 : f32 = p12_1.x;
    var d_1 : f32 = sqrt(max(_S17 * _S17 - _S16 * _S18, 0.0f));
    var _S19 : f32 = (_S17 - d_1) * ra_1;
    var _S20 : f32 = (_S17 + d_1) * ra_1;
    var t1_1 : f32;
    var t2_2 : f32;
    if((abs(_S16)) < 0.0000152587890625f)
    {
        var t2_3 : f32 = _S18 * rb_1;
        t1_1 = t2_3;
        t2_2 = t2_3;
    }
    else
    {
        t1_1 = _S19;
        t2_2 = _S20;
    }
    var _S21 : f32 = a_1.y;
    var _S22 : f32 = b_1.y * 2.0f;
    var _S23 : f32 = p12_1.y;
    return vec2<f32>((_S21 * t1_1 - _S22) * t1_1 + _S23, (_S21 * t2_2 - _S22) * t2_2 + _S23);
}

fn CalcCoverage_0( xcov_0 : f32,  ycov_0 : f32,  xwgt_0 : f32,  ywgt_0 : f32) -> f32
{
    return saturate(max(abs(xcov_0 * xwgt_0 + ycov_0 * ywgt_0) / max(xwgt_0 + ywgt_0, 0.0000152587890625f), min(abs(xcov_0), abs(ycov_0))));
}

fn SlugRender_0( renderCoord_0 : vec2<f32>,  banding_0 : vec4<f32>,  glyphData_0 : vec4<i32>) -> f32
{
    var ycov_1 : f32;
    var ywgt_1 : f32;
    var ycov_2 : f32;
    var ywgt_2 : f32;
    var _S24 : vec2<f32> = vec2<f32>(1.0f) / (abs(dpdx(renderCoord_0)) + abs(dpdy(renderCoord_0)));
    var glyphLoc_1 : vec2<i32> = glyphData_0.xy;
    var bandMax_0 : vec2<i32> = glyphData_0.zw;
    bandMax_0[i32(1)] = ((bandMax_0[i32(1)]) & (i32(255)));
    var bandIndex_0 : vec2<i32> = clamp(vec2<i32>(renderCoord_0 * banding_0.xy + banding_0.zw), vec2<i32>(i32(0), i32(0)), bandMax_0);
    var _S25 : i32 = glyphLoc_1.x;
    var _S26 : i32 = glyphLoc_1.y;
    var _S27 : vec2<i32> = BandLoad_0(vec2<i32>(_S25 + bandIndex_0.y, _S26));
    var _S28 : i32 = _S27.x;
    var _S29 : i32 = bandIndex_0.x;
    var _S30 : u32 = u32(_S27.y);
    var _S31 : vec4<f32> = vec4<f32>(renderCoord_0, renderCoord_0);
    var _S32 : f32 = _S24.x;
    var _S33 : f32 = _S24.y;
    var xwgt_1 : f32 = 0.0f;
    var ci_0 : i32 = i32(0);
    var xcov_1 : f32 = 0.0f;
    var _S34 : vec2<f32> = vec2<f32>(_S32);
    var _S35 : vec2<f32> = vec2<f32>(_S33);
    for(;;)
    {
        if(ci_0 < _S28)
        {
        }
        else
        {
            break;
        }
        var ref_0 : vec2<i32> = BandLoad_0(CalcBandLoc_0(glyphLoc_1, _S30 + u32(ci_0)));
        var _S36 : i32 = ((ref_0.x) & (i32(4095)));
        var _S37 : i32 = ref_0.y;
        var p12_2 : vec4<f32> = CurveLoad_0(vec2<i32>(_S36, _S37)) - _S31;
        var p3_2 : vec2<f32> = CurveLoad_0(vec2<i32>(_S36 + i32(1), _S37)).xy - renderCoord_0;
        if((max(max(p12_2.x, p12_2.z), p3_2.x) * _S32) < -0.5f)
        {
            break;
        }
        var code_0 : u32 = CalcRootCode_0(p12_2.y, p12_2.w, p3_2.y);
        if(code_0 != u32(0))
        {
            var r_0 : vec2<f32> = SolveHorizPoly_0(p12_2, p3_2) * _S34;
            if(((code_0 & (u32(1)))) != u32(0))
            {
                var _S38 : f32 = r_0.x;
                var xcov_2 : f32 = xcov_1 + saturate(_S38 + 0.5f);
                ywgt_2 = max(xwgt_1, saturate(1.0f - abs(_S38) * 2.0f));
                ycov_2 = xcov_2;
            }
            else
            {
                ywgt_2 = xwgt_1;
                ycov_2 = xcov_1;
            }
            if(code_0 > u32(1))
            {
                var _S39 : f32 = r_0.y;
                var xcov_3 : f32 = ycov_2 - saturate(_S39 + 0.5f);
                ywgt_1 = max(ywgt_2, saturate(1.0f - abs(_S39) * 2.0f));
                ycov_1 = xcov_3;
            }
            else
            {
                ywgt_1 = ywgt_2;
                ycov_1 = ycov_2;
            }
            xwgt_1 = ywgt_1;
            xcov_1 = ycov_1;
        }
        ci_0 = ci_0 + i32(1);
    }
    var _S40 : vec2<i32> = BandLoad_0(vec2<i32>(_S25 + bandMax_0.y + i32(1) + _S29, _S26));
    var _S41 : i32 = _S40.x;
    var _S42 : u32 = u32(_S40.y);
    ywgt_2 = 0.0f;
    var ci2_0 : i32 = i32(0);
    ycov_2 = 0.0f;
    for(;;)
    {
        if(ci2_0 < _S41)
        {
        }
        else
        {
            break;
        }
        var ref_1 : vec2<i32> = BandLoad_0(CalcBandLoc_0(glyphLoc_1, _S42 + u32(ci2_0)));
        var _S43 : i32 = ((ref_1.x) & (i32(4095)));
        var _S44 : i32 = ref_1.y;
        var p12_3 : vec4<f32> = CurveLoad_0(vec2<i32>(_S43, _S44)) - _S31;
        var p3_3 : vec2<f32> = CurveLoad_0(vec2<i32>(_S43 + i32(1), _S44)).xy - renderCoord_0;
        if((max(max(p12_3.y, p12_3.w), p3_3.y) * _S33) < -0.5f)
        {
            break;
        }
        var code_1 : u32 = CalcRootCode_0(p12_3.x, p12_3.z, p3_3.x);
        if(code_1 != u32(0))
        {
            var r_1 : vec2<f32> = SolveVertPoly_0(p12_3, p3_3) * _S35;
            if(((code_1 & (u32(1)))) != u32(0))
            {
                var _S45 : f32 = r_1.x;
                var ycov_3 : f32 = ycov_2 - saturate(_S45 + 0.5f);
                ywgt_1 = max(ywgt_2, saturate(1.0f - abs(_S45) * 2.0f));
                ycov_1 = ycov_3;
            }
            else
            {
                ywgt_1 = ywgt_2;
                ycov_1 = ycov_2;
            }
            var ywgt_3 : f32;
            var ycov_4 : f32;
            if(code_1 > u32(1))
            {
                var _S46 : f32 = r_1.y;
                var ycov_5 : f32 = ycov_1 + saturate(_S46 + 0.5f);
                ywgt_3 = max(ywgt_1, saturate(1.0f - abs(_S46) * 2.0f));
                ycov_4 = ycov_5;
            }
            else
            {
                ywgt_3 = ywgt_1;
                ycov_4 = ycov_1;
            }
            ywgt_2 = ywgt_3;
            ycov_2 = ycov_4;
        }
        ci2_0 = ci2_0 + i32(1);
    }
    return CalcCoverage_0(xcov_1, ycov_2, xwgt_1, ywgt_2);
}

fn sRGBToLinearCh_0( x_0 : f32) -> f32
{
    var _S47 : f32;
    if(x_0 <= 0.04044999927282333f)
    {
        _S47 = x_0 / 12.92000007629394531f;
    }
    else
    {
        _S47 = pow((x_0 + 0.05499999970197678f) / 1.0549999475479126f, 2.40000009536743164f);
    }
    return _S47;
}

fn sRGBToLinear3_0( c_0 : vec3<f32>) -> vec3<f32>
{
    return vec3<f32>(sRGBToLinearCh_0(c_0.x), sRGBToLinearCh_0(c_0.y), sRGBToLinearCh_0(c_0.z));
}

fn sRGBToOkLab_0( c_1 : vec3<f32>) -> vec3<f32>
{
    var lin_0 : vec3<f32> = sRGBToLinear3_0(c_1);
    var _S48 : f32 = lin_0.x;
    var _S49 : f32 = lin_0.y;
    var _S50 : f32 = lin_0.z;
    var l_0 : f32 = 0.41222146153450012f * _S48 + 0.53633254766464233f * _S49 + 0.05144599452614784f * _S50;
    var m_0 : f32 = 0.21190349757671356f * _S48 + 0.68069952726364136f * _S49 + 0.10739696025848389f * _S50;
    var s_0 : f32 = 0.08830246329307556f * _S48 + 0.28171885013580322f * _S49 + 0.6299787163734436f * _S50;
    var l_1 : f32 = f32(sign(l_0)) * pow(abs(l_0), 0.3333333432674408f);
    var m_1 : f32 = f32(sign(m_0)) * pow(abs(m_0), 0.3333333432674408f);
    var s_1 : f32 = f32(sign(s_0)) * pow(abs(s_0), 0.3333333432674408f);
    return vec3<f32>(l_1 * 0.21045425534248352f + m_1 * 0.79361778497695923f + s_1 * -0.00407204683870077f, l_1 * 1.97799849510192871f + m_1 * -2.42859220504760742f + s_1 * 0.45059370994567871f, l_1 * 0.02590403705835342f + m_1 * 0.7827717661857605f + s_1 * -0.80867576599121094f);
}

fn sRGBToOkLch_0( c_2 : vec3<f32>) -> vec3<f32>
{
    var lab_0 : vec3<f32> = sRGBToOkLab_0(c_2);
    var _S51 : f32 = lab_0.y;
    var _S52 : f32 = lab_0.z;
    var C_0 : f32 = sqrt(_S51 * _S51 + _S52 * _S52);
    var h_0 : f32 = atan2(_S52, _S51);
    var h_1 : f32;
    if(h_0 < 0.0f)
    {
        h_1 = h_0 + 6.28318548202514648f;
    }
    else
    {
        h_1 = h_0;
    }
    return vec3<f32>(lab_0.x, C_0, h_1 / 6.28318548202514648f);
}

fn sRGBToHSV_0( c_3 : vec3<f32>) -> vec3<f32>
{
    var r_2 : f32 = c_3.x;
    var g_0 : f32 = c_3.y;
    var b_2 : f32 = c_3.z;
    var g_1 : f32;
    var K_0 : f32;
    var b_3 : f32;
    if(g_0 < b_2)
    {
        g_1 = b_2;
        K_0 = -1.0f;
        b_3 = g_0;
    }
    else
    {
        g_1 = g_0;
        K_0 = 0.0f;
        b_3 = b_2;
    }
    var r_3 : f32;
    if(r_2 < g_1)
    {
        var _S53 : f32 = -0.3333333432674408f - K_0;
        r_3 = g_1;
        g_1 = r_2;
        K_0 = _S53;
    }
    else
    {
        r_3 = r_2;
    }
    var chroma_0 : f32 = r_3 - min(g_1, b_3);
    return vec3<f32>(abs(K_0 + (g_1 - b_3) / (6.0f * chroma_0 + 9.99999968265522539e-21f)), chroma_0 / (r_3 + 9.99999968265522539e-21f), r_3);
}

fn linearToSRGBCh_0( x_1 : f32) -> f32
{
    var _S54 : f32;
    if(x_1 <= 0.00313080009073019f)
    {
        _S54 = 12.92000007629394531f * x_1;
    }
    else
    {
        _S54 = 1.0549999475479126f * pow(x_1, 0.4166666567325592f) - 0.05499999970197678f;
    }
    return _S54;
}

fn linearToSRGB3_0( c_4 : vec3<f32>) -> vec3<f32>
{
    return saturate(vec3<f32>(linearToSRGBCh_0(c_4.x), linearToSRGBCh_0(c_4.y), linearToSRGBCh_0(c_4.z)));
}

fn okLabToSRGB_0( Lab_0 : vec3<f32>) -> vec3<f32>
{
    var _S55 : f32 = Lab_0.x;
    var _S56 : f32 = Lab_0.y;
    var _S57 : f32 = Lab_0.z;
    var l_2 : f32 = _S55 + _S56 * 0.39633777737617493f + _S57 * 0.21580375730991364f;
    var m_2 : f32 = _S55 + _S56 * -0.10556134581565857f + _S57 * -0.06385417282581329f;
    var s_2 : f32 = _S55 + _S56 * -0.08948417752981186f + _S57 * -1.29148554801940918f;
    var l_3 : f32 = l_2 * l_2 * l_2;
    var m_3 : f32 = m_2 * m_2 * m_2;
    var s_3 : f32 = s_2 * s_2 * s_2;
    return linearToSRGB3_0(vec3<f32>(l_3 * 4.07674169540405273f + m_3 * -3.30771160125732422f + s_3 * 0.23096993565559387f, l_3 * -1.26843798160552979f + m_3 * 2.60975742340087891f + s_3 * -0.34131938219070435f, l_3 * -0.0041960864327848f + m_3 * -0.70341861248016357f + s_3 * 1.70761466026306152f));
}

fn okLchToSRGB_0( lch_0 : vec3<f32>) -> vec3<f32>
{
    var _S58 : f32 = lch_0.y;
    var _S59 : f32 = lch_0.z * 6.28318548202514648f;
    return okLabToSRGB_0(vec3<f32>(lch_0.x, _S58 * cos(_S59), _S58 * sin(_S59)));
}

fn hsvToSRGB_0( c_5 : vec3<f32>) -> vec3<f32>
{
    var h_2 : f32 = c_5.x;
    var s_4 : f32 = c_5.y;
    var v_0 : f32 = c_5.z;
    if(s_4 < 9.99999997475242708e-07f)
    {
        return vec3<f32>(v_0, v_0, v_0);
    }
    var h_3 : f32 = ((((h_2)) % ((1.0f))));
    var h_4 : f32;
    if(h_3 < 0.0f)
    {
        h_4 = h_3 + 1.0f;
    }
    else
    {
        h_4 = h_3;
    }
    var h_5 : f32 = h_4 * 6.0f;
    var i_0 : i32 = i32(floor(h_5));
    var f_0 : f32 = h_5 - f32(i_0);
    var p_0 : f32 = v_0 * (1.0f - s_4);
    var q_0 : f32 = v_0 * (1.0f - s_4 * f_0);
    var t_0 : f32 = v_0 * (1.0f - s_4 * (1.0f - f_0));
    if(i_0 == i32(0))
    {
        return vec3<f32>(v_0, t_0, p_0);
    }
    if(i_0 == i32(1))
    {
        return vec3<f32>(q_0, v_0, p_0);
    }
    if(i_0 == i32(2))
    {
        return vec3<f32>(p_0, v_0, t_0);
    }
    if(i_0 == i32(3))
    {
        return vec3<f32>(p_0, q_0, v_0);
    }
    if(i_0 == i32(4))
    {
        return vec3<f32>(t_0, p_0, v_0);
    }
    return vec3<f32>(v_0, p_0, q_0);
}

fn lerpInColorSpace_0( c0_0 : vec4<f32>,  c1_0 : vec4<f32>,  ft_0 : f32,  space_0 : f32) -> vec4<f32>
{
    var a_2 : vec3<f32> = c0_0.xyz;
    var b_4 : vec3<f32> = c1_0.xyz;
    var _S60 : bool = space_0 < 0.5f;
    var a_3 : vec3<f32>;
    var b_5 : vec3<f32>;
    if(_S60)
    {
        a_3 = a_2;
        b_5 = b_4;
    }
    else
    {
        if(space_0 < 1.5f)
        {
            var _S61 : vec3<f32> = sRGBToLinear3_0(b_4);
            a_3 = sRGBToLinear3_0(a_2);
            b_5 = _S61;
        }
        else
        {
            if(space_0 < 2.5f)
            {
                var _S62 : vec3<f32> = sRGBToOkLab_0(b_4);
                a_3 = sRGBToOkLab_0(a_2);
                b_5 = _S62;
            }
            else
            {
                if(space_0 < 3.5f)
                {
                    var _S63 : vec3<f32> = sRGBToOkLch_0(b_4);
                    a_3 = sRGBToOkLch_0(a_2);
                    b_5 = _S63;
                }
                else
                {
                    var _S64 : vec3<f32> = sRGBToHSV_0(b_4);
                    a_3 = sRGBToHSV_0(a_2);
                    b_5 = _S64;
                }
            }
        }
    }
    var r_4 : vec3<f32> = mix(a_3, b_5, vec3<f32>(ft_0));
    var r_5 : vec3<f32>;
    if(_S60)
    {
        r_5 = r_4;
    }
    else
    {
        if(space_0 < 1.5f)
        {
            r_5 = linearToSRGB3_0(r_4);
        }
        else
        {
            if(space_0 < 2.5f)
            {
                r_5 = okLabToSRGB_0(r_4);
            }
            else
            {
                if(space_0 < 3.5f)
                {
                    r_5 = okLchToSRGB_0(r_4);
                }
                else
                {
                    r_5 = hsvToSRGB_0(r_4);
                }
            }
        }
    }
    return vec4<f32>(saturate(r_5), mix(c0_0.w, c1_0.w, ft_0));
}

struct pixelOutput_0
{
    @location(0) output_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) color_0 : vec4<f32>,
    @location(3) texcoord_0 : vec2<f32>,
    @interpolate(flat) @location(1) banding_1 : vec4<f32>,
    @interpolate(flat) @location(2) glyph_0 : vec4<i32>,
};

@fragment
fn main_ps( _S65 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> pixelOutput_0
{
    var coverage_0 : f32 = SlugRender_0(_S65.texcoord_0, _S65.banding_1, _S65.glyph_0);
    var _S66 : vec2<f32> = _S65.color_0.xy;
    var uv_0 : vec2<f32> = (position_0.xy - _S66) / max(_S65.color_0.zw - _S66, vec2<f32>(1.0f, 1.0f));
    if((fillParams_0.fillGrad_0.x) < 2.5f)
    {
        var ft_1 : f32;
        if((fillParams_0.fillGrad_0.x) < 0.5f)
        {
            var dir_0 : vec2<f32> = fillParams_0.fillUVEnd_0.xy - fillParams_0.fillUVStart_0.xy;
            var denom_0 : f32 = dot(dir_0, dir_0);
            if(denom_0 > 9.99999993922529029e-09f)
            {
                ft_1 = dot(uv_0 - fillParams_0.fillUVStart_0.xy, dir_0) / denom_0;
            }
            else
            {
                ft_1 = 0.0f;
            }
        }
        else
        {
            if((fillParams_0.fillGrad_0.x) < 1.5f)
            {
                ft_1 = length(uv_0 - fillParams_0.fillUVStart_0.xy) / max(length(fillParams_0.fillUVEnd_0.xy - fillParams_0.fillUVStart_0.xy), 0.00000999999974738f);
            }
            else
            {
                var d_2 : vec2<f32> = (uv_0 - fillParams_0.fillUVStart_0.xy) / vec2<f32>(max(length(fillParams_0.fillUVEnd_0.xy - fillParams_0.fillUVStart_0.xy), 0.00000999999974738f));
                ft_1 = abs(d_2.x) + abs(d_2.y);
            }
        }
        var fillCol_0 : vec4<f32> = lerpInColorSpace_0(fillParams_0.fillColor0_0, fillParams_0.fillColor1_0, saturate(ft_1), fillParams_0.fillGrad_0.y);
        var _S67 : pixelOutput_0 = pixelOutput_0( vec4<f32>(fillCol_0.xyz, fillCol_0.w * coverage_0) );
        return _S67;
    }
    else
    {
        var texColor_0 : vec4<f32> = (textureSample((fillTexture_0), (fillSampler_0), (uv_0 * fillParams_0.fillUVEnd_0.xy + fillParams_0.fillUVStart_0.xy))) * fillParams_0.fillColor0_0;
        var _S68 : pixelOutput_0 = pixelOutput_0( vec4<f32>(texColor_0.xyz, texColor_0.w * coverage_0) );
        return _S68;
    }
}

