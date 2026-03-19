// Slug GPU Font Rendering - Vertex Shader (WGSL)
// Based on the Slug Algorithm by Eric Lengyel (public domain, 2026)

struct SlugVSConstants {
    SlugProjMtx : mat4x4<f32>,
    SlugViewport : vec2<f32>,
    _pad : vec2<f32>,
};

@binding(0) @group(0) var<uniform> cb : SlugVSConstants;

struct VS_INPUT {
    @location(0) pos : vec4<f32>,
    @location(1) tex : vec4<f32>,
    @location(2) jac : vec4<f32>,
    @location(3) bnd : vec4<f32>,
    @location(4) col : vec4<f32>,
};

struct PS_INPUT {
    @builtin(position) pos       : vec4<f32>,
    @location(0)        color    : vec4<f32>,
    @location(1)        texcoord : vec2<f32>,
    @location(2) @interpolate(flat) banding : vec4<f32>,
    @location(3) @interpolate(flat) glyph   : vec4<i32>,
};

@vertex
fn main_vs(input : VS_INPUT) -> PS_INPUT {
    var output : PS_INPUT;

    // Unpack glyph info from bit-packed floats
    let gx : u32 = bitcast<u32>(input.tex.z);
    let gy : u32 = bitcast<u32>(input.tex.w);
    output.glyph   = vec4<i32>(i32(gx & 0xFFFFu), i32(gx >> 16u),
                                i32(gy & 0xFFFFu), i32(gy >> 16u));
    output.banding = input.bnd;
    output.color   = input.col;

    let n : vec2<f32> = normalize(input.pos.zw);
    let M : mat4x4<f32> = cb.SlugProjMtx;

    // Extract rows 0, 1, 3 from column-major matrix
    let m0 = vec4<f32>(M[0][0], M[1][0], M[2][0], M[3][0]);
    let m1 = vec4<f32>(M[0][1], M[1][1], M[2][1], M[3][1]);
    let m3 = vec4<f32>(M[0][3], M[1][3], M[2][3], M[3][3]);

    let s  : f32 = dot(m3.xy, input.pos.xy) + m3.w;
    let t  : f32 = dot(m3.xy, n);
    let u  : f32 = (s * dot(m0.xy, n) - t * (dot(m0.xy, input.pos.xy) + m0.w)) * cb.SlugViewport.x;
    let v  : f32 = (s * dot(m1.xy, n) - t * (dot(m1.xy, input.pos.xy) + m1.w)) * cb.SlugViewport.y;

    let s2   : f32 = s * s;
    let st   : f32 = s * t;
    let uv2  : f32 = u * u + v * v;
    let denom: f32 = uv2 - st * st;
    var d    : vec2<f32>;
    if (abs(denom) > 1e-10) {
        d = input.pos.zw * (s2 * (st + sqrt(max(uv2, 0.0))) / denom);
    } else {
        d = input.pos.zw;
    }

    let p = input.pos.xy + d;
    output.texcoord = vec2<f32>(input.tex.x + dot(d, input.jac.xy),
                                input.tex.y + dot(d, input.jac.zw));
    output.pos = M * vec4<f32>(p, 0.0, 1.0);
    return output;
}
