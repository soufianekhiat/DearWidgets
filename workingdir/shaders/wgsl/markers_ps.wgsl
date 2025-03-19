struct SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0
{
    @align(16) fg_color_0 : vec4<f32>,
    @align(16) bg_color_0 : vec4<f32>,
    @align(16) rotation_0 : vec2<f32>,
    @align(8) linewidth_0 : f32,
    @align(4) size_0 : f32,
    @align(16) type_0 : f32,
    @align(4) antialiasing_0 : f32,
    @align(8) draw_type_0 : f32,
    @align(4) pad0_0 : f32,
};

@binding(0) @group(0) var<uniform> PS_CONSTANT_BUFFER_0 : SLANG_ParameterGroup_PS_CONSTANT_BUFFER_std140_0;
fn disc_0( P_0 : vec2<f32>,  size_1 : f32) -> f32
{
    return length(P_0) - size_1 * 0.5f;
}

fn square_0( P_1 : vec2<f32>,  size_2 : f32) -> f32
{
    return max(abs(P_1.x), abs(P_1.y)) - size_2 / 2.82842707633972168f;
}

fn triangle2_0( P_2 : vec2<f32>,  size_3 : f32) -> f32
{
    var _S1 : f32 = P_2.x;
    var _S2 : f32 = P_2.y;
    return max(max(abs(0.70710676908493042f * (_S1 - _S2)), abs(0.70710676908493042f * (_S1 + _S2))) - size_3 / 2.82842707633972168f, _S2);
}

fn diamond_0( P_3 : vec2<f32>,  size_4 : f32) -> f32
{
    var _S3 : f32 = P_3.x;
    var _S4 : f32 = P_3.y;
    return max(abs(0.70710676908493042f * (_S3 - _S4)), abs(0.70710676908493042f * (_S3 + _S4))) - size_4 / 2.82842707633972168f;
}

fn heart_0( P_4 : vec2<f32>,  size_5 : f32) -> f32
{
    var _S5 : f32 = P_4.x;
    var _S6 : f32 = P_4.y;
    var _S7 : vec2<f32> = vec2<f32>(0.70710676908493042f);
    var _S8 : vec2<f32> = vec2<f32>(size_5);
    var _S9 : vec2<f32> = vec2<f32>(3.5f);
    return min(min(max(abs(0.70710676908493042f * (_S5 - _S6)), abs(0.70710676908493042f * (_S5 + _S6))) - size_5 / 3.5f, length(P_4 - _S7 * vec2<f32>(1.0f, -1.0f) * _S8 / _S9) - size_5 / 3.5f), length(P_4 - _S7 * vec2<f32>(-1.0f, -1.0f) * _S8 / _S9) - size_5 / 3.5f);
}

fn spade_0( P_5 : vec2<f32>,  size_6 : f32) -> f32
{
    var s_0 : f32 = size_6 * 0.85000002384185791f / 3.5f;
    var _S10 : f32 = P_5.x;
    var _S11 : f32 = P_5.y;
    var _S12 : f32 = 0.40000000596046448f * s_0;
    var _S13 : vec2<f32> = vec2<f32>(0.70710676908493042f);
    var _S14 : vec2<f32> = vec2<f32>(s_0);
    var _S15 : vec2<f32> = vec2<f32>(size_6);
    return min(min(min(max(abs(0.70710676908493042f * (_S10 + _S11) + _S12), abs(0.70710676908493042f * (_S10 - _S11) - _S12)) - s_0, length(P_5 - _S13 * vec2<f32>(1.0f, 0.20000000298023224f) * _S14) - s_0), length(P_5 - _S13 * vec2<f32>(-1.0f, 0.20000000298023224f) * _S14) - s_0), max(- min(length(P_5 - vec2<f32>(0.64999997615814209f, 0.125f) * _S15) - size_6 / 1.60000002384185791f, length(P_5 - vec2<f32>(-0.64999997615814209f, 0.125f) * _S15) - size_6 / 1.60000002384185791f), max(_S11 - 0.5f * size_6, 0.10000000149011612f * size_6 - _S11)));
}

fn club_0( P_6 : vec2<f32>,  size_7 : f32) -> f32
{
    var _S16 : vec2<f32> = vec2<f32>(0.22499999403953552f);
    var _S17 : vec2<f32> = vec2<f32>(size_7);
    var _S18 : f32 = P_6.y;
    return min(min(min(length(P_6 - _S16 * vec2<f32>(cos(-1.57079637050628662f), sin(-1.57079637050628662f)) * _S17) - size_7 / 4.25f, length(P_6 - _S16 * vec2<f32>(cos(0.52359879016876221f), sin(0.52359879016876221f)) * _S17) - size_7 / 4.25f), length(P_6 - _S16 * vec2<f32>(cos(2.61799383163452148f), sin(2.61799383163452148f)) * _S17) - size_7 / 4.25f), max(- min(length(P_6 - vec2<f32>(0.64999997615814209f, 0.125f) * _S17) - size_7 / 1.60000002384185791f, length(P_6 - vec2<f32>(-0.64999997615814209f, 0.125f) * _S17) - size_7 / 1.60000002384185791f), max(_S18 - 0.5f * size_7, 0.20000000298023224f * size_7 - _S18)));
}

fn chevron_0( P_7 : vec2<f32>,  size_8 : f32) -> f32
{
    var _S19 : f32 = P_7.x;
    var _S20 : f32 = P_7.y;
    var x_0 : f32 = 0.70710676908493042f * (_S19 - _S20);
    var y_0 : f32 = 0.70710676908493042f * (_S19 + _S20);
    return max(max(abs(x_0), abs(y_0)) - size_8 / 3.0f, - (max(abs(x_0 - size_8 / 3.0f), abs(y_0 - size_8 / 3.0f)) - size_8 / 3.0f));
}

fn clover_0( P_8 : vec2<f32>,  size_9 : f32) -> f32
{
    var _S21 : vec2<f32> = vec2<f32>(0.25f);
    var _S22 : vec2<f32> = vec2<f32>(size_9);
    return min(min(length(P_8 - _S21 * vec2<f32>(cos(-1.57079637050628662f), sin(-1.57079637050628662f)) * _S22) - size_9 / 3.5f, length(P_8 - _S21 * vec2<f32>(cos(0.52359879016876221f), sin(0.52359879016876221f)) * _S22) - size_9 / 3.5f), length(P_8 - _S21 * vec2<f32>(cos(2.61799383163452148f), sin(2.61799383163452148f)) * _S22) - size_9 / 3.5f);
}

fn ring_0( P_9 : vec2<f32>,  size_10 : f32) -> f32
{
    var _S23 : f32 = length(P_9);
    return max(_S23 - size_10 / 2.0f, - (_S23 - size_10 / 4.0f));
}

fn tag_0( P_10 : vec2<f32>,  size_11 : f32) -> f32
{
    var _S24 : f32 = P_10.x;
    var _S25 : f32 = abs(P_10.y);
    return max(max(abs(_S24) - size_11 / 2.0f, _S25 - size_11 / 6.0f), 0.75f * (abs(_S24 - size_11 / 1.5f) + _S25 - size_11));
}

fn cross_0( P_11 : vec2<f32>,  size_12 : f32) -> f32
{
    var _S26 : f32 = P_11.x;
    var _S27 : f32 = P_11.y;
    var x_1 : f32 = 0.70710676908493042f * (_S26 - _S27);
    var y_1 : f32 = 0.70710676908493042f * (_S26 + _S27);
    return max(min(max(abs(x_1 - size_12 / 3.0f), abs(x_1 + size_12 / 3.0f)), max(abs(y_1 - size_12 / 3.0f), abs(y_1 + size_12 / 3.0f))), max(abs(x_1), abs(y_1))) - size_12 / 2.0f;
}

fn asterisk_0( P_12 : vec2<f32>,  size_13 : f32) -> f32
{
    var _S28 : f32 = P_12.x;
    var _S29 : f32 = P_12.y;
    var _S30 : f32 = abs(0.70710676908493042f * (_S28 - _S29));
    var _S31 : f32 = abs(0.70710676908493042f * (_S28 + _S29));
    var _S32 : f32 = abs(_S28);
    var _S33 : f32 = abs(_S29);
    return min(min(max(_S30 - size_13 / 2.0f, _S31 - size_13 / 10.0f), max(_S31 - size_13 / 2.0f, _S30 - size_13 / 10.0f)), min(max(_S32 - size_13 / 2.0f, _S33 - size_13 / 10.0f), max(_S33 - size_13 / 2.0f, _S32 - size_13 / 10.0f)));
}

fn infinity_0( P_13 : vec2<f32>,  size_14 : f32) -> f32
{
    var _S34 : vec2<f32> = vec2<f32>(size_14);
    var _S35 : f32 = length(P_13 - vec2<f32>(0.21250000596046448f, 0.0f) * _S34);
    var _S36 : f32 = length(P_13 - vec2<f32>(-0.21250000596046448f, 0.0f) * _S34);
    return min(max(_S35 - size_14 / 3.5f, - (_S35 - size_14 / 7.5f)), max(_S36 - size_14 / 3.5f, - (_S36 - size_14 / 7.5f)));
}

fn pin_0( P_14 : vec2<f32>,  size_15 : f32) -> f32
{
    var _S37 : vec2<f32> = vec2<f32>(size_15);
    var _S38 : f32 = length(P_14 - vec2<f32>(0.0f, -0.15000000596046448f) * _S37);
    var _S39 : f32 = 2.0f * size_15;
    return max(min(_S38 - size_15 / 2.67499995231628418f, max(max(length(P_14 - vec2<f32>(1.49000000953674316f, -0.80000001192092896f) * _S37) - _S39, length(P_14 - vec2<f32>(-1.49000000953674316f, -0.80000001192092896f) * _S37) - _S39), - P_14.y)), - (_S38 - size_15 / 5.0f));
}

fn arrow_0( P_15 : vec2<f32>,  size_16 : f32) -> f32
{
    var _S40 : f32 = P_15.x;
    var _S41 : f32 = abs(P_15.y);
    return min(max(abs(_S40 - size_16 / 6.0f) - size_16 / 4.0f, _S41 - size_16 / 4.0f), max(0.75f * (abs(_S40) + _S41 - size_16 / 2.0f), max(abs(_S40 + size_16 / 2.0f), _S41) - size_16 / 2.0f));
}

fn ellipse_0( P_16 : vec2<f32>,  size_17 : f32) -> f32
{
    var _S42 : f32 = size_17 / 3.0f;
    var _S43 : f32 = size_17 / 2.0f;
    var ab_0 : vec2<f32> = vec2<f32>(_S42, _S43);
    var p_0 : vec2<f32> = abs(P_16);
    var ab_1 : vec2<f32>;
    var p_1 : vec2<f32>;
    if((p_0.x) > (p_0.y))
    {
        var _S44 : vec2<f32> = p_0.yx;
        ab_1 = vec2<f32>(_S43, _S42);
        p_1 = _S44;
    }
    else
    {
        ab_1 = ab_0;
        p_1 = p_0;
    }
    var _S45 : f32 = ab_1.y;
    var _S46 : f32 = ab_1.x;
    var l_0 : f32 = _S45 * _S45 - _S46 * _S46;
    var m_0 : f32 = _S46 * p_1.x / l_0;
    var _S47 : f32 = p_1.y;
    var n_0 : f32 = _S45 * _S47 / l_0;
    var m2_0 : f32 = m_0 * m_0;
    var n2_0 : f32 = n_0 * n_0;
    var c_0 : f32 = (m2_0 + n2_0 - 1.0f) / 3.0f;
    var c3_0 : f32 = c_0 * c_0 * c_0;
    var _S48 : f32 = m2_0 * n2_0;
    var q_0 : f32 = c3_0 + _S48 * 2.0f;
    var d_0 : f32 = c3_0 + _S48;
    var g_0 : f32 = m_0 + m_0 * n2_0;
    var co_0 : f32;
    if(d_0 < 0.0f)
    {
        var p_2 : f32 = acos(q_0 / c3_0) / 3.0f;
        var s_1 : f32 = cos(p_2);
        var t_0 : f32 = sin(p_2) * sqrt(3.0f);
        var _S49 : f32 = - c_0;
        var rx_0 : f32 = sqrt(_S49 * (s_1 + t_0 + 2.0f) + m2_0);
        var ry_0 : f32 = sqrt(_S49 * (s_1 - t_0 + 2.0f) + m2_0);
        co_0 = (ry_0 + f32(sign(l_0)) * rx_0 + abs(g_0) / (rx_0 * ry_0) - m_0) / 2.0f;
    }
    else
    {
        var h_0 : f32 = 2.0f * m_0 * n_0 * sqrt(d_0);
        var _S50 : f32 = q_0 + h_0;
        var s_2 : f32 = f32(sign(_S50)) * pow(abs(_S50), 0.3333333432674408f);
        var _S51 : f32 = q_0 - h_0;
        var u_0 : f32 = f32(sign(_S51)) * pow(abs(_S51), 0.3333333432674408f);
        var rx_1 : f32 = - s_2 - u_0 - c_0 * 4.0f + 2.0f * m2_0;
        var ry_1 : f32 = (s_2 - u_0) * sqrt(3.0f);
        var rm_0 : f32 = sqrt(rx_1 * rx_1 + ry_1 * ry_1);
        co_0 = (ry_1 / sqrt(rm_0 - rx_1) + 2.0f * g_0 / rm_0 - m_0) / 2.0f;
    }
    var _S52 : f32 = _S45 * sqrt(1.0f - co_0 * co_0);
    return length(vec2<f32>(_S46 * co_0, _S52) - p_1) * f32(sign(_S47 - _S52));
}

fn ellipse_fast_0( P_17 : vec2<f32>,  size_18 : f32) -> f32
{
    var f_0 : f32 = length(P_17 * vec2<f32>(1.0f, 3.0f));
    return f_0 * (f_0 - 0.89999997615814209f) / length(P_17 * vec2<f32>(1.0f, 9.0f));
}

fn filled_0( distance_0 : f32,  linewidth_1 : f32,  antialias_0 : f32,  fill_0 : vec4<f32>) -> vec4<f32>
{
    var border_distance_0 : f32 = abs(distance_0) - (linewidth_1 / 2.0f - antialias_0);
    var alpha_0 : f32 = border_distance_0 / antialias_0;
    var alpha_1 : f32 = exp(- alpha_0 * alpha_0);
    var frag_color_0 : vec4<f32>;
    if(border_distance_0 < 0.0f)
    {
        frag_color_0 = fill_0;
    }
    else
    {
        if(distance_0 < 0.0f)
        {
            frag_color_0 = fill_0;
        }
        else
        {
            frag_color_0 = vec4<f32>(fill_0.xyz, alpha_1 * fill_0.w);
        }
    }
    return frag_color_0;
}

fn stroke_0( distance_1 : f32,  linewidth_2 : f32,  antialias_1 : f32,  stroke_1 : vec4<f32>) -> vec4<f32>
{
    var border_distance_1 : f32 = abs(distance_1) - (linewidth_2 / 2.0f - antialias_1);
    var alpha_2 : f32 = border_distance_1 / antialias_1;
    var alpha_3 : f32 = exp(- alpha_2 * alpha_2);
    var frag_color_1 : vec4<f32>;
    if(border_distance_1 < 0.0f)
    {
        frag_color_1 = stroke_1;
    }
    else
    {
        frag_color_1 = vec4<f32>(stroke_1.xyz, stroke_1.w * alpha_3);
    }
    return frag_color_1;
}

fn outline_0( distance_2 : f32,  linewidth_3 : f32,  antialias_2 : f32,  stroke_2 : vec4<f32>,  fill_1 : vec4<f32>) -> vec4<f32>
{
    var border_distance_2 : f32 = abs(distance_2) - (linewidth_3 / 2.0f - antialias_2);
    var alpha_4 : f32 = border_distance_2 / antialias_2;
    var alpha_5 : f32 = exp(- alpha_4 * alpha_4);
    var frag_color_2 : vec4<f32>;
    if(border_distance_2 < 0.0f)
    {
        frag_color_2 = stroke_2;
    }
    else
    {
        if(distance_2 < 0.0f)
        {
            frag_color_2 = mix(fill_1, stroke_2, vec4<f32>(sqrt(alpha_5)));
        }
        else
        {
            frag_color_2 = vec4<f32>(stroke_2.xyz, stroke_2.w * alpha_5);
        }
    }
    return frag_color_2;
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
fn main_ps( _S53 : pixelInput_0, @builtin(position) pos_0 : vec4<f32>) -> pixelOutput_0
{
    const _S54 : vec4<f32> = vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f);
    var _S55 : vec2<f32> = _S53.uv_0.xy - vec2<f32>(0.5f);
    var P_18 : vec2<f32> = _S55;
    P_18[i32(1)] = - _S55.y;
    P_18 = vec2<f32>(PS_CONSTANT_BUFFER_0.rotation_0.x * P_18.x - PS_CONSTANT_BUFFER_0.rotation_0.y * P_18.y, PS_CONSTANT_BUFFER_0.rotation_0.y * P_18.x + PS_CONSTANT_BUFFER_0.rotation_0.x * P_18.y);
    var antialias_3 : f32 = PS_CONSTANT_BUFFER_0.antialiasing_0;
    var point_size_0 : f32 = 1.41421353816986084f * PS_CONSTANT_BUFFER_0.size_0 + 2.0f * (PS_CONSTANT_BUFFER_0.linewidth_0 + 1.5f * PS_CONSTANT_BUFFER_0.antialiasing_0);
    var distance_3 : f32;
    if((PS_CONSTANT_BUFFER_0.type_0) == 0.0f)
    {
        distance_3 = disc_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
    }
    else
    {
        if((PS_CONSTANT_BUFFER_0.type_0) == 1.0f)
        {
            distance_3 = square_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
        }
        else
        {
            if((PS_CONSTANT_BUFFER_0.type_0) == 2.0f)
            {
                distance_3 = triangle2_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
            }
            else
            {
                if((PS_CONSTANT_BUFFER_0.type_0) == 3.0f)
                {
                    distance_3 = diamond_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                }
                else
                {
                    if((PS_CONSTANT_BUFFER_0.type_0) == 4.0f)
                    {
                        distance_3 = heart_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                    }
                    else
                    {
                        if((PS_CONSTANT_BUFFER_0.type_0) == 5.0f)
                        {
                            distance_3 = spade_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                        }
                        else
                        {
                            if((PS_CONSTANT_BUFFER_0.type_0) == 6.0f)
                            {
                                distance_3 = club_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                            }
                            else
                            {
                                if((PS_CONSTANT_BUFFER_0.type_0) == 7.0f)
                                {
                                    distance_3 = chevron_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                }
                                else
                                {
                                    if((PS_CONSTANT_BUFFER_0.type_0) == 8.0f)
                                    {
                                        distance_3 = clover_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                    }
                                    else
                                    {
                                        if((PS_CONSTANT_BUFFER_0.type_0) == 9.0f)
                                        {
                                            distance_3 = ring_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                        }
                                        else
                                        {
                                            if((PS_CONSTANT_BUFFER_0.type_0) == 10.0f)
                                            {
                                                distance_3 = tag_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                            }
                                            else
                                            {
                                                if((PS_CONSTANT_BUFFER_0.type_0) == 11.0f)
                                                {
                                                    distance_3 = cross_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                                }
                                                else
                                                {
                                                    if((PS_CONSTANT_BUFFER_0.type_0) == 12.0f)
                                                    {
                                                        distance_3 = asterisk_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                                    }
                                                    else
                                                    {
                                                        if((PS_CONSTANT_BUFFER_0.type_0) == 13.0f)
                                                        {
                                                            distance_3 = infinity_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                                        }
                                                        else
                                                        {
                                                            if((PS_CONSTANT_BUFFER_0.type_0) == 14.0f)
                                                            {
                                                                distance_3 = pin_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                                            }
                                                            else
                                                            {
                                                                if((PS_CONSTANT_BUFFER_0.type_0) == 15.0f)
                                                                {
                                                                    distance_3 = arrow_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                                                }
                                                                else
                                                                {
                                                                    if((PS_CONSTANT_BUFFER_0.type_0) == 16.0f)
                                                                    {
                                                                        distance_3 = ellipse_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                                                    }
                                                                    else
                                                                    {
                                                                        if((PS_CONSTANT_BUFFER_0.type_0) == 17.0f)
                                                                        {
                                                                            distance_3 = ellipse_fast_0(P_18 * vec2<f32>(point_size_0), PS_CONSTANT_BUFFER_0.size_0);
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    var col_out_0 : vec4<f32>;
    if((PS_CONSTANT_BUFFER_0.draw_type_0) == 0.0f)
    {
        col_out_0 = filled_0(distance_3, PS_CONSTANT_BUFFER_0.linewidth_0, antialias_3, PS_CONSTANT_BUFFER_0.fg_color_0);
    }
    else
    {
        if((PS_CONSTANT_BUFFER_0.draw_type_0) == 1.0f)
        {
            col_out_0 = stroke_0(distance_3, PS_CONSTANT_BUFFER_0.linewidth_0, antialias_3, PS_CONSTANT_BUFFER_0.fg_color_0);
        }
        else
        {
            if((PS_CONSTANT_BUFFER_0.draw_type_0) == 2.0f)
            {
                col_out_0 = outline_0(distance_3, PS_CONSTANT_BUFFER_0.linewidth_0, antialias_3, PS_CONSTANT_BUFFER_0.fg_color_0, PS_CONSTANT_BUFFER_0.bg_color_0);
            }
            else
            {
                if((PS_CONSTANT_BUFFER_0.draw_type_0) == 3.0f)
                {
                    col_out_0 = vec4<f32>(vec3<f32>(pow(abs(distance_3), 0.45454543828964233f)), 1.0f);
                }
                else
                {
                    if((PS_CONSTANT_BUFFER_0.draw_type_0) == 4.0f)
                    {
                        var _S56 : vec4<f32> = PS_CONSTANT_BUFFER_0.fg_color_0;
                        var _S57 : vec4<f32> = PS_CONSTANT_BUFFER_0.bg_color_0;
                        if(distance_3 > 0.0f)
                        {
                            col_out_0 = vec4<f32>(1.0f);
                        }
                        else
                        {
                            col_out_0 = vec4<f32>(0.0f);
                        }
                        col_out_0 = mix(_S56, _S57, col_out_0);
                    }
                    else
                    {
                        col_out_0 = _S54;
                    }
                }
            }
        }
    }
    var _S58 : pixelOutput_0 = pixelOutput_0( col_out_0 );
    return _S58;
}

