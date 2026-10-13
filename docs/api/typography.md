# Typography -- Slug GPU Text Rendering

> **Requires:** `ImWidgetsFeatures_RichFont` set before `CreateContext()`.
> A custom-shader backend (`IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER`) is mandatory
> for the GPU paths.

Slug renders text directly from TrueType/OpenType bezier outlines on the GPU --
no bitmap atlas, no SDF baking. Results are crisp at any scale, zoom or viewing
angle. Color fonts (COLR v0/v1) and gradient fills are supported natively.
Complex-script shaping (Arabic, Hebrew, Indic) is provided by the vendored
`kb_text_shape`.

See [concepts/slug.md](../concepts/slug.md) for architecture details.

> `dear_widgets.h` `#undef`s the Win32 `DrawText` macro so `ImWidgets::DrawText`
> resolves. If you include `<windows.h>` *after* DearWidgets the macro comes
> back -- include DearWidgets last, or `#undef DrawText` again.

---

## Font Loading

### `GetSlugFontLoader`
```cpp
ImFontLoader const* GetSlugFontLoader();
```
An `ImFontLoader` backend that rasterizes Slug glyphs (including colour and
gradient layers) into ImGui's bitmap atlas, for widgets that still use the CPU
rasterizer.
```cpp
ImFontConfig cfg;
cfg.FontLoader = ImWidgets::GetSlugFontLoader();
io.Fonts->AddFontFromFileTTF( "myfont.ttf", 16.0f, &cfg );
```

### `GetSlugFontInfo`
```cpp
bool GetSlugFontInfo( ImFont* font, void* outStbttFontInfo, float* outEmScale );
```
Retrieves the underlying `stbtt_fontinfo` (as `void*`) and em-scale for a loaded
font. Advanced use only.

### `SlugBuildGlyphByID`
```cpp
void SlugBuildGlyphByID( ImFont* font, int glyphID );
```
Force-build the Slug geometry for one glyph by **glyph ID** (not codepoint).

### `PrewarmTessellationCache`
```cpp
void PrewarmTessellationCache( ImFont* font, float tess_tol = 0.0f );
```
Tessellate every glyph in `font` up front so the first frame that draws text
does not stall. Parallelised internally; call it once after loading a font that
will be used for tessellated fills.

---

## Drawing

### `DrawText`
```cpp
void DrawText( ImDrawList* pDrawList, ImVec2 pos, ImU32 col,
               char const* text, char const* text_end = nullptr );
void DrawText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos,
               ImU32 col, char const* text, char const* text_end = nullptr );
```
Renders `text` through the Slug GPU shader. The first overload uses the current
ImGui font and size; the second takes them explicitly (`nullptr` / `0` falls
back to the current ones). `pos` is the top-left of the text region.

### `DrawTextOnPath`
```cpp
void DrawTextOnPath( ImDrawList* draw, ImFont* font, float font_size, char const* text,
                     ImVec2 const* path_pts, int path_count, ImU32 col,
                     float offset_along = 0.0f, bool baseline_above = false );
```
Lays the string along an arbitrary polyline, rotating each glyph to the local
tangent. `offset_along` shifts the start position in arc length;
`baseline_above` flips which side of the path the baseline sits on.

### `DrawTextInsideRect` / `DrawTextInsideConvex`
```cpp
void DrawTextInsideRect  ( ImDrawList* draw, ImFont* font, float font_size,
                           ImRect bounds, char const* text, ImU32 col );
void DrawTextInsideConvex( ImDrawList* draw, ImFont* font, float font_size,
                           ImVec2 const* convex_poly, int poly_count,
                           char const* text, ImU32 col );
```
Wrap and fit a string inside a rectangle or a convex polygon, shrinking the
effective size until it fits.

### `DrawFontWaterfall`
```cpp
float DrawFontWaterfall( ImDrawList* draw, ImFont* font, ImVec2 pos, char const* text,
                         float const* sizes, int size_count, ImU32 col, float gap = 4.0f );
```
Draws the same string once per entry in `sizes`, stacked vertically. Returns the
total height consumed.

---

## Measurement

```cpp
ImVec2 CalcTextSize       ( ImFont* font, float font_size,
                            char const* text, char const* text_end = nullptr,
                            float* out_ascent = nullptr );
float  CalcShapedTextWidth( ImFont* font, float font_size,
                            char const* text, char const* text_end = nullptr );
```
`CalcTextSize` returns `(width, height)` in pixels for text rendered by
`DrawText`. `out_ascent`, when non-null, receives the distance above the
baseline -- use `pos.y + ascent` as the baseline Y when aligning with other
elements.

`CalcShapedTextWidth` returns only the advance width, after full shaping
(ligatures, kerning, complex scripts). Cheaper than `CalcTextSize` when you do
not need the height.

---

## Tessellated Text (CPU geometry)

These convert outlines into triangle meshes (`ImWidgetsShape`) for gradient or
image fills. The tessellation itself needs no shader.

```cpp
void TesselateText        ( ImFont* font, float font_size, char const* text,
                            ImWidgetsShape& outShape, char const* text_end = nullptr,
                            float tess_tol = 0.0f, int iterations = 0 );
void TesselateTextPerGlyph( ImFont* font, float font_size, char const* text,
                            ImVector<ImWidgetsShape>& outShapes, char const* text_end = nullptr,
                            float tess_tol = 0.0f, int iterations = 0 );
void ExtractTextContours  ( ImFont* font, float font_size, char const* text, char const* text_end,
                            ImVec2 offset, ImVector<ImVec2>& outPoly, ImRect& outBB,
                            float tess_tol = 0.0f );
```
`tess_tol = 0` selects an automatic tolerance; `iterations` adds smoothing
passes. `TesselateTextPerGlyph` preserves one shape per shaped glyph (including
ligatures). `ExtractTextContours` returns raw contour points -- explicitly
closed, clockwise outer / counter-clockwise holes -- suitable for
`DrawShapeWithHole`.

---

## Gradient & Image Text Fills

### CPU (tessellated)

```cpp
void DrawImageText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos,
                    ImTextureID tex, char const* text, char const* text_end = nullptr,
                    ImU32 tint = IM_COL32_WHITE,
                    ImVec2 uv_offset = ImVec2( 0, 0 ), ImVec2 uv_scale = ImVec2( 1, 1 ),
                    float tess_tol = 0.0f, int iterations = 0 );

void DrawLinearGradientText ( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos,
                              char const* text, ImVec2 uv_start, ImVec2 uv_end,
                              ImU32 col0, ImU32 col1,
                              ImWidgetsGradientInterp interp = ImWidgetsGradientInterp_sRGB,
                              char const* text_end = nullptr,
                              float tess_tol = 0.0f, int iterations = 0 );
void DrawRadialGradientText ( ... same signature ... );
void DrawDiamondGradientText( ... same signature ... );
```

`uv_start` / `uv_end` are in widget coordinates. For the radial form `uv_start`
is the centre and `uv_end` the outer edge; the diamond form uses a rhombus.

> **Changed in 2.0.0.** These took a `pfSpace2sRGB` / `pfsRGB2Space` function
> pointer pair; they now take a single `ImWidgetsGradientInterp interp`
> (`_sRGB`, `_LinearSRGB`, `_OkLab`, `_OkLCH`, `_HSV`).

### GPU

```cpp
void DrawLinearGradientTextGPU ( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos,
                                 char const* text, ImVec2 uv_start, ImVec2 uv_end,
                                 ImU32 col0, ImU32 col1, char const* text_end = nullptr,
                                 bool perChar = false,
                                 ImWidgetsGradientInterp interp = ImWidgetsGradientInterp_sRGB );
void DrawRadialGradientTextGPU ( ... same signature ... );
void DrawDiamondGradientTextGPU( ... same signature ... );

void DrawImageTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos,
                       char const* text, ImTextureID tex, ImU32 tint = IM_COL32_WHITE,
                       ImVec2 uv_offset = ImVec2( 0, 0 ), ImVec2 uv_scale = ImVec2( 1, 1 ),
                       char const* text_end = nullptr, bool perChar = false );
void DrawImageTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos,
                       char const* text, ImTextureID const* textures, int nTextures,
                       ImU32 tint = IM_COL32_WHITE,
                       ImVec2 uv_offset = ImVec2( 0, 0 ), ImVec2 uv_scale = ImVec2( 1, 1 ),
                       char const* text_end = nullptr );
```

The gradient is evaluated in the pixel shader, so there is no tessellation cost
and the result stays sharp at any zoom. `perChar = true` restarts the gradient at
each glyph instead of spanning the whole string. The multi-texture
`DrawImageTextGPU` overload cycles through `textures` per glyph.

> **Changed in 2.0.0.** The GPU forms took `int colorSpace = 0` with a comment
> explaining the magic numbers `0..4`. They now take
> `ImWidgetsGradientInterp interp`, the same enum as everything else.

---

## Font Tooling Widgets

### `FontInspector`
```cpp
void FontInspector( char const* label, ImFont* font,
                    ImFontInspectorMode mode = ImFontInspectorMode_Grid,
                    float display_size = 64.0f, ImVec2 size = ImVec2( 0, 0 ) );
```
Glyph browser and metrics viewer.
```cpp
enum ImFontInspectorMode_
{
    ImFontInspectorMode_Grid,     // glyph grid
    ImFontInspectorMode_Metrics,  // ascent / descent / x-height / cap-height overlay
    ImFontInspectorMode_Curves,   // outline control points and curves
    ImFontInspectorMode_Kerning,  // kerning-pair matrix
    ImFontInspectorMode_COUNT
};
```
Style vars: `ImWidgetsStyleVar_FontInspector_GlyphCell` / `_MetricAlpha`.

### `KerningPairEditor`
```cpp
bool KerningPairEditor( char const* id_str, ImFont* font, float font_size,
                        char left_glyph, char right_glyph, float* p_kern_px,
                        ImVec2 size = ImVec2( 0, 0 ) );
```
Drag the right glyph to adjust the kerning for one pair; the value is written to
`*p_kern_px` in pixels.

### `VarFontAxisSliders`
```cpp
bool VarFontAxisSliders( char const* id_str, ImWidgetsVarFontAxis* axes, int axis_count,
                         ImFont* preview_font = NULL,
                         char const* preview_text = "Aa Bb Cc 0123" );
```
One slider per variable-font axis (`wght`, `wdth`, `slnt`, ...) with a live
preview line. `axes` is caller-owned; each entry carries the tag, min / default /
max and the current value.

---

## Debug

```cpp
void DrawTextDebugCurves ( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos,
                           char const* text, char const* text_end = nullptr, int flags = 0xFF );
void DrawTextDebugLayers ( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos,
                           char const* text, char const* text_end = nullptr );
void DrawTesselateDebug  ( ImDrawList* dl, ImFont* font, float font_size, char const* text,
                           ImVec2 pos, float tess_tol, float spacing, float rowH );

void SetSlugDebugShader( bool enabled );
bool GetSlugDebugShader();
```

`DrawTextDebugCurves` visualizes Slug glyph geometry; `flags` is a bitmask:
`1` = curves, `2` = control points, `4` = bounding boxes, `8` = band grid.

`DrawTextDebugLayers` draws each COLR layer quad as a flat semi-transparent
rectangle, showing layer boundaries without running the Slug shader.

`DrawTesselateDebug` steps through the tessellation algorithm for a single
character.

`SetSlugDebugShader(true)` switches the renderer to the debug shader:
`xcov -> R`, `ycov -> G`, `coverage -> B`.

> **Changed in 2.0.0.** These replace the public mutable global
> `extern bool g_SlugDebugShader`.
