# Compatibility

## Dependency Tiers

Features fall into three tiers based on what they actually call at runtime:

| Tier | Requirement | Backends |
|---|---|---|
| **ImGui only** | Only `ImDrawList` / ImGui APIs. No ImPlatform calls or types in the code path. Could be compiled with zero ImPlatform dependency. | Any ImGui backend |
| **ImPlatform (any)** | Uses `ImPlatform_CreateTexture`, `ImPlatform_UpdateTexture`, `ImPlatform_DestroyTexture`, or `ImPlatform_PixelFormat`. No custom shader. | All ImPlatform backends including DX9 |
| **ImPlatform + Shader** | Uses `ImPlatform_CreateShader`, `ImPlatform_BeginCustomShader`, GPU vertex/index buffers, etc. | DX10, DX11, DX12, OpenGL3+, Vulkan |

> **Note on the current header:** `dear_widgets.h` unconditionally includes `<ImPlatform.h>` today because `ImPaintCanvasData` and `ImImageViewerState` embed `ImPlatform_PixelFormat`. A future refactor (separate canvas/viewer headers, or an opaque pixel-format alias) would let all ImGui-only features compile without ImPlatform entirely.

---

## Feature Compatibility Table

| Feature | Key API | ImGui only | ImPlatform (any) | ImPlatform + Shader |
|---|---|:---:|:---:|:---:|
| **Style system** | `GetStyle`, `PushStyleColor`, `PushStyleVar` | Y | | |
| **Slider2D** | `Slider2DFloat/Int/Scalar` | Y | | |
| **SliderN** | `SliderNFloat/Int/Scalar` | Y | | |
| **SliderRing** | `SliderRingFloat/Int/Scalar` | Y | | |
| **SliderGradientRing / Arc** | `SliderGradientRingFloat/Int/Scalar`, `SliderGradientArcFloat/Int/Scalar`, `SliderGradientArcRangeFloat/Int/Scalar` | Y | | |
| **SliderSpline** | `SliderSplineFloat/Int/Scalar` | Y | | |
| **SliderSplineGradient** | `SliderSplineGradientFloat/Int/Scalar`, `SliderSplineGradientRangeFloat/Int/Scalar` | Y | | |
| **SliderGradient** | `SliderGradientFloat/Int/Scalar` | Y | | |
| **SliderGradientRange** | `SliderGradientRangeFloat/Int/Scalar` | Y | | |
| **DragFloatPrecise** | `DragFloatPrecise` | Y | | |
| **HueSelector** | `HueSelector` | Y | | |
| **GradientEditor** | `GradientEditor`, `GradientSample`, `DrawGradientBar` | Y | | |
| **CurveEditor** | `CurveEditor`, `CurveEditorSample`, `CurveEditorEvalEasing` | Y | | |
| **ColorWheel** | `ColorWheel`, `HDRWheel` ^4 | Y | | |
| **ColorPicker** | `ColorPicker`, `ColorPickerSRGB/HSV/OkLab/OkLCH/CIELab/XYZ` | Y | | |
| **ColorWarper** | `ColorWarper` | Y | | |
| **ColorCurve** | `ColorCurve`, `ColorCurveSample` | Y | | |
| **ToneCurve** | `ToneCurve`, `ToneCurveSample` | Y | | |
| **ParadeScope** | `ParadeScope`, `ImParadeScopeData::Accumulate` | Y | | |
| **VectorScope** | `VectorScope`, `ImVectorScopeData::Accumulate` | Y | | |
| **Histogram** | `Histogram`, `ImHistogramData::Accumulate` | Y | | |
| **CIEChromaticity** | `CIEChromaticity`, `ImCIEChromaticityData::Accumulate` | Y | | |
| **UnitField** | `UnitField` | Y | | |
| **UpVector** | `UpVector` | Y | | |
| **VectorDrawingTool** | `VectorDrawingTool`, `ImVectorDrawingData` | Y | | |
| **ImageCarousel** | `ImageCarousel` | Y | | |
| **ImageBento** | `ImageBento` | Y | | |
| **TransformGizmo** | `TransformGizmo`, `ImTransformGizmoCallbacks` | Y | | |
| **Custom-shape buttons** | `ButtonEx*`, `ImageButtonEx*` | Y | | |
| **Render frames** | `RenderFrame*` | Y | | |
| **Nav cursors** | `RenderNavCursor*` | Y | | |
| **Shape generation** | `GenShape*` | Y | | |
| **Shape gradient fills** | `ShapeLinearGradient`, `ShapeRadialGradient`, `ShapeDiamondGradient` | Y | | |
| **Draw shapes** | `DrawShape`, `DrawImageShape`, `DrawShapeWithHole` | Y | | |
| **Color bands / rings** | `DrawHueBand`, `DrawLuminanceBand`, `DrawSaturationBand`, `DrawColorRing` | Y | | |
| **Procedural color** | `DrawProceduralColor1D/2D` (nearest + bilinear) | Y | | |
| **OkLab / OkLCH quads** | `DrawOkLabQuad`, `DrawOkLchQuad` | Y | | |
| **Chromatic diagrams** | `DrawChromaticityPlot`, `DrawChromaticityPoints`, `DrawChromaticityLines` | Y | | |
| **Graduation lines** | `DrawLinearLine/CircularGraduation`, `DrawLogLine/CircularGraduation` | Y | | |
| **Draw cursors** | `DrawTriangleCursor`, `DrawSignetCursor` | Y | | |
| **Checkerboard** | `DrawCheckerboard` | Y | | |
| **Spline gradient strokes** | `DrawSplineGradient`, `DrawSplineGradientCut` | Y | | |
| **Thick lines / curves** ^5 | `DrawThickLine`, `GetThickLineModeName`, `DrawStrokedPolyline`, `DrawStrokedBezierPath`, `DrawStrokedDashedPolyline`, `DrawStrokedDashedBezierPath`, `DrawStrokedCubicBezier` | Y | | |
| **Dashed polylines (CPU)** | `DrawDashedPolylineAA` with `SetDashedLinesUseGPU(false)` | Y | | |
| **Color conversions** | `ColorConvert*`, `ColorBlend`, `ImColorBlendsRGB`, `KelvinTemperatureTosRGBColors` | Y | | |
| **Window background** | `SetCurrentWindowBackgroundImage` | Y | | |
| **Tessellated text** ^1 | `TesselateText`, `TesselateTextPerGlyph`, `ExtractTextContours` | Y ^1 | | |
| **Gradient/image text fills** ^1 | `DrawImageText`, `DrawLinearGradientText`, `DrawRadialGradientText`, `DrawDiamondGradientText` | Y ^1 | | |
| **Slider2DRange / Slider2DDisc** | `Slider2DRangeFloat/Int/Scalar`, `Slider2DDiscFloat/Int/Scalar` | Y | | |
| **SliderNVertical** | `SliderNVerticalFloat/Int/Scalar` | Y | | |
| **SliderGradientVertical** | `SliderGradientVerticalFloat/Int/Scalar` | Y | | |
| **RangeSlider** | `RangeSliderFloat/Int/Scalar`, `RangeSliderVerticalFloat/Int/Scalar` | Y | | |
| **SegmentedControl** | `SegmentedControlInt/Scalar` | Y | | |
| **SliderRingFloatEx** | `SliderRingFloatEx`, `ImWidgetsSliderFlags` | Y | | |
| **Underlined drag fields** | `DragFloatColorUnderline`, `DragFloatGradientUnderline` | Y | | |
| **Physical / artistic pickers** | the 29 domain `ColorPicker*` widgets | Y | | |
| **Color-space introspection** | `ColorSpaceName`, `ColorSpaceGetPrimaries`, `ColorSpaceGetTransfer`, `ColorSpaceEncode/Decode`, `ColorConvertRGBSpacetoXYZ`, `ColorConvertXYZtoRGBSpace`, `ColorAdaptSpaceToD65`, `ColorAdaptD65ToSpace`, `ChromaticityInsideGamut`, `ColorInsideSpaceGamut` | Y | | |
| **Color model introspection** | `ColorModelName/CompName/CompRange/AxisIndex/DefaultTriple/GetFunctions/IsWorkingSpace`, `ColorModelToDisplay`, `ColorDisplayToModel`, `ColorAxisName`, `ColorHarmonyOffsets`, name helpers | Y | | |
| **Scalar -> colour** | `ColorScalarResolve`, `ColorFromScalar*`, `ColorScalarPreset`, `ColorScalarCallback`, `ColorScalarInSpace` | Y | | |
| **Grading gradients** | `GradingGradient` | Y | | |
| **ColorBars / ColorSlice / ChromaWarp** | `ColorBars`, `ColorSlice`, `ChromaWarp` | Y | | |
| **SpectrumEditor** | `SpectrumEditor`, `SpectrumToXYZ`, `WavelengthToRGB` | Y | | |
| **GridWarp** | `GridWarp`, `ImGridWarpData` | Y | | |
| **Chromaticity gamuts** | `DrawChromaticityGamuts` | Y | | |
| **Delta-E maths** | `ColorDeltaE76/94/2000/OK`, `DeltaEFormulaName`, `DeltaEPerceptualLabel`, `ColorDifferenceVisualizer` | Y | | |
| **3D LUT** | `LoadCubeLUT`, `SaveCubeLUT`, `SampleLUT3D`, `InitIdentityLUT3D`, `ColorLUT3DViewer` | Y | | |
| **LookDev compare** | `LookDevCompare` | Y | | |
| **Volume field generation** | `VolumeGenerateField` | Y | | |
| **Image overlays** | `DetectionOverlay`, `KeypointOverlay`, `TextLabelOverlay`, `AnnotationEditor` | Y | | |
| **NetworkGraph** | `NetworkGraph`, `NetworkGraphLayout` | Y | | |
| **Timeline** | `ImTimeline` | Y | | |
| **Canvas** | `BeginCanvas`, `EndCanvas`, `CanvasWorldToScreen`, `CanvasScreenToWorld`, `CanvasViewportWorld`, `DrawCanvasMinimap` | Y | | |
| **Selection** | `BeginSelection`, `TestSelectionPoints`, `TestSelectionRects` | Y | | |
| **Snapping** | `ComputeSnapAndDraw` | Y | | |
| **Crop / size control** | `CropRect`, `SizeControl2D`, `SizeControl3D` | Y | | |
| **Eyedropper** | `Eyedropper`, `EyedropperSampleBitmap`, `GradientDrop` | Y | | |
| **Onion skin** | `DrawOnionSkin`, `DrawOnionSkinAuto` | Y | | |
| **Arrows / dimensions** | `DrawArrow`, `DrawArrowHead`, `DrawAxisArrows`, `DrawDimensionLine`, `GetArrowHeadName`, `GetDimensionTextOrientName` | Y | | |
| **Crosshair / ruler / grid** | `DrawCrosshair`, `DrawCrosshairInRect`, `GetCrosshairStyleName`, `DrawRuler`, `DrawGridOverlay` | Y | | |
| **Shadow / glow** | `DrawDropShadowRect`, `DrawInnerGlowRect` | Y | | |
| **Gradient mesh / toon ramp** | `GradientMeshInit`, `GradientMeshEditor`, `ToonRampEditor`, `ToonRampSample` | Y | | |
| **Matrix editor** | `MatrixEditor`, `MatrixEditorDouble` | Y | | |
| **Dials & fields** | `NotchedDial`, `AngleDial` | Y | | |
| **Precision popups** | `BeginPrecisionPopup`, `EndPrecisionPopup`, `PrecisionFloat`, `PrecisionInt` | Y | | |
| **Interaction helpers** | `KeyboardNudgeXY`, `KeyboardNudge1D`, `AxisConstrain`, `PushDeltaOverlay`, `RenderDeltaOverlay`, `PushDragGroup`, `PopDragGroup`, `GetDragGroupDelta`, `SetDragGroupActive` | Y | | |
| **Hit-test primitives** | `Im_Is*Contains`, `IsMouseHovering`, `ItemHoverable`, `ButtonBehavior*`, `IsBoundingBoxWellFormed` | Y | | |
| **Hatch / stipple fills** | `DrawHatchFill`, `DrawStippleFill` | Y | | |
| **Coons / Gregory patches** | `DrawCoonsPatch`, `DrawCoonsPatchGradient`, `DrawCoonsPatchWireframe`, `DrawGregoryPatch`, `DrawGregoryPatchGradient` | Y | | |
| **Offset curves** | `OffsetPolyline`, `DrawOffsetOutline`, `DrawOffsetFilled` | Y | | |
| **Iso-contours** | `DrawIsoContour`, `DrawIsoContourTiered`, `DrawIsoFilled` | Y | | |
| **Conic gradients** | `DrawConicGradient`, `DrawConicGradientRect` | Y | | |
| **Ephemerides / sky charts** | `DrawMoonEphemeris`, `DrawEarthSunEphemeris`, `DrawStarChart`, `DrawSunPath`, `GetSkyCultureName`, `GetSkyCultureCount` | Y | | |
| **Text on path / in shape** ^1 | `DrawTextOnPath`, `DrawTextInsideRect`, `DrawTextInsideConvex`, `DrawFontWaterfall` | Y ^1 | | |
| **Font tooling** ^1 | `FontInspector`, `KerningPairEditor`, `VarFontAxisSliders` | Y ^1 | | |
| **EquationInput** ^7 | `EquationInput` | | | Y `LaTeX` |
| **Style persistence** | `SaveStyleToFile`, `LoadStyleFromFile`, `ShowStyleEditor` | Y | | |
| **Context lifecycle** | `CreateContext`, `DestroyContext`, `SetCurrentContext` | | Y ^2 | |
| **System textures** | `GetWhiteTexture`, `OwnTexture` | | Y | |
| **PaintCanvas** | `PaintCanvas`, `ImPaintCanvasData` | | Y | |
| **ImageViewer (pixel inspector)** | `ImageViewer` with `state.Pixels` set and `shaderProgram = nullptr` | | Y ^3 | |
| **Dashed polylines (GPU)** | `DrawDashedPolylineAA` with `SetDashedLinesUseGPU(true)` | | | Y |
| **Markers** | `DrawMarker` | | | Y `Markers` |
| **Slug GPU text** | `DrawText`, `CalcTextSize`, `CalcShapedTextWidth` | | | Y `RichFont` |
| **Slug font loader** | `GetSlugFontLoader`, `GetSlugFontInfo`, `SlugBuildGlyphByID`, `PrewarmTessellationCache` | | | Y `RichFont` |
| **Slug debug** | `DrawTextDebugCurves`, `DrawTextDebugLayers`, `SetSlugDebugShader` / `GetSlugDebugShader` | | | Y `RichFont` |
| **LaTeX** | `LoadLaTeXFont`, `DrawLaTeX`, `CalcLaTeXSize`, `DrawLaTeXDebug`, `TesselateLaTeX` | | | Y `LaTeX` |
| **GPU gradient / image text** | `DrawLinearGradientTextGPU`, `DrawRadialGradientTextGPU`, `DrawDiamondGradientTextGPU`, `DrawImageTextGPU` | | | Y `RichFont` |
| **ImageInspector** | `ImageInspector`, `ImageInspectorSupported`, `ImageInspectorReleaseState` | | | Y ^6 |
| **ImageViewer (custom shader)** | `ImageViewer` with a non-null `shaderProgram` ^3 | | | Y ^6 |
| **VolumeViewer** | `VolumeViewer` | | | Y ^6 |
| **VolumeSliceViewer** | `VolumeSliceViewer` | | Y | |
| **LookDevInspector** | `LookDevInspector` | | | Y ^6 |
| **Delta-E image compare** | `ColorDifferenceImageViewer` | | | Y ^6 |
| **Window blur / background effects** | `SetCurrentWindowBlurBackground`, `BlurBackgroundNewFrame` | | | Y ^6 |
| **Shader plumbing** | `PrebuildShaders`, `CreateInternalShader` | | | Y ^6 |

**^1 Tessellated text** -- the algorithm itself is pure CPU (stbtt + bezier tessellation -> `ImWidgetsShape` -> `DrawShape`). It is listed as "ImGui only" in intent. In the current implementation it shares the Slug context initialised by `CreateContext`, so enabling `ImWidgetsFeatures_RichFont` is needed today.

**^2 `CreateContext`** -- always calls `ImPlatform_CreateTexture` for the internal black/white system textures even when no shader feature is enabled. Destroying those textures on `DestroyContext` calls `ImPlatform_DestroyTexture`.

**^3 `ImageViewer`** -- pan/zoom display is pure ImGui (forwards a user-provided `ImTextureID`). The pixel-inspector loupe (right-click) reads `ImImageViewerState::PixelFormat` which is an `ImPlatform_PixelFormat` type; setting `state.Pixels = nullptr` disables the inspector and removes any runtime ImPlatform dependency.

The widget has **two rows** because its tier depends on how it is called. Pan/zoom display forwards a caller-provided `ImTextureID` and is pure ImGui; the pixel inspector adds the ImPlatform dependency above; and passing a non-null `shaderProgram` wraps the image draw in a custom pixel shader, which moves that call to the shader tier. The shader binds *inside* the widget, so it also applies to the expand-to-window modal. `overlay_callback` / `overlay_user_data` add no dependency of their own.

**^4 `HDRWheel`** -- unified ColorWheel + gradient indicator ring + optional right/left arc sliders. Pass `rightValue = leftValue = NULL` (the default) to get the layout previously exposed as `PrimariesWheel` (no arcs, tighter footprint); pass non-null to get the arc-slider HDR layout. The standalone `PrimariesWheel` symbol has been removed.


**^6 Lazily compiled shaders -- not behind a feature flag.** `image_inspector.hlsl`, `volume_viewer.hlsl`, `lookdev_inspector.hlsl`, `delta_e_compare.hlsl` and `blur.hlsl` are compiled **on first use** through `CreateInternalShader`, not at `CreateContext` time, and are **not** gated by any `ImWidgetsFeatures_` value. There is no flag to set: the widget only needs a backend with `IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER`. `ImageInspectorSupported()` reports whether the current build and backend can run the inspector; the others render a "not supported" message and return `false`. Call `PrebuildShaders()` once after `CreateContext()` to pay the compile cost up front instead of on the first frame that uses one.

**^7 `EquationInput`** -- the text field itself is ImGui-only, but it typesets its content through the LaTeX renderer, so it needs `ImWidgetsFeatures_LaTeX` and therefore the Slug shader path.

### ^5 `DrawThickLine` — runtime fallback details

`DrawThickLine(ImDrawList*, ImVec2 const* points, int count, ImWidgetsThickLineDesc const&)` is a single dispatcher behind which sit five distinct line-rendering techniques. **The dispatcher itself is callable from an ImGui-only build**: every mode either uses ImGui-native primitives directly, links against unconditionally-compiled CPU code in `dear_widgets_stroke.cpp`, or — in the case of the GPU-accelerated AA mode — has an explicit runtime fallback that swaps the implementation to ImGui-native primitives without changing the caller-visible enum value or behavior.

#### Per-mode tier and runtime path

| `ImWidgetsThickLineMode` | Technique | Backend used | Tier when shader on | Tier when shader off |
|---|---|---|---|---|
| `AddPolyline` | ImGui's `ImDrawList::AddPolyline` | ImGui only | ImGui only | ImGui only |
| `PolylineAA` | Rougier 2013 SDF polyline, GPU fast path with CPU fringe fallback inside `DrawPolylineAA` / `DrawDashedPolylineAA` | ImPlatform + Shader (GPU) **or** silently degraded to `AddPolyline` (+ software dash) when the function isn't compiled in | ImPlatform + Shader | ImGui only ^5a |
| `StrokedPolyline` | Linebender Euler-spiral offset curves on the raw polyline (`DrawStrokedPolyline` / `DrawStrokedDashedPolyline`) | Pure CPU; not gated by `IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER` | ImGui only | ImGui only |
| `StrokedBezierPath` | Same Euler-spiral renderer applied to a cubic Bezier path obtained by Catmull-Rom -> cubic conversion (`CatmullRomToCubicBezierPath`) of the input polyline (`DrawStrokedBezierPath` / `DrawStrokedDashedBezierPath`) | Pure CPU | ImGui only | ImGui only |
| `ImGuiBezier` | Same Catmull-Rom -> cubic conversion; each cubic emitted via `ImDrawList::AddBezierCubic` (ImGui's own adaptive tessellation) | ImGui only | ImGui only | ImGui only |

The "ImGui only" tier in the main feature row above is therefore not a compile-time downgrade — it is the **caller-visible** tier. The dispatcher is portable; the *quality* of `PolylineAA` rendering is what changes between shader-enabled and shader-less builds.

**^5a `PolylineAA` runtime fallback** -- when `IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER` is not defined, `DrawPolylineAA` / `DrawDashedPolylineAA` are not linked. The dispatcher detects this at compile time and routes solid `PolylineAA` calls to `ImDrawList::AddPolyline`, and dashed `PolylineAA` calls to a software dash splitter (`DW_SoftwareDashedPolyline` in `dear_widgets.cpp`). The caller's `ImWidgetsThickLineMode_PolylineAA` enum value, descriptor fields, and saved-style files are all preserved — only the on-screen pixels differ. This is invisible to users of the four curve widgets that consume the mode through style.

#### Dashed handling

`ImWidgetsThickLineDesc::dashed` is orthogonal to the mode. It is honored by all modes that have a dashed primitive available, with a software splitter used wherever a native one isn't:

- `AddPolyline` and `ImGuiBezier` -> always software dash via `DW_SoftwareDashedPolyline` (the latter tessellates the cubic path to a polyline first).
- `PolylineAA` -> `DrawDashedPolylineAA` with shader; software dash without.
- `StrokedPolyline` / `StrokedBezierPath` -> their `DrawStrokedDashed*` siblings; no fallback needed (CPU).

The software dasher places dash N at arc-length `[N*period + offset, +dash_len]` rather than toggling state along the path, so **negative `gap_len` is supported** — it makes successive dashes overlap (period < dash_len), matching `DrawDashedPolylineAA`'s pattern semantics. Only `gap_len <= -dash_len` (period <= 0) is rejected and rendered solid, since infinite overlap would otherwise apply.

#### Style integration

The four curve-rendering widgets carry a `*_LineMode` (`int`) and `*_LineDashed` (`bool`) field in `ImWidgetsStyle` and route their curve emission through `DrawThickLine`:

| Widget | Style fields | Internal sample source |
|---|---|---|
| `DrawChromaticityLines{,HDR}` | `ChromaticityLine_Mode`, `ChromaticityLine_Dashed`, `ChromaticityLine_Thickness` | Caller-supplied colors -> chromaticity (x,y) |
| `CurveEditor` | `CurveEditor_LineMode`, `CurveEditor_LineDashed`, `CurveEditor_LineThickness` | One polyline accumulating Step / Linear / Bezier segments (sampled at 64 per smooth segment) |
| `ColorCurve` | `ColorCurve_LineMode`, `ColorCurve_LineDashed`, `ColorCurve_LineThickness` | `ColorCurveSample` over `max(64, w/2)` samples |
| `ToneCurve` | `ToneCurve_LineMode`, `ToneCurve_LineDashed`, `ToneCurve_LineThickness` | `ToneCurveSample` per channel |

Style fields persist via `dw_style.ini` through the `DW_SI` / `DW_LI` (int) and `DW_SB` / `DW_LB` (bool) save/load macros. A style file written by a shader-enabled build remains valid in a shader-less build: `PolylineAA` simply renders via the runtime fallback there.

#### Where to see it interactively

The Demo's `Draw -> Thick Line` tree node (in `src/demo/demo.cpp`) exposes every descriptor field — mode combo, dashed checkbox, color, thickness, cap/join, miter limit, dash length / gap length / dash offset (gap slider includes negative range), tolerance — together with a curve-shape selector (Sinusoid / Lissajous / Spiral / Polyline kink) and a sample-count slider so each mode can be visually compared against the same input.

---

## Supported Backends

| Backend | ImGui only | ImPlatform (any) | ImPlatform + Shader |
|---|:---:|:---:|:---:|
| Any ImGui backend (no ImPlatform) | Y | -- | -- |
| DirectX 9 | Y | Y | N |
| DirectX 10 | Y | Y | Y |
| DirectX 11 | Y | Y | Y |
| DirectX 12 | Y | Y | Y |
| OpenGL 3+ | Y | Y | Y |
| Vulkan | Y | Y | Y |

---

## Feature Flag Summary

| Flag | Features Enabled | Tier |
|---|---|---|
| *(none -- `CreateContext` only)* | All ImGui-only features + system textures | ImPlatform (any) |
| `ImWidgetsFeatures_Markers` | GPU shape markers | ImPlatform + Shader |
| `ImWidgetsFeatures_RichFont` | Slug GPU text, color fonts, gradient text, tessellated fills | ImPlatform + Shader |
| `ImWidgetsFeatures_LaTeX` | LaTeX math rendering (implies `RichFont`) | ImPlatform + Shader |

Flags combine with `|`. Setting no flags and calling `CreateContext` gives you all drawing and widget features using the CPU path only.

This is a bit-flag enum with no `_COUNT` sentinel (one used to be declared by mistake; removed in 2.0.0).

> **Not every shader is behind a flag.** `image_inspector.hlsl`, `volume_viewer.hlsl`, `lookdev_inspector.hlsl`, `delta_e_compare.hlsl` and `blur.hlsl` are compiled lazily on first use through `CreateInternalShader` and are **not** gated by any `ImWidgetsFeatures_` value -- so `ImageInspector`, `VolumeViewer`, `LookDevInspector`, `ColorDifferenceImageViewer` and `SetCurrentWindowBlurBackground` work with no flag set, provided the backend supports custom shaders. See footnote ^6 above.
