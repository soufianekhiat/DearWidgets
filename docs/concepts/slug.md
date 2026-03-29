# Slug GPU Font Rendering

## What is Slug?

Slug is an algorithm (originally by Eric Lengyel, Terathon Software) for rendering TrueType/OpenType glyphs directly on the GPU from their Bézier outline data. Unlike bitmap atlases or signed distance fields, Slug evaluates the exact coverage of each glyph curve per-pixel in the fragment shader.

DearWidgets implements its own Slug-inspired renderer integrated with the ImGui/ImPlatform pipeline.

---

## Key Properties

| Property | Bitmap Atlas | SDF | Slug (GPU) |
|---|---|---|---|
| Quality at small sizes | Good | Medium | Good |
| Quality at large sizes / zoom-in | Pixelated | Blurry | Perfect |
| Quality at oblique angles | Pixelated | OK | Perfect |
| Memory (atlas) | Medium | Small | None (per-glyph on demand) |
| Color fonts (COLR) | Limited | No | Full COLR v0/v1 |
| Gradient fills | No | No | Yes (arbitrary) |
| GPU requirement | None | None | Custom shader |

---

## Architecture

### Glyph Cache

Each ImFont gets a `SlugGlyphData` entry in the per-context `ImWidgetsSlugState`. First time a glyph is requested:

1. The TTF outline is read from the in-memory font data via `stbtt`.
2. The bezier contours are decomposed into line and quadratic segments.
3. Per-band coverage tables (`xcov`, `ycov`) are precomputed and uploaded as a small per-glyph float buffer.
4. A draw call is recorded as an `ImDrawCmd` with the Slug shader.

Subsequent frames reuse the cached band data — no atlas needed.

### Shader Pipeline

Two vertex + pixel shader pairs are compiled at `CreateContext()` time:

| Shader | Purpose |
|---|---|
| `slugShader` | Monochrome glyphs |
| `slugColorShader` | COLR v0 color layers |
| `slugGradientShader` | COLR v1 linear gradient stops |
| `slugDebugShader` | Debug: xcov/ycov/coverage as RGB |

Each glyph emits a quad (`ImDrawCmd`) into the ImGui draw list. The vertex buffer carries the per-glyph curve data as vertex attributes.

### Text Shaping

When `kb_text_shape` is available, Slug uses it for full Unicode text shaping (kerning, ligatures, Arabic/Hebrew bidirectionality). This is the same shaper used by the LaTeX renderer for `\text{...}` content.

---

## Usage Pattern

```cpp
// Init (once, before CreateContext)
ImWidgets::SetFeatures(ImWidgetsFeatures_RichFont);

// Per-frame
ImDrawList* dl = ImGui::GetWindowDrawList();
ImWidgets::DrawText(dl, ImGui::GetCursorScreenPos(), IM_COL32_WHITE, "Hello, Slug!");

// Measure for layout
ImVec2 sz = ImWidgets::CalcTextSize(nullptr, 0, "Hello, Slug!");
```

---

## Color Fonts

Slug renders COLR v0 (simple color layers) and COLR v1 (gradients) using separate draw calls per layer, each with its own shader variant. No additional setup is needed — fonts with COLR tables are detected automatically.

---

## Tessellated Text

For gradient or image fills applied to text outlines (e.g. a gold-foil effect), the CPU tessellator converts glyph bezier outlines into `ImWidgetsShape` triangle meshes:

```cpp
ImWidgetsShape shape;
ImWidgets::TesselateText(font, 48.0f, "DearWidgets", shape);
ImWidgets::ShapeOkLchLinearGradient(shape, pos, pos + size, gold, amber);
ImWidgets::DrawShape(dl, shape);
```

The tessellation tolerance (`tess_tol`) controls the maximum pixel error of the linearized curves. `0` = auto (scales with font size).

---

## Limitations

- Requires a custom shader backend (`IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER`): OpenGL3+, DX10, DX11, DX12, Vulkan.
- DX9 and other fixed-function backends are not supported.
- Emoji (bitmap-only fonts like Noto Color Emoji) use bitmap layers, not bezier outlines.
- Very large glyph counts in a single frame may stress the per-glyph upload bandwidth; pre-warm with `SlugBuildGlyphByID` if needed.
