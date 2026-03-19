@binding(1) @group(0) var bandTexture_0 : texture_2d<f32>;

@binding(0) @group(0) var curveTexture_0 : texture_2d<f32>;

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

fn SolveCubicRoots_0( a_1 : f32,  b_1 : f32,  c_0 : f32,  d_1 : f32) -> vec3<f32>
{
    if((abs(a_1)) < 0.0000152587890625f)
    {
        if((abs(b_1)) < 0.0000152587890625f)
        {
            if((abs(c_0)) < 0.0000152587890625f)
            {
                return vec3<f32>(1.0e+09f, 1.0e+09f, 1.0e+09f);
            }
            return vec3<f32>(- d_1 / c_0, 1.0e+09f, 1.0e+09f);
        }
        var disc2_0 : f32 = c_0 * c_0 - 4.0f * b_1 * d_1;
        if(disc2_0 < 0.0f)
        {
            return vec3<f32>(1.0e+09f, 1.0e+09f, 1.0e+09f);
        }
        var sq_0 : f32 = sqrt(disc2_0);
        var inv2b_0 : f32 = 0.5f / b_1;
        var _S14 : f32 = - c_0;
        return vec3<f32>((_S14 - sq_0) * inv2b_0, (_S14 + sq_0) * inv2b_0, 1.0e+09f);
    }
    var inv_a_0 : f32 = 1.0f / a_1;
    var B_0 : f32 = b_1 * inv_a_0;
    var C_0 : f32 = c_0 * inv_a_0;
    var shift_0 : f32 = - B_0 / 3.0f;
    var p_0 : f32 = C_0 - B_0 * B_0 / 3.0f;
    var q_0 : f32 = d_1 * inv_a_0 + B_0 * (2.0f * B_0 * B_0 - 9.0f * C_0) / 27.0f;
    var disc_0 : f32 = - (4.0f * p_0 * p_0 * p_0 + 27.0f * q_0 * q_0);
    if(disc_0 >= 0.0f)
    {
        var m_0 : f32 = 2.0f * sqrt(max(- p_0 / 3.0f, 0.0f));
        var arg_0 : f32;
        if(m_0 > 1.00000001168609742e-07f)
        {
            arg_0 = clamp(3.0f * q_0 / (p_0 * m_0), -1.0f, 1.0f);
        }
        else
        {
            arg_0 = 0.0f;
        }
        var phi_0 : f32 = acos(arg_0) / 3.0f;
        return vec3<f32>(m_0 * cos(phi_0) + shift_0, m_0 * cos(phi_0 - 2.09439516067504883f) + shift_0, m_0 * cos(phi_0 - 4.18879032135009766f) + shift_0);
    }
    else
    {
        var sq_1 : f32 = sqrt(max(- disc_0 / 108.0f, 0.0f));
        var hq_0 : f32 = - q_0 * 0.5f;
        var _S15 : f32 = hq_0 + sq_1;
        var _S16 : f32 = hq_0 - sq_1;
        return vec3<f32>(f32(sign(_S15)) * pow(abs(_S15), 0.3333333432674408f) + f32(sign(_S16)) * pow(abs(_S16), 0.3333333432674408f) + shift_0, 1.0e+09f, 1.0e+09f);
    }
}

fn ApplyCubicRoot_0( t_0 : f32,  x_em_0 : f32,  pxPerEm_0 : f32,  deriv_0 : f32,  sign_pos_0 : f32,  cov_0 : ptr<function, f32>,  wgt_0 : ptr<function, f32>)
{
    var _S17 : bool;
    if(t_0 < 0.0f)
    {
        _S17 = true;
    }
    else
    {
        _S17 = t_0 >= 1.0f;
    }
    if(_S17)
    {
        return;
    }
    var r_0 : f32 = x_em_0 * pxPerEm_0;
    var contrib_0 : f32 = saturate(r_0 + 0.5f);
    var w_0 : f32 = saturate(1.0f - abs(r_0) * 2.0f);
    if(deriv_0 > 0.0f)
    {
        (*cov_0) = (*cov_0) + sign_pos_0 * contrib_0;
        (*wgt_0) = max((*wgt_0), w_0);
    }
    else
    {
        if(deriv_0 < 0.0f)
        {
            (*cov_0) = (*cov_0) - sign_pos_0 * contrib_0;
            (*wgt_0) = max((*wgt_0), w_0);
        }
    }
    return;
}

fn SolveVertPoly_0( p12_1 : vec4<f32>,  p3_1 : vec2<f32>) -> vec2<f32>
{
    var _S18 : vec2<f32> = p12_1.xy;
    var _S19 : vec2<f32> = p12_1.zw;
    var a_2 : vec2<f32> = _S18 - _S19 * vec2<f32>(2.0f) + p3_1;
    var b_2 : vec2<f32> = _S18 - _S19;
    var _S20 : f32 = a_2.x;
    var ra_1 : f32 = 1.0f / _S20;
    var _S21 : f32 = b_2.x;
    var rb_1 : f32 = 0.5f / _S21;
    var _S22 : f32 = p12_1.x;
    var d_2 : f32 = sqrt(max(_S21 * _S21 - _S20 * _S22, 0.0f));
    var _S23 : f32 = (_S21 - d_2) * ra_1;
    var _S24 : f32 = (_S21 + d_2) * ra_1;
    var t1_1 : f32;
    var t2_2 : f32;
    if((abs(_S20)) < 0.0000152587890625f)
    {
        var t2_3 : f32 = _S22 * rb_1;
        t1_1 = t2_3;
        t2_2 = t2_3;
    }
    else
    {
        t1_1 = _S23;
        t2_2 = _S24;
    }
    var _S25 : f32 = a_2.y;
    var _S26 : f32 = b_2.y * 2.0f;
    var _S27 : f32 = p12_1.y;
    return vec2<f32>((_S25 * t1_1 - _S26) * t1_1 + _S27, (_S25 * t2_2 - _S26) * t2_2 + _S27);
}

fn CalcCoverage_0( xcov_0 : f32,  ycov_0 : f32,  xwgt_0 : f32,  ywgt_0 : f32,  flags_0 : i32) -> f32
{
    return saturate(max(abs(xcov_0 * xwgt_0 + ycov_0 * ywgt_0) / max(xwgt_0 + ywgt_0, 0.0000152587890625f), min(abs(xcov_0), abs(ycov_0))));
}

fn SlugRender_0( renderCoord_0 : vec2<f32>,  banding_0 : vec4<f32>,  glyphData_0 : vec4<i32>) -> f32
{
    var maxY_0 : f32;
    var _S28 : vec2<f32> = vec2<f32>(1.0f) / (abs(dpdx(renderCoord_0)) + abs(dpdy(renderCoord_0)));
    var glyphLoc_1 : vec2<i32> = glyphData_0.xy;
    var bandMax_0 : vec2<i32> = glyphData_0.zw;
    bandMax_0[i32(1)] = ((bandMax_0[i32(1)]) & (i32(255)));
    var bandIndex_0 : vec2<i32> = clamp(vec2<i32>(renderCoord_0 * banding_0.xy + banding_0.zw), vec2<i32>(i32(0), i32(0)), bandMax_0);
    var xcov_1 : f32 = 0.0f;
    var xwgt_1 : f32 = 0.0f;
    var _S29 : i32 = glyphLoc_1.x;
    var _S30 : i32 = glyphLoc_1.y;
    var _S31 : vec2<i32> = BandLoad_0(vec2<i32>(_S29 + bandIndex_0.y, _S30));
    var ci_0 : i32 = i32(0);
    for(;;)
    {
        if(ci_0 < (_S31.x))
        {
        }
        else
        {
            break;
        }
        var ref_0 : vec2<i32> = BandLoad_0(CalcBandLoc_0(glyphLoc_1, u32(_S31.y) + u32(ci_0)));
        var _S32 : i32 = ref_0.x;
        var isCubic_0 : bool = ((_S32 & (i32(4096)))) != i32(0);
        var _S33 : i32 = (_S32 & (i32(4095)));
        var _S34 : i32 = ref_0.y;
        var p12_2 : vec4<f32> = CurveLoad_0(vec2<i32>(_S33, _S34)) - vec4<f32>(renderCoord_0, renderCoord_0);
        var texel1_0 : vec4<f32> = CurveLoad_0(vec2<i32>(_S33 + i32(1), _S34));
        var p3_2 : vec2<f32> = texel1_0.xy - renderCoord_0;
        var p4_0 : vec2<f32> = texel1_0.zw - renderCoord_0;
        if(isCubic_0)
        {
            maxY_0 = max(max(max(p12_2.x, p12_2.z), p3_2.x), p4_0.x);
        }
        else
        {
            maxY_0 = max(max(p12_2.x, p12_2.z), p3_2.x);
        }
        var _S35 : f32 = _S28.x;
        if((maxY_0 * _S35) < -0.5f)
        {
            break;
        }
        if(!isCubic_0)
        {
            var code_0 : u32 = CalcRootCode_0(p12_2.y, p12_2.w, p3_2.y);
            if(code_0 != u32(0))
            {
                var r_1 : vec2<f32> = SolveHorizPoly_0(p12_2, p3_2) * vec2<f32>(_S35);
                if(((code_0 & (u32(1)))) != u32(0))
                {
                    var _S36 : f32 = r_1.x;
                    xcov_1 = xcov_1 + saturate(_S36 + 0.5f);
                    xwgt_1 = max(xwgt_1, saturate(1.0f - abs(_S36) * 2.0f));
                }
                if(code_0 > u32(1))
                {
                    var _S37 : f32 = r_1.y;
                    xcov_1 = xcov_1 - saturate(_S37 + 0.5f);
                    xwgt_1 = max(xwgt_1, saturate(1.0f - abs(_S37) * 2.0f));
                }
            }
        }
        else
        {
            var _S38 : f32 = p12_2.y;
            var _S39 : f32 = p12_2.w;
            var _S40 : f32 = 3.0f * _S39;
            var _S41 : f32 = 3.0f * p3_2.y;
            var ay_0 : f32 = - _S38 + _S40 - _S41 + p4_0.y;
            var by_0 : f32 = 3.0f * _S38 - 6.0f * _S39 + _S41;
            var cy_0 : f32 = -3.0f * _S38 + _S40;
            var _S42 : f32 = p12_2.x;
            var _S43 : f32 = p12_2.z;
            var _S44 : f32 = 3.0f * _S43;
            var _S45 : f32 = 3.0f * p3_2.x;
            var ax_0 : f32 = - _S42 + _S44 - _S45 + p4_0.x;
            var bx_0 : f32 = 3.0f * _S42 - 6.0f * _S43 + _S45;
            var cx_0 : f32 = -3.0f * _S42 + _S44;
            var ts_0 : vec3<f32> = SolveCubicRoots_0(ay_0, by_0, cy_0, _S38);
            var t0_0 : f32 = ts_0.x;
            var _S46 : f32 = 3.0f * ay_0;
            var _S47 : f32 = 2.0f * by_0;
            ApplyCubicRoot_0(t0_0, ((ax_0 * t0_0 + bx_0) * t0_0 + cx_0) * t0_0 + _S42, _S35, (_S46 * t0_0 + _S47) * t0_0 + cy_0, 1.0f, &(xcov_1), &(xwgt_1));
            var t1_2 : f32 = ts_0.y;
            ApplyCubicRoot_0(t1_2, ((ax_0 * t1_2 + bx_0) * t1_2 + cx_0) * t1_2 + _S42, _S35, (_S46 * t1_2 + _S47) * t1_2 + cy_0, 1.0f, &(xcov_1), &(xwgt_1));
            var t2_4 : f32 = ts_0.z;
            ApplyCubicRoot_0(t2_4, ((ax_0 * t2_4 + bx_0) * t2_4 + cx_0) * t2_4 + _S42, _S35, (_S46 * t2_4 + _S47) * t2_4 + cy_0, 1.0f, &(xcov_1), &(xwgt_1));
        }
        ci_0 = ci_0 + i32(1);
    }
    var ycov_1 : f32 = 0.0f;
    var ywgt_1 : f32 = 0.0f;
    var _S48 : vec2<i32> = BandLoad_0(vec2<i32>(_S29 + bandMax_0.y + i32(1) + bandIndex_0.x, _S30));
    var ci2_0 : i32 = i32(0);
    for(;;)
    {
        if(ci2_0 < (_S48.x))
        {
        }
        else
        {
            break;
        }
        var ref_1 : vec2<i32> = BandLoad_0(CalcBandLoc_0(glyphLoc_1, u32(_S48.y) + u32(ci2_0)));
        var _S49 : i32 = ref_1.x;
        var isCubic_1 : bool = ((_S49 & (i32(4096)))) != i32(0);
        var _S50 : i32 = (_S49 & (i32(4095)));
        var _S51 : i32 = ref_1.y;
        var p12_3 : vec4<f32> = CurveLoad_0(vec2<i32>(_S50, _S51)) - vec4<f32>(renderCoord_0, renderCoord_0);
        var texel1_1 : vec4<f32> = CurveLoad_0(vec2<i32>(_S50 + i32(1), _S51));
        var p3_3 : vec2<f32> = texel1_1.xy - renderCoord_0;
        var p4_1 : vec2<f32> = texel1_1.zw - renderCoord_0;
        if(isCubic_1)
        {
            maxY_0 = max(max(max(p12_3.y, p12_3.w), p3_3.y), p4_1.y);
        }
        else
        {
            maxY_0 = max(max(p12_3.y, p12_3.w), p3_3.y);
        }
        var _S52 : f32 = _S28.y;
        if((maxY_0 * _S52) < -0.5f)
        {
            break;
        }
        if(!isCubic_1)
        {
            var code_1 : u32 = CalcRootCode_0(p12_3.x, p12_3.z, p3_3.x);
            if(code_1 != u32(0))
            {
                var r_2 : vec2<f32> = SolveVertPoly_0(p12_3, p3_3) * vec2<f32>(_S52);
                if(((code_1 & (u32(1)))) != u32(0))
                {
                    var _S53 : f32 = r_2.x;
                    ycov_1 = ycov_1 - saturate(_S53 + 0.5f);
                    ywgt_1 = max(ywgt_1, saturate(1.0f - abs(_S53) * 2.0f));
                }
                if(code_1 > u32(1))
                {
                    var _S54 : f32 = r_2.y;
                    ycov_1 = ycov_1 + saturate(_S54 + 0.5f);
                    ywgt_1 = max(ywgt_1, saturate(1.0f - abs(_S54) * 2.0f));
                }
            }
        }
        else
        {
            var _S55 : f32 = p12_3.x;
            var _S56 : f32 = p12_3.z;
            var _S57 : f32 = 3.0f * _S56;
            var _S58 : f32 = 3.0f * p3_3.x;
            var ax_1 : f32 = - _S55 + _S57 - _S58 + p4_1.x;
            var bx_1 : f32 = 3.0f * _S55 - 6.0f * _S56 + _S58;
            var cx_1 : f32 = -3.0f * _S55 + _S57;
            var _S59 : f32 = p12_3.y;
            var _S60 : f32 = p12_3.w;
            var _S61 : f32 = 3.0f * _S60;
            var _S62 : f32 = 3.0f * p3_3.y;
            var ay_1 : f32 = - _S59 + _S61 - _S62 + p4_1.y;
            var by_1 : f32 = 3.0f * _S59 - 6.0f * _S60 + _S62;
            var cy_1 : f32 = -3.0f * _S59 + _S61;
            var ts_1 : vec3<f32> = SolveCubicRoots_0(ax_1, bx_1, cx_1, _S55);
            var t0_1 : f32 = ts_1.x;
            var _S63 : f32 = 3.0f * ax_1;
            var _S64 : f32 = 2.0f * bx_1;
            ApplyCubicRoot_0(t0_1, ((ay_1 * t0_1 + by_1) * t0_1 + cy_1) * t0_1 + _S59, _S52, (_S63 * t0_1 + _S64) * t0_1 + cx_1, -1.0f, &(ycov_1), &(ywgt_1));
            var t1_3 : f32 = ts_1.y;
            ApplyCubicRoot_0(t1_3, ((ay_1 * t1_3 + by_1) * t1_3 + cy_1) * t1_3 + _S59, _S52, (_S63 * t1_3 + _S64) * t1_3 + cx_1, -1.0f, &(ycov_1), &(ywgt_1));
            var t2_5 : f32 = ts_1.z;
            ApplyCubicRoot_0(t2_5, ((ay_1 * t2_5 + by_1) * t2_5 + cy_1) * t2_5 + _S59, _S52, (_S63 * t2_5 + _S64) * t2_5 + cx_1, -1.0f, &(ycov_1), &(ywgt_1));
        }
        ci2_0 = ci2_0 + i32(1);
    }
    return CalcCoverage_0(xcov_1, ycov_1, xwgt_1, ywgt_1, glyphData_0.w);
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
    var _S66 : pixelOutput_0 = pixelOutput_0( _S65.color_0 * vec4<f32>(SlugRender_0(_S65.texcoord_0, _S65.banding_1, _S65.glyph_0)) );
    return _S66;
}

