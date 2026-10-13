# DearWidgets Documentation

DearWidgets is a Dear ImGui extension library providing GPU-accelerated widgets,
color science tools, scopes, LaTeX math rendering, and a complete GPU font
pipeline. This directory contains the full reference for the public API and the
architecture behind it.

Documentation target: **DearWidgets 2.0.0**. Signatures here are copied from
`src/api/*.h`, which is the single source of truth -- if the two ever disagree,
the header wins and the documentation is a bug.

---

## Where to Start

| Goal | Start here |
|---|---|
| Use the library for the first time | [context.md](api/context.md) -- init order, feature flags, shaders |
| See what changed in 2.0.0 | [../CHANGELOG.md](../CHANGELOG.md) -- breaking changes first |
| Understand which features need a custom shader backend | [compatibility.md](compatibility.md) |
| Browse all widgets | [widgets.md](api/widgets.md) |
| Draw shapes, gradients, markers | [drawlist.md](api/drawlist.md) |
| Render GPU text or LaTeX | [typography.md](api/typography.md), [latex.md](api/latex.md) |
| Work with color spaces | [color.md](api/color.md), [color-spaces.md](concepts/color-spaces.md) |
| Restyle the widgets | [style.md](api/style.md) |

---

## API Reference (`./api/`)

Function-level reference grouped by category.

| Document | Contents |
|---|---|
| [context.md](api/context.md) | Version macros, context lifecycle, feature flags, shader plumbing, global configuration |
| [style.md](api/style.md) | All 137 style colours and 123 style vars, the push/pop API, style persistence |
| [widgets.md](api/widgets.md) | Every interactive widget: sliders, colour editors and pickers, grading, scopes, image widgets, LUT / LookDev / volume, canvas toolkit, graphs and timelines, custom-shape buttons, interaction helpers |
| [typography.md](api/typography.md) | Slug GPU text: `DrawText`, measurement, tessellated text, gradient and image fills, text on a path, font tooling widgets, debug |
| [latex.md](api/latex.md) | LaTeX math rendering: setup, `DrawLaTeX`, `CalcLaTeXSize`, supported syntax |
| [color.md](api/color.md) | Colour space conversions, blending, gradient sampling, colour-space / model introspection, the scalar -> colour API, Delta-E |
| [drawlist.md](api/drawlist.md) | DrawList primitives: shapes, gradients, fills and patches, iso-contours, lines and strokes, cursors, annotations, chromaticity plots, ephemerides, markers |
| [data-types.md](api/data-types.md) | All public data structs and the enums that describe them |

---

## Concepts (`./concepts/`)

Architecture and design documents -- useful when the API reference alone does
not explain why something works the way it does.

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
| [compatibility.md](compatibility.md) | Per-feature table: ImGui only / ImPlatform / custom shader requirement, plus the `DrawThickLine` fallback details |
| [interactions.md](interactions.md) | Keyboard and mouse interaction reference for the classic editing widgets |

---

## Contributor notes

Not part of the public API reference -- these describe internals and are aimed
at people working *on* DearWidgets rather than *with* it.

| Document | Contents |
|---|---|
| [modals.md](modals.md) | The expand-to-window pattern: the `static` infrastructure in `dear_widgets.cpp`, the per-widget layout convention, and which widgets implement it |

Working notes that used to live here (design reviews, idea lists, release
checklists) are in the repository's `notes/` directory and are not part of the
published documentation.
