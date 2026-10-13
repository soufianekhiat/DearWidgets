# DrawList Primitives

Functions that draw shapes, gradients, chromatic diagrams, markers, polylines
and annotations directly into an `ImDrawList`. All are in `namespace ImWidgets`;
all types are at global scope.

Unless a section says otherwise, everything here is **ImGui only** -- see
[compatibility.md](../compatibility.md).

---

## Shapes

`ImWidgetsShape` is a triangle mesh (`vertices`, `triangles`, `bb`). The usual
flow is *generate geometry -> fill with colour -> draw*.

### Geometry generation

```cpp
void GenShapeRect         ( ImWidgetsShape& shape, ImRect const& r );
void GenShapeCircle       ( ImWidgetsShape& shape, ImVec2 center, float radius, int side_count );
void GenShapeCircleArc    ( ImWidgetsShape& shape, ImVec2 center, float radius,
                            float angle_min, float angle_max, int side_count );
void GenShapeRegularNGon  ( ImWidgetsShape& shape, ImVec2 center, float radius, int side_count );
void GenShapeSquircle     ( ImWidgetsShape& shape, ImVec2 center, float radius,
                            int side_count, float n = 4.0f );
void GenShapeSuperellipse ( ImWidgetsShape& shape, ImVec2 center,
                            float rx, float ry, float nx, float ny, int sides );
void GenShapeConcavePoly  ( ImWidgetsShape& shape, ImVec2 const* pts, int pts_count,
                            ImVec2 origin = ImVec2( 0.0f, 0.0f ) );
void GenShapeTriangleSubdiv( ImWidgetsShape& shape, ImVec2 A, ImVec2 B, ImVec2 C, int subdivisions );
```

Subdivided shapes, for smooth per-vertex procedural colour:

```cpp
void GenShapeVerticalBand  ( ImWidgetsShape& shape, ImRect const& r, int divisions );
void GenShapeHorizontalBand( ImWidgetsShape& shape, ImRect const& r, int divisions );
void GenShapeRectGrid      ( ImWidgetsShape& shape, ImRect const& r, int divisionsX, int divisionsY );
void GenShapeDiscRings     ( ImWidgetsShape& shape, ImVec2 center, float radius,
                             int numSectors, int numRings );
void GenShapeAnnulus       ( ImWidgetsShape& shape, ImVec2 center,
                             float innerRadius, float outerRadius, int numSectors );
void GenShapeAnnulusRings  ( ImWidgetsShape& shape, ImVec2 center,
                             float innerRadius, float outerRadius, int numSectors, int numRings );
void GenShapeAnnulusArc    ( ImWidgetsShape& shape, ImVec2 center,
                             float innerRadius, float outerRadius,
                             float startAngle, float sweepAngle, int divisions );
```

### Shape utilities

```cpp
void ShapeTessellationUniform      ( ImWidgetsShape& shape );
void ShapeTranslate               ( ImWidgetsShape& shape, ImVec2 t );
void ShapeSetBound                ( ImWidgetsShape& shape );        // recompute shape.bb
void ShapeSetDefaultUV            ( ImWidgetsShape& shape );
void ShapeSetDefaultUVCol         ( ImWidgetsShape& shape );
void ShapeSetDefaultBoundUV       ( ImWidgetsShape& shape );        // UV from the bounding box
void ShapeSetDefaultBoundUVWhiteCol( ImWidgetsShape& shape );
void ShapeSetDefaultWhiteCol      ( ImWidgetsShape& shape );
void ClearShapeCache              ();                               // drop the DEAR_WIDGETS_SHAPE_CACHING cache
```

### Solid and procedural fills

```cpp
void ShapeFillSolidColor        ( ImWidgetsShape& shape, ImU32 col );
void ShapeFillProceduralColor1D ( ImWidgetsShape& shape, ImWidgetsColor1DCallback func,
                                  void* pUserData, bool sample_v );
void ShapeFillProceduralColor2D ( ImWidgetsShape& shape, ImWidgetsColor2DCallback func,
                                  void* pUserData );
```
`sample_v` picks which UV axis drives the 1D callback. A colour-space-aware
overload of `ShapeFillProceduralColor1D` taking `ImWidgetsColorScalar` is in
[color.md](color.md#space-aware-drawing-overloads).

### Gradient fills on shapes

```cpp
void ShapeLinearGradient ( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end,
                           ImU32 col0, ImU32 col1,
                           ImWidgetsGradientInterp interp = ImWidgetsGradientInterp_sRGB );
void ShapeRadialGradient ( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end,
                           ImU32 col0, ImU32 col1,
                           ImWidgetsGradientInterp interp = ImWidgetsGradientInterp_sRGB );
void ShapeDiamondGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end,
                           ImU32 col0, ImU32 col1,
                           ImWidgetsGradientInterp interp = ImWidgetsGradientInterp_sRGB );
```

`interp` selects the interpolation space: `ImWidgetsGradientInterp_sRGB`,
`_LinearSRGB`, `_OkLab`, `_OkLCH` or `_HSV`. The colour is written per vertex, so
the mesh must be dense enough for the gradient you want -- use the subdivided
`GenShape*` generators above.

> **Changed in 2.0.0.** The 15 per-space spellings
> (`ShapeSRGBLinearGradient`, `ShapeOkLabRadialGradient`,
> `ShapeHSVDiamondGradient`, ...) are gone; so are the function-pointer forms
> `Shape{Linear,Radial,Diamond}GradientGeneric`, which moved to
> `dear_widgets_internal.h`.
>
> `ShapeOkLabRadialGradient(s, a, b, c0, c1)` becomes
> `ShapeRadialGradient(s, a, b, c0, c1, ImWidgetsGradientInterp_OkLab)`.

### Drawing shapes

```cpp
void DrawShape          ( ImDrawList* pDrawList, ImWidgetsShape& shape );
void DrawImageShape     ( ImDrawList* pDrawList, ImTextureID tex, ImWidgetsShape& shape );
void DrawShapeDebug     ( ImDrawList* pDrawList, ImWidgetsShape& shape,
                          float edge_thickness, ImU32 edge_col, ImU32 triangle_col,
                          float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1 );
void DrawShapeDebugEx   ( ImDrawList* pDrawList, ImTextureID tex, ImWidgetsShape& shape,
                          float edge_thickness, ImU32 edge_col, ImU32 triangle_col,
                          float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1 );
void DrawImageShapeDebug( ImDrawList* pDrawList, ImTextureID tex, ImWidgetsShape& shape,
                          float edge_thickness, ImU32 edge_col, ImU32 triangle_col,
                          float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1 );
```
`DrawShape` uses the context's white texture; `DrawImageShape` samples `tex`
through the shape's UVs. `tri_idx >= 0` highlights a single triangle.

> **Removed in 2.0.0:** `DrawShapeEx`. It was the shared implementation behind
> `DrawShape` / `DrawImageShape` and had a signature identical to
> `DrawImageShape`; use that.

### Procedural colour straight onto a shape

Convenience wrappers that generate, fill and draw in one call:

```cpp
void DrawShapeProceduralColorVerticalBand  ( ImDrawList*, ImRect const& bb, int divisions,
                                             ImWidgetsColor1DCallback func, void* pUserData );
void DrawShapeProceduralColorHorizontalBand( ImDrawList*, ImRect const& bb, int divisions,
                                             ImWidgetsColor1DCallback func, void* pUserData );
void DrawShapeProceduralColorRectGrid      ( ImDrawList*, ImRect const& bb,
                                             int divisionsX, int divisionsY,
                                             ImWidgetsColor2DCallback func, void* pUserData );
void DrawShapeProceduralColorDiscRings     ( ImDrawList*, ImVec2 center, float radius,
                                             int numSectors, int numRings,
                                             ImWidgetsColor2DCallback func, void* pUserData );
void DrawShapeProceduralColorAnnulus       ( ImDrawList*, ImVec2 center,
                                             float innerRadius, float outerRadius, int numSectors,
                                             ImWidgetsColor1DCallback func, void* pUserData );
void DrawShapeProceduralColorAnnulusRings  ( ImDrawList*, ImVec2 center,
                                             float innerRadius, float outerRadius,
                                             int numSectors, int numRings,
                                             ImWidgetsColor2DCallback func, void* pUserData );
```

### Polygons with holes

Contour winding: **clockwise = outer boundary, counter-clockwise = hole**.

```cpp
void DrawConvexShape       ( ImDrawList* draw, ImVec2* poly, int points_count, ImU32 color );
void DrawConcaveShape      ( ImDrawList* draw, ImVec2* poly, int points_count, ImU32 color );
void DrawShapeWithHole     ( ImDrawList* draw, ImVec2* poly, int points_count, ImU32 color,
                             ImRect* p_bb = NULL, int gap = 1, int strokeWidth = 1 );
void DrawImageConvexShape  ( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count,
                             ImU32 tint, ImVec2 uv_offset = ImVec2( 0, 0 ), ImVec2 uv_scale = ImVec2( 1, 1 ) );
void DrawImageConcaveShape ( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count,
                             ImU32 tint, ImVec2 uv_offset = ImVec2( 0, 0 ), ImVec2 uv_scale = ImVec2( 1, 1 ) );
void DrawImageShapeWithHole( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count,
                             ImU32 tint, ImVec2 uv_offset = ImVec2( 0, 0 ), ImVec2 uv_scale = ImVec2( 1, 1 ),
                             int gap = 3, int strokeWidth = 3 );
```

---

## Color Bands & Rings

```cpp
void DrawHueBand       ( ImDrawList*, ImVec2 const vpos, ImVec2 const size,
                         int division, float alpha, float gamma, float offset );
void DrawHueBand       ( ImDrawList*, ImVec2 const vpos, ImVec2 const size,
                         int division, float colorStartRGB[ 3 ], float alpha, float gamma );
void DrawLuminanceBand ( ImDrawList*, ImVec2 const vpos, ImVec2 const size,
                         int division, ImVec4 const& color, float gamma );
void DrawSaturationBand( ImDrawList*, ImVec2 const vpos, ImVec2 const size,
                         int division, ImVec4 const& color, float gamma );
void DrawColorRing     ( ImDrawList*, ImVec2 const curPos, ImVec2 const size, float thickness_,
                         ImWidgetsColor1DCallback func, void* pUserData,
                         int division, float colorOffset, bool bIsBilinear );
void DrawColorDisc     ( ImDrawList*, ImVec2 center, float radius,
                         ImColorWheelMode mode, float thirdAxis,
                         int numSectors = 64, int numRings = 16 );
void DrawCircularGradientIndicator( ImDrawList*, ImVec2 center,
                                    float outerRadius, float innerRadius, float t );
```

---

## Procedural Color Drawing

```cpp
typedef ImU32 ( *ImWidgetsColor1DCallback )( float x, void* );
typedef ImU32 ( *ImWidgetsColor2DCallback )( float x, float y, void* );

void DrawProceduralColor1DNearestHorizontal ( ImDrawList*, ImWidgetsColor1DCallback func, void* pUserData,
                                              float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX );
void DrawProceduralColor1DBilinearHorizontal( ImDrawList*, ImWidgetsColor1DCallback func, void* pUserData,
                                              float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX );
void DrawProceduralColor1DNearestVertical   ( ImDrawList*, ImWidgetsColor1DCallback func, void* pUserData,
                                              float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionY );
void DrawProceduralColor1DBilinearVertical  ( ImDrawList*, ImWidgetsColor1DCallback func, void* pUserData,
                                              float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionY );

void DrawProceduralColor2DNearest ( ImDrawList*, ImWidgetsColor2DCallback func, void* pUserData,
                                    float minX, float maxX, float minY, float maxY,
                                    ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );
void DrawProceduralColor2DBilinear( ImDrawList*, ImWidgetsColor2DCallback func, void* pUserData,
                                    float minX, float maxX, float minY, float maxY,
                                    ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );

void DrawProceduralColorArcBilinear   ( ImDrawList*, ImVec2 center,
                                        float innerRadius, float outerRadius,
                                        float startAngle, float sweepAngle,
                                        ImWidgetsColor1DCallback func, void* pUserData,
                                        int division, bool bilinear );
void DrawProceduralColorSplineBilinear( ImDrawList*, ImVec2 const* points, int points_count,
                                        float thickness,
                                        ImWidgetsColor1DCallback func, void* pUserData,
                                        int resolution, bool closed );
```

> There is no undirected `DrawProceduralColor1DNearest` /
> `DrawProceduralColor1DBilinear`. Only the `...Horizontal` and `...Vertical`
> forms exist; earlier documentation listed the undirected names in error.

Colour-space-aware overloads of all of the above take an
`ImWidgetsColorScalar const&` instead of the callback pair -- see
[color.md](color.md#space-aware-drawing-overloads).

---

## Gradients

```cpp
void DrawGradientBar      ( ImDrawList*, ImGradientData const& gradient,
                            ImVec2 position, ImVec2 size, int resolution );
void DrawCheckerboard     ( ImDrawList*, ImVec2 position, ImVec2 size,
                            float cellSize, ImU32 col1, ImU32 col2 );
void DrawSplineGradient   ( ImDrawList*, ImGradientData const& gradient,
                            ImVec2 const* points, int points_count,
                            float thickness, int resolution, bool closed = false );
void DrawSplineGradientCut( ImDrawList*, ImGradientData const& gradient,
                            float const min, float const max,
                            ImVec2 const* points, int points_count,
                            float thickness, int resolution, bool closed = false );
void DrawConicGradient    ( ImDrawList*, ImVec2 center, float radius,
                            ImGradientData const& gradient, float start_angle_rad,
                            int resolution = 128 );
void DrawConicGradientRect( ImDrawList*, ImVec2 min, ImVec2 max, ImVec2 center_uv,
                            ImGradientData const& gradient, float start_angle_rad,
                            int resolution = 128 );
```

`DrawSplineGradient` paints a multi-stop gradient along a cubic Bezier spline.
`DrawSplineGradientCut` restricts it to the normalized arc range `[min, max]`;
outside that range the stroke is fully transparent, which is how the gradient
range sliders render their spline tracks.

`DrawConicGradient` sweeps the gradient around a disc; `DrawConicGradientRect`
does the same over a rectangle with the cone apex at `center_uv` (normalized
inside the rect).

### OkLab / OkLCH quads

```cpp
void DrawOkLabQuad( ImDrawList*, ImVec2 start, ImVec2 size, float L, int resX = 16, int resY = 16 );
void DrawOkLchQuad( ImDrawList*, ImVec2 start, ImVec2 size, float L, int resX = 16, int resY = 16 );
```
A 2D grid of colours at fixed lightness `L`, covering the `a/b` (OkLab) or `C/H`
(OkLCH) plane.

---

## Fills and Patches

```cpp
void DrawHatchFill  ( ImDrawList*, ImVec2 const* poly, int poly_count,
                      ImWidgetsHatchPattern pattern, float spacing, float angle_rad,
                      float thickness, ImU32 col );
void DrawStippleFill( ImDrawList*, ImVec2 const* poly, int poly_count,
                      float density, float jitter, float radius, ImU32 col,
                      unsigned int seed = 1u );
```
```cpp
enum ImWidgetsHatchPattern_
{
    ImWidgetsHatchPattern_Parallel, ImWidgetsHatchPattern_Cross,
    ImWidgetsHatchPattern_Diagonal, ImWidgetsHatchPattern_DiagonalCross,
    ImWidgetsHatchPattern_Dots,     ImWidgetsHatchPattern_ConcentricRings,
    ImWidgetsHatchPattern_BenDay,   ImWidgetsHatchPattern_Screentone,
    ImWidgetsHatchPattern_COUNT
};
```
Defaults live in `ImWidgetsStyleVar_Hatch_DefaultSpacing` / `_DefaultThickness`.

```cpp
void DrawCoonsPatch         ( ImDrawList*, ImCoonsPatch const& patch, ImU32 col,
                              int resU = 16, int resV = 16 );
void DrawCoonsPatchGradient ( ImDrawList*, ImCoonsPatch const& patch,
                              ImU32 c00, ImU32 c10, ImU32 c11, ImU32 c01,
                              int resU = 16, int resV = 16 );
void DrawCoonsPatchWireframe( ImDrawList*, ImCoonsPatch const& patch, ImU32 col,
                              float thickness, int resU = 16, int resV = 16 );
void DrawGregoryPatch       ( ImDrawList*, ImGregoryPatch const& patch, ImU32 col,
                              int resU = 16, int resV = 16 );
void DrawGregoryPatchGradient( ImDrawList*, ImGregoryPatch const& patch,
                              ImU32 col_bl, ImU32 col_br, ImU32 col_tr, ImU32 col_tl,
                              int resU = 16, int resV = 16 );
```
A Coons patch is bounded by four cubic Bezier edges; a Gregory patch adds
interior control points so adjacent patches can meet with tangent continuity.

---

## Offsets and Iso-Contours

```cpp
void OffsetPolyline   ( ImVec2 const* in_pts, int n, float offset,
                        ImWidgetsJoin join, float miter_limit,
                        ImVector<ImVec2>& out_pts, bool closed );
void DrawOffsetOutline( ImDrawList*, ImVec2 const* pts, int n, float offset,
                        ImWidgetsJoin join, float miter_limit,
                        float thickness, ImU32 col, bool closed );
void DrawOffsetFilled ( ImDrawList*, ImVec2 const* pts, int n, float inset,
                        ImU32 col, bool closed );
```
Parallel-curve offsetting with proper joins. `OffsetPolyline` gives you the
points; the two `Draw*` helpers stroke or fill the result.

```cpp
typedef float ( *ImWidgetsScalar2DCallback )( float x, float y, void* user );

void DrawIsoContour      ( ImDrawList*, ImVec2 pos, ImVec2 size, int resX, int resY,
                           ImWidgetsScalar2DCallback f, void* user,
                           float const* iso_values, int iso_count,
                           ImU32 col, float thickness );
void DrawIsoContourTiered( ImDrawList*, ImVec2 pos, ImVec2 size, int resX, int resY,
                           ImWidgetsScalar2DCallback f, void* user,
                           ImIsoContourTier const* tiers, int tier_count,
                           float min_value = -FLT_MAX, float max_value = FLT_MAX,
                           bool pre_log_transform = false, bool half_pixel_sample = true,
                           int sample_subdiv = 1 );
void DrawIsoFilled       ( ImDrawList*, ImVec2 pos, ImVec2 size, int resX, int resY,
                           ImWidgetsScalar2DCallback f, void* user,
                           float const* band_edges, int band_edge_count,
                           ImU32 const* band_cols );
```
Marching-squares contouring of a scalar field. `DrawIsoContourTiered` draws
major / medium / minor tiers with distinct thicknesses
(`ImWidgetsStyleVar_IsoContour_MajorThickness` and siblings). `DrawIsoFilled`
paints filled bands between successive `band_edges` (there must be
`band_edge_count - 1` colours).

---

## Vector Paths (SVG / Canvas2D)

Paths drawn with plain `ImDrawList` triangles: no custom shader, so they work on
stock Dear ImGui and every backend.

```cpp
ImWidgetsPath p;
p.AddSvgPathData( "M12 21 C12 21 2 14 2 8 A5 5 0 0 1 12 6 A5 5 0 0 1 22 8 C22 14 12 21 12 21 Z" );
p.Transform( ImWidgetsTransform2D::Translate( origin.x, origin.y ) * ImWidgetsTransform2D::Scale( 8, 8 ) );
ImWidgets::PathFill  ( dl, p, IM_COL32( 230, 60, 80, 255 ) );
ImWidgetsStrokeStyle st; st.Width = 3; st.Join = ImWidgetsJoin_Round;
ImWidgets::PathStroke( dl, p, IM_COL32_WHITE, st );
```

### `ImWidgetsPath`
Canvas2D-style builder (calls chain): `MoveTo`, `LineTo`, `QuadTo`, `CubicTo`,
`ArcTo` (SVG elliptical arc: radii, x-axis rotation, large-arc and sweep flags),
`Arc` (Canvas2D `arc()`), `Close`, plus `Rect`, `RoundRect`, `Ellipse`, `Circle`.
`AddSvgPathData( d )` parses SVG path data: `M L H V C S Q T A Z`, absolute and
relative, implicit command repeats, packed numbers (`10-5.5.5`) and packed arc
flags; on a syntax error it keeps everything before the error and returns
`false`, like a browser. Every segment is stored as a cubic Bezier, so
`Transform( ImWidgetsTransform2D )` is exact for lines and curves.
`Flatten` returns polylines; `GetBounds` the control-point bounds.

`ImWidgetsTransform2D` is SVG's `matrix(a b c d e f)`; `A * B` applies `B`
first, as in an SVG `transform` list.

### `FillPolygons` / `PathFill`
```cpp
void FillPolygons( ImDrawList*, ImVec2 const* points, int const* contour_counts, int contour_count,
                   ImU32 col, ImWidgetsFillRule rule = ImWidgetsFillRule_NonZero, float aa = 1.0f );
void PathFill    ( ImDrawList*, ImWidgetsPath const&, ImU32 col,
                   ImWidgetsFillRule rule = ImWidgetsFillRule_NonZero );
```
Anti-aliased fill with the SVG `fill-rule` (`NonZero` / `EvenOdd`): holes,
multiple contours and self-intersecting outlines are exact. The covered area is
decomposed into trapezoids (merged along each edge pair, so the triangle count
follows the number of edges) with a 1 px anti-aliasing fringe; no pixel is
covered twice, so translucent fills have no seams. `PathFill` treats every
subpath as closed, like SVG.

### `PathStroke`
```cpp
void PathStroke( ImDrawList*, ImWidgetsPath const&, ImU32 col,
                 ImWidgetsStrokeStyle const& style = ImWidgetsStrokeStyle() );
```
`ImWidgetsStrokeStyle` mirrors SVG's `stroke-*` properties: `Width`, `Cap`,
`Join`, `MiterLimit`, `Dashes` / `DashCount` (`stroke-dasharray`; an odd list
repeats, as in SVG), `DashOffset`, `Tolerance`. Strokes use the Euler-spiral
stroker below; when the stroke shader is unavailable its exact outline is filled
on the CPU with the nonzero rule, so caps, joins and dashes are kept.

## Lines, Strokes and Dashes

### Unified dispatcher

```cpp
void        DrawThickLine( ImDrawList* dl, ImVec2 const* points, int point_count,
                           ImWidgetsThickLineDesc const& desc );
char const* GetThickLineModeName( ImWidgetsThickLineMode mode );
```
One entry point behind five line-rendering techniques. See
[compatibility.md](../compatibility.md#5-drawthickline--runtime-fallback-details)
for the per-mode tiers and the runtime fallback.

### Anti-aliased polylines

```cpp
void DrawPolylineAA      ( ImDrawList*, ImVec2 const* points, int points_count,
                           ImU32 col, float thickness, bool closed = false,
                           ImWidgetsCap cap = ImWidgetsCap_Butt,
                           ImWidgetsJoin join = ImWidgetsJoin_Miter,
                           float miter_limit = 4.0f );

// multi-dash pattern
void DrawDashedPolylineAA( ImDrawList*, ImVec2 const* points, int points_count,
                           ImU32 col, float thickness,
                           float const* dashes, int dashes_count, float dash_offset,
                           bool closed = false,
                           ImWidgetsCap cap = ImWidgetsCap_Butt,
                           ImWidgetsJoin join = ImWidgetsJoin_Miter,
                           float miter_limit = 4.0f );

// single on/off convenience
void DrawDashedPolylineAA( ImDrawList*, ImVec2 const* points, int points_count,
                           ImU32 col, float thickness,
                           float dash_len, float gap_len, float dash_offset,
                           bool closed = false,
                           ImWidgetsCap cap = ImWidgetsCap_Butt,
                           ImWidgetsJoin join = ImWidgetsJoin_Miter,
                           float miter_limit = 4.0f );
```

`dashes` is an alternating `[on, off, on, off, ...]` array in pixels.

**Gap semantics (envelope parametrization).** `dash_len` is the cap-tip-to-cap-tip
*envelope* length; `gap_len` is the distance between the edges of adjacent
envelopes:

| `gap_len` | Result |
|---|---|
| `= 0` | Dashes are back-to-back -- the trailing cap edge of one touches the leading cap edge of the next. No visible gap, regardless of cap style. |
| `> 0` | Visible gap of exactly `gap_len` pixels between adjacent cap edges. |
| `< 0` | Overlapping envelopes: caps from adjacent dashes interpenetrate by `\|gap_len\|` pixels. Useful for seamless joined-cap effects. |

For outward caps (`Square`, `Round`, `TriangleOut`) the body rectangle is shorter
than the envelope by `thickness` (one radius each side). For `TriangleIn` the
envelope equals the body -- the notch is carved *inward*, so `gap_len = 0` is
already flush.

**Cap types (`ImWidgetsCap`):** `None`, `Butt`, `Square`, `Round`, `TriangleOut`,
`TriangleIn`.
**Join types (`ImWidgetsJoin`):** `Round`, `Miter`, `Bevel`.

GPU acceleration: `SetDashedLinesUseGPU(true)` (requires shader support). CPU and
GPU paths produce identical results for the same parameters.

### Euler-spiral strokes

```cpp
void DrawStrokedPolyline    ( ImDrawList*, ImVec2 const* points, int points_count,
                              ImU32 col, float thickness,
                              ImWidgetsCap cap = ImWidgetsCap_Round,
                              ImWidgetsJoin join = ImWidgetsJoin_Round,
                              float miter_limit = 4.0f, bool closed = false );

void DrawStrokedBezierPath  ( ImDrawList*, ImVec2 const* points, int points_count,
                              ImU32 col, float thickness,
                              ImWidgetsCap cap = ImWidgetsCap_Round,
                              ImWidgetsJoin join = ImWidgetsJoin_Round,
                              float miter_limit = 4.0f, float tolerance = 0.25f,
                              bool closed = false,
                              ImWidgetsPrimitive primitive = ImWidgetsPrimitive_Line,
                              ImWidgetsCorrectness correctness = ImWidgetsCorrectness_Weak );

void DrawStrokedCubicBezier ( ImDrawList*, ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3,
                              ImU32 col, float thickness,
                              ImWidgetsCap cap = ImWidgetsCap_Round,
                              float tolerance = 0.25f,
                              ImWidgetsPrimitive primitive = ImWidgetsPrimitive_Line,
                              ImWidgetsCorrectness correctness = ImWidgetsCorrectness_Weak );

void DrawStrokedDashedPolyline  ( ImDrawList*, ImVec2 const* points, int points_count,
                                  ImU32 col, float thickness,
                                  float const* dash_array, int dash_count, float dash_offset = 0.0f,
                                  ImWidgetsCap cap = ImWidgetsCap_Round,
                                  ImWidgetsJoin join = ImWidgetsJoin_Round,
                                  float miter_limit = 4.0f, bool closed = false );

void DrawStrokedDashedBezierPath( ImDrawList*, ImVec2 const* points, int points_count,
                                  ImU32 col, float thickness,
                                  float const* dash_array, int dash_count, float dash_offset = 0.0f,
                                  ImWidgetsCap cap = ImWidgetsCap_Round,
                                  ImWidgetsJoin join = ImWidgetsJoin_Round,
                                  float miter_limit = 4.0f, float tolerance = 0.25f,
                                  bool closed = false,
                                  ImWidgetsPrimitive primitive = ImWidgetsPrimitive_Line,
                                  ImWidgetsCorrectness correctness = ImWidgetsCorrectness_Weak );
```

> **The dashed stroke functions take a dash *array*, not `dash_len` / `gap_len`.**
> Earlier documentation showed the two-float form, which does not exist for this
> family. Pass `float dashes[] = { on, off }` with `dash_count = 2` for the simple
> case. `DrawStrokedDashedBezierPath` additionally takes `tolerance`,
> `primitive` and `correctness`.

The `*BezierPath` forms convert the input polyline to a cubic Bezier path with a
Catmull-Rom -> cubic conversion, then stroke it with Euler-spiral offset curves.
These are pure CPU and are **not** gated by
`IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER`.

### Stroke configuration and debug

```cpp
void SetDashedLinesUseGPU    ( bool enable );
bool GetDashedLinesUseGPU    ();
void SetDashedLinesDebugJoins( bool enable );
bool GetDashedLinesDebugJoins();
void SetStrokeDebugWireframe ( bool enable );
bool GetStrokeDebugWireframe ();
```

---

## Cursors and Crosshairs

```cpp
void DrawTriangleCursor      ( ImDrawList*, ImVec2 targetPoint, float angle,
                               float size, float thickness, ImU32 col );
void DrawTriangleCursorFilled( ImDrawList*, ImVec2 targetPoint, float angle,
                               float size, ImU32 col );
void DrawSignetCursor        ( ImDrawList*, ImVec2 targetPoint,
                               float width, float height, float height_ratio, float align01,
                               float angle, float thickness, ImU32 col );
void DrawSignetFilledCursor  ( ImDrawList*, ImVec2 targetPoint,
                               float width, float height, float height_ratio, float align01,
                               float angle, ImU32 col );

char const* GetCrosshairStyleName( ImWidgetsCrosshairStyle style );
void DrawCrosshair      ( ImDrawList* draw, ImVec2 center, float radius,
                          ImWidgetsCrosshairStyle style, ImU32 col,
                          float thickness = 1.0f, float gap_radius = 0.0f );
void DrawCrosshairInRect( ImDrawList* draw, ImVec2 center, ImRect bounds,
                          ImU32 col, float thickness = 1.0f );
```
```cpp
enum ImWidgetsCrosshairStyle_
{
    ImWidgetsCrosshairStyle_Plus, ImWidgetsCrosshairStyle_GappedPlus,
    ImWidgetsCrosshairStyle_MilDot, ImWidgetsCrosshairStyle_T,
    ImWidgetsCrosshairStyle_Dot, ImWidgetsCrosshairStyle_CircleDot,
    ImWidgetsCrosshairStyle_COUNT
};
```

---

## Arrows, Dimensions, Rulers and Grids

```cpp
char const* GetArrowHeadName( ImWidgetsArrowHead head );

void DrawArrow     ( ImDrawList* draw, ImVec2 from, ImVec2 to, ImU32 col,
                     float thickness = 1.0f,
                     ImWidgetsArrowHead head_end   = ImWidgetsArrowHead_Triangle,
                     ImWidgetsArrowHead head_start = ImWidgetsArrowHead_None,
                     float head_size = 10.0f );
void DrawArrowHead ( ImDrawList* draw, ImVec2 tip, ImVec2 dir, ImU32 col,
                     ImWidgetsArrowHead style, float size = 10.0f, float thickness = 1.0f );
void DrawAxisArrows( ImDrawList* draw, ImVec2 origin, float len_x, float len_y,
                     ImU32 col_x = IM_COL32( 230,  80,  80, 255 ),
                     ImU32 col_y = IM_COL32(  80, 200,  80, 255 ),
                     float thickness = 1.0f, float head_size = 10.0f,
                     char const* label_x = "x", char const* label_y = "y",
                     ImU32 label_col = IM_COL32( 230, 230, 230, 255 ) );
```
Heads: `None`, `Triangle`, `Open`, `Diamond`, `Stealth`, `Tick`, `Dot`, `Square`.

```cpp
char const* GetDimensionTextOrientName( ImWidgetsDimensionTextOrient orient );

void DrawDimensionLine( ImDrawList* draw, ImVec2 from, ImVec2 to, float offset,
                        char const* text, ImU32 line_col, ImU32 text_col,
                        float line_thickness = 1.0f,
                        ImWidgetsArrowHead head = ImWidgetsArrowHead_Triangle,
                        float head_size = 8.0f,
                        float ext_overshoot = 4.0f, float ext_gap = 2.0f,
                        ImWidgetsDimensionTextOrient text_orient = ImWidgetsDimensionTextOrient_FollowReading,
                        ImWidgetsDimensionFlags flags = ImWidgetsDimensionFlags_Default,
                        ImFont* font = NULL, float font_size = 0.0f );
```
Technical-drawing dimension line: offset witness lines, arrowheads, and a text
label that can follow the line, stay horizontal, follow reading direction, or sit
perpendicular. Flags control extension lines, head placement, text placement and
whether the line breaks for the text.

```cpp
void DrawRuler      ( ImDrawList* draw, ImRect bounds, ImWidgetsRulerOrient orient,
                      float world_min, float world_max,
                      float major_step, int minor_subdivs,
                      char const* label_format = "%g",
                      ImU32 line_col  = IM_COL32( 200, 200, 200, 220 ),
                      ImU32 label_col = IM_COL32( 230, 230, 230, 255 ),
                      float major_tick_height = 10.0f, float minor_tick_height = 5.0f,
                      float thickness = 1.0f,
                      float const* p_mouse_world = NULL,
                      ImU32 tracker_col = IM_COL32( 255, 200, 60, 230 ),
                      ImFont* font = NULL, float font_size = 0.0f );

void DrawGridOverlay( ImDrawList* draw, ImRect bounds, ImVec2 origin,
                      float major_step, int minor_subdivs,
                      ImU32 major_col  = IM_COL32( 100, 100, 100, 180 ),
                      ImU32 minor_col  = IM_COL32(  60,  60,  60, 120 ),
                      ImU32 origin_col = IM_COL32( 220, 220, 220, 220 ),
                      ImWidgetsGridFlags flags = ImWidgetsGridFlags_Default,
                      float thickness = 1.0f );
```
`p_mouse_world` (when non-null) draws a position tracker on the ruler.
`ImWidgetsGridFlags_Dots` renders intersections as dots instead of lines.

---

## Shadows and Glows

```cpp
void DrawDropShadowRect( ImDrawList* draw, ImRect r, float radius, ImVec2 offset,
                         ImU32 col, float corner_round = 0.0f );
void DrawInnerGlowRect ( ImDrawList* draw, ImRect r, float radius, ImU32 col,
                         float corner_round = 0.0f );
```
Layered-quad approximations of a Gaussian falloff -- no shader required.

---

## Graduation Lines

```cpp
void DrawLinearLineGraduation    ( ImDrawList*, ImVec2 start, ImVec2 end,
                                   float mainLineThickness, ImU32 mainCol,
                                   int division0, float height0, float thickness0, float angle0, ImU32 col0,
                                   int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f,
                                   float angle1 = -1.0f, ImU32 col1 = 0u,
                                   int division2 = -1, float height2 = -1.0f, float thickness2 = -1.0f,
                                   float angle2 = -1.0f, ImU32 col2 = 0u );
void DrawLinearCircularGraduation( ImDrawList*, ImVec2 center, float radius,
                                   float start_angle, float end_angle, int num_segments,
                                   float mainLineThickness, ImU32 mainCol,
                                   int division0, ... /* same three tiers */ );
void DrawLogLineGraduation       ( ImDrawList*, ImVec2 start, ImVec2 end,
                                   float mainLineThickness, ImU32 mainCol,
                                   int division0, ... /* two tiers */ );
void DrawLogCircularGraduation   ( ImDrawList*, ImVec2 center, float radius,
                                   float start_angle, float end_angle, int num_segments,
                                   float mainLineThickness, ImU32 mainCol,
                                   int division0, ... /* two tiers */ );
```
Tick-marked scale lines with linear or logarithmic spacing along a straight or
circular path. Linear variants take up to three tick tiers, log variants two;
a tier is disabled by leaving its `divisionN` at `-1`.

---

## Chromatic Plots

```cpp
void DrawChromaticityPlot( ImDrawList* draw,
                           ImWidgetsIlluminant illuminance, ImWidgetsObserver observer,
                           ImWidgetsColorSpace colorSpace, int chromeLineSamplesCount,
                           ImVec2 const vpos, ImVec2 const size,
                           int resolutionX, int resolutionY, ImU32 maskColor,
                           float wavelengthMin = 400.0f, float wavelengthMax = 700.0f,
                           float minX = 0.0f, float maxX = 0.8f,
                           float minY = 0.0f, float maxY = 0.9f,
                           bool showColorSpaceTriangle = true,
                           bool showWhitePoint = true, bool showBorder = true,
                           ImU32 borderColor = IM_COL32( 0, 0, 0, 255 ),
                           float borderThickness = 1.0f );
```
> The first argument's type is `ImWidgetsIlluminant` (not `ImWidgetsIlluminance`,
> which does not exist). The parameter is still named `illuminance`.

```cpp
void DrawChromaticityPoints( ImDrawList*, ImVec2 curPos, ImVec2 size,
                             ImU32* colors4, int color_count,
                             float minX, float maxX, float minY, float maxY,
                             ImU32 plotColor, float radius, int num_segments );
void DrawChromaticityPoints( ImDrawList*, ImVec2 curPos, ImVec2 size,
                             ImWidgetsColorSpace colorSpace,
                             float* colors4, int color_count,        // AoS
                             float minX, float maxX, float minY, float maxY,
                             ImU32 plotColor, float radius, int num_segments,
                             int colorStride = 4 );

void DrawChromaticityLines ( ImDrawList*, ImVec2 curPos, ImVec2 size,
                             ImU32* color, int color_count,
                             float minX, float maxX, float minY, float maxY,
                             ImU32 plotColor, ImDrawFlags flags, float thickness );
void DrawChromaticityLines ( ImDrawList*, ImVec2 curPos, ImVec2 size,
                             ImWidgetsColorSpace colorSpace,
                             float* colors4, int color_count,        // AoS
                             float minX, float maxX, float minY, float maxY,
                             ImU32 plotColor, ImDrawFlags flags, float thickness,
                             int colorStride = 4 );

void DrawChromaticityGamuts( ImDrawList*, ImVec2 pos, ImVec2 size,
                             ImWidgetsColorSpace const* spaces, int space_count,
                             float test_x, float test_y,
                             float minX = 0.0f, float maxX = 0.8f,
                             float minY = 0.0f, float maxY = 0.9f,
                             bool show_labels = true, float thickness = 1.5f );
```
`colorStride` is `4` for `rgba,rgba,...` and `3` for `rgb,rgb,...`.
`DrawChromaticityGamuts` overlays several gamut triangles and marks whether the
test chromaticity `(test_x, test_y)` falls inside each.

Style: `ImWidgetsCol_ChromaticityPlot_*` / `ImWidgetsCol_ChromaticityLine_Default`
/ `ImWidgetsCol_ChromaticityPoint_Default`, plus
`ImWidgetsStyleVar_ChromaticityPlot_*`, `_ChromaticityLine_Thickness`,
`_ChromaticityPoint_Radius` and the `int` fields
`ChromaticityPlot_Resolution`, `ChromaticityPlot_LineSamples`,
`ChromaticityPoint_Segments`, `ChromaticityLine_SmoothIterations`.

---

## Ephemerides and Sky Charts

```cpp
void DrawMoonEphemeris    ( ImDrawList*, ImVec2 center, float radius,
                            int year, int month, int day, float lonDeg, float latDeg,
                            ImU32 litCol     = IM_COL32( 190, 185, 172, 255 ),
                            ImU32 darkCol    = IM_COL32(  28,  36,  56, 255 ),
                            ImU32 outlineCol = IM_COL32( 170, 175, 200, 255 ) );

void DrawEarthSunEphemeris( ImDrawList*, ImVec2 areaMin, ImVec2 areaSize,
                            int year, int month, int day, int hour, int minute,
                            float obsLon = 0.0f, float obsLat = 0.0f,
                            ImU32 sunCol     = IM_COL32( 255, 220,  80, 255 ),
                            ImU32 dayCol     = IM_COL32( 110, 165, 220, 255 ),
                            ImU32 nightCol   = IM_COL32(  25,  35,  60, 255 ),
                            ImU32 outlineCol = IM_COL32( 180, 190, 210, 255 ) );

void DrawStarChart( ImDrawList*, ImVec2 center, float radius,
                    int year, int month, int day, int hour, int minute,
                    float obsLon = 0.0f, float obsLat = 48.85f,
                    float magLimit = 5.5f, float starScale = 1.0f,
                    ImWidgetsSkyCulture culture = ImWidgetsSkyCulture_Western,
                    bool showSolarSystem = true,
                    ImU32 skyCol     = IM_COL32(   8,  11,  22, 255 ),
                    ImU32 outlineCol = IM_COL32( 180, 195, 225, 230 ),
                    ImFont* labelFont = nullptr );

void DrawSunPath( ImDrawList*, ImVec2 areaMin, ImVec2 areaSize,
                  float obsLat = 48.85f, float obsLon = 0.0f,
                  int tzOffset = 0, int year = 2026,
                  ImWidgetsSunPathMode mode = ImWidgetsSunPathMode_Polar,
                  ImU32 bgCol      = IM_COL32(  12,  18,  30, 255 ),
                  ImU32 outlineCol = IM_COL32( 190, 205, 230, 240 ) );

char const* GetSkyCultureName ( ImWidgetsSkyCulture culture );
int         GetSkyCultureCount();
```

`DrawMoonEphemeris` draws a schematic lunar phase for a date and observer.
`DrawEarthSunEphemeris` draws the day/night terminator on an equirectangular
Earth with the subsolar point.

`DrawStarChart` is an observer-centred sky chart: stars projected through
equatorial-to-horizon (alt/az), culture-specific constellation figure lines,
native star names, and an optional solar-system overlay (Moon with phase,
naked-eye planets, Sun when above the horizon).

```cpp
enum ImWidgetsSkyCulture
{
    ImWidgetsSkyCulture_Western = 0,  // IAU modern, 88 constellations
    ImWidgetsSkyCulture_Chinese,      // Han-dynasty 28 lunar mansions
    ImWidgetsSkyCulture_Arabic,       // Manazil al-Qamar (right-to-left text)
    ImWidgetsSkyCulture_Polynesian,   // Maori navigation
    ImWidgetsSkyCulture_COUNT
};
```
> When the culture uses non-Latin glyphs (Arabic, CJK), pass `labelFont` with the
> matching ranges; otherwise the active ImGui font is used.

`DrawSunPath` draws a sun-path diagram plus analemma.
`ImWidgetsSunPathMode_Polar` is a zenith-centred dome (sun arcs are radial curves
from sunrise to sunset); `_Cartesian` is azimuth x altitude. The yellow
figure-of-eight analemma loops are drawn at five fixed local clock hours
(06/09/12/15/18h); the equation of time plus the longitude offset from the
time-zone meridian produce their east-west spread.

The defaults `obsLat = 48.85f` and `year = 2026` are Paris and the library's
release year -- pass your own.

---

## Markers

> **Requires:** `ImWidgetsFeatures_Markers` and
> `IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER`.

```cpp
void DrawMarker( ImDrawList*, ImVec2 start, ImVec2 size,
                 ImU32 fg_color, ImU32 bg_color,
                 float rot_angle_rad, float shape_size, float linewidth, float antialiasing,
                 ImWidgetsMarker marker, ImWidgetsDrawType draw_type );
```
**Marker shapes (`ImWidgetsMarker`):** `Disc`, `Square`, `Triangle`, `Diamond`,
`Heart`, `Spade`, `Club`, `Chevron`, `Clover`, `Ring`, `Tag`, `Cross`,
`Asterisk`, `Infinity`, `Pin`, `Arrow`, `Ellipse`, `EllipseApprox`.

**Draw types (`ImWidgetsDrawType`):** `Filled`, `Stroke`, `Outline`,
`SignedDistanceField`, `CutOff`.

---

## Custom Render Frames & Nav Cursors

```cpp
void RenderFrameEx     ( ImU32 fill_col, bool border,
                         ImDrawShape outline, ImDrawShapeFilled fill, ImDrawShapeFilledTex fill_tex,
                         void* data, ImTextureID* tex = NULL,
                         ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
void RenderFrameCircle ( ImVec2 center, float radius, ImU32 fill_col, bool border );
void RenderFrameConvex ( ImVec2* pts, int pts_count, ImU32 fill_col, bool border );
void RenderFrameConcave( ImVec2* pts, int pts_count, ImU32 fill_col, bool border );

void RenderNavCursorEx     ( ImGuiID id, ImDrawShape func, void* data, ImRect display_rect,
                             ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );
void RenderNavCursorCircle ( ImVec2 center, float radius, ImGuiID id,
                             ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );
void RenderNavCursorConvex ( ImVec2* pts, int pts_count, ImGuiID id,
                             ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );
void RenderNavCursorConcave( ImVec2* pts, int pts_count, ImGuiID id,
                             ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );
void RenderCursorWithHole  ( ImVec2* pts, int pts_count, ImGuiID id,
                             ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );
```
Nav cursor geometry: `ImWidgetsStyleVar_NavCursor_Thickness` / `_Distance`.

> **Removed in 2.0.0:** `RenderFrameWithHole` -- it was declared but never
> defined. `RenderCursorWithHole` (the nav-cursor form) does exist.

---

## Window Background

`SetCurrentWindowBackgroundImage`, `SetCurrentWindowBlurBackground` and
`BlurBackgroundNewFrame` are documented in
[widgets.md -- Window Background](widgets.md#window-background).

---

## Text Drawing

Slug GPU text, tessellated text, gradient / image text fills, text on a path and
the font tooling widgets are in [typography.md](typography.md).
