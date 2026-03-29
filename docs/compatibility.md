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
| **Style system** | `GetStyle`, `PushStyleColor`, `PushStyleVar` | ✓ | | |
| **Slider2D** | `Slider2DFloat/Int/Scalar` | ✓ | | |
| **SliderN** | `SliderNFloat/Int/Scalar` | ✓ | | |
| **SliderRing** | `SliderRingFloat/Int/Scalar` | ✓ | | |
| **SliderSpline** | `SliderSplineFloat/Int/Scalar` | ✓ | | |
| **DragFloatPrecise** | `DragFloatPrecise` | ✓ | | |
| **HueSelector** | `HueSelector` | ✓ | | |
| **GradientEditor** | `GradientEditor`, `GradientSample`, `DrawGradientBar` | ✓ | | |
| **CurveEditor** | `CurveEditor`, `CurveEditorSample`, `CurveEditorEvalEasing` | ✓ | | |
| **ColorWheel** | `ColorWheel`, `PrimariesWheel`, `HDRWheel` | ✓ | | |
| **ColorPicker** | `ColorPicker`, `ColorPickerSRGB/HSV/OkLab/OkLCH/CIELab/XYZ` | ✓ | | |
| **ColorWarper** | `ColorWarper` | ✓ | | |
| **ColorCurve** | `ColorCurve`, `ColorCurveSample` | ✓ | | |
| **ToneCurve** | `ToneCurve`, `ToneCurveSample` | ✓ | | |
| **ParadeScope** | `ParadeScope`, `ImParadeScopeData::Accumulate` | ✓ | | |
| **VectorScope** | `VectorScope`, `ImVectorScopeData::Accumulate` | ✓ | | |
| **Histogram** | `Histogram`, `ImHistogramData::Accumulate` | ✓ | | |
| **CIEChromaticity** | `CIEChromaticity`, `ImCIEChromaticityData::Accumulate` | ✓ | | |
| **UnitField** | `UnitField` | ✓ | | |
| **UpVector** | `UpVector` | ✓ | | |
| **ImageCarousel** | `ImageCarousel` | ✓ | | |
| **ImageBento** | `ImageBento` | ✓ | | |
| **ImageTransformGizmo** | `ImageTransformGizmo` | ✓ | | |
| **Custom-shape buttons** | `ButtonEx*`, `ImageButtonEx*` | ✓ | | |
| **Render frames** | `RenderFrame*` | ✓ | | |
| **Nav cursors** | `RenderNavCursor*` | ✓ | | |
| **Shape generation** | `GenShape*` | ✓ | | |
| **Shape gradient fills** | `Shape*Gradient` (sRGB, OkLab, HSV…) | ✓ | | |
| **Draw shapes** | `DrawShape`, `DrawImageShape`, `DrawShapeWithHole` | ✓ | | |
| **Color bands / rings** | `DrawHueBand`, `DrawLumianceBand`, `DrawSaturationBand`, `DrawColorRing` | ✓ | | |
| **Procedural color** | `DrawProceduralColor1D/2D` (nearest + bilinear) | ✓ | | |
| **OkLab / OkLCH quads** | `DrawOkLabQuad`, `DrawOkLchQuad` | ✓ | | |
| **Chromatic diagrams** | `DrawChromaticityPlot`, `DrawChromaticityPoints`, `DrawChromaticityLines` | ✓ | | |
| **Graduation lines** | `DrawLinearLine/CircularGraduation`, `DrawLogLine/CircularGraduation` | ✓ | | |
| **Draw cursors** | `DrawTriangleCursor`, `DrawSignetCursor` | ✓ | | |
| **Checkerboard** | `DrawCheckerboard` | ✓ | | |
| **Dashed polylines (CPU)** | `DrawDashedPolylineAA` with `SetDashedLinesUseGPU(false)` | ✓ | | |
| **Color conversions** | `ColorConvert*`, `ImColorBlend*`, `KelvinTemperatureTosRGBColors` | ✓ | | |
| **Window background** | `SetCurrentWindowBackgroundImage` | ✓ | | |
| **Tessellated text** ¹ | `TesselateText`, `TesselateTextPerGlyph`, `ExtractTextContours` | ✓ ¹ | | |
| **Gradient/image text fills** ¹ | `DrawImageText`, `DrawLinearGradientText`, `DrawRadialGradientText`, `DrawDiamondGradientText` | ✓ ¹ | | |
| **Context lifecycle** | `CreateContext`, `DestroyContext`, `SetCurrentContext` | | ✓ ² | |
| **System textures** | `GetWhiteTexture`, `OwnTexture` | | ✓ | |
| **PaintCanvas** | `PaintCanvas`, `ImPaintCanvasData` | | ✓ | |
| **ImageViewer (pixel inspector)** | `ImageViewer` with `state.Pixels` set | | ✓ ³ | |
| **Dashed polylines (GPU)** | `DrawDashedPolylineAA` with `SetDashedLinesUseGPU(true)` | | | ✓ |
| **Markers** | `DrawMarker` | | | ✓ `Markers` |
| **Slug GPU text** | `DrawText`, `DrawTextGradient`, `CalcTextSize` | | | ✓ `RichFont` |
| **Slug font loader** | `GetSlugFontLoader`, `SlugBuildGlyphByID` | | | ✓ `RichFont` |
| **Slug debug** | `DrawTextDebugCurves`, `DrawTextDebugLayers`, `g_SlugDebugShader` | | | ✓ `RichFont` |
| **LaTeX** | `LoadLaTeXFont`, `DrawLaTeX`, `CalcLaTeXSize`, `DrawLaTeXDebug`, `TesselateLaTeX` | | | ✓ `LaTeX` |

**¹ Tessellated text** — the algorithm itself is pure CPU (stbtt + bezier tessellation → `ImWidgetsShape` → `DrawShape`). It is listed as "ImGui only" in intent. In the current implementation it shares the Slug context initialised by `CreateContext`, so enabling `ImWidgetsFeatures_RichFont` is needed today.

**² `CreateContext`** — always calls `ImPlatform_CreateTexture` for the internal black/white system textures even when no shader feature is enabled. Destroying those textures on `DestroyContext` calls `ImPlatform_DestroyTexture`.

**³ `ImageViewer`** — pan/zoom display is pure ImGui (forwards a user-provided `ImTextureID`). The pixel-inspector loupe (right-click) reads `ImImageViewerState::PixelFormat` which is an `ImPlatform_PixelFormat` type; setting `state.Pixels = nullptr` disables the inspector and removes any runtime ImPlatform dependency.

---

## Supported Backends

| Backend | ImGui only | ImPlatform (any) | ImPlatform + Shader |
|---|:---:|:---:|:---:|
| Any ImGui backend (no ImPlatform) | ✓ | — | — |
| DirectX 9 | ✓ | ✓ | ✗ |
| DirectX 10 | ✓ | ✓ | ✓ |
| DirectX 11 | ✓ | ✓ | ✓ |
| DirectX 12 | ✓ | ✓ | ✓ |
| OpenGL 3+ | ✓ | ✓ | ✓ |
| Vulkan | ✓ | ✓ | ✓ |

---

## Feature Flag Summary

| Flag | Features Enabled | Tier |
|---|---|---|
| *(none — `CreateContext` only)* | All ImGui-only features + system textures | ImPlatform (any) |
| `ImWidgetsFeatures_Markers` | GPU shape markers | ImPlatform + Shader |
| `ImWidgetsFeatures_RichFont` | Slug GPU text, color fonts, gradient text, tessellated fills | ImPlatform + Shader |
| `ImWidgetsFeatures_LaTeX` | LaTeX math rendering (implies `RichFont`) | ImPlatform + Shader |

Flags combine with `|`. Setting no flags and calling `CreateContext` gives you all drawing and widget features using the CPU path only.
