# DearWidgets
DearWidgets aim to produce useful Widgets particulary useful on Graphics (Image Processing, 3D, ...).
DearWidgets aim to provide helper to simplify creation of custom widgets.

DearWidgets is 6 collections of helpers:
- DrawLists
- Interactions
- Widgets
- Math Helpers
- 'Shape' (2D Geometry)
- Helpers to have type independent "Scalar"

Dear Widgets is a collection of help to simplify the develoment of application.

DearWidgets add some helpers which allow us to create Custom Widget independently of the try. A Scalar is stored as an ImU64, which is a memcpy of any type supported by ImGui {Im{U|S}{8, 16, 32, 64} | bool | float | double}.
* bool	IsNegativeScalar
* void	EqualScalar
* float	ScalarToFloat
* ImU64	FloatToScalar
* ImU64	AddScalar
* ImU64	SubScalar
* ImU64	MulScalar
* ImU64	DivScalar
* ImU64	Normalize01
* ...

DearWidgets is using ImPlatform.

### Incentivise development:

[<img src="https://c5.patreon.com/external/logo/become_a_patron_button@2x.png" alt="Become a Patron" width="150"/>](https://www.patreon.com/SoufianeKHIAT)

https://www.patreon.com/SoufianeKHIAT

PR & Discussion are open.

## What's new?

### More linked to DearImGui styles:
![](https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/WithStyle.gif)

### Interactions

* Is Hovered

Convex
```cpp
bool IsPolyConvexContains( ImVec2* pts, int pts_count, ImVec2 p );
bool IsMouseHoveringPolyConvex( const ImVec2& r_min, const ImVec2& r_max, ImVec2* pts, int pts_count, bool clip = true );
bool ItemHoverablePolyConvex( const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/IsHoveredConvex.gif" alt="IsHoveredConvex" width="200px"/>

Concave
```cpp
bool IsPolyConcaveContains( ImVec2* pts, int pts_count, ImVec2 p );
bool IsMouseHoveringPolyConcave( const ImVec2& r_min, const ImVec2& r_max, ImVec2* pts, int pts_count, bool clip = true );
bool ItemHoverablePolyConcave( const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/IsHoveredConcave.gif" alt="IsHoveredConcave" width="200px"/>

With Hole (warning rely on 'thick' scanline)
```cpp
bool IsPolyWithHoleContains( ImVec2* pts, int pts_count, ImVec2 p, ImRect* p_bb = NULL, int gap = 1, int strokeWidth = 1 );
bool IsMouseHoveringPolyWithHole( const ImVec2& r_min, const ImVec2& r_max, ImVec2* pts, int pts_count, bool clip = true );
bool ItemHoverablePolyWithHole( const ImRect& bb, ImGuiID id, ImVec2* pts, int pts_count, ImGuiItemFlags item_flags );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/IsHoveredHole.gif" alt="IsHoveredHole" width="200px"/>

### Window

* Background
```cpp
void SetCurrentWindowBackgroundImage( ImTextureID id, ImVec2 imgSize, bool fixedSize = false, ImU32 col = IM_COL32( 255, 255, 255, 255 ) );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Background.png" alt="IsHoveredHole" width="300px"/>

### DrawList
* Triangle Pointer

Used internally for HueSelector
```cpp
void DrawTriangleCursor( ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, float thickness, ImU32 col );
void DrawTriangleCursorFilled( ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, ImU32 col );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/triangle_pointer.png" alt="triangle_pointer" width="300px"/>

* Signet Pointer
```cpp
void DrawSignetCursor( ImDrawList* pDrawList, ImVec2 targetPoint, float width, float height, float height_ratio, float align01, float angle, float thickness, ImU32 col );
void DrawSignetFilledCursor( ImDrawList* pDrawList, ImVec2 targetPoint, float width, float height, float height_ratio, float align01, float angle, ImU32 col );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/signet_pointer.png" alt="signet_pointer" width="300px"/>

* Hue Band
* Luminance Band
* Saturation Band

Used Internally to implement HueSelector.

```cpp
void DrawHueBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float alpha, float gamma, float offset );
void DrawHueBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float colorStartRGB[ 3 ], float alpha, float gamma );
void DrawLumianceBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma );
void DrawSaturationBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/color_band.png" alt="color_band" width="300px"/>

* Graduations

Linear Line
```cpp
void DrawLinearLineGraduation( ImDrawList* drawlist, ImVec2 start, ImVec2 end, float mainLineThickness, ImU32 mainCol,
int division0, float height0, float thickness0, float angle0, ImU32 col0,
int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u,
int division2 = -1, float height2 = -1.0f, float thickness2 = -1.0f, float angle2 = -1.0f, ImU32 col2 = 0u );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LinearLineGraduation.jpg" alt="LinearLineGraduation" width="300px"/>

Linear Circular
```cpp
void DrawLinearCircularGraduation( ImDrawList* drawlist, ImVec2 center, float radius, float start_angle, float end_angle, int num_segments,
float mainLineThickness, ImU32 mainCol,
int division0, float height0, float thickness0, float angle0, ImU32 col0,
int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u,
int division2 = -1, float height2 = -1.0f, float thickness2 = -1.0f, float angle2 = -1.0f, ImU32 col2 = 0u );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LinearCircularGraduation.jpg" alt="LinearCircularGraduation" width="300px"/>

Log Line
```cpp
void DrawLogLineGraduation( ImDrawList* drawlist, ImVec2 start, ImVec2 end,
float mainLineThickness, ImU32 mainCol,
int division0, float height0, float thickness0, float angle0, ImU32 col0,
int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LogLinearGraduation.jpg" alt="LogLinearGraduation" width="300px"/>

Log Circular
```cpp
void DrawLogCircularGraduation( ImDrawList* drawlist, ImVec2 center, float radius, float start_angle, float end_angle, int num_segments,
float mainLineThickness, ImU32 mainCol,
int division0, float height0, float thickness0, float angle0, ImU32 col0,
int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LogCircularGraduation.jpg" alt="LogCircularGraduation" width="300px"/>

* Shape

Tesselation:
```cpp
#ifdef DEAR_WIDGETS_TESSELATION
	void	ShapeTesselationUniform( ImShape& shape );
#endif
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Shape.gif" alt="Shape" width="300px"/>

```cpp
void DrawImageShape( ImDrawList* pDrawList, ImTextureID tex, ImShape& shape );
```
Convex
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ImageConvex.png" alt="ImageConvex" width="300px"/>

Concave
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ImageConcave.png" alt="ImageConcave" width="300px"/>

** Gradients

Linear
```cpp
void	ShapeLinearGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/LinearGradient.png" alt="LinearGradient" width="300px"/>

Radial
```cpp
void	ShapeRadialGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/CircularGraduation.png" alt="CircularGraduation" width="300px"/>

Diamond
```cpp
void	ShapeDiamondGradient( ImShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/DiamondGradient.png" alt="DiamondGradient" width="300px"/>

With Hole
// TODO

* Color Ring

TODO: Ring HueSelector

TODO: Add support for 2D (angle, radius)

```cpp
void DrawColorRing( ImDrawList* pDrawList, ImVec2 const curPos, ImVec2 const size, float thickness_, ImColor1DCallback func, void* pUserData, int division, float colorOffset, bool bIsBilinear );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/GQLfC3C7Jk.gif" alt="DrawColorRing" width="300px"/>

* Custom Color Ring

<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Kt4ye6FDWq.gif" alt="DrawColorRing" width="300px"/>

* Chromatic Plot{Bilinear, Nearest}
    * Chromatic Point
    * Chromatic Line

```cpp
void DrawChromaticityPlot( ... );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/chromaticityplot_0.png" alt="chromaticityplot_0" width="300px"/>

<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/chromaticityplot_1.png" alt="chromaticityplot_1" width="300px"/>

```cpp
void DrawChromaticityPoints( ... );
void DrawChromaticityLines( ... );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/chromaticityline_0.png" alt="chromaticityline_0" width="300px"/>

* DrawColorDensityPlot (aka ShaderToy)

Use carefully that can have impact on your performances for HighRes canvas or/and expensive lambda.

```cpp
void DrawProceduralColor2DNearest( ImDrawList* pDrawList, ImColor2DCallback func, void* pUserData, float minX, float maxX, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );
void DrawProceduralColor2DBilinear( ImDrawList* pDrawList, ImColor2DCallback func, void* pUserData, float minX, float maxX, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/us8Fc2jkIh.png" alt="DrawProceduralColor2DBilinear" width="300px"/>

<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/yEGBSzv2F8.gif" alt="DrawProceduralColor2DBilinear" width="512px"/>

### Widgets

* Button

Convex
```cpp
bool ButtonBehaviorConvex( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ButtonConvex.gif" alt="ButtonConvex" width="300px"/>

Concave
```cpp
bool ButtonBehaviorConcave( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ButtonConcave.gif" alt="ButtonBehaviorConcave" width="300px"/>

With Hole
```cpp
bool ButtonBehaviorWithHole( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/ButtonHole.gif" alt="ButtonHole" width="300px"/>

* Hue Selector

```cpp
bool HueSelector( char const* label, float hueHeight, float cursorHeight, float* hueCenter, float* hueWidth, float* featherLeft, float* featherRight, int division = 32, float alpha = 1.0f, float hideHueAlpha = 0.75f, float offset = 0.0f );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/W0Q9VXNeGK.gif" alt="HueSelector"/>

* Slider 2D Float
A version for Slider2DScaler is available for (Im{S|U}{8,16,32,64}, Float and Double)

```cpp
bool Slider2DScalar( char const* pLabel, ImGuiDataType data_type, void* pValueX, void* pValueY, void* p_minX, void* p_maxX, void* p_minY, void* p_maxY );
bool Slider2DFloat( char const* pLabel, float* pValueX, float* pValueY, float v_minX, float v_maxX, float v_minY, float v_maxY );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Slider2DFloat.gif" alt="Slider2DFloat"/>

* Slider 2D Int

```cpp
bool Slider2DInt( char const* pLabel, int* pValueX, void* pValueY, int v_minX, int v_maxX, int v_minY, int v_maxY );
```
<img src="https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/Slider2DInt.gif" alt="Slider2DInt"/>

* SliderN
```cpp
bool SliderNScalar( char const* label, ImGuiDataType data_type, void* ordered_value, int value_count, void* p_min, void* p_max, float cursor_width, bool show_hover_by_region );
bool SliderNFloat( char const* label, ImGuiDataType data_type, float* ordered_value, int value_count, float v_min, float v_max, float cursor_width, bool show_hover_by_region );
bool SliderNInt( char const* label, ImGuiDataType data_type, int* ordered_value, int value_count, int v_min, int v_max, float cursor_width, bool show_hover_by_region );
```
![](https://github.com/soufianekhiat/DearWidgetsImages/raw/main/Images/SliderN.gif)

## Performance Considerations
Notice some DrawList are purely calling DrawList from Dear ImGui. So based on your parameters a non-negligeable impact on performance can be notice. Do not put an unreasonable resolution.

The Gradient rely on the vertex blending with the default shader. So to have a smooth gradient it may require lot of vertices via tesselation, that may impact performance and limit of vertices and imply a use of ImDrawIdx in 32 bits.

## Constrains
C++ features (optionaly) **used** internally:
* std::map<T, K> used for ImTesselator can be enabled with #define DEAR_WIDGETS_TESSELATION

## Legacy
**Removed** feature:
* AnalyticalPlotEx
* AnalyticalPlot Just explode the number of vertices
* CenterNextItem
* DragFloatLog TBD
* RangeSelect2D didn't find a proper control
* Slider3D no real use case

**Removed** C++ feature dependencies:
* template
* constexpr
* if constexpr
* Lambda from template
* auto
* std::vector&lt;float&gt;, std::vector&lt;bool&gt; for isoline

## Contributor
Future feature:
* Hue Ring select
* Slider2DWithRingConstraint // Only only a circular region on the 2D selector
* InputFloatUnit cf. History of Dear Widgets
* Add Shape from SDF with ImShader cd ImPlatform (only if IM_SUPPORT_CUSTOM_SHADER)
* Guizmo2D{Translate, Rotate, Scale}
* Gauge
