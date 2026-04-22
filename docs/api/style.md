# Style System

## Overview

`ImWidgetsStyle` mirrors Dear ImGui's style pattern. Each widget has named color slots (`ImWidgetsStyleColor`) and float/vec2 metric slots (`ImWidgetsStyleVar`). A push/pop stack lets you override style locally within a scope.

The global style is accessed via `ImWidgets::GetStyle()`.

---

## Style Colors (`ImWidgetsStyleColor`)

Each constant maps to a named `ImVec4` color stored in `ImWidgetsStyle::Colors[]`.

| Group | Color Name | Purpose |
|---|---|---|
| General | `StyleColor_Value` | Generic value indicator |
| Slider2D | `StyleColor_Slider2D_CursorX` | X-axis cursor |
| Slider2D | `StyleColor_Slider2D_CursorY` | Y-axis cursor |
| SliderRing | `StyleColor_SliderRing_Track` | Inactive track arc |
| SliderRing | `StyleColor_SliderRing_TrackActive` | Filled arc (min -> current) |
| SliderRing | `StyleColor_SliderRing_Grab` | Grab handle |
| SliderRing | `StyleColor_SliderRing_GrabActive` | Grab handle while dragging |
| SliderSpline | `StyleColor_SliderSpline_Track` | Inactive track |
| SliderSpline | `StyleColor_SliderSpline_TrackActive` | Filled track |
| SliderSpline | `StyleColor_SliderSpline_Grab` | Grab handle |
| SliderSpline | `StyleColor_SliderSpline_GrabActive` | Grab handle while dragging |
| GradientEditor | `StyleColor_Gradient_MarkerOutline` | Stop marker border |
| GradientEditor | `StyleColor_Gradient_MarkerOutlineHovered` | ... when hovered |
| GradientEditor | `StyleColor_Gradient_MarkerOutlineSelected` | ... when selected |
| GradientEditor | `StyleColor_Gradient_AlphaIndicator` | Alpha strip overlay |
| GradientEditor | `StyleColor_Gradient_Checkerboard1/2` | Transparency pattern |
| CurveEditor | `StyleColor_CurveEditor_Line` | Curve line |
| CurveEditor | `StyleColor_CurveEditor_GridMinor/Major` | Grid lines |
| CurveEditor | `StyleColor_CurveEditor_ZeroLine` | Zero-value line |
| CurveEditor | `StyleColor_CurveEditor_Key/KeyHovered/KeySelected` | Key dots |
| CurveEditor | `StyleColor_CurveEditor_KeyOutline*` | Key dot borders |
| CurveEditor | `StyleColor_CurveEditor_TangentLine/TangentHovered` | Tangent arms |
| CurveEditor | `StyleColor_CurveEditor_Crosshair/AddIndicator/AxisLabel` | UX chrome |
| ColorWheel | `StyleColor_ColorWheel_DotOutline/DotOutlineActive` | Cursor dot borders |
| ColorWheel | `StyleColor_ColorWheel_Crosshair/SliderOutline` | UX chrome |
| ColorWarper | `StyleColor_ColorWarper_GridLine/GridLineHovered` | Mesh lines |
| ColorWarper | `StyleColor_ColorWarper_PointOutline/PointOutlineActive` | Grid point borders |
| ColorWarper | `StyleColor_ColorWarper_PointFill/PointFillMoved` | Grid point fills |
| ColorWarper | `StyleColor_ColorWarper_Crosshair` | Crosshair |
| ColorCurve | `StyleColor_ColorCurve_Background/Grid/Line` | Background, grid, curve |
| ColorCurve | `StyleColor_ColorCurve_NeutralLine` | Identity line |
| ColorCurve | `StyleColor_ColorCurve_Key*` | Key dots |
| ColorCurve | `StyleColor_ColorCurve_Crosshair/HistogramOverlay` | UX chrome |
| ParadeScope | `StyleColor_ParadeScope_Background/Grid` | Background + grid |
| ParadeScope | `StyleColor_ParadeScope_Channel{R,G,B,Luma,Cb,Cr}` | Channel colors |
| ParadeScope | `StyleColor_ParadeScope_GradTick/GradLabel` | Graduation marks |
| VectorScope | `StyleColor_VectorScope_Background/Grid/Graticule` | Background + grid |
| VectorScope | `StyleColor_VectorScope_Signal/SkinToneLine` | Signal + indicator |
| VectorScope | `StyleColor_VectorScope_Target{R,G,B,Cy,Mg,Yl}` | Target markers |
| Histogram | `StyleColor_Histogram_Background/Grid` | Background + grid |
| Histogram | `StyleColor_Histogram_Channel{R,G,B,Luma,Cb,Cr,H,S,V,OkL,OkC,OkH}` | Channel colors |
| Histogram | `StyleColor_Histogram_GradTick/GradLabel` | Graduation marks |
| CIEChromaticity | `StyleColor_CIEChromaticity_Background/Grid` | Background + grid |
| CIEChromaticity | `StyleColor_CIEChromaticity_Signal/GamutLine/WhitePoint` | Plot elements |
| CIEChromaticity | `StyleColor_CIEChromaticity_Primary{R,G,B}` | Primary markers |
| CIEChromaticity | `StyleColor_CIEChromaticity_GradTick/GradLabel` | Graduation marks |
| ToneCurve | `StyleColor_ToneCurve_Background/Grid/NeutralLine` | Background, grid, identity |
| ToneCurve | `StyleColor_ToneCurve_Key/KeySelected/KeyOutline` | Key dots |
| ToneCurve | `StyleColor_ToneCurve_Crosshair/HistogramOverlay` | UX chrome |
| ToneCurve | `StyleColor_ToneCurve_GradTick/GradLabel` | Graduation marks |
| HDRWheel | `StyleColor_HDRWheel_RightArc` | Right arc fill |
| HDRWheel | `StyleColor_HDRWheel_LeftArcMin/Max` | Left arc gradient |
| Gizmo | `StyleColor_Gizmo_Canvas/Outline/Handle/HandleActive` | Canvas chrome |
| ColorPicker | `StyleColor_ColorPicker_DotOutline/DotOutlineActive` | Cursor dot |
| ColorPicker | `StyleColor_ColorPicker_Crosshair/SliderOutline/SliderHandle` | UX chrome |

---

## Style Vars (`ImWidgetsStyleVar`)

Float metrics per widget group. All values are in pixels unless noted.

| Group | Var | Default | Description |
|---|---|---|---|
| HueSelector | `StyleVar_HueSelector_Thickness_ZeroWidth` | 2 | Bar thickness at zero-width |
| Slider2D | `StyleVar_Slider2D_DragThickness` | 8 | Border drag zone width |
| Slider2D | `StyleVar_Slider2D_BorderThickness` | 2 | Outer border |
| Slider2D | `StyleVar_Slider2D_LineThickness` | 2 | Cursor cross lines |
| Slider2D | `StyleVar_Slider2D_CursorRadius` | 4 | Cursor dot radius |
| Slider2D | `StyleVar_Slider2D_CursorOffset` | 16 | Cursor distance from border |
| Slider2D | `StyleVar_Slider2D_CornerRadius` | 2 | Border corner rounding |
| SliderRing | `StyleVar_SliderRing_TrackThickness` | 8 | Track arc thickness |
| SliderRing | `StyleVar_SliderRing_GrabRadius` | 7 | Grab handle radius |
| SliderSpline | `StyleVar_SliderSpline_TrackThickness` | 4 | Track line thickness |
| SliderSpline | `StyleVar_SliderSpline_GrabRadius` | 7 | Grab handle radius |
| General | `StyleVar_NavCursor_Thickness` | 2 | Nav cursor outline |
| General | `StyleVar_NavCursor_Distance` | 3 | Nav cursor gap from shape |
| General | `StyleVar_WhitePoint_Radius` | 5 | White-point marker radius |
| General | `StyleVar_PrecisionDrag_BlockSize` | 28 | Precision drag grid block |
| GradientEditor | `StyleVar_Gradient_MarkerHeight` | 12 | Stop marker height |
| GradientEditor | `StyleVar_Gradient_CheckerboardCellSize` | 6 | Checkerboard cell size |
| GradientEditor | `StyleVar_Gradient_MarkerThickness` | 1 | Stop marker outline |
| CurveEditor | `StyleVar_CurveEditor_DefaultHeight` | 200 | Default widget height |
| CurveEditor | `StyleVar_CurveEditor_KeyRadius` | 5 | Key dot radius |
| CurveEditor | `StyleVar_CurveEditor_TangentRadius` | 4 | Tangent handle radius |
| CurveEditor | `StyleVar_CurveEditor_HitRadius` | 8 | Click hit zone |
| CurveEditor | `StyleVar_CurveEditor_LineThickness` | 2 | Curve line thickness |
| CurveEditor | `StyleVar_CurveEditor_KeyOutlineThickness` | 1.5 | Key outline thickness |
| CurveEditor | `StyleVar_CurveEditor_ZeroLineThickness` | 1 | Zero-line thickness |
| ColorWheel | `StyleVar_ColorWheel_DotRadius` | 6 | Cursor dot radius |
| ColorWheel | `StyleVar_ColorWheel_RingThickness` | 12 | Hue ring thickness |
| ColorWheel | `StyleVar_ColorWheel_DiscSectors` | 96 | Angular resolution |
| ColorWheel | `StyleVar_ColorWheel_DiscRings` | 24 | Radial resolution |
| ColorWheel | `StyleVar_ColorWheel_SliderHeight` | 20 | Master slider height |
| ColorWarper | `StyleVar_ColorWarper_PointRadius` | 4 | Control point radius |
| ColorWarper | `StyleVar_ColorWarper_HitRadius` | 8 | Click hit zone |
| ColorWarper | `StyleVar_ColorWarper_GridThickness` | 1 | Grid line thickness |
| ColorWarper | `StyleVar_ColorWarper_DiscSectors` | 96 | Background disc resolution |
| ColorWarper | `StyleVar_ColorWarper_DiscRings` | 24 | Background disc rings |
| ColorCurve | `StyleVar_ColorCurve_DefaultHeight` | 150 | Default widget height |
| ColorCurve | `StyleVar_ColorCurve_KeyRadius` | 5 | Key dot radius |
| ColorCurve | `StyleVar_ColorCurve_LineThickness` | 2 | Curve line thickness |
| ParadeScope | `StyleVar_ParadeScope_DefaultHeight` | 200 | Default widget height |
| ParadeScope | `StyleVar_ParadeScope_OverlayAlpha` | 0.6 | Max alpha (overlay mode) |
| ParadeScope | `StyleVar_ParadeScope_GradTickLength` | 6 | Tick length |
| ParadeScope | `StyleVar_ParadeScope_GradTickThickness` | 1 | Tick thickness |
| ParadeScope | `StyleVar_ParadeScope_GradMargin` | 40 | Left margin for labels |
| VectorScope | `StyleVar_VectorScope_DefaultSize` | 200 | Default side length |
| VectorScope | `StyleVar_VectorScope_SignalAlpha` | 0.8 | Max signal alpha |
| VectorScope | `StyleVar_VectorScope_GraticuleThickness` | 1 | Graticule line thickness |
| Histogram | `StyleVar_Histogram_DefaultHeight` | 200 | Default widget height |
| Histogram | `StyleVar_Histogram_OverlayAlpha` | 0.6 | Max alpha (overlay mode) |
| Histogram | `StyleVar_Histogram_GradTickLength` | 6 | Tick length |
| Histogram | `StyleVar_Histogram_GradTickThickness` | 1 | Tick thickness |
| Histogram | `StyleVar_Histogram_GradMarginLeft` | 40 | Left margin for labels |
| Histogram | `StyleVar_Histogram_GradMarginBottom` | 20 | Bottom margin for labels |
| CIEChromaticity | `StyleVar_CIEChromaticity_DefaultSize` | 300 | Default side length |
| CIEChromaticity | `StyleVar_CIEChromaticity_SignalRadius` | 1.5 | Signal dot radius |
| CIEChromaticity | `StyleVar_CIEChromaticity_GamutLineThickness` | 1.5 | Gamut triangle line |
| CIEChromaticity | `StyleVar_CIEChromaticity_WhitePointRadius` | 4 | White-point marker radius |
| CIEChromaticity | `StyleVar_CIEChromaticity_GradMargin` | 30 | Margin for grad labels |
| CIEChromaticity | `StyleVar_CIEChromaticity_SignalAlpha` | 0.6 | Signal alpha |
| ToneCurve | `StyleVar_ToneCurve_DefaultHeight` | 200 | Default widget height |
| ToneCurve | `StyleVar_ToneCurve_KeyRadius` | 5 | Key dot radius |
| ToneCurve | `StyleVar_ToneCurve_LineThickness` | 2 | Curve line thickness |
| ToneCurve | `StyleVar_ToneCurve_GradMarginLeft` | 30 | Left margin for labels |
| ToneCurve | `StyleVar_ToneCurve_GradMarginBottom` | 20 | Bottom margin for labels |
| ToneCurve | `StyleVar_ToneCurve_BandThickness` | 8 | Gradient band thickness |
| ToneCurve | `StyleVar_ToneCurve_BandGap` | 2 | Gap between scope and band |
| ColorPicker | `StyleVar_ColorPicker_DotRadius` | 6 | Cursor dot radius |
| ColorPicker | `StyleVar_ColorPicker_PlaneResolution` | 24 | 2D plane grid resolution |
| ColorPicker | `StyleVar_ColorPicker_SliderWidth` | 20 | Vertical slider width |
| ColorPicker | `StyleVar_ColorPicker_SliderResolution` | 16 | Vertical slider segments |
| ColorPicker | `StyleVar_ColorPicker_ComponentSliderHeight` | 16 | Component slider height |

---

## Functions

### `GetStyle`
```cpp
ImWidgetsStyle& GetStyle();
```
Returns a reference to the global style. Modify it directly to change defaults.

---

### `ShowStyleEditor`
```cpp
void ShowStyleEditor(ImWidgetsStyle* ref = NULL);
```
Opens an interactive style editor window. `ref` = reference style for the "Revert" button; `NULL` uses the default-constructed style.

---

### `PushStyleColor` / `PopStyleColor`
```cpp
void PushStyleColor(ImWidgetsStyleColor colorIndex, const ImVec4& color);
void PopStyleColor(int count = 1);
```
Temporarily override a color. Must be balanced with `PopStyleColor`.

---

### `PushStyleVar` / `PopStyleVar`
```cpp
void PushStyleVar(ImWidgetsStyleVar varIndex, float value);
void PushStyleVar(ImWidgetsStyleVar varIndex, const ImVec2& value);
void PushStyleVar(ImWidgetsStyleVar varIndex, const ImVec4& value);
void PopStyleVar(int count = 1);
```
Temporarily override a style var. Must be balanced with `PopStyleVar`.

---

### `ScaleAllSizes`
```cpp
void ImWidgetsStyle::ScaleAllSizes(float scale_factor);
```
Multiply all pixel-dimension vars by `scale_factor`. Call once after DPI changes.
