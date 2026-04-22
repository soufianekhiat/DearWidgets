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
| **ColorWheel** | `ColorWheel`, `PrimariesWheel`, `HDRWheel` | Y | | |
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
