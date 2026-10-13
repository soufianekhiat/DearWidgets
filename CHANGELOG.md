# Changelog

All notable changes to DearWidgets.

The version is exposed by the public header:

```cpp
#define IMWIDGETS_VERSION       "2.0.0"
#define IMWIDGETS_VERSION_NUM   20000     // major*10000 + minor*100 + patch
```

---

## 2.0.0 -- 2026-09-18

First tagged release. 2.0.0 freezes the public API: names, argument order, enum
prefixes and type scope are now considered stable. Everything below is relative
to the `refacto` branch state of 2026-04-19.

---

### Breaking changes

Read this section before upgrading. Most breaks are mechanical renames that a
find-and-replace will handle; the argument-order change (#5), the replaced
widgets (#11-#13) and the removed declarations (#16-#17, #19, #22, #23) need a
closer look.

#### 1. Compatibility shim removed

The `#if !__cpp_if_constexpr / #define constexpr` block at the top of
`dear_widgets.h` is gone. It redefined a language keyword for every translation
unit that included the header, and the header never used `if constexpr`.

#### 2. `Slider2DInt` signature fixed

```cpp
// before
bool Slider2DInt( char const* pLabel, int* pValueX, void* pValueY, ... );
// now
bool Slider2DInt( char const* pLabel, int* pValueX, int* pValueY,
                  int v_minX, int v_maxX, int v_minY, int v_maxY );
```

The Y pointer was typed `void*` while X was `int*`.

#### 3. Colour blending consolidated

`ImColorBlendLinear`, `ImColorBlendHSL`, `ImColorBlendHSLa`, `ImColorBlendHWB`,
`ImColorBlendLCH`, `ImColorBlendLab`, `ImColorBlendOklab` and `ImColorBlendOkLCH`
were declared but never defined -- calling any of them was a link error. All
eight are removed. `ImColorBlendsRGB` stays, and one general form replaces them:

```cpp
ImU32 ColorBlend( ImU32 col0, ImU32 col1, float t, ImWidgetsGradientInterp interp );
```

It interpolates in the requested space and takes the shortest hue arc for
cylindrical spaces.

#### 4. `RenderFrameWithHole` removed

Declared, never defined.

#### 5. HSL / HSY / HSP / HSPLog conversions now take outputs FIRST

Affected: `ColorConvertRGBtoHSL`, `ColorConvertHSLtoRGB`, `ColorConvertRGBtoHSY`,
`ColorConvertHSYtoRGB`, `ColorConvertRGBtoHSP`, `ColorConvertHSPtoRGB`,
`ColorConvertRGBtoHSPLog`, `ColorConvertHSPLogtoRGB`.

```cpp
// before
void ColorConvertRGBtoHSL( float r, float g, float b, float& out_h, float& out_s, float& out_l );
// now
void ColorConvertRGBtoHSL( float& out_h, float& out_s, float& out_l, float r, float g, float b );
```

They now match every other `ColorConvert*` in the library and can be used as
`pfColorModelToRGB` / `pfColorModelFromRGB` callbacks, which they could not
before.

> **Note.** DearWidgets' convention is **outputs first** ("right-to-left, like
> `=`"). This is the *opposite* of `ImGui::ColorConvertRGBtoHSV`. The header now
> says so explicitly. Several `ColorConvert*` parameter names were also corrected
> (they claimed `r,g,b` where the values were `L,C,H` or linear RGB).

#### 6. Style enums renamed and prefixed

~930 occurrences updated.

| Before | After |
|---|---|
| `enum ImWidgetsStyleColor` | `enum ImWidgetsCol_` + `typedef int ImWidgetsCol;` |
| `enum ImWidgetsStyleVar` | `enum ImWidgetsStyleVar_` + `typedef int ImWidgetsStyleVar;` |
| `StyleColor_Xxx` | `ImWidgetsCol_Xxx` |
| `StyleVar_Xxx` | `ImWidgetsStyleVar_Xxx` |
| `..._Count` | `..._COUNT` |

New colour `ImWidgetsCol_SliderN_Cursor` (was a hard-coded red), and a new
helper `ImU32 GetColorU32( ImWidgetsCol colorIndex )` returning the packed form
of `GetStyle().Colors[idx]` honouring `PushStyleColor`.

Also removed from the **style var enum**: `StyleVar_ColorWheel_DiscSectors`,
`StyleVar_ColorWheel_DiscRings`, `StyleVar_ColorWarper_DiscSectors`,
`StyleVar_ColorWarper_DiscRings`, `StyleVar_ColorPicker_PlaneResolution`,
`StyleVar_ColorPicker_SliderResolution` and `StyleColor_Value`. The resolution
counts are `int`, not `float`, so they were never pushable; they remain as
direct fields on `ImWidgetsStyle` (`GetStyle().ColorWheel_DiscSectors`, ...).
See [docs/api/style.md](docs/api/style.md).

#### 7. Delta-E formula enum unified

`ImDeltaEFormulaShader_`, which numbered the formulas differently from the public
`ImDeltaEFormula_`, is gone. `ImDeltaECompareState::Formula` is now typed
`ImDeltaEFormula`. The GPU path remaps to the shader ordinal internally.

#### 8. Arc slider overloads renamed

A forgotten `sweepAngle` used to silently bind a `float` to
`bool fill_up_to_cursor`. The angle-taking overloads are now distinct functions:

| Before | After |
|---|---|
| `SliderGradientRing{Scalar,Float,Int}` with explicit `startAngle`/`sweepAngle` | `SliderGradientArc{Scalar,Float,Int}` |
| `SliderGradientRingRange{Scalar,Float,Int}` | `SliderGradientArcRange{Scalar,Float,Int}` |

The full-circle `SliderGradientRing{Scalar,Float,Int}` (no angle arguments) is
unchanged.

#### 9. `g_SlugDebugShader` global replaced

```cpp
void SetSlugDebugShader( bool enabled );
bool GetSlugDebugShader();
```

The public mutable global is gone; this matches the existing
`SetDashedLinesUseGPU` pattern.

#### 10. All public types are now at global scope

~100 struct / enum / typedef declarations that used to live inside
`namespace ImWidgets` were moved out, so every DearWidgets type is spelled the
same way (like `ImVec2` / `ImDrawList` in ImGui). Only *functions* remain in
`namespace ImWidgets`.

Code that said `ImWidgets::ImColorSliceData`, `ImWidgets::ImWidgetsColorScalar`,
`ImWidgets::ImDetectionBox`, `ImWidgets::ImWidgetsCropState`,
`ImWidgets::ImTimelineBandKind`, ... must drop the `ImWidgets::` prefix. A
`// [SECTION] Public types` banner in the header marks the block.

#### 11. `PrimariesWheel` removed

Superseded by `HDRWheel`. Call `HDRWheel` with `rightValue = leftValue = NULL`
(the defaults) for the old arc-less layout.

#### 12. `ImageTransformGizmo` / `ImTransformImage` replaced by `TransformGizmo`

```cpp
bool TransformGizmo( char const* label,
                     ImTransformData* transforms, ImVec2* sizes, int count,
                     int* selectedIndex,
                     ImTransformGizmoCallbacks const* callbacks = nullptr,
                     ImTransformGizmoFlags flags = ImTransformGizmoFlags_None,
                     ImVec2 canvasSize = ImVec2( 0, 0 ) );
```

The gizmo no longer owns textures. Transform state and per-item size are passed
as parallel arrays, and drawing is delegated to
`ImTransformGizmoCallbacks` / `ImTransformGizmoDrawParams`, so the gizmo can
drive anything -- not just `ImTextureID`s. The `ImTransformImage` struct is gone;
`ImTransformData` stays.

#### 13. `ImageViewer` gained three parameters

```cpp
bool ImageViewer( char const* label, ImTextureID image, ImVec2 imageSize,
                  ImImageViewerState& state, ImVec2 widgetSize = ImVec2( 0, 0 ),
                  ImPlatform_ShaderProgram shaderProgram = nullptr,
                  ImImageViewerOverlayCallback overlay_callback = NULL,
                  void* overlay_user_data = NULL );
```

All three default, so existing calls compile unchanged. `shaderProgram` wraps
the image draw in a custom pixel shader; `overlay_callback` is invoked once per
presentation so overlays also appear inside the built-in expand-to-window modal.

#### 14. HarfBuzz backend removed

Complex-script shaping is now provided solely by `kb_text_shape`
(`src/api/kb_text_shape.h`), which is header-only and vendored. The HarfBuzz
submodule, its build configuration and the `#if` branches selecting it are gone.

#### 15. `ImParadeBitDepth` / `ImParadeLayout` renamed

They describe any pixel buffer, not just the parade scope:

| Before | After |
|---|---|
| `ImParadeBitDepth` | `ImPixelBitDepth` |
| `ImParadeLayout` | `ImPixelLayout` |

Every `Accumulate(...)` method (`ImParadeScopeData`, `ImVectorScopeData`,
`ImHistogramData`, `ImCIEChromaticityData`, `ImColorWarperOverlay`) takes the new
names. `ImParadeMode` and `ImParadeScale` are unchanged.

#### 16. Dead declarations removed

No definition and/or no caller: `ImWidgetsVertexLine`, `ImWidgetsShapeLine`,
`ShapeLineSetBound`, `ImWidgetsLengthUnit` + `ImWidgetsLengthUnit_`,
`ImWidgetsChromaticPlot` + `ImWidgetsChromaticPlot_`, `ImWidgetsPointer`,
`ImWidgetsFeatures_COUNT` (a bogus sentinel inside a bit-flag enum).

#### 17. Other removals

- **`DrawTextGradient`** -- superseded by `DrawLinearGradientText` and the GPU
  forms `DrawLinearGradientTextGPU` / `DrawRadialGradientTextGPU` /
  `DrawDiamondGradientTextGPU`.
- **Box-select family** -- `ImBoxSelectState`, `BoxSelectUpdate`, `BoxSelectDraw`,
  `BoxSelectRect`. Zero callers; superseded by `ImWidgetsSelectionState` with
  `BeginSelection` / `TestSelectionPoints` / `TestSelectionRects`.
- **`VolumeGenerateTestField`** -- `VolumeGenerateField( ..., ImVolumeField mode )`
  covers it.
- **Unfinished third curve model** -- `ImCurveKey`, `ImCurveData`, `ImCurveInterp`
  + `ImCurveInterp_`, `ImCurveEval`, `ImCurveAdd`, `ImCurveRemove`, `ImCurveSort`.
  It was never wired up and 2.0 must not freeze it. The shipping curve types are
  `ImCurveEditorData` (CurveEditor), `ImColorCurveData` (ColorCurve) and
  `ImToneCurveData` (ToneCurve).

#### 18. Physically-based and artistic colour pickers now honour `size`

`size.x` overrides the item width, `size.y` overrides the 2D plane height.
Previously `size` was accepted and ignored (`IM_UNUSED(size)` in 27 functions).
`ColorPickerSRGB` / `HSV` / `OkLab` / `OkLCH` / `CIELab` / `XYZ` honour it too.

The header now states, once for the whole family, that `*color` is **output
only** for the physical pickers -- the forward physical model is not invertible,
so the widget cannot seed itself from an arbitrary colour.

#### 19. The 15 per-space shape gradient fills collapsed into 3

```cpp
void ShapeLinearGradient ( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end,
                           ImU32 col0, ImU32 col1,
                           ImWidgetsGradientInterp interp = ImWidgetsGradientInterp_sRGB );
void ShapeRadialGradient ( ... same ... );
void ShapeDiamondGradient( ... same ... );
```

Removed: `ShapeSRGB{Linear,Radial,Diamond}Gradient`,
`ShapeLinearSRGB{...}`, `ShapeOkLab{...}`, `ShapeOkLch{...}`, `ShapeHSV{...}`.
`ImWidgetsGradientInterp_` already enumerated the five spaces, so the per-space
spellings were redundant.

The function-pointer forms `Shape{Linear,Radial,Diamond}GradientGeneric` are no
longer public either -- they moved to `dear_widgets_internal.h`, because only the
Slug renderer needs to pass its own conversion pair.

Migration: `ShapeOkLabRadialGradient(s, a, b, c0, c1)` becomes
`ShapeRadialGradient(s, a, b, c0, c1, ImWidgetsGradientInterp_OkLab)`.

#### 20. CPU gradient text takes the space enum, not function pointers

```cpp
// before
void DrawLinearGradientText( ..., ImU32 col0, ImU32 col1,
                             pfSpace2sRGB space2sRGB = nullptr, pfsRGB2Space sRGB2Space = nullptr,
                             char const* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
// now
void DrawLinearGradientText( ..., ImU32 col0, ImU32 col1,
                             ImWidgetsGradientInterp interp = ImWidgetsGradientInterp_sRGB,
                             char const* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
```

Same change for `DrawRadialGradientText` and `DrawDiamondGradientText`.

#### 21. GPU gradient text is typed

`DrawLinearGradientTextGPU` / `DrawRadialGradientTextGPU` /
`DrawDiamondGradientTextGPU` took an untyped `int colorSpace = 0` whose magic
numbers happened to match `ImWidgetsGradientInterp_`. The parameter is now
`ImWidgetsGradientInterp interp = ImWidgetsGradientInterp_sRGB`.

#### 22. `DrawShapeEx` removed from the public header

It was the shared implementation behind `DrawShape` / `DrawImageShape` and had a
signature identical to `DrawImageShape`. Call `DrawImageShape` instead; the
symbol moved to `dear_widgets_internal.h`.

#### 23. The 17 `ColorScalar<Space>` functions are no longer exported

`ColorScalarAdobeRGB`, `ColorScalarAppleRGB`, `ColorScalarBest`,
`ColorScalarBeta`, `ColorScalarBruce`, `ColorScalarCIERGB`,
`ColorScalarColorMatch`, `ColorScalarDonRGB4`, `ColorScalarECI`,
`ColorScalarEktaSpacePS5`, `ColorScalarNTSC`, `ColorScalarPALSECAM`,
`ColorScalarProPhoto`, `ColorScalarSMPTEC`, `ColorScalarsRGB`,
`ColorScalarWideGamutRGB`, `ColorScalarRec2020`. They differed only by the space
they pinned.

Use `ColorScalarCallback( ImWidgetsColorSpace space )`, which returns the same
`ImWidgetsColor1DCallback` for any space and is unchanged.

#### 24. `DrawLumianceBand` spelling fixed

```cpp
// before
void DrawLumianceBand( ImDrawList*, ImVec2 const vpos, ImVec2 const size,
                       int division, ImVec4 const& color, float gamma );
// now
void DrawLuminanceBand( ImDrawList*, ImVec2 const vpos, ImVec2 const size,
                        int division, ImVec4 const& color, float gamma );
```

There is **no compatibility alias** -- rename the call site.

#### 25. `ImWidgetsJoin_Mitter` spelling fixed

The enumerator is now `ImWidgetsJoin_Miter`, matching the `miter_limit`
parameter it sits next to. It is the default `join` argument of `DrawPolylineAA`
and both `DrawDashedPolylineAA` overloads, so only explicit uses need updating.
The other two enumerators (`ImWidgetsJoin_Round`, `ImWidgetsJoin_Bevel`) are
unchanged.

#### 26. `PushStyleVar( ImVec2 )` / `PushStyleVar( ImVec4 )` removed

No `ImWidgetsStyleVar_` is a vector, so both overloads could only assert. Every
style var is a `float`; `int` / `bool` fields are set directly on `GetStyle()`.

#### 27. `size` is in logical pixels for three more widgets

`NetworkGraph`, `GradientMeshEditor` and `ToonRampEditor` took `size` in screen
pixels, unlike every other widget. They now take logical pixels (DPI applied
internally), and `ImNetworkGraphData::Zoom` is a logical zoom. Callers that
wrapped the size in `ImPlatform_LpToPx` should drop the wrap; `0` still means
"fill the available width".

#### 28. "Tesselate" spelling fixed, `RemoveFeature` pluralised

| before | now |
|---|---|
| `TesselateText` / `TesselateTextPerGlyph` | `TessellateText` / `TessellateTextPerGlyph` |
| `TesselateLaTeX` | `TessellateLaTeX` |
| `DrawTesselateDebug` | `DrawTessellateDebug` |
| `ShapeTesselationUniform` | `ShapeTessellationUniform` |
| `RemoveFeature` | `RemoveFeatures` (matches `SetFeatures` / `AddFeatures`) |

There are no compatibility aliases -- rename the call sites.

---

### Added

#### Edit reporting on composite widgets

`IsItemEdited()`, `IsItemActive()` and `IsItemDeactivatedAfterEdit()` now work
after the 29 physical colour pickers, `ColorSlice`, `SpectrumEditor`,
`ChromaWarp`, `GridWarp`, `UnitField`, `PaintCanvas`, `TransformGizmo`,
`NotchedDial`, `AngleDial`, `SizeControl2D/3D`, `GradientMeshEditor`,
`ToonRampEditor`, `VectorDrawingTool` and `CropRect`. These widgets are wrapped
in a group (as ImGui's own `ColorPicker4` is); the ones with a single main item
still report its ID, so `BeginPopupContextItem()` right after them keeps working.

#### Caret, hit-testing and selection

`CalcTextGraphemes` exposes the graphemes of a shaped line with their visual
boxes; `TextCaretX` / `TextHitTest` (with caret affinity at bidi run
boundaries), `TextSelectionRanges` and `TextPrev/NextGrapheme` are the building
blocks for editing shaped, mixed-direction text.

#### Text on any backend

`SetTextRenderer` selects how `DrawText` renders: the Slug GPU shader, a new CPU
vector renderer (the same shaped, bidi-ordered glyph outlines filled with plain
`ImDrawList` triangles, cached per string), or the ImGui atlas. `Auto` uses the
GPU path when its shader is available and the CPU vector renderer otherwise --
including on OpenGL3, whose custom-shader texture path still renders
incorrectly. `DrawTextOnPath`, `DrawTextInsideRect` and `DrawTextInsideConvex`
now shape their text too (they used unshaped ImGui glyphs; `DrawTextOnPath`
did not even decode UTF-8).

#### Vector paths (SVG / Canvas2D) on a plain ImDrawList

New `ImWidgetsPath` (moveTo / lineTo / quadratic / cubic / SVG elliptical arc /
Canvas2D arc / rect / round rect / ellipse, 2x3 transforms) with an SVG path
data parser (`AddSvgPathData`), and `FillPolygons` / `PathFill` / `PathStroke`.
Fills are anti-aliased, honour the SVG fill rules (nonzero / evenodd, holes,
self-intersections) and never double-cover a pixel. Everything is ImDrawList
triangles, so it works on stock Dear ImGui. `ImWidgetsPaint` adds CSS / SVG gradient fills (linear,
radial, conic; stops, hard stops, pad / repeat / reflect, gradient transform,
sRGB / linear / OkLab / OkLCH / HSV interpolation). `PathStroke` also takes a paint
(gradient strokes, no self-overlap darkening), and `DrawVariableWidthStroke`
draws pressure / taper strokes. Strokes now fall back to filling
their exact outline on the CPU when the stroke shader is unavailable, instead
of degrading to `AddPolyline` (no caps / joins) or drawing nothing.

#### Bidirectional text

`DrawText`, the gradient / image text variants and `CalcTextSize` now run the
Unicode bidirectional algorithm (UAX #9 implicit levels, rules P2-L2) before
shaping: each direction run is shaped with kb_text_shape on its own and the runs
are reordered visually. Mixed Arabic + Latin + numbers (European and
Arabic-Indic) now lay out as in a browser; previously runs were placed in
logical order. New `ImWidgetsStyle::TextDirection` (`Auto` / `LTR` / `RTL`)
sets the paragraph direction. `CalcTextSize` measures the shaped glyphs, so it
matches what `DrawText` draws.

New `DrawTextBlock` / `CalcTextBlockSize` lay out multi-line paragraphs:
UAX #14 line breaking, `
` paragraphs with their own direction, per-line bidi,
`ImWidgetsTextLayout` (wrap width, start / end / left / right / center
alignment, line height).

#### Interaction additions

- `BeginSelection`: Escape cancels a drag (`s.Cancelled`); Shift / Alt at drag
  start set `s.Op` to `ImWidgetsSelectionOp_Add` / `_Subtract`.
- `SliderRingFloatEx`: `ImWidgetsSliderFlags_Stepped` snaps to the format's
  precision; `ImWidgetsSliderFlags_ProportionalDrag` joins the enclosing
  `PushDragGroup` with an automatic slot.
- `GradientMeshEditor`: nodes can be dragged (corners pinned, edge nodes slide
  along their edge) and the fill follows the moved nodes; click a node without
  dragging to edit its colour.
- `RangeSliderVertical*` shows its values (inside the bar when it fits,
  otherwise as a tooltip).
- Scopes show a hover readout: `ParadeScope` level and trace density,
  `VectorScope` Cb/Cr, hue and saturation, `Histogram` value and per-channel
  counts, `CIEChromaticity` xy, gamut membership and CCT.
- `VectorDrawingTool`: click a curve to select its path, double-click to insert
  a node without changing the curve's shape.
- `TransformGizmo`: Ctrl snaps (10 lp grid, 15 degrees, 0.1 scale), Shift locks
  a translation to its dominant axis.

#### Version and licence banner

`IMWIDGETS_VERSION` / `IMWIDGETS_VERSION_NUM` plus a short licence banner at the
top of `dear_widgets.h`.

#### Physically-based and artistic colour pickers

29 domain-specific pickers, each a forward physical or artistic model driving a
2D plane plus a 1D slider:

`ColorPickerSkin`, `ColorPickerHair`, `ColorPickerLeaf`, `ColorPickerBlackbody`,
`ColorPickerPigment`, `ColorPickerGem`, `ColorPickerWater`, `ColorPickerIris`,
`ColorPickerFlame`, `ColorPickerBruise`, `ColorPickerNebula`,
`ColorPickerMaillard`, `ColorPickerPatina`, `ColorPickerSubsurface`,
`ColorPickerDischarge`, `ColorPickerIce`, `ColorPickerOchre`, `ColorPickerSky`,
`ColorPickerStar`, `ColorPickerHemoglobin`, `ColorPickerCloud`,
`ColorPickerStreetlight`, `ColorPickerPaletteHarmony`,
`ColorPickerTrichromaticMixer` (with CMYK mode), `ColorPickerWeatheredMetal`,
`ColorPickerFabricDye`, `ColorPickerMoodPalette`, `ColorPickerToonRamp`,
`ColorPickerHarmonyWheel`.

#### Unified scalar -> colour API

One `float -> ImU32` mapping (`ImWidgetsColor1DCallback`) described by a single
options struct, resolved once to function pointers instead of being re-dispatched
per sample:

```cpp
void   ColorScalarResolve( ImWidgetsColorScalarResolved* out, ImWidgetsColorScalar const& opt,
                           ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
ImVec4 ColorFromScalarResolved( float t, ImWidgetsColorScalarResolved const& resolved );
ImU32  ColorFromScalarResolvedCallback( float t, void* pUserData );
ImVec4 ColorFromScalar( float t, ImWidgetsColorScalar const& opt );
ImU32  ColorFromScalarU32( float t, ImWidgetsColorScalar const& opt );
ImU32  ColorFromScalarCallback( float t, void* pUserData );
void   ColorScalarPreset( ImWidgetsColorScalar* opt, ImWidgetsColorRamp ramp );
ImWidgetsColor1DCallback ColorScalarCallback( ImWidgetsColorSpace space );
```

plus colour-space-aware
overloads of `ShapeFillProceduralColor1D`, `DrawShapeProceduralColor*`,
`DrawProceduralColor1D*`, `DrawProceduralColorArcBilinear`,
`DrawProceduralColorSplineBilinear` and `DrawColorRing` that take the options
directly.

#### Colour-space introspection

`ColorSpaceName`, `ColorSpaceGetPrimaries`, `ColorSpaceGetTransfer`,
`ColorSpaceEncode`, `ColorSpaceDecode`, `ColorConvertRGBSpacetoXYZ`,
`ColorConvertXYZtoRGBSpace`, `ColorAdaptSpaceToD65`, `ColorAdaptD65ToSpace`,
`ChromaticityInsideGamut`, `ColorInsideSpaceGamut`, `ColorModelToDisplay`,
`ColorDisplayToModel`, `ColorModelIsWorkingSpace`, `ColorModelGetFunctions`,
`ColorModelAxisIndex`, `ColorModelCompRange`, `ColorModelDefaultTriple`,
`ColorHarmonyOffsets`, and the name helpers `ColorModelName`,
`ColorModelCompName`, `ColorAxisName`, `ColorSourceName`, `ColorHarmonyName`,
`ColorWrapName`, `ColorEaseName`, `ColorGamutName`, `ColorRampName`.

#### Grading widgets

- `ColorBars` -- Resolve-style Y/R/G/B lift-gamma-gain bar group.
- `ColorSlice` -- multi-vector hue/sat slice grader (`ImColorSliceData`).
- `ChromaWarp` -- chromaticity-plane warp grid (`ImChromaWarpData`).
- `SpectrumEditor` + `SpectrumToXYZ` + `WavelengthToRGB` -- spectral power
  distribution editor and its CIE integration.
- `GridWarp` -- free-form deformation lattice over an optional background image.
- `GradingGradient( ImWidgetsGradingGradient )` -- built-in grading gradients.
- `DragFloatColorUnderline` / `DragFloatGradientUnderline` -- drag fields with a
  colour or gradient underline.

#### Astronomy / ephemerides drawing

`DrawMoonEphemeris`, `DrawEarthSunEphemeris`, `DrawStarChart` (with
`ImWidgetsSkyCulture`, `GetSkyCultureName`, `GetSkyCultureCount`) and
`DrawSunPath` (polar / cartesian analemma).

#### Network graph and timeline widgets

- `NetworkGraph` / `NetworkGraphLayout` (`dear_widgets_network.h/.cpp`) --
  read-only hierarchically grouped DAG illustration with layered layout,
  zoom-to-cursor, LOD and drill-down.
- `ImTimeline` (`dear_widgets_timeline.h/.cpp`) -- temporal timeline with
  playhead, authored keys, events and `[begin, end)` bands. Band kinds:
  `CacheCoverage`, `Invalid`, `StaleBranch`, `Checkpoint` and (added later in the
  cycle) `CommittedHistory`.

#### Canvas / authoring toolkit

`BeginCanvas` / `EndCanvas` / `CanvasWorldToScreen` / `CanvasScreenToWorld` /
`CanvasViewportWorld` / `DrawCanvasMinimap`, `BeginSelection` /
`TestSelectionPoints` / `TestSelectionRects`, `ComputeSnapAndDraw`,
`Eyedropper` / `EyedropperSampleBitmap`, `GradientDrop`, `CropRect`,
`SizeControl2D` / `SizeControl3D`, `DrawOnionSkin` / `DrawOnionSkinAuto`,
`DrawArrow` / `DrawArrowHead` / `DrawAxisArrows`, `DrawDimensionLine`,
`DrawCrosshair` / `DrawCrosshairInRect`, `DrawDropShadowRect` /
`DrawInnerGlowRect`, `DrawRuler`, `DrawGridOverlay`, `GradientMeshInit` /
`GradientMeshEditor`, `ToonRampEditor` / `ToonRampSample`, `MatrixEditor` /
`MatrixEditorDouble`, plus the interaction helpers `KeyboardNudgeXY`,
`KeyboardNudge1D`, `AxisConstrain`, `PushDeltaOverlay` / `RenderDeltaOverlay`,
`PushDragGroup` / `PopDragGroup` / `GetDragGroupDelta` / `SetDragGroupActive`,
and `BeginPrecisionPopup` / `EndPrecisionPopup` / `PrecisionFloat` /
`PrecisionInt`.

#### Image inspection and look development

`ImageInspector` / `ImageInspectorSupported` / `ImageInspectorReleaseState`
(`dear_widgets_image_inspector.cpp`), `LookDevCompare`, `LookDevInspector`,
`ColorDifferenceImageViewer`, `ColorDifferenceVisualizer`, the Delta-E helpers
`ColorDeltaE76` / `ColorDeltaE94` / `ColorDeltaE2000` / `ColorDeltaEOK` /
`DeltaEFormulaName` / `DeltaEPerceptualLabel`, the 3D LUT family
`LoadCubeLUT` / `SaveCubeLUT` / `SampleLUT3D` / `InitIdentityLUT3D` /
`ColorLUT3DViewer`, and the volume viewers `VolumeViewer` / `VolumeSliceViewer` /
`VolumeGenerateField`.

#### Image overlays

Composed on top of `ImageViewer` in UV space: `DetectionOverlay`,
`KeypointOverlay`, `TextLabelOverlay` and the interactive `AnnotationEditor`
(returns edit intents via `ImAnnotationResult`, never mutates its inputs).

#### Extended drawing primitives

`DrawHatchFill`, `DrawStippleFill`, `DrawCoonsPatch` / `DrawCoonsPatchGradient` /
`DrawCoonsPatchWireframe`, `DrawGregoryPatch` / `DrawGregoryPatchGradient`,
`OffsetPolyline` / `DrawOffsetOutline` / `DrawOffsetFilled`, `DrawIsoContour` /
`DrawIsoContourTiered` / `DrawIsoFilled`, `DrawConicGradient` /
`DrawConicGradientRect`, `GenShapeSuperellipse`, `DrawChromaticityGamuts`, and
the tessellated-shape generators `GenShapeConcavePoly`, `GenShapeVerticalBand`,
`GenShapeHorizontalBand`, `GenShapeRectGrid`, `GenShapeDiscRings`,
`GenShapeAnnulus`, `GenShapeAnnulusRings`, `GenShapeAnnulusArc`,
`GenShapeTriangleSubdiv` with matching `DrawShapeProceduralColor*` helpers.

#### Slider variants

`SliderNVertical{Scalar,Float,Int}`, `Slider2DRange{Scalar,Float,Int}`,
`Slider2DDisc{Scalar,Float,Int}`, `SliderGradientVertical{Scalar,Float,Int}`,
`RangeSlider{Scalar,Float,Int}`, `RangeSliderVertical{Scalar,Float,Int}`,
`SegmentedControl{Scalar,Int}`, `SliderRingFloatEx`.

#### Typography helpers

`DrawTextOnPath`, `DrawFontWaterfall`, `VarFontAxisSliders`, `KerningPairEditor`,
`DrawTextInsideRect`, `DrawTextInsideConvex`, `CalcShapedTextWidth`,
`FontInspector`.

#### Dials, fields and misc widgets

`NotchedDial`, `AngleDial` (continuous), `EquationInput`, `UnitField` precision
popups.

#### Background effects

`SetCurrentWindowBlurBackground` + `BlurBackgroundNewFrame` -- blur / effect pass
behind the current window (`ImWidgetsBgEffect`). The blur uber-shader was split
per effect through a `BG_EFFECT` define so only the requested variant compiles.

---

### Fixed

- **Dashed stroke leaked every frame.** `DrawStrokedDashedBezierPath` and
  `DrawStrokedDashedPolyline` built an `ImVector<ImVector<ImVec2>>`; `~ImVector`
  frees only the outer buffer, so every dash sub-path leaked on every call.
- **Slug atlas teardown leaked.** `SlugDestroyAtlas` never freed `bandHScratch`,
  `bandVScratch` or `shapeCacheTable[i].glyphs`.
- **`ImVectorDrawingData` had no cleanup.** It now has `Clear()`, a destructor,
  and deep copy / assignment, so paths no longer leak their node buffers.
- **Precision popups never clamped.** `ClampScalar` computed the clamped value,
  returned it and left the target untouched -- and all seven call sites discarded
  the return. It now clamps in place and returns whether the value changed. This
  affected the Slider2D, Slider2DRange and PrecisionFloat / PrecisionInt typed
  input popups.
- **`SliderNVerticalScalar` bailed out of its band loop** with `return false` on a
  clipped band, discarding changes from earlier bands and desyncing ImGui's id
  tracking. It now `continue`s, matching the horizontal version.
- **`SliderNScalar` drew every cursor twice**, the first pass hidden under the
  frame render.
- **Colour pickers reported "changed" on every frame the mouse was held**, even
  without movement. They now report a change only when the colour they drive
  actually moved.
- **Slug `ImageFill` alpha.** The `slug_fill` shader mishandled alpha, so image
  fills over glyph outlines came out wrong.
- **Glyph holes in CDT tessellation** needed a winding guard in the hole
  classification pass.
- **Chromaticity lines** are smoothed (`ChromaticityLine_SmoothIterations`)
  instead of showing raw sample noise.
- **Slider2D range** clamping and int handling cleaned up.
- **`ContainerAtlas` -> `OwnerAtlas`**, following the upstream ImGui rename.
- **Demo font download link fixed** -- the Noto COLRv1 emoji font moved to
  `2D/fonts/` upstream.
- **Scopes orphaned their expand window on empty data.** `ParadeScope`,
  `VectorScope` and `Histogram` returned before drawing the expand button.
- **Double DPI scaling** in the `PaintCanvas` and `VectorDrawingTool` expand
  windows; `NetworkGraph` ignored DPI entirely.
- **Range sliders on int types reported a change every frame** while the mouse
  sat between two integer steps (the test ran before quantisation).
- **`VectorDrawingTool`**: Shift+click on a node now multi-selects (Shift used
  to always pan); deselecting a node no longer makes the rest run away on drag.
- **`CropRect`** picks the nearest grip instead of the first one in range.
- **`ColorWheel` "Copy Hex"** clamps HDR / negative values.
- **Image loupe** (`ImageViewer`, `ImageInspector`) owns its right-button drag.
- **`TransformGizmo` translation was in screen pixels** although documented as
  logical pixels; it is now converted, so objects keep their place across DPI.
- **`CalcTextSize` without the shader tier returned width 0** for every
  string; it now measures with ImGui. `DrawText` falls back to `AddText` when
  the Slug program failed to build instead of drawing nothing, and
  `CalcShapedTextWidth` no longer applies the DPI scale twice.
- **`DrawText` without the shader tier drew one ascent too low**: its `pos.y`
  is the baseline, but the `AddText` fallback treated it as the top.
- **`PushDragGroup`** delivers each delta exactly once (the release frame used
  to re-apply the last delta to slots after the initiator).

---

### Removed

Summarised here; see **Breaking changes** above for the details and migration.

| Symbol | Replacement |
|---|---|
| `PrimariesWheel` | `HDRWheel` with `rightValue = leftValue = NULL` |
| `ImageTransformGizmo`, `ImTransformImage` | `TransformGizmo` + `ImTransformGizmoCallbacks` / `ImTransformGizmoDrawParams` |
| `DrawTextGradient` | `DrawLinearGradientText`, `Draw*GradientTextGPU` |
| `ImColorBlend{Linear,HSL,HSLa,HWB,LCH,Lab,Oklab,OkLCH}` | `ColorBlend(col0, col1, t, interp)` |
| `RenderFrameWithHole` | -- (never existed as a definition) |
| `g_SlugDebugShader` | `SetSlugDebugShader` / `GetSlugDebugShader` |
| `ImDeltaEFormulaShader_` | `ImDeltaEFormula_` |
| `ImBoxSelectState`, `BoxSelectUpdate/Draw/Rect` | `ImWidgetsSelectionState`, `BeginSelection`, `TestSelection*` |
| `VolumeGenerateTestField` | `VolumeGenerateField(..., ImVolumeField mode)` |
| `ImCurveKey/Data/Interp`, `ImCurveEval/Add/Remove/Sort` | `ImCurveEditorData`, `ImColorCurveData`, `ImToneCurveData` |
| `ImWidgetsVertexLine`, `ImWidgetsShapeLine`, `ShapeLineSetBound` | -- |
| `ImWidgetsLengthUnit`, `ImWidgetsChromaticPlot`, `ImWidgetsPointer` | -- |
| `ImWidgetsFeatures_COUNT` | -- (bit-flag enum, no sentinel) |
| `StyleColor_Value` | -- |
| `StyleVar_ColorWheel_DiscSectors/DiscRings` | `GetStyle().ColorWheel_DiscSectors/DiscRings` (int fields) |
| `StyleVar_ColorWarper_DiscSectors/DiscRings` | `GetStyle().ColorWarper_DiscSectors/DiscRings` |
| `StyleVar_ColorPicker_PlaneResolution/SliderResolution` | `GetStyle().ColorPicker_PlaneResolution/SliderResolution` |
| HarfBuzz shaping backend | `kb_text_shape` |
| `ShapeSRGB/LinearSRGB/OkLab/OkLch/HSV{Linear,Radial,Diamond}Gradient` (15) | `ShapeLinearGradient` / `ShapeRadialGradient` / `ShapeDiamondGradient` with `ImWidgetsGradientInterp` |
| `Shape{Linear,Radial,Diamond}GradientGeneric` | internal (`dear_widgets_internal.h`) |
| `DrawShapeEx` | `DrawImageShape` (identical signature) |
| `ColorScalar<Space>` (17) | `ColorScalarCallback( ImWidgetsColorSpace )` |
| `pfSpace2sRGB` / `pfsRGB2Space` parameters on `Draw*GradientText` | `ImWidgetsGradientInterp interp` |
| `int colorSpace` parameter on `Draw*GradientTextGPU` | `ImWidgetsGradientInterp interp` |
| `DrawLumianceBand` | `DrawLuminanceBand` (spelling; no alias) |
| `ImWidgetsJoin_Mitter` | `ImWidgetsJoin_Miter` (spelling) |

---

### Housekeeping

- ~1,000 lines of dead code removed: a 406-line `#if 0` legacy typography block in
  `dear_widgets_slug.cpp`, 13 unreferenced helpers, and several duplicated helpers
  consolidated (one `ImLerpU32`, one sRGB transfer pair, one harmony-offset table,
  one sampled-RGB accumulator, one `DW_PixelToChannels` shared by the parade scope
  and the histogram).
- Internal sprint codenames removed from public headers.
- The library compiles warning-free (C4100 / C4244 / C4566 fixed).
- `PrewarmTessellationCache` parallelised (Release: 30 s -> 0.44 s).
- Shader compile time cut by splitting the blur uber-shader per effect
  (9.5 s -> 0.15 s).

---
---

## Prior to 2.0.0

Content accumulated on the `refacto` branch up to 2026-04-19, kept for reference.
Prototypes below are as they were at the time; where a signature changed for
2.0.0 the **Breaking changes** section above is authoritative.

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
                               bool fill_up_to_cursor = false,
                               ImGuiSliderFlags flags = 0);
bool SliderSplineGradientInt(...);
bool SliderSplineGradientScalar(...);
```

> In 2.0.0 the angle-taking ring overloads are named `SliderGradientArc*`; see
> Breaking change #8.

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
bool SliderSplineGradientRangeScalar(...);
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
| `< 0` | Overlapping envelopes: caps from adjacent dashes interpenetrate by `\|gap_len\|` pixels. Useful for creating seamless joined-cap effects. |

`dash_len` is the total envelope length (cap tip to cap tip). For outward caps (Square, Round, TriangleOut) the body is shorter than the envelope by `2 x (thickness/2)`. For `TriangleIn` the envelope equals the body (the notch is carved inward; no outward extension).

**Migration:** callers that previously passed `gap_len = thickness` to produce a flush result should now pass `gap_len = 0`.

---

### Bug Fixes

- **`TriangleIn` cap in dashed lines.** Both the CPU (Alg 1 / `DrawDashedPolylineAA`) and GPU (Alg 2 / `DrawStrokedDashedBezierPath`) paths now correctly handle `TriangleIn`. The cap extends zero units outward (the notch is carved *into* the body endpoint), so `cap_ext = 0` for TriangleIn everywhere. The GPU shader evaluates an inward-notch SDF at both ends of each envelope.
- **Dash-length clamp parity.** Alg 2 implicitly clamped very short dashes (body = 0 when `dash_len < 2 x cap_ext`) but Alg 1 did not, producing cap-overlap artefacts. Alg 1 now pre-clamps every even-indexed dash to `max(dash_len, 2 x cap_ext)` before splitting, matching Alg 2 behaviour.
- **GPU dashed phantom-segment.** The phantom-segment discard (used for wrap detection at polyline endpoints) is now correctly skipped when `gap_len <= 0`, preventing incorrect pixel rejection in flush/overlap mode.

---

### Typography

- `DrawLinearGradientTextGPU` / `DrawRadialGradientTextGPU` / `DrawDiamondGradientTextGPU` -- GPU gradient fills on GPU-rendered text, evaluated per-glyph or per-string, with color-space selection.
- `DrawImageTextGPU` -- GPU text with an arbitrary texture fill (single or multi-texture).
- `TesselateTextPerGlyph` -- tessellate text and return one `ImWidgetsShape` per glyph for custom downstream processing.
- `ExtractTextContours` -- extract flat polyline contours from tessellated text.

> The original `DrawTextGradient` entry from this cycle is obsolete: the function
> was removed in 2.0.0 (Breaking change #17).

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
