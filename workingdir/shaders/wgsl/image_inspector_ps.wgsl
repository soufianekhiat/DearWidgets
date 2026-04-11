struct SLANG_ParameterGroup_ImageInspectorParams_std140_0
{
    @align(16) imgSize_0 : vec4<f32>,
    @align(16) panZoom_0 : vec4<f32>,
    @align(16) viewportPx_0 : vec4<f32>,
    @align(16) packedTexDims_0 : vec4<u32>,
    @align(16) layoutPack_0 : vec4<u32>,
    @align(16) formatPack_0 : vec4<u32>,
    @align(16) exposureParams_0 : vec4<f32>,
    @align(16) tempTint_0 : vec4<f32>,
    @align(16) inGamut_r0_0 : vec4<f32>,
    @align(16) inGamut_r1_0 : vec4<f32>,
    @align(16) inGamut_r2_0 : vec4<f32>,
    @align(16) outGamut_r0_0 : vec4<f32>,
    @align(16) outGamut_r1_0 : vec4<f32>,
    @align(16) outGamut_r2_0 : vec4<f32>,
    @align(16) pipelinePack_0 : vec4<u32>,
    @align(16) channelMask_0 : vec4<f32>,
    @align(16) nanColor_0 : vec4<f32>,
    @align(16) modePack_0 : vec4<u32>,
};

@binding(1) @group(0) var<uniform> ImageInspectorParams_0 : SLANG_ParameterGroup_ImageInspectorParams_std140_0;
@binding(3) @group(0) var texture0_0 : texture_2d<f32>;

fn LoadWord_0( byte_off_0 : u32) -> u32
{
    var word_idx_0 : u32 = (byte_off_0 >> (u32(2)));
    var lane_0 : u32 = (word_idx_0 & (u32(3)));
    var texel_0 : u32 = (word_idx_0 >> (u32(2)));
    var tw_0 : u32 = ImageInspectorParams_0.packedTexDims_0.x;
    var tx_0 : i32 = i32((texel_0 & ((tw_0 - u32(1)))));
    var _S1 : u32 = texel_0 / tw_0;
    var _S2 : vec3<i32> = vec3<i32>(tx_0, i32(_S1), i32(0));
    return (bitcast<vec4<u32>>(((textureLoad((texture0_0), ((_S2)).xy, ((_S2)).z)))))[lane_0];
}

fn SignExtend8_0( v_0 : u32) -> i32
{
    return (((i32(v_0) << (u32(24)))) >> (u32(24)));
}

fn SignExtend16_0( v_1 : u32) -> i32
{
    return (((i32(v_1) << (u32(16)))) >> (u32(16)));
}

fn FetchSample_0( sx_0 : i32,  sy_0 : i32,  c_0 : i32) -> f32
{
    var sample_type_0 : u32 = ImageInspectorParams_0.formatPack_0.x;
    var off_0 : u32 = u32(sy_0) * ImageInspectorParams_0.layoutPack_0.y + u32(sx_0) * ImageInspectorParams_0.layoutPack_0.x + u32(c_0) * ImageInspectorParams_0.layoutPack_0.z;
    var w0_0 : u32 = LoadWord_0(off_0);
    var result_0 : f32;
    switch(sample_type_0)
    {
    case u32(0), :
        {
            result_0 = f32((((w0_0 >> ((((off_0 & (u32(3)))) * u32(8))))) & (u32(255)))) / 255.0f;
            break;
        }
    case u32(1), :
        {
            result_0 = f32(SignExtend8_0((((w0_0 >> ((((off_0 & (u32(3)))) * u32(8))))) & (u32(255))))) / 127.0f;
            break;
        }
    case u32(2), :
        {
            result_0 = f32((((w0_0 >> ((((off_0 & (u32(3)))) * u32(8))))) & (u32(65535)))) / 65535.0f;
            break;
        }
    case u32(3), :
        {
            result_0 = f32(SignExtend16_0((((w0_0 >> ((((off_0 & (u32(3)))) * u32(8))))) & (u32(65535))))) / 32767.0f;
            break;
        }
    case u32(4), :
        {
            result_0 = f32(w0_0) / 4.294967296e+09f;
            break;
        }
    case u32(5), :
        {
            result_0 = f32((bitcast<i32>((w0_0)))) / 2.147483648e+09f;
            break;
        }
    case u32(8), :
        {
            result_0 = (unpack2x16float(((((w0_0 >> ((((off_0 & (u32(2)))) * u32(8))))) & (u32(65535))))).x);
            break;
        }
    case u32(9), :
        {
            result_0 = (bitcast<f32>((w0_0)));
            break;
        }
    case u32(6), :
        {
            var hi_0 : u32 = LoadWord_0(off_0 + u32(4));
            result_0 = f32(w0_0) + f32(hi_0) * 4.294967296e+09f;
            break;
        }
    case u32(7), :
        {
            var hi_1 : u32 = LoadWord_0(off_0 + u32(4));
            if(((hi_1 & (u32(2147483648)))) != u32(0))
            {
                var nlo_0 : u32 = ~w0_0 + u32(1);
                var _S3 : u32 = ~hi_1;
                var _S4 : u32;
                if(nlo_0 == u32(0))
                {
                    _S4 = u32(1);
                }
                else
                {
                    _S4 = u32(0);
                }
                result_0 = - (f32(nlo_0) + f32(_S3 + _S4) * 4.294967296e+09f);
            }
            else
            {
                result_0 = f32(w0_0) + f32(hi_1) * 4.294967296e+09f;
            }
            break;
        }
    case u32(10), :
        {
            var hi_2 : u32 = LoadWord_0(off_0 + u32(4));
            var sign_0 : u32 = (hi_2 & (u32(2147483648)));
            var mant32_0 : u32 = (((((hi_2 & (u32(1048575)))) << (u32(3)))) | (((w0_0 >> (u32(29))))));
            var exp32_0 : i32 = i32((((hi_2 >> (u32(20)))) & (u32(2047)))) - i32(1023) + i32(127);
            if(exp32_0 <= i32(0))
            {
                result_0 = (bitcast<f32>((sign_0)));
            }
            else
            {
                if(exp32_0 >= i32(255))
                {
                    result_0 = (bitcast<f32>(((sign_0 | (u32(2139095040))))));
                }
                else
                {
                    result_0 = (bitcast<f32>(((((sign_0 | (((u32(exp32_0) << (u32(23))))))) | (mant32_0)))));
                }
            }
            break;
        }
    case default, :
        {
            result_0 = 0.0f;
            break;
        }
    }
    return result_0;
}

fn FetchPixel_0( sx_1 : i32,  sy_1 : i32) -> vec4<f32>
{
    var channels_0 : i32 = i32(ImageInspectorParams_0.formatPack_0.y);
    var _S5 : i32 = clamp(sx_1, i32(0), i32(ImageInspectorParams_0.imgSize_0.x) - i32(1));
    var _S6 : i32 = clamp(sy_1, i32(0), i32(ImageInspectorParams_0.imgSize_0.y) - i32(1));
    var v_2 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    if(channels_0 == i32(1))
    {
        var y_0 : f32 = FetchSample_0(_S5, _S6, i32(0));
        v_2 = vec4<f32>(y_0, y_0, y_0, 1.0f);
    }
    else
    {
        if(channels_0 == i32(2))
        {
            var _S7 : f32 = FetchSample_0(_S5, _S6, i32(0));
            v_2[i32(0)] = _S7;
            var _S8 : f32 = FetchSample_0(_S5, _S6, i32(1));
            v_2[i32(1)] = _S8;
        }
        else
        {
            if(channels_0 == i32(3))
            {
                var _S9 : f32 = FetchSample_0(_S5, _S6, i32(0));
                v_2[i32(0)] = _S9;
                var _S10 : f32 = FetchSample_0(_S5, _S6, i32(1));
                v_2[i32(1)] = _S10;
                var _S11 : f32 = FetchSample_0(_S5, _S6, i32(2));
                v_2[i32(2)] = _S11;
            }
            else
            {
                var _S12 : f32 = FetchSample_0(_S5, _S6, i32(0));
                v_2[i32(0)] = _S12;
                var _S13 : f32 = FetchSample_0(_S5, _S6, i32(1));
                v_2[i32(1)] = _S13;
                var _S14 : f32 = FetchSample_0(_S5, _S6, i32(2));
                v_2[i32(2)] = _S14;
                var _S15 : f32 = FetchSample_0(_S5, _S6, i32(3));
                v_2[i32(3)] = _S15;
            }
        }
    }
    return v_2;
}

fn MosaicSampleXTrans_0( sx_2 : i32,  sy_2 : i32) -> vec4<f32>
{
    if((ImageInspectorParams_0.modePack_0.x) == u32(0))
    {
        var v_3 : f32 = FetchSample_0(sx_2, sy_2, i32(0));
        return vec4<f32>(v_3, v_3, v_3, 1.0f);
    }
    var _S16 : i32 = sx_2 - i32(u32(sx_2) % u32(6));
    var _S17 : i32 = sy_2 - i32(u32(sy_2) % u32(6));
    var j_0 : i32 = i32(0);
    var sum_0 : f32 = 0.0f;
    var n_0 : i32 = i32(0);
    for(;;)
    {
        if(j_0 < i32(6))
        {
        }
        else
        {
            break;
        }
        var _S18 : i32 = _S17 + j_0;
        var i_0 : i32 = i32(0);
        for(;;)
        {
            if(i_0 < i32(6))
            {
            }
            else
            {
                break;
            }
            var _S19 : f32 = FetchSample_0(clamp(_S16 + i_0, i32(0), i32(ImageInspectorParams_0.imgSize_0.x) - i32(1)), clamp(_S18, i32(0), i32(ImageInspectorParams_0.imgSize_0.y) - i32(1)), i32(0));
            var sum_1 : f32 = sum_0 + _S19;
            var n_1 : i32 = n_0 + i32(1);
            i_0 = i_0 + i32(1);
            sum_0 = sum_1;
            n_0 = n_1;
        }
        j_0 = j_0 + i32(1);
    }
    var v_4 : f32;
    if(n_0 > i32(0))
    {
        v_4 = sum_0 / f32(n_0);
    }
    else
    {
        v_4 = 0.0f;
    }
    return vec4<f32>(v_4, v_4, v_4, 1.0f);
}

fn MosaicSampleBayer_0( sx_3 : i32,  sy_3 : i32,  pattern_0 : u32) -> vec4<f32>
{
    var xp_0 : u32 = u32((sx_3 & (i32(1))));
    var yp_0 : u32 = u32((sy_3 & (i32(1))));
    var _S20 : bool = pattern_0 == u32(1);
    var color_0 : i32;
    if(_S20)
    {
        if(yp_0 == u32(0))
        {
            if(xp_0 == u32(0))
            {
                color_0 = i32(0);
            }
            else
            {
                color_0 = i32(1);
            }
        }
        else
        {
            if(xp_0 == u32(0))
            {
                color_0 = i32(1);
            }
            else
            {
                color_0 = i32(2);
            }
        }
    }
    else
    {
        if(pattern_0 == u32(2))
        {
            if(yp_0 == u32(0))
            {
                if(xp_0 == u32(0))
                {
                    color_0 = i32(1);
                }
                else
                {
                    color_0 = i32(0);
                }
            }
            else
            {
                if(xp_0 == u32(0))
                {
                    color_0 = i32(2);
                }
                else
                {
                    color_0 = i32(1);
                }
            }
        }
        else
        {
            if(pattern_0 == u32(3))
            {
                if(yp_0 == u32(0))
                {
                    if(xp_0 == u32(0))
                    {
                        color_0 = i32(1);
                    }
                    else
                    {
                        color_0 = i32(2);
                    }
                }
                else
                {
                    if(xp_0 == u32(0))
                    {
                        color_0 = i32(0);
                    }
                    else
                    {
                        color_0 = i32(1);
                    }
                }
            }
            else
            {
                if(yp_0 == u32(0))
                {
                    if(xp_0 == u32(0))
                    {
                        color_0 = i32(2);
                    }
                    else
                    {
                        color_0 = i32(1);
                    }
                }
                else
                {
                    if(xp_0 == u32(0))
                    {
                        color_0 = i32(1);
                    }
                    else
                    {
                        color_0 = i32(0);
                    }
                }
            }
        }
    }
    if((ImageInspectorParams_0.modePack_0.x) == u32(0))
    {
        var v_5 : f32 = FetchSample_0(sx_3, sy_3, i32(0));
        const _S21 : vec3<f32> = vec3<f32>(1.0f, 1.0f, 1.0f);
        var tint_0 : vec3<f32>;
        if(color_0 == i32(0))
        {
            tint_0 = vec3<f32>(1.10000002384185791f, 0.94999998807907104f, 0.94999998807907104f);
        }
        else
        {
            tint_0 = _S21;
        }
        if(color_0 == i32(1))
        {
            tint_0 = vec3<f32>(0.94999998807907104f, 1.10000002384185791f, 0.94999998807907104f);
        }
        if(color_0 == i32(2))
        {
            tint_0 = vec3<f32>(0.94999998807907104f, 0.94999998807907104f, 1.10000002384185791f);
        }
        return vec4<f32>(vec3<f32>(v_5) * tint_0, 1.0f);
    }
    var _S22 : bool = pattern_0 == u32(2);
    var _S23 : bool = pattern_0 == u32(3);
    var dy_0 : i32 = i32(-1);
    var r_0 : f32 = 0.0f;
    var nr_0 : i32 = i32(0);
    var g_0 : f32 = 0.0f;
    var ng_0 : i32 = i32(0);
    var b_0 : f32 = 0.0f;
    var nb_0 : i32 = i32(0);
    for(;;)
    {
        if(dy_0 <= i32(1))
        {
        }
        else
        {
            break;
        }
        var y2_0 : i32 = sy_3 + dy_0;
        var _S24 : bool = u32((y2_0 & (i32(1)))) == u32(0);
        var dx_0 : i32 = i32(-1);
        var r_1 : f32 = r_0;
        var nr_1 : i32 = nr_0;
        var g_1 : f32 = g_0;
        var ng_1 : i32 = ng_0;
        var b_1 : f32 = b_0;
        var nb_1 : i32 = nb_0;
        for(;;)
        {
            if(dx_0 <= i32(1))
            {
            }
            else
            {
                break;
            }
            var x2_0 : i32 = sx_3 + dx_0;
            var xp2_0 : u32 = u32((x2_0 & (i32(1))));
            var c2_0 : i32;
            if(_S20)
            {
                if(_S24)
                {
                    if(xp2_0 == u32(0))
                    {
                        color_0 = i32(0);
                    }
                    else
                    {
                        color_0 = i32(1);
                    }
                }
                else
                {
                    if(xp2_0 == u32(0))
                    {
                        color_0 = i32(1);
                    }
                    else
                    {
                        color_0 = i32(2);
                    }
                }
                c2_0 = color_0;
            }
            else
            {
                if(_S22)
                {
                    if(_S24)
                    {
                        if(xp2_0 == u32(0))
                        {
                            color_0 = i32(1);
                        }
                        else
                        {
                            color_0 = i32(0);
                        }
                    }
                    else
                    {
                        if(xp2_0 == u32(0))
                        {
                            color_0 = i32(2);
                        }
                        else
                        {
                            color_0 = i32(1);
                        }
                    }
                    c2_0 = color_0;
                }
                else
                {
                    if(_S23)
                    {
                        if(_S24)
                        {
                            if(xp2_0 == u32(0))
                            {
                                color_0 = i32(1);
                            }
                            else
                            {
                                color_0 = i32(2);
                            }
                        }
                        else
                        {
                            if(xp2_0 == u32(0))
                            {
                                color_0 = i32(0);
                            }
                            else
                            {
                                color_0 = i32(1);
                            }
                        }
                        c2_0 = color_0;
                    }
                    else
                    {
                        if(_S24)
                        {
                            if(xp2_0 == u32(0))
                            {
                                color_0 = i32(2);
                            }
                            else
                            {
                                color_0 = i32(1);
                            }
                        }
                        else
                        {
                            if(xp2_0 == u32(0))
                            {
                                color_0 = i32(1);
                            }
                            else
                            {
                                color_0 = i32(0);
                            }
                        }
                        c2_0 = color_0;
                    }
                }
            }
            var v_6 : f32 = FetchSample_0(clamp(x2_0, i32(0), i32(ImageInspectorParams_0.imgSize_0.x) - i32(1)), clamp(y2_0, i32(0), i32(ImageInspectorParams_0.imgSize_0.y) - i32(1)), i32(0));
            if(c2_0 == i32(0))
            {
                var nr_2 : i32 = nr_1 + i32(1);
                r_1 = r_1 + v_6;
                nr_1 = nr_2;
            }
            if(c2_0 == i32(1))
            {
                var ng_2 : i32 = ng_1 + i32(1);
                g_1 = g_1 + v_6;
                ng_1 = ng_2;
            }
            if(c2_0 == i32(2))
            {
                var nb_2 : i32 = nb_1 + i32(1);
                b_1 = b_1 + v_6;
                nb_1 = nb_2;
            }
            dx_0 = dx_0 + i32(1);
        }
        dy_0 = dy_0 + i32(1);
        r_0 = r_1;
        nr_0 = nr_1;
        g_0 = g_1;
        ng_0 = ng_1;
        b_0 = b_1;
        nb_0 = nb_1;
    }
    if(nr_0 > i32(0))
    {
        r_0 = r_0 / f32(nr_0);
    }
    if(ng_0 > i32(0))
    {
        g_0 = g_0 / f32(ng_0);
    }
    if(nb_0 > i32(0))
    {
        b_0 = b_0 / f32(nb_0);
    }
    return vec4<f32>(r_0, g_0, b_0, 1.0f);
}

fn FetchPixelMosaic_0( sx_4 : i32,  sy_4 : i32) -> vec4<f32>
{
    var pattern_1 : u32 = ImageInspectorParams_0.formatPack_0.z;
    if(pattern_1 == u32(0))
    {
        var _S25 : vec4<f32> = FetchPixel_0(sx_4, sy_4);
        return _S25;
    }
    var _S26 : i32 = clamp(sx_4, i32(0), i32(ImageInspectorParams_0.imgSize_0.x) - i32(1));
    var _S27 : i32 = clamp(sy_4, i32(0), i32(ImageInspectorParams_0.imgSize_0.y) - i32(1));
    if(pattern_1 == u32(5))
    {
        var _S28 : vec4<f32> = MosaicSampleXTrans_0(_S26, _S27);
        return _S28;
    }
    var _S29 : vec4<f32> = MosaicSampleBayer_0(_S26, _S27, pattern_1);
    return _S29;
}

fn FilterNearest_0( src_0 : vec2<f32>) -> vec4<f32>
{
    var _S30 : vec4<f32> = FetchPixelMosaic_0(i32(floor(src_0.x)), i32(floor(src_0.y)));
    return _S30;
}

fn FilterBilinear_0( src_1 : vec2<f32>) -> vec4<f32>
{
    var fx_0 : f32 = src_1.x - 0.5f;
    var fy_0 : f32 = src_1.y - 0.5f;
    var x0_0 : i32 = i32(floor(fx_0));
    var y0_0 : i32 = i32(floor(fy_0));
    var tx_1 : f32 = fx_0 - f32(x0_0);
    var ty_0 : f32 = fy_0 - f32(y0_0);
    var c00_0 : vec4<f32> = FetchPixelMosaic_0(x0_0, y0_0);
    var _S31 : i32 = x0_0 + i32(1);
    var c10_0 : vec4<f32> = FetchPixelMosaic_0(_S31, y0_0);
    var _S32 : i32 = y0_0 + i32(1);
    var c01_0 : vec4<f32> = FetchPixelMosaic_0(x0_0, _S32);
    var c11_0 : vec4<f32> = FetchPixelMosaic_0(_S31, _S32);
    var _S33 : vec4<f32> = vec4<f32>(tx_1);
    return mix(mix(c00_0, c10_0, _S33), mix(c01_0, c11_0, _S33), vec4<f32>(ty_0));
}

fn MitchellWeight_0( x_0 : f32,  B_0 : f32,  C_0 : f32) -> f32
{
    var ax_0 : f32 = abs(x_0);
    var ax2_0 : f32 = ax_0 * ax_0;
    var ax3_0 : f32 = ax2_0 * ax_0;
    if(ax_0 < 1.0f)
    {
        var _S34 : f32 = 6.0f * C_0;
        return ((12.0f - 9.0f * B_0 - _S34) * ax3_0 + (-18.0f + 12.0f * B_0 + _S34) * ax2_0 + (6.0f - 2.0f * B_0)) * 0.1666666716337204f;
    }
    if(ax_0 < 2.0f)
    {
        return ((- B_0 - 6.0f * C_0) * ax3_0 + (6.0f * B_0 + 30.0f * C_0) * ax2_0 + (-12.0f * B_0 - 48.0f * C_0) * ax_0 + (8.0f * B_0 + 24.0f * C_0)) * 0.1666666716337204f;
    }
    return 0.0f;
}

fn FilterBicubic_0( src_2 : vec2<f32>,  B_1 : f32,  C_1 : f32) -> vec4<f32>
{
    var fx_1 : f32 = src_2.x - 0.5f;
    var fy_1 : f32 = src_2.y - 0.5f;
    var ix_0 : i32 = i32(floor(fx_1));
    var iy_0 : i32 = i32(floor(fy_1));
    var _S35 : f32 = fx_1 - f32(ix_0);
    var _S36 : f32 = fy_1 - f32(iy_0);
    const _S37 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    var j_1 : i32 = i32(-1);
    var sum_2 : vec4<f32> = _S37;
    var ws_0 : f32 = 0.0f;
    for(;;)
    {
        if(j_1 <= i32(2))
        {
        }
        else
        {
            break;
        }
        var _S38 : f32 = MitchellWeight_0(f32(j_1) - _S36, B_1, C_1);
        var _S39 : i32 = iy_0 + j_1;
        var i_1 : i32 = i32(-1);
        for(;;)
        {
            if(i_1 <= i32(2))
            {
            }
            else
            {
                break;
            }
            var w_0 : f32 = MitchellWeight_0(f32(i_1) - _S35, B_1, C_1) * _S38;
            var _S40 : vec4<f32> = FetchPixelMosaic_0(ix_0 + i_1, _S39);
            var sum_3 : vec4<f32> = sum_2 + vec4<f32>(w_0) * _S40;
            var ws_1 : f32 = ws_0 + w_0;
            i_1 = i_1 + i32(1);
            sum_2 = sum_3;
            ws_0 = ws_1;
        }
        j_1 = j_1 + i32(1);
    }
    if(ws_0 != 0.0f)
    {
        sum_2 = sum_2 / vec4<f32>(ws_0);
    }
    return sum_2;
}

fn Sinc_0( x_1 : f32) -> f32
{
    if((abs(x_1)) < 9.99999997475242708e-07f)
    {
        return 1.0f;
    }
    var px_0 : f32 = 3.14159274101257324f * x_1;
    return sin(px_0) / px_0;
}

fn LanczosWeight_0( x_2 : f32,  a_0 : f32) -> f32
{
    if((abs(x_2)) >= a_0)
    {
        return 0.0f;
    }
    return Sinc_0(x_2) * Sinc_0(x_2 / a_0);
}

fn FilterLanczos_0( src_3 : vec2<f32>,  a_1 : i32) -> vec4<f32>
{
    var fx_2 : f32 = src_3.x - 0.5f;
    var fy_2 : f32 = src_3.y - 0.5f;
    var ix_1 : i32 = i32(floor(fx_2));
    var iy_1 : i32 = i32(floor(fy_2));
    var _S41 : f32 = fx_2 - f32(ix_1);
    var _S42 : f32 = fy_2 - f32(iy_1);
    const _S43 : vec4<f32> = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
    var _S44 : f32 = f32(a_1);
    var j_2 : i32 = i32(-2);
    var sum_4 : vec4<f32> = _S43;
    var ws_2 : f32 = 0.0f;
    for(;;)
    {
        if(j_2 <= i32(3))
        {
        }
        else
        {
            break;
        }
        var _S45 : f32 = LanczosWeight_0(f32(j_2) - _S42, _S44);
        var _S46 : i32 = iy_1 + j_2;
        var i_2 : i32 = i32(-2);
        for(;;)
        {
            if(i_2 <= i32(3))
            {
            }
            else
            {
                break;
            }
            var w_1 : f32 = LanczosWeight_0(f32(i_2) - _S41, _S44) * _S45;
            var _S47 : vec4<f32> = FetchPixelMosaic_0(ix_1 + i_2, _S46);
            var sum_5 : vec4<f32> = sum_4 + vec4<f32>(w_1) * _S47;
            var ws_3 : f32 = ws_2 + w_1;
            i_2 = i_2 + i32(1);
            sum_4 = sum_5;
            ws_2 = ws_3;
        }
        j_2 = j_2 + i32(1);
    }
    if(ws_2 != 0.0f)
    {
        sum_4 = sum_4 / vec4<f32>(ws_2);
    }
    return sum_4;
}

fn SampleSource_0( src_4 : vec2<f32>,  src_step_0 : f32) -> vec4<f32>
{
    if(src_step_0 > 1.0f)
    {
        var _S48 : vec4<f32> = FetchPixelMosaic_0(i32(floor(src_4.x)), i32(floor(src_4.y)));
        return _S48;
    }
    var r_2 : vec4<f32>;
    switch(ImageInspectorParams_0.formatPack_0.w)
    {
    case u32(0), :
        {
            var _S49 : vec4<f32> = FilterNearest_0(src_4);
            r_2 = _S49;
            break;
        }
    case u32(1), :
        {
            var _S50 : vec4<f32> = FilterBilinear_0(src_4);
            r_2 = _S50;
            break;
        }
    case u32(2), :
        {
            var _S51 : vec4<f32> = FilterBicubic_0(src_4, 0.3333333432674408f, 0.3333333432674408f);
            r_2 = _S51;
            break;
        }
    case u32(3), :
        {
            var _S52 : vec4<f32> = FilterBicubic_0(src_4, 0.0f, 0.5f);
            r_2 = _S52;
            break;
        }
    case u32(4), :
        {
            var _S53 : vec4<f32> = FilterLanczos_0(src_4, i32(2));
            r_2 = _S53;
            break;
        }
    case u32(5), :
        {
            var _S54 : vec4<f32> = FilterLanczos_0(src_4, i32(3));
            r_2 = _S54;
            break;
        }
    case default, :
        {
            var _S55 : vec4<f32> = FilterBilinear_0(src_4);
            r_2 = _S55;
            break;
        }
    }
    return r_2;
}

fn isnan_0( x_3 : vec3<f32>) -> vec3<bool>
{
    var result_1 : vec3<bool>;
    var i_3 : i32 = i32(0);
    for(;;)
    {
        if(i_3 < i32(3))
        {
        }
        else
        {
            break;
        }
        var _S56 : f32 = x_3[i_3];
        result_1[i_3] = ((_S56) != (_S56));
        i_3 = i_3 + i32(1);
    }
    return result_1;
}

fn isinf_0( x_4 : vec3<f32>) -> vec3<bool>
{
    var result_2 : vec3<bool>;
    var i_4 : i32 = i32(0);
    for(;;)
    {
        if(i_4 < i32(3))
        {
        }
        else
        {
            break;
        }
        var _S57 : f32 = x_4[i_4];
        result_2[i_4] = (((_S57) > 0x1.fffffep+127f) || ((_S57) < -0x1.fffffep+127f));
        i_4 = i_4 + i32(1);
    }
    return result_2;
}

fn SrgbToLinear_0( v_7 : f32) -> f32
{
    if(v_7 <= 0.04044999927282333f)
    {
        return v_7 / 12.92000007629394531f;
    }
    return pow((v_7 + 0.05499999970197678f) / 1.0549999475479126f, 2.40000009536743164f);
}

fn Rec709Inverse_0( v_8 : f32) -> f32
{
    if(v_8 < 0.08100000023841858f)
    {
        return v_8 / 4.5f;
    }
    return pow((v_8 + 0.0989999994635582f) / 1.09899997711181641f, 2.22222232818603516f);
}

fn Rec1886Inverse_0( v_9 : f32) -> f32
{
    return pow(max(v_9, 0.0f), 2.40000009536743164f);
}

fn CineonInverse_0( v_10 : f32) -> f32
{
    return pow(10.0f, (v_10 * 1023.0f - 685.0f) * 0.0020000000949949f) - 0.01080000028014183f;
}

fn SLog2Inverse_0( v_11 : f32) -> f32
{
    if(v_11 >= 0.03000122308731079f)
    {
        return (pow(10.0f, (v_11 - 0.61659598350524902f - 0.02999999932944775f) / 0.43269899487495422f) - 0.03758399933576584f) * 219.0f / 155.0f;
    }
    return (v_11 - 0.03000122308731079f) / 3.53881287574768066f * 219.0f / 155.0f;
}

fn SLog3Inverse_0( v_12 : f32) -> f32
{
    if(v_12 >= 0.16736099123954773f)
    {
        return pow(10.0f, (v_12 * 1023.0f - 420.0f) / 261.5f) * 0.1900000125169754f - 0.00999999977648258f;
    }
    return (v_12 * 1023.0f - 95.0f) * 0.01125000044703484f / 76.210296630859375f;
}

fn LogC3Inverse_0( v_13 : f32) -> f32
{
    if(v_13 > 0.89612150192260742f)
    {
        return (pow(10.0f, (v_13 - 0.38553699851036072f) / 0.24718999862670898f) - 0.0522719994187355f) / 5.55555582046508789f;
    }
    return (v_13 - 0.09280899912118912f) / 5.36765480041503906f;
}

fn LogC4Inverse_0( v_14 : f32) -> f32
{
    var a_2 : f32 = (pow(2.0f, 18.0f) - 16.0f) / 117.4499969482421875f;
    var s_0 : f32 = 7.0f * log(2.0f) * pow(2.0f, 5.56681060791015625f) / (a_2 * 0.90713590383529663f);
    var t_0 : f32 = (pow(2.0f, 4.56681060791015625f) - 64.0f) / a_2;
    if(v_14 < 0.0f)
    {
        return v_14 * s_0 + t_0;
    }
    return (pow(2.0f, 14.0f * ((v_14 - 0.09286412596702576f) / 0.90713590383529663f) + 6.0f) - 64.0f) / a_2;
}

fn CanonLogInverse_0( v_15 : f32) -> f32
{
    if(v_15 < 0.12512247264385223f)
    {
        return - (pow(10.0f, (0.12512247264385223f - v_15) / 0.45310178399085999f) - 1.0f) / 10.15960025787353516f;
    }
    return (pow(10.0f, (v_15 - 0.12512247264385223f) / 0.45310178399085999f) - 1.0f) / 10.15960025787353516f;
}

fn CanonLog2Inverse_0( v_16 : f32) -> f32
{
    if(v_16 < 0.09286412596702576f)
    {
        return - (pow(10.0f, (0.09286412596702576f - v_16) / 0.24136076867580414f) - 1.0f) / 87.09937286376953125f;
    }
    return (pow(10.0f, (v_16 - 0.09286412596702576f) / 0.24136076867580414f) - 1.0f) / 87.09937286376953125f;
}

fn CanonLog3Inverse_0( v_17 : f32) -> f32
{
    if(v_17 < 0.09746547043323517f)
    {
        return - (pow(10.0f, (0.12783901393413544f - v_17) / 0.36726844310760498f) - 1.0f) / 14.98324966430664062f;
    }
    if(v_17 <= 0.15277890861034393f)
    {
        return (v_17 - 0.12512218952178955f) / 1.9754798412322998f;
    }
    return (pow(10.0f, (v_17 - 0.12240537256002426f) / 0.36726844310760498f) - 1.0f) / 14.98324966430664062f;
}

fn VLogInverse_0( v_18 : f32) -> f32
{
    if(v_18 < 0.1809999942779541f)
    {
        return (v_18 - 0.125f) / 5.59999990463256836f;
    }
    return pow(10.0f, (v_18 - 0.59820598363876343f) / 0.24151399731636047f) - 0.00872999988496304f;
}

fn Log3G10Inverse_0( v_19 : f32) -> f32
{
    var x_5 : f32 = v_19 / 0.22249700129032135f;
    var sign_x_0 : f32;
    if(x_5 < 0.0f)
    {
        sign_x_0 = -1.0f;
    }
    else
    {
        sign_x_0 = 1.0f;
    }
    return sign_x_0 * (pow(10.0f, abs(x_5)) - 1.0f) * 0.00999999977648258f;
}

fn BMFilmGen5Inverse_0( v_20 : f32) -> f32
{
    if(v_20 < 0.09246575087308884f)
    {
        return (v_20 - 0.09286399930715561f) / 8.28360557556152344f;
    }
    return pow(2.0f, (v_20 - 0.5300133228302002f) / 0.08692876249551773f) - 0.00549407256767154f;
}

fn AppleLogInverse_0( v_21 : f32) -> f32
{
    if(v_21 < 0.00964455958455801f)
    {
        return v_21 / 47.28711318969726562f + -0.05641087889671326f;
    }
    return pow(2.0f, (v_21 - 0.69336944818496704f) / 0.08550479263067245f) - 0.00999999977648258f;
}

fn FLogInverse_0( v_22 : f32) -> f32
{
    if(v_22 < 0.10053777694702148f)
    {
        return (v_22 - 0.09286399930715561f) / 8.73563098907470703f;
    }
    return (pow(10.0f, (v_22 - 0.79045301675796509f) / 0.34467598795890808f) - 0.00946800038218498f) / 0.55555599927902222f;
}

fn DLogInverse_0( v_23 : f32) -> f32
{
    if(v_23 <= 0.14000000059604645f)
    {
        return (v_23 - 0.09290000051259995f) / 6.02500009536743164f;
    }
    return (pow(10.0f, (v_23 - 0.80749499797821045f) / 0.25562068819999695f) - 0.01080000028014183f) / 0.98919999599456787f;
}

fn PQInverse_0( v_24 : f32) -> f32
{
    var vp_0 : f32 = pow(max(v_24, 0.0f), 0.01268331333994865f);
    return pow(max(vp_0 - 0.8359375f, 0.0f) / (18.8515625f - 18.6875f * vp_0), 6.27739477157592773f);
}

fn HLGInverse_0( v_25 : f32) -> f32
{
    if(v_25 <= 0.5f)
    {
        return v_25 * v_25 / 3.0f;
    }
    return (exp((v_25 - 0.55991071462631226f) / 0.1788327693939209f) + 0.28466892242431641f) / 12.0f;
}

fn ApplyInputTransfer_0( c_1 : vec3<f32>) -> vec3<f32>
{
    var r_3 : vec3<f32>;
    switch(ImageInspectorParams_0.pipelinePack_0.x)
    {
    case u32(0), :
        {
            r_3 = c_1;
            break;
        }
    case u32(1), :
        {
            var g_2 : f32 = ImageInspectorParams_0.exposureParams_0.w;
            r_3 = vec3<f32>(pow(max(c_1.x, 0.0f), g_2), pow(max(c_1.y, 0.0f), g_2), pow(max(c_1.z, 0.0f), g_2));
            break;
        }
    case u32(2), :
        {
            r_3 = vec3<f32>(SrgbToLinear_0(c_1.x), SrgbToLinear_0(c_1.y), SrgbToLinear_0(c_1.z));
            break;
        }
    case u32(3), :
        {
            r_3 = vec3<f32>(Rec709Inverse_0(c_1.x), Rec709Inverse_0(c_1.y), Rec709Inverse_0(c_1.z));
            break;
        }
    case u32(4), :
        {
            r_3 = vec3<f32>(Rec1886Inverse_0(c_1.x), Rec1886Inverse_0(c_1.y), Rec1886Inverse_0(c_1.z));
            break;
        }
    case u32(5), :
        {
            r_3 = vec3<f32>(CineonInverse_0(c_1.x), CineonInverse_0(c_1.y), CineonInverse_0(c_1.z));
            break;
        }
    case u32(6), :
        {
            r_3 = vec3<f32>(SLog2Inverse_0(c_1.x), SLog2Inverse_0(c_1.y), SLog2Inverse_0(c_1.z));
            break;
        }
    case u32(7), :
        {
            r_3 = vec3<f32>(SLog3Inverse_0(c_1.x), SLog3Inverse_0(c_1.y), SLog3Inverse_0(c_1.z));
            break;
        }
    case u32(8), :
        {
            r_3 = vec3<f32>(LogC3Inverse_0(c_1.x), LogC3Inverse_0(c_1.y), LogC3Inverse_0(c_1.z));
            break;
        }
    case u32(9), :
        {
            r_3 = vec3<f32>(LogC4Inverse_0(c_1.x), LogC4Inverse_0(c_1.y), LogC4Inverse_0(c_1.z));
            break;
        }
    case u32(10), :
        {
            r_3 = vec3<f32>(CanonLogInverse_0(c_1.x), CanonLogInverse_0(c_1.y), CanonLogInverse_0(c_1.z));
            break;
        }
    case u32(11), :
        {
            r_3 = vec3<f32>(CanonLog2Inverse_0(c_1.x), CanonLog2Inverse_0(c_1.y), CanonLog2Inverse_0(c_1.z));
            break;
        }
    case u32(12), :
        {
            r_3 = vec3<f32>(CanonLog3Inverse_0(c_1.x), CanonLog3Inverse_0(c_1.y), CanonLog3Inverse_0(c_1.z));
            break;
        }
    case u32(13), :
        {
            r_3 = vec3<f32>(VLogInverse_0(c_1.x), VLogInverse_0(c_1.y), VLogInverse_0(c_1.z));
            break;
        }
    case u32(14), :
        {
            r_3 = vec3<f32>(Log3G10Inverse_0(c_1.x), Log3G10Inverse_0(c_1.y), Log3G10Inverse_0(c_1.z));
            break;
        }
    case u32(15), :
        {
            r_3 = vec3<f32>(BMFilmGen5Inverse_0(c_1.x), BMFilmGen5Inverse_0(c_1.y), BMFilmGen5Inverse_0(c_1.z));
            break;
        }
    case u32(16), :
        {
            r_3 = vec3<f32>(AppleLogInverse_0(c_1.x), AppleLogInverse_0(c_1.y), AppleLogInverse_0(c_1.z));
            break;
        }
    case u32(17), :
        {
            r_3 = vec3<f32>(FLogInverse_0(c_1.x), FLogInverse_0(c_1.y), FLogInverse_0(c_1.z));
            break;
        }
    case u32(18), :
        {
            r_3 = vec3<f32>(DLogInverse_0(c_1.x), DLogInverse_0(c_1.y), DLogInverse_0(c_1.z));
            break;
        }
    case u32(19), :
        {
            r_3 = vec3<f32>(PQInverse_0(c_1.x), PQInverse_0(c_1.y), PQInverse_0(c_1.z));
            break;
        }
    case u32(20), :
        {
            r_3 = vec3<f32>(HLGInverse_0(c_1.x), HLGInverse_0(c_1.y), HLGInverse_0(c_1.z));
            break;
        }
    case default, :
        {
            r_3 = c_1;
            break;
        }
    }
    return r_3;
}

fn MulMatrix_0( r0_0 : vec4<f32>,  r1_0 : vec4<f32>,  r2_0 : vec4<f32>,  v_26 : vec3<f32>) -> vec3<f32>
{
    return vec3<f32>(dot(r0_0.xyz, v_26), dot(r1_0.xyz, v_26), dot(r2_0.xyz, v_26));
}

fn ApplyTempTint_0( c_2 : vec3<f32>,  temp_0 : f32,  tint_1 : f32) -> vec3<f32>
{
    var _S58 : vec3<f32> = c_2;
    var tw_1 : f32 = temp_0 * 0.10000000149011612f;
    var tt_0 : f32 = tint_1 * 0.10000000149011612f;
    _S58[i32(0)] = _S58[i32(0)] * (1.0f + tw_1 + tt_0);
    _S58[i32(1)] = _S58[i32(1)] * (1.0f - tt_0);
    _S58[i32(2)] = _S58[i32(2)] * (1.0f - tw_1 + tt_0);
    return _S58;
}

fn TonemapReinhard_0( c_3 : vec3<f32>) -> vec3<f32>
{
    return c_3 / (vec3<f32>(1.0f) + c_3);
}

fn TonemapReinhardExt_0( c_4 : vec3<f32>) -> vec3<f32>
{
    var W_0 : f32 = max(ImageInspectorParams_0.exposureParams_0.z, 0.00100000004749745f);
    var _S59 : vec3<f32> = vec3<f32>(1.0f);
    return c_4 * (_S59 + c_4 / vec3<f32>((W_0 * W_0))) / (_S59 + c_4);
}

fn TonemapACES_0( c_5 : vec3<f32>) -> vec3<f32>
{
    return saturate(c_5 * (vec3<f32>(2.50999999046325684f) * c_5 + vec3<f32>(0.02999999932944775f)) / (c_5 * (vec3<f32>(2.43000006675720215f) * c_5 + vec3<f32>(0.5899999737739563f)) + vec3<f32>(0.14000000059604645f)));
}

fn TonemapAGX_0( c_6 : vec3<f32>) -> vec3<f32>
{
    var v_27 : vec3<f32> = saturate((log2(max(c_6, vec3<f32>(1.00000001335143196e-10f))) + vec3<f32>(12.47393035888671875f)) / vec3<f32>(16.5f));
    var v2_0 : vec3<f32> = v_27 * v_27;
    var v3_0 : vec3<f32> = v2_0 * v_27;
    var v4_0 : vec3<f32> = v2_0 * v2_0;
    return vec3<f32>(-17.8600006103515625f) * v3_0 * v3_0 + vec3<f32>(78.01000213623046875f) * v4_0 * v_27 - vec3<f32>(126.6999969482421875f) * v4_0 + vec3<f32>(92.05999755859375f) * v3_0 - vec3<f32>(28.71999931335449219f) * v2_0 + vec3<f32>(4.36100006103515625f) * v_27 - vec3<f32>(0.17180000245571136f);
}

fn TonemapPBRNeutral_0( c_7 : vec3<f32>) -> vec3<f32>
{
    var x_6 : f32 = min(c_7.x, min(c_7.y, c_7.z));
    var offset_0 : f32;
    if(x_6 < 0.07999999821186066f)
    {
        offset_0 = x_6 - 6.25f * x_6 * x_6;
    }
    else
    {
        offset_0 = 0.03999999910593033f;
    }
    var _S60 : vec3<f32> = c_7 - vec3<f32>(offset_0);
    var peak_0 : f32 = max(_S60.x, max(_S60.y, _S60.z));
    if(peak_0 < 0.75999999046325684f)
    {
        return _S60;
    }
    var newPeak_0 : f32 = 1.0f - 0.0576000027358532f / (peak_0 + 0.24000000953674316f - 0.75999999046325684f);
    return mix(_S60 * vec3<f32>((newPeak_0 / peak_0)), vec3<f32>(newPeak_0), vec3<f32>((1.0f - 1.0f / (0.15000000596046448f * (peak_0 - newPeak_0) + 1.0f))));
}

fn TonemapHable_0( c_8 : vec3<f32>) -> vec3<f32>
{
    var _S61 : vec3<f32> = vec3<f32>(0.15000000596046448f) * c_8;
    return ((c_8 * (_S61 + vec3<f32>(0.05000000074505806f)) + vec3<f32>(0.00400000018998981f)) / (c_8 * (_S61 + vec3<f32>(0.5f)) + vec3<f32>(0.06000000238418579f)) - vec3<f32>(0.06666666269302368f)) / vec3<f32>(0.72512936592102051f);
}

fn ApplyTonemap_0( c_9 : vec3<f32>) -> vec3<f32>
{
    var r_4 : vec3<f32>;
    switch(ImageInspectorParams_0.pipelinePack_0.z)
    {
    case u32(0), :
        {
            r_4 = c_9;
            break;
        }
    case u32(1), :
        {
            r_4 = TonemapReinhard_0(c_9);
            break;
        }
    case u32(2), :
        {
            r_4 = TonemapReinhardExt_0(c_9);
            break;
        }
    case u32(3), :
        {
            r_4 = TonemapACES_0(c_9);
            break;
        }
    case u32(4), :
        {
            r_4 = TonemapAGX_0(c_9);
            break;
        }
    case u32(5), :
        {
            r_4 = TonemapPBRNeutral_0(c_9);
            break;
        }
    case u32(6), :
        {
            r_4 = TonemapHable_0(c_9);
            break;
        }
    case default, :
        {
            r_4 = c_9;
            break;
        }
    }
    return r_4;
}

fn LinearToSrgb_0( v_28 : f32) -> f32
{
    if(v_28 <= 0.00313080009073019f)
    {
        return v_28 * 12.92000007629394531f;
    }
    return 1.0549999475479126f * pow(max(v_28, 0.0f), 0.4166666567325592f) - 0.05499999970197678f;
}

fn PQForward_0( v_29 : f32) -> f32
{
    var lp_0 : f32 = pow(max(v_29, 0.0f), 0.1593017578125f);
    return pow((0.8359375f + 18.8515625f * lp_0) / (1.0f + 18.6875f * lp_0), 78.84375f);
}

fn HLGForward_0( v_30 : f32) -> f32
{
    if(v_30 <= 0.0833333358168602f)
    {
        return sqrt(3.0f * v_30);
    }
    return 0.1788327693939209f * log(12.0f * v_30 - 0.28466892242431641f) + 0.55991071462631226f;
}

fn ApplyOutputTransfer_0( c_10 : vec3<f32>) -> vec3<f32>
{
    var r_5 : vec3<f32>;
    switch(ImageInspectorParams_0.pipelinePack_0.y)
    {
    case u32(0), :
        {
            r_5 = c_10;
            break;
        }
    case u32(1), :
        {
            var g_3 : f32 = 1.0f / max(ImageInspectorParams_0.exposureParams_0.w, 0.00100000004749745f);
            r_5 = vec3<f32>(pow(max(c_10.x, 0.0f), g_3), pow(max(c_10.y, 0.0f), g_3), pow(max(c_10.z, 0.0f), g_3));
            break;
        }
    case u32(2), :
        {
            r_5 = vec3<f32>(LinearToSrgb_0(c_10.x), LinearToSrgb_0(c_10.y), LinearToSrgb_0(c_10.z));
            break;
        }
    case u32(3), :
        {
            r_5 = vec3<f32>(PQForward_0(c_10.x), PQForward_0(c_10.y), PQForward_0(c_10.z));
            break;
        }
    case u32(4), :
        {
            r_5 = vec3<f32>(HLGForward_0(c_10.x), HLGForward_0(c_10.y), HLGForward_0(c_10.z));
            break;
        }
    case default, :
        {
            r_5 = c_10;
            break;
        }
    }
    return r_5;
}

fn PaletteViridis_0( t_1 : f32) -> vec3<f32>
{
    var _S62 : vec3<f32> = vec3<f32>(saturate(t_1));
    return vec3<f32>(0.2669999897480011f, 0.00499999988824129f, 0.32899999618530273f) + _S62 * (vec3<f32>(0.10499999672174454f, 1.40499997138977051f, 1.38499999046325684f) + _S62 * (vec3<f32>(-0.33000001311302185f, -0.31700000166893005f, 0.21400000154972076f) + _S62 * (vec3<f32>(6.22800016403198242f, -2.51999998092651367f, -2.66499996185302734f) + _S62 * (vec3<f32>(-13.09000015258789062f, 1.39499998092651367f, 6.33099985122680664f) + _S62 * (vec3<f32>(11.10000038146972656f, 0.0f, -7.2350001335144043f) + _S62 * vec3<f32>(-3.65799999237060547f, 0.0f, 2.09299993515014648f))))));
}

fn PaletteMagma_0( t_2 : f32) -> vec3<f32>
{
    var _S63 : vec3<f32> = vec3<f32>(saturate(t_2));
    return vec3<f32>(-0.0020000000949949f, -0.0f, -0.01400000043213367f) + _S63 * (vec3<f32>(0.25499999523162842f, 0.0390000008046627f, 1.60199999809265137f) + _S63 * (vec3<f32>(7.18699979782104492f, 2.79399991035461426f, 4.80399990081787109f) + _S63 * (vec3<f32>(-25.95000076293945312f, -7.70100021362304688f, -23.3899993896484375f) + _S63 * (vec3<f32>(38.20999908447265625f, 8.50199985504150391f, 38.29999923706054688f) + _S63 * (vec3<f32>(-25.54999923706054688f, -3.85100007057189941f, -27.39999961853027344f) + _S63 * vec3<f32>(6.48400020599365234f, 0.5339999794960022f, 7.46500015258789062f))))));
}

fn PaletteInferno_0( t_3 : f32) -> vec3<f32>
{
    var _S64 : vec3<f32> = vec3<f32>(saturate(t_3));
    return vec3<f32>(0.00019999999494758f, 0.00170000002253801f, -0.01930000074207783f) + _S64 * (vec3<f32>(0.10599999874830246f, 0.56099998950958252f, 3.9869999885559082f) + _S64 * (vec3<f32>(11.60200023651123047f, -3.9719998836517334f, -15.93999958038330078f) + _S64 * (vec3<f32>(-41.70999908447265625f, 17.43000030517578125f, 44.34999847412109375f) + _S64 * (vec3<f32>(77.160003662109375f, -33.40000152587890625f, -81.8000030517578125f) + _S64 * (vec3<f32>(-71.31999969482421875f, 32.63000106811523438f, 73.20999908447265625f) + _S64 * vec3<f32>(25.12999916076660156f, -12.23999977111816406f, -23.06999969482421875f))))));
}

fn PalettePlasma_0( t_4 : f32) -> vec3<f32>
{
    var _S65 : vec3<f32> = vec3<f32>(saturate(t_4));
    return vec3<f32>(0.05880000069737434f, 0.02969999983906746f, 0.53100001811981201f) + _S65 * (vec3<f32>(2.17600011825561523f, 0.23800000548362732f, 1.07200002670288086f) + _S65 * (vec3<f32>(0.11699999868869781f, 0.48059999942779541f, -3.02900004386901855f) + _S65 * (vec3<f32>(-9.62800025939941406f, -1.92900002002716064f, 4.42299985885620117f) + _S65 * (vec3<f32>(20.79000091552734375f, 1.94000005722045898f, -3.13700008392333984f) + _S65 * (vec3<f32>(-17.05999946594238281f, -0.73299998044967651f, 0.9649999737739563f) + _S65 * vec3<f32>(4.68800020217895508f, 0.06970000267028809f, -0.08299999684095383f))))));
}

fn PaletteCividis_0( t_5 : f32) -> vec3<f32>
{
    var _S66 : vec3<f32> = vec3<f32>(saturate(t_5));
    return vec3<f32>(-0.00860000029206276f, 0.13220000267028809f, 0.30140000581741333f) + _S66 * (vec3<f32>(0.54439997673034668f, 0.71960002183914185f, 1.45229995250701904f) + _S66 * (vec3<f32>(-0.09080000221729279f, 0.13339999318122864f, -3.95300006866455078f) + _S66 * (vec3<f32>(0.55629998445510864f, 0.01810000091791153f, 4.08459997177124023f) + _S66 * vec3<f32>(-0.00170000002253801f, -0.00460000010207295f, -1.88569998741149902f))));
}

fn PaletteTurbo_0( t_6 : f32) -> vec3<f32>
{
    var _S67 : vec3<f32> = vec3<f32>(saturate(t_6));
    return vec3<f32>(0.13570000231266022f, 0.09139999747276306f, 0.10670000314712524f) + _S67 * (vec3<f32>(4.59740018844604492f, 2.18560004234313965f, 12.59249973297119141f) + _S67 * (vec3<f32>(-42.65999984741210938f, 4.84100008010864258f, -60.582000732421875f) + _S67 * (vec3<f32>(132.1300048828125f, -14.18500041961669922f, 110.51000213623046875f) + _S67 * (vec3<f32>(-152.94000244140625f, 4.27740001678466797f, -89.9010009765625f) + _S67 * vec3<f32>(59.28599929809570312f, 2.79929995536804199f, 27.34300041198730469f)))));
}

fn PaletteCinema_0( v_31 : f32) -> vec3<f32>
{
    if(v_31 < 0.0f)
    {
        return vec3<f32>(0.5f, 0.0f, 0.5f);
    }
    if(v_31 < 0.03999999910593033f)
    {
        return vec3<f32>(0.0f, 0.0f, 1.0f);
    }
    if(v_31 < 0.10000000149011612f)
    {
        return vec3<f32>(0.0f, 0.5f, 1.0f);
    }
    if(v_31 < 0.18000000715255737f)
    {
        return vec3<f32>(0.0f, 1.0f, 1.0f);
    }
    if(v_31 < 0.41999998688697815f)
    {
        return vec3<f32>(0.5f, 1.0f, 0.0f);
    }
    if(v_31 < 0.77999997138977051f)
    {
        return vec3<f32>(1.0f, 1.0f, 0.0f);
    }
    if(v_31 < 0.94999998807907104f)
    {
        return vec3<f32>(1.0f, 0.5f, 0.0f);
    }
    if(v_31 <= 1.0f)
    {
        return vec3<f32>(1.0f, 0.0f, 0.0f);
    }
    return vec3<f32>(1.0f, 1.0f, 1.0f);
}

fn ApplyFalseColor_0( c_11 : vec3<f32>) -> vec3<f32>
{
    var _S68 : f32 = c_11.x;
    var _S69 : f32 = c_11.y;
    var _S70 : f32 = c_11.z;
    var v_32 : f32 = (_S68 + _S69 + _S70) * 0.3333333432674408f;
    var r_6 : vec3<f32>;
    switch(ImageInspectorParams_0.pipelinePack_0.w)
    {
    case u32(0), :
        {
            r_6 = c_11;
            break;
        }
    case u32(1), :
        {
            r_6 = PaletteViridis_0(v_32);
            break;
        }
    case u32(2), :
        {
            r_6 = PaletteMagma_0(v_32);
            break;
        }
    case u32(3), :
        {
            r_6 = PaletteInferno_0(v_32);
            break;
        }
    case u32(4), :
        {
            r_6 = PalettePlasma_0(v_32);
            break;
        }
    case u32(5), :
        {
            r_6 = PaletteCividis_0(v_32);
            break;
        }
    case u32(6), :
        {
            r_6 = PaletteTurbo_0(v_32);
            break;
        }
    case u32(7), :
        {
            r_6 = PaletteCinema_0(v_32);
            break;
        }
    case u32(8), :
        {
            var oog_0 : bool;
            if(_S68 < 0.0f)
            {
                oog_0 = true;
            }
            else
            {
                oog_0 = _S69 < 0.0f;
            }
            if(oog_0)
            {
                oog_0 = true;
            }
            else
            {
                oog_0 = _S70 < 0.0f;
            }
            if(oog_0)
            {
                oog_0 = true;
            }
            else
            {
                oog_0 = _S68 > 1.0f;
            }
            if(oog_0)
            {
                oog_0 = true;
            }
            else
            {
                oog_0 = _S69 > 1.0f;
            }
            if(oog_0)
            {
                oog_0 = true;
            }
            else
            {
                oog_0 = _S70 > 1.0f;
            }
            if(oog_0)
            {
                r_6 = vec3<f32>(1.0f, 1.0f, 0.0f);
            }
            else
            {
                r_6 = c_11;
            }
            break;
        }
    case default, :
        {
            r_6 = c_11;
            break;
        }
    }
    return r_6;
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
fn main_ps( _S71 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    var totalScale_0 : f32 = ImageInspectorParams_0.panZoom_0.z * ImageInspectorParams_0.panZoom_0.w;
    var _S72 : vec2<f32> = vec2<f32>(0.5f);
    var sampled_0 : vec4<f32> = SampleSource_0(ImageInspectorParams_0.imgSize_0.xy * _S72 + ImageInspectorParams_0.panZoom_0.xy + (_S71.uv_0 - _S72) * ImageInspectorParams_0.viewportPx_0.xy / vec2<f32>(totalScale_0), 1.0f / max(totalScale_0, 9.99999997475242708e-07f));
    var rgb_0 : vec3<f32> = sampled_0.xyz;
    var a_3 : f32 = sampled_0.w;
    var _S73 : bool;
    if(any(isnan_0(rgb_0)))
    {
        _S73 = true;
    }
    else
    {
        _S73 = any(isinf_0(rgb_0));
    }
    if(_S73)
    {
        var _S74 : pixelOutput_0 = pixelOutput_0( vec4<f32>(ImageInspectorParams_0.nanColor_0.xyz, 1.0f) );
        return _S74;
    }
    var _S75 : pixelOutput_0 = pixelOutput_0( vec4<f32>(ApplyFalseColor_0(ApplyOutputTransfer_0(MulMatrix_0(ImageInspectorParams_0.outGamut_r0_0, ImageInspectorParams_0.outGamut_r1_0, ImageInspectorParams_0.outGamut_r2_0, ApplyTonemap_0(ApplyTempTint_0((MulMatrix_0(ImageInspectorParams_0.inGamut_r0_0, ImageInspectorParams_0.inGamut_r1_0, ImageInspectorParams_0.inGamut_r2_0, ApplyInputTransfer_0(rgb_0)) - vec3<f32>(ImageInspectorParams_0.exposureParams_0.y)) / vec3<f32>(max(ImageInspectorParams_0.exposureParams_0.z - ImageInspectorParams_0.exposureParams_0.y, 9.99999997475242708e-07f)) * vec3<f32>(exp2(ImageInspectorParams_0.exposureParams_0.x)), ImageInspectorParams_0.tempTint_0.x, ImageInspectorParams_0.tempTint_0.y)))) * ImageInspectorParams_0.channelMask_0.xyz), a_3 * ImageInspectorParams_0.channelMask_0.w) * _S71.col_0 );
    return _S75;
}

