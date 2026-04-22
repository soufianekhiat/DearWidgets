# Typography -- Slug GPU Text Rendering

> **Requires:** `ImWidgetsFeatures_RichFont` set before `CreateContext()`.
> Custom shader backend (`IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER`) is mandatory.

Slug renders text directly from TrueType/OpenType bezier outlines on the GPU -- no bitmap atlas, no SDF baking. Results are crisp at any scale, zoom, or viewing angle. Color fonts (COLR v0/v1) and gradient fills are supported natively.

See [concepts/slug.md](../concepts/slug.md) for architecture details.

---

## Font Loading

### `GetSlugFontLoader`
```cpp
const ImFontLoader* GetSlugFontLoader();
```
Returns an `ImFontLoader` backend that rasterizes Slug glyphs (including color/gradient layers) into ImGui's bitmap atlas for widgets that still use the CPU rasterizer. Use as:
```cpp
ImFontConfig cfg;
cfg.FontLoader = ImWidgets::GetSlugFontLoader();
io.Fonts->AddFontFromFileTTF("myfont.ttf", 16.0f, &cfg);
```

### `GetSlugFontInfo`
```cpp
bool GetSlugFontInfo(ImFont* font, void* outStbttFontInfo, float* outEmScale);
```
Retrieves the underlying `stbtt_fontinfo` and em-scale for a loaded font. Advanced use only.

### `SlugBuildGlyphByID`
```cpp
void SlugBuildGlyphByID(ImFont* font, int glyphID);
```
Force-build the Slug geometry for a glyph by its glyph ID (not codepoint). Used for pre-warming the glyph cache.

---

## Drawing

### `DrawText` (current font/size)
```cpp
void DrawText(ImDrawList* pDrawList, ImVec2 pos, ImU32 col,
              const char* text, const char* text_end = nullptr);
```
Renders `text` with the current ImGui font and font size using the Slug GPU shader. `pos` is the top-left corner of the text baseline region.

### `DrawText` (explicit font/size)
```cpp
void DrawText(ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos,
              ImU32 col, const char* text, const char* text_end = nullptr);
```
Same as above but with an explicit font and size. Pass `nullptr`/`0` to use the current ImGui font/size.

### `DrawTextGradient`
```cpp
void DrawTextGradient(ImDrawList* pDrawList, ImFont* font, float font_size,
                      ImVec2 pos, ImU32 col_left, ImU32 col_right,
                      const char* text, const char* text_end = nullptr);
```
Horizontal linear gradient fill: `col_left` at the first character, `col_right` at the last.

---

## Measurement

### `CalcTextSize`
```cpp
ImVec2 CalcTextSize(ImFont* font, float font_size,
                    const char* text, const char* text_end = nullptr,
                    float* out_ascent = nullptr);
```
Returns `(width, height)` in pixels for text rendered via `DrawText`.
`out_ascent`: if non-null, receives the distance above the baseline. Use `pos.y + ascent` as the baseline Y when aligning with other elements.

---

## Tessellated Text (CPU geometry)

These functions convert text outlines into triangle meshes (`ImWidgetsShape`) for gradient or image fills. No shader is needed for the tessellation itself, but the resulting shape is typically drawn via `DrawShape*` which uses the standard ImGui white texture path.

### `TesselateText`
```cpp
void TesselateText(ImFont* font, float font_size,
                   const char* text, ImWidgetsShape& outShape,
                   const char* text_end = nullptr,
                   float tess_tol = 0.0f, int iterations = 0);
```
Tessellates all glyphs into a single merged `ImWidgetsShape`. `tess_tol = 0` = auto tolerance. `iterations` = extra smoothing passes (0 = none).

### `TesselateTextPerGlyph`
```cpp
void TesselateTextPerGlyph(ImFont* font, float font_size,
                            const char* text,
                            ImVector<ImWidgetsShape>& outShapes,
                            const char* text_end = nullptr,
                            float tess_tol = 0.0f, int iterations = 0);
```
Same as `TesselateText` but preserves per-glyph shapes (including ligatures from full text shaping via `kb_text_shape`).

### `ExtractTextContours`
```cpp
void ExtractTextContours(ImFont* font, float font_size,
                         const char* text, const char* text_end,
                         ImVec2 offset,
                         ImVector<ImVec2>& outPoly, ImRect& outBB,
                         float tess_tol = 0.0f);
```
Extracts raw contour points (explicitly closed, clockwise outer / counter-clockwise holes) into a flat `ImVector<ImVec2>`. For use with `DrawShapeWithHole`. Debug/internal use.

---

## Gradient & Image Text Fills

### `DrawImageText`
```cpp
void DrawImageText(ImDrawList* pDrawList, ImFont* font, float font_size,
                   ImVec2 pos, ImTextureID tex, const char* text,
                   const char* text_end = nullptr,
                   ImU32 tint = IM_COL32_WHITE,
                   ImVec2 uv_offset = {0,0}, ImVec2 uv_scale = {1,1},
                   float tess_tol = 0.0f, int iterations = 0);
```
Tessellates text and fills it with a texture. `uv_offset` and `uv_scale` control the texture mapping.

### `DrawLinearGradientText`
```cpp
void DrawLinearGradientText(ImDrawList* pDrawList, ImFont* font, float font_size,
                             ImVec2 pos, const char* text,
                             ImVec2 uv_start, ImVec2 uv_end,
                             ImU32 col0, ImU32 col1,
                             pfSpace2sRGB space2sRGB = nullptr,
                             pfsRGB2Space sRGB2Space = nullptr,
                             const char* text_end = nullptr,
                             float tess_tol = 0.0f, int iterations = 0);
```
Text filled with a linear gradient. `uv_start`/`uv_end` are in widget coordinates. Pass color-space function pointers for OkLab/OkLCH interpolation (see [color.md](color.md)).

### `DrawRadialGradientText`
Same as `DrawLinearGradientText` but with a radial gradient. `uv_start` = center, `uv_end` = outer edge.

### `DrawDiamondGradientText`
Same as `DrawLinearGradientText` but with a diamond (rhombus) gradient.

---

## Debug

### `DrawTextDebugCurves`
```cpp
void DrawTextDebugCurves(ImDrawList* pDrawList, ImFont* font, float font_size,
                          ImVec2 pos, const char* text,
                          const char* text_end = nullptr, int flags = 0xFF);
```
Visualizes Slug glyph geometry. `flags` bitmask: `1`=curves, `2`=control points, `4`=bounding boxes, `8`=band grid.

### `DrawTextDebugLayers`
```cpp
void DrawTextDebugLayers(ImDrawList* pDrawList, ImFont* font, float font_size,
                          ImVec2 pos, const char* text,
                          const char* text_end = nullptr);
```
Draws each COLR layer quad as a flat semi-transparent rectangle. Shows layer boundaries without running the Slug shader.

### `DrawTesselateDebug`
```cpp
void DrawTesselateDebug(ImDrawList* dl, ImFont* font, float font_size,
                         const char* text, ImVec2 pos,
                         float tess_tol, float spacing, float rowH);
```
Step-by-step visualization of the tessellation algorithm for a single character.

### `g_SlugDebugShader`
```cpp
extern bool g_SlugDebugShader;
```
Set to `true` to use the debug shader: xcov->R, ycov->G, coverage->B instead of normal rendering.
