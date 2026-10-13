# Widgets

All widget functions live in `namespace ImWidgets`. All *types* are at global
scope (2.0.0).

Conventions:

- Return `bool` -- `true` when the value changed this frame. A handful of
  display-only widgets return `void` (`ParadeScope`, `VectorScope`, `Histogram`,
  `CIEChromaticity`, `FontInspector`, `ColorDifferenceVisualizer`, the read-only
  image overlays).
- `label` is used as the ImGui ID; use a `##hidden` suffix to hide it.
- `size = ImVec2(0,0)` means "auto size": fill the available width and use the
  style default height (`ImWidgetsStyleVar_<Widget>_DefaultHeight` /
  `_DefaultSize`).
- Style colours are `ImWidgetsCol_*`, metrics `ImWidgetsStyleVar_*` -- see
  [style.md](style.md).

---

## Sliders

### `Slider2DFloat` / `Slider2DInt` / `Slider2DScalar`
```cpp
bool Slider2DFloat ( char const* pLabel, float* pValueX, float* pValueY,
                     float v_minX, float v_maxX, float v_minY, float v_maxY );
bool Slider2DInt   ( char const* pLabel, int* pValueX, int* pValueY,
                     int v_minX, int v_maxX, int v_minY, int v_maxY );
bool Slider2DScalar( char const* pLabel, ImGuiDataType data_type,
                     void* pValueX, void* pValueY,
                     void* p_minX, void* p_maxX, void* p_minY, void* p_maxY );
```
2D pad. Click-drag to move the cursor; double-click opens a typed-input
precision popup. Style vars: `ImWidgetsStyleVar_Slider2D_*`.

> **Fixed in 2.0.0.** `Slider2DInt`'s `pValueY` used to be typed `void*`.

---

### `Slider2DRangeFloat` / `Slider2DRangeInt` / `Slider2DRangeScalar`
```cpp
bool Slider2DRangeFloat ( char const* label, float* pMinX, float* pMinY, float* pMaxX, float* pMaxY,
                          float bMinX, float bMaxX, float bMinY, float bMaxY );
bool Slider2DRangeInt   ( char const* label, int* pMinX, int* pMinY, int* pMaxX, int* pMaxY,
                          int bMinX, int bMaxX, int bMinY, int bMaxY );
bool Slider2DRangeScalar( char const* label, ImGuiDataType data_type,
                          void* pMinX, void* pMinY, void* pMaxX, void* pMaxY,
                          void* bMinX, void* bMaxX, void* bMinY, void* bMaxY );
```
2D pad editing a rectangular *range*: two corner handles plus a draggable fill.
`b*` are the bounds. Style: `ImWidgetsCol_Slider2DRange_*`,
`ImWidgetsStyleVar_Slider2DRange_*`.

---

### `Slider2DDiscFloat` / `Slider2DDiscInt` / `Slider2DDiscScalar`
```cpp
bool Slider2DDiscFloat ( char const* label, float* pValueX, float* pValueY, float v_min, float v_max );
bool Slider2DDiscInt   ( char const* label, int* pValueX, int* pValueY, int v_min, int v_max );
bool Slider2DDiscScalar( char const* label, ImGuiDataType data_type,
                         void* pValueX, void* pValueY, void* p_min, void* p_max );
```
Circular 2D pad: the value is constrained to the unit disc and drawn over a
crosshair plus concentric rings. Style: `ImWidgetsCol_Slider2DDisc_*`,
`ImWidgetsStyleVar_Slider2DDisc_*`, `GetStyle().Slider2DDisc_GridRings`.

---

### `SliderNFloat` / `SliderNInt` / `SliderNScalar`
```cpp
bool SliderNFloat ( char const* label, float* ordered_value, int value_count,
                    float v_min, float v_max, float cursor_width, bool show_hover_by_region );
bool SliderNInt   ( char const* label, int* ordered_value, int value_count,
                    int v_min, int v_max, float cursor_width, bool show_hover_by_region );
bool SliderNScalar( char const* label, ImGuiDataType data_type,
                    void* ordered_value, int value_count,
                    void* p_min, void* p_max, float cursor_width, bool show_hover_by_region );
```
Multi-handle ordered slider. Values remain sorted; dragging a handle pushes its
neighbours. Boundary cursors use `ImWidgetsCol_SliderN_Cursor` *(new in 2.0.0;
it was a hard-coded red)*.

### `SliderNVerticalFloat` / `SliderNVerticalInt` / `SliderNVerticalScalar`
```cpp
bool SliderNVerticalFloat ( char const* label, float* ordered_value, int value_count,
                            float v_min, float v_max, float cursor_height,
                            bool show_hover_by_region, ImVec2 size = ImVec2( 0, 0 ) );
bool SliderNVerticalInt   ( char const* label, int* ordered_value, int value_count,
                            int v_min, int v_max, float cursor_height,
                            bool show_hover_by_region, ImVec2 size = ImVec2( 0, 0 ) );
bool SliderNVerticalScalar( char const* label, ImGuiDataType data_type,
                            void* ordered_value, int value_count, void* p_min, void* p_max,
                            float cursor_height, bool show_hover_by_region,
                            ImVec2 size = ImVec2( 0, 0 ) );
```
Vertical sibling of `SliderN*`.

---

### `SliderRingFloat` / `SliderRingInt` / `SliderRingScalar`
```cpp
bool SliderRingFloat ( char const* label, float* value, float v_min, float v_max,
                       float v_angle_min = -0.75f * IM_PI, float v_angle_max = 0.75f * IM_PI,
                       float v_thickness = 0.0f, char const* format = "%.3f",
                       ImGuiSliderFlags flags = 0 );
bool SliderRingInt   ( char const* label, int* value, int v_min, int v_max, ... );
bool SliderRingScalar( char const* label, ImGuiDataType data_type,
                       void* p_value, void* p_min, void* p_max, ... );
```
Circular arc slider; `v_angle_min/max` are radians (default +/-135 deg).
`v_thickness = 0` uses `ImWidgetsStyleVar_SliderRing_TrackThickness`.

### `SliderRingFloatEx`
```cpp
bool SliderRingFloatEx( char const* label, float* value, float v_min, float v_max,
                        float v_angle_min, float v_angle_max, float v_thickness,
                        char const* format, ImWidgetsSliderFlags widgets_flags,
                        ImGuiSliderFlags imgui_flags = 0 );
```
`SliderRingFloat` plus DearWidgets-specific behaviour flags:

```cpp
enum ImWidgetsSliderFlags_
{
    ImWidgetsSliderFlags_None             = 0,
    ImWidgetsSliderFlags_AngleWrap,        // value wraps through the arc ends
    ImWidgetsSliderFlags_AngleClamp,       // value stops at the arc ends (default)
    ImWidgetsSliderFlags_ShowDelta,        // delta overlay while dragging
    ImWidgetsSliderFlags_ProportionalDrag, // follow the enclosing PushDragGroup (auto slot, submission order)
    ImWidgetsSliderFlags_Stepped           // snap to the format's precision
};
```

---

### `SliderSplineFloat` / `SliderSplineInt` / `SliderSplineScalar`
```cpp
bool SliderSplineFloat ( char const* label, float* value, float v_min, float v_max,
                         ImVec2 const* control_points = NULL, int num_points = 4,
                         float v_height = 0.0f, float v_thickness = 0.0f,
                         char const* format = "%.3f", ImGuiSliderFlags flags = 0 );
bool SliderSplineInt   ( char const* label, int* value, int v_min, int v_max,
                         ImVec2 const* control_points = NULL, int num_points = 4,
                         float v_height = 0.0f, float v_thickness = 0.0f,
                         char const* format = "%d", ImGuiSliderFlags flags = 0 );
bool SliderSplineScalar( char const* label, ImGuiDataType data_type,
                         void* p_value, void* p_min, void* p_max,
                         ImVec2 const* control_points = NULL, int num_points = 4,
                         float v_height = 0.0f, float v_thickness = 0.0f,
                         char const* format = NULL, ImGuiSliderFlags flags = 0 );
```
Slider whose track follows a cubic Bezier spline. `control_points` is in
normalized `[0,1]^2`; `num_points = 4` is one segment, `3N+1` is N chained
segments, `NULL` uses a default S-curve. Style vars:
`ImWidgetsStyleVar_SliderSpline_*`.

---

### Gradient sliders -- horizontal
```cpp
bool SliderGradientFloat ( char const* label, float* v, float v_min, float v_max,
                           ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ),
                           bool fill_up_to_cursor = false, bool right_to_left = false );
bool SliderGradientInt   ( char const* label, int* v, int v_min, int v_max,
                           ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ),
                           bool fill_up_to_cursor = false, bool right_to_left = false );
bool SliderGradientScalar( char const* label, ImGuiDataType data_type,
                           void* p_value, void const* p_min, void const* p_max,
                           ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ),
                           bool fill_up_to_cursor = false, bool right_to_left = false );
```
The track is painted with a full `ImGradientData`. With `fill_up_to_cursor`,
only `[0, cursor]` is painted and the FrameBg shows through the rest.

### Gradient sliders -- vertical
```cpp
bool SliderGradientVerticalFloat ( char const* label, float* v, float v_min, float v_max,
                                   ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ),
                                   ImWidgetsSliderFill fill = ImWidgetsSliderFill_None,
                                   bool inverted = false );
bool SliderGradientVerticalInt   ( ... int* v, int v_min, int v_max, ... );
bool SliderGradientVerticalScalar( char const* label, ImGuiDataType data_type,
                                   void* p_value, void const* p_min, void const* p_max,
                                   ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ),
                                   ImWidgetsSliderFill fill = ImWidgetsSliderFill_None,
                                   bool inverted = false );
```
```cpp
enum ImWidgetsSliderFill_ { ImWidgetsSliderFill_None, ImWidgetsSliderFill_FromMin, ImWidgetsSliderFill_FromCenter };
```

### Gradient sliders -- two-handle range
```cpp
bool SliderGradientRangeFloat ( char const* label, float* v_lower, float* v_upper,
                                float v_min, float v_max, ImGradientData const* gradient,
                                ImVec2 size = ImVec2( 0, 0 ), bool right_to_left = false );
bool SliderGradientRangeInt   ( char const* label, int* v_lower, int* v_upper,
                                int v_min, int v_max, ImGradientData const* gradient,
                                ImVec2 size = ImVec2( 0, 0 ), bool right_to_left = false );
bool SliderGradientRangeScalar( char const* label, ImGuiDataType data_type,
                                void* p_lower, void* p_upper, void const* p_min, void const* p_max,
                                ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ),
                                bool right_to_left = false );
```
The gradient is painted only between the handles. Clicking latches the nearest
handle; dragging is clamped so `lower <= upper`.

### Gradient sliders -- full ring
```cpp
bool SliderGradientRingFloat ( char const* label, float* v, float v_min, float v_max,
                               ImGradientData const* gradient, float outerRadius, float thickness,
                               bool fill_up_to_cursor = false );
bool SliderGradientRingInt   ( char const* label, int* v, int v_min, int v_max, ... );
bool SliderGradientRingScalar( char const* label, ImGuiDataType data_type,
                               void* p_value, void const* p_min, void const* p_max,
                               ImGradientData const* gradient, float outerRadius, float thickness,
                               bool fill_up_to_cursor = false );
```

### Gradient sliders -- arc
```cpp
bool SliderGradientArcFloat ( char const* label, float* v, float v_min, float v_max,
                              ImGradientData const* gradient, float outerRadius, float thickness,
                              float startAngle, float sweepAngle, bool fill_up_to_cursor = false );
bool SliderGradientArcInt   ( char const* label, int* v, int v_min, int v_max, ... );
bool SliderGradientArcScalar( char const* label, ImGuiDataType data_type,
                              void* p_value, void const* p_min, void const* p_max,
                              ImGradientData const* gradient, float outerRadius, float thickness,
                              float startAngle, float sweepAngle, bool fill_up_to_cursor = false );

bool SliderGradientArcRangeFloat ( char const* label, float* v_lower, float* v_upper,
                                   float v_min, float v_max, ImGradientData const* gradient,
                                   float outerRadius, float thickness,
                                   float startAngle, float sweepAngle );
bool SliderGradientArcRangeInt   ( char const* label, int* v_lower, int* v_upper, ... );
bool SliderGradientArcRangeScalar( char const* label, ImGuiDataType data_type,
                                   void* p_lower, void* p_upper, void const* p_min, void const* p_max,
                                   ImGradientData const* gradient, float outerRadius, float thickness,
                                   float startAngle, float sweepAngle );
```

> **Renamed in 2.0.0.** These used to be `SliderGradientRing*` /
> `SliderGradientRingRange*` overloads taking two extra trailing floats, where a
> forgotten `sweepAngle` silently bound a `float` to `bool fill_up_to_cursor`.
>
> Full-circle arcs have ambiguous wrap semantics for a two-handle range; use two
> `SliderGradientRing*` calls instead.

### Gradient sliders -- spline track
```cpp
bool SliderSplineGradientFloat ( char const* label, float* value, float v_min, float v_max,
                                 ImGradientData const* gradient,
                                 ImVec2 const* control_points = NULL, int num_points = 4,
                                 float v_height = 0.0f, float v_thickness = 0.0f,
                                 char const* format = "%.3f",
                                 bool fill_up_to_cursor = false, ImGuiSliderFlags flags = 0 );
bool SliderSplineGradientInt   ( char const* label, int* value, int v_min, int v_max, ...,
                                 char const* format = "%d",
                                 bool fill_up_to_cursor = false, ImGuiSliderFlags flags = 0 );
bool SliderSplineGradientScalar( char const* label, ImGuiDataType data_type,
                                 void* p_value, void* p_min, void* p_max,
                                 ImGradientData const* gradient,
                                 ImVec2 const* control_points = NULL, int num_points = 4,
                                 float v_height = 0.0f, float v_thickness = 0.0f,
                                 char const* format = NULL,
                                 bool fill_up_to_cursor = false, ImGuiSliderFlags flags = 0 );

bool SliderSplineGradientRangeFloat ( char const* label, float* v_lower, float* v_upper,
                                      float v_min, float v_max, ImGradientData const* gradient,
                                      ImVec2 const* control_points = NULL, int num_points = 4,
                                      float v_height = 0.0f, float v_thickness = 0.0f,
                                      char const* format = "%.3f", ImGuiSliderFlags flags = 0 );
bool SliderSplineGradientRangeInt   ( char const* label, int* v_lower, int* v_upper, ... );
bool SliderSplineGradientRangeScalar( char const* label, ImGuiDataType data_type,
                                      void* p_lower, void* p_upper, void* p_min, void* p_max,
                                      ImGradientData const* gradient,
                                      ImVec2 const* control_points = NULL, int num_points = 4,
                                      float v_height = 0.0f, float v_thickness = 0.0f,
                                      char const* format = NULL, ImGuiSliderFlags flags = 0 );
```
Same spline parametrization as `SliderSplineFloat`; the track is painted with
the gradient along the arc. The single-handle forms take `fill_up_to_cursor`
(this parameter was missing from earlier documentation); the range forms do not.

---

### `RangeSlider*` / `RangeSliderVertical*`
```cpp
bool RangeSliderFloat ( char const* label, float* v_lower, float* v_upper,
                        float v_min, float v_max,
                        char const* format = "%.3f", ImVec2 size = ImVec2( 0, 0 ) );
bool RangeSliderInt   ( char const* label, int* v_lower, int* v_upper, int v_min, int v_max,
                        char const* format = "%d", ImVec2 size = ImVec2( 0, 0 ) );
bool RangeSliderScalar( char const* label, ImGuiDataType data_type,
                        void* p_lower, void* p_upper, void const* p_min, void const* p_max,
                        char const* format = NULL, ImVec2 size = ImVec2( 0, 0 ) );

bool RangeSliderVerticalFloat ( char const* label, float* v_lower, float* v_upper,
                                float v_min, float v_max,
                                char const* format = "%.3f", ImVec2 size = ImVec2( 0, 0 ) );
bool RangeSliderVerticalInt   ( char const* label, int* v_lower, int* v_upper, int v_min, int v_max,
                                char const* format = "%d", ImVec2 size = ImVec2( 0, 0 ) );
bool RangeSliderVerticalScalar( char const* label, ImGuiDataType data_type,
                                void* p_lower, void* p_upper, void const* p_min, void const* p_max,
                                char const* format = NULL, ImVec2 size = ImVec2( 0, 0 ) );
```
Plain two-handle range slider (no gradient). Horizontal and vertical.

---

### `SegmentedControlInt` / `SegmentedControlScalar`
```cpp
bool SegmentedControlInt   ( char const* label, int* p_value,
                             char const* const* labels, int label_count,
                             ImVec2 size = ImVec2( 0, 0 ) );
bool SegmentedControlScalar( char const* label, ImGuiDataType data_type, void* p_value,
                             char const* const* labels, int label_count,
                             ImVec2 size = ImVec2( 0, 0 ) );
```
A row of mutually exclusive segments; the value is the selected index.

---

### `DragFloatPrecise`
```cpp
bool DragFloatPrecise( char const* label, float* value,
                       float v_min = 0.0f, float v_max = 0.0f,
                       char const* format = NULL, ImGuiSliderFlags flags = 0 );
```
`DragFloat` with a precision-drag mode: while held, a block grid appears and
vertical rows nudge by successively smaller increments. Block size:
`ImWidgetsStyleVar_PrecisionDrag_BlockSize`.

### `DragFloatColorUnderline` / `DragFloatGradientUnderline`
```cpp
bool DragFloatColorUnderline   ( char const* label, float* v, float v_speed,
                                 float v_min, float v_max, float v_default,
                                 ImU32 underline_col, char const* format = "%.2f",
                                 float width = 0.0f, float underline_thickness = 0.0f );
bool DragFloatGradientUnderline( char const* label, float* v, float v_speed,
                                 float v_min, float v_max, float v_default,
                                 ImGradientData const* gradient, char const* format = "%.2f",
                                 float width = 0.0f, float underline_thickness = 0.0f );
```
Grading-style drag fields: a thin flat-colour or gradient underline shows the
parameter's meaning. Double-click resets to `v_default`. Pair
`DragFloatGradientUnderline` with `GradingGradient(...)` -- see
[color.md](color.md#grading-gradients).

---

## Color Editors

### `HueSelector`
```cpp
bool HueSelector( char const* label, float hueHeight, float cursorHeight,
                  float* hueCenter, float* hueWidth,
                  float* featherLeft, float* featherRight,
                  int division = 32, float alpha = 1.0f,
                  float hideHueAlpha = 0.75f, float offset = 0.0f );
```
Horizontal hue-range selector: centre hue, width, and left/right feather.

---

### `GradientEditor`
```cpp
bool GradientEditor( char const* label, ImGradientData* gradient,
                     bool alpha = true, ImVec2 size = ImVec2( 0, 0 ) );
```
Interactive multi-stop gradient editor. Left-click the bar to add a stop,
Delete to remove the selected one, drag to reorder. With `gradient->SplitAlpha`
the alpha track is edited independently. See
[data-types.md -- ImGradientData](data-types.md#imgradientdata).

---

### `CurveEditor`
```cpp
bool CurveEditor( char const* label, ImCurveEditorData* curve, ImVec2 size = ImVec2( 0, 0 ) );
```
2D animation curve editor with full Bezier handles and per-segment easing.
See [data-types.md -- ImCurveEditorData](data-types.md#imcurveeditordata).

---

### `ColorWheel`
```cpp
bool ColorWheel( char const* label, ImVec4* color,
                 ImColorWheelMode mode = ImColorWheelMode_HSV,
                 float hdr_max = 1.0f, bool fixedIntensity = false,
                 ImVec2 size = ImVec2( 0, 0 ) );
```
Hue/chroma disc plus a value/lightness slider. `mode` is `ImColorWheelMode_HSV`
or `ImColorWheelMode_OkLCH`. `hdr_max > 1` enables an HDR range. Style vars:
`ImWidgetsStyleVar_ColorWheel_*`; disc resolution is
`GetStyle().ColorWheel_DiscSectors / DiscRings`.

---

### `HDRWheel`
```cpp
bool HDRWheel( char const* label, ImVec4* color,
               float* yValue, float yMin, float yMax,
               float* rightValue = NULL, float rightMin = 0.0f, float rightMax = 0.0f,
               float* leftValue  = NULL, float leftMin  = 0.0f, float leftMax  = 0.0f,
               ImColorWheelMode mode = ImColorWheelMode_HSV,
               ImVec2 size = ImVec2( 0, 0 ) );
```
Colour wheel with a linear Y output and up to two optional arc sliders around
the disc. Pass `rightValue = leftValue = NULL` (the defaults) for the compact
layout that used to be exposed as `PrimariesWheel`. Style colours:
`ImWidgetsCol_HDRWheel_*`; ring geometry:
`ImWidgetsStyleVar_HDRWheel_RingThickness` / `_RingGap`.

> **13 parameters, not 14.** There is no `ringThickness` argument -- earlier
> documentation showed one. Use the style var.
>
> `PrimariesWheel` was removed in 2.0.0.

---

### `ColorPicker` family
```cpp
bool ColorPicker      ( char const* label, ImVec4* color,
                        ImColorPickerSpace space = ImColorPickerSpace_sRGB,
                        int fixedAxis = 2, ImVec2 size = ImVec2( 0, 0 ) );

bool ColorPickerSRGB  ( char const* label, ImVec4* color, int fixedAxis = 2, ImVec2 size = ImVec2( 0, 0 ) );
bool ColorPickerHSV   ( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
bool ColorPickerOkLab ( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
bool ColorPickerOkLCH ( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
bool ColorPickerCIELab( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
bool ColorPickerXYZ   ( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
```
2D plane plus a 1D slider. `fixedAxis`: `0` = X fixed, `1` = Y fixed, `2` = Z
fixed. `size.x` overrides the item width, `size.y` the plane height
*(honoured since 2.0.0)*. Style: `ImWidgetsCol_ColorPicker_*`,
`ImWidgetsStyleVar_ColorPicker_*`, `GetStyle().ColorPicker_PlaneResolution /
_SliderResolution`.

---

### Physically-based and artistic colour pickers

Each of these drives a forward physical or artistic model: two model parameters
on a 2D plane, a third on a slider, and the resulting colour is written out.

> **`*color` is OUTPUT ONLY.** The forward models are not invertible, so these
> pickers cannot seed themselves from an arbitrary colour. They keep their own
> parameter state and overwrite `*color` every frame the user edits.
>
> `size.x` overrides the item width and `size.y` the plane height
> *(honoured since 2.0.0; previously accepted and ignored)*.

The following 22 share the signature
`bool F( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) )`:

| Function | Model |
|---|---|
| `ColorPickerSkin` | Two-layer epidermis/dermis skin model: melanin, haemoglobin, dermal thickness |
| `ColorPickerHair` | Chiang 2016 melanin hair model: eumelanin / pheomelanin / roughness |
| `ColorPickerLeaf` | PROSPECT-D leaf plate model: chlorophyll, carotenoid, anthocyanin, brown |
| `ColorPickerHemoglobin` | Oxy / deoxy haemoglobin mix and blood volume fraction |
| `ColorPickerSubsurface` | Scattering / absorption albedo of a translucent slab |
| `ColorPickerBruise` | Haemoglobin -> bilirubin -> biliverdin healing sequence |
| `ColorPickerIris` | Iris stroma melanin + Tyndall scattering |
| `ColorPickerBlackbody` | Planckian locus: temperature and exposure |
| `ColorPickerFlame` | Soot-continuum flame emission, temperature x soot density |
| `ColorPickerStar` | Stellar spectral class / temperature / luminosity |
| `ColorPickerNebula` | Emission-line nebula mix (H-alpha / OIII / SII) |
| `ColorPickerSky` | Rayleigh + Mie atmospheric scattering, sun elevation and turbidity |
| `ColorPickerCloud` | Cloud optical depth and sun angle |
| `ColorPickerIce` | Ice absorption depth and bubble scattering |
| `ColorPickerWater` | Water depth, turbidity and chlorophyll |
| `ColorPickerGem` | Gemstone absorption band + dispersion |
| `ColorPickerPigment` | Kubelka-Munk two-constant pigment mixing |
| `ColorPickerOchre` | Iron-oxide earth pigments: hematite / goethite / calcination |
| `ColorPickerPatina` | Copper oxidation / carbonate patina progression |
| `ColorPickerMaillard` | Maillard browning: temperature x time |
| `ColorPickerDischarge` | Gas-discharge emission spectra (Na, Hg, Ne, ...) |
| `ColorPickerStreetlight` | Street lighting classes (LPS, HPS, MH, LED CCT) |

The remaining 7 take extra outputs or no `size`:

```cpp
bool ColorPickerPaletteHarmony     ( char const* label, ImVec4* color,
                                     ImVec4* out_palette = nullptr, int* out_count = nullptr );
bool ColorPickerHarmonyWheel       ( char const* label, ImVec4* color,
                                     ImVec4* out_palette = nullptr, int* out_count = nullptr );
bool ColorPickerMoodPalette        ( char const* label, ImVec4* color, ImVec4 out_palette[ 5 ] = nullptr );
bool ColorPickerToonRamp           ( char const* label, ImVec4* color, ImVec4 out_ramp[ 3 ] = nullptr );
bool ColorPickerTrichromaticMixer  ( char const* label, ImVec4* color, bool enable_k = false );
bool ColorPickerWeatheredMetal     ( char const* label, ImVec4* color );
bool ColorPickerFabricDye          ( char const* label, ImVec4* color );
```
`ColorPickerTrichromaticMixer` mixes three primaries subtractively;
`enable_k = true` adds a black channel (CMYK mode). The palette pickers fill an
optional caller-owned array with the derived swatches.

---

### `ColorWarper`
```cpp
bool ColorWarper( char const* label, ImColorWarperData* data,
                  ImColorWarperMode mode = ImColorWarperMode_Circular,
                  ImColorWarperSpace space = ImColorWarperSpace_HSV,
                  float thirdAxis = 1.0f,
                  ImColorWarperOverlay const* signalOverlay = NULL,
                  ImColorWarperSignalColor signalColor = ImColorWarperSignalColor_PixelColor,
                  float axisAngle = 0.0f, ImVec2 size = ImVec2( 0, 0 ) );
```
Mesh-warp colour grader. Drag control points on a hue/saturation grid to shift
colours. Modes: `Circular` (polar disc), `Square` (cartesian), `ChromaLuma`
(two-square layout). Spaces: `HSV`, `HSL`, `HSY`, `HSP`, `HSPLog`, `OkLab`,
`OkLCH`. See
[data-types.md -- ImColorWarperData](data-types.md#imcolorwarperdata).

---

### `ColorCurve`
```cpp
bool ColorCurve( char const* label, ImColorCurveData* curve, ImColorCurveMode mode,
                 ImHistogramData const* histogramOverlay = NULL,
                 bool advancedSegments = false, ImVec2 size = ImVec2( 0, 0 ) );
```
Single-axis colour curve. Modes: `HueVsHue`, `HueVsSat`, `HueVsLum`, `LumVsSat`,
`SatVsSat`. Optional histogram underlay.

### `ToneCurve`
```cpp
bool ToneCurve( char const* label, ImToneCurveData* curve, ImHistogramMode mode,
                ImHistogramData const* histogramOverlay = NULL,
                bool advancedSegments = false, ImVec2 size = ImVec2( 0, 0 ) );
```
Multi-channel tone curve (up to 4 channels, count derived from `mode`), with a
histogram underlay and gradient bands along both axes.

---

## Grading Widgets

### `ColorBars`
```cpp
bool ColorBars( char const* label, float* yrgb,
                float v_min = -1.0f, float v_max = 1.0f, float v_default = 0.0f,
                bool link_master = true, bool show_values = true,
                ImVec2 size = ImVec2( 0, 0 ),
                ImWidgetsSliderFill fill = ImWidgetsSliderFill_FromCenter );
```
Resolve-style Y/R/G/B bar group. `yrgb` is 4 floats (master, R, G, B). With
`link_master`, dragging Y offsets the other three.

### `ColorSlice`
```cpp
bool ColorSlice( char const* label, ImColorSliceData& data, ImVec2 size = ImVec2( 0, 0 ) );
```
Multi-vector hue/saturation slice grader: named colour vectors, each with a hue
centre, width and a per-vector gain. See `ImColorSliceData` /
`ImColorSliceVector`.

### `ChromaWarp`
```cpp
bool ChromaWarp( char const* label, ImChromaWarpData& data, ImVec2 size = ImVec2( 0, 0 ) );
```
Warp grid over the chromaticity plane itself, rather than over a hue wheel.

### `SpectrumEditor`
```cpp
bool SpectrumEditor( char const* label, ImSpectrumData& spectrum,
                     ImVec2 size = ImVec2( 0, 0 ), float y_max = 1.0f,
                     ImWidgetsObserver observer = ImWidgetsObserver_CIE1931_2deg );
```
Edit a spectral power distribution; the resulting colour is previewed live.
Pair with `SpectrumToXYZ` / `WavelengthToRGB` -- see [color.md](color.md).

### `GridWarp`
```cpp
bool GridWarp( char const* label, ImGridWarpData* grid,
               ImTextureID background = 0, ImVec2 size = ImVec2( 0, 0 ) );
```
Free-form deformation lattice over an optional background image.

---

## Scopes & Analysis

### `ParadeScope`
```cpp
void        ParadeScope( char const* label, ImParadeScopeData const& data,
                         bool overlay = false, ImParadeScale scale = ImParadeScale_Linear,
                         ImVec2 size = ImVec2( 0, 0 ) );
char const* ParadeModeName( ImParadeMode mode );
```
Waveform parade (`Luma`, `RGB`, `YRGB`, `YCbCr`). Fill via `data.Accumulate(...)`.

### `VectorScope`
```cpp
void VectorScope( char const* label, ImVectorScopeData const& data,
                  bool showSkinToneLine = true, ImVec2 size = ImVec2( 0, 0 ) );
```
Cb/Cr chrominance scatter with target boxes and graticule.

### `Histogram`
```cpp
void        Histogram( char const* label, ImHistogramData const& data,
                       ImHistogramLayout layout = ImHistogramLayout_Overlapped,
                       ImParadeScale xScale = ImParadeScale_Linear,
                       ImParadeScale yScale = ImParadeScale_Linear,
                       ImVec2 size = ImVec2( 0, 0 ) );
char const* HistogramModeName( ImHistogramMode mode );
```
Per-channel histogram. Modes: `Luma`, `RGB`, `YRGB`, `YCbCr`, `HSV`, `OkLCH`.

### `CIEChromaticity`
```cpp
void        CIEChromaticity( char const* label, ImCIEChromaticityData const& data,
                             ImCIEChromaticityGamut gamut = ImCIEChromaticityGamut_sRGB_Rec709,
                             bool showBackground = false,
                             ImCIEChromaticitySignalColor signalColor = ImCIEChromaticitySignalColor_Flat,
                             ImVec2 size = ImVec2( 0, 0 ) );
char const* CIEChromaticityGamutName( ImCIEChromaticityGamut gamut );
```
CIE 1931 xy plot with gamut overlay. Gamuts: `sRGB_Rec709`, `Rec2020`, `DCI_P3`,
`ACEScg`, `AdobeRGB`, `ProPhoto`.

### Curve sampling helpers
```cpp
float       ColorCurveDefaultValue( ImColorCurveMode mode );
void        ColorCurveRange       ( ImColorCurveMode mode, float* out_min, float* out_max );
char const* ColorCurveModeName    ( ImColorCurveMode mode );
float       ColorCurveSample      ( ImColorCurveData const& curve, ImColorCurveMode mode,
                                    float x, bool advancedSegments = false );
int         ToneCurveChannelCount ( ImHistogramMode mode );
char const* ToneCurveChannelName  ( ImHistogramMode mode, int channel );
float       ToneCurveSample       ( ImColorCurveData const& curve, float x,
                                    bool advancedSegments = false );
float       CurveEditorSample     ( ImCurveEditorData const& curve, float x );
float       CurveEditorEvalEasing ( ImCurveEditorSeg seg, float t );
char const* CurveEditorSegName    ( ImCurveEditorSeg seg );
char const* CurveEditorTangentModeName( ImCurveEditorTangentMode mode );
```

---

## Image Widgets

### `ImageViewer`
```cpp
bool ImageViewer( char const* label, ImTextureID image, ImVec2 imageSize,
                  ImImageViewerState& state, ImVec2 widgetSize = ImVec2( 0, 0 ),
                  ImPlatform_ShaderProgram shaderProgram = nullptr,
                  ImImageViewerOverlayCallback overlay_callback = NULL,
                  void* overlay_user_data = NULL );
```
Pan (left-drag) and zoom (wheel) image viewer. Double-click resets. Right-click
opens a pixel-inspector loupe showing RGBA values when `state.Pixels` is set.

- `shaderProgram` -- optional custom pixel shader wrapping the image draw. The
  shader binds *inside* the widget, so it applies to both the inline draw and the
  expand-to-window modal. The loupe stays unshaded so the inspector shows source
  texels. Author the vertex shader as a standard ImGui VS (ImGui's `b0 ProjMtx`
  and vertex interface still apply). Passing non-null moves the widget to the
  **ImPlatform + Shader** tier.
- `overlay_callback` / `overlay_user_data` -- invoked once per presentation
  (compact widget, and again inside the modal) right after the image is drawn.
  Drive the overlays below from here rather than calling them after
  `ImageViewer` returns, so they appear in the modal too.

> **Three parameters added in 2.0.0.** All default, so existing calls still
> compile.

### Image overlays

Coordinates are UV `[0,1]^2` relative to the image, so overlays compose with
the viewer's pan/zoom. Call `ImageViewer` first, then the overlays with the same
state.

```cpp
int  DetectionOverlay( char const* str_id, ImImageViewerState const& viewer_state,
                       ImDetectionBox const* boxes, int box_count,
                       char const* const* class_names, int class_name_count,
                       ImOverlayStyle const* style = NULL );

void KeypointOverlay ( char const* str_id, ImImageViewerState const& viewer_state,
                       ImKeypoint const* keypoints, int keypoint_count,
                       ImSkeletonEdge const* edges, int edge_count,
                       char const* const* part_names, int part_name_count,
                       ImOverlayStyle const* style = NULL );

void TextLabelOverlay( char const* str_id, ImImageViewerState const& viewer_state,
                       ImTextLabel const* labels, int label_count );

ImAnnotationResult AnnotationEditor( char const* str_id, ImImageViewerState const& viewer_state,
                                     ImAnnotationEditorState& editor_state,
                                     ImDetectionBox const* boxes, int box_count,
                                     char const* const* class_names, int class_name_count,
                                     ImOverlayStyle const* style = NULL );
```
`DetectionOverlay` returns the `user_id` of the hovered box (`-1` = none). The
three read-only overlays never mutate their inputs. `AnnotationEditor` is the
only interactive one: it returns edit *intents* in `ImAnnotationResult`
(`ImAnnotationAction_*`) for the host to apply; it never edits `boxes` itself.

### `ImageInspector`
```cpp
bool ImageInspector( char const* label, ImImageBuffer const& buffer,
                     ImImageInspectorState& state, ImVec2 widgetSize = ImVec2( 0, 0 ) );
bool ImageInspectorSupported();
void ImageInspectorReleaseState( ImImageInspectorState& state );
```
GPU pixel inspector: transfer function, gamut, exposure, false colour, channel
isolation, zoom-to-pixel readout. Requires a custom-shader backend --
`ImageInspectorSupported()` reports whether the build and backend can run it.
`ImageInspectorReleaseState` frees the GPU resources cached in the state; call
it before the state goes out of scope.

### `ImageCarousel` / `ImageBento`
```cpp
bool ImageCarousel( char const* label, ImTextureID* images, ImVec2* imageSizes,
                    int imageCount, int* pSelectedIndex, ImVec2 size = ImVec2( 0, 0 ) );

bool ImageBento( char const* label, ImTextureID* images, ImVec2* imageSizes,
                 int imageCount, int* pSelectedIndex,
                 int columnsPerRow = 4, float cellAspect = 1.0f, float spacing = 4.0f,
                 char const* const* pItemIds = nullptr,
                 int* pReorderFrom = nullptr, int* pReorderTo = nullptr );
```
Horizontal scrolling strip and a grid gallery with centre-crop to `cellAspect`.

**Drag-to-reorder (optional):** pass a `pItemIds` array of stable per-cell
C-strings so each cell's ImGui ID follows the item rather than the position.
When the user drags a cell over an adjacent one, `*pReorderFrom` / `*pReorderTo`
are set and the call returns `true`. `ImageBento` never mutates the arrays --
the caller performs the swap. Swaps are reported one adjacent step at a time
(`|from-to| == 1` horizontally, `== columnsPerRow` vertically).

### `TransformGizmo`
```cpp
bool TransformGizmo( char const* label,
                     ImTransformData* transforms, ImVec2* sizes, int count,
                     int* selectedIndex,
                     ImTransformGizmoCallbacks const* callbacks = nullptr,
                     ImTransformGizmoFlags flags = ImTransformGizmoFlags_None,
                     ImVec2 canvasSize = ImVec2( 0, 0 ) );
```
2D transform gizmo with translate / rotate / scale handles over `count` items.
`transforms` and `sizes` are parallel caller-owned arrays.
`ImTransformGizmoFlags_NonUniformScale` enables edge-midpoint handles. Drawing is
delegated to `callbacks` (`ImTransformGizmoCallbacks` /
`ImTransformGizmoDrawParams`), so the gizmo can drive any content; passing
`nullptr` draws outlines only. Style: `ImWidgetsCol_Gizmo_*`,
`ImWidgetsStyleVar_Gizmo_*`.

> **Replaces `ImageTransformGizmo`** (2.0.0). The old `ImTransformImage` struct
> is gone; the gizmo no longer owns textures.

### `PaintCanvas`
```cpp
bool PaintCanvas( char const* label, ImPaintCanvasData* canvas, ImVec2 size = ImVec2( 0, 0 ) );
```
CPU paint surface backed by a GPU texture: brush / eraser, hard / soft, `Mask`,
`Grayscale` and `Color` modes. Pixel data is caller-owned (`canvas->Pixels`).
Requires ImPlatform for texture upload.

---

## Look Development, LUTs and Volumes

### 3D LUT
```cpp
bool   LoadCubeLUT     ( ImColorLUT3D& out, char const* filename );
bool   SaveCubeLUT     ( ImColorLUT3D const& in, char const* filename );
ImVec4 SampleLUT3D     ( ImColorLUT3D const& lut, ImVec4 rgb_in );
void   InitIdentityLUT3D( ImColorLUT3D& out, int size );
bool   ColorLUT3DViewer( char const* label, ImColorLUT3D* lut, ImVec2 size = ImVec2( 0, 0 ) );
```
Read / write Adobe `.cube` files, trilinearly sample a LUT, build an identity
LUT, and inspect one as a 3D cube of transformed sample points.

### LookDev
```cpp
bool LookDevCompare  ( char const* label, ImTextureID tex_a, ImTextureID tex_b,
                       ImVec2 a_uv_min, ImVec2 a_uv_max, ImVec2 b_uv_min, ImVec2 b_uv_max,
                       ImLookDevState* state, ImVec2 size = ImVec2( 0, 0 ) );
bool LookDevInspector( char const* label, ImTextureID tex_a, ImTextureID tex_b,
                       ImLookDevInspectorState* state, ImVec2 size = ImVec2( 0, 0 ) );
```
`LookDevCompare` is a wipe / side-by-side / difference comparator built from
ImGui primitives. `LookDevInspector` adds the shader-backed inspection modes
(exposure, false colour, per-channel difference) and an orbit-style navigation.
Style vars: `ImWidgetsStyleVar_LookDev_*`.

### Delta-E comparison
```cpp
void ColorDifferenceVisualizer  ( char const* label, ImVec4* a, ImVec4* b,
                                  ImDeltaEFormula formula = ImDeltaEFormula_E2000 );
bool ColorDifferenceImageViewer ( char const* label, ImTextureID tex_a, ImTextureID tex_b,
                                  ImDeltaECompareState* state, ImVec2 size = ImVec2( 0, 0 ) );
```
Two-swatch Delta-E readout, and a GPU per-pixel Delta-E heat map between two
images. The formula helpers are in [color.md](color.md#color-difference-delta-e).

### Volumes
```cpp
bool VolumeViewer       ( char const* label, ImVolumeViewerState* state, ImVec2 size = ImVec2( 0, 0 ) );
bool VolumeSliceViewer  ( char const* label, ImVolumeSliceState*  state, ImVec2 size = ImVec2( 0, 0 ) );
void VolumeGenerateField( float* out_voxels, int w, int h, int d,
                          ImVolumeField mode, float seed = 0.0f );
```
`VolumeViewer` ray-marches a 3D texture (shader tier); `VolumeSliceViewer` shows
axis-aligned slices (ImPlatform tier). `VolumeGenerateField` fills a voxel buffer
with a procedural field:

```cpp
enum ImVolumeField_
{
    ImVolumeField_Sines, ImVolumeField_Sphere, ImVolumeField_NestedSpheres,
    ImVolumeField_Torus, ImVolumeField_Gyroid, ImVolumeField_SphericalNoise,
    ImVolumeField_COUNT
};
```

> **Removed in 2.0.0:** `VolumeGenerateTestField` -- use
> `VolumeGenerateField(..., ImVolumeField_Sines)`.

---

## Canvas & Authoring Toolkit

### Canvas
```cpp
bool   BeginCanvas( char const* id_str, ImVec2 size, ImWidgetsCanvasState& s,
                    ImU32 bg_col   = IM_COL32( 30, 30, 30, 255 ),
                    ImU32 grid_col = IM_COL32( 80, 80, 80, 110 ),
                    float grid_step_world = 32.0f );
void   EndCanvas();
ImVec2 CanvasWorldToScreen ( ImWidgetsCanvasState const& s, ImVec2 w );
ImVec2 CanvasScreenToWorld ( ImWidgetsCanvasState const& s, ImVec2 p );
ImRect CanvasViewportWorld ( ImWidgetsCanvasState const& s );
void   DrawCanvasMinimap( ImWidgetsCanvasState& s, ImRect world_content,
                          ImVec2 minimap_size = ImVec2( 160, 110 ),
                          ImWidgetsCornerAnchor anchor = ImWidgetsCornerAnchor_TopRight,
                          float margin_px = 8.0f,
                          ImU32 bg_col = IM_COL32( 20, 20, 20, 200 ),
                          ImU32 content_col = IM_COL32( 80, 80, 80, 200 ),
                          ImU32 viewport_col = IM_COL32( 255, 200, 60, 230 ) );
```
Pannable / zoomable world-space canvas. `BeginCanvas` returns `false` when the
canvas is fully clipped -- still call `EndCanvas` only if it returned `true`.
`DrawCanvasMinimap` is interactive: dragging inside it moves the viewport.

### Marquee selection
```cpp
bool BeginSelection      ( char const* id_str, ImRect bounds, ImWidgetsSelectionState& s,
                           ImU32 fill_col = IM_COL32( 80, 160, 255, 60 ),
                           ImU32 line_col = IM_COL32( 80, 160, 255, 220 ),
                           float thickness = 1.0f );
void TestSelectionPoints ( ImWidgetsSelectionState const& s, ImVec2 const* pts, int count,
                           bool* inside_out );
void TestSelectionRects  ( ImWidgetsSelectionState const& s, ImRect const* rects, int count,
                           bool* inside_out, bool fully_contained_only = false );
```
`BeginSelection` returns `true` while dragging and on the frame the marquee is
released (`s.Closed`); test your items against `s` and write the per-item result
into `inside_out`. Holding Shift or Alt when the drag starts sets `s.Op` to
`ImWidgetsSelectionOp_Add` / `_Subtract`, which the caller applies to its own
selection set. Escape aborts the drag and sets `s.Cancelled`.

> **Replaces** the removed `ImBoxSelectState` / `BoxSelectUpdate` /
> `BoxSelectDraw` / `BoxSelectRect` family (2.0.0).

### Snapping
```cpp
ImVec2 ComputeSnapAndDraw( ImDrawList* draw, ImRect moving,
                           ImRect const* targets, int target_count,
                           float snap_radius_px = 6.0f,
                           ImWidgetsSnapFlags flags = ImWidgetsSnapFlags_All,
                           ImU32 guide_col = IM_COL32( 255, 80, 220, 220 ),
                           float guide_extend_px = 16.0f );
```
Returns the offset to apply to `moving` and draws the alignment guides it used.
Flags select which edges / centres participate (`ImWidgetsSnapFlags_Edges`,
`_Centers`, `_All`, or individual edges).

### Crop
```cpp
bool CropRect( char const* id_str, ImWidgetsCropState& s, ImRect bounds,
               float aspect_ratio = 0.0f,                       // 0 = free; >0 = width/height locked
               ImWidgetsCropGuides guides = ImWidgetsCropGuides_Default,
               float grip_size = 6.0f,
               ImU32 mask_col  = IM_COL32(   0,   0,   0, 140 ),
               ImU32 line_col  = IM_COL32( 255, 255, 255, 220 ),
               ImU32 grip_col  = IM_COL32( 255, 255, 255, 255 ),
               ImU32 guide_col = IM_COL32( 255, 255, 255,  90 ) );
```
Eight-grip crop rectangle with a dimmed mask and optional composition guides
(`RuleOfThirds`, `GoldenRatio`, `Diagonals`, `Center`).

### Colour picking from the screen
```cpp
ImU32 EyedropperSampleBitmap( ImVec2 uv, void* user_data );   // user_data: ImWidgetsEyedropperBitmap*

ImWidgetsEyedropperResult Eyedropper(
        char const* id_str, ImRect screen_rect,
        ImWidgetsEyedropperSampleFn sample_fn, void* user_data,
        int   neighbor_cells = 7,                             // odd; 0 disables the loupe
        float neighbor_pixel = 14.0f,                         // loupe cell size, screen px
        ImU32 outline_col    = IM_COL32(   0,   0,   0, 220 ),
        ImU32 highlight_col  = IM_COL32( 255, 255, 255, 220 ) );

bool GradientDrop( char const* id_str, ImRect screen_rect,
                   ImWidgetsEyedropperSampleFn sample_fn, void* user_data,
                   ImWidgetsGradientDropState& state, ImGradientData* out_gradient,
                   int   max_stops            = 8,
                   float perceptual_threshold = 0.04f,        // OkLab dE
                   float min_step_px          = 3.0f,
                   float path_thickness       = 3.0f,
                   bool  show_live_preview    = true );
```
`Eyedropper` samples through a caller-supplied callback, so it works over an
image, a canvas or anything else you can sample.
`EyedropperSampleBitmap` is a ready-made callback for a CPU bitmap.
`GradientDrop` drags a path across the sampled surface and builds an
`ImGradientData` from it, merging stops closer than `perceptual_threshold` in
OkLab.

### Size control
```cpp
bool SizeControl2D( char const* label, float* width, float* height, bool* lock,
                    float v_min = 0.0f, float v_max = 0.0f, char const* format = "%.1f",
                    ImVec2 size = ImVec2( 0, 0 ),
                    ImWidgetsSizePreview preview = ImWidgetsSizePreview_Top );
bool SizeControl3D( char const* label, float* width, float* height, float* depth, bool* lock,
                    float v_min = 0.0f, float v_max = 0.0f, char const* format = "%.1f",
                    ImVec2 size = ImVec2( 0, 0 ),
                    ImWidgetsSizePreview preview = ImWidgetsSizePreview_Top );
```
Compact dimension editor: an interactive proxy (a rectangle in 2D, an isometric
box in 3D) with drag handles, one numeric drag field per axis, and a padlock.
While `*lock` is engaged, editing any one dimension scales the others by the same
factor so every ratio is preserved. `v_max <= 0` means unbounded; values are
always kept `> 0` to protect the ratio math. `preview` places the proxy above or
below the fields, or hides it.

### Onion skin
```cpp
void DrawOnionSkin    ( ImDrawList* draw, ImRect bounds,
                        ImWidgetsOnionFrame const* prev, int prev_count,
                        ImWidgetsOnionFrame const* next, int next_count,
                        ImTextureID current_tex,
                        ImVec2 cur_uv0 = ImVec2( 0, 0 ), ImVec2 cur_uv1 = ImVec2( 1, 1 ) );
void DrawOnionSkinAuto( ImDrawList* draw, ImRect bounds,
                        ImTextureID const* frames, int frame_count, int current_idx,
                        int back = 1, int forward = 1,
                        ImU32 back_tint    = IM_COL32(  80, 160, 255, 255 ),
                        ImU32 forward_tint = IM_COL32( 255, 100,  80, 255 ),
                        float base_alpha   = 0.4f );
```
`DrawOnionSkinAuto` derives the tints and alphas from a flat frame array;
`DrawOnionSkin` takes explicit per-frame descriptors.

### Gradient mesh, toon ramp, matrices
```cpp
void  GradientMeshInit  ( ImWidgetsGradientMesh& m, int nx, int ny,
                          ImU32 c00, ImU32 c10, ImU32 c01, ImU32 c11 );
bool  GradientMeshEditor( char const* id_str, ImWidgetsGradientMesh& m, ImVec2 size,
                          int sampleX = 32, int sampleY = 32 );
bool  ToonRampEditor    ( char const* id_str, ImWidgetsToonRamp& r, ImVec2 size );
ImU32 ToonRampSample    ( ImWidgetsToonRamp const& r, float t01 );
bool  MatrixEditor      ( char const* label, float*  data, int rows, int cols,
                          float speed = 0.1f, char const* format = "%.3f" );
bool  MatrixEditorDouble( char const* label, double* data, int rows, int cols,
                          float speed = 0.1f, char const* format = "%.3f" );
```
`GradientMeshEditor` edits a bilinear colour mesh: drag a node to move it
(corner nodes are pinned, edge nodes slide along their edge), click a node
without dragging to edit its colour. `size` is in logical pixels; `sampleX` /
`sampleY` set the total fill subdivision across the mesh. `ToonRampEditor` edits a stepped cel-shading ramp and
`ToonRampSample` evaluates it. The matrix editors lay out a `rows x cols` grid of
drag fields over a caller-owned row-major buffer.

---

## Dials, Fields, Misc

### `NotchedDial`
```cpp
bool NotchedDial( char const* label, float* v, float const* stops, int stop_count,
                  char const* const* stop_labels = nullptr, ImVec2 size = ImVec2( 0, 0 ) );
```
Rotary control that snaps to a fixed list of stops, with optional per-stop
labels. Style vars: `ImWidgetsStyleVar_NotchedDial_*`.

### `AngleDial`
```cpp
bool AngleDial( char const* label, float* v_deg,
                float v_min_deg = -180.0f, float v_max_deg = 180.0f,
                ImVec2 size = ImVec2( 0, 0 ) );
```
Continuous angle dial in degrees.

### `UpVector`
```cpp
bool UpVector( char const* label, float* direction, int defaultUpAxis = 1,
               ImVec2 size = ImVec2( 0, 0 ) );
```
Hemisphere picker producing a unit vector (3 floats). `defaultUpAxis`:
`0` = X, `1` = Y, `2` = Z.

### `UnitField`
```cpp
bool UnitField( char const* label, float* pValue,
                ImUnitDef* units, int unitCount, int* pSelectedUnit,
                float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f,
                char const* format = NULL );
```
DragFloat plus a unit selector. The stored value is always in the base unit; the
display converts through `ImUnitDef`. Build units with `ImUnitDef_Simple()` or
`ImUnitDef_Custom()`.

### `EquationInput`
```cpp
bool EquationInput( char const* label, char* buf, size_t buf_size,
                    ImVec2 size = ImVec2( 0, 0 ), ImWidgetsEquationFlags flags = 0 );
```
LaTeX-ish equation field: type source, see it typeset live through the LaTeX
renderer. Flags select inline vs block presentation, inline baseline alignment,
read-only, and whether parse errors are surfaced. Requires
`ImWidgetsFeatures_LaTeX`.

### `VectorDrawingTool`
```cpp
bool VectorDrawingTool( char const* label, ImVectorDrawingData& data,
                        ImVec2 size = ImVec2( 0, 0 ) );
```
Bezier path authoring canvas with zoom/pan. Click to place anchors, drag tangent
handles to shape curves, click the first anchor to close a path. Multiple paths
live in `data.Paths`; `data.SelectedPath` / `data.SelectedNode` track the
selection. See
[data-types.md -- ImVectorDrawingData](data-types.md#imvectordrawingdata).

### `FontInspector`, `KerningPairEditor`, `VarFontAxisSliders`

Font-oriented widgets are documented with the rest of the text pipeline in
[typography.md](typography.md#font-tooling-widgets).

---

## Graphs and Timelines

### `NetworkGraph`
```cpp
void NetworkGraphLayout( ImNetworkGraphData& data, int group_id );
bool NetworkGraph      ( char const* str_id, ImNetworkGraphData& data,
                         ImVec2 size = ImVec2( 0, 0 ) );
```
Read-only, hierarchically grouped DAG illustration (neural networks, dataflow
graphs). Rounded node boxes, bezier edges with arrowheads, wheel zoom-to-cursor,
left/middle-drag pan, visible-rect culling, LOD (text skipped below ~0.4 zoom,
dot boxes below ~0.15), hover highlight and tooltip.

Double-clicking a group box sets `data.ActivatedGroup`; double-clicking an op
node sets `data.ActivatedNode` -- the host pushes and handles levels. The view
auto-fits when the displayed level changes or `data.FitRequest` is set. Returns
`true` when something was activated this frame.

`NetworkGraphLayout` computes and caches the layered layout of one level (the
direct children of `group_id`, `-1` = root): longest-path layering, barycentre
ordering sweeps, per-layer centred X. `NetworkGraph` calls it lazily; call it
directly only if you need the geometry before drawing.

> After **any** mutation of `Nodes` / `Groups` / `Edges` made outside
> `ImNetworkGraphData::AddNode` / `AddGroup` / `AddEdge`, call
> `data.InvalidateLayout()`. A cached layout stores raw indices into those
> vectors and the draw path dereferences them unguarded.

### `ImTimeline`
```cpp
bool ImTimeline( char const* label, ImTimelineData const& data,
                 ImVec2 const& size, ImTimelineAction* out_action );
```
Temporal timeline for a document with exact 64-bit ticks: playable range with a
playhead, authored keys on named track rows, ruler event markers, and
half-open `[BeginTick, EndTick)` bands. Supports scrubbing and key insertion at
arbitrary ticks, *including before computed frames*.

The widget never mutates `ImTimelineData`. It returns `true` when `out_action`
carries a user action (`Scrub`, `InsertKey`, `SelectKey`, `DeleteKey`); the host
applies the edit and re-feeds the data.

Band kinds (`ImTimelineBandKind`): `ImTimelineBand_CacheCoverage`,
`_Invalid`, `_StaleBranch`, `_Checkpoint` (a thin pin line at `BeginTick`;
`EndTick` ignored) and `_CommittedHistory`.

> `CommittedHistory` is *retained versions on the tracks*, not cache residency.
> Hosts must not present history as cache coverage.

---

## Custom-Shape Buttons

```cpp
bool ButtonExCircle  ( char const* label, float radius, ImGuiButtonFlags flags );
bool ButtonExCapsuleH( char const* label, float length, float thickness, ImGuiButtonFlags flags );
bool ButtonExCapsuleV( char const* label, float length, float thickness, ImGuiButtonFlags flags );
bool ButtonExConvex  ( char const* label, ImVec2 const& size_arg, ImVec2* pts, int pts_count,
                       ImGuiButtonFlags flags );
bool ButtonExConcave ( char const* label, ImVec2 const& size_arg, ImVec2* pts, int pts_count,
                       ImVec2 text_offset, ImGuiButtonFlags flags );
bool ButtonExWithHole( char const* label, ImVec2 const& size_arg, ImVec2* pts, int pts_count,
                       ImVec2 text_offset, ImGuiButtonFlags flags );
```
Hit-testing uses the actual shape, not the bounding box.

Image variants (all take `ImU32 col = IM_COL32_WHITE`, `ImVec2 uv_min = {0,0}`,
`ImVec2 uv_max = {1,1}`):
```cpp
bool ImageButtonExCircle  ( char const* label, ImTextureID tex, float radius, ImGuiButtonFlags flags, ... );
bool ImageButtonExCapsuleH( char const* label, ImTextureID tex, float length, float thickness, ImGuiButtonFlags flags, ... );
bool ImageButtonExCapsuleV( char const* label, ImTextureID tex, float length, float thickness, ImGuiButtonFlags flags, ... );
bool ImageButtonExConvex  ( char const* label, ImTextureID tex, ImVec2 const& size_arg,
                            ImVec2* pts, int pts_count, ImGuiButtonFlags flags, ... );
bool ImageButtonExConcave ( char const* label, ImTextureID tex, ImVec2 const& size_arg,
                            ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags, ... );
```

The fully general form, used to build the ones above:
```cpp
bool ButtonEx( char const* label, ImVec2 const& size_arg, ImRect bb, ImVec2 text_offset,
               ImGuiButtonFlags flags, IsContains isContains,
               ImDrawShape outline, ImDrawShapeFilled fill, ImDrawShapeFilledTex fill_tex,
               ImInlineOffset offset, FromRect from_rect, void* extra_data,
               ImTextureID* tex = NULL, ImVec2 uv_min = { 0, 0 }, ImVec2 uv_max = { 1, 1 } );
```

### Hit-test and behaviour primitives

```cpp
bool IsBoundingBoxWellFormed( ImVec2 const& r_min, ImVec2 const& r_max, ImVec2* pts, int pts_count );

bool Im_IsCircleContains     ( ImVec2 p, void* data );
bool Im_IsCapsuleHContains   ( ImVec2 p, void* data );
bool Im_IsCapsuleVContains   ( ImVec2 p, void* data );
bool Im_IsPolyConvexContains ( ImVec2 p, void* data );
bool Im_IsPolyConcaveContains( ImVec2 p, void* data );
bool Im_IsPolyWithHoleContains( ImVec2 p, void* data );

bool IsMouseHovering( ImVec2 const& r_min, ImVec2 const& r_max, IsContains contains,
                      void* data, bool clip = true );
bool ItemHoverable  ( ImRect const& bb, ImGuiID id, ImGuiItemFlags item_flags,
                      IsContains isContains, void* extra_data );

bool ButtonBehaviorEx       ( ImRect const& bb, ImGuiID id, bool* out_hovered, bool* out_held,
                              ImGuiButtonFlags flags, IsContains isContains, void* extra_data );
bool ButtonBehaviorCircle   ( ImVec2 center, float radius, ImGuiID id,
                              bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
bool ButtonBehaviorCapsuleH ( ImVec2 pos, float length, float radius, ImGuiID id,
                              bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
bool ButtonBehaviorCapsuleV ( ImVec2 pos, float length, float radius, ImGuiID id,
                              bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
bool ButtonBehaviorConvex   ( ImVec2* pts, int pts_count, ImGuiID id,
                              bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
bool ButtonBehaviorConcave  ( ImVec2* pts, int pts_count, ImGuiID id,
                              bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
bool ButtonBehaviorWithHole ( ImVec2* pts, int pts_count, ImGuiID id,
                              bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
```
`IsContains` is `bool ( * )( ImVec2 p, void* data )` -- the `Im_Is*Contains`
functions above are the stock implementations.

---

## Interaction Helpers

### Precision popups
```cpp
bool BeginPrecisionPopup( char const* str_id, ImVec2 anchor_screen_pos );
void EndPrecisionPopup();
bool PrecisionFloat( char const* label, float* v, float step = 0.0f,
                     float v_min = -FLT_MAX, float v_max = FLT_MAX,
                     char const* fmt = "%.4f" );
bool PrecisionInt  ( char const* label, int* v, int step = 1,
                     int v_min = -INT_MAX, int v_max = INT_MAX );
```
The typed-input popups that Slider2D, Slider2DRange and friends open on
double-click. `BeginPrecisionPopup` must be balanced with `EndPrecisionPopup`
only when it returns `true`. Width:
`ImWidgetsStyleVar_PrecisionPopup_InputWidth`.

> **Fixed in 2.0.0.** These popups computed a clamped value and never wrote it
> back.

### Keyboard nudging and axis constraint
```cpp
ImVec2 KeyboardNudgeXY();
float  KeyboardNudge1D( bool horizontal = true );
ImVec2 AxisConstrain( ImVec2 drag_delta );
```
`KeyboardNudge*` return the arrow-key delta for this frame (Shift = 10x).
`AxisConstrain` snaps a drag delta to the dominant axis while Shift is held.

### Delta overlay
```cpp
void PushDeltaOverlay  ( ImGuiID id, float start_value );
void RenderDeltaOverlay( ImGuiID id, float current, char const* fmt = nullptr );
```
Shows `+/-delta` next to a control being dragged. Push on activation, render
each frame while active.

### Proportional drag groups
```cpp
void  PushDragGroup     ( ImGuiID group_id, float falloff_radius = 2.0f,
                          ImWidgetsFalloff kernel = ImWidgetsFalloff_Gaussian );
void  PopDragGroup      ();
float GetDragGroupDelta ( int group_slot );
void  SetDragGroupActive( int group_slot, float drag_delta );
```
Lets a row of controls respond to one drag with a distance-based falloff
(`Linear`, `Gaussian`, `Smoothstep`) -- the "soft selection" behaviour used by
the grading bars.

---

## Window Background

```cpp
void SetCurrentWindowBackgroundImage( ImTextureID id, ImVec2 imgSize, bool fixedSize = false,
                                      ImU32 col = IM_COL32( 255, 255, 255, 255 ) );
void SetCurrentWindowBlurBackground  ( ImWidgetsBgEffect effect,
                                       float param0 = 0.0f, float param1 = 0.0f, float param2 = 0.0f,
                                       ImU32 tint = IM_COL32( 255, 255, 255, 255 ) );
void BlurBackgroundNewFrame();
```
`SetCurrentWindowBackgroundImage` replaces the current window's background with
a texture (this breaks corner rounding).

`SetCurrentWindowBlurBackground` runs a full-screen effect pass behind the
current window. Call `BlurBackgroundNewFrame()` once per frame, before any
window that uses it. 18 effects
(`ImWidgetsBgEffect_Blur`, `GlassRefraction`, `FrostedGlass`, `Pixelate`,
`ChromaticAberration`, `LiquidGlass`, `HeatHaze`, `Voronoi`, `EdgeGlow`,
`Halftone`, `MouseEdge`, `CRT`, `DotMatrix`, `Glitch`, `StainedGlass`, `Rain`,
`Kaleidoscope`), each compiled on demand from `blur.hlsl`. Requires a
custom-shader backend; see [compatibility.md](../compatibility.md).
