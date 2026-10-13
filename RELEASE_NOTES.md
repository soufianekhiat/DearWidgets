# DearWidgets 2.0.0 -- Release Notes

DearWidgets 2.0 is the first tagged release. It freezes the public API (names,
argument order, enum prefixes, type scope) and brings the web's text and vector
graphics model to a plain `ImDrawList`: shaped, bidirectional text laid out like
a browser paragraph, SVG / Canvas2D paths with real fill rules, and SVG
documents.

The full list of changes, with migration notes for every breaking change, is in
[CHANGELOG.md](CHANGELOG.md).

```cpp
#if IMWIDGETS_VERSION_NUM >= 20000
    // 2.0 API
#endif
```

---

## Highlights

### Text that behaves like the web

- **Bidirectional text.** `DrawText`, `CalcTextSize` and the gradient / image
  text variants run the Unicode bidi algorithm (UAX #9) before shaping, so mixed
  Arabic, Hebrew, Latin and numbers lay out as in a browser.
  `GetStyle().TextDirection` sets the paragraph direction (`Auto` / `LTR` / `RTL`).
- **Paragraphs.** `DrawTextBlock` / `CalcTextBlockSize`: UAX #14 line breaking,
  per-paragraph direction, start / end / left / right / center alignment, line
  height, underline / line-through / overline, `MaxLines` line-clamp with an
  ellipsis, and CSS `letter-spacing` / `word-spacing`.
- **Font fallback.** `SetFontFallbacks` gives a font a CSS-like family chain:
  characters it lacks come from the first fallback that has them.
- **Editing.** `InputTextShaped` and `InputTextMultilineShaped` are
  grapheme-aware, bidi-ordered input fields (selection, clipboard, IME, wrapping,
  vertical caret movement, Escape restores the original text). The building
  blocks are public: `CalcTextGraphemes`, `TextCaretX`, `TextHitTest`,
  `TextSelectionRanges`.
- **Any backend.** `SetTextRenderer` chooses the Slug GPU renderer, a CPU vector
  renderer (glyph outlines as `ImDrawList` triangles) or the ImGui atlas; `Auto`
  falls back to the CPU renderer where the shader is unavailable.

### Vector graphics on a plain ImDrawList

- **Paths.** `ImWidgetsPath` (moveTo / lineTo / quadratic / cubic / SVG arcs /
  rect / round rect / ellipse, transforms) with an SVG path-data parser.
- **Fills.** `PathFill` / `FillPolygons`: anti-aliased, nonzero and even-odd
  fill rules, holes and self-intersections handled exactly, no double-covered
  pixels. `PathContains` hit-tests with the same rule.
- **Paint.** CSS / SVG gradients (linear, radial, conic; hard stops,
  pad / repeat / reflect, gradient transform, sRGB / linear / OkLab / OkLCH / HSV
  interpolation) for fills and strokes.
- **Strokes.** `PathStroke` with caps, joins, dashes and gradient paint;
  `DrawVariableWidthStroke` for pressure / taper strokes.
- **Clipping.** `PushPathClip` / `PopPathClip`: anti-aliased clipping by any
  path, nested up to 4 deep.

Everything above is ImDrawList triangles: it works on stock Dear ImGui, no
shader required.

### SVG documents

`SvgLoad` / `SvgDraw` / `SvgFree` draw icons and illustrations: shapes, paths,
groups, transforms, gradients, fill / stroke properties, opacity, dashes,
`currentColor`, `<clipPath>`, `<use>` / `<symbol>`, `<style>` sheets (simple
selectors), markers (arrowheads, dots) and shaped `<text>`.

### Widgets

29 physically-based and artistic colour pickers, a unified scalar -> colour
API, grading widgets (colour bars, colour slice, warpers), image inspection and
look-dev tools, network graph and timeline widgets, a canvas / authoring toolkit
(pan-zoom canvas, marquee / lasso selection, crop, eyedropper, gradient drop,
size controls), new slider variants and dials. Composite widgets now report
edits through `IsItemEdited()` / `IsItemDeactivatedAfterEdit()`.

---

## Breaking changes

2.0 renames, retypes or removes a number of 1.x symbols. The most visible:

- All public types are at global scope; style enums are prefixed.
- Colour blending, gradient fills and gradient text take an
  `ImWidgetsGradientInterp` instead of per-space functions or pointers.
- HSL / HSY / HSP conversions take their outputs first.
- `PrimariesWheel`, `ImageTransformGizmo`, the HarfBuzz backend,
  `DrawDropShadowRect` / `DrawInnerGlowRect` and several dead declarations are
  removed.
- Spelling fixes without aliases: `Tessellate*`, `RemoveFeatures`,
  `DrawLuminanceBand`, `ImWidgetsJoin_Miter`.
- Double-click on a slider resets it to its default; typed input moved to
  Ctrl+click.

Each one, with its replacement, is listed under **Breaking changes** and
**Removed** in [CHANGELOG.md](CHANGELOG.md).

---

## Requirements

- C++11, Dear ImGui (bundled through the ImPlatform submodule).
- Most widgets and the whole vector / SVG layer need only Dear ImGui. The Slug
  GPU text, LaTeX, GPU markers and the shader-based inspectors need a backend
  with custom shaders (DX10 / DX11 / DX12 / OpenGL 3+ / Vulkan through
  ImPlatform); see [docs/compatibility.md](docs/compatibility.md).
- The bundled build generator targets Windows (D3D11, D3D12, OpenGL3); the
  sources are portable.

---

## Known limitations

- **OpenGL3:** Slug GPU text renders incorrectly; the `Auto` text renderer uses
  the CPU vector renderer there instead.
- **Font fallback** is global (not per context), at most 8 fonts, and is not
  invalidated when a font is destroyed.
- **Path clips** apply to CPU fills only; plain `ImDrawList` calls and GPU-shader
  drawing (Slug text) are not clipped.
- **SVG** does not support images, masks, filters, patterns, radial focal
  points, `<tspan>` positioning, stroked text or text inside clip paths.
- **Experimental:** snap lines / smart guides and the onion-skin overlay.

---

## Licence

The DearWidgets sources and documentation are CC0. Third-party components keep
their own licences (CDT is MPL-2.0, vendored in `extern/CDT/`), and the Slug
technique is covered by patents held by Terathon Software -- see
[README.md](README.md#third-party-components) before redistributing.
