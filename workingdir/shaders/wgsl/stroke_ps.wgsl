struct SLANG_ParameterGroup_strokeBuffer_std140_0
{
    @align(16) params_0 : vec4<f32>,
    @align(16) strokeColor_0 : vec4<f32>,
    @align(16) bounds_0 : vec4<f32>,
    @align(16) segments_0 : array<vec4<f32>, i32(1024)>,
};

@binding(1) @group(0) var<uniform> strokeBuffer_0 : SLANG_ParameterGroup_strokeBuffer_std140_0;
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
fn main_ps( _S1 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    var _S2 : vec2<f32> = mix(strokeBuffer_0.bounds_0.xy, strokeBuffer_0.bounds_0.zw, _S1.uv_0.xy);
    var _S3 : i32 = i32(strokeBuffer_0.params_0.x);
    var aa_0 : f32 = strokeBuffer_0.params_0.z;
    var _S4 : f32 = _S2.y;
    var _S5 : f32 = _S2.x;
    var min_dist_sq_0 : f32 = 1.0e+10f;
    var i_0 : i32 = i32(0);
    var winding_0 : i32 = i32(0);
    for(;;)
    {
        if(i_0 < _S3)
        {
        }
        else
        {
            break;
        }
        var p0_0 : vec2<f32> = strokeBuffer_0.segments_0[i_0].xy;
        var p1_0 : vec2<f32> = strokeBuffer_0.segments_0[i_0].zw;
        var _S6 : f32 = p0_0.y;
        var winding_1 : i32;
        if(_S6 <= _S4)
        {
            var _S7 : f32 = p1_0.y;
            if(_S7 > _S4)
            {
                var _S8 : f32 = p0_0.x;
                if(((p1_0.x - _S8) * (_S4 - _S6) - (_S5 - _S8) * (_S7 - _S6)) > 0.0f)
                {
                    winding_1 = winding_0 + i32(1);
                }
                else
                {
                    winding_1 = winding_0;
                }
            }
            else
            {
                winding_1 = winding_0;
            }
        }
        else
        {
            var _S9 : f32 = p1_0.y;
            if(_S9 <= _S4)
            {
                var _S10 : f32 = p0_0.x;
                if(((p1_0.x - _S10) * (_S4 - _S6) - (_S5 - _S10) * (_S9 - _S6)) < 0.0f)
                {
                    winding_1 = winding_0 - i32(1);
                }
                else
                {
                    winding_1 = winding_0;
                }
            }
            else
            {
                winding_1 = winding_0;
            }
        }
        var d_0 : vec2<f32> = p1_0 - p0_0;
        var len_sq_0 : f32 = dot(d_0, d_0);
        var t_0 : f32;
        if(len_sq_0 > 9.99999993922529029e-09f)
        {
            t_0 = saturate(dot(_S2 - p0_0, d_0) / len_sq_0);
        }
        else
        {
            t_0 = 0.0f;
        }
        var diff_0 : vec2<f32> = _S2 - (p0_0 + d_0 * vec2<f32>(t_0));
        var _S11 : f32 = min(min_dist_sq_0, dot(diff_0, diff_0));
        var _S12 : i32 = i_0 + i32(1);
        min_dist_sq_0 = _S11;
        i_0 = _S12;
        winding_0 = winding_1;
    }
    var min_dist_0 : f32 = sqrt(min_dist_sq_0);
    if(winding_0 == i32(0))
    {
        var _S13 : bool;
        if(aa_0 > 0.0f)
        {
            _S13 = min_dist_0 < aa_0;
        }
        else
        {
            _S13 = false;
        }
        if(_S13)
        {
            var _S14 : pixelOutput_0 = pixelOutput_0( vec4<f32>(strokeBuffer_0.strokeColor_0.xyz, strokeBuffer_0.strokeColor_0.w * (1.0f - min_dist_0 / aa_0)) );
            return _S14;
        }
        discard;
        var _S15 : pixelOutput_0 = pixelOutput_0( vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f) );
        return _S15;
    }
    else
    {
        var _S16 : pixelOutput_0 = pixelOutput_0( strokeBuffer_0.strokeColor_0 );
        return _S16;
    }
}

