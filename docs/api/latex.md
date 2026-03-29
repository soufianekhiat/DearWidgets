# LaTeX Math Rendering

> **Requires:** `ImWidgetsFeatures_LaTeX` (implies `ImWidgetsFeatures_RichFont`) set before `CreateContext()`.

DearWidgets includes a self-contained LaTeX math renderer: tokenizer → parser → box tree → layout → render, using Latin Modern Math as the math font rendered via the Slug GPU pipeline.

See [concepts/latex.md](../concepts/latex.md) for supported syntax and architecture.

---

## Setup

Call `LoadLaTeXFont()` **during the font loading phase**, before `ImGui::CreateContext()`:

```cpp
ImWidgets::SetFeatures(ImWidgetsFeatures_LaTeX);
ImWidgets::LoadLaTeXFont();           // loads Latin Modern Math
// ... add other fonts ...
ImGui::CreateContext();
ImWidgetsContext* ctx = ImWidgets::CreateContext();
```

---

## Functions

### `LoadLaTeXFont`
```cpp
void LoadLaTeXFont();
```
Loads the bundled Latin Modern Math TTF into the ImGui font atlas. Must be called before `ImGui::CreateContext()`.

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
ImWidgetsShape shape;
ImWidgets::TesselateLaTeX(24.0f, "E = mc^2", pos, shape);
ImWidgets::DrawLinearGradientText(dl, nullptr, 0, pos, "E = mc^2",
    pos, pos + sz, col0, col1);
// Or directly:
ImWidgets::ShapeSRGBLinearGradient(shape, pos, pos + sz, col0, col1);
ImWidgets::DrawShape(dl, shape);
```

---

## Supported LaTeX Syntax

| Category | Examples |
|---|---|
| Superscript / subscript | `x^2`, `a_n`, `x^{i+1}_{k}` |
| Fractions | `\frac{a}{b}`, `\binom{n}{k}` |
| Square root | `\sqrt{x}`, `\sqrt[3]{x}` |
| Greek letters | `\alpha \beta \gamma \delta \pi \Sigma \Omega …` |
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
