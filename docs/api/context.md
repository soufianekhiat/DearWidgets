# Context & Initialization

## Overview

`ImWidgetsContext` holds all GPU state for DearWidgets: shader handles, texture resources, and the Slug font atlas cache. It maps to — and must be paired with — a single `ImGuiContext`.

---

## Lifecycle

```cpp
// After ImGui::CreateContext() and ImPlatform::InitGfx()
ImWidgets::SetFeatures(ImWidgetsFeatures_Markers | ImWidgetsFeatures_RichFont);
ImWidgetsContext* ctx = ImWidgets::CreateContext();
ImWidgets::SetCurrentContext(ctx);

// ... frame loop ...

ImWidgets::DestroyContext(ctx);
```

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
```

Features must be set **before** `CreateContext()`. Shaders are compiled at context creation time.

---

## Functions

### `SetFeatures`
```cpp
void SetFeatures(ImWidgetsFeatures features);
```
Replace the current feature set (before `CreateContext()`).

---

### `AddFeatures`
```cpp
void AddFeatures(ImWidgetsFeatures features);
```
OR the given flags into the current feature set.

---

### `RemoveFeature`
```cpp
void RemoveFeature(ImWidgetsFeatures features);
```
Remove specific feature flags.

---

### `CreateContext`
```cpp
ImWidgetsContext* CreateContext();
```
Allocate and initialize a context. Compiles GPU shaders for all enabled features. Must be called after `ImPlatform::InitGfx()`.

---

### `DestroyContext`
```cpp
void DestroyContext(ImWidgetsContext* ctx);
```
Release all GPU resources owned by the context.

---

### `SetCurrentContext` / `GetCurrentContext`
```cpp
void               SetCurrentContext(ImWidgetsContext* ctx);
ImWidgetsContext*  GetCurrentContext();
```
Manage the thread-local current context, mirroring ImGui's own pattern.

---

### `GetWhiteTexture`
```cpp
ImTextureID GetWhiteTexture();
```
Returns the 4×4 white RGBA texture owned by the context (useful as a default for shape drawing).

---

### `OwnTexture`
```cpp
void OwnTexture(ImTextureID tex);
```
Transfer ownership of a texture to the context; it will be destroyed with `DestroyContext()`.

---

## Configuration

```cpp
// GPU vs CPU dashed lines (default: CPU)
void SetDashedLinesUseGPU(bool enable);
bool GetDashedLinesUseGPU();

// Debug: overlay join-geometry for dashed lines
void SetDashedLinesDebugJoins(bool enable);
bool GetDashedLinesDebugJoins();
```
