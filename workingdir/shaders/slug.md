# Slug GPU Font Rendering — Implementation Reference

Based on the algorithm by Eric Lengyel (2017).
Our implementation lives in `src/api/dear_widgets.cpp` (CPU side) and `workingdir/shaders/hlsl_src/slug.hlsl` (GPU side).

---

## Overview

Slug renders TrueType/OpenType glyphs entirely on the GPU from their original Bézier outlines. There is no pre-rasterization and no SDF atlas. Coverage is computed analytically per pixel by casting two rays (leftward + upward) and counting signed winding crossings. A band acceleration structure limits how many curves each pixel needs to test.

---

## Coordinate Systems

| Space | Description |
|---|---|
| **Font units** | Raw integer units from the TTF file. |
| **Em-space** | Font units × `emScale` where `emScale = 1 / unitsPerEm`. 1.0 = one full em height. All curve data and band transforms live here. |
| **Screen-space** | Pixels on screen. The Jacobian maps screen offsets to em offsets. |

`emScale` is computed once per font:
`atlas->emScale = stbtt_ScaleForMappingEmToPixels(&stbFont, 1.0f)` — `dear_widgets.cpp:3643`

---

## Data Textures

Both textures are **RGBA32F**, width = 4096 (`SLUG_TEX_WIDTH`), height grows dynamically starting at 16 rows (`SLUG_TEX_INIT_H`). Integer values are stored as exact floats (integers up to 2^24 are representable exactly in float32).

### Curve Texture (`t0` — `curveTexture`)

One curve = **2 consecutive texels on the same row** (allocation ensures they never straddle a row boundary):

```
texel[cx,   cy] = (p1.x, p1.y, p2.x, p2.y)
texel[cx+1, cy] = (p3.x, p3.y, p4.x, p4.y)
```

- Quadratic: p1=start, p2=control, p3=end. p4 = (0,0), unused.
- Cubic:     p1=start, p2=ctrl1,   p3=ctrl2, p4=end.

The cubic flag is **not** stored in the curve texture — it lives in the band ref (bit 12 of the X coordinate).

Allocation: `SlugAllocOneCurve` — `dear_widgets.cpp:3236`
Write: `SlugCurveWrite4f` — `dear_widgets.cpp:3218`

### Band Texture (`t1` — `bandTexture`)

Stores band headers and curve ref lists as integer-valued floats in `.rg` (`.ba` is always 0).

```
header: (count, offset, 0, 0)   -- count = number of curves in this band
                                 -- offset = start of ref list, relative to glyphLoc
ref:    (curveTexX | flags, curveTexY, 0, 0)
```

The `curveTexX` field of a ref encodes a flag in bit 12:
- bits 0–11: X coordinate of the curve's first texel in the curve texture (max 4095)
- bit 12 (`0x1000`): 1 = cubic Bézier, 0 = quadratic

Allocation: `SlugBandWrite2f` — `dear_widgets.cpp:3248`
Row-wrap helper: `SlugBandCalcLoc` — `dear_widgets.cpp:3264`

---

## Band Acceleration Structure

### Purpose

Rather than testing every curve for every pixel, the glyph bounding box is divided into a grid of bands. Each pixel only tests the curves that overlap its specific band. Curves within each band are sorted so the pixel shader can break early.

### Layout

For each glyph, two independent grids are built:

- **Horizontal bands** — NBY strips partitioning Y. Used for the leftward ray cast (measures horizontal crossings → `xcov`). Curves sorted by **descending maxX**: the shader breaks as soon as `maxX < pixel.x − 0.5px`.
- **Vertical bands** — NBX strips partitioning X. Used for the upward ray cast (measures vertical crossings → `ycov`). Curves sorted by **descending maxY**.

### Adaptive Band Count

`dear_widgets.cpp:3401`

Band counts scale with glyph complexity and aspect ratio:

```
sqrtC = sqrt(curve_count)
sqA   = sqrt(glyphW / glyphH)            -- aspect ratio weight
NBX   = clamp(ceil(sqrtC × sqA),  4, 64)
NBY   = clamp(ceil(sqrtC / sqA),  4, 64)
```

NBY is capped at 255 (packed into 8 bits in the vertex data). NBX is capped at 64 for safety.

The reference implementation uses a fixed 8×8 grid. Adaptive counts give more bands to complex glyphs and respect aspect ratio to avoid under-sampling in one axis.

### Curve-to-Band Assignment

`dear_widgets.cpp:3427`

A curve is assigned to every band whose range overlaps the curve's **control-point bounding box**. This is conservative: the actual curve lies inside the convex hull of its control points, so no curve is ever under-assigned to bands.

```
hyMin = int((cMinY − minY) × bsy)     // truncation = floor for positive values
hyMax = int((cMaxY − minY) × bsy)
assign curve to hBand[hyMin .. hyMax]
```

For cubics, all four control points (p1–p4) are included in the bounding box.

### Band Texture Layout per Glyph

```
glyphLocX + 0          : horizontal band 0 header
glyphLocX + 1          : horizontal band 1 header
...
glyphLocX + NBY−1      : horizontal band NBY−1 header
glyphLocX + NBY        : vertical band 0 header
glyphLocX + NBY + 1    : vertical band 1 header
...
glyphLocX + NBY+NBX−1  : vertical band NBX−1 header
[curve refs follow, packed contiguously, wrapped across rows via CalcBandLoc]
```

All headers are forced onto a single row (`dear_widgets.cpp:3510`). Curve refs overflow into subsequent rows as needed.

---

## Vertex Format

`SlugVertex` — 80 bytes = 5 × float4 — `dear_widgets.cpp:3715`

| Attribute | Semantic | Content |
|---|---|---|
| `pos` | POSITION | `xy` = screen position (undilated), `zw` = outward vertex normal |
| `tex` | TEXCOORD0 | `xy` = em-space UV (undilated), `zw` = packed glyph data (see below) |
| `jac` | TEXCOORD1 | Inverse Jacobian `(1/sz, 0, 0, −1/sz)`: screen-delta → em-delta |
| `bnd` | TEXCOORD2 | Band transform `(scaleX, scaleY, offsetX, offsetY)` |
| `col` | COLOR0 | RGBA vertex color (float) |

### Packed Glyph Data (`tex.zw`, bit-cast as uint)

```
tex.z : bits  0–15 = bandTexX (glyph header location X in band texture)
        bits 16–31 = bandTexY (glyph header location Y in band texture)

tex.w : bits  0–7  = bandMaxX (= NBX − 1)
        bits 16–23 = bandMaxY (= NBY − 1)
        bit  28    = even-odd fill flag
```

### Band Transform

Converts em-space coordinate to band index:

```
bandScaleX  = NBX / glyphW       bandScaleY  = NBY / glyphH
bandOffsetX = −minX × scaleX     bandOffsetY = −minY × scaleY
```

In the shader: `bandIndex = int2(renderCoord × scale + offset)` → integer band indices.

---

## Vertex Shader

`main_vs` / `SlugDilate` — `slug.hlsl:87`

Each vertex is **dilated** 0.5 screen pixels outward along its corner normal. This expands the quad slightly beyond the glyph's em-space bounds so that anti-aliasing at the glyph edge has enough coverage data.

Dilation is computed in screen space using the full MVP matrix, then mapped back to em-space via the inverse Jacobian to update `texcoord`. This handles rotation, scale, and perspective correctly.

The dilated em-space `texcoord` is what the pixel shader receives as `renderCoord`.

---

## Pixel Shader

`SlugRender` — `slug.hlsl:272`

### 1. Determine band indices

```hlsl
int2 bandIndex = clamp(int2(renderCoord * banding.xy + banding.zw), 0, bandMax);
```

### 2. Horizontal band — leftward ray

Load `hData = (count, offset)` for `bandIndex.y`.
For each curve ref (early-exit when `maxX × pxPerEm < −0.5`):

**Quadratic** (`slug.hlsl:307`):
- `CalcRootCode(p1.y, p2.y, p3.y)` — classifies which of the two parametric roots is valid based on sign changes of the Y component relative to the pixel. Encodes a 2-bit result via a lookup on the sign bits.
- `SolveHorizPoly` — solves the quadratic for t where Y(t) = 0, returns x(t) for each valid root.
- Each root contributes `±saturate(x × pxPerEm + 0.5)` to `xcov`.

**Cubic** (`slug.hlsl:326`):
- Build polynomial coefficients for Y(t): `ay·t³ + by·t² + cy·t + dy`
- `SolveCubicRoots` — trigonometric method (3 real roots when discriminant ≥ 0), Cardano (1 real root), with quadratic/linear fallback.
- For each root t ∈ [0,1): evaluate x(t), use `dy/dt` sign for winding direction, call `ApplyCubicRoot`.

### 3. Vertical band — upward ray

Load `vData = (count, offset)` for `bandIndex.x` (header at `glyphLoc.x + NBY + bandIndex.x`).
Same structure as horizontal, but solves X(t) = 0 instead of Y(t) = 0.
Winding convention is flipped: `sign_pos = −1.0` so that `dx/dt > 0` subtracts from `ycov`.

### 4. Combine coverage

`CalcCoverage` — `slug.hlsl:247`

```
coverage = max(
    abs(xcov × xwgt + ycov × ywgt) / (xwgt + ywgt),   // weighted blend
    min(|xcov|, |ycov|)                                  // fallback at corners
)
coverage = saturate(coverage)   // or even-odd mod if flag set
```

`xwgt` / `ywgt` accumulate `saturate(1 − |x| × 2)` — highest near the curve boundary, zero beyond 0.5px. They act as a confidence weight: a ray that barely grazes a curve contributes little to the blend.

---

## Row-Wrap Fix (Deviation from Reference)

`slug.hlsl:289` and `slug.hlsl:364`

The reference implementation reads curve refs as:
```hlsl
int2 hLoc = CalcBandLoc(glyphLoc, uint(hData.y));
int2 ref  = BandLoad(int2(hLoc.x + ci, hLoc.y));   // reference
```

This assumes each band's ref list fits entirely within one texture row. The reference allocator guarantees this.

Our allocator packs all glyphs contiguously into a shared cursor without per-band row alignment. A ref list that crosses the row boundary at x = 4096 would cause `hLoc.x + ci` to exceed 4095; GPU `Load()` returns zero for out-of-bounds coordinates, making those curves invisible and producing horizontal/vertical stripe artifacts.

**Our fix** — call `CalcBandLoc` per ref:
```hlsl
int2 ref = BandLoad(CalcBandLoc(glyphLoc, uint(hData.y) + uint(ci)));
```
`CalcBandLoc` costs 3 ALU ops (`add`, `>> 12`, `& 4095`) and always produces a valid wrapped coordinate.

---

## CalcBandLoc

`slug.hlsl:140` / `SlugBandCalcLoc` — `dear_widgets.cpp:3264`

Converts a (glyphLoc, relative-offset) pair into an absolute texture coordinate with row wrapping:

```hlsl
int2 CalcBandLoc(int2 glyphLoc, uint offset)
{
    int2 loc = int2(glyphLoc.x + int(offset), glyphLoc.y);
    loc.y   += loc.x >> 12;    // integer divide by 4096
    loc.x   &= 4095;           // integer mod 4096
    return loc;
}
```

---

## Key Constants

| Constant | Value | Location |
|---|---|---|
| `SLUG_TEX_WIDTH` | 4096 | `dear_widgets.cpp:3147` |
| `SLUG_LOG_TEX_W` | 12 (log₂ 4096) | `dear_widgets.cpp:3148` / `slug.hlsl:127` |
| `SLUG_TEX_INIT_H` | 16 rows | `dear_widgets.cpp:3151` |
| `pad` (glyph bbox) | 0.01 em | `dear_widgets.cpp:3299` |
| NBX/NBY min | 4 | `dear_widgets.cpp:3412` |
| NBX max | 64 | `dear_widgets.cpp:3412` |
| NBY max | 255 | `dear_widgets.cpp:3403` |

---

## File Map

| File | Role |
|---|---|
| `src/api/dear_widgets.cpp` | CPU: atlas build, curve/band packing, vertex emission, GPU upload |
| `workingdir/shaders/hlsl_src/slug.hlsl` | GPU: vertex dilation, band lookup, coverage computation |
| `workingdir/shaders/glsl/slug_ps.glsl` | GLSL pixel shader (generated by slangc) |
| `workingdir/shaders/glsl/slug_vs.glsl` | GLSL vertex shader (generated by slangc) |
| `workingdir/shaders/msl/slug_ps.msl` | Metal pixel shader (generated by slangc) |
| `workingdir/shaders/msl/slug_vs.msl` | Metal vertex shader (generated by slangc) |
| `workingdir/shaders/wgsl/slug_ps.wgsl` | WGSL pixel shader (generated by slangc) |
| `workingdir/shaders/wgsl/slug_vs.wgsl` | WGSL vertex shader (generated by slangc) |
