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

struct pixelOutput_0
{
    @location(0) output_0 : vec4<f32>,
};

struct pixelInput_0
{
    @location(0) texcoord_0 : vec2<f32>,
    @interpolate(flat) @location(1) banding_1 : vec4<f32>,
    @interpolate(flat) @location(2) glyph_0 : vec4<i32>,
    @interpolate(flat) @location(4) gradColor0_0 : vec4<f32>,
    @interpolate(flat) @location(5) gradColor1_0 : vec4<f32>,
    @interpolate(flat) @location(3) gradParams_0 : vec4<f32>,
};

@fragment
fn main_ps( _S47 : pixelInput_0, @builtin(position) position_0 : vec4<f32>) -> pixelOutput_0
{
    var coverage_0 : f32 = SlugRender_0(_S47.texcoord_0, _S47.banding_1, _S47.glyph_0);
    var t_0 : f32;
    if((((_S47.glyph_0.w) & (i32(256)))) != i32(0))
    {
        t_0 = length(_S47.texcoord_0 - _S47.gradParams_0.xy) * _S47.gradParams_0.z + _S47.gradParams_0.w;
    }
    else
    {
        t_0 = dot(_S47.texcoord_0, _S47.gradParams_0.xy) * _S47.gradParams_0.z + _S47.gradParams_0.w;
    }
    var gradColor_0 : vec4<f32> = mix(_S47.gradColor0_0, _S47.gradColor1_0, vec4<f32>(saturate(t_0)));
    var _S48 : pixelOutput_0 = pixelOutput_0( vec4<f32>(gradColor_0.xyz, gradColor_0.w * coverage_0) );
    return _S48;
}

