# Changelog

## refacto branch -- 2026-04-19

This release tracks all changes accumulated on the `refacto` branch.

---

### New Widgets

#### `VectorDrawingTool`
Interactive bezier-path authoring canvas with zoom/pan. Paths support multiple styles (polyline, anti-aliased polyline, Euler-spiral stroke, dashed variants), per-path color, thickness, dash parameters, cap, and join. The widget has been extracted into its own translation unit (`dear_widgets_vector_drawing.h` / `dear_widgets_vector_drawing.cpp`).

```cpp
bool VectorDrawingTool(const char* label, ImVectorDrawingData& data, ImVec2 size = {});
```

Key data types: `ImVectorDrawingData`, `ImVectorDrawingPath`, `ImVectorDrawingNode`, `ImVectorDrawingStyle_`.

#### Gradient Sliders -- single-handle

Three new slider shapes (horizontal bar, ring arc, Bezier spline) that paint a full `ImGradientData` gradient on the track instead of a flat colour. An optional `fill_up_to_cursor` flag renders only the gradient up to the current handle position.

```cpp
bool SliderGradientFloat(const char* label, float* v, float v_min, float v_max,
                         const ImGradientData* gradient, ImVec2 size = {},
                         bool fill_up_to_cursor = false, bool right_to_left = false);
bool SliderGradientInt(...);
bool SliderGradientScalar(...);

bool SliderGradientRingFloat(const char* label, float* v, float v_min, float v_max,
                             const ImGradientData* gradient,
                             float outerRadius, float thickness,
                             float startAngle, float sweepAngle,
                             bool fill_up_to_cursor = false);
bool SliderGradientRingInt(...);
bool SliderGradientRingScalar(...);

bool SliderSplineGradientFloat(const char* label, float* v, float v_min, float v_max,
                               const ImGradientData* gradient,
                               const ImVec2* control_points = NULL, int num_points = 4,
                               float v_height = 0.f, float v_thickness = 0.f,
                               const char* format = "%.3f",
                               ImGuiSliderFlags flags = 0);
bool SliderSplineGradientInt(...);
bool SliderSplineGradientScalar(...);
```

#### Gradient Sliders -- two-handle range

Two-handle variant of each gradient slider shape. The gradient is painted only between the two handles; the FrameBg track shows through outside. Clicking latches the nearest handle; dragging it is clamped so lower <= upper.

```cpp
bool SliderGradientRangeFloat(const char* label, float* v_lower, float* v_upper,
                              float v_min, float v_max,
                              const ImGradientData* gradient, ImVec2 size = {},
                              bool right_to_left = false);
bool SliderGradientRangeInt(...);
bool SliderGradientRangeScalar(...);

bool SliderGradientRingRangeFloat(const char* label, float* v_lower, float* v_upper,
                                  float v_min, float v_max,
                                  const ImGradientData* gradient,
                                  float outerRadius, float thickness,
                                  float startAngle, float sweepAngle);
bool SliderGradientRingRangeInt(...);
bool SliderGradientRingRangeScalar(...);

bool SliderSplineGradientRangeFloat(const char* label, float* v_lower, float* v_upper,
                                    float v_min, float v_max,
                                    const ImGradientData* gradient,
                                    const ImVec2* control_points = NULL, int num_points = 4,
                                    float v_height = 0.f, float v_thickness = 0.f,
                                    const char* format = "%.3f",
                                    ImGuiSliderFlags flags = 0);
bool SliderSplineGradientRangeInt(...);
bool SliderSplineGradientScalar(...);
```

> **Note on ring range.** Full-circle rings (where `sweepAngle` covers 2PI) have ambiguous wrap semantics for a two-handle range. Use two separate `SliderGradientRing` calls instead.

---

### New DrawList Primitives

#### `DrawSplineGradient` / `DrawSplineGradientCut`

Paint a multi-stop gradient along a Bezier spline path. `DrawSplineGradientCut` restricts the gradient to the `[min, max]` normalized arc range; the rest is fully transparent.

```cpp
void DrawSplineGradient(ImDrawList* dl, const ImGradientData& gradient,
                        const ImVec2* points, int count,
                        float thickness, int resolution, bool closed = false);
void DrawSplineGradientCut(ImDrawList* dl, const ImGradientData& gradient,
                           float min, float max,
                           const ImVec2* points, int count,
                           float thickness, int resolution, bool closed = false);
```

#### `DrawProceduralColorArcBilinear`

Sample a 1D color callback along a circular arc; useful for gradient rings.

```cpp
void DrawProceduralColorArcBilinear(ImDrawList* dl, ImVec2 center,
                                    float innerRadius, float outerRadius,
                                    float startAngle, float sweepAngle,
                                    ImWidgetsColor1DCallback func, void* pUserData,
                                    int division, bool bilinear);
```

#### `DrawProceduralColorSplineBilinear`

Sample a 1D color callback along a Bezier spline.

```cpp
void DrawProceduralColorSplineBilinear(ImDrawList* dl,
                                       const ImVec2* points, int count,
                                       float thickness,
                                       ImWidgetsColor1DCallback func, void* pUserData,
                                       int resolution, bool closed);
```

#### `DrawPolylineAA` -- solid GPU variant

Solid (non-dashed) GPU anti-aliased polyline now has its own dispatch, avoiding overhead from the dashed code path when `dashes == NULL`.

---

### Dashed Line Parametrization Change (breaking)

`DrawDashedPolylineAA`, `DrawStrokedDashedBezierPath`, and `DrawStrokedDashedPolyline` have a **new gap semantics**. Previously a positive `gap_len` always produced a visible gap regardless of cap style, which caused unintended space at `gap_len = 0`. The new semantics:

| `gap_len` | Result |
|---|---|
| `= 0` | Dashes are back-to-back: the trailing cap of one envelope touches the leading cap of the next. No visible gap, regardless of cap style. |
| `> 0` | Visible gap of exactly `gap_len` pixels between the edges of adjacent cap envelopes. |
| `< 0` | Overlapping envelopes: caps from adjacent dashes interpenetrate by `|gap_len|` pixels. Useful for creating seamless joined-cap effects. |

`dash_len` is the total envelope length (cap tip to cap tip). For outward caps (Square, Round, TriangleOut) the body is shorter than the envelope by `2 x (thickness/2)`. For `TriangleIn` the envelope equals the body (the notch is carved inward; no outward extension).

**Migration:** callers that previously passed `gap_len = thickness` to produce a flush result should now pass `gap_len = 0`.

---

### Bug Fixes

- **`TriangleIn` cap in dashed lines.** Both the CPU (Alg 1 / `DrawDashedPolylineAA`) and GPU (Alg 2 / `DrawStrokedDashedBezierPath`) paths now correctly handle `TriangleIn`. The cap extends zero units outward (the notch is carved *into* the body endpoint), so `cap_ext = 0` for TriangleIn everywhere. The GPU shader evaluates an inward-notch SDF at both ends of each envelope.
- **Dash-length clamp parity.** Alg 2 implicitly clamped very short dashes (body = 0 when `dash_len < 2 x cap_ext`) but Alg 1 did not, producing cap-overlap artefacts. Alg 1 now pre-clamps every even-indexed dash to `max(dash_len, 2 x cap_ext)` before splitting, matching Alg 2 behaviour.
- **GPU dashed phantom-segment.** The phantom-segment discard (used for wrap detection at polyline endpoints) is now correctly skipped when `gap_len <= 0`, preventing incorrect pixel rejection in flush/overlap mode.

---

### Typography

- `DrawTextGradient` -- simple left-to-right color gradient across a Slug-rendered string.
- `DrawLinearGradientTextGPU` / `DrawRadialGradientTextGPU` / `DrawDiamondGradientTextGPU` -- GPU gradient fills on GPU-rendered text, evaluated per-glyph or per-string, with color-space selection.
- `DrawImageTextGPU` -- GPU text with an arbitrary texture fill (single or multi-texture).
- `TesselateTextPerGlyph` -- tessellate text and return one `ImWidgetsShape` per glyph for custom downstream processing.
- `ExtractTextContours` -- extract flat polyline contours from tessellated text.

---

### Other Improvements

- `SliderRingFloat/Int/Scalar` and `SliderSplineFloat/Int/Scalar` -- new shaped sliders (ring arc and Bezier spline track).
- `GradientSample` / `GradientAlphaSample` -- sample an `ImGradientData` gradient at a normalised `t`.
- `GradientEditor` -- rationalized implementation; alpha/color track are now decoupled.
- `VectorScope` -- added CCW ring variant.
- `DrawCheckerboard` -- now in public API.
- `DrawGradientBar` -- now in public API.
- `PrebuildShaders` -- pre-warms all internal shader pipelines at startup.
- `ShowStyleEditor` -- opens the DearWidgets style editor window.
- Line fringe and memory fix in `DrawPolylineAA` wide-line path.
- Image inspector improvements.
