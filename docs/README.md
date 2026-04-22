# DearWidgets Documentation

DearWidgets is an ImGui extension library providing GPU-accelerated widgets, color science tools, scopes, LaTeX math rendering, and a complete GPU font pipeline. This directory contains the full reference for the public API and the architecture behind it.

---

## Where to Start

| Goal | Start here |
|---|---|
| Use the library for the first time | [context.md](api/context.md) -- init, feature flags |
| Understand which features need a custom shader backend | [compatibility.md](compatibility.md) |
| Browse all widgets | [widgets.md](api/widgets.md) |
| Draw shapes, gradients, markers | [drawlist.md](api/drawlist.md) |
| Render GPU text or LaTeX | [typography.md](api/typography.md), [latex.md](api/latex.md) |
| Work with color spaces | [color.md](api/color.md), [color-spaces.md](concepts/color-spaces.md) |

---

## API Reference (`./api/`)

Function-level reference grouped by category.

| Document | Contents |
|---|---|
| [context.md](api/context.md) | Context lifecycle, feature flags, configuration |
| [style.md](api/style.md) | Style colors, style vars, push/pop API |
| [widgets.md](api/widgets.md) | All interactive widgets (sliders, color editors, scopes, etc.) |
| [typography.md](api/typography.md) | Slug GPU text: DrawText, CalcTextSize, tessellated text, gradient fills |
| [latex.md](api/latex.md) | LaTeX math rendering: DrawLaTeX, CalcLaTeXSize, supported syntax |
| [color.md](api/color.md) | Color space conversions, blending, gradient sampling |
| [drawlist.md](api/drawlist.md) | DrawList primitives: shapes, gradients, markers, polylines, chromatic plots |
| [data-types.md](api/data-types.md) | All data structs (ImGradientData, ImCurveEditorData, ImHistogramData, etc.) |

---

## Concepts (`./concepts/`)

Architecture and design documents -- useful when the API reference alone does not explain why something works the way it does.

| Document | Contents |
|---|---|
| [slug.md](concepts/slug.md) | Slug GPU font rendering -- architecture, glyph cache, color fonts, limitations |
| [latex.md](concepts/latex.md) | LaTeX box model, pipeline, math font, supported commands |
| [color-spaces.md](concepts/color-spaces.md) | Color spaces used throughout the library |
| [shaders.md](concepts/shaders.md) | Slang shader cross-compilation -- HLSL authoring, slangc usage, output layout |

---

## Guides

Task-oriented references that cut across multiple API areas.

| Document | Contents |
|---|---|
| [compatibility.md](compatibility.md) | Per-feature table: ImGui only / ImPlatform / custom shader requirement |
| [interactions.md](interactions.md) | Widget keyboard/mouse interaction reference |
| [modals.md](modals.md) | Expand-to-window pattern -- architecture and per-widget layout |
