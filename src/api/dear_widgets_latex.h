// dear_widgets_latex.h -- LaTeX math expression parser and layout engine.
// Internal header for dear_widgets_latex.cpp (standalone TU).
// Inspired by MicroTeX's architecture: tokenize -> parse -> box tree -> layout -> render.
#pragma once

namespace ImWidgets {

// ---- Box types for the layout tree ----
enum LaTeXBoxType {
	LaTeXBox_Glyph,     // single character/glyph
	LaTeXBox_HBox,      // horizontal box (children laid out left-to-right)
	LaTeXBox_VBox,      // vertical box (children stacked: numerator/denominator)
	LaTeXBox_Script,    // base with optional superscript/subscript
	LaTeXBox_Frac,      // fraction: numerator over denominator with rule line
	LaTeXBox_Sqrt,      // square root
	LaTeXBox_Space,     // horizontal spacing
	LaTeXBox_Matrix,    // matrix/vector: grid of cells (children = row-major cells)
	LaTeXBox_Delim,     // left/right delimiter pair wrapping content
};

struct LaTeXBox {
	LaTeXBoxType type;
	// Layout results (filled by layout pass)
	float width, height, depth;  // depth = distance below baseline
	float shiftX, shiftY;       // offset from parent's reference point

	// Glyph box
	ImWchar codepoint;           // Unicode codepoint for glyph boxes
	bool    isMathItalic;        // use math italic variant
	char    text[64];            // shaped text string (rendered as single DrawText call via kbts)

	// Container children
	ImVector<LaTeXBox*> children;

	// Fraction-specific
	float ruleThickness;

	// Script-specific
	LaTeXBox* base;
	LaTeXBox* superscript;
	LaTeXBox* subscript;

	// Matrix-specific
	int matRows, matCols;        // grid dimensions
	ImWchar delimLeft, delimRight; // delimiter codepoints (0 = none)

	// Style (relative sizing)
	float sizeFactor;  // 1.0 = display, 0.7 = script, 0.5 = scriptscript
	ImU32 colorOverride; // 0 = inherit parent color

	LaTeXBox()
		: type(LaTeXBox_Glyph), width(0.f), height(0.f), depth(0.f),
		  shiftX(0.f), shiftY(0.f), codepoint(0), isMathItalic(false),
		  ruleThickness(0.f), base(nullptr), superscript(nullptr), subscript(nullptr),
		  matRows(0), matCols(0), delimLeft(0), delimRight(0),
		  sizeFactor(1.0f), colorOverride(0)
	{ memset(text, 0, sizeof(text)); }
	~LaTeXBox() {
		for (int i = 0; i < children.Size; i++) IM_DELETE(children[i]);
		if (base) IM_DELETE(base);
		if (superscript) IM_DELETE(superscript);
		if (subscript) IM_DELETE(subscript);
	}
};

// ---- Parser: LaTeX string -> box tree ----
LaTeXBox* LaTeXParse(const char* latex);

// ---- Layout: compute widths/heights/positions ----
void LaTeXLayout(LaTeXBox* box, float fontSize);

// ---- Render: draw the laid-out box tree with Slug ----
void LaTeXRender(ImDrawList* drawList, LaTeXBox* box, ImFont* mathFont, float fontSize, ImVec2 pos, ImU32 col);

// ---- Measure: get bounding box ----
ImVec2 LaTeXMeasure(LaTeXBox* box);

// ---- Font loading ----
ImFont* LaTeXGetMathFont();
void    LaTeXLoadMathFont();  // Call during init, before CreateContext

} // namespace ImWidgets
