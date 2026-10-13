# Context & Initialization

## Version

```cpp
#define IMWIDGETS_VERSION       "2.0.0"
#define IMWIDGETS_VERSION_NUM   20000     // major*10000 + minor*100 + patch
```

`IMWIDGETS_VERSION_NUM` is comparable numerically:

```cpp
#if IMWIDGETS_VERSION_NUM >= 20000
    // 2.0 API
#endif
```

---

## Overview

`ImWidgetsContext` holds all GPU state for DearWidgets: shader handles, texture
resources, and the Slug font atlas cache. It maps to -- and must be paired with
-- a single `ImGuiContext`.

---

## Lifecycle

`CreateContext()` creates GPU resources (at minimum the internal system
textures; with feature flags set, also the shader pipelines). It must therefore
run **after** the graphics backend is up.

Using ImPlatform, the full sequence is:

```cpp
ImPlatform_CreateWindow( "App", pos, w, h );
ImPlatform_InitGfxAPI();               // graphics device
ImPlatform_ShowWindow();

ImGui::CreateContext();
// ... io.Fonts->AddFontFromFileTTF(...), ImWidgets::LoadLaTeXFont(), ...

ImPlatform_InitPlatform();             // ImGui platform backend
ImPlatform_InitGfx();                  // graphics resources + font atlas upload

ImWidgets::SetFeatures( ImWidgetsFeatures_Markers | ImWidgetsFeatures_RichFont );
ImWidgetsContext* ctx = ImWidgets::CreateContext();
ImWidgets::SetCurrentContext( ctx );

// ... frame loop ...

ImWidgets::DestroyContext( ctx );
```

> There is no `ImPlatform::InitGfx` -- ImPlatform's API is C-style free
> functions: `ImPlatform_InitGfxAPI()`, `ImPlatform_InitPlatform()`,
> `ImPlatform_InitGfx()`. Earlier revisions of this page used the wrong name.

With a plain ImGui backend (no ImPlatform), call `ImWidgets::CreateContext()`
after your backend's `ImGui_ImplXxx_Init` pair.

---

## Feature Flags

```cpp
enum ImWidgetsFeatures_
{
    ImWidgetsFeatures_None     = 0,
    ImWidgetsFeatures_Markers  = 1 << 0,  // GPU marker shapes (requires shader support)
    ImWidgetsFeatures_RichFont = 1 << 1,  // Slug GPU text rendering
    ImWidgetsFeatures_LaTeX    = 1 << 2,  // LaTeX math rendering (implies RichFont)
};
typedef int ImWidgetsFeatures;
```

Features must be set **before** `CreateContext()`; their shaders are compiled at
context creation.

> This is a bit-flag enum: there is no `ImWidgetsFeatures_COUNT` sentinel
> (one used to be declared by mistake and was removed in 2.0.0).

> **Not every shader is behind a flag.** `image_inspector.hlsl`,
> `volume_viewer.hlsl`, `lookdev_inspector.hlsl`, `delta_e_compare.hlsl` and
> `blur.hlsl` are compiled lazily on first use through `CreateInternalShader`
> and are not gated by any `ImWidgetsFeatures_` value. See
> [compatibility.md](../compatibility.md).

---

## Functions

### `SetFeatures` / `AddFeatures` / `RemoveFeature`
```cpp
void SetFeatures  ( ImWidgetsFeatures features );   // replace the set
void AddFeatures  ( ImWidgetsFeatures features );   // OR into the set
void RemoveFeature( ImWidgetsFeatures features );   // clear flags
```
Call before `CreateContext()`.

---

### `CreateContext` / `DestroyContext`
```cpp
ImWidgetsContext* CreateContext();
void              DestroyContext( ImWidgetsContext* ctx );
```
`CreateContext` allocates the context, creates the internal system textures and
compiles the GPU shaders for all enabled features. `DestroyContext` releases
every GPU resource the context owns, including textures handed over with
`OwnTexture`.

---

### `SetCurrentContext` / `GetCurrentContext`
```cpp
void              SetCurrentContext( ImWidgetsContext* ctx );
ImWidgetsContext* GetCurrentContext();
```
Manage the current context, mirroring ImGui's own pattern.

---

### `GetWhiteTexture`
```cpp
ImTextureID GetWhiteTexture();
```
The small white RGBA texture owned by the context -- a useful default for shape
drawing.

---

### `OwnTexture`
```cpp
void OwnTexture( ImTextureID tex );
```
Transfer ownership of a texture to the context; it is destroyed with
`DestroyContext()`.

---

## Shaders

> **Requires:** `IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER`.

```cpp
void PrebuildShaders();
void CreateInternalShader( ImDrawShader* shaders_out, char const* shader_name,
                           int sizeof_vs_const_buffer, void* vs_const_buffer,
                           int sizeof_ps_const_buffer, void* ps_const_buffer,
                           char const* extra_define = nullptr,
                           char const* cache_suffix = nullptr,
                           unsigned int compile_flags = IMPLATFORM_SHADER_COMPILE_DEFAULT );
```

`PrebuildShaders()` compiles every internal shader pipeline up front, so the
first frame that uses one does not stall. Call it once after
`CreateContext()`.

`CreateInternalShader` is the lazy compile-and-cache entry point the library
uses for its own shaders, exposed so applications can add their own variants.
`extra_define` injects a preprocessor define (this is how one `blur.hlsl`
produces 18 separate effect variants through `BG_EFFECT`); `cache_suffix`
distinguishes the resulting bytecode in the on-disk cache.

---

## Configuration

```cpp
// GPU vs CPU dashed lines (default: CPU)
void SetDashedLinesUseGPU( bool enable );
bool GetDashedLinesUseGPU();

// Debug: overlay join geometry for dashed lines
void SetDashedLinesDebugJoins( bool enable );
bool GetDashedLinesDebugJoins();

// Debug: wireframe the Euler-spiral stroke tessellation
void SetStrokeDebugWireframe( bool enable );
bool GetStrokeDebugWireframe();

// Debug: Slug coverage shader (xcov -> R, ycov -> G, coverage -> B)
void SetSlugDebugShader( bool enabled );
bool GetSlugDebugShader();
```

> **Changed in 2.0.0.** `SetSlugDebugShader` / `GetSlugDebugShader` replace the
> public mutable global `extern bool g_SlugDebugShader`.

---

## Style persistence

```cpp
ImWidgetsStyle& GetStyle();
void            SaveStyleToFile( char const* filename, ImWidgetsStyle const* style = NULL );
bool            LoadStyleFromFile( char const* filename, ImWidgetsStyle* style = NULL );
void            ShowStyleEditor( ImWidgetsStyle* ref = NULL );
```
See [style.md](style.md).
