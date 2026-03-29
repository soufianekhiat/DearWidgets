# DrawList Primitives

Functions for drawing shapes, gradients, chromatic diagrams, markers, and polylines directly into an `ImDrawList`.

---

## Shapes

### Geometry Generation

```cpp
void GenShapeRect(ImWidgetsShape& shape, const ImRect& r);
void GenShapeCircle(ImWidgetsShape& shape, ImVec2 center, float radius, int side_count);
void GenShapeCircleArc(ImWidgetsShape& shape, ImVec2 center, float radius,
                       float angle_min, float angle_max, int side_count);
void GenShapeRegularNGon(ImWidgetsShape& shape, ImVec2 center, float radius, int side_count);
void GenShapeSquircle(ImWidgetsShape& shape, ImVec2 center, float radius,
                      int side_count, float n = 4.0f);
```

### Gradient Fill on Shapes

Each function fills an existing `ImWidgetsShape` with a gradient in the chosen color space.

```cpp
// Linear gradients
void ShapeSRGBLinearGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeLinearSRGBLinearGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeOkLabLinearGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeOkLchLinearGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeHSVLinearGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);

// Radial gradients
void ShapeSRGBRadialGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeOkLabRadialGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeOkLchRadialGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeHSVRadialGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);

// Diamond (rhombus) gradients
void ShapeSRGBDiamondGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeOkLabDiamondGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeOkLchDiamondGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeHSVDiamondGradient(ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
```

Generic version (any color space via function pointers):
```cpp
void ShapeLinearGradientGeneric(ImWidgetsShape& shape,
    ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1,
    pfSpace2sRGB space2sRGB, pfsRGB2Space sRGB2Space);
```

### Drawing Shapes

```cpp
void DrawShape(ImDrawList* pDrawList, ImWidgetsShape& shape);
void DrawShapeEx(ImDrawList* pDrawList, ImTextureID tex, ImWidgetsShape& shape);
void DrawImageShape(ImDrawList* pDrawList, ImTextureID tex, ImWidgetsShape& shape);
void DrawShapeDebug(ImDrawList* pDrawList, ImWidgetsShape& shape,
                    float edge_thickness, ImU32 edge_col, ImU32 triangle_col,
                    float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1);
```

### Shapes with Holes (Concave / Complex Polygons)

The contour winding convention: **clockwise = outer boundary, counter-clockwise = hole**.

```cpp
void DrawShapeWithHole(ImDrawList* draw, ImVec2* poly, int points_count,
                       ImU32 color, ImRect* p_bb = NULL,
                       int gap = 1, int strokeWidth = 1);
void DrawImageShapeWithHole(ImDrawList* draw, ImTextureID img,
                             ImVec2* poly, int points_count, ImU32 tint,
                             ImVec2 uv_offset = {0,0}, ImVec2 uv_scale = {1,1},
                             int gap = 3, int strokeWidth = 3);
void DrawImageConvexShape(ImDrawList* draw, ImTextureID img,
                          ImVec2* poly, int points_count, ImU32 tint,
                          ImVec2 uv_offset = {0,0}, ImVec2 uv_scale = {1,1});
void DrawImageConcaveShape(ImDrawList* draw, ImTextureID img,
                           ImVec2* poly, int points_count, ImU32 tint,
                           ImVec2 uv_offset = {0,0}, ImVec2 uv_scale = {1,1});
```

---

## Color Bands & Rings

```cpp
void DrawHueBand(ImDrawList* pDrawList, ImVec2 vpos, ImVec2 size,
                 int division, float alpha, float gamma, float offset);
void DrawHueBand(ImDrawList* pDrawList, ImVec2 vpos, ImVec2 size,
                 int division, float colorStartRGB[3], float alpha, float gamma);
void DrawLumianceBand(ImDrawList* pDrawList, ImVec2 vpos, ImVec2 size,
                      int division, const ImVec4& color, float gamma);
void DrawSaturationBand(ImDrawList* pDrawList, ImVec2 vpos, ImVec2 size,
                        int division, const ImVec4& color, float gamma);
void DrawColorRing(ImDrawList* pDrawList, ImVec2 curPos, ImVec2 size,
                   float thickness, ImWidgetsColor1DCallback func, void* pUserData,
                   int division, float colorOffset, bool bIsBilinear);
```

---

## Procedural Color Drawing

```cpp
typedef ImU32 (*ImWidgetsColor1DCallback)(float x, void*);
typedef ImU32 (*ImWidgetsColor2DCallback)(float x, float y, void*);

void DrawProceduralColor1DNearest(ImDrawList* pDrawList, ImWidgetsColor1DCallback func, void* pUserData,
                                   float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX);
void DrawProceduralColor1DBilinear(ImDrawList* pDrawList, ImWidgetsColor1DCallback func, void* pUserData,
                                    float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX);
void DrawProceduralColor2DNearest(ImDrawList* pDrawList, ImWidgetsColor2DCallback func, void* pUserData,
                                   float minX, float maxX, float minY, float maxY,
                                   ImVec2 position, ImVec2 size, int resolutionX, int resolutionY);
void DrawProceduralColor2DBilinear(ImDrawList* pDrawList, ImWidgetsColor2DCallback func, void* pUserData,
                                    float minX, float maxX, float minY, float maxY,
                                    ImVec2 position, ImVec2 size, int resolutionX, int resolutionY);
```

---

## Gradient Bar & Checkerboard

```cpp
void DrawGradientBar(ImDrawList* pDrawList, const ImGradientData& gradient,
                     ImVec2 position, ImVec2 size, int resolution);
void DrawCheckerboard(ImDrawList* pDrawList, ImVec2 position, ImVec2 size,
                      float cellSize, ImU32 col1, ImU32 col2);
```

---

## OkLab / OkLCH Quads

```cpp
void DrawOkLabQuad(ImDrawList* pDrawList, ImVec2 start, ImVec2 size,
                   float L, int resX = 16, int resY = 16);
void DrawOkLchQuad(ImDrawList* pDrawList, ImVec2 start, ImVec2 size,
                   float L, int resX = 16, int resY = 16);
```
Draws a 2D grid of colors at a fixed lightness `L`, covering the `a/b` (OkLab) or `C/H` (OkLCH) plane.

---

## Cursors

```cpp
void DrawTriangleCursor(ImDrawList* pDrawList, ImVec2 targetPoint, float angle,
                        float size, float thickness, ImU32 col);
void DrawTriangleCursorFilled(ImDrawList* pDrawList, ImVec2 targetPoint,
                               float angle, float size, ImU32 col);
void DrawSignetCursor(ImDrawList* pDrawList, ImVec2 targetPoint,
                      float width, float height, float height_ratio, float align01,
                      float angle, float thickness, ImU32 col);
void DrawSignetFilledCursor(ImDrawList* pDrawList, ImVec2 targetPoint,
                             float width, float height, float height_ratio,
                             float align01, float angle, ImU32 col);
```

---

## Markers

> **Requires:** `ImWidgetsFeatures_Markers` and `IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER`.

```cpp
void DrawMarker(ImDrawList* pDrawList, ImVec2 start, ImVec2 size,
                ImU32 fg_color, ImU32 bg_color,
                float rot_angle_rad, float shape_size, float linewidth,
                float antialiasing,
                ImWidgetsMarker marker, ImWidgetsDrawType draw_type);
```

**Marker shapes (`ImWidgetsMarker`):** `Disc`, `Square`, `Triangle`, `Diamond`, `Heart`, `Spade`, `Club`, `Chevron`, `Clover`, `Ring`, `Tag`, `Cross`, `Asterisk`, `Infinity`, `Pin`, `Arrow`, `Ellipse`, `EllipseApprox`

**Draw types (`ImWidgetsDrawType`):** `Filled`, `Stroke`, `Outline`, `SignedDistanceField`, `CutOff`

---

## Chromatic Plots

```cpp
void DrawChromaticityPlot(ImDrawList* draw,
                           ImWidgetsIlluminance illuminance,
                           ImWidgetsObserver observer,
                           ImWidgetsColorSpace colorSpace,
                           int chromeLineSamplesCount,
                           ImVec2 vpos, ImVec2 size,
                           int resolutionX, int resolutionY,
                           ImU32 maskColor, ...);
void DrawChromaticityPoints(ImDrawList* pDrawList, ImVec2 curPos, ImVec2 size,
                             ImU32* colors4, int color_count,
                             float minX, float maxX, float minY, float maxY,
                             ImU32 plotColor, float radius, int num_segments);
void DrawChromaticityLines(ImDrawList* pDrawList, ImVec2 curPos, ImVec2 size,
                            ImU32* color, int color_count,
                            float minX, float maxX, float minY, float maxY,
                            ImU32 plotColor, ImDrawFlags flags, float thickness);
```
Generic overloads accept custom `rgbToXYZ` matrices and color strides.

---

## Graduation Lines

```cpp
void DrawLinearLineGraduation(ImDrawList* drawlist, ImVec2 start, ImVec2 end,
    float mainLineThickness, ImU32 mainCol,
    int division0, float height0, float thickness0, float angle0, ImU32 col0,
    int division1 = -1, ...,
    int division2 = -1, ...);
void DrawLinearCircularGraduation(ImDrawList* drawlist, ImVec2 center, float radius,
    float start_angle, float end_angle, int num_segments, ...);
void DrawLogLineGraduation(ImDrawList* drawlist, ImVec2 start, ImVec2 end, ...);
void DrawLogCircularGraduation(ImDrawList* drawlist, ImVec2 center, float radius, ...);
```
Draw tick-marked scale lines (linear or log spacing) along a straight or circular path. Up to three tick subdivisions.

---

## Dashed Polylines

```cpp
// Multi-dash pattern
void DrawDashedPolylineAA(ImDrawList* drawlist,
    const ImVec2* points, int points_count,
    ImU32 col, float thickness,
    const float* dashes, int dashes_count, float dash_offset,
    bool closed = false,
    ImWidgetsCap cap = ImWidgetsCap_Butt,
    ImWidgetsJoin join = ImWidgetsJoin_Mitter,
    float miter_limit = 4.0f);

// Single on/off convenience
void DrawDashedPolylineAA(ImDrawList* drawlist,
    const ImVec2* points, int points_count,
    ImU32 col, float thickness,
    float dash_len, float gap_len, float dash_offset,
    bool closed = false, ...);
```

`dashes` is an alternating `[on, off, on, off, …]` pattern in pixels.

Cap types: `None`, `Butt`, `Square`, `Round`, `TriangleOut`, `TriangleIn`
Join types: `Round`, `Mitter`, `Bevel`

GPU acceleration: enable with `SetDashedLinesUseGPU(true)` (requires shader support). CPU fallback is always available.

---

## Custom Render Frames & Nav Cursors

```cpp
void RenderFrameCircle(ImVec2 center, float radius, ImU32 fill_col, bool border);
void RenderFrameConvex(ImVec2* pts, int pts_count, ImU32 fill_col, bool border);
void RenderFrameConcave(ImVec2* pts, int pts_count, ImU32 fill_col, bool border);
void RenderFrameWithHole(ImVec2* pts, int pts_count, ImU32 fill_col, bool border);

void RenderNavCursorCircle(ImVec2 center, float radius, ImGuiID id, ImGuiNavRenderCursorFlags flags = 0);
void RenderNavCursorConvex(ImVec2* pts, int pts_count, ImGuiID id, ImGuiNavRenderCursorFlags flags = 0);
void RenderNavCursorConcave(ImVec2* pts, int pts_count, ImGuiID id, ImGuiNavRenderCursorFlags flags = 0);
```

---

## Window Customization

```cpp
void SetCurrentWindowBackgroundImage(ImTextureID id, ImVec2 imgSize,
                                      bool fixedSize = false,
                                      ImU32 col = IM_COL32(255,255,255,255));
```
Replace the current window's background with a texture. Note: breaks corner rounding.
