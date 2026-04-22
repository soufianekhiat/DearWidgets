# Widgets

All widget functions follow Dear ImGui conventions:
- Return `bool` -- `true` when the value changed this frame.
- `label` is used as the ImGui ID; use `##hidden` suffix to hide it.
- `size = ImVec2(0,0)` means "auto size" (fill available width, or use the style default height).

---

## Sliders

### `Slider2DFloat` / `Slider2DInt` / `Slider2DScalar`
```cpp
bool Slider2DFloat(const char* label, float* pValueX, float* pValueY,
                   float v_minX, float v_maxX, float v_minY, float v_maxY);
bool Slider2DInt(const char* label, int* pValueX, void* pValueY,
                 int v_minX, int v_maxX, int v_minY, int v_maxY);
bool Slider2DScalar(const char* label, ImGuiDataType data_type,
                    void* pValueX, void* pValueY,
                    void* p_minX, void* p_maxX, void* p_minY, void* p_maxY);
```
2D pad widget. Click-drag to move the cursor. Style vars: `StyleVar_Slider2D_*`.

---

### `SliderNFloat` / `SliderNInt` / `SliderNScalar`
```cpp
bool SliderNFloat(const char* label, float* ordered_value, int value_count,
                  float v_min, float v_max, float cursor_width, bool show_hover_by_region);
bool SliderNInt(const char* label, int* ordered_value, int value_count,
                int v_min, int v_max, float cursor_width, bool show_hover_by_region);
bool SliderNScalar(const char* label, ImGuiDataType data_type,
                   void* ordered_value, int value_count,
                   void* p_min, void* p_max, float cursor_width, bool show_hover_by_region);
```
Multi-handle ordered slider. Values remain sorted; dragging a handle pushes neighbors.

---

### `SliderRingFloat` / `SliderRingInt` / `SliderRingScalar`
```cpp
bool SliderRingFloat(const char* label, float* value, float v_min, float v_max,
                     float v_angle_min = -0.75f*IM_PI, float v_angle_max = 0.75f*IM_PI,
                     float v_thickness = 0.0f, const char* format = "%.3f",
                     ImGuiSliderFlags flags = 0);
bool SliderRingInt(const char* label, int* value, int v_min, int v_max, ...);
bool SliderRingScalar(const char* label, ImGuiDataType data_type,
                      void* p_value, void* p_min, void* p_max, ...);
```
Circular arc slider. `v_angle_min/max` control the arc sweep in radians (default: +/-135 deg). Style vars: `StyleVar_SliderRing_*`.

---

### `SliderSplineFloat` / `SliderSplineInt` / `SliderSplineScalar`
```cpp
bool SliderSplineFloat(const char* label, float* value, float v_min, float v_max,
                       const ImVec2* control_points = NULL, int num_points = 4,
                       float v_height = 0.0f, float v_thickness = 0.0f,
                       const char* format = "%.3f", ImGuiSliderFlags flags = 0);
```
Slider whose track follows a cubic Bezier spline. `control_points` is an array of `ImVec2` in normalized `[0,1]x[0,1]` space. `num_points = 4` = single segment; `3N+1` = N chained segments. `NULL` uses a default S-curve. Style vars: `StyleVar_SliderSpline_*`.

---

### `DragFloatPrecise`
```cpp
bool DragFloatPrecise(const char* label, float* value,
                      float v_min = 0.0f, float v_max = 0.0f,
                      const char* format = NULL, ImGuiSliderFlags flags = 0);
```
`DragFloat` with a precision-drag mode: hold while over the block grid to nudge by small increments. Block size: `StyleVar_PrecisionDrag_BlockSize`.

---

### `SliderGradientFloat` / `SliderGradientInt` / `SliderGradientScalar`
```cpp
bool SliderGradientFloat(const char* label, float* v, float v_min, float v_max,
                         const ImGradientData* gradient, ImVec2 size = {},
                         bool fill_up_to_cursor = false, bool right_to_left = false);
bool SliderGradientInt(const char* label, int* v, int v_min, int v_max,
                       const ImGradientData* gradient, ImVec2 size = {},
                       bool fill_up_to_cursor = false, bool right_to_left = false);
bool SliderGradientScalar(const char* label, ImGuiDataType data_type,
                          void* p_value, const void* p_min, const void* p_max,
                          const ImGradientData* gradient, ImVec2 size = {},
                          bool fill_up_to_cursor = false, bool right_to_left = false);
```
Horizontal gradient slider. The track is filled with a full `ImGradientData` gradient. When `fill_up_to_cursor` is true, only the `[0, cursor]` portion is painted; the rest is transparent (the FrameBg shows through). Style vars: `StyleVar_SliderSpline_*`.

---

### `SliderGradientRangeFloat` / `SliderGradientRangeInt` / `SliderGradientRangeScalar`
```cpp
bool SliderGradientRangeFloat(const char* label, float* v_lower, float* v_upper,
                              float v_min, float v_max,
                              const ImGradientData* gradient, ImVec2 size = {},
                              bool right_to_left = false);
bool SliderGradientRangeInt(const char* label, int* v_lower, int* v_upper,
                            int v_min, int v_max,
                            const ImGradientData* gradient, ImVec2 size = {},
                            bool right_to_left = false);
bool SliderGradientRangeScalar(const char* label, ImGuiDataType data_type,
                               void* p_lower, void* p_upper,
                               const void* p_min, const void* p_max,
                               const ImGradientData* gradient, ImVec2 size = {},
                               bool right_to_left = false);
```
Two-handle gradient slider. The gradient is painted only between the two handles; the FrameBg shows through elsewhere. Clicking picks the nearest handle; dragging it is clamped so `lower <= upper`.

---

### `SliderGradientRingFloat` / `SliderGradientRingInt` / `SliderGradientRingScalar`
```cpp
bool SliderGradientRingFloat(const char* label, float* v, float v_min, float v_max,
                             const ImGradientData* gradient,
                             float outerRadius, float thickness,
                             float startAngle = -0.75f*IM_PI, float sweepAngle = 1.5f*IM_PI,
                             bool fill_up_to_cursor = false);
```
Ring (arc) gradient slider. `startAngle` / `sweepAngle` define the arc; positive sweep is clockwise. `fill_up_to_cursor` paints only the portion up to the handle.

---

### `SliderGradientRingRangeFloat` / `SliderGradientRingRangeInt` / `SliderGradientRingRangeScalar`
```cpp
bool SliderGradientRingRangeFloat(const char* label, float* v_lower, float* v_upper,
                                  float v_min, float v_max,
                                  const ImGradientData* gradient,
                                  float outerRadius, float thickness,
                                  float startAngle, float sweepAngle);
```
Two-handle ring gradient slider. Gradient is painted between the two handles on the arc. Full-circle rings are not supported by the range variant (ambiguous wrap semantics) -- use two `SliderGradientRing` calls instead.

---

### `SliderSplineGradientFloat` / `SliderSplineGradientInt` / `SliderSplineGradientScalar`
```cpp
bool SliderSplineGradientFloat(const char* label, float* v, float v_min, float v_max,
                               const ImGradientData* gradient,
                               const ImVec2* control_points = NULL, int num_points = 4,
                               float v_height = 0.f, float v_thickness = 0.f,
                               const char* format = "%.3f",
                               ImGuiSliderFlags flags = 0);
```
Spline-track gradient slider. Same spline parametrization as `SliderSplineFloat`. The track is painted with the gradient along the arc.

---

### `SliderSplineGradientRangeFloat` / `SliderSplineGradientRangeInt` / `SliderSplineGradientRangeScalar`
```cpp
bool SliderSplineGradientRangeFloat(const char* label, float* v_lower, float* v_upper,
                                    float v_min, float v_max,
                                    const ImGradientData* gradient,
                                    const ImVec2* control_points = NULL, int num_points = 4,
                                    float v_height = 0.f, float v_thickness = 0.f,
                                    const char* format = "%.3f",
                                    ImGuiSliderFlags flags = 0);
```
Two-handle spline gradient slider. Gradient paints between handles; FrameBg shows elsewhere.

---

## Color Editors

### `HueSelector`
```cpp
bool HueSelector(const char* label, float hueHeight, float cursorHeight,
                 float* hueCenter, float* hueWidth,
                 float* featherLeft, float* featherRight,
                 int division = 32, float alpha = 1.0f,
                 float hideHueAlpha = 0.75f, float offset = 0.0f);
```
Horizontal hue-range selector. Edits a center hue, width, and left/right feather.

---

### `GradientEditor`
```cpp
bool GradientEditor(const char* label, ImGradientData* gradient,
                    bool alpha = true, ImVec2 size = ImVec2(0,0));
```
Interactive multi-stop gradient editor. Left-click on the bar to add a stop; right-click a stop to delete; drag stops to reorder. See [data-types.md -- ImGradientData](data-types.md#imgradientdata).

---

### `CurveEditor`
```cpp
bool CurveEditor(const char* label, ImCurveEditorData* curve,
                 ImVec2 size = ImVec2(0,0));
```
Interactive 2D animation curve editor with full Bezier handles. Left-click to add keys; Delete to remove. See [data-types.md -- ImCurveEditorData](data-types.md#imcurveeditordata).

---

### `ColorWheel`
```cpp
bool ColorWheel(const char* label, ImVec4* color,
                ImColorWheelMode mode = ImColorWheelMode_HSV,
                float hdr_max = 1.0f, bool fixedIntensity = false,
                ImVec2 size = ImVec2(0,0));
```
Hue-chroma disc + value/lightness slider. `mode`: `HSV` or `OkLCH` (perceptual). `hdr_max > 1` enables HDR range. Style vars: `StyleVar_ColorWheel_*`.

---

### `PrimariesWheel`
```cpp
bool PrimariesWheel(const char* label, ImVec4* color, float* yValue,
                    float yMin, float yMax,
                    ImColorWheelMode mode = ImColorWheelMode_HSV,
                    float ringThickness = 12.0f, ImVec2 size = ImVec2(0,0));
```
Color wheel with a separate linear Y (luminance) output. Useful for shadow/midtone/highlight color pickers.

---

### `HDRWheel`
```cpp
bool HDRWheel(const char* label, ImVec4* color, float* yValue, float yMin, float yMax,
              float* rightValue, float rightMin, float rightMax,
              float* leftValue, float leftMin, float leftMax,
              ImColorWheelMode mode = ImColorWheelMode_HSV,
              float ringThickness = 12.0f, ImVec2 size = ImVec2(0,0));
```
Color wheel with two arc sliders (left and right of the disc) for extra HDR parameters (e.g. saturation, exposure). Style colors: `StyleColor_HDRWheel_*`.

---

### `ColorPicker` family
```cpp
bool ColorPicker(const char* label, ImVec4* color,
                 ImColorPickerSpace space = ImColorPickerSpace_sRGB,
                 int fixedAxis = 2, ImVec2 size = ImVec2(0,0));

bool ColorPickerSRGB(const char* label, ImVec4* color, int fixedAxis = 2, ImVec2 size = {});
bool ColorPickerHSV(const char* label, ImVec4* color, ImVec2 size = {});
bool ColorPickerOkLab(const char* label, ImVec4* color, ImVec2 size = {});
bool ColorPickerOkLCH(const char* label, ImVec4* color, ImVec2 size = {});
bool ColorPickerCIELab(const char* label, ImVec4* color, ImVec2 size = {});
bool ColorPickerXYZ(const char* label, ImVec4* color, ImVec2 size = {});
```
3D color picker: 2D plane + 1D slider. `fixedAxis`: 0=X fixed, 1=Y fixed, 2=Z fixed. Color spaces: sRGB, HSV, OkLab, OkLCH, CIELab, XYZ.

---

### `ColorWarper`
```cpp
bool ColorWarper(const char* label, ImColorWarperData* data,
                 ImColorWarperMode mode = ImColorWarperMode_Circular,
                 ImColorWarperSpace space = ImColorWarperSpace_HSV,
                 float thirdAxis = 1.0f,
                 const ImColorWarperOverlay* signalOverlay = NULL,
                 ImColorWarperSignalColor signalColor = ImColorWarperSignalColor_PixelColor,
                 float axisAngle = 0.0f, ImVec2 size = ImVec2(0,0));
```
Mesh warp color grader. Drag control points on a hue/saturation grid to shift colors. Modes: `Circular` (polar disc), `Square` (cartesian), `ChromaLuma` (two-square layout). See [data-types.md -- ImColorWarperData](data-types.md#imcolorwarperdata).

---

### `ColorCurve`
```cpp
bool ColorCurve(const char* label, ImColorCurveData* curve,
                ImColorCurveMode mode,
                const ImHistogramData* histogramOverlay = NULL,
                bool advancedSegments = false, ImVec2 size = ImVec2(0,0));
```
Single-axis color curve editor. Modes: `HueVsHue`, `HueVsSat`, `HueVsLum`, `LumVsSat`, `SatVsSat`. Optional histogram underlay.

---

## Scopes & Analysis

### `ParadeScope`
```cpp
void ParadeScope(const char* label, const ImParadeScopeData& data,
                 bool overlay = false,
                 ImParadeScale scale = ImParadeScale_Linear,
                 ImVec2 size = ImVec2(0,0));
```
Waveform parade (Luma, RGB, YRGB, YCbCr). `overlay`: draw all channels overlapped. Fill `ImParadeScopeData` via `data.Accumulate(...)`.

---

### `VectorScope`
```cpp
void VectorScope(const char* label, const ImVectorScopeData& data,
                 bool showSkinToneLine = true, ImVec2 size = ImVec2(0,0));
```
Cb/Cr chrominance scatter plot with target boxes and graticule. Fill via `data.Accumulate(...)`.

---

### `Histogram`
```cpp
void Histogram(const char* label, const ImHistogramData& data,
               ImHistogramLayout layout = ImHistogramLayout_Overlapped,
               ImParadeScale xScale = ImParadeScale_Linear,
               ImParadeScale yScale = ImParadeScale_Linear,
               ImVec2 size = ImVec2(0,0));
```
Per-channel histogram. Modes: Luma, RGB, YRGB, YCbCr, HSV, OkLCH. Fill via `data.Accumulate(...)`.

---

### `CIEChromaticity`
```cpp
void CIEChromaticity(const char* label, const ImCIEChromaticityData& data,
                     ImCIEChromaticityGamut gamut = ImCIEChromaticityGamut_sRGB_Rec709,
                     bool showBackground = false,
                     ImCIEChromaticitySignalColor signalColor = ImCIEChromaticitySignalColor_Flat,
                     ImVec2 size = ImVec2(0,0));
```
CIE 1931 xy chromaticity plot with gamut overlay. Fill via `data.Accumulate(...)`.

---

### `ToneCurve`
```cpp
bool ToneCurve(const char* label, ImToneCurveData* curve,
               ImHistogramMode mode,
               const ImHistogramData* histogramOverlay = NULL,
               bool advancedSegments = false, ImVec2 size = ImVec2(0,0));
```
Multi-channel tone curve editor (up to 4 channels) with histogram underlay. Channel count is determined by `mode`.

---

## Utility Widgets

### `UnitField`
```cpp
bool UnitField(const char* label, float* pValue,
               ImUnitDef* units, int unitCount, int* pSelectedUnit,
               float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
               const char* format = NULL);
```
DragFloat combined with a unit selector. The stored value is always in the base unit; the display converts via `ImUnitDef`. Define units with `ImUnitDef_Simple()` or `ImUnitDef_Custom()`.

---

### `PaintCanvas`
```cpp
bool PaintCanvas(const char* label, ImPaintCanvasData* canvas,
                 ImVec2 size = ImVec2(0,0));
```
Interactive CPU paint canvas with brush, eraser, hard/soft brush types. Pixel data is owned by the caller (`canvas->Pixels`). Supports `Mask`, `Grayscale`, and `Color` modes. Requires ImPlatform for texture upload.

---

### `UpVector`
```cpp
bool UpVector(const char* label, float* direction, int defaultUpAxis = 1,
              ImVec2 size = ImVec2(0,0));
```
Hemisphere picker. Drag to set a unit-vector direction. `defaultUpAxis`: 0=X, 1=Y, 2=Z.

---

### `ImageTransformGizmo`
```cpp
bool ImageTransformGizmo(const char* label,
                         ImTransformImage* images, int imageCount, int* selectedIndex,
                         ImTransformGizmoFlags flags = ImTransformGizmoFlags_None,
                         ImVec2 canvasSize = ImVec2(0,0));
```
2D image transform gizmo with translation, rotation, and scale handles. Each `ImTransformImage` holds a texture + `ImTransformData`. `ImTransformGizmoFlags_NonUniformScale` enables edge-midpoint handles.

---

### `ImageCarousel`
```cpp
bool ImageCarousel(const char* label, ImTextureID* images, ImVec2* imageSizes,
                   int imageCount, int* pSelectedIndex, ImVec2 size = ImVec2(0,0));
```
Horizontal scrolling image strip. Returns `true` when the selection changes.

---

### `ImageBento`
```cpp
bool ImageBento(const char* label, ImTextureID* images, ImVec2* imageSizes,
                int imageCount, int* pSelectedIndex,
                int columnsPerRow = 4, float cellAspect = 1.0f, float spacing = 4.0f,
                const char* const* pItemIds = nullptr,
                int* pReorderFrom = nullptr, int* pReorderTo = nullptr);
```
Grid thumbnail gallery with center-crop to `cellAspect` ratio.

**Drag-to-reorder (optional):** pass a `pItemIds` array of stable per-cell C-strings so each cell's ImGui ID follows the item rather than the position -- required for clean drag-and-drop. When the user drags a cell over an adjacent cell, `*pReorderFrom`/`*pReorderTo` are set to the source/target indices and the function returns `true`. The caller is responsible for actually swapping the data -- `ImageBento` never mutates the arrays. Swaps are reported one adjacent step at a time (`|from-to| == 1` horizontally, or `== columnsPerRow` vertically); multi-cell drags accumulate as multiple single-frame reports.

---

### `ImageViewer`
```cpp
bool ImageViewer(const char* label, ImTextureID image, ImVec2 imageSize,
                 ImImageViewerState& state, ImVec2 widgetSize = ImVec2(0,0));
```
Pan (left-drag) and zoom (scroll wheel) image viewer. Double-click resets. Right-click opens a pixel-inspector loupe showing RGBA values when `state.Pixels` is set.

---

### `VectorDrawingTool`
```cpp
bool VectorDrawingTool(const char* label, ImVectorDrawingData& data, ImVec2 size = {});
```
Interactive bezier path authoring canvas with zoom/pan. Click to place anchors; drag tangent handles to shape curves; click the first anchor to close a path. Multiple paths are stored in `data.Paths`; `data.SelectedPath` / `data.SelectedNode` track the active selection.

**Key types:**
- `ImVectorDrawingData` -- canvas state: paths, pan/zoom, selection.
- `ImVectorDrawingPath` -- one path: `Nodes`, `Color`, `Thickness`, `Style`, `DashLen`, `GapLen`, `Cap`, `Join`, `Closed`.
- `ImVectorDrawingNode` -- one anchor: `Anchor`, `InTangent`, `OutTangent`, `Broken`.
- `ImVectorDrawingStyle_` -- `Polyline`, `PolylineAA`, `StrokedBezier`, `StrokedDashedBezier`, `DashedPolyline`.

`Cap` and `Join` accept `ImWidgetsCap_*` and `ImWidgetsJoin_*` values respectively. The plain `Polyline` style ignores cap/join.

---

## Custom Buttons

```cpp
bool ButtonExCircle(const char* label, float radius, ImGuiButtonFlags flags);
bool ButtonExCapsuleH(const char* label, float length, float thickness, ImGuiButtonFlags flags);
bool ButtonExCapsuleV(const char* label, float length, float thickness, ImGuiButtonFlags flags);
bool ButtonExConvex(const char* label, const ImVec2& size, ImVec2* pts, int pts_count, ImGuiButtonFlags flags);
bool ButtonExConcave(const char* label, const ImVec2& size, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags);
bool ButtonExWithHole(const char* label, const ImVec2& size, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags);
```
Buttons with non-rectangular shapes. Hit-testing uses the actual shape, not the bounding box.

Image variants:
```cpp
bool ImageButtonExCircle(const char* label, ImTextureID tex, float radius, ...);
bool ImageButtonExCapsuleH(...);
bool ImageButtonExCapsuleV(...);
bool ImageButtonExConvex(...);
bool ImageButtonExConcave(...);
```
