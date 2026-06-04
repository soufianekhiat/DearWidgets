# DearWidgets

Advanced widgets and rendering utilities for [Dear ImGui](https://github.com/ocornut/imgui), built for graphics applications: color grading tools, image processing, 3D visualization, scientific plotting, and rich-text rendering.

![DearWidgets Style Integration](https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/WithStyle.gif)

---

## What's Inside

### Interactive Widgets
- **Slider2D / SliderN / SliderRing / SliderSpline** -- multi-dimensional and shaped sliders
- **SliderGradient / SliderGradientRing / SliderSplineGradient** -- gradient-painted sliders (single handle) with optional fill-up-to-cursor
- **SliderGradientRange / SliderGradientRingRange / SliderSplineGradientRange** -- two-handle range sliders with gradient painted between handles
- **GradientEditor** -- multi-stop gradient with color space interpolation (sRGB, OkLab, OkLCH, HSV, Linear sRGB)
- **CurveEditor / ColorCurve / ToneCurve** -- spline curve editors with Catmull-Rom and easing support
- **ColorWheel / PrimariesWheel / HDRWheel** -- perceptual color selectors in HSV, OkLCH, and HDR modes
- **ColorPicker** -- unified picker with per-space variants: sRGB, HSV, OkLab, OkLCH, CIELab, XYZ
- **ColorWarper** -- 2D hue/saturation warp grid in HSV, OkLab, OkLCH, and more
- **HueSelector** -- hue band picker with feathering
- **VectorDrawingTool** -- bezier path authoring canvas with zoom/pan; supports polyline, anti-aliased polyline, Euler-spiral stroke, and dashed variants with per-path cap/join
- **UnitField / UpVector / DragFloatPrecise** -- specialized numeric inputs
- **ImageViewer** -- pan/zoom viewer with optional pixel inspector
- **PaintCanvas** -- pixel-level paint surface backed by a GPU texture
- **ImageCarousel / ImageBento / ImageTransformGizmo** -- image layout and transform tools
- **ParadeScope / VectorScope / Histogram / CIEChromaticity** -- color analysis scopes

### DrawList Extensions
- **Shape system** -- generate, fill (linear/radial/diamond gradients in any color space), and draw arbitrary 2D shapes with holes
- **GPU markers** -- resolution-independent shape markers for plots
- **Dashed polylines** -- CPU or GPU anti-aliased dashed lines; `gap=0` = flush, `gap<0` = overlapping envelopes
- **Spline gradient strokes** -- draw a multi-stop gradient along a Bezier spline (`DrawSplineGradient`, `DrawSplineGradientCut`)
- **Color bands and rings** -- hue, saturation, luminance, and arbitrary callback-driven rings
- **OkLab / OkLCH quads** -- perceptually uniform gradient fills for rectangular regions
- **Procedural color** -- ShaderToy-style 1D/2D CPU-evaluated color functions (horizontal, vertical, arc, spline variants)
- **Chromaticity plots** -- CIE 1931/1964 gamut diagrams with illuminants, primaries, and observer support
- **Graduation lines** -- linear and circular scales, linear and logarithmic
- **Custom-shape buttons** -- convex, concave, and holed polygon buttons
- **Window background** -- set a texture as the current window background

### GPU Font Rendering (Slug)
- **DrawText / CalcTextSize** -- resolution-independent glyph rendering from Bezier outlines (no atlas, no SDF)
- **Color fonts** -- full COLR v0/v1 support (color layers, gradients)
- **Gradient and image text fills** -- tessellated outline fills with arbitrary gradients (CPU and GPU paths)
- **GPU gradient text** -- `DrawLinearGradientTextGPU`, `DrawRadialGradientTextGPU`, `DrawDiamondGradientTextGPU`, `DrawImageTextGPU`; per-glyph or per-string, with color-space selection
- **Contour extraction** -- `TesselateTextPerGlyph`, `ExtractTextContours` for downstream use of glyph geometry
- **Arabic / Hebrew shaping** -- bidirectional text via `kb_text_shape`

### LaTeX Math Rendering
- **DrawLaTeX / CalcLaTeXSize** -- self-contained math typesetter using Latin Modern Math
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

Most widgets and all DrawList extensions are **ImGui only**. The shader tier is required only for GPU markers, Slug font rendering, LaTeX, and GPU dashed lines.

See [docs/compatibility.md](docs/compatibility.md) for the full per-feature table.

---

## Getting Started

### Prerequisites

- C++11 or later (MSVC 2019+, GCC, Clang)
- Dear ImGui (bundled via the ImPlatform submodule)
- ImPlatform (included as submodule) -- optional for ImGui-only usage

### Clone

```bash
git clone --recursive https://github.com/soufianekhiat/DearWidgets.git
cd DearWidgets
```

### Build (Windows)

```bash
generateprojects.bat          # Sharpmake generates Visual Studio solutions
# open projects/win64/*.sln
# select a configuration: D3D11_Debug, OpenGL3_Release, Vulkan_Release, etc.
```

### Build (Linux)

```bash
cd sharpmakes/linux
./build.sh                    # requires Mono for Sharpmake
```

### Integration

```cpp
#include "dear_widgets.h"

// Initialization (after ImGui::CreateContext and ImPlatform::InitGfx)
ImWidgets::SetFeatures(ImWidgetsFeatures_RichFont | ImWidgetsFeatures_Markers);
ImWidgetsContext* ctx = ImWidgets::CreateContext();
ImWidgets::SetCurrentContext(ctx);

// Per-frame usage
ImGui::Begin("Color Tool");

ImVec4 color = { 1.0f, 0.5f, 0.0f, 1.0f };
ImWidgets::ColorWheel("##wheel", &color);

float stops[2] = { 0.2f, 0.8f };
ImWidgets::SliderNFloat("Range", stops, 2, 0.0f, 1.0f, 8.0f, true);

ImGui::End();

// Shutdown
ImWidgets::DestroyContext(ctx);
```

---

## Feature Flags

Set before `CreateContext()`:

| Flag | Enables |
|---|---|
| *(none)* | All ImGui-only features + system textures |
| `ImWidgetsFeatures_Markers` | GPU shape markers |
| `ImWidgetsFeatures_RichFont` | Slug GPU text, color fonts, gradient text fills |
| `ImWidgetsFeatures_LaTeX` | LaTeX math rendering (implies `RichFont`) |

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
|   |   +-- dear_widgets.h/cpp          # Core library
|   |   +-- dear_widgets_slug.h/cpp     # Slug GPU font renderer
|   |   +-- dear_widgets_latex.h/cpp    # LaTeX math renderer
|   |   +-- implatform_impl.cpp         # ImPlatform backend glue
|   +-- demo/
|       +-- demo.cpp
+-- extern/
|   +-- ImPlatform/                     # Rendering abstraction (DX9-DX12, GL, Vulkan)
|   +-- imgui/                          # Dear ImGui
|   +-- stb/                            # stb_truetype (glyph data)
|   +-- FiraCode/                       # Bundled font
|   +-- Sharpmake/                      # Build system
+-- workingdir/
|   +-- shaders/
|   |   +-- hlsl_src/                   # Authoritative HLSL shader sources
|   |   +-- glsl/ msl/ wgsl/ wgpu/      # Cross-compiled outputs
|   |   +-- slug.md                     # Internal Slug shader reference
|   +-- generate_shaders_all.bat        # Recompile all shaders via slangc
+-- sharpmakes/                         # Sharpmake build configuration
+-- docs/                               # Documentation
+-- projects/                           # Generated project files (git-ignored)
```

---

## Dependencies

| Library | Role |
|---|---|
| [Dear ImGui](https://github.com/ocornut/imgui) | UI framework |
| [ImPlatform](extern/ImPlatform) | Cross-platform graphics abstraction |
| [stb_truetype](https://github.com/nothings/stb) | TTF outline data for Slug |
| [FiraCode](extern/FiraCode) | Bundled monospace font |
| [Sharpmake](extern/Sharpmake) | Build system |

Zero STL dependencies -- no `std::vector`, `std::map`, or other standard containers.

---

## License

**CC0 1.0 Universal** -- Public Domain Dedication.
Copy, modify, distribute, and use for any purpose without restriction. See [LICENSE](LICENSE).

## Credits

**Author**: Soufiane KHIAT
**Slug algorithm**: Eric Lengyel, Terathon Software
**Dear ImGui**: Omar Cornut and contributors
