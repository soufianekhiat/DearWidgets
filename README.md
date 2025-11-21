# DearWidgets

A comprehensive collection of advanced widgets and utilities for [Dear ImGui](https://github.com/ocornut/imgui), particularly designed for graphics applications (image processing, 3D visualization, color science, and more).

![DearWidgets Style Integration](https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/WithStyle.gif)

## Overview

DearWidgets extends Dear ImGui with powerful, production-ready widgets and utilities that simplify the development of advanced graphical interfaces. The library provides six core collections:

- **DrawList Extensions** - Advanced rendering primitives (gradients, color rings, procedural plots, custom shapes)
- **Interaction Helpers** - Polygon hit-testing (convex, concave, with holes)
- **Custom Widgets** - Specialized UI controls (2D sliders, hue selectors, multi-sliders)
- **Math Helpers** - Type-independent scalar operations
- **Shape System** - 2D geometry manipulation with tesselation support
- **Scalar Abstraction** - Generic scalar operations supporting all ImGui data types

## Key Features

### 🎨 Advanced Color & Graphics Tools
- Multiple color space gradients (sRGB, OkLab, OkLch, HSV)
- Chromaticity plots with CIE diagrams
- Customizable hue/saturation/luminance selectors
- Procedural color generation (ShaderToy-like)

### 🖱️ Sophisticated Interactions
- Polygon-based hit testing for arbitrary shapes
- Custom button behaviors for non-rectangular regions
- Multi-dimensional sliders (2D, N-dimensional)
- Advanced hover detection

### 📊 Visualization Components
- Graduated scales (linear and logarithmic)
- Color wheels and rings with customizable divisions
- Shape rendering with GPU-accelerated custom shaders

### 🔧 Developer-Friendly
- Type-safe scalar operations across all ImGui numeric types
- **Zero STL dependencies** - completely container-free
- ImPlatform integration for cross-platform rendering
- C++11 compatible with minimal language features

## Getting Started

### Prerequisites

- **C++ Compiler**: C++11 or later (MSVC 2019+, GCC, Clang)
- **Dear ImGui**: Bundled via ImPlatform submodule
- **ImPlatform**: Included as submodule (cross-platform rendering abstraction)
- **Sharpmake**: For project generation (included)

### Supported Platforms & Graphics APIs

| Platform | Graphics APIs |
|----------|--------------|
| Windows  | DirectX 9/10/11/12, OpenGL 3, Vulkan |
| Linux    | OpenGL 3, Vulkan |
| macOS    | OpenGL 3, Metal (planned) |

### Building on Windows

1. **Clone with submodules:**
   ```bash
   git clone --recursive https://github.com/soufianekhiat/DearWidgets.git
   cd DearWidgets
   ```

2. **Generate project files:**
   ```bash
   generateprojects.bat
   ```
   This uses Sharpmake to generate Visual Studio solutions for all configurations.

3. **Build:**
   - Open `dearwidgets_full.sln` in Visual Studio 2019/2022
   - Select configuration (e.g., `D3D11_Debug` or `OpenGL3_Release`)
   - Build solution (F7)

4. **Run the demo:**
   ```bash
   cd WorkingDir
   dearwidgetsdemo.exe
   ```

### Building on Linux

```bash
git clone --recursive https://github.com/soufianekhiat/DearWidgets.git
cd DearWidgets
./generateprojects.sh  # Requires Mono for Sharpmake
# Build with make or open generated projects
```

### Integration

Add to your ImGui project:

```cpp
#include "dear_widgets.h"

// In your render loop:
ImGui::Begin("My Window");

// Use widgets
float value_x = 0.5f, value_y = 0.5f;
ImWidgets::Slider2DFloat("Position", &value_x, &value_y, 0.0f, 1.0f, 0.0f, 1.0f);

ImGui::End();
```

## Core Features

### 1. Interaction Helpers

Precise hit-testing for complex shapes:

#### Convex Polygons
```cpp
bool IsPolyConvexContains(ImVec2* pts, int pts_count, ImVec2 p);
bool IsMouseHoveringPolyConvex(const ImVec2& r_min, const ImVec2& r_max, ImVec2* pts, int pts_count, bool clip = true);
bool ItemHoverablePolyConvex(const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/IsHoveredConvex.gif" alt="Convex hit-testing" width="200"/>

#### Concave Polygons
```cpp
bool IsPolyConcaveContains(ImVec2* pts, int pts_count, ImVec2 p);
bool IsMouseHoveringPolyConcave(const ImVec2& r_min, const ImVec2& r_max, ImVec2* pts, int pts_count, bool clip = true);
bool ItemHoverablePolyConcave(const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/IsHoveredConcave.gif" alt="Concave hit-testing" width="200"/>

#### Polygons with Holes
```cpp
bool IsPolyWithHoleContains(ImVec2* pts, int pts_count, ImVec2 p, ImRect* p_bb = NULL, int gap = 1, int strokeWidth = 1);
bool ItemHoverablePolyWithHole(const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/IsHoveredHole.gif" alt="Polygon with holes" width="200"/>

### 2. Window Enhancements

#### Background Images
```cpp
void SetCurrentWindowBackgroundImage(ImTextureID id, ImVec2 imgSize, bool fixedSize = false, ImU32 col = IM_COL32(255, 255, 255, 255));
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Background.png" alt="Window background" width="300"/>

### 3. DrawList Extensions

#### Custom Cursors

**Triangle Cursor** (used internally by HueSelector):
```cpp
void DrawTriangleCursor(ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, float thickness, ImU32 col);
void DrawTriangleCursorFilled(ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, ImU32 col);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/triangle_pointer.png" alt="Triangle cursor" width="300"/>

**Signet Cursor**:
```cpp
void DrawSignetCursor(ImDrawList* pDrawList, ImVec2 targetPoint, float width, float height, float height_ratio, float align01, float angle, float thickness, ImU32 col);
void DrawSignetFilledCursor(ImDrawList* pDrawList, ImVec2 targetPoint, float width, float height, float height_ratio, float align01, float angle, ImU32 col);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/signet_pointer.png" alt="Signet cursor" width="300"/>

#### Color Bands

For color pickers and selectors:
```cpp
void DrawHueBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float alpha, float gamma, float offset);
void DrawLumianceBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma);
void DrawSaturationBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/color_band.png" alt="Color bands" width="300"/>

#### Graduated Scales

**Linear Line Graduations**:
```cpp
void DrawLinearLineGraduation(ImDrawList* drawlist, ImVec2 start, ImVec2 end,
    float mainLineThickness, ImU32 mainCol,
    int division0, float height0, float thickness0, float angle0, ImU32 col0,
    int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u,
    int division2 = -1, float height2 = -1.0f, float thickness2 = -1.0f, float angle2 = -1.0f, ImU32 col2 = 0u);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LinearLineGraduation.jpg" alt="Linear line graduation" width="300"/>

**Circular Graduations**:
```cpp
void DrawLinearCircularGraduation(ImDrawList* drawlist, ImVec2 center, float radius, float start_angle, float end_angle, int num_segments,
    float mainLineThickness, ImU32 mainCol,
    int division0, float height0, float thickness0, float angle0, ImU32 col0,
    ...);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LinearCircularGraduation.jpg" alt="Circular graduation" width="300"/>

**Logarithmic Scales**:
```cpp
void DrawLogLineGraduation(ImDrawList* drawlist, ImVec2 start, ImVec2 end, ...);
void DrawLogCircularGraduation(ImDrawList* drawlist, ImVec2 center, float radius, ...);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LogLinearGraduation.jpg" alt="Log linear" width="300"/>
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LogCircularGraduation.jpg" alt="Log circular" width="300"/>

### 4. Shape System

#### Tessellation Support
```cpp
#ifdef DEAR_WIDGETS_TESSELATION
void ShapeTesselationUniform(ImShape& shape);
#endif
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Shape.gif" alt="Shape tessellation" width="300"/>

#### Image-Filled Shapes
```cpp
void DrawImageShape(ImDrawList* pDrawList, ImTextureID tex, ImShape& shape);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ImageConvex.png" alt="Image convex" width="300"/>
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ImageConcave.png" alt="Image concave" width="300"/>

#### Multi-Colorspace Gradients

**Linear Gradients**:
```cpp
void ShapeSRGBLinearGradient(ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeOkLabLinearGradient(ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeOkLchLinearGradient(ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeLinearSRGBLinearGradient(ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeHSVLinearGradient(ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LinearGradient.png" alt="Linear gradients" width="300"/>
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/GradientColorSpace.png" alt="Gradient color spaces" width="200"/>

**Radial Gradients**:
```cpp
void ShapeSRGBRadialGradient(ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
void ShapeOkLabRadialGradient(ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
// ... + OkLch, LinearSRGB, HSV variants
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/CircularGraduation.png" alt="Radial gradients" width="300"/>

**Diamond Gradients**:
```cpp
void ShapeSRGBDiamondGradient(ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1);
// ... + OkLab, OkLch, LinearSRGB, HSV variants
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/DiamondGradient.png" alt="Diamond gradients" width="300"/>

#### Color Ring
```cpp
void DrawColorRing(ImDrawList* pDrawList, ImVec2 const curPos, ImVec2 const size,
    float thickness_, ImColor1DCallback func, void* pUserData,
    int division, float colorOffset, bool bIsBilinear);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/GQLfC3C7Jk.gif" alt="Color ring" width="300"/>
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Kt4ye6FDWq.gif" alt="Custom color ring" width="300"/>

#### Chromaticity Diagrams
```cpp
void DrawChromaticityPlot(...);  // CIE chromaticity diagrams
void DrawChromaticityPoints(...);
void DrawChromaticityLines(...);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/chromaticityplot_0.png" alt="Chromaticity plot" width="300"/>
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/chromaticityplot_1.png" alt="Chromaticity plot detail" width="300"/>
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/chromaticityline_0.png" alt="Chromaticity lines" width="300"/>

#### Procedural Color Plots
ShaderToy-like procedural rendering (use carefully - can impact performance):
```cpp
void DrawProceduralColor2DNearest(ImDrawList* pDrawList, ImColor2DCallback func, void* pUserData,
    float minX, float maxX, float minY, float maxY,
    ImVec2 position, ImVec2 size, int resolutionX, int resolutionY);

void DrawProceduralColor2DBilinear(ImDrawList* pDrawList, ImColor2DCallback func, void* pUserData,
    float minX, float maxX, float minY, float maxY,
    ImVec2 position, ImVec2 size, int resolutionX, int resolutionY);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/us8Fc2jkIh.png" alt="Procedural plot" width="300"/>
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/yEGBSzv2F8.gif" alt="Procedural animation" width="512"/>

### 5. Custom Widgets

#### Polygon Buttons

**Convex Buttons**:
```cpp
bool ButtonBehaviorConvex(ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ButtonConvex.gif" alt="Convex button" width="300"/>

**Concave Buttons**:
```cpp
bool ButtonBehaviorConcave(ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ButtonConcave.gif" alt="Concave button" width="300"/>

**Buttons with Holes**:
```cpp
bool ButtonBehaviorWithHole(ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ButtonHole.gif" alt="Button with hole" width="300"/>

#### Hue Selector
```cpp
bool HueSelector(char const* label, float hueHeight, float cursorHeight,
    float* hueCenter, float* hueWidth, float* featherLeft, float* featherRight,
    int division = 32, float alpha = 1.0f, float hideHueAlpha = 0.75f, float offset = 0.0f);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/W0Q9VXNeGK.gif" alt="Hue selector" width="512"/>

#### 2D Sliders

**Float Sliders** (also supports all ImGui scalar types):
```cpp
bool Slider2DScalar(char const* pLabel, ImGuiDataType data_type,
    void* pValueX, void* pValueY, void* p_minX, void* p_maxX, void* p_minY, void* p_maxY);

bool Slider2DFloat(char const* pLabel, float* pValueX, float* pValueY,
    float v_minX, float v_maxX, float v_minY, float v_maxY);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Slider2DFloat.gif" alt="2D float slider" width="512"/>

**Integer Sliders**:
```cpp
bool Slider2DInt(char const* pLabel, int* pValueX, void* pValueY,
    int v_minX, int v_maxX, int v_minY, int v_maxY);
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Slider2DInt.gif" alt="2D int slider" width="512"/>

#### Multi-Sliders (N-dimensional)
```cpp
bool SliderNScalar(char const* label, ImGuiDataType data_type, void* ordered_value, int value_count,
    void* p_min, void* p_max, float cursor_width, bool show_hover_by_region);

bool SliderNFloat(char const* label, ImGuiDataType data_type, float* ordered_value, int value_count,
    float v_min, float v_max, float cursor_width, bool show_hover_by_region);

bool SliderNInt(char const* label, ImGuiDataType data_type, int* ordered_value, int value_count,
    int v_min, int v_max, float cursor_width, bool show_hover_by_region);
```
![SliderN widget](https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/SliderN.gif)

### 6. Scalar Abstraction

Type-independent scalar operations for all ImGui data types (`ImS8`, `ImU8`, `ImS16`, `ImU16`, `ImS32`, `ImU32`, `ImS64`, `ImU64`, `bool`, `float`, `double`):

```cpp
bool IsNegativeScalar(ImGuiDataType data_type, ImU64 scalar);
void EqualScalar(ImGuiDataType data_type, ImU64* dest, ImU64 src);
float ScalarToFloat(ImGuiDataType data_type, ImU64 scalar);
ImU64 FloatToScalar(ImGuiDataType data_type, float value);
ImU64 AddScalar(ImGuiDataType data_type, ImU64 a, ImU64 b);
ImU64 SubScalar(ImGuiDataType data_type, ImU64 a, ImU64 b);
ImU64 MulScalar(ImGuiDataType data_type, ImU64 a, ImU64 b);
ImU64 DivScalar(ImGuiDataType data_type, ImU64 a, ImU64 b);
ImU64 Normalize01(ImGuiDataType data_type, ImU64 value, ImU64 min, ImU64 max);
// ... and more
```

Scalars are stored as `ImU64` using `memcpy` for type punning, enabling generic widget implementations.

## Configuration

### Build Configurations

DearWidgets uses a configuration naming scheme: `{GraphicsAPI}_{BuildType}`

Examples:
- `D3D11_Debug` - DirectX 11 Debug build
- `OpenGL3_Release` - OpenGL 3 Release build
- `Vulkan_Release` - Vulkan Release build

### Compile-Time Options

Define in your project or `dear_widgets.h`:

```cpp
// Enable tessellation support (completely STL-free)
#define DEAR_WIDGETS_TESSELATION

// Set ImDrawIdx to 32-bit for large vertex buffers (required for complex gradients)
#define ImDrawIdx unsigned int
```

## Performance Considerations

### DrawList Performance
- Many DrawList functions directly call Dear ImGui primitives - high resolution settings can impact performance
- Avoid excessive divisions/resolution in procedural plots (`DrawProceduralColor2D*`)
- Use Nearest sampling when bilinear filtering isn't needed

### Gradient Performance
- Gradients rely on vertex color blending, requiring many vertices for smoothness
- Tessellation increases vertex count significantly - may require 32-bit `ImDrawIdx`
- Consider using GPU shaders (via ImPlatform) for complex gradients

### Recommendations
- Profile before optimizing - many operations are already fast
- For real-time applications, cache procedural plots when possible
- Reduce graduation divisions for less critical visual elements

## Dependencies

### Required
- **Dear ImGui** (via ImPlatform submodule)
- **ImPlatform** (included) - cross-platform rendering abstraction

### Zero STL Dependencies
DearWidgets is **completely STL-free** - no standard library containers or features are required!

### Removed C++ Feature Dependencies
DearWidgets has been refactored to minimize C++ requirements and is compatible with C++11:
- ❌ Templates
- ❌ `constexpr` / `if constexpr`
- ❌ Template lambdas
- ❌ `auto` keyword
- ❌ `std::vector`
- ❌ `std::map`
- ❌ `std::pair`
- ❌ Range-based for loops with `auto`

## Project Structure

```
DearWidgets/
├── src/
│   ├── api/              # Core library (dear_widgets.h/.cpp)
│   │   ├── dear_widgets.h
│   │   ├── dear_widgets.cpp
│   │   └── implatform_impl.cpp  # ImPlatform implementation
│   └── demo/             # Demo application
│       └── demo.cpp
├── extern/
│   ├── ImPlatform/       # Rendering abstraction
│   ├── imgui/            # Dear ImGui (via ImPlatform)
│   └── Sharpmake/        # Project generator
├── sharpmakes/           # Build configuration
│   ├── APIProject.cs
│   ├── DemoProject.cs
│   └── common.cs
├── WorkingDir/           # Runtime directory
│   └── shaders/          # HLSL/GLSL shaders
└── projects/             # Generated project files
```

## Support & Contributing

### Incentivize Development

Support continued development:

[<img src="https://c5.patreon.com/external/logo/become_a_patron_button@2x.png" alt="Become a Patron" width="150"/>](https://www.patreon.com/SoufianeKHIAT)

https://www.patreon.com/SoufianeKHIAT

### Contributing

Pull requests and discussions are welcome! Areas for contribution:

#### Planned Features
- Additional color spaces for gradients (LAB, LCH variants)
- Hue ring selector (circular hue picker)
- Slider2DWithRingConstraint (circular 2D region constraints)
- InputFloatUnit (unit-aware numeric inputs)
- SDF-based shapes with custom shaders (via ImPlatform)
- 2D Guizmo widgets (translate, rotate, scale)
- Gauge widgets

#### Legacy Features (Removed)
The following were removed for maintenance reasons:
- `AnalyticalPlot` / `AnalyticalPlotEx` - excessive vertex generation
- `CenterNextItem` - limited use case
- `DragFloatLog` - TBD for reimplementation
- `RangeSelect2D` - no proper control paradigm found
- `Slider3D` - no real-world use cases identified

### Reporting Issues

Please report bugs with:
- Platform and graphics API
- Build configuration
- Minimal reproduction code
- Screenshots/videos if applicable

## License

**CC0 1.0 Universal** - Public Domain Dedication

This work has been dedicated to the public domain under the CC0 1.0 Universal license. You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission. See [LICENSE](LICENSE) for details.

## Credits

**Author**: Soufiane KHIAT

**ImGui**: Omar Cornut and contributors

**ImPlatform**: Included rendering abstraction layer

## Gallery

For more examples and visual demonstrations, see the [DearWidgets Image Repository](https://github.com/soufianekhiat/DearWidgetsImages).

---

*DearWidgets - Advanced widgets for advanced applications*
