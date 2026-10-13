# DearWidgets

Advanced widgets and rendering utilities for [Dear ImGui](https://github.com/ocornut/imgui), built for graphics applications: color grading tools, image processing, 3D visualization, scientific plotting, and rich-text rendering.

**Version 2.0.0** -- the public API is frozen. The header exposes
`IMWIDGETS_VERSION` (`"2.0.0"`) and `IMWIDGETS_VERSION_NUM` (`20000`, i.e.
`major*10000 + minor*100 + patch`) so you can branch on it:

```cpp
#if IMWIDGETS_VERSION_NUM >= 20000
    // 2.0 API
#endif
```

See [CHANGELOG.md](CHANGELOG.md) for the 2.0.0 breaking changes and migration notes.

![DearWidgets Style Integration](https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/WithStyle.gif)

---

## What's Inside

### Sliders and numeric input
- **Slider2D / Slider2DRange / Slider2DDisc** -- 2D pads: point, rectangle range, disc
- **SliderN / SliderNVertical** -- multi-handle ordered sliders
- **SliderRing / SliderRingFloatEx / SliderSpline** -- arc and Bezier-track sliders
- **SliderGradient / SliderGradientVertical / SliderGradientRing / SliderGradientArc / SliderSplineGradient** -- gradient-painted tracks, single handle, optional fill-up-to-cursor
- **...Range variants** of each gradient slider -- two handles, gradient painted between them
- **RangeSlider / RangeSliderVertical** -- plain two-handle range sliders
- **SegmentedControl** -- mutually exclusive segment row
- **DragFloatPrecise / DragFloatColorUnderline / DragFloatGradientUnderline** -- precision drag and grading-style drag fields
- **NotchedDial / AngleDial / UpVector / UnitField** -- dials, direction picker, unit-aware field
- **SizeControl2D / SizeControl3D** -- dimension editors with an aspect-ratio lock
- **MatrixEditor / MatrixEditorDouble** -- grid of drag fields over a row-major buffer
- **EquationInput** -- text field that typesets its content as LaTeX
- **PrecisionFloat / PrecisionInt** -- typed-input popups

### Color editing and grading
- **GradientEditor** -- multi-stop gradient with independent alpha track and color-space interpolation (sRGB, Linear sRGB, OkLab, OkLCH, HSV)
- **CurveEditor / ColorCurve / ToneCurve** -- spline curve editors with Catmull-Rom and easing
- **ColorWheel / HDRWheel** -- perceptual color selectors in HSV and OkLCH, with optional HDR arc sliders
- **ColorPicker** -- unified picker with per-space variants: sRGB, HSV, OkLab, OkLCH, CIELab, XYZ
- **29 physically-based and artistic pickers** -- skin, hair, leaf, haemoglobin, subsurface, bruise, iris, blackbody, flame, star, nebula, sky, cloud, ice, water, gem, pigment, ochre, patina, weathered metal, Maillard browning, gas discharge, streetlight, fabric dye, toon ramp, trichromatic mixer, and three harmony/palette pickers
- **ColorWarper / ChromaWarp / GridWarp** -- hue/saturation, chromaticity-plane and free-form warp grids
- **ColorBars / ColorSlice** -- Resolve-style lift-gamma-gain bars and multi-vector slice grading
- **SpectrumEditor** -- spectral power distribution with live CIE integration
- **HueSelector** -- hue band picker with feathering
- **Unified scalar -> colour API** -- one `float -> ImU32` mapping described by an options struct (source, model, space, axis, harmony, easing, gamut policy), resolved once to function pointers
- **Color-space and color-model introspection** -- primaries, transfer functions, white-point adaptation, gamut tests, component names and ranges

### Scopes and analysis
- **ParadeScope / VectorScope / Histogram / CIEChromaticity** -- color analysis scopes
- **Delta-E** -- CIE76 / CIE94 / CIEDE2000 / OkLab metrics, two-swatch visualizer, and a GPU per-pixel difference heat map

### Images
- **ImageViewer** -- pan/zoom viewer with a pixel-inspector loupe, an optional custom pixel shader, and overlay callbacks that survive its expand-to-window modal
- **Detection / Keypoint / TextLabel overlays and AnnotationEditor** -- UV-space overlays composed on top of the viewer
- **ImageInspector** -- GPU inspector: transfer curves, gamut, exposure, false colour, NaN highlight, mosaic decode, exact-precision loupe
- **LookDevCompare / LookDevInspector** -- wipe and side-by-side comparison, plus shader-backed inspection modes
- **ColorLUT3DViewer** -- `.cube` 3D LUT load/save, sampling and inspection
- **VolumeViewer / VolumeSliceViewer** -- ray-marched and slice-based 3D field viewers
- **ImageCarousel / ImageBento** -- strip and grid galleries with drag-to-reorder
- **TransformGizmo** -- 2D translate/rotate/scale gizmo, drawing delegated to callbacks
- **PaintCanvas** -- pixel-level paint surface backed by a GPU texture

### Graphs, timelines and authoring
- **ImNetworkGraph** -- read-only hierarchically grouped DAG illustration with layered layout, LOD and drill-down
- **ImTimeline** -- temporal timeline with playhead, authored keys, events and cache/history bands
- **VectorDrawingTool** -- bezier path authoring canvas with zoom/pan, per-path cap/join and dash style
- **GradientMeshEditor / ToonRampEditor** -- Coons-patch colour mesh and cel-shading ramp editors
- **Canvas toolkit** -- `BeginCanvas`/`EndCanvas` with minimap, marquee/lasso selection, alignment snapping with guides, crop rectangle, eyedropper and gradient-drop sampling, onion skin, ruler, grid overlay, crosshairs, drop shadow and inner glow

### DrawList extensions
- **Shape system** -- generate, fill (linear/radial/diamond gradients in any color space) and draw arbitrary 2D shapes with holes
- **GPU markers** -- resolution-independent shape markers for plots
- **Thick lines** -- one `DrawThickLine` dispatcher over five techniques, with a runtime fallback where shaders are unavailable
- **Dashed polylines** -- CPU or GPU anti-aliased dashes; `gap=0` = flush, `gap<0` = overlapping envelopes
- **Euler-spiral strokes** -- `DrawStrokedPolyline`, `DrawStrokedBezierPath`, `DrawStrokedCubicBezier` and dashed siblings
- **Spline gradient strokes** -- multi-stop gradient along a Bezier spline (`DrawSplineGradient`, `DrawSplineGradientCut`)
- **Conic gradients** -- around a disc or across a rectangle
- **Hatch and stipple fills** -- parallel, cross, diagonal, dots, concentric, Ben-Day, screentone
- **Coons and Gregory patches** -- flat, per-corner gradient and wireframe
- **Offset curves** -- parallel-curve offsetting with proper joins, outlined or filled
- **Iso-contours** -- marching squares, tiered (major/medium/minor) and filled bands
- **Color bands and rings** -- hue, saturation, luminance and arbitrary callback-driven rings
- **OkLab / OkLCH quads** -- perceptually uniform gradient fills for rectangular regions
- **Procedural color** -- ShaderToy-style 1D/2D CPU-evaluated color functions (horizontal, vertical, arc, spline)
- **Chromaticity plots** -- CIE 1931/1964 diagrams with illuminants, primaries, observers, plus multi-gamut overlays
- **Ephemerides and sky charts** -- moon phase, day/night terminator, star chart with four sky cultures, sun path with analemma
- **Arrows and dimension lines** -- eight arrowheads, technical-drawing dimensions with witness lines
- **Graduation lines** -- linear and circular scales, linear and logarithmic
- **Custom-shape buttons** -- convex, concave and holed polygon buttons with real hit-testing
- **Window background** -- set a texture as the current window background, or run one of 18 blur / glass / CRT / halftone effects behind it

### GPU font rendering (Slug)
- **DrawText / CalcTextSize / CalcShapedTextWidth** -- resolution-independent glyph rendering from Bezier outlines (no atlas, no SDF)
- **Color fonts** -- full COLR v0/v1 support (color layers, gradients)
- **Gradient and image text fills** -- tessellated outline fills with arbitrary gradients (CPU and GPU paths)
- **GPU gradient text** -- `DrawLinearGradientTextGPU`, `DrawRadialGradientTextGPU`, `DrawDiamondGradientTextGPU`, `DrawImageTextGPU`; per-glyph or per-string, with color-space selection
- **Text on a path, inside a rect, inside a convex polygon, and font waterfalls**
- **Contour extraction** -- `TessellateTextPerGlyph`, `ExtractTextContours` for downstream use of glyph geometry
- **FontInspector / KerningPairEditor / VarFontAxisSliders** -- glyph grid, metrics, curves, kerning and variable-axis tooling
- **Arabic / Hebrew / Indic shaping** -- via the vendored `kb_text_shape`

### LaTeX math rendering
- **DrawLaTeX / CalcLaTeXSize / TessellateLaTeX** -- self-contained math typesetter using Latin Modern Math
- Supports scripts, fractions, roots, matrices, `\left...\right` stretchy delimiters, aligned environments
- Rendered via the Slug GPU pipeline

---

## Dependency Tiers

Features are grouped into three tiers depending on what they call at runtime:

| Tier | Requirements | Backends |
|---|---|---|
| **ImGui only** | Pure `ImDrawList` / ImGui APIs | Any ImGui backend |
| **ImPlatform (any)** | Texture creation/destruction, pixel format | All ImPlatform backends including DX9 |
| **ImPlatform + Shader** | Custom shader compilation and dispatch | DX10, DX11, DX12, OpenGL 3+, Vulkan |

Most widgets and almost all DrawList extensions are **ImGui only**. The shader tier is required for GPU markers, Slug font rendering, LaTeX, GPU dashed lines, the image inspector, the volume and LookDev inspectors, the GPU Delta-E compare and the window background effects.

See [docs/compatibility.md](docs/compatibility.md) for the full per-feature table.

---

## Getting Started

### Prerequisites

- C++11 or later (MSVC 2019+, GCC, Clang)
- Dear ImGui (bundled via the ImPlatform submodule)
- ImPlatform (included as a submodule) -- optional for ImGui-only usage
- .NET 6 SDK, to build Sharpmake (Windows build only)

### Clone

```bash
git clone --recursive https://github.com/soufianekhiat/DearWidgets.git
cd DearWidgets
```

### Build (Windows)

The build is generated by [Sharpmake](https://github.com/ubisoft/Sharpmake). Build Sharpmake once, then generate the solutions:

```
Startup.bat                   :: copies the built Sharpmake binaries into sharpmakes\bin\
generateprojects.bat          :: generates the Visual Studio solutions
:: open dearwidgets_full.sln (or _apionly / _demoonly)
:: select a configuration: D3D11_Debug, OpenGL3_Release, D3D12_Release, ...
```

> **Run `generateprojects.bat` from PowerShell, not `cmd.exe`.** It passes the
> Sharpmake source list using PowerShell string syntax
> (`/sources( @'./sharpmakes/main.cs' )`), which `cmd.exe` does not parse the
> same way.

`Startup.bat` expects Sharpmake to have been built in Release first
(`extern/Sharpmake`, `dotnet build -c Release`).

The generated targets are **win64 only** today: D3D11, D3D12 and OpenGL3, in Debug and Release. There is no Linux or macOS generator wired up -- the corresponding `AddTargets` call in `sharpmakes/main.cs` is commented out. The library sources themselves are portable; only the build generation is Windows-specific.

### Integration

```cpp
#include "dear_widgets.h"

// Initialization -- after the graphics backend is up.
// With ImPlatform that is:
//   ImPlatform_CreateWindow(...); ImPlatform_InitGfxAPI(); ImPlatform_ShowWindow();
//   ImGui::CreateContext(); ... fonts ...
//   ImPlatform_InitPlatform(); ImPlatform_InitGfx();
ImWidgets::SetFeatures( ImWidgetsFeatures_RichFont | ImWidgetsFeatures_Markers );
ImWidgetsContext* ctx = ImWidgets::CreateContext();
ImWidgets::SetCurrentContext( ctx );

// Per-frame usage
ImGui::Begin( "Color Tool" );

ImVec4 color = { 1.0f, 0.5f, 0.0f, 1.0f };
ImWidgets::ColorWheel( "##wheel", &color );

float stops[ 2 ] = { 0.2f, 0.8f };
ImWidgets::SliderNFloat( "Range", stops, 2, 0.0f, 1.0f, 8.0f, true );

ImGui::End();

// Shutdown
ImWidgets::DestroyContext( ctx );
```

See [docs/api/context.md](docs/api/context.md) for the full initialization order.

---

## Feature Flags

Set before `CreateContext()`:

| Flag | Enables |
|---|---|
| *(none)* | All ImGui-only features + system textures |
| `ImWidgetsFeatures_Markers` | GPU shape markers |
| `ImWidgetsFeatures_RichFont` | Slug GPU text, color fonts, gradient text fills |
| `ImWidgetsFeatures_LaTeX` | LaTeX math rendering (implies `RichFont`) |

> **Not every shader is behind a flag.** `image_inspector.hlsl`,
> `volume_viewer.hlsl`, `lookdev_inspector.hlsl`, `delta_e_compare.hlsl` and
> `blur.hlsl` are compiled lazily on first use through `CreateInternalShader`
> and are **not** gated by any `ImWidgetsFeatures_` value -- `ImageInspector`,
> `VolumeViewer`, `LookDevInspector`, `ColorDifferenceImageViewer` and
> `SetCurrentWindowBlurBackground` work with no flag set, as long as the backend
> supports custom shaders. Call `ImWidgets::PrebuildShaders()` after
> `CreateContext()` to pay the compile cost up front.

---

## Documentation

Full API reference and concept guides are in [`docs/`](docs/README.md):

- [API Reference](docs/README.md#api-reference-api) -- all public functions grouped by category
- [Compatibility](docs/compatibility.md) -- which features need ImPlatform or a custom shader
- [Slug GPU Fonts](docs/concepts/slug.md) -- architecture and limitations
- [LaTeX Rendering](docs/concepts/latex.md) -- pipeline, box model, supported syntax
- [Color Spaces](docs/concepts/color-spaces.md) -- all spaces used by the library
- [Shader Cross-Compilation](docs/concepts/shaders.md) -- Slang / HLSL authoring and generation

---

## Project Structure

```
DearWidgets/
+-- src/
|   +-- api/
|   |   +-- dear_widgets.h                  # Umbrella public header (all public types + functions)
|   |   +-- dear_widgets.cpp                # Core implementation
|   |   +-- dear_widgets_internal.h         # Internal helpers -- NOT public API
|   |   +-- dear_widgets_slug.h             # 4-line stub marking the Slug TU
|   |   +-- dear_widgets_slug.cpp           # Slug GPU font renderer (the actual implementation)
|   |   +-- dear_widgets_latex.h/.cpp       # LaTeX math renderer (#included, not a separate TU)
|   |   +-- dear_widgets_stroke.cpp         # Euler-spiral stroke expansion, dashes
|   |   +-- dear_widgets_text_shape.h/.cpp  # Shaping front-end over kb_text_shape
|   |   +-- kb_text_shape.h                 # Vendored complex-script shaper (zlib)
|   |   +-- dear_widgets_image_inspector.cpp# GPU image inspector
|   |   +-- dear_widgets_network.h/.cpp     # ImNetworkGraph widget
|   |   +-- dear_widgets_timeline.h/.cpp    # ImTimeline widget
|   |   +-- dear_widgets_vector_drawing.h/.cpp  # VectorDrawingTool widget
|   |   +-- implatform_impl.cpp             # ImPlatform backend glue
|   +-- demo/
|       +-- demo.h                          # ShowDemo / ShowSamples entry points
|       +-- demo.cpp                        # The demo application
|       +-- font_downloader.h               # On-demand demo font downloader (C++17, STL)
|       +-- font_manifest.inl               # 72 demo fonts: name, category, source, URL
+-- extern/
|   +-- ImPlatform/                         # Rendering abstraction (DX9-DX12, GL, Vulkan) + bundled Dear ImGui
|   +-- imgui/                              # Dear ImGui submodule (the build uses ImPlatform's copy)
|   +-- stb/                                # stb_truetype (glyph data)
|   +-- CDT/                                # Constrained Delaunay triangulation (vendored, MPL-2.0)
|   +-- IconFontCppHeaders/                 # Icon-font C++ headers (zlib)
|   +-- FiraCode/                           # Bundled monospace font
|   +-- Sharpmake/                          # Build system
+-- workingdir/                             # Run the demo from here (assets are CWD-relative)
|   +-- fonts/                              # Downloaded demo fonts (git-ignored)
|   +-- latex_fonts/                        # latinmodern-math.otf (GUST Font License)
|   +-- fa-*.ttf                            # Font Awesome 6 Free (OFL 1.1)
|   +-- shaders/
|   |   +-- hlsl_src/                       # Authoritative HLSL shader sources
|   |   +-- glsl/ msl/ wgsl/ wgpu/          # Cross-compiled outputs
|   |   +-- bytecode_cache/                 # Compiled shader cache (git-ignored)
|   |   +-- slug.md                         # Internal Slug shader reference
|   +-- generate_shaders_all.bat            # Recompile all shaders via slangc
+-- sharpmakes/                             # Sharpmake build configuration
|   +-- main.cs  common.cs  APIProject.cs  DemoProject.cs
|   +-- bin/                                # Sharpmake binaries (populated by Startup.bat)
+-- docs/                                   # Documentation
+-- projects/                               # Generated project files (git-ignored)
```

`dear_widgets_latex.cpp` and `dear_widgets_slug.cpp` are `#include`d into other translation units rather than compiled standalone -- if you add them to a build system, keep them out of the compiled-source list.

---

## Dependencies

| Library | Role | Where |
|---|---|---|
| [Dear ImGui](https://github.com/ocornut/imgui) | UI framework | `extern/ImPlatform/imgui` (the build uses ImPlatform's copy) |
| [ImPlatform](https://github.com/soufianekhiat/ImPlatform) | Cross-platform graphics abstraction | `extern/ImPlatform` |
| [stb_truetype](https://github.com/nothings/stb) | TTF/OTF outline data for Slug | `extern/stb` |
| [CDT](https://github.com/artem-ogre/CDT) | Constrained Delaunay triangulation for glyph and shape tessellation | `extern/CDT` |
| [kb_text_shape](https://github.com/JimmyLefevre/kb) | Complex-script text shaping (Arabic, Hebrew, Indic) | `src/api/kb_text_shape.h` |
| [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders) | Icon-font codepoint headers | `extern/IconFontCppHeaders` |
| [FiraCode](https://github.com/tonsky/FiraCode) | Bundled monospace font | `extern/FiraCode` |
| [Sharpmake](https://github.com/ubisoft/Sharpmake) | Build system | `extern/Sharpmake` |

**Zero STL dependencies in the library core** -- `src/api/` uses no `std::vector`, `std::map` or other standard containers; `ImVector` is used throughout. The *demo* is not held to this: `src/demo/font_downloader.h` uses `<thread>`, `<mutex>`, `<string>`, `<vector>`, `<deque>` and `<filesystem>` (C++17).

---

## Third-party components

The project itself is CC0, but CC0 cannot cover code that arrives under another
licence. Each component below keeps its own terms; if you redistribute
DearWidgets, carry these with it.

| Component | Licence | Where | Notes |
|---|---|---|---|
| **CDT** | Mozilla Public License 2.0 | `extern/CDT/` (`LICENSE` in that folder) | Vendored sources, not a submodule. MPL-2.0 is file-level copyleft: modifications to the CDT files must stay MPL-2.0 and be made available. |
| **kb_text_shape** | zlib | `src/api/kb_text_shape.h` | (c) 2024-2025 Jimmy Lefevre. Single-header; licence banner near the bottom of the file. |
| **IconFontCppHeaders** | zlib | `extern/IconFontCppHeaders/` | (c) 2017 Juliette Foucaut and Doug Binks. |
| **stb_truetype** | MIT / public domain (dual) | `extern/stb/` | |
| **Dear ImGui** | MIT | `extern/ImPlatform/imgui/` | |
| **ImPlatform** | see its own repository | `extern/ImPlatform/` | |
| **Sharpmake** | Apache 2.0 | `extern/Sharpmake/` | Build-time only; not linked into any binary. |
| **FiraCode** | SIL Open Font License 1.1 | `extern/FiraCode/` | |
| **Font Awesome 6 Free** | SIL Open Font License 1.1 | `workingdir/fa-solid-900.ttf`, `fa-regular-400.ttf`, `fa-brands-400.ttf` | Shipped in the repository. |
| **Latin Modern Math** | GUST Font License | `workingdir/latex_fonts/latinmodern-math.otf` | Shipped in the repository; required by the LaTeX renderer. |

### Note on Slug

The GPU glyph-rendering approach used by `dear_widgets_slug.cpp` follows the
technique published by **Eric Lengyel / Terathon Software** (the "Slug"
algorithm: rendering glyphs directly from their Bezier outlines in the pixel
shader, with a per-glyph band acceleration structure). This is an independent
implementation -- no Terathon code is included -- but **the technique is covered
by patents held by Terathon Software**.

DearWidgets makes no claim about whether any particular use is or is not
licensed. If you intend to ship a product using this renderer, satisfy yourself
about the patent position for your jurisdiction and use case before you do.
See [terathon.com/slug](https://terathon.com/blog/slug-algorithm.html) for the
author's own description of the algorithm and licensing.

---

## Fonts

Two fonts are committed to this repository (Latin Modern Math and Font Awesome 6
Free, listed above), plus FiraCode as a submodule.

**The demo's 72 showcase fonts are not redistributed.** They are downloaded on
demand, at runtime, by the font panel in the demo (Draw > Text > GPU Text (Slug)),
driven by `src/demo/font_manifest.inl`, into `workingdir/fonts/` (git-ignored).
Nothing is downloaded unless you ask for it.

Sources, as recorded in the manifest's `source` field:

| Source | Count | Typical licence |
|---|---|---|
| Google Fonts | 35 | SIL OFL 1.1 or Apache 2.0 |
| Velvetyne | 10 | SIL OFL 1.1 |
| Fontshare | 9 | Fontshare / ITF Free Font Licence |
| 1001fonts | 7 | mixed -- see below |
| GitHub project releases (aliftype, Adobe, rastikerdar, psb1558, tetunori) | 6 | per project |
| OpenMoji | 2 | CC BY-SA 4.0 |
| Typodermic (via DaFont) | 2 | CC0 |
| Other (CC-BY-SA 4.0) | 1 | |

> **Check the licence before you redistribute a downloaded font.** A few
> 1001fonts entries (Aquaphonic, Cimero Pro, Color Tube, Multicolore Pro,
> Fattern) are "free for personal and commercial **use**", which is not an open
> licence and does not necessarily permit redistribution. The other two
> (Primecolor G and M) are SIL OFL. The manifest's `source` field
> records provenance, not permission.

---

## License

**CC0 1.0 Universal** -- Public Domain Dedication, for the DearWidgets sources in
`src/` and the documentation. Copy, modify, distribute and use for any purpose
without restriction. See [LICENSE](LICENSE).

The CC0 dedication does **not** extend to the third-party components listed
above, nor to any font downloaded by the demo. See
[Third-party components](#third-party-components).

## Credits

**Author**: Soufiane KHIAT
**Slug algorithm**: Eric Lengyel, Terathon Software (see the note above)
**Dear ImGui**: Omar Cornut and contributors
**CDT**: Artem Amirkhanov and contributors
**kb_text_shape**: Jimmy Lefevre
