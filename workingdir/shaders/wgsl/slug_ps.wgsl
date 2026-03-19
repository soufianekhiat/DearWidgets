// Slug GPU Font Rendering - Pixel Shader (WGSL)
// Based on the Slug Algorithm by Eric Lengyel (public domain, 2026)

@binding(0) @group(1) var curveTexture : texture_2d<f32>;
@binding(1) @group(1) var bandTexture  : texture_2d<f32>;

struct PS_INPUT {
    @builtin(position) pos       : vec4<f32>,
    @location(0)        color    : vec4<f32>,
    @location(1)        texcoord : vec2<f32>,
    @location(2) @interpolate(flat) banding : vec4<f32>,
    @location(3) @interpolate(flat) glyph   : vec4<i32>,
};

const SLUG_BAND_TEXTURE_WIDTH : i32 = 4096;
const SLUG_LOG_BAND_TEX_WIDTH : u32 = 12u;

fn BandLoad(coord : vec2<i32>) -> vec2<i32> {
    let raw = textureLoad(bandTexture, coord, 0);
    return vec2<i32>(i32(raw.r + 0.5), i32(raw.g + 0.5));
}

fn CurveLoad(coord : vec2<i32>) -> vec4<f32> {
    return textureLoad(curveTexture, coord, 0);
}

fn CalcBandLoc(glyphLoc : vec2<i32>, offset : i32) -> vec2<i32> {
    var loc = vec2<i32>(glyphLoc.x + offset, glyphLoc.y);
    loc.y += loc.x >> i32(SLUG_LOG_BAND_TEX_WIDTH);
    loc.x &= SLUG_BAND_TEXTURE_WIDTH - 1;
    return loc;
}

fn CalcRootCode(y1 : f32, y2 : f32, y3 : f32) -> u32 {
    let i1 : u32 = bitcast<u32>(y1) >> 31u;
    let i2 : u32 = bitcast<u32>(y2) >> 30u;
    let i3 : u32 = bitcast<u32>(y3) >> 29u;
    var shift : u32 = (i2 & 2u) | (i1 & ~2u);
    shift = (i3 & 4u) | (shift & ~4u);
    return (0x2E74u >> shift) & 0x0101u;
}

fn SolveHorizPoly(p12 : vec4<f32>, p3 : vec2<f32>) -> vec2<f32> {
    let a  = p12.xy - p12.zw * 2.0 + p3;
    let b  = p12.xy - p12.zw;
    let ra = 1.0 / a.y;
    let rb = 0.5 / b.y;
    let d  = sqrt(max(b.y * b.y - a.y * p12.y, 0.0));
    var t1 = (b.y - d) * ra;
    var t2 = (b.y + d) * ra;
    if (abs(a.y) < 1.0 / 65536.0) { t1 = p12.y * rb; t2 = t1; }
    return vec2<f32>((a.x * t1 - b.x * 2.0) * t1 + p12.x,
                     (a.x * t2 - b.x * 2.0) * t2 + p12.x);
}

fn SolveVertPoly(p12 : vec4<f32>, p3 : vec2<f32>) -> vec2<f32> {
    let a  = p12.xy - p12.zw * 2.0 + p3;
    let b  = p12.xy - p12.zw;
    let ra = 1.0 / a.x;
    let rb = 0.5 / b.x;
    let d  = sqrt(max(b.x * b.x - a.x * p12.x, 0.0));
    var t1 = (b.x - d) * ra;
    var t2 = (b.x + d) * ra;
    if (abs(a.x) < 1.0 / 65536.0) { t1 = p12.x * rb; t2 = t1; }
    return vec2<f32>((a.y * t1 - b.y * 2.0) * t1 + p12.y,
                     (a.y * t2 - b.y * 2.0) * t2 + p12.y);
}

@fragment
fn main_ps(input : PS_INPUT) -> @location(0) vec4<f32> {
    let renderCoord = input.texcoord;
    let glyphLoc    = input.glyph.xy;
    var bandMax     = input.glyph.zw;
    bandMax.y       = bandMax.y & 0x00FF;

    let emsPerPixel = vec2<f32>(abs(dpdx(renderCoord.x)) + abs(dpdy(renderCoord.x)),
                                abs(dpdx(renderCoord.y)) + abs(dpdy(renderCoord.y)));
    let pixelsPerEm = 1.0 / max(emsPerPixel, vec2<f32>(1e-10));

    let bandIndex = clamp(vec2<i32>(renderCoord * input.banding.xy + input.banding.zw),
                          vec2<i32>(0, 0), bandMax);

    var xcov : f32 = 0.0; var xwgt : f32 = 0.0;
    let hData = BandLoad(vec2<i32>(glyphLoc.x + bandIndex.y, glyphLoc.y));
    let hLoc  = CalcBandLoc(glyphLoc, hData.y);

    for (var ci : i32 = 0; ci < hData.x; ci++) {
        let ref = BandLoad(vec2<i32>(hLoc.x + ci, hLoc.y));
        let cl  = vec2<i32>(ref.x, ref.y);
        let p12 = CurveLoad(cl)                          - vec4<f32>(renderCoord, renderCoord);
        let p3  = CurveLoad(vec2<i32>(cl.x + 1, cl.y)).xy - renderCoord;
        if (max(max(p12.x, p12.z), p3.x) * pixelsPerEm.x < -0.5) { break; }
        let code = CalcRootCode(p12.y, p12.w, p3.y);
        if (code != 0u) {
            let r = SolveHorizPoly(p12, p3) * pixelsPerEm.x;
            if ((code & 1u) != 0u) { xcov += clamp(r.x+0.5,0.,1.); xwgt = max(xwgt, clamp(1.-abs(r.x)*2.,0.,1.)); }
            if (code > 1u)         { xcov -= clamp(r.y+0.5,0.,1.); xwgt = max(xwgt, clamp(1.-abs(r.y)*2.,0.,1.)); }
        }
    }

    var ycov : f32 = 0.0; var ywgt : f32 = 0.0;
    let vData = BandLoad(vec2<i32>(glyphLoc.x + bandMax.y + 1 + bandIndex.x, glyphLoc.y));
    let vLoc  = CalcBandLoc(glyphLoc, vData.y);

    for (var ci : i32 = 0; ci < vData.x; ci++) {
        let ref = BandLoad(vec2<i32>(vLoc.x + ci, vLoc.y));
        let cl  = vec2<i32>(ref.x, ref.y);
        let p12 = CurveLoad(cl)                          - vec4<f32>(renderCoord, renderCoord);
        let p3  = CurveLoad(vec2<i32>(cl.x + 1, cl.y)).xy - renderCoord;
        if (max(max(p12.y, p12.w), p3.y) * pixelsPerEm.y < -0.5) { break; }
        let code = CalcRootCode(p12.x, p12.z, p3.x);
        if (code != 0u) {
            let r = SolveVertPoly(p12, p3) * pixelsPerEm.y;
            if ((code & 1u) != 0u) { ycov -= clamp(r.x+0.5,0.,1.); ywgt = max(ywgt, clamp(1.-abs(r.x)*2.,0.,1.)); }
            if (code > 1u)         { ycov += clamp(r.y+0.5,0.,1.); ywgt = max(ywgt, clamp(1.-abs(r.y)*2.,0.,1.)); }
        }
    }

    let wsum : f32 = max(xwgt + ywgt, 1.0 / 65536.0);
    let cov  : f32 = max(abs(xcov * xwgt + ycov * ywgt) / wsum, min(abs(xcov), abs(ycov)));
    return input.color * clamp(cov, 0.0, 1.0);
}
