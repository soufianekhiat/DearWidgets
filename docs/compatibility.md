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
| **SliderGradientRing** | `SliderGradientRingFloat/Int/Scalar`, `SliderGradientRingRangeFloat/Int/Scalar` | Y | | |
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
| **ImageTransformGizmo** | `ImageTransformGizmo` | Y | | |
| **Custom-shape buttons** | `ButtonEx*`, `ImageButtonEx*` | Y | | |
| **Render frames** | `RenderFrame*` | Y | | |
| **Nav cursors** | `RenderNavCursor*` | Y | | |
| **Shape generation** | `GenShape*` | Y | | |
| **Shape gradient fills** | `Shape*Gradient` (sRGB, OkLab, HSV...) | Y | | |
| **Draw shapes** | `DrawShape`, `DrawImageShape`, `DrawShapeWithHole` | Y | | |
| **Color bands / rings** | `DrawHueBand`, `DrawLumianceBand`, `DrawSaturationBand`, `DrawColorRing` | Y | | |
| **Procedural color** | `DrawProceduralColor1D/2D` (nearest + bilinear) | Y | | |
| **OkLab / OkLCH quads** | `DrawOkLabQuad`, `DrawOkLchQuad` | Y | | |
| **Chromatic diagrams** | `DrawChromaticityPlot`, `DrawChromaticityPoints`, `DrawChromaticityLines` | Y | | |
| **Graduation lines** | `DrawLinearLine/CircularGraduation`, `DrawLogLine/CircularGraduation` | Y | | |
| **Draw cursors** | `DrawTriangleCursor`, `DrawSignetCursor` | Y | | |
| **Checkerboard** | `DrawCheckerboard` | Y | | |
| **Spline gradient strokes** | `DrawSplineGradient`, `DrawSplineGradientCut` | Y | | |
| **Thick lines / curves** ^5 | `DrawThickLine`, `GetThickLineModeName`, `DrawStrokedPolyline`, `DrawStrokedBezierPath`, `DrawStrokedDashedPolyline`, `DrawStrokedDashedBezierPath`, `DrawStrokedCubicBezier` | Y | | |
| **Dashed polylines (CPU)** | `DrawDashedPolylineAA` with `SetDashedLinesUseGPU(false)` | Y | | |
| **Color conversions** | `ColorConvert*`, `ImColorBlend*`, `KelvinTemperatureTosRGBColors` | Y | | |
| **Window background** | `SetCurrentWindowBackgroundImage` | Y | | |
| **Tessellated text** ^1 | `TesselateText`, `TesselateTextPerGlyph`, `ExtractTextContours` | Y ^1 | | |
| **Gradient/image text fills** ^1 | `DrawImageText`, `DrawLinearGradientText`, `DrawRadialGradientText`, `DrawDiamondGradientText` | Y ^1 | | |
| **Context lifecycle** | `CreateContext`, `DestroyContext`, `SetCurrentContext` | | Y ^2 | |
| **System textures** | `GetWhiteTexture`, `OwnTexture` | | Y | |
| **PaintCanvas** | `PaintCanvas`, `ImPaintCanvasData` | | Y | |
| **ImageViewer (pixel inspector)** | `ImageViewer` with `state.Pixels` set | | Y ^3 | |
| **Dashed polylines (GPU)** | `DrawDashedPolylineAA` with `SetDashedLinesUseGPU(true)` | | | Y |
| **Markers** | `DrawMarker` | | | Y `Markers` |
| **Slug GPU text** | `DrawText`, `DrawTextGradient`, `CalcTextSize` | | | Y `RichFont` |
| **Slug font loader** | `GetSlugFontLoader`, `SlugBuildGlyphByID` | | | Y `RichFont` |
| **Slug debug** | `DrawTextDebugCurves`, `DrawTextDebugLayers`, `g_SlugDebugShader` | | | Y `RichFont` |
| **LaTeX** | `LoadLaTeXFont`, `DrawLaTeX`, `CalcLaTeXSize`, `DrawLaTeXDebug`, `TesselateLaTeX` | | | Y `LaTeX` |

**^1 Tessellated text** -- the algorithm itself is pure CPU (stbtt + bezier tessellation -> `ImWidgetsShape` -> `DrawShape`). It is listed as "ImGui only" in intent. In the current implementation it shares the Slug context initialised by `CreateContext`, so enabling `ImWidgetsFeatures_RichFont` is needed today.

**^2 `CreateContext`** -- always calls `ImPlatform_CreateTexture` for the internal black/white system textures even when no shader feature is enabled. Destroying those textures on `DestroyContext` calls `ImPlatform_DestroyTexture`.

**^3 `ImageViewer`** -- pan/zoom display is pure ImGui (forwards a user-provided `ImTextureID`). The pixel-inspector loupe (right-click) reads `ImImageViewerState::PixelFormat` which is an `ImPlatform_PixelFormat` type; setting `state.Pixels = nullptr` disables the inspector and removes any runtime ImPlatform dependency.

**^4 `HDRWheel`** -- unified ColorWheel + gradient indicator ring + optional right/left arc sliders. Pass `rightValue = leftValue = NULL` (the default) to get the layout previously exposed as `PrimariesWheel` (no arcs, tighter footprint); pass non-null to get the arc-slider HDR layout. The standalone `PrimariesWheel` symbol has been removed.

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
