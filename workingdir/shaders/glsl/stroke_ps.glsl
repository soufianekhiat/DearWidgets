#version 450
layout(row_major) uniform;
layout(row_major) buffer;

#line 20 0
struct _Array_std140_vectorx3Cfloatx2C4x3E1024_0
{
    vec4  data_0[1024];
};


#line 20
struct SLANG_ParameterGroup_strokeBuffer_std140_0
{
    vec4 params_0;
    vec4 strokeColor_0;
    vec4 bounds_0;
    _Array_std140_vectorx3Cfloatx2C4x3E1024_0 segments_0;
};


#line 15
layout(binding = 1)
layout(std140) uniform block_SLANG_ParameterGroup_strokeBuffer_std140_0
{
    vec4 params_0;
    vec4 strokeColor_0;
    vec4 bounds_0;
    _Array_std140_vectorx3Cfloatx2C4x3E1024_0 segments_0;
}strokeBuffer_0;

#line 12386 1
float saturate_0(float x_0)
{

#line 12394
    return clamp(x_0, 0.0, 1.0);
}


#line 12833
layout(location = 0)
out vec4 entryPointParam_main_ps_0;


#line 12833
layout(location = 1)
in vec2 input_uv_0;


#line 46 0
void main()
{

    vec2 _S1 = mix(strokeBuffer_0.bounds_0.xy, strokeBuffer_0.bounds_0.zw, input_uv_0.xy);
    int _S2 = int(strokeBuffer_0.params_0.x);
    float aa_0 = strokeBuffer_0.params_0.z;

#line 65
    float _S3 = _S1.y;



    float _S4 = _S1.x;

#line 69
    float min_dist_sq_0 = 1.0e+10;

#line 69
    int i_0 = 0;

#line 69
    int winding_0 = 0;

#line 59
    for(;;)
    {

#line 59
        if(i_0 < _S2)
        {
        }
        else
        {

#line 59
            break;
        }
        vec2 p0_0 = strokeBuffer_0.segments_0.data_0[i_0].xy;
        vec2 p1_0 = strokeBuffer_0.segments_0.data_0[i_0].zw;


        float _S5 = p0_0.y;

#line 65
        int winding_1;

#line 65
        if(_S5 <= _S3)
        {
            float _S6 = p1_0.y;

#line 67
            if(_S6 > _S3)
            {
                float _S7 = p0_0.x;
                if(((p1_0.x - _S7) * (_S3 - _S5) - (_S4 - _S7) * (_S6 - _S5)) > 0.0)
                {

#line 70
                    winding_1 = winding_0 + 1;

#line 70
                }
                else
                {

#line 70
                    winding_1 = winding_0;

#line 70
                }

#line 67
            }
            else
            {

#line 67
                winding_1 = winding_0;

#line 67
            }

#line 65
        }
        else
        {

#line 75
            float _S8 = p1_0.y;

#line 75
            if(_S8 <= _S3)
            {
                float _S9 = p0_0.x;
                if(((p1_0.x - _S9) * (_S3 - _S5) - (_S4 - _S9) * (_S8 - _S5)) < 0.0)
                {

#line 78
                    winding_1 = winding_0 - 1;

#line 78
                }
                else
                {

#line 78
                    winding_1 = winding_0;

#line 78
                }

#line 75
            }
            else
            {

#line 75
                winding_1 = winding_0;

#line 75
            }

#line 65
        }

#line 83
        vec2 d_0 = p1_0 - p0_0;
        float len_sq_0 = dot(d_0, d_0);

#line 84
        float t_0;
        if(len_sq_0 > 9.99999993922529029e-09)
        {

#line 85
            t_0 = saturate_0(dot(_S1 - p0_0, d_0) / len_sq_0);

#line 85
        }
        else
        {

#line 85
            t_0 = 0.0;

#line 85
        }

        vec2 diff_0 = _S1 - (p0_0 + d_0 * t_0);

        float _S10 = min(min_dist_sq_0, dot(diff_0, diff_0));

#line 59
        int _S11 = i_0 + 1;

#line 59
        min_dist_sq_0 = _S10;

#line 59
        i_0 = _S11;

#line 59
        winding_0 = winding_1;

#line 59
    }

#line 93
    float min_dist_0 = sqrt(min_dist_sq_0);

    if(winding_0 == 0)
    {

#line 95
        bool _S12;


        if(aa_0 > 0.0)
        {

#line 98
            _S12 = min_dist_0 < aa_0;

#line 98
        }
        else
        {

#line 98
            _S12 = false;

#line 98
        }

#line 98
        if(_S12)
        {

#line 98
            entryPointParam_main_ps_0 = vec4(strokeBuffer_0.strokeColor_0.xyz, strokeBuffer_0.strokeColor_0.w * (1.0 - min_dist_0 / aa_0));

#line 98
            return;
        }



        discard;

#line 103
        entryPointParam_main_ps_0 = vec4(0.0, 0.0, 0.0, 0.0);

#line 103
        return;
    }
    else
    {

#line 103
        entryPointParam_main_ps_0 = strokeBuffer_0.strokeColor_0;

#line 103
        return;
    }

#line 103
}

