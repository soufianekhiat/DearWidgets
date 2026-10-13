# Style System

## Overview

`ImWidgetsStyle` mirrors Dear ImGui's style pattern. Each widget has named color
slots (`ImWidgetsCol`) and float metric slots (`ImWidgetsStyleVar`). A push/pop
stack lets you override style locally within a scope.

The global style is accessed via `ImWidgets::GetStyle()`.

> **Renamed in 2.0.0.** The enums were prefixed:
> `ImWidgetsStyleColor` -> `ImWidgetsCol_` (+ `typedef int ImWidgetsCol`),
> `ImWidgetsStyleVar` -> `ImWidgetsStyleVar_` (+ `typedef int ImWidgetsStyleVar`),
> every `StyleColor_Xxx` -> `ImWidgetsCol_Xxx`, every `StyleVar_Xxx` ->
> `ImWidgetsStyleVar_Xxx`, and the sentinels `_Count` -> `_COUNT`.

Metrics marked **(lp)** are *logical pixels*: multiply through
`ImWidgetsStyle::ScaleAllSizes` for DPI.

---

## Style Colors (`ImWidgetsCol`)

Each constant maps to a named `ImVec4` color stored in `ImWidgetsStyle::Colors[]`.
There are `ImWidgetsCol_COUNT` = **137** slots.

| Group | Color | Purpose |
|---|---|---|
| Slider2D | `ImWidgetsCol_Slider2D_CursorX` | Color for Slider2D X-axis cursor |
| Slider2D | `ImWidgetsCol_Slider2D_CursorY` | Color for Slider2D Y-axis cursor |
| SliderN | `ImWidgetsCol_SliderN_Cursor` | Signet cursor marking each SliderN boundary |
| Slider2DRange | `ImWidgetsCol_Slider2DRange_MinHandle` | Min-corner handle |
| Slider2DRange | `ImWidgetsCol_Slider2DRange_MaxHandle` | Max-corner handle |
| Slider2DRange | `ImWidgetsCol_Slider2DRange_Fill` | Selection rectangle fill |
| Slider2DDisc | `ImWidgetsCol_Slider2DDisc_Background` | Disc fill (transparent = use FrameBg) |
| Slider2DDisc | `ImWidgetsCol_Slider2DDisc_Cursor` | Handle dot |
| Slider2DDisc | `ImWidgetsCol_Slider2DDisc_CursorOutline` | Handle outline |
| Slider2DDisc | `ImWidgetsCol_Slider2DDisc_Ring` | Outer ring |
| Slider2DDisc | `ImWidgetsCol_Slider2DDisc_Grid` | Crosshair and concentric rings |
| SliderRing | `ImWidgetsCol_SliderRing_Track` | Track arc color |
| SliderRing | `ImWidgetsCol_SliderRing_TrackActive` | Track arc fill from min to current value |
| SliderRing | `ImWidgetsCol_SliderRing_Grab` | Grab handle fill |
| SliderRing | `ImWidgetsCol_SliderRing_GrabActive` | Grab handle fill while dragging |
| SliderSpline | `ImWidgetsCol_SliderSpline_Track` | Spline track color |
| SliderSpline | `ImWidgetsCol_SliderSpline_TrackActive` | Spline track fill from min to current value |
| SliderSpline | `ImWidgetsCol_SliderSpline_Grab` | Grab handle fill |
| SliderSpline | `ImWidgetsCol_SliderSpline_GrabActive` | Grab handle fill while dragging |
| GradientEditor | `ImWidgetsCol_Gradient_MarkerOutline` |  |
| GradientEditor | `ImWidgetsCol_Gradient_MarkerOutlineHovered` |  |
| GradientEditor | `ImWidgetsCol_Gradient_MarkerOutlineSelected` |  |
| GradientEditor | `ImWidgetsCol_Gradient_AlphaIndicator` |  |
| GradientEditor | `ImWidgetsCol_Gradient_Checkerboard1` |  |
| GradientEditor | `ImWidgetsCol_Gradient_Checkerboard2` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_Line` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_GridMinor` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_GridMajor` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_ZeroLine` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_Key` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_KeyHovered` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_KeySelected` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_KeyOutline` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_KeyOutlineHovered` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_KeyOutlineSelected` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_TangentLine` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_TangentHovered` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_Crosshair` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_AddIndicator` |  |
| CurveEditor | `ImWidgetsCol_CurveEditor_AxisLabel` |  |
| ColorWheel | `ImWidgetsCol_ColorWheel_DotOutline` |  |
| ColorWheel | `ImWidgetsCol_ColorWheel_DotOutlineActive` |  |
| ColorWheel | `ImWidgetsCol_ColorWheel_Crosshair` |  |
| ColorWheel | `ImWidgetsCol_ColorWheel_SliderOutline` |  |
| ColorWarper | `ImWidgetsCol_ColorWarper_GridLine` |  |
| ColorWarper | `ImWidgetsCol_ColorWarper_GridLineHovered` |  |
| ColorWarper | `ImWidgetsCol_ColorWarper_PointOutline` |  |
| ColorWarper | `ImWidgetsCol_ColorWarper_PointOutlineActive` |  |
| ColorWarper | `ImWidgetsCol_ColorWarper_PointFill` |  |
| ColorWarper | `ImWidgetsCol_ColorWarper_PointFillMoved` |  |
| ColorWarper | `ImWidgetsCol_ColorWarper_Crosshair` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_Background` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_Grid` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_Line` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_NeutralLine` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_Key` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_KeyHovered` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_KeySelected` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_KeyOutline` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_Crosshair` |  |
| ColorCurve | `ImWidgetsCol_ColorCurve_HistogramOverlay` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_Background` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_Grid` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_ChannelR` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_ChannelG` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_ChannelB` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_ChannelLuma` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_ChannelCb` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_ChannelCr` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_GradTick` |  |
| ParadeScope | `ImWidgetsCol_ParadeScope_GradLabel` |  |
| VectorScope | `ImWidgetsCol_VectorScope_Background` |  |
| VectorScope | `ImWidgetsCol_VectorScope_Grid` |  |
| VectorScope | `ImWidgetsCol_VectorScope_Graticule` |  |
| VectorScope | `ImWidgetsCol_VectorScope_Signal` |  |
| VectorScope | `ImWidgetsCol_VectorScope_SkinToneLine` |  |
| VectorScope | `ImWidgetsCol_VectorScope_TargetR` |  |
| VectorScope | `ImWidgetsCol_VectorScope_TargetG` |  |
| VectorScope | `ImWidgetsCol_VectorScope_TargetB` |  |
| VectorScope | `ImWidgetsCol_VectorScope_TargetCy` |  |
| VectorScope | `ImWidgetsCol_VectorScope_TargetMg` |  |
| VectorScope | `ImWidgetsCol_VectorScope_TargetYl` |  |
| Histogram | `ImWidgetsCol_Histogram_Background` |  |
| Histogram | `ImWidgetsCol_Histogram_Grid` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelR` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelG` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelB` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelLuma` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelCb` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelCr` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelH` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelS` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelV` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelOkL` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelOkC` |  |
| Histogram | `ImWidgetsCol_Histogram_ChannelOkH` |  |
| Histogram | `ImWidgetsCol_Histogram_GradTick` |  |
| Histogram | `ImWidgetsCol_Histogram_GradLabel` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_Background` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_Grid` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_Signal` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_GamutLine` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_WhitePoint` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_PrimaryR` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_PrimaryG` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_PrimaryB` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_GradTick` |  |
| CIEChromaticity | `ImWidgetsCol_CIEChromaticity_GradLabel` |  |
| ChromaticityPlot | `ImWidgetsCol_ChromaticityPlot_Triangle` | Color space triangle outline |
| ChromaticityPlot | `ImWidgetsCol_ChromaticityPlot_WhitePoint` | White point marker fill |
| ChromaticityLine | `ImWidgetsCol_ChromaticityLine_Default` | Default fallback line color |
| ChromaticityPoint | `ImWidgetsCol_ChromaticityPoint_Default` | Default fallback point color |
| HueSelector | `ImWidgetsCol_HueSelector_ZeroWidthLine` | Vertical line when width=0 |
| HueSelector | `ImWidgetsCol_HueSelector_Cursor` | Triangle cursor fill |
| ToneCurve | `ImWidgetsCol_ToneCurve_Background` |  |
| ToneCurve | `ImWidgetsCol_ToneCurve_Grid` |  |
| ToneCurve | `ImWidgetsCol_ToneCurve_NeutralLine` |  |
| ToneCurve | `ImWidgetsCol_ToneCurve_Key` |  |
| ToneCurve | `ImWidgetsCol_ToneCurve_KeySelected` |  |
| ToneCurve | `ImWidgetsCol_ToneCurve_KeyOutline` |  |
| ToneCurve | `ImWidgetsCol_ToneCurve_Crosshair` |  |
| ToneCurve | `ImWidgetsCol_ToneCurve_HistogramOverlay` |  |
| ToneCurve | `ImWidgetsCol_ToneCurve_GradTick` |  |
| ToneCurve | `ImWidgetsCol_ToneCurve_GradLabel` |  |
| HDRWheel | `ImWidgetsCol_HDRWheel_RightArc` | Right arc fill color (default red) |
| HDRWheel | `ImWidgetsCol_HDRWheel_LeftArcMin` | Left arc gradient start color (black) |
| HDRWheel | `ImWidgetsCol_HDRWheel_LeftArcMax` | Left arc gradient end color (white) |
| TransformGizmo | `ImWidgetsCol_Gizmo_Canvas` | Canvas background |
| TransformGizmo | `ImWidgetsCol_Gizmo_Outline` | Bounding box outline |
| TransformGizmo | `ImWidgetsCol_Gizmo_Handle` | Handle fill |
| TransformGizmo | `ImWidgetsCol_Gizmo_HandleActive` | Handle fill when active |
| TransformGizmo | `ImWidgetsCol_Gizmo_DimOutline` | Unselected image outline (low alpha) |
| ColorPicker | `ImWidgetsCol_ColorPicker_DotOutline` | Dot outline (white) |
| ColorPicker | `ImWidgetsCol_ColorPicker_DotOutlineActive` | Dot outline when active (yellow) |
| ColorPicker | `ImWidgetsCol_ColorPicker_Crosshair` | Crosshair lines (white, low alpha) |
| ColorPicker | `ImWidgetsCol_ColorPicker_SliderOutline` | Vertical slider outline (black, medium alpha) |
| ColorPicker | `ImWidgetsCol_ColorPicker_SliderHandle` | Vertical slider handle (white) |

---

## Style Vars (`ImWidgetsStyleVar`)

Float metrics per widget group, `ImWidgetsStyleVar_COUNT` = **123** slots.
All values are logical pixels unless the description says otherwise.

| Group | Var | Default | Description |
|---|---|---|---|
| HueSelector | `ImWidgetsStyleVar_HueSelector_Thickness_ZeroWidth` | 5 | Bar thickness at zero-width |
| Slider2D | `ImWidgetsStyleVar_Slider2D_DragThickness` | 8 | Border drag zone width |
| Slider2D | `ImWidgetsStyleVar_Slider2D_BorderThickness` | 2 | Outer border |
| Slider2D | `ImWidgetsStyleVar_Slider2D_LineThickness` | 2 | Cursor cross lines |
| Slider2D | `ImWidgetsStyleVar_Slider2D_CursorRadius` | 7.048 | Cursor dot radius |
| Slider2D | `ImWidgetsStyleVar_Slider2D_CursorOffset` | 12 | Cursor distance from border |
| Slider2D | `ImWidgetsStyleVar_Slider2D_CornerRadius` | 4 | Border corner rounding |
| Slider2DRange | `ImWidgetsStyleVar_Slider2DRange_FillAlpha` | 0.15 |  |
| Slider2DRange | `ImWidgetsStyleVar_Slider2DRange_HandleRadius` | 5 | corner handle size (lp) |
| Slider2DDisc | `ImWidgetsStyleVar_Slider2DDisc_CursorRadius` | 5 |  |
| Slider2DDisc | `ImWidgetsStyleVar_Slider2DDisc_CursorOutlineThickness` | 1.5 |  |
| Slider2DDisc | `ImWidgetsStyleVar_Slider2DDisc_RingThickness` | 2 |  |
| SliderRing | `ImWidgetsStyleVar_SliderRing_TrackThickness` | 12 | Track arc thickness (lp) |
| SliderRing | `ImWidgetsStyleVar_SliderRing_GrabRadius` | 8 | Grab handle radius (lp) |
| SliderSpline | `ImWidgetsStyleVar_SliderSpline_TrackThickness` | 12 | Spline track thickness (lp) |
| SliderSpline | `ImWidgetsStyleVar_SliderSpline_GrabRadius` | 12 | Grab handle radius (lp) |
| General | `ImWidgetsStyleVar_NavCursor_Thickness` | 5 | Nav cursor outline |
| General | `ImWidgetsStyleVar_NavCursor_Distance` | 7 | Nav cursor gap from shape |
| General | `ImWidgetsStyleVar_WhitePoint_Radius` | 5 | White-point marker radius |
| General | `ImWidgetsStyleVar_PrecisionDrag_BlockSize` | 70 | Precision drag grid block |
| GradientEditor | `ImWidgetsStyleVar_Gradient_MarkerHeight` | 30 | Stop marker height |
| GradientEditor | `ImWidgetsStyleVar_Gradient_CheckerboardCellSize` | 15 | Checkerboard cell size |
| GradientEditor | `ImWidgetsStyleVar_Gradient_MarkerThickness` | 2 | Stop marker outline |
| CurveEditor | `ImWidgetsStyleVar_CurveEditor_DefaultHeight` | 500 | Default widget height |
| CurveEditor | `ImWidgetsStyleVar_CurveEditor_KeyRadius` | 6 | Key dot radius |
| CurveEditor | `ImWidgetsStyleVar_CurveEditor_TangentRadius` | 5 | Tangent handle radius |
| CurveEditor | `ImWidgetsStyleVar_CurveEditor_HitRadius` | 22 | Click hit zone |
| CurveEditor | `ImWidgetsStyleVar_CurveEditor_LineThickness` | 4 | Curve line thickness |
| CurveEditor | `ImWidgetsStyleVar_CurveEditor_KeyOutlineThickness` | 2 | Key outline thickness |
| CurveEditor | `ImWidgetsStyleVar_CurveEditor_ZeroLineThickness` | 2 | Zero-line thickness |
| ColorWheel | `ImWidgetsStyleVar_ColorWheel_DotRadius` | 8 | Cursor dot radius |
| ColorWheel | `ImWidgetsStyleVar_ColorWheel_RingThickness` | 16 | Hue ring thickness |
| ColorWheel | `ImWidgetsStyleVar_ColorWheel_SliderHeight` | 32 | Master slider height (lp) |
| ColorWarper | `ImWidgetsStyleVar_ColorWarper_PointRadius` | 8 | Control point dot radius (lp) |
| ColorWarper | `ImWidgetsStyleVar_ColorWarper_HitRadius` | 18 | Hit testing radius for points (lp) |
| ColorWarper | `ImWidgetsStyleVar_ColorWarper_GridThickness` | 2 | Mesh grid line thickness (lp) |
| ColorWarper | `ImWidgetsStyleVar_ColorWarper_DiscScale` | 1 | Disc radius as fraction of half-side (0..1) |
| ColorCurve | `ImWidgetsStyleVar_ColorCurve_DefaultHeight` | 375 | Default widget height |
| ColorCurve | `ImWidgetsStyleVar_ColorCurve_KeyRadius` | 8 | Key dot radius |
| ColorCurve | `ImWidgetsStyleVar_ColorCurve_LineThickness` | 4 | Curve line thickness |
| ParadeScope | `ImWidgetsStyleVar_ParadeScope_DefaultHeight` | 500 | Default widget height |
| ParadeScope | `ImWidgetsStyleVar_ParadeScope_OverlayAlpha` | 0.75 | Max alpha in overlay mode (0..1) |
| ParadeScope | `ImWidgetsStyleVar_ParadeScope_GradTickLength` | 12 | Graduation tick mark length (lp) |
| ParadeScope | `ImWidgetsStyleVar_ParadeScope_GradTickThickness` | 3.27 | Graduation tick line thickness |
| ParadeScope | `ImWidgetsStyleVar_ParadeScope_GradMargin` | 40 | Left margin for graduation labels (lp) |
| VectorScope | `ImWidgetsStyleVar_VectorScope_DefaultSize` | 500 | Default square side length (lp) |
| VectorScope | `ImWidgetsStyleVar_VectorScope_SignalAlpha` | 0.75 | Max alpha for signal dots (0..1) |
| VectorScope | `ImWidgetsStyleVar_VectorScope_GraticuleThickness` | 1 | Graticule line thickness (lp) |
| VectorScope | `ImWidgetsStyleVar_VectorScope_DiscScale` | 1 | Disc/graticule radius as fraction of half-side (0..1) |
| Histogram | `ImWidgetsStyleVar_Histogram_DefaultHeight` | 500 | Default widget height (lp) |
| Histogram | `ImWidgetsStyleVar_Histogram_OverlayAlpha` | 0.5 | Max alpha in overlay mode (0..1) |
| Histogram | `ImWidgetsStyleVar_Histogram_GradTickLength` | 10 | Graduation tick mark length (lp) |
| Histogram | `ImWidgetsStyleVar_Histogram_GradTickThickness` | 3 | Graduation tick line thickness |
| Histogram | `ImWidgetsStyleVar_Histogram_GradMarginLeft` | 42 | Left margin for Y-axis labels (lp) |
| Histogram | `ImWidgetsStyleVar_Histogram_GradMarginBottom` | 24 | Bottom margin for X-axis labels (lp) |
| CIEChromaticity | `ImWidgetsStyleVar_CIEChromaticity_DefaultSize` | 750 | Default square side (lp) |
| CIEChromaticity | `ImWidgetsStyleVar_CIEChromaticity_SignalRadius` | 3 | Signal dot radius (lp) |
| CIEChromaticity | `ImWidgetsStyleVar_CIEChromaticity_GamutLineThickness` | 3 | Gamut triangle line thickness (lp) |
| CIEChromaticity | `ImWidgetsStyleVar_CIEChromaticity_WhitePointRadius` | 10 | White point marker radius (lp) |
| CIEChromaticity | `ImWidgetsStyleVar_CIEChromaticity_GradMargin` | 24 | Left/bottom margin for graduation labels (lp) |
| CIEChromaticity | `ImWidgetsStyleVar_CIEChromaticity_SignalAlpha` | 0.6 | Signal point alpha (0..1) |
| CIEChromaticity | `ImWidgetsStyleVar_CIEChromaticity_GradMarginRight` | 64 | Right margin (lp) |
| CIEChromaticity | `ImWidgetsStyleVar_CIEChromaticity_GradMarginTop` | 16 | Top margin (lp) |
| ChromaticityPlot | `ImWidgetsStyleVar_ChromaticityPlot_BorderThickness` | 2 | Horseshoe outline thickness (lp) |
| ChromaticityPlot | `ImWidgetsStyleVar_ChromaticityPlot_TriangleThickness` | 3 | Color space triangle thickness (lp) |
| ChromaticityLine | `ImWidgetsStyleVar_ChromaticityLine_Thickness` | 3 | Polyline thickness (lp) |
| ChromaticityPoint | `ImWidgetsStyleVar_ChromaticityPoint_Radius` | 8 | Point radius (lp) |
| ToneCurve | `ImWidgetsStyleVar_ToneCurve_DefaultHeight` | 500 | Default widget height (lp) |
| ToneCurve | `ImWidgetsStyleVar_ToneCurve_KeyRadius` | 8 | Key dot radius (lp) |
| ToneCurve | `ImWidgetsStyleVar_ToneCurve_LineThickness` | 4 | Curve line thickness (lp) |
| ToneCurve | `ImWidgetsStyleVar_ToneCurve_GradMarginLeft` | 24 | Left margin for Y-axis labels (lp) |
| ToneCurve | `ImWidgetsStyleVar_ToneCurve_GradMarginBottom` | 24 | Bottom margin for X-axis labels (lp) |
| ToneCurve | `ImWidgetsStyleVar_ToneCurve_BandThickness` | 16 | Gradient band thickness (lp) |
| ToneCurve | `ImWidgetsStyleVar_ToneCurve_BandGap` | 4 | Gap between scope and band (lp) |
| ToneCurve | `ImWidgetsStyleVar_ToneCurve_GradMarginRight` | 0 | Right margin (lp) |
| ToneCurve | `ImWidgetsStyleVar_ToneCurve_GradMarginTop` | 4 | Top margin (lp) |
| HDRWheel | `ImWidgetsStyleVar_HDRWheel_RingThickness` | 8 | Gradient indicator ring thickness (lp) |
| HDRWheel | `ImWidgetsStyleVar_HDRWheel_RingGap` | 2 | Gap between indicator ring and inner ColorWheel (lp) |
| TransformGizmo | `ImWidgetsStyleVar_Gizmo_HandleSize` | 6 | Corner handle half-size (lp) |
| TransformGizmo | `ImWidgetsStyleVar_Gizmo_RotationHandleOffset` | 62 | Distance from top edge to rotation handle (lp) |
| TransformGizmo | `ImWidgetsStyleVar_Gizmo_OutlineThickness` | 3 | Bounding box outline thickness (lp) |
| TransformGizmo | `ImWidgetsStyleVar_Gizmo_RotationLineThickness` | 2 | Rotation stem line thickness (lp) |
| TransformGizmo | `ImWidgetsStyleVar_Gizmo_CenterDotRadius` | 4 | Selected-object center dot radius (lp) |
| TransformGizmo | `ImWidgetsStyleVar_Gizmo_EdgeHandleScale` | 0.8 | Edge handle size as fraction of corner handle size |
| TransformGizmo | `ImWidgetsStyleVar_Gizmo_DefaultCanvasHeight` | 400 | Canvas height when no size given (lp) |
| ColorPicker | `ImWidgetsStyleVar_ColorPicker_DotRadius` | 8 | Dot indicator radius (lp) |
| ColorPicker | `ImWidgetsStyleVar_ColorPicker_SliderWidth` | 24 | Vertical slider width (lp) |
| ColorPicker | `ImWidgetsStyleVar_ColorPicker_ComponentSliderHeight` | 24 | Component slider height (lp) |
| ColorPicker | `ImWidgetsStyleVar_ColorPicker_DotOutlineThickness` | 1 | Dot outline ring thickness (lp) |
| ColorPicker | `ImWidgetsStyleVar_ColorPicker_SliderHandleHeight` | 6 | Vertical slider handle height (lp) |
| ColorPicker | `ImWidgetsStyleVar_ColorPicker_ComponentHandleWidth` | 6 | Component slider handle width (lp) |
| Hatching | `ImWidgetsStyleVar_Hatch_DefaultSpacing` | 20 |  |
| Hatching | `ImWidgetsStyleVar_Hatch_DefaultThickness` | 2 |  |
| BezierCurve | `ImWidgetsStyleVar_BezierCurve_KeyRadius` | 12 |  |
| BezierCurve | `ImWidgetsStyleVar_BezierCurve_TangentRadius` | 10 |  |
| BezierCurve | `ImWidgetsStyleVar_BezierCurve_LineThickness` | 5 |  |
| BezierCurve | `ImWidgetsStyleVar_BezierCurve_SnapAngleDeg` | 5 |  |
| FontInspector | `ImWidgetsStyleVar_FontInspector_GlyphCell` | 180 |  |
| FontInspector | `ImWidgetsStyleVar_FontInspector_MetricAlpha` | 0.6 |  |
| NotchedDial | `ImWidgetsStyleVar_NotchedDial_TickLength` | 16 |  |
| NotchedDial | `ImWidgetsStyleVar_NotchedDial_TickThickness` | 2 |  |
| Delta-E | `ImWidgetsStyleVar_DeltaE_SwatchSize` | 128 |  |
| EquationInput | `ImWidgetsStyleVar_Equation_BlockPadding` | 20 |  |
| EquationInput | `ImWidgetsStyleVar_Equation_InlineBaselineOffset` | 0 |  |
| IsoContour | `ImWidgetsStyleVar_IsoContour_MajorThickness` | 5 |  |
| IsoContour | `ImWidgetsStyleVar_IsoContour_MediumThickness` | 2 |  |
| IsoContour | `ImWidgetsStyleVar_IsoContour_MinorThickness` | 1 |  |
| LookDev | `ImWidgetsStyleVar_LookDev_DividerThickness` | 2 |  |
| LookDev | `ImWidgetsStyleVar_LookDev_HandleRadius` | 8 |  |
| VolumeSliceViewer | `ImWidgetsStyleVar_VolumeSlice_HandleRadius` | 15 |  |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_AnchorRadius` | 8 |  |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_TangentRadius` | 5 |  |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_GridSpacing` | 64 |  |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_TangentLineThickness` | 2 | (lp) |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_AnchorOutlineThickness` | 1 | (lp) |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_ClosingRingRadius` | 12 | First-node closing-ring radius (lp) |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_ClosingRingThickness` | 2 | (lp) |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_HoverRingGap` | 6 | Gap added to anchor radius for hover highlight (lp) |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_HoverRingThickness` | 4 | (lp) |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_GhostRingThickness` | 2 | Empty-canvas ghost ring thickness (lp) |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_CrosshairArmLength` | 12 | Ghost crosshair arm length (lp) |
| VectorDrawingTool | `ImWidgetsStyleVar_VectorDrawing_CrosshairThickness` | 1 | Ghost crosshair line thickness (lp) |
| PrecisionPopup | `ImWidgetsStyleVar_PrecisionPopup_InputWidth` | 300 |  |

### Non-pushable style fields

`PushStyleVar` only carries `float` (and the `ImVec2` / `ImVec4` overloads).
These `ImWidgetsStyle` members are `int`, `bool` or an enum, so they have no
`ImWidgetsStyleVar_` entry -- set them directly on `GetStyle()`:

```cpp
ImWidgets::GetStyle().ColorWheel_DiscSectors = 96;
```

| Field | Type | Default | Description |
|---|---|---|---|
| `Slider2DDisc_GridRings` | `int` | 2 | Concentric grid rings in Slider2DDisc |
| `ColorWheel_DiscSectors` | `int` | 64 | Colour wheel disc angular resolution |
| `ColorWheel_DiscRings` | `int` | 16 | Colour wheel disc radial resolution |
| `ColorWarper_DiscSectors` | `int` | 64 | Warper background disc angular resolution |
| `ColorWarper_DiscRings` | `int` | 16 | Warper background disc radial resolution |
| `ChromaticityPlot_Resolution` | `int` | 128 | Procedural fill resolution per axis |
| `ChromaticityPlot_LineSamples` | `int` | 128 | Spectral-locus horseshoe sample count |
| `ChromaticityPoint_Segments` | `int` | 24 | Point circle segment count |
| `ChromaticityLine_SmoothIterations` | `int` | 2 | Binomial smoothing passes on the locus polyline (0 = raw) |
| `ColorPicker_PlaneResolution` | `int` | 24 | Picker 2D plane grid resolution per axis |
| `ColorPicker_SliderResolution` | `int` | 16 | Picker vertical slider segment count |
| `CurveEditor_LineMode` | `ImWidgetsThickLineMode` | `StrokedBezierPath` | How the curve polyline is stroked |
| `ColorCurve_LineMode` | `ImWidgetsThickLineMode` | `AddPolyline` | See note below |
| `ToneCurve_LineMode` | `ImWidgetsThickLineMode` | `AddPolyline` | See note below |
| `ChromaticityLine_Mode` | `ImWidgetsThickLineMode` | `StrokedBezierPath` | Chromaticity polyline stroke mode |
| `CurveEditor_LineDashed` | `bool` | `false` | Dash the curve line |
| `ColorCurve_LineDashed` | `bool` | `false` | Dash the curve line |
| `ToneCurve_LineDashed` | `bool` | `false` | Dash the curve line |
| `ChromaticityLine_Dashed` | `bool` | `false` | Dash the chromaticity line |

> `ColorCurve` and `ToneCurve` default to `AddPolyline` on purpose: the
> shader-backed stroke modes silently draw nothing if the stroke shader fails or
> is rebuilding. See [compatibility.md](../compatibility.md#5-drawthickline--runtime-fallback-details).

> Removed in 2.0.0: `StyleColor_Value`, `StyleVar_ColorWheel_DiscSectors`,
> `StyleVar_ColorWheel_DiscRings`, `StyleVar_ColorWarper_DiscSectors`,
> `StyleVar_ColorWarper_DiscRings`, `StyleVar_ColorPicker_PlaneResolution` and
> `StyleVar_ColorPicker_SliderResolution`. The five resolution counts live on in
> the table above as plain `int` fields.

---

## Functions

### `GetStyle`
```cpp
ImWidgetsStyle& GetStyle();
```
Returns a reference to the global style. Modify it directly to change defaults.

---

### `GetColorU32`
```cpp
ImU32 GetColorU32( ImWidgetsCol colorIndex );
```
Packed form of `GetStyle().Colors[idx]`, honouring any active `PushStyleColor`.
*(New in 2.0.0.)*

---

### `GetStyleColorName`
```cpp
char const* GetStyleColorName( ImWidgetsCol colorIndex );
```
Human-readable name for a colour slot (used by the style editor and the `.ini`
serializer).

---

### `ShowStyleEditor`
```cpp
void ShowStyleEditor( ImWidgetsStyle* ref = NULL );
```
Opens an interactive style editor window. `ref` = reference style for the
"Revert" button; `NULL` uses the default-constructed style.

---

### `SaveStyleToFile` / `LoadStyleFromFile`
```cpp
void SaveStyleToFile( char const* filename, ImWidgetsStyle const* style = NULL );
bool LoadStyleFromFile( char const* filename, ImWidgetsStyle* style = NULL );
```
Serialize a style to / from an `.ini`-style text file. `NULL` means the global
style. `LoadStyleFromFile` returns `false` if the file cannot be read; unknown
keys are skipped, so a file written by a newer build still loads.

---

### `PushStyleColor` / `PopStyleColor`
```cpp
void PushStyleColor( ImWidgetsCol colorIndex, ImVec4 const& color );
void PopStyleColor( int count = 1 );
```
Temporarily override a color. Must be balanced with `PopStyleColor`.

---

### `PushStyleVar` / `PopStyleVar`
```cpp
void PushStyleVar( ImWidgetsStyleVar varIndex, float value );
void PushStyleVar( ImWidgetsStyleVar varIndex, ImVec2 const& value );
void PushStyleVar( ImWidgetsStyleVar varIndex, ImVec4 const& value );
void PopStyleVar( int count = 1 );
```
Temporarily override a style var. Must be balanced with `PopStyleVar`.

> Every var in the table above is a `float`. The `ImVec2` / `ImVec4` overloads
> exist for forward compatibility and currently assert: there is no `ImVec2` or
> `ImVec4` style var to target.

---

### `ScaleAllSizes`
```cpp
void ImWidgetsStyle::ScaleAllSizes( float scale_factor );
```
Multiply all logical-pixel vars by `scale_factor`. Call once after a DPI change.
