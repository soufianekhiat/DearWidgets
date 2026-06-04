# LaTeX Math Rendering

## Overview

DearWidgets includes a self-contained LaTeX math typesetting engine. It parses a subset of LaTeX math mode syntax and lays out the result using a TeX-inspired box model, then renders it via the Slug GPU font pipeline using Latin Modern Math.

Architecture mirrors [MicroTeX](https://github.com/NanoMichael/MicroTeX): **tokenize -> parse -> box tree -> layout -> render**.

---

## Pipeline

```
latex string
    |
    v
Tokenizer          (Tokenizer::Next)
    |  TOK_CHAR, TOK_CMD, TOK_LBRACE, TOK_RBRACE, TOK_SUPER, TOK_SUB
    v
Parser             (ParseExpr / ParseAtom)
    |  builds a tree of LaTeXBox nodes
    v
Layout             (LaTeXLayout)
    |  computes width / height / depth / shiftX / shiftY for each node
    v
Render             (RenderBox -> ImWidgets::DrawText / Slug)
```

---

## Box Model

Each node in the tree is a `LaTeXBox` with one of these types:

| Type | Description |
|---|---|
| `LaTeXBox_Glyph` | Single Unicode codepoint, rendered as a Slug glyph |
| `LaTeXBox_HBox` | Horizontal box -- children laid out left-to-right |
| `LaTeXBox_VBox` | Vertical box -- numerator/denominator stacking |
| `LaTeXBox_Script` | Base with optional superscript and/or subscript |
| `LaTeXBox_Frac` | Fraction: numerator, rule line, denominator |
| `LaTeXBox_Sqrt` | Square root with radical sign |
| `LaTeXBox_Space` | Horizontal spacing (quad, thin space, etc.) |
| `LaTeXBox_Matrix` | Grid: row-major array of cells (matrix, cases, aligned) |
| `LaTeXBox_Delim` | Stretchy delimiter pair (`\left...\right`) |

Box metrics follow the TeX convention:
- **width**: horizontal extent
- **height**: extent above the baseline
- **depth**: extent below the baseline

---

## Size Factors

Scripts and nested fractions use relative `sizeFactor` scaling:

| Context | `sizeFactor` |
|---|---|
| Display / base | `1.0` |
| Superscript / subscript | `0.7` |
| Sub-superscript | `0.5` |

---

## Math Font

The renderer uses **Latin Modern Math** (LM Math), an OpenType math font with:
- Full Unicode math coverage (U+0000-U+1FFFF)
- Glyph assembly tables for large delimiters (`\left(`, `\right)`, etc.)
- MATH table for axis height, rule thickness, and script kerning

The font is loaded via `LoadLaTeXFont()` and rendered through Slug.

---

## Character Substitutions

The tokenizer applies the following automatic substitutions before the parser sees a character:

| Input | Codepoint | Reason |
|---|---|---|
| `-` (hyphen-minus, ASCII 0x002D) | `-` (U+2212) | Proper mathematical minus sign |

Letters `a-z`, `A-Z` are automatically rendered in math italic. Numbers and operators are upright. Use `\mathrm{...}` or `\textrm{...}` to force upright.

---

## Delimiter Sizing

`\left(...\right)` delimiters stretch vertically to fit the enclosed content. The MATH glyph assembly tables (horizontal and vertical) from the font are used for large sizes; fixed-size glyphs are used for small ones. Assembly parts are synthesized on-demand by the Slug draw system.

---

## Supported Commands Reference

See the complete list in [api/latex.md -- Supported LaTeX Syntax](../api/latex.md#supported-latex-syntax).

---

## Limitations

- **Math mode only.** Text mode (`\text{...}` / `\mathrm{...}`) is supported for inline roman text, but full LaTeX document mode (environments, cross-references, `\newcommand`, etc.) is not.
- **No line breaking.** Expressions are rendered on a single horizontal line.
- **No number alignment.** Multi-line `aligned` environments align on `&` markers but do not number equations.
- **No TikZ / PGF.** Drawing macros are not supported.
- **Requires Slug.** All rendering goes through the GPU Slug pipeline; CPU fallback is not available for LaTeX.
