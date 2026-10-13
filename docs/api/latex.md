# LaTeX Math Rendering

> **Requires:** `ImWidgetsFeatures_LaTeX` (implies `ImWidgetsFeatures_RichFont`) set before `CreateContext()`.

DearWidgets includes a self-contained LaTeX math renderer: tokenizer -> parser -> box tree -> layout -> render, using Latin Modern Math as the math font rendered via the Slug GPU pipeline.

See [concepts/latex.md](../concepts/latex.md) for supported syntax and architecture.

---

## Setup

`LoadLaTeXFont()` calls `ImGui::GetIO().Fonts->AddFontFromFileTTF(...)`, so it
needs a **live ImGui context** and must run **during the font-loading phase** --
that is, *after* `ImGui::CreateContext()` and *before* the font atlas is built
and uploaded (`ImPlatform_InitGfx()`, or whatever your backend's equivalent is).

```cpp
ImPlatform_CreateWindow( "App", pos, w, h );
ImPlatform_InitGfxAPI();
ImPlatform_ShowWindow();

ImGui::CreateContext();               // 1. ImGui context first
// ... io.Fonts->AddFontFromFileTTF(...) for your own fonts ...
ImWidgets::LoadLaTeXFont();           // 2. adds Latin Modern Math to the atlas

ImPlatform_InitPlatform();
ImPlatform_InitGfx();                 // 3. builds and uploads the font atlas

ImWidgets::AddFeatures( ImWidgetsFeatures_LaTeX );
ImWidgetsContext* ctx = ImWidgets::CreateContext();   // 4. compiles the shaders
ImWidgets::SetCurrentContext( ctx );
```

> Earlier revisions of this page said to call `LoadLaTeXFont()` *before*
> `ImGui::CreateContext()`. That is wrong -- there is no font atlas to add to at
> that point.

### Where the font comes from

The math font is **not compiled into the library**. `LoadLaTeXFont()` loads

```
latex_fonts/latinmodern-math.otf
```

as a path **relative to the process working directory**, using the Slug font
loader (`ImFontConfig::FontLoader = GetSlugFontLoader()`) at 24 px with the
math-relevant Unicode ranges (Greek, arrows, operators, Mathematical
Alphanumeric Symbols, ...).

The file ships in `workingdir/latex_fonts/`, so either run with `workingdir/` as
the current directory or copy that folder next to your executable. If the file
is missing, `LoadLaTeXFont()` silently leaves the math font null and every
`DrawLaTeX` call renders nothing.

Latin Modern Math is an **OTF** (CFF outlines), not a TTF, and is distributed
under the GUST Font License -- see the Third-party components section of
[README.md](../../README.md).

---

## Functions

### `LoadLaTeXFont`
```cpp
void LoadLaTeXFont();
```
Loads `latex_fonts/latinmodern-math.otf` into the ImGui font atlas. Idempotent:
a second call is a no-op. Must be called **after** `ImGui::CreateContext()` and
**before** the atlas is built.

---

### `DrawLaTeX`
```cpp
void DrawLaTeX(ImDrawList* pDrawList, float font_size, ImVec2 pos,
               ImU32 col, const char* latex);
```
Parse, layout, and render a LaTeX math expression in one call.

- `font_size`: math font size in pixels (display-style scaling).
- `pos`: top-left corner of the expression bounding box.
- `col`: base text color (overridden by `\color{...}` within the expression).
- `latex`: math mode string without surrounding `$` delimiters, e.g. `"x^2 + \\frac{\\alpha}{\\beta} = 0"`.

```cpp
ImWidgets::DrawLaTeX(dl, 24.0f, ImVec2(10, 10), IM_COL32_WHITE,
                     "E = mc^2");
```

---

### `CalcLaTeXSize`
```cpp
ImVec2 CalcLaTeXSize(float font_size, const char* latex);
```
Returns `(width, height)` of the bounding box that `DrawLaTeX` would produce, without drawing. Use for layout/alignment:

```cpp
ImVec2 sz = ImWidgets::CalcLaTeXSize(24.0f, "\\int_0^\\infty e^{-x} dx = 1");
ImVec2 pos = ImGui::GetCursorScreenPos();
pos.x += (ImGui::GetContentRegionAvail().x - sz.x) * 0.5f; // center
ImWidgets::DrawLaTeX(dl, 24.0f, pos, col, "...");
```

---

### `DrawLaTeXDebug`
```cpp
void DrawLaTeXDebug(ImDrawList* pDrawList, float font_size,
                    ImVec2 pos, const char* latex);
```
Draws the same expression as `DrawLaTeX` plus colored bounding rectangles for each box in the layout tree (glyphs, fractions, scripts, matrices, etc.). Useful for debugging layout issues.

---

### `TesselateLaTeX`
```cpp
void TesselateLaTeX(float font_size, const char* latex, ImVec2 pos,
                    ImWidgetsShape& outShape,
                    float tess_tol = 0.25f, int iterations = 0);
```
Tessellates a LaTeX expression into an `ImWidgetsShape` (triangle mesh). Use with gradient or image fill functions:

```cpp
ImVec2 sz = ImWidgets::CalcLaTeXSize( 24.0f, "E = mc^2" );

ImWidgetsShape shape;
ImWidgets::TesselateLaTeX( 24.0f, "E = mc^2", pos, shape );
ImWidgets::ShapeLinearGradient( shape, pos, pos + sz, col0, col1,
                                ImWidgetsGradientInterp_OkLab );
ImWidgets::DrawShape( dl, shape );
```

---

## Supported LaTeX Syntax

| Category | Examples |
|---|---|
| Superscript / subscript | `x^2`, `a_n`, `x^{i+1}_{k}` |
| Fractions | `\frac{a}{b}`, `\binom{n}{k}` |
| Square root | `\sqrt{x}`, `\sqrt[3]{x}` |
| Greek letters | `\alpha \beta \gamma \delta \pi \Sigma \Omega ...` |
| Binary operators | `+ - \times \div \pm \mp \cdot` |
| Relations | `= \neq \leq \geq \approx \equiv \sim` |
| Arrows | `\to \leftarrow \Rightarrow` |
| Big operators | `\sum \prod \int \iint \iiint \oint \bigcap \bigcup` |
| Dots | `\cdots \ldots \vdots \ddots` |
| Accents | `\vec{v}`, `\hat{x}`, `\bar{z}`, `\dot{f}`, `\ddot{f}`, `\tilde{a}` |
| Math italic | Letters auto-italicized; `\mathrm{text}` for upright |
| Blackboard bold | `\mathbb{R}`, `\mathbb{C}`, `\mathbb{N}`, `\mathbb{Z}`, `\mathbb{Q}` |
| Delimiters | `\left( \right)`, `\left[ \right]`, `\left\{ \right\}`, `\left\| \right\|` |
| Stacks | `\overset{a}{b}`, `\underset{a}{b}`, `\stackrel{a}{b}` |
| Over/underbrace | `\overbrace{a+b}^{n}`, `\underbrace{a+b}_{n}` |
| Matrices | `\begin{matrix}...\end{matrix}`, `pmatrix`, `bmatrix`, `vmatrix` |
| Aligned equations | `\begin{aligned}...\end{aligned}` |
| Cases | `\begin{cases}...\end{cases}` |
| Color | `\color{#RRGGBB}{expr}` or `\color{name}{expr}` |
| Spacing | `\quad`, `\qquad`, `\ `, `\,`, `\;` |
| Cancellation | `\cancel{x}`, `\bcancel{x}` |
| Box | `\boxed{expr}` |
| Minus sign | `-` is automatically mapped to U+2212 (proper math minus) |
