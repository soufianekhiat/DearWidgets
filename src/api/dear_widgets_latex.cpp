// dear_widgets_latex.cpp — LaTeX math expression parser, layout, and renderer.
// Uses Slug GPU font rendering for glyph output.
// This file is #included from dear_widgets.cpp — do NOT compile it separately.
#ifdef _DEAR_WIDGETS_LATEX_INCLUDED
#include "dear_widgets_latex.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

namespace ImWidgets {

// ---- Minimal OpenType MATH table parser for extensible glyph assembly ----
// Reads MathVariants to get horizontal/vertical glyph construction pieces.

struct MathGlyphPart {
	int glyphID;
	float startConnector; // em-space
	float endConnector;   // em-space
	float fullAdvance;    // em-space
	bool  isExtender;     // can be repeated
};

struct MathGlyphAssembly {
	MathGlyphPart parts[16];
	int partCount;
	float italicsCorrection; // em-space
};

// Read big-endian values from font data
static uint16_t MathR16(const uint8_t* p) { return (uint16_t)((p[0] << 8) | p[1]); }
static int16_t  MathRS16(const uint8_t* p) { return (int16_t)((p[0] << 8) | p[1]); }
static uint32_t MathR32(const uint8_t* p) { return (uint32_t)((p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3]); }

// Find the MATH table in the font
static const uint8_t* MathFindTable(const stbtt_fontinfo* fi, uint32_t tag, uint32_t* outLen) {
	const uint8_t* data = fi->data + fi->fontstart;
	int numTables = MathR16(data + 4);
	const uint8_t* rec = data + 12;
	for (int i = 0; i < numTables; i++, rec += 16) {
		uint32_t t = MathR32(rec);
		if (t == tag) {
			uint32_t off = MathR32(rec + 8);
			if (outLen) *outLen = MathR32(rec + 12);
			return fi->data + fi->fontstart + off;
		}
	}
	return NULL;
}

// Get horizontal glyph assembly for a given glyph (e.g., overbrace)
static bool MathGetHAssembly(const stbtt_fontinfo* fi, int glyphID, float emScale, MathGlyphAssembly* out) {
	out->partCount = 0;
	uint32_t mathLen = 0;
	const uint8_t* math = MathFindTable(fi, 0x4D415448 /*'MATH'*/, &mathLen);
	if (!math) return false;
	// MATH header: majorVersion(2), minorVersion(2), MathConstants offset(2), MathGlyphInfo offset(2), MathVariants offset(2)
	uint16_t varOff = MathR16(math + 8);
	if (!varOff) return false;
	const uint8_t* var = math + varOff;
	// MathVariants: MinConnectorOverlap(2), VertGlyphCoverage(2), HorizGlyphCoverage(2),
	//               VertGlyphCount(2), HorizGlyphCount(2), then VertGlyphConstruction[], HorizGlyphConstruction[]
	uint16_t horizCovOff = MathR16(var + 4);
	uint16_t vertCount   = MathR16(var + 6);
	uint16_t horizCount  = MathR16(var + 8);
	if (!horizCovOff || !horizCount) return false;
	// Parse Coverage table to find index of our glyphID
	const uint8_t* cov = var + horizCovOff;
	uint16_t covFmt = MathR16(cov);
	int covIdx = -1;
	if (covFmt == 1) { // Coverage Format 1: list of glyph IDs
		uint16_t cnt = MathR16(cov + 2);
		for (int i = 0; i < cnt; i++)
			if (MathR16(cov + 4 + i * 2) == (uint16_t)glyphID) { covIdx = i; break; }
	} else if (covFmt == 2) { // Coverage Format 2: ranges
		uint16_t cnt = MathR16(cov + 2);
		for (int i = 0; i < cnt; i++) {
			uint16_t startGI = MathR16(cov + 4 + i * 6);
			uint16_t endGI   = MathR16(cov + 4 + i * 6 + 2);
			uint16_t startCI = MathR16(cov + 4 + i * 6 + 4);
			if ((uint16_t)glyphID >= startGI && (uint16_t)glyphID <= endGI) { covIdx = startCI + (glyphID - startGI); break; }
		}
	}
	if (covIdx < 0 || covIdx >= horizCount) return false;
	// MathGlyphConstruction offset array starts at var + 10 + vertCount*2
	uint16_t constrOff = MathR16(var + 10 + vertCount * 2 + covIdx * 2);
	if (!constrOff) return false;
	const uint8_t* constr = var + constrOff;
	// MathGlyphConstruction: GlyphAssembly offset(2), VariantCount(2), MathGlyphVariantRecord[]
	uint16_t asmOff = MathR16(constr);
	if (!asmOff) return false;
	const uint8_t* asmb = constr + asmOff;
	// GlyphAssembly: ItalicsCorrection MathValueRecord(4), PartCount(2), GlyphPartRecord[]
	out->italicsCorrection = MathRS16(asmb) * emScale;
	uint16_t partCnt = MathR16(asmb + 4);
	if (partCnt > 16) partCnt = 16;
	out->partCount = partCnt;
	const uint8_t* pr = asmb + 6;
	for (int i = 0; i < partCnt; i++, pr += 10) {
		out->parts[i].glyphID        = MathR16(pr);
		out->parts[i].startConnector = MathR16(pr + 2) * emScale;
		out->parts[i].endConnector   = MathR16(pr + 4) * emScale;
		out->parts[i].fullAdvance    = MathR16(pr + 6) * emScale;
		out->parts[i].isExtender     = (MathR16(pr + 8) & 1) != 0;
	}
	return true;
}

// ---- Greek letter and command mapping ----
struct LaTeXCommand { const char* name; ImWchar codepoint; };
static const LaTeXCommand kCommands[] = {
	// Lowercase Greek
	{"alpha",    0x03B1}, {"beta",     0x03B2}, {"gamma",    0x03B3}, {"delta",    0x03B4},
	{"epsilon",  0x03F5}, {"zeta",     0x03B6}, {"eta",      0x03B7}, {"theta",    0x03B8},
	{"iota",     0x03B9}, {"kappa",    0x03BA}, {"lambda",   0x03BB}, {"mu",       0x03BC},
	{"nu",       0x03BD}, {"xi",       0x03BE}, {"pi",       0x03C0}, {"rho",      0x03C1},
	{"sigma",    0x03C3}, {"tau",      0x03C4}, {"upsilon",  0x03C5}, {"phi",      0x03D5},
	{"chi",      0x03C7}, {"psi",      0x03C8}, {"omega",    0x03C9},
	{"varepsilon",0x03B5},{"vartheta", 0x03D1}, {"varphi",   0x03C6}, {"varrho",   0x03F1},
	// Uppercase Greek
	{"Gamma",    0x0393}, {"Delta",    0x0394}, {"Theta",    0x0398}, {"Lambda",   0x039B},
	{"Xi",       0x039E}, {"Pi",       0x03A0}, {"Sigma",    0x03A3}, {"Phi",      0x03A6},
	{"Psi",      0x03A8}, {"Omega",    0x03A9},
	// Operators and symbols
	{"times",    0x00D7}, {"div",      0x00F7}, {"cdot",     0x22C5}, {"pm",       0x00B1},
	{"mp",       0x2213}, {"leq",      0x2264}, {"geq",      0x2265}, {"neq",      0x2260},
	{"approx",   0x2248}, {"equiv",    0x2261}, {"infty",    0x221E}, {"partial",  0x2202},
	{"nabla",    0x2207}, {"forall",   0x2200}, {"exists",   0x2203}, {"in",       0x2208},
	{"notin",    0x2209}, {"subset",   0x2282}, {"supset",   0x2283}, {"cup",      0x222A},
	{"cap",      0x2229}, {"emptyset", 0x2205}, {"rightarrow",0x2192},{"leftarrow",0x2190},
	{"Rightarrow",0x21D2},{"Leftarrow",0x21D0},{"leftrightarrow",0x2194},
	// Big operators
	{"sum",      0x2211}, {"prod",     0x220F}, {"int",      0x222B},
	{"iint",     0x222C}, {"iiint",    0x222D}, {"oint",     0x222E},
	// Misc
	{"sqrt",     0x221A}, {"langle",   0x27E8}, {"rangle",   0x27E9},
	{"ldots",    0x2026}, {"cdots",    0x22EF}, {"vdots",    0x22EE}, {"ddots",    0x22F1},
	// Arrows
	{"to",       0x2192}, {"gets",     0x2190}, {"mapsto",   0x21A6},
	{"uparrow",  0x2191}, {"downarrow",0x2193}, {"updownarrow",0x2195},
	{"Uparrow",  0x21D1}, {"Downarrow",0x21D3},
	{"iff",      0x27FA}, {"implies",  0x27F9},
	// Set/logic
	{"land",     0x2227}, {"lor",      0x2228}, {"neg",      0x00AC}, {"lnot",     0x00AC},
	{"setminus", 0x2216}, {"complement",0x2201},
	{"subseteq", 0x2286}, {"supseteq", 0x2287},
	// Relations
	{"sim",      0x223C}, {"simeq",    0x2243}, {"cong",     0x2245},
	{"propto",   0x221D}, {"prec",     0x227A}, {"succ",     0x227B},
	{"perp",     0x22A5}, {"parallel", 0x2225},
	// Misc math
	{"hbar",     0x210F}, {"ell",      0x2113}, {"Re",       0x211C}, {"Im",       0x2111},
	{"aleph",    0x2135}, {"wp",       0x2118},
	{"prime",    0x2032}, {"angle",    0x2220}, {"triangle", 0x25B3},
	{"star",     0x22C6}, {"circ",     0x2218}, {"bullet",   0x2219},
	{"oplus",    0x2295}, {"otimes",   0x2297}, {"odot",     0x2299},
	// Delimiters
	{"lfloor",   0x230A}, {"rfloor",   0x230B}, {"lceil",    0x2308}, {"rceil",    0x2309},
	{"lbrace",   0x007B}, {"rbrace",   0x007D}, {"vert",     0x007C}, {"Vert",     0x2016},
	// Accents (handled as commands with argument)
	{"dot",      0x02D9}, {"ddot",     0x00A8}, {"tilde",    0x02DC}, {"vec",      0x20D7},
	// Number sets (double-struck)
	{"mathbb",   0},  // handled specially in parser
	// Spacing commands (handled specially)
	{"quad",     0}, {"qquad",    0}, {",", 0}, {";", 0}, {"!", 0},
};
static const int kCommandCount = sizeof(kCommands) / sizeof(kCommands[0]);

static ImWchar LookupCommand(const char* name, int len) {
	for (int i = 0; i < kCommandCount; i++)
		if ((int)strlen(kCommands[i].name) == len && strncmp(kCommands[i].name, name, len) == 0)
			return kCommands[i].codepoint;
	return 0;
}

// ---- Tokenizer ----
enum TokenType { TOK_CHAR, TOK_CMD, TOK_LBRACE, TOK_RBRACE, TOK_SUPER, TOK_SUB, TOK_END };
struct Token { TokenType type; ImWchar ch; char cmd[32]; };

struct Tokenizer {
	const char* p;
	Token Peek() {
		const char* save = p;
		Token t = Next();
		p = save;
		return t;
	}
	Token Next() {
		Token t; memset(&t, 0, sizeof(t)); t.type = TOK_END;
		while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
		if (!*p) return t;
		if (*p == '{') { t.type = TOK_LBRACE; p++; return t; }
		if (*p == '}') { t.type = TOK_RBRACE; p++; return t; }
		if (*p == '^') { t.type = TOK_SUPER; p++; return t; }
		if (*p == '_') { t.type = TOK_SUB; p++; return t; }
		if (*p == '\\') {
			p++;
			if (*p == '\\') { t.type = TOK_CMD; t.cmd[0] = '\\'; t.cmd[1] = 0; p++; return t; } // \\ = line break
			if (*p == '{' || *p == '}') { t.type = TOK_CHAR; t.ch = *p++; return t; }
			// Single non-alpha char commands: \, \; \! \& etc.
			if (*p && !isalpha((unsigned char)*p)) {
				t.cmd[0] = *p++; t.cmd[1] = 0;
				ImWchar cp2 = LookupCommand(t.cmd, 1);
				if (cp2) { t.type = TOK_CHAR; t.ch = cp2; return t; }
				t.type = TOK_CMD; return t; // spacing commands like \, \; \!
			}
			// Read alpha command name
			int i = 0;
			while (isalpha((unsigned char)*p) && i < 30) t.cmd[i++] = *p++;
			t.cmd[i] = 0;
			// Structural commands (parser handles these)
			if (strcmp(t.cmd, "frac") == 0 || strcmp(t.cmd, "sqrt") == 0 ||
			    strcmp(t.cmd, "begin") == 0 || strcmp(t.cmd, "end") == 0 ||
			    strcmp(t.cmd, "quad") == 0 || strcmp(t.cmd, "qquad") == 0 ||
			    strcmp(t.cmd, "mathbb") == 0 || strcmp(t.cmd, "vec") == 0 ||
			    strcmp(t.cmd, "hat") == 0 || strcmp(t.cmd, "bar") == 0 ||
			    strcmp(t.cmd, "dot") == 0 || strcmp(t.cmd, "ddot") == 0 ||
			    strcmp(t.cmd, "tilde") == 0 ||
			    strcmp(t.cmd, "binom") == 0 || strcmp(t.cmd, "boxed") == 0 ||
			    strcmp(t.cmd, "cancel") == 0 || strcmp(t.cmd, "bcancel") == 0 ||
			    strcmp(t.cmd, "color") == 0 || strcmp(t.cmd, "text") == 0 ||
			    strcmp(t.cmd, "mathrm") == 0 || strcmp(t.cmd, "textrm") == 0 ||
			    strcmp(t.cmd, "overset") == 0 || strcmp(t.cmd, "underset") == 0 ||
			    strcmp(t.cmd, "stackrel") == 0 ||
			    strcmp(t.cmd, "overbrace") == 0 || strcmp(t.cmd, "underbrace") == 0) {
				t.type = TOK_CMD; return t;
			}
			// Greek/symbol lookup
			ImWchar cp = LookupCommand(t.cmd, i);
			if (cp) { t.type = TOK_CHAR; t.ch = cp; return t; }
			// Unknown command — emit as CMD for parser to handle
			t.type = TOK_CMD;
			return t;
		}
		// Regular character
		t.type = TOK_CHAR;
		// UTF-8 decode
		unsigned char c = (unsigned char)*p;
		if (c < 0x80) { t.ch = (c == '-') ? (ImWchar)0x2212 : (ImWchar)c; p++; } // hyphen → minus sign U+2212
		else {
			unsigned int cp = 0;
			p += ImTextCharFromUtf8(&cp, p, p + 4);
			t.ch = (ImWchar)cp;
		}
		return t;
	}
};

// Recursively set all glyphs in a subtree to upright (non-italic)
static void SetUpright(LaTeXBox* box) {
	if (!box) return;
	if (box->type == LaTeXBox_Glyph) box->isMathItalic = false;
	for (int i = 0; i < box->children.Size; i++) SetUpright(box->children[i]);
	SetUpright(box->base);
	SetUpright(box->superscript);
	SetUpright(box->subscript);
}

// ---- Parser: recursive descent ----
// Grammar:
//   expr     = atom ('^' atom)? ('_' atom)?  |  expr expr
//   atom     = CHAR | '{' expr '}' | '\frac{' expr '}{' expr '}' | '\sqrt{' expr '}'
//   CHAR     = letter | digit | operator

static LaTeXBox* ParseExpr(Tokenizer& tok, float sizeFactor);

static LaTeXBox* ParseAtom(Tokenizer& tok, float sizeFactor) {
	Token t = tok.Next();
	if (t.type == TOK_END || t.type == TOK_RBRACE) return NULL;

	if (t.type == TOK_LBRACE) {
		// Group: { expr }
		LaTeXBox* box = ParseExpr(tok, sizeFactor);
		Token closing = tok.Next(); // consume '}'
		(void)closing;
		return box;
	}

	if (t.type == TOK_CMD) {
		// Style hints that don't produce output and don't consume arguments — skip silently.
		// The caller's loop will continue and parse the next real atom.
		if (strcmp(t.cmd, "displaystyle") == 0 || strcmp(t.cmd, "textstyle") == 0 ||
		    strcmp(t.cmd, "scriptstyle") == 0 || strcmp(t.cmd, "scriptscriptstyle") == 0 ||
		    strcmp(t.cmd, "nolimits") == 0 || strcmp(t.cmd, "limits") == 0 ||
		    strcmp(t.cmd, "left") == 0 || strcmp(t.cmd, "right") == 0 ||
		    strcmp(t.cmd, "big") == 0 || strcmp(t.cmd, "Big") == 0 ||
		    strcmp(t.cmd, "bigg") == 0 || strcmp(t.cmd, "Bigg") == 0 ||
		    strcmp(t.cmd, "rlap") == 0 || strcmp(t.cmd, "llap") == 0 ||
		    strcmp(t.cmd, "hspace") == 0 || strcmp(t.cmd, "vspace") == 0 ||
		    strcmp(t.cmd, "mkern") == 0 || strcmp(t.cmd, "kern") == 0 ||
		    strcmp(t.cmd, "mbox") == 0 || strcmp(t.cmd, "hbox") == 0) {
			// Return a zero-width space so the parser continues without consuming real content
			LaTeXBox* sp = IM_NEW(LaTeXBox); sp->type = LaTeXBox_Space;
			sp->sizeFactor = sizeFactor; sp->width = 0; return sp;
		}
		// \text{...}, \mathrm{...}, \textrm{...} — upright (non-italic) text
		if (strcmp(t.cmd, "text") == 0 || strcmp(t.cmd, "mathrm") == 0 ||
		    strcmp(t.cmd, "textrm") == 0 || strcmp(t.cmd, "operatorname") == 0) {
			// Read raw text from braces into text[] — single DrawText call via kbts
			while (*tok.p == ' ') tok.p++;
			if (*tok.p == '{') {
				tok.p++; // consume {
				LaTeXBox* g = IM_NEW(LaTeXBox);
				g->type = LaTeXBox_Glyph;
				g->isMathItalic = false;
				g->sizeFactor = sizeFactor;
				int ti = 0;
				while (*tok.p && *tok.p != '}' && ti < 62) g->text[ti++] = *tok.p++;
				g->text[ti] = 0;
				if (*tok.p == '}') tok.p++;
				return g;
			}
			// Fallback: parse as normal atom
			LaTeXBox* arg = ParseAtom(tok, sizeFactor);
			SetUpright(arg);
			return arg;
		}
		// \overbrace{...} / \underbrace{...} — horizontal brace above/below content
		if (strcmp(t.cmd, "overbrace") == 0 || strcmp(t.cmd, "underbrace") == 0) {
			bool over = (t.cmd[0] == 'o');
			LaTeXBox* content = ParseAtom(tok, sizeFactor);
			LaTeXBox* vbox = IM_NEW(LaTeXBox);
			vbox->type = LaTeXBox_VBox;
			vbox->sizeFactor = sizeFactor;
			vbox->delimLeft = over ? 'O' : 'U'; // overbrace / underbrace marker
			vbox->delimRight = '{'; // brace marker
			if (content) vbox->children.push_back(content);
			return vbox;
		}
		// \overset{above}{main}, \stackrel{above}{main}
		if (strcmp(t.cmd, "overset") == 0 || strcmp(t.cmd, "stackrel") == 0) {
			LaTeXBox* above = ParseAtom(tok, sizeFactor * 0.7f);
			LaTeXBox* main  = ParseAtom(tok, sizeFactor);
			LaTeXBox* vbox = IM_NEW(LaTeXBox);
			vbox->type = LaTeXBox_VBox;
			vbox->sizeFactor = sizeFactor;
			vbox->delimLeft = 'O'; // overset
			if (main) vbox->children.push_back(main);
			if (above) vbox->children.push_back(above);
			return vbox;
		}
		// \underset{below}{main}
		if (strcmp(t.cmd, "underset") == 0) {
			LaTeXBox* below = ParseAtom(tok, sizeFactor * 0.7f);
			LaTeXBox* main  = ParseAtom(tok, sizeFactor);
			LaTeXBox* vbox = IM_NEW(LaTeXBox);
			vbox->type = LaTeXBox_VBox;
			vbox->sizeFactor = sizeFactor;
			vbox->delimLeft = 'U'; // underset
			if (main) vbox->children.push_back(main);
			if (below) vbox->children.push_back(below);
			return vbox;
		}
		// \binom{n}{k} — binomial coefficient (like frac but parens, no rule)
		if (strcmp(t.cmd, "binom") == 0) {
			LaTeXBox* frac = IM_NEW(LaTeXBox);
			frac->type = LaTeXBox_Frac;
			frac->sizeFactor = sizeFactor;
			frac->ruleThickness = -1.0f; // sentinel: no rule line
			frac->delimLeft = '(';
			frac->delimRight = ')';
			LaTeXBox* num = ParseAtom(tok, sizeFactor * 0.85f);
			LaTeXBox* den = ParseAtom(tok, sizeFactor * 0.85f);
			if (num) frac->children.push_back(num);
			if (den) frac->children.push_back(den);
			return frac;
		}
		// \boxed{expr} — box around expression
		if (strcmp(t.cmd, "boxed") == 0) {
			LaTeXBox* arg = ParseAtom(tok, sizeFactor);
			LaTeXBox* hbox = IM_NEW(LaTeXBox);
			hbox->type = LaTeXBox_HBox;
			hbox->sizeFactor = sizeFactor;
			hbox->delimRight = 0xFFFF; // sentinel: boxed mode
			if (arg) hbox->children.push_back(arg);
			return hbox;
		}
		// \cancel{x} / \bcancel{x} — diagonal strikethrough
		if (strcmp(t.cmd, "cancel") == 0 || strcmp(t.cmd, "bcancel") == 0) {
			LaTeXBox* arg = ParseAtom(tok, sizeFactor);
			LaTeXBox* hbox = IM_NEW(LaTeXBox);
			hbox->type = LaTeXBox_HBox;
			hbox->sizeFactor = sizeFactor;
			hbox->delimRight = (t.cmd[0] == 'b') ? 0xFFFE : 0xFFFD; // cancel sentinels
			if (arg) hbox->children.push_back(arg);
			return hbox;
		}
		// \color{name}{content} — colored sub-expression
		if (strcmp(t.cmd, "color") == 0) {
			while (*tok.p == ' ') tok.p++;
			char colorName[32] = {}; int ci = 0;
			if (*tok.p == '{') {
				tok.p++;
				while (*tok.p && *tok.p != '}' && ci < 30) colorName[ci++] = *tok.p++;
				if (*tok.p == '}') tok.p++;
			}
			ImU32 col = IM_COL32(255,255,255,255);
			if (strcmp(colorName, "red") == 0)           col = IM_COL32(255,0,0,255);
			else if (strcmp(colorName, "blue") == 0)     col = IM_COL32(60,60,255,255);
			else if (strcmp(colorName, "green") == 0)    col = IM_COL32(0,180,0,255);
			else if (strcmp(colorName, "yellow") == 0)   col = IM_COL32(255,255,0,255);
			else if (strcmp(colorName, "orange") == 0)   col = IM_COL32(255,165,0,255);
			else if (strcmp(colorName, "cyan") == 0)     col = IM_COL32(0,255,255,255);
			else if (strcmp(colorName, "magenta") == 0)  col = IM_COL32(255,0,255,255);
			else if (strcmp(colorName, "purple") == 0)   col = IM_COL32(128,0,128,255);
			else if (strcmp(colorName, "white") == 0)    col = IM_COL32(255,255,255,255);
			else if (strcmp(colorName, "gray") == 0)     col = IM_COL32(128,128,128,255);
			else if (strcmp(colorName, "black") == 0)    col = IM_COL32(0,0,0,255);
			else if (colorName[0] == '#' && strlen(colorName) == 7) {
				unsigned int r=0, g=0, b=0;
				sscanf(colorName+1, "%02x%02x%02x", &r, &g, &b);
				col = IM_COL32(r, g, b, 255);
			}
			LaTeXBox* content = ParseAtom(tok, sizeFactor);
			LaTeXBox* hbox = IM_NEW(LaTeXBox);
			hbox->type = LaTeXBox_HBox;
			hbox->sizeFactor = sizeFactor;
			hbox->colorOverride = col;
			if (content) hbox->children.push_back(content);
			return hbox;
		}
		// Commands that take one argument — parse the argument and return it (ignoring the style)
		if (strcmp(t.cmd, "mathit") == 0 ||
		    strcmp(t.cmd, "mathbf") == 0 ||
		    strcmp(t.cmd, "boldsymbol") == 0 || strcmp(t.cmd, "overline") == 0 ||
		    strcmp(t.cmd, "phantom") == 0 || strcmp(t.cmd, "hphantom") == 0 ||
		    strcmp(t.cmd, "vphantom") == 0 || strcmp(t.cmd, "smash") == 0 ||
		    strcmp(t.cmd, "textbf") == 0 ||
		    strcmp(t.cmd, "textit") == 0 ||
		    strcmp(t.cmd, "mathcal") == 0 || strcmp(t.cmd, "mathfrak") == 0 ||
		    strcmp(t.cmd, "mathnormal") == 0 || strcmp(t.cmd, "bm") == 0) {
			return ParseAtom(tok, sizeFactor);
		}
		// \mathbb{R} → double-struck letters (ℝ, ℕ, ℤ, ℚ, ℂ)
		if (strcmp(t.cmd, "mathbb") == 0) {
			LaTeXBox* arg = ParseAtom(tok, sizeFactor);
			if (arg && arg->type == LaTeXBox_Glyph) {
				ImWchar ch = arg->codepoint;
				// Map to double-struck Unicode block U+1D538+
				if (ch == 'C') arg->codepoint = 0x2102;
				else if (ch == 'H') arg->codepoint = 0x210D;
				else if (ch == 'N') arg->codepoint = 0x2115;
				else if (ch == 'P') arg->codepoint = 0x2119;
				else if (ch == 'Q') arg->codepoint = 0x211A;
				else if (ch == 'R') arg->codepoint = 0x211D;
				else if (ch == 'Z') arg->codepoint = 0x2124;
				else if (ch >= 'A' && ch <= 'Z') arg->codepoint = 0x1D538 + (ch - 'A');
				arg->isMathItalic = false;
			}
			return arg;
		}
		// Function names: \cos, \sin, \tan, \log, \ln, \exp, \lim, \min, \max, etc.
		// Rendered as upright (non-italic) text in an HBox.
		{
			static const char* kFuncNames[] = {
				"cos", "sin", "tan", "cot", "sec", "csc",
				"arccos", "arcsin", "arctan",
				"cosh", "sinh", "tanh", "coth",
				"log", "ln", "exp", "lim", "limsup", "liminf",
				"min", "max", "sup", "inf", "det", "dim",
				"ker", "deg", "gcd", "hom", "arg", "mod", "Pr",
			};
			bool isFunc = false;
			for (int fi = 0; fi < IM_ARRAYSIZE(kFuncNames); fi++) {
				if (strcmp(t.cmd, kFuncNames[fi]) == 0) { isFunc = true; break; }
			}
			if (isFunc) {
				// Single Glyph box with shaped text — rendered via kbts in one DrawText call
				LaTeXBox* hbox = IM_NEW(LaTeXBox);
				hbox->type = LaTeXBox_HBox;
				hbox->sizeFactor = sizeFactor;
				LaTeXBox* g = IM_NEW(LaTeXBox);
				g->type = LaTeXBox_Glyph;
				g->isMathItalic = false;
				g->sizeFactor = sizeFactor;
				strncpy(g->text, t.cmd, sizeof(g->text) - 1);
				hbox->children.push_back(g);
				// Add thin space after function name
				LaTeXBox* sp = IM_NEW(LaTeXBox);
				sp->type = LaTeXBox_Space;
				sp->sizeFactor = sizeFactor;
				sp->width = 0.17f;
				hbox->children.push_back(sp);
				return hbox;
			}
		}
		// \vec{x}, \hat{x}, \bar{x}, \dot{x}, \tilde{x} — accent over argument
		// Store accent type in delimLeft (unused by HBox) for layout/render
		if (strcmp(t.cmd, "vec") == 0 || strcmp(t.cmd, "hat") == 0 ||
		    strcmp(t.cmd, "bar") == 0 || strcmp(t.cmd, "dot") == 0 ||
		    strcmp(t.cmd, "ddot") == 0 || strcmp(t.cmd, "tilde") == 0) {
			ImWchar accentCode = 0;
			if (t.cmd[0] == 'v') accentCode = 0x20D7; // vec
			else if (t.cmd[0] == 'h') accentCode = 0x02C6; // hat
			else if (t.cmd[0] == 'b') accentCode = 0x00AF; // bar
			else if (strcmp(t.cmd, "ddot") == 0) accentCode = 0x00A8; // ddot
			else if (t.cmd[0] == 'd') accentCode = 0x02D9; // dot
			else if (t.cmd[0] == 't') accentCode = 0x02DC; // tilde
			LaTeXBox* arg = ParseAtom(tok, sizeFactor);
			LaTeXBox* hbox = IM_NEW(LaTeXBox);
			hbox->type = LaTeXBox_HBox;
			hbox->sizeFactor = sizeFactor;
			hbox->delimLeft = accentCode; // accent marker
			if (arg) hbox->children.push_back(arg);
			return hbox;
		}
		// Thin/medium/thick spacing
		if (strcmp(t.cmd, ",") == 0) {
			LaTeXBox* sp = IM_NEW(LaTeXBox); sp->type = LaTeXBox_Space;
			sp->sizeFactor = sizeFactor; sp->width = 0.17f; return sp;
		}
		if (strcmp(t.cmd, ";") == 0) {
			LaTeXBox* sp = IM_NEW(LaTeXBox); sp->type = LaTeXBox_Space;
			sp->sizeFactor = sizeFactor; sp->width = 0.28f; return sp;
		}
		if (strcmp(t.cmd, "!") == 0) {
			LaTeXBox* sp = IM_NEW(LaTeXBox); sp->type = LaTeXBox_Space;
			sp->sizeFactor = sizeFactor; sp->width = -0.17f; return sp; // negative thin space
		}
		// Matrix environments: \begin{pmatrix}...\end{pmatrix} etc.
		if (strcmp(t.cmd, "begin") == 0) {
			// Parse environment name: {pmatrix}, {bmatrix}, {vmatrix}, {matrix}
			// Skip whitespace before '{'
			while (*tok.p == ' ' || *tok.p == '\t' || *tok.p == '\n' || *tok.p == '\r') tok.p++;
			if (*tok.p == '{') tok.p++; // consume '{'
			char envName[32] = {}; int ei = 0;
			while (*tok.p && *tok.p != '}' && ei < 30) envName[ei++] = *tok.p++;
			if (*tok.p == '}') tok.p++;

			ImWchar delimL = 0, delimR = 0;
			ImWchar alignMarker = 0; // 0=center, 'C'=cases(left-align), 'A'=aligned(alternating)
			if (strcmp(envName, "pmatrix") == 0) { delimL = '('; delimR = ')'; }
			else if (strcmp(envName, "bmatrix") == 0) { delimL = '['; delimR = ']'; }
			else if (strcmp(envName, "vmatrix") == 0) { delimL = '|'; delimR = '|'; }
			else if (strcmp(envName, "Bmatrix") == 0) { delimL = '{'; delimR = '}'; }
			else if (strcmp(envName, "Vmatrix") == 0) { delimL = 0x2016; delimR = 0x2016; } // double vert
			else if (strcmp(envName, "cases") == 0) { delimL = '{'; delimR = 0; alignMarker = 'C'; }
			else if (strcmp(envName, "aligned") == 0) { alignMarker = 'A'; }

			// Parse cells: separated by '&' (columns) and '\\' (rows)
			LaTeXBox* mat = IM_NEW(LaTeXBox);
			mat->type = LaTeXBox_Matrix;
			mat->sizeFactor = sizeFactor;
			mat->delimLeft = delimL;
			mat->delimRight = delimR;
			mat->codepoint = alignMarker; // alignment mode
			int rows = 1, cols = 1, curCol = 1;

			// Parse first cell
			LaTeXBox* cell = ParseExpr(tok, sizeFactor);
			if (cell) mat->children.push_back(cell); else { LaTeXBox* e = IM_NEW(LaTeXBox); e->type = LaTeXBox_Space; e->sizeFactor = sizeFactor; e->width = 0; mat->children.push_back(e); }

			auto PushCell = [&](LaTeXBox* c) {
				if (c) mat->children.push_back(c);
				else { LaTeXBox* e = IM_NEW(LaTeXBox); e->type = LaTeXBox_Space; e->sizeFactor = sizeFactor; e->width = 0; mat->children.push_back(e); }
			};
			auto IsRowBreak = [](const Token& tk) {
				return (tk.type == TOK_CMD && tk.cmd[0] == '\\' && tk.cmd[1] == 0) ||
				       (tk.type == TOK_CMD && strcmp(tk.cmd, "cr") == 0);
			};

			while (true) {
				Token peek = tok.Peek();
				if (peek.type == TOK_END) break;
				if (peek.type == TOK_CHAR && peek.ch == '&') {
					tok.Next(); // consume &
					curCol++;
					if (curCol > cols) cols = curCol;
					PushCell(ParseExpr(tok, sizeFactor));
					continue;
				}
				if (IsRowBreak(peek)) {
					tok.Next(); // consume \\ or \cr
					rows++;
					curCol = 1;
					PushCell(ParseExpr(tok, sizeFactor));
					continue;
				}
				if (peek.type == TOK_CMD && strcmp(peek.cmd, "end") == 0) {
					tok.Next(); // consume \end
					// consume {envName}
					if (*tok.p == '{') { tok.p++; while (*tok.p && *tok.p != '}') tok.p++; if (*tok.p == '}') tok.p++; }
					break;
				}
				// Skip unknown commands inside matrix (like \hdotsfor{3})
				if (peek.type == TOK_CMD && strcmp(peek.cmd, "hdotsfor") == 0) {
					tok.Next(); // consume \hdotsfor
					ParseAtom(tok, sizeFactor); // consume {3} argument
					// Fill current row with dots
					LaTeXBox* dots = IM_NEW(LaTeXBox); dots->type = LaTeXBox_Glyph;
					dots->codepoint = 0x22EF; dots->sizeFactor = sizeFactor; dots->isMathItalic = false;
					PushCell(dots);
					// Pad remaining columns in this row
					while (curCol < cols) { curCol++; LaTeXBox* e = IM_NEW(LaTeXBox); e->type = LaTeXBox_Space; e->sizeFactor = sizeFactor; e->width = 0; mat->children.push_back(e); }
					continue;
				}
				break;
			}
			mat->matRows = rows;
			mat->matCols = cols;
			// Pad with empty cells if needed
			while (mat->children.Size < rows * cols) {
				LaTeXBox* e = IM_NEW(LaTeXBox); e->type = LaTeXBox_Space;
				e->sizeFactor = sizeFactor; e->width = 0; mat->children.push_back(e);
			}
			return mat;
		}
		if (strcmp(t.cmd, "frac") == 0) {
			LaTeXBox* frac = IM_NEW(LaTeXBox);
			frac->type = LaTeXBox_Frac;
			frac->sizeFactor = sizeFactor;
			// Parse numerator
			LaTeXBox* num = ParseAtom(tok, sizeFactor * 0.8f);
			// Parse denominator
			LaTeXBox* den = ParseAtom(tok, sizeFactor * 0.8f);
			if (num) frac->children.push_back(num);
			if (den) frac->children.push_back(den);
			return frac;
		}
		if (strcmp(t.cmd, "sqrt") == 0) {
			LaTeXBox* sq = IM_NEW(LaTeXBox);
			sq->type = LaTeXBox_Sqrt;
			sq->sizeFactor = sizeFactor;
			// Optional [n] for nth root
			while (*tok.p == ' ') tok.p++;
			if (*tok.p == '[') {
				tok.p++; // consume [
				// Read degree text until ]
				const char* start = tok.p;
				while (*tok.p && *tok.p != ']') tok.p++;
				if (*tok.p == ']') tok.p++;
				// Parse the degree as a sub-expression
				Tokenizer degTok; degTok.p = start;
				const char* savedEnd = tok.p;
				// Temporarily null-terminate... instead, just parse from saved string
				char degBuf[32] = {};
				int dlen = (int)(savedEnd - 1 - start); // exclude ]
				if (dlen > 0 && dlen < 31) { memcpy(degBuf, start, dlen); degBuf[dlen] = 0; }
				Tokenizer dt; dt.p = degBuf;
				sq->base = ParseExpr(dt, sizeFactor * 0.6f);
			}
			LaTeXBox* inner = ParseAtom(tok, sizeFactor);
			if (inner) sq->children.push_back(inner);
			return sq;
		}
		if (strcmp(t.cmd, "quad") == 0) {
			LaTeXBox* sp = IM_NEW(LaTeXBox); sp->type = LaTeXBox_Space;
			sp->sizeFactor = sizeFactor; sp->width = 1.0f; return sp;
		}
		if (strcmp(t.cmd, "qquad") == 0) {
			LaTeXBox* sp = IM_NEW(LaTeXBox); sp->type = LaTeXBox_Space;
			sp->sizeFactor = sizeFactor; sp->width = 2.0f; return sp;
		}
		// Unknown command — return zero-width space (don't consume next token)
		{ LaTeXBox* sp = IM_NEW(LaTeXBox); sp->type = LaTeXBox_Space;
		  sp->sizeFactor = sizeFactor; sp->width = 0; return sp; }
	}

	if (t.type == TOK_CHAR) {
		LaTeXBox* glyph = IM_NEW(LaTeXBox);
		glyph->type = LaTeXBox_Glyph;
		glyph->codepoint = t.ch;
		glyph->sizeFactor = sizeFactor;
		// Big operators render larger (display-style scaling)
		if (t.ch == 0x2211 || t.ch == 0x220F || t.ch == 0x222B || // sum, prod, int
		    t.ch == 0x222C || t.ch == 0x222D || t.ch == 0x222E || // iint, iiint, oint
		    t.ch == 0x2229 || t.ch == 0x222A ||                   // cap, cup
		    t.ch == 0x2A00 || t.ch == 0x2A01 || t.ch == 0x2A02)   // big operators
			glyph->sizeFactor = sizeFactor * 1.5f;
		// Letters are math italic, numbers/operators are upright
		glyph->isMathItalic = (t.ch >= 'a' && t.ch <= 'z') || (t.ch >= 'A' && t.ch <= 'Z');
		return glyph;
	}

	return NULL;
}

static LaTeXBox* ParseExpr(Tokenizer& tok, float sizeFactor) {
	LaTeXBox* hbox = IM_NEW(LaTeXBox);
	hbox->type = LaTeXBox_HBox;
	hbox->sizeFactor = sizeFactor;

	while (true) {
		Token peek = tok.Peek();
		if (peek.type == TOK_END || peek.type == TOK_RBRACE) break;
		// Stop at matrix cell/row separators so the matrix parser can handle them
		if (peek.type == TOK_CHAR && peek.ch == '&') break;
		if (peek.type == TOK_CMD && peek.cmd[0] == '\\' && peek.cmd[1] == 0) break; // backslash-backslash
		if (peek.type == TOK_CMD && strcmp(peek.cmd, "cr") == 0) break;              // \cr
		if (peek.type == TOK_CMD && strcmp(peek.cmd, "end") == 0) break;

		// Skip super/sub if we're at the hbox level (handled below)
		if (peek.type == TOK_SUPER || peek.type == TOK_SUB) {
			// Attach to last child
			if (hbox->children.Size > 0) {
				LaTeXBox* last = hbox->children[hbox->children.Size - 1];
				// Wrap in script box
				LaTeXBox* script = IM_NEW(LaTeXBox);
				script->type = LaTeXBox_Script;
				script->sizeFactor = sizeFactor;
				script->base = last;
				hbox->children[hbox->children.Size - 1] = script;

				while (true) {
					Token st = tok.Peek();
					if (st.type == TOK_SUPER) {
						tok.Next(); // consume ^
						script->superscript = ParseAtom(tok, sizeFactor * 0.7f);
					} else if (st.type == TOK_SUB) {
						tok.Next(); // consume _
						script->subscript = ParseAtom(tok, sizeFactor * 0.7f);
					} else break;
				}
			} else {
				tok.Next(); // skip orphan ^/_
			}
			continue;
		}

		LaTeXBox* atom = ParseAtom(tok, sizeFactor);
		if (!atom) break;
		hbox->children.push_back(atom);
	}

	// Flatten single-child hbox
	if (hbox->children.Size == 1) {
		LaTeXBox* child = hbox->children[0];
		hbox->children.Size = 0;
		IM_DELETE(hbox);
		return child;
	}
	if (hbox->children.Size == 0) {
		IM_DELETE(hbox);
		return NULL;
	}

	return hbox;
}

LaTeXBox* LaTeXParse(const char* latex) {
	Tokenizer tok; tok.p = latex;
	return ParseExpr(tok, 1.0f);
}

// ---- Layout ----
// Uses real font metrics from stbtt for glyph sizing.
// Based on standard TeX math layout: axis height, rule thickness, script shifts.

// Map ASCII letters to math italic Unicode range
static ImWchar MathItalicize(ImWchar ch) {
	if (ch >= 'A' && ch <= 'Z') return 0x1D434 + (ch - 'A');
	if (ch >= 'a' && ch <= 'z') return 0x1D44E + (ch - 'a');
	return ch;
}

// UTF-8 encode helper
static void EncodeUTF8(ImWchar ch, char* utf8) {
	memset(utf8, 0, 8);
	if (ch < 0x80) { utf8[0] = (char)ch; }
	else if (ch < 0x800) { utf8[0] = (char)(0xC0 | (ch >> 6)); utf8[1] = (char)(0x80 | (ch & 0x3F)); }
	else if (ch < 0x10000) { utf8[0] = (char)(0xE0 | (ch >> 12)); utf8[1] = (char)(0x80 | ((ch >> 6) & 0x3F)); utf8[2] = (char)(0x80 | (ch & 0x3F)); }
	else { utf8[0] = (char)(0xF0 | (ch >> 18)); utf8[1] = (char)(0x80 | ((ch >> 12) & 0x3F)); utf8[2] = (char)(0x80 | ((ch >> 6) & 0x3F)); utf8[3] = (char)(0x80 | (ch & 0x3F)); }
}

// Forward declarations for glyph measurement (defined in render section)
static float GlyphWidthAtH(ImFont* font, float baseSz, ImWchar ch, float targetH);
static float GlyphHeightAtW(ImFont* font, float baseSz, ImWchar ch, float targetW);

static int s_layoutDepth = 0;
static void LayoutBox(LaTeXBox* box, float fontSize) {
	if (!box) return;
	if (++s_layoutDepth > 100) { --s_layoutDepth; return; } // prevent stack overflow
	float sz = fontSize * box->sizeFactor;
	// TeX-standard math constants (fractions of em)
	float axisHeight   = sz * 0.25f;  // math axis (center of operators, fraction line)
	float ruleThick    = ImMax(1.0f, sz * 0.04f);
	float thinSpace    = sz * 0.17f;

	switch (box->type) {
	case LaTeXBox_Glyph: {
		ImFont* mf = LaTeXGetMathFont();
		float asc = 0;
		ImVec2 tsz;
		ImWchar ch = box->codepoint;
		if (box->text[0]) {
			// Shaped text string — measure the full string via kbts
			tsz = mf ? CalcTextSize_Impl(mf, sz, box->text, NULL, &asc) : ImVec2(sz * 0.5f, sz);
		} else {
			if (box->isMathItalic) ch = MathItalicize(ch);
			char utf8[8]; EncodeUTF8(ch, utf8);
			tsz = mf ? CalcTextSize_Impl(mf, sz, utf8, NULL, &asc) : ImVec2(sz * 0.5f, sz);
		}
		box->width  = tsz.x > 0 ? tsz.x : sz * 0.5f;
		box->height = asc > 0 ? asc : sz * 0.7f;          // ascent above baseline
		box->depth  = tsz.y - asc;                          // descent below baseline
		if (box->depth < 0) box->depth = 0;
		// Operators get thin space padding
		if (!box->text[0] && (ch == '+' || ch == 0x2212 || ch == '=' || ch == 0x00D7 || ch == 0x00F7 ||
		    ch == 0x2264 || ch == 0x2265 || ch == 0x2260 || ch == 0x2248 || ch == 0x2261 ||
		    ch == 0x2192 || ch == 0x2190 || ch == 0x21D2)) {
			box->width += thinSpace * 2;  // padding both sides
		}
		break;
	}
	case LaTeXBox_Space:
		box->width  = sz * box->width;
		box->height = 0;
		box->depth  = 0;
		break;
	case LaTeXBox_HBox: {
		float x = 0, maxH = 0, maxD = 0;
		for (int i = 0; i < box->children.Size; i++) {
			LayoutBox(box->children[i], fontSize);
			LaTeXBox* c = box->children[i];
			c->shiftX = x;
			c->shiftY = 0;
			x += c->width;
			// Account for shifted children: child shifted up by -shiftY increases height
			// child shifted down by +shiftY increases depth
			maxH = ImMax(maxH, c->height - c->shiftY);
			maxD = ImMax(maxD, c->depth + c->shiftY);
		}
		box->width = x; box->height = maxH; box->depth = maxD;
		// Accent: add space above for the accent mark
		if (box->delimLeft && box->delimLeft != 0xFFFF && box->delimLeft != 0xFFFE && box->delimLeft != 0xFFFD)
			box->height += sz * 0.25f;
		// Boxed: add padding
		if (box->delimRight == 0xFFFF) {
			float pad = sz * 0.15f;
			for (int i2 = 0; i2 < box->children.Size; i2++)
				box->children[i2]->shiftX += pad;
			box->width += pad * 2;
			box->height += pad;
			box->depth += pad;
		}
		break;
	}
	case LaTeXBox_Script: {
		if (box->base) LayoutBox(box->base, fontSize);
		if (box->superscript) LayoutBox(box->superscript, fontSize);
		if (box->subscript) LayoutBox(box->subscript, fontSize);
		float bw = box->base ? box->base->width : 0;
		float bh = box->base ? box->base->height : sz * 0.7f;
		float bd = box->base ? box->base->depth : 0;

		// Check if base is a big operator → use limits layout (above/below)
		bool isLimits = false;
		bool isIntegral = false;
		if (box->base && box->base->type == LaTeXBox_Glyph) {
			ImWchar cp = box->base->codepoint;
			isLimits = (cp == 0x2211 || cp == 0x220F ||                 // sum, prod
			            cp == 0x2229 || cp == 0x222A ||                 // cap, cup
			            cp == 0x2A00 || cp == 0x2A01 || cp == 0x2A02);  // big operators
			isIntegral = (cp == 0x222B || cp == 0x222C || cp == 0x222D || cp == 0x222E);
		}
		// Overbrace/underbrace: treat as operator with limits (^/_ go above/below)
		if (box->base && box->base->type == LaTeXBox_VBox && box->base->delimRight == '{')
			isLimits = true;

		float w = bw, h = bh, d = bd;

		if (isIntegral) {
			// Integral-style: side placement, positioned relative to glyph bounds
			// bh/bd are the integral glyph's actual ascent/descent (scaled 1.5x)
			float supShift = bh * 0.65f;   // near top of the integral sign
			float subShift = bd * 0.65f;   // near bottom of the integral sign
			if (box->superscript) {
				box->superscript->shiftX = bw;
				box->superscript->shiftY = -supShift;
				w = ImMax(w, bw + box->superscript->width);
				h = ImMax(h, supShift + box->superscript->height);
			}
			if (box->subscript) {
				box->subscript->shiftX = bw;
				box->subscript->shiftY = subShift;
				w = ImMax(w, bw + box->subscript->width);
				d = ImMax(d, subShift + box->subscript->depth);
			}
		} else if (isLimits) {
			// Display-style limits: scripts go ABOVE and BELOW the operator, centered
			float gap = sz * 0.08f;
			if (box->superscript) {
				float supW = box->superscript->width;
				float supH = box->superscript->height + box->superscript->depth;
				box->superscript->shiftX = (bw - supW) * 0.5f;
				box->superscript->shiftY = -(bh + gap + box->superscript->depth);
				w = ImMax(w, supW);
				h = bh + gap + supH;
			}
			if (box->subscript) {
				float subW = box->subscript->width;
				float subH = box->subscript->height + box->subscript->depth;
				box->subscript->shiftX = (bw - subW) * 0.5f;
				box->subscript->shiftY = bd + gap + box->subscript->height;
				w = ImMax(w, subW);
				d = bd + gap + subH;
			}
		} else {
			float supShift = sz * 0.4f;
			float subShift = sz * 0.2f;
			if (box->superscript) {
				box->superscript->shiftX = bw;
				box->superscript->shiftY = -supShift;
				w = ImMax(w, bw + box->superscript->width);
				h = ImMax(h, supShift + box->superscript->height);
				d = ImMax(d, box->superscript->depth - supShift);
			}
			if (box->subscript) {
				box->subscript->shiftX = bw;
				box->subscript->shiftY = subShift;
				w = ImMax(w, bw + box->subscript->width);
				d = ImMax(d, subShift + box->subscript->depth + box->subscript->height);
			}
		}
		if (d < 0) d = 0;
		box->width = w; box->height = h; box->depth = d;
		break;
	}
	case LaTeXBox_Frac: {
		LaTeXBox* num = box->children.Size > 0 ? box->children[0] : NULL;
		LaTeXBox* den = box->children.Size > 1 ? box->children[1] : NULL;
		LayoutBox(num, fontSize);
		LayoutBox(den, fontSize);
		float nw = num ? num->width : 0, nh = num ? (num->height + num->depth) : sz * 0.5f;
		float dw = den ? den->width : 0, dh = den ? (den->height + den->depth) : sz * 0.5f;
		float padH = sz * 0.15f;  // vertical gap between rule and content
		float padW = sz * 0.15f;
		float maxW = ImMax(nw, dw) + padW * 2;
		if (box->ruleThickness < 0) box->ruleThickness = 0; // binom: no rule
		else box->ruleThickness = ruleThick;

		float totalFracH = nh + dh + padH * 2 + ruleThick;
		float delimW = box->delimLeft ? GlyphWidthAtH(LaTeXGetMathFont(), sz, box->delimLeft, totalFracH) : 0;

		// Numerator: its bottom edge sits at (axisHeight + ruleThick/2 + padH) above baseline
		float numBottom = axisHeight + ruleThick * 0.5f + padH;
		float numShift = -(numBottom + (num ? num->depth : 0));
		// Denominator: its top edge sits at (axisHeight - ruleThick/2 - padH) below... wait
		// denTop = -(axisHeight - ruleThick/2 - padH) relative to baseline = below axis
		float denTop = -(axisHeight - ruleThick * 0.5f - padH);  // positive = below baseline
		float denShift = denTop + (den ? den->height : 0);

		if (num) { num->shiftX = delimW + (maxW - nw) * 0.5f; num->shiftY = numShift; }
		if (den) { den->shiftX = delimW + (maxW - dw) * 0.5f; den->shiftY = denShift; }

		// Total height above baseline = numBottom + numerator total height
		box->height = numBottom + nh;
		// Total depth below baseline = denTop + denominator total height
		box->depth  = denTop + dh;
		box->width  = maxW + delimW * 2;
		break;
	}
	case LaTeXBox_Sqrt: {
		LaTeXBox* inner = box->children.Size > 0 ? box->children[0] : NULL;
		LayoutBox(inner, fontSize);
		if (box->base) LayoutBox(box->base, fontSize); // nth root degree
		float iw = inner ? inner->width : 0;
		float padTop = sz * 0.15f;
		float padRight = sz * 0.08f;
		float contentH = (inner ? (inner->height + inner->depth) : sz * 0.7f) + padTop;
		float radW = GlyphWidthAtH(LaTeXGetMathFont(), sz, 0x221A, contentH);
		if (inner) { inner->shiftX = radW; inner->shiftY = 0; }
		box->width  = radW + iw + padRight;
		box->height = (inner ? inner->height : sz * 0.7f) + padTop;
		box->depth  = inner ? inner->depth : 0;
		// Position degree in crook — MicroTeX uses 55% of radical total height
		if (box->base) {
			float totalRadH = box->height + box->depth;
			float bottomRaise = totalRadH * 0.55f;
			box->base->shiftX = 0;
			box->base->shiftY = box->depth - box->base->depth - bottomRaise;
		}
		break;
	}
	case LaTeXBox_Matrix: {
		int rows = box->matRows, cols = box->matCols;
		if (rows <= 0) rows = 1;
		if (cols <= 0) cols = 1;
		float cellPadX = sz * 0.3f, cellPadY = sz * 0.15f;

		// Layout all cells
		for (int i = 0; i < box->children.Size; i++)
			LayoutBox(box->children[i], fontSize);

		// Compute column widths and row heights
		ImVector<float> colW, rowH, rowD;
		colW.resize(cols, 0); rowH.resize(rows, 0); rowD.resize(rows, 0);
		for (int r = 0; r < rows; r++) {
			for (int c = 0; c < cols; c++) {
				int idx = r * cols + c;
				if (idx >= box->children.Size) continue;
				LaTeXBox* cell = box->children[idx];
				if (!cell) continue;
				colW[c] = ImMax(colW[c], cell->width);
				rowH[r] = ImMax(rowH[r], cell->height);
				rowD[r] = ImMax(rowD[r], cell->depth);
			}
		}

		// Position cells
		float totalW = 0;
		for (int c = 0; c < cols; c++) totalW += colW[c];
		totalW += (cols - 1) * cellPadX;

		float totalH = 0;
		for (int r = 0; r < rows; r++) totalH += rowH[r] + rowD[r];
		totalH += (rows - 1) * cellPadY;

		float delimW = box->delimLeft ? GlyphWidthAtH(LaTeXGetMathFont(), sz, box->delimLeft, totalH) : 0;

		float cy = -totalH * 0.5f - axisHeight; // center vertically around math axis
		for (int r = 0; r < rows; r++) {
			float cx = delimW;
			float baseline = cy + rowH[r]; // baseline of this row
			for (int c = 0; c < cols; c++) {
				int idx = r * cols + c;
				if (idx < box->children.Size && box->children[idx]) {
					LaTeXBox* cell = box->children[idx];
					// Alignment: center (default), left (cases), alternating RL (aligned)
					float cellAlignX;
					if (box->codepoint == 'C') cellAlignX = 0; // cases: left-align
					else if (box->codepoint == 'A') cellAlignX = (c % 2 == 0) ? (colW[c] - cell->width) : 0; // aligned: R,L,R,L
					else cellAlignX = (colW[c] - cell->width) * 0.5f; // matrix: center
					cell->shiftX = cx + cellAlignX;
					cell->shiftY = baseline;
				}
				cx += colW[c] + cellPadX;
			}
			cy += rowH[r] + rowD[r] + cellPadY;
		}

		box->width = totalW + delimW * 2;
		box->height = totalH * 0.5f + axisHeight;
		box->depth = totalH * 0.5f - axisHeight;
		break;
	}
	case LaTeXBox_VBox: {
		LaTeXBox* main  = box->children.Size > 0 ? box->children[0] : NULL;
		LaTeXBox* annot = box->children.Size > 1 ? box->children[1] : NULL;
		if (main) LayoutBox(main, fontSize);
		if (annot) LayoutBox(annot, fontSize);
		float mw = main ? main->width : 0, mh = main ? main->height : 0, md = main ? main->depth : 0;
		float aw = annot ? annot->width : 0;
		float ah = annot ? (annot->height + annot->depth) : 0;
		float gap = sz * 0.08f;
		float braceH = 0;
		if (box->delimRight == '{') {
			// Compute brace height
			ImFont* mf = LaTeXGetMathFont();
			ImWchar cuspCh = (box->delimLeft == 'O') ? 0x23DE : 0x23DF;
			stbtt_fontinfo fi; float emSc = 0;
			bool hasMath = mf ? GetSlugFontInfo(mf, &fi, &emSc) : false;
			int braceGI = hasMath ? stbtt_FindGlyphIndex(&fi, cuspCh) : 0;
			MathGlyphAssembly asm2 = {};
			bool hasAsm = braceGI > 0 && MathGetHAssembly(&fi, braceGI, emSc, &asm2) && asm2.partCount > 0;
			if (hasAsm) {
				// Assembly part glyphs are built on-demand by DrawText (0x100000+ codepoints)
				// Compute same scaling as render will use
				float targetW2 = ImMax(mw, aw) / sz;
				float fixedW2 = 0; int extIdx2 = -1;
				for (int i2 = 0; i2 < asm2.partCount; i2++) {
					if (asm2.parts[i2].isExtender) extIdx2 = i2;
					else fixedW2 += asm2.parts[i2].fullAdvance;
				}
				float extAdv2 = (extIdx2 >= 0) ? asm2.parts[extIdx2].fullAdvance : 0;
				float ov2 = (extIdx2 >= 0) ? asm2.parts[extIdx2].startConnector * 0.5f : 0;
				int ext2 = 0;
				if (extAdv2 > ov2 && targetW2 > fixedW2) ext2 = (int)((targetW2 - fixedW2) / (extAdv2 - ov2));
				float totalW2 = 0; int jc2 = 0;
				for (int i2 = 0; i2 < asm2.partCount; i2++) {
					int cp2 = asm2.parts[i2].isExtender ? ext2 : 1;
					for (int c2 = 0; c2 < cp2; c2++) { if (jc2 > 0) totalW2 -= ov2; totalW2 += asm2.parts[i2].fullAdvance; jc2++; }
				}
				float braceSz2 = (totalW2 > 0.01f) ? sz * targetW2 / totalW2 : sz;
				// Measure cusp glyph at scaled size for height
				char cUtf8[8]; EncodeUTF8(cuspCh, cUtf8);
				float bAsc2 = 0;
				ImVec2 bRef2 = CalcTextSize_Impl(mf, braceSz2, cUtf8, NULL, &bAsc2);
				braceH = (box->delimLeft == 'O') ? bAsc2 : (bRef2.y - bAsc2);
			} else {
				char cUtf8[8]; EncodeUTF8(cuspCh, cUtf8);
				float bAsc = 0;
				ImVec2 bRef = CalcTextSize_Impl(mf, sz, cUtf8, NULL, &bAsc);
				braceH = (box->delimLeft == 'O') ? bAsc : (bRef.y - bAsc);
			}
		}

		if (box->delimLeft == 'O') {
			// Overset or overbrace: annotation/brace above main
			float maxW = ImMax(mw, aw);
			if (main) { main->shiftX = (maxW - mw) * 0.5f; main->shiftY = 0; }
			if (annot) { annot->shiftX = (maxW - aw) * 0.5f; annot->shiftY = -(mh + gap + braceH + (annot ? annot->depth : 0)); }
			box->width = maxW;
			box->height = mh + gap + braceH + ah;
			box->depth = md;
		} else {
			// Underset or underbrace: annotation/brace below main
			float maxW = ImMax(mw, aw);
			if (main) { main->shiftX = (maxW - mw) * 0.5f; main->shiftY = 0; }
			if (annot) { annot->shiftX = (maxW - aw) * 0.5f; annot->shiftY = md + gap + braceH + (annot ? annot->height : 0); }
			box->width = maxW;
			box->height = mh;
			box->depth = md + gap + braceH + ah;
		}
		break;
	}
	case LaTeXBox_Delim:
		break;
	}
	--s_layoutDepth;
}

void LaTeXLayout(LaTeXBox* box, float fontSize) {
	s_layoutDepth = 0;
	if (box) LayoutBox(box, fontSize);
}

ImVec2 LaTeXMeasure(LaTeXBox* box) {
	if (!box) return ImVec2(0, 0);
	return ImVec2(box->width, box->height + box->depth);
}

// Walk tree exactly like RenderBox but accumulate pixel bounds instead of drawing.
// x,y = baseline-relative position (same coordinate system as RenderBox).
static void MeasureBounds(LaTeXBox* box, float fontSize, float x, float y,
                          float* outMinX, float* outMinY, float* outMaxX, float* outMaxY)
{
	if (!box) return;
	float px = x + box->shiftX;
	float py = y + box->shiftY;

	switch (box->type) {
	case LaTeXBox_Glyph: {
		// Glyph occupies [px, px+width] x [py - height, py + depth] approximately
		// Simplify: use the box's own height/depth which are set from CalcTextSize
		*outMinX = ImMin(*outMinX, px);
		*outMaxX = ImMax(*outMaxX, px + box->width);
		*outMinY = ImMin(*outMinY, py - box->height);
		*outMaxY = ImMax(*outMaxY, py + box->depth);
		break;
	}
	case LaTeXBox_Space:
		break;
	case LaTeXBox_HBox:
		for (int i = 0; i < box->children.Size; i++)
			MeasureBounds(box->children[i], fontSize, px, py, outMinX, outMinY, outMaxX, outMaxY);
		// Include accent, boxed, or cancel area in bounds
		if (box->delimLeft || box->delimRight == 0xFFFF || box->delimRight == 0xFFFD || box->delimRight == 0xFFFE) {
			*outMinX = ImMin(*outMinX, px);
			*outMaxX = ImMax(*outMaxX, px + box->width);
			*outMinY = ImMin(*outMinY, py - box->height);
			*outMaxY = ImMax(*outMaxY, py + box->depth);
		}
		break;
	case LaTeXBox_Script:
		MeasureBounds(box->base, fontSize, px, py, outMinX, outMinY, outMaxX, outMaxY);
		MeasureBounds(box->superscript, fontSize, px, py, outMinX, outMinY, outMaxX, outMaxY);
		MeasureBounds(box->subscript, fontSize, px, py, outMinX, outMinY, outMaxX, outMaxY);
		break;
	case LaTeXBox_Frac: {
		for (int i = 0; i < box->children.Size; i++)
			MeasureBounds(box->children[i], fontSize, px, py, outMinX, outMinY, outMaxX, outMaxY);
		// Include fraction line
		float sz = fontSize * box->sizeFactor;
		float axisY = py - sz * 0.25f;
		*outMinX = ImMin(*outMinX, px);
		*outMaxX = ImMax(*outMaxX, px + box->width);
		*outMinY = ImMin(*outMinY, axisY - box->ruleThickness);
		*outMaxY = ImMax(*outMaxY, axisY + box->ruleThickness);
		break;
	}
	case LaTeXBox_Sqrt: {
		for (int i = 0; i < box->children.Size; i++)
			MeasureBounds(box->children[i], fontSize, px, py, outMinX, outMinY, outMaxX, outMaxY);
		if (box->base) // nth root degree
			MeasureBounds(box->base, fontSize, px, py, outMinX, outMinY, outMaxX, outMaxY);
		*outMinX = ImMin(*outMinX, px);
		*outMaxX = ImMax(*outMaxX, px + box->width);
		*outMinY = ImMin(*outMinY, py - box->height);
		*outMaxY = ImMax(*outMaxY, py + box->depth);
		break;
	}
	case LaTeXBox_Matrix: {
		for (int i = 0; i < box->children.Size; i++)
			MeasureBounds(box->children[i], fontSize, px, py, outMinX, outMinY, outMaxX, outMaxY);
		*outMinX = ImMin(*outMinX, px);
		*outMaxX = ImMax(*outMaxX, px + box->width);
		*outMinY = ImMin(*outMinY, py - box->height);
		*outMaxY = ImMax(*outMaxY, py + box->depth);
		break;
	}
	case LaTeXBox_VBox:
		for (int i = 0; i < box->children.Size; i++)
			MeasureBounds(box->children[i], fontSize, px, py, outMinX, outMinY, outMaxX, outMaxY);
		*outMinX = ImMin(*outMinX, px);
		*outMaxX = ImMax(*outMaxX, px + box->width);
		*outMinY = ImMin(*outMinY, py - box->height);
		*outMaxY = ImMax(*outMaxY, py + box->depth);
		break;
	case LaTeXBox_Delim:
		break;
	}
}

// ---- Render ----
static void RenderBox(ImDrawList* dl, LaTeXBox* box, ImFont* mathFont, float fontSize, float x, float y, ImU32 col);

static bool IsOperator(ImWchar ch) {
	return ch == '+' || ch == 0x2212 || ch == '=' || ch == 0x00D7 || ch == 0x00F7 ||
	       ch == 0x2264 || ch == 0x2265 || ch == 0x2260 || ch == 0x2248 || ch == 0x2261 ||
	       ch == 0x2192 || ch == 0x2190 || ch == 0x21D2;
}

// Draw a font glyph scaled to a target height. posX/posY = top-left of target area.
// Uses linear scaling: ascent ratio is constant regardless of font size.
static void DrawGlyphH(ImDrawList* dl, ImFont* font, float baseSz, ImWchar ch, float targetH, float posX, float posY, ImU32 col) {
	char utf8[8]; EncodeUTF8(ch, utf8);
	float asc = 0;
	ImVec2 refSz = CalcTextSize_Impl(font, baseSz, utf8, NULL, &asc);
	if (refSz.y < 1.0f) return;
	float fontSz = baseSz * targetH / refSz.y;
	float baselineY = posY + targetH * (asc / refSz.y);
	DrawText_Impl(dl, font, fontSz, ImVec2(posX, baselineY), col, utf8);
}

// Same but right-aligned.
static void DrawGlyphHR(ImDrawList* dl, ImFont* font, float baseSz, ImWchar ch, float targetH, float rightX, float posY, ImU32 col) {
	char utf8[8]; EncodeUTF8(ch, utf8);
	float asc = 0;
	ImVec2 refSz = CalcTextSize_Impl(font, baseSz, utf8, NULL, &asc);
	if (refSz.y < 1.0f) return;
	float fontSz = baseSz * targetH / refSz.y;
	float scaledW = refSz.x * targetH / refSz.y;
	float baselineY = posY + targetH * (asc / refSz.y);
	DrawText_Impl(dl, font, fontSz, ImVec2(rightX - scaledW, baselineY), col, utf8);
}

// Compute the advance width of a glyph scaled to a target height.
static float GlyphWidthAtH(ImFont* font, float baseSz, ImWchar ch, float targetH) {
	char utf8[8]; EncodeUTF8(ch, utf8);
	ImVec2 refSz = CalcTextSize_Impl(font, baseSz, utf8);
	if (refSz.y < 1.0f) return baseSz * 0.3f;
	return refSz.x * targetH / refSz.y;
}

// Draw a font glyph scaled to a target width. posX/posY = top-left of target area.
static void DrawGlyphW(ImDrawList* dl, ImFont* font, float baseSz, ImWchar ch, float targetW, float posX, float posY, ImU32 col) {
	char utf8[8]; EncodeUTF8(ch, utf8);
	float asc = 0;
	ImVec2 refSz = CalcTextSize_Impl(font, baseSz, utf8, NULL, &asc);
	if (refSz.x < 1.0f) return;
	float fontSz = baseSz * targetW / refSz.x;
	float scaledAsc = asc * targetW / refSz.x;
	DrawText_Impl(dl, font, fontSz, ImVec2(posX, posY + scaledAsc), col, utf8);
}

// Compute the height of a glyph scaled to a target width.
static float GlyphHeightAtW(ImFont* font, float baseSz, ImWchar ch, float targetW) {
	char utf8[8]; EncodeUTF8(ch, utf8);
	ImVec2 refSz = CalcTextSize_Impl(font, baseSz, utf8);
	if (refSz.x < 1.0f) return baseSz * 0.3f;
	return refSz.y * targetW / refSz.x;
}

// x,y = position where y is the BASELINE (text drawn downward from ascent above y)
static void RenderBox(ImDrawList* dl, LaTeXBox* box, ImFont* mathFont, float fontSize, float x, float y, ImU32 col) {
	if (!box) return;
	if (box->colorOverride) col = box->colorOverride; // color override cascades
	float px = x + box->shiftX;
	float py = y + box->shiftY;

	switch (box->type) {
	case LaTeXBox_Glyph: {
		float sz = fontSize * box->sizeFactor;
		if (box->text[0]) {
			// Shaped text — single DrawText call through kbts for proper kerning
			DrawText_Impl(dl, mathFont, sz, ImVec2(px, py), col, box->text);
		} else {
			ImWchar ch = box->codepoint;
			if (box->isMathItalic) ch = MathItalicize(ch);
			char utf8[8]; EncodeUTF8(ch, utf8);
			float offX = IsOperator(box->codepoint) ? sz * 0.17f : 0;
			DrawText_Impl(dl, mathFont, sz, ImVec2(px + offX, py), col, utf8);
		}
		break;
	}
	case LaTeXBox_Space:
		break;
	case LaTeXBox_HBox: {
		for (int i = 0; i < box->children.Size; i++)
			RenderBox(dl, box->children[i], mathFont, fontSize, px, py, col);
		// Draw accent mark if present (stored in delimLeft) — font glyph at natural size
		if (box->delimLeft && box->delimLeft < 0xFFF0) {
			float sz = fontSize * box->sizeFactor;
			float contentH = box->height - sz * 0.25f;
			float accentBaseY = py - contentH - sz * 0.05f;
			// Map to standalone accent glyphs
			ImWchar accentCh = 0;
			if (box->delimLeft == 0x20D7) accentCh = 0x2192;      // vec → arrow
			else if (box->delimLeft == 0x02C6) accentCh = 0x02C6;  // hat
			else if (box->delimLeft == 0x00AF) accentCh = 0x00AF;  // bar
			else if (box->delimLeft == 0x02D9) accentCh = 0x02D9;  // dot
			else if (box->delimLeft == 0x00A8) accentCh = 0x00A8;  // ddot
			else if (box->delimLeft == 0x02DC) accentCh = 0x02DC;  // tilde
			if (accentCh) {
				char utf8[8]; EncodeUTF8(accentCh, utf8);
				float accentSz = sz * 0.7f;
				// Render centered over content — position at content left edge
				// so the glyph visually centers (advance width includes bearings)
				DrawText_Impl(dl, mathFont, accentSz, ImVec2(px, accentBaseY), col, utf8);
			}
		}
		// Boxed: draw rectangle
		if (box->delimRight == (ImWchar)0xFFFF) {
			float thick = ImMax(1.5f, fontSize * box->sizeFactor * 0.05f);
			dl->AddRect(ImVec2(px, py - box->height), ImVec2(px + box->width, py + box->depth), col, 0, 0, thick);
		}
		// Cancel: diagonal strikethrough
		if (box->delimRight == (ImWchar)0xFFFD) {
			float thick = ImMax(1.5f, fontSize * box->sizeFactor * 0.05f);
			dl->AddLine(ImVec2(px, py + box->depth), ImVec2(px + box->width, py - box->height), col, thick);
		}
		if (box->delimRight == (ImWchar)0xFFFE) {
			float thick = ImMax(1.5f, fontSize * box->sizeFactor * 0.05f);
			dl->AddLine(ImVec2(px, py - box->height), ImVec2(px + box->width, py + box->depth), col, thick);
		}
		break;
	}
	case LaTeXBox_Script:
		RenderBox(dl, box->base, mathFont, fontSize, px, py, col);
		if (box->superscript) RenderBox(dl, box->superscript, mathFont, fontSize, px, py, col);
		if (box->subscript)   RenderBox(dl, box->subscript, mathFont, fontSize, px, py, col);
		break;
	case LaTeXBox_Frac: {
		for (int i = 0; i < box->children.Size; i++)
			RenderBox(dl, box->children[i], mathFont, fontSize, px, py, col);
		float sz = fontSize * box->sizeFactor;
		// Fraction line at the math axis (skip for binom)
		if (box->ruleThickness > 0) {
			float axisY = py - sz * 0.25f;
			dl->AddLine(ImVec2(px, axisY), ImVec2(px + box->width, axisY), col, ImMax(1.0f, box->ruleThickness));
		}
		// Binom delimiters
		if (box->delimLeft || box->delimRight) {
			float topY = py - box->height, botY = py + box->depth;
			float delimH = botY - topY;
			if (box->delimLeft)
				DrawGlyphH(dl, mathFont, sz, box->delimLeft, delimH, px, topY, col);
			if (box->delimRight)
				DrawGlyphHR(dl, mathFont, sz, box->delimRight, delimH, px + box->width, topY, col);
		}
		break;
	}
	case LaTeXBox_Sqrt: {
		float sz = fontSize * box->sizeFactor;
		float topY = py - box->height;
		float radH = box->height + box->depth;
		float radW = GlyphWidthAtH(mathFont, sz, 0x221A, radH);
		float thick = ImMax(1.0f, sz * 0.04f);
		// Radical sign glyph + overline
		DrawGlyphH(dl, mathFont, sz, 0x221A, radH, px, topY, col);
		dl->AddLine(ImVec2(px + radW, topY), ImVec2(px + box->width, topY), col, thick);
		for (int i = 0; i < box->children.Size; i++)
			RenderBox(dl, box->children[i], mathFont, fontSize, px, py, col);
		if (box->base) // nth root degree
			RenderBox(dl, box->base, mathFont, fontSize, px, py, col);
		break;
	}
	case LaTeXBox_Matrix: {
		float sz = fontSize * box->sizeFactor;
		float topY = py - box->height;
		float botY = py + box->depth;

		float delimH = botY - topY;
		if (box->delimLeft)
			DrawGlyphH(dl, mathFont, sz, box->delimLeft, delimH, px, topY, col);
		float rx = px + box->width;
		if (box->delimRight)
			DrawGlyphHR(dl, mathFont, sz, box->delimRight, delimH, rx, topY, col);

		// Draw cells
		for (int i = 0; i < box->children.Size; i++)
			RenderBox(dl, box->children[i], mathFont, fontSize, px, py, col);
		break;
	}
	case LaTeXBox_VBox: {
		for (int i = 0; i < box->children.Size; i++)
			RenderBox(dl, box->children[i], mathFont, fontSize, px, py, col);
		// Extensible horizontal brace from MATH table glyph assembly
		if (box->delimRight == '{') {
			float sz = fontSize * box->sizeFactor;
			LaTeXBox* main = box->children.Size > 0 ? box->children[0] : NULL;
			float cw = box->width;
			float targetW = cw / sz; // target width in em-space
			ImWchar braceCh = (box->delimLeft == 'O') ? 0x23DE : 0x23DF;
			// Get glyph ID for the brace character
			stbtt_fontinfo fi; float emSc = 0;
			bool hasMath = GetSlugFontInfo(mathFont, &fi, &emSc);
			int braceGI = hasMath ? stbtt_FindGlyphIndex(&fi, braceCh) : 0;
			MathGlyphAssembly assembly;
			bool hasAssembly = (braceGI > 0) && MathGetHAssembly(&fi, braceGI, emSc, &assembly) && assembly.partCount > 0;
			float baseY = (box->delimLeft == 'O')
				? py - (main ? main->height : 0) - sz * 0.05f
				: py + (main ? main->depth : 0) + sz * 0.05f;
			if (hasAssembly) {
				// Build the brace from assembly parts
				// First pass: compute total width of non-extender parts
				float fixedW = 0;
				int extIdx = -1;
				for (int i = 0; i < assembly.partCount; i++) {
					if (assembly.parts[i].isExtender) { extIdx = i; }
					else { fixedW += assembly.parts[i].fullAdvance; }
				}
				// How many extender copies needed?
				float extAdv = (extIdx >= 0) ? assembly.parts[extIdx].fullAdvance : 0;
				float overlap = (extIdx >= 0) ? assembly.parts[extIdx].startConnector * 0.5f : 0;
				// Compute extender copies, then scale font to match target width exactly
				int extCopies = 0;
				if (extAdv > overlap && targetW > fixedW) {
					float needed = targetW - fixedW;
					extCopies = (int)(needed / (extAdv - overlap));
				}
				// Compute assembled width in em
				float totalAsmW = 0;
				int joinCount = 0;
				for (int i2 = 0; i2 < assembly.partCount; i2++) {
					int cp2 = (assembly.parts[i2].isExtender) ? extCopies : 1;
					for (int c2 = 0; c2 < cp2; c2++) {
						if (joinCount > 0) totalAsmW -= overlap;
						totalAsmW += assembly.parts[i2].fullAdvance;
						joinCount++;
					}
				}
				if (totalAsmW < 0.01f) totalAsmW = 1.0f;
				// Scale font so assembled width = content width
				float braceSz = sz * targetW / totalAsmW;
				float braceOverlap = overlap * braceSz;
				// Assembly glyphs pre-built during layout
				// Render from content left edge
				float penX2 = px;
				int jc = 0;
				for (int i = 0; i < assembly.partCount; i++) {
					int copies = (assembly.parts[i].isExtender) ? extCopies : 1;
					ImWchar partKey = (ImWchar)(0x100000 + assembly.parts[i].glyphID);
					char partUtf8[8]; EncodeUTF8(partKey, partUtf8);
					for (int c = 0; c < copies; c++) {
						float ov = (jc > 0) ? braceOverlap : 0;
						DrawText_Impl(dl, mathFont, braceSz, ImVec2(penX2 - ov, baseY), col, partUtf8);
						penX2 += assembly.parts[i].fullAdvance * braceSz - ov;
						jc++;
					}
				}
			} else {
				// Fallback: render single brace glyph centered
				char bUtf8[8]; EncodeUTF8(braceCh, bUtf8);
				ImVec2 bSz = CalcTextSize_Impl(mathFont, sz, bUtf8);
				DrawText_Impl(dl, mathFont, sz, ImVec2(px + (cw - bSz.x) * 0.5f, baseY), col, bUtf8);
			}
		}
		break;
	}
	case LaTeXBox_Delim:
		break;
	}
}

void LaTeXRender(ImDrawList* drawList, LaTeXBox* box, ImFont* mathFont, float fontSize, ImVec2 pos, ImU32 col) {
	if (!box) return;
	RenderBox(drawList, box, mathFont, fontSize, pos.x, pos.y, col);
}

// ---- Debug: draw bounding boxes for each box in the tree ----
static const ImU32 kDbgColors[] = {
	IM_COL32(255, 80, 80, 100), IM_COL32(80, 200, 80, 100), IM_COL32(80, 120, 255, 100),
	IM_COL32(255, 200, 40, 100), IM_COL32(200, 80, 255, 100), IM_COL32(80, 220, 220, 100),
};
static const ImU32 kDbgOutlines[] = {
	IM_COL32(255, 80, 80, 200), IM_COL32(80, 200, 80, 200), IM_COL32(80, 120, 255, 200),
	IM_COL32(255, 200, 40, 200), IM_COL32(200, 80, 255, 200), IM_COL32(80, 220, 220, 200),
};

static void DebugDrawBox(ImDrawList* dl, LaTeXBox* box, float fontSize, float x, float y, int* colorIdx)
{
	if (!box) return;
	float px = x + box->shiftX;
	float py = y + box->shiftY;

	int ci = (*colorIdx) % 6;

	switch (box->type) {
	case LaTeXBox_Glyph: {
		// Glyph bbox: [px, px+width] x [py-height, py+depth]
		ImVec2 tl(px, py - box->height);
		ImVec2 br(px + box->width, py + box->depth);
		dl->AddRectFilled(tl, br, kDbgColors[ci]);
		dl->AddRect(tl, br, kDbgOutlines[ci], 0, 0, 1.0f);
		// Baseline tick
		dl->AddLine(ImVec2(px, py), ImVec2(px + box->width, py), IM_COL32(255, 255, 0, 120), 1.0f);
		(*colorIdx)++;
		break;
	}
	case LaTeXBox_Space:
		break;
	case LaTeXBox_HBox:
		for (int i = 0; i < box->children.Size; i++)
			DebugDrawBox(dl, box->children[i], fontSize, px, py, colorIdx);
		break;
	case LaTeXBox_Script:
		DebugDrawBox(dl, box->base, fontSize, px, py, colorIdx);
		DebugDrawBox(dl, box->superscript, fontSize, px, py, colorIdx);
		DebugDrawBox(dl, box->subscript, fontSize, px, py, colorIdx);
		break;
	case LaTeXBox_Frac: {
		// Draw fraction bbox
		float sz = fontSize * box->sizeFactor;
		float axisY = py - sz * 0.25f;
		dl->AddRect(ImVec2(px, py - box->height), ImVec2(px + box->width, py + box->depth),
			IM_COL32(255, 128, 0, 150), 0, 0, 1.0f);
		// Axis line
		dl->AddLine(ImVec2(px, axisY), ImVec2(px + box->width, axisY), IM_COL32(255, 128, 0, 80), 1.0f);
		for (int i = 0; i < box->children.Size; i++)
			DebugDrawBox(dl, box->children[i], fontSize, px, py, colorIdx);
		break;
	}
	case LaTeXBox_Sqrt: {
		dl->AddRect(ImVec2(px, py - box->height), ImVec2(px + box->width, py + box->depth),
			IM_COL32(0, 200, 200, 150), 0, 0, 1.0f);
		for (int i = 0; i < box->children.Size; i++)
			DebugDrawBox(dl, box->children[i], fontSize, px, py, colorIdx);
		if (box->base) DebugDrawBox(dl, box->base, fontSize, px, py, colorIdx);
		break;
	}
	case LaTeXBox_Matrix: {
		dl->AddRect(ImVec2(px, py - box->height), ImVec2(px + box->width, py + box->depth),
			IM_COL32(200, 100, 200, 150), 0, 0, 1.0f);
		for (int i = 0; i < box->children.Size; i++)
			DebugDrawBox(dl, box->children[i], fontSize, px, py, colorIdx);
		break;
	}
	case LaTeXBox_VBox: {
		dl->AddRect(ImVec2(px, py - box->height), ImVec2(px + box->width, py + box->depth),
			IM_COL32(100, 200, 100, 150), 0, 0, 1.0f);
		for (int i = 0; i < box->children.Size; i++)
			DebugDrawBox(dl, box->children[i], fontSize, px, py, colorIdx);
		break;
	}
	case LaTeXBox_Delim:
		break;
	}
}

// ---- Math font management ----
static ImFont* g_mathFont = NULL;

ImFont* LaTeXGetMathFont() {
	return g_mathFont;
}

// Called during font loading (before CreateContext or at init time)
void LaTeXLoadMathFont() {
	if (g_mathFont) return;
	static const ImWchar mathRanges[] = {
		0x0020, 0x007E,   // Basic Latin
		0x0080, 0x00FF,   // Latin Supplement
		0x0300, 0x036F,   // Combining Diacritical Marks
		0x0370, 0x03FF,   // Greek and Coptic
		0x2000, 0x206F,   // General Punctuation
		0x2070, 0x209F,   // Superscripts and Subscripts
		0x2100, 0x214F,   // Letterlike Symbols
		0x2190, 0x21FF,   // Arrows
		0x2200, 0x22FF,   // Mathematical Operators
		0x2300, 0x23FF,   // Misc Technical
		0x27C0, 0x27EF,   // Misc Math Symbols A
		0x27F0, 0x27FF,   // Supplemental Arrows A
		0x2900, 0x297F,   // Supplemental Arrows B
		0x2980, 0x29FF,   // Misc Math Symbols B
		0x2A00, 0x2AFF,   // Supplemental Math Operators
		0x1D400, 0x1D7FF, // Mathematical Alphanumeric Symbols (bold, italic, etc.)
		0
	};
	ImFontConfig cfg;
	cfg.FontLoader = GetSlugFontLoader();
	g_mathFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(
		"latex_fonts/latinmodern-math.otf", 24.0f, &cfg, mathRanges);
}

// ---- Public API implementation ----

// Helper: parse + layout + measure actual pixel bounds.
// Returns bbox as (minX, minY, maxX, maxY) relative to baseline at (0, 0).
static ImVec4 LaTeXBuildAndMeasure(const char* latex, float font_size, LaTeXBox** outTree)
{
	LaTeXBox* tree = LaTeXParse(latex);
	if (!tree) { if (outTree) *outTree = NULL; return ImVec4(0,0,0,0); }
	LaTeXLayout(tree, font_size);
	// Measure with baseline at y=0
	float mnX = 1e30f, mnY = 1e30f, mxX = -1e30f, mxY = -1e30f;
	MeasureBounds(tree, font_size, 0, 0, &mnX, &mnY, &mxX, &mxY);
	if (mnX > mxX) { mnX = mnY = 0; mxX = mxY = 0; }
	if (outTree) *outTree = tree; else IM_DELETE(tree);
	return ImVec4(mnX, mnY, mxX, mxY);
}

// pos = top-left corner of the visible bounding box.
void DrawLaTeX(ImDrawList* pDrawList, float font_size, ImVec2 pos, ImU32 col, const char* latex)
{
	font_size = SlugLpToPx(font_size);
	if (!pDrawList || !latex || !*latex) return;
	ImFont* mathFont = LaTeXGetMathFont();
	if (!mathFont) return;

	LaTeXBox* tree = NULL;
	ImVec4 bb = LaTeXBuildAndMeasure(latex, font_size, &tree);
	if (!tree) return;

	// bb is relative to baseline at (0,0).
	// We want pos to be the top-left corner, so:
	//   renderX = pos.x - bb.x  (shift so left edge of content = pos.x)
	//   baseline = pos.y - bb.y  (shift so top edge of content = pos.y)
	LaTeXRender(pDrawList, tree, mathFont, font_size, ImVec2(pos.x - bb.x, pos.y - bb.y), col);

	IM_DELETE(tree);
}

// Returns (width, height) of the actual visible bounding box.
ImVec2 CalcLaTeXSize(float font_size, const char* latex)
{
	font_size = SlugLpToPx(font_size);
	if (!latex || !*latex) return ImVec2(0, 0);
	ImVec4 bb = LaTeXBuildAndMeasure(latex, font_size, NULL);
	return ImVec2(bb.z - bb.x, bb.w - bb.y);
}

void LoadLaTeXFont() { LaTeXLoadMathFont(); }

void DrawLaTeXDebug(ImDrawList* pDrawList, float font_size, ImVec2 pos, const char* latex)
{
	font_size = SlugLpToPx(font_size);
	if (!pDrawList || !latex || !*latex) return;

	LaTeXBox* tree = NULL;
	ImVec4 bb = LaTeXBuildAndMeasure(latex, font_size, &tree);
	if (!tree) return;

	// Same offset as DrawLaTeX
	float baseX = pos.x - bb.x;
	float baseY = pos.y - bb.y;

	int colorIdx = 0;
	DebugDrawBox(pDrawList, tree, font_size, baseX, baseY, &colorIdx);

	// Draw overall bbox outline
	pDrawList->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + bb.z - bb.x, pos.y + bb.w - bb.y),
		IM_COL32(255, 255, 255, 200), 0, 0, 1.5f);

	IM_DELETE(tree);
}

// ---- Tessellation helpers (mirror of the DrawGlyphH / DrawGlyphHR render helpers) ----

// Add a filled axis-aligned rectangle to the shape as two triangles.
// Used for fraction rules and sqrt overlines (dl->AddLine equivalents).
static void AddRuleToShape(ImWidgetsShape& outShape, float x0, float y0, float x1, float y1)
{
	if (x1 <= x0 || y1 <= y0) return;
	ImVec2 wuv = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
	int base = outShape.vertices.Size;
	outShape.vertices.resize(base + 4);
	outShape.vertices[base+0] = { ImVec2(x0, y0), wuv, IM_COL32_WHITE };
	outShape.vertices[base+1] = { ImVec2(x1, y0), wuv, IM_COL32_WHITE };
	outShape.vertices[base+2] = { ImVec2(x1, y1), wuv, IM_COL32_WHITE };
	outShape.vertices[base+3] = { ImVec2(x0, y1), wuv, IM_COL32_WHITE };
	int bt = outShape.triangles.Size;
	outShape.triangles.resize(bt + 2);
	outShape.triangles[bt+0] = ImWidgetsTriIdx((ImDrawIdx)(base+0),(ImDrawIdx)(base+1),(ImDrawIdx)(base+2));
	outShape.triangles[bt+1] = ImWidgetsTriIdx((ImDrawIdx)(base+0),(ImDrawIdx)(base+2),(ImDrawIdx)(base+3));
	outShape.bb.Add(ImRect(x0, y0, x1, y1));
}

// Tessellate a text string at baseline (baseX, baselineY) and merge into outShape.
static void TessLatexText(ImFont* font, float fontSize, const char* text,
                          float baseX, float baselineY,
                          ImWidgetsShape& outShape, float tess_tol, int iterations)
{
    ImWidgetsShape tmp;
    tmp.bb = ImRect(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    TesselateText_Impl(font, fontSize, text, tmp, nullptr, tess_tol, iterations);
    if (tmp.triangles.Size == 0) return;
    int baseVtx = outShape.vertices.Size;
    outShape.vertices.resize(baseVtx + tmp.vertices.Size);
    for (int i = 0; i < tmp.vertices.Size; i++) {
        outShape.vertices[baseVtx + i] = tmp.vertices[i];
        outShape.vertices[baseVtx + i].pos.x += baseX;
        outShape.vertices[baseVtx + i].pos.y += baselineY;
    }
    int baseTri = outShape.triangles.Size;
    outShape.triangles.resize(baseTri + tmp.triangles.Size);
    for (int i = 0; i < tmp.triangles.Size; i++)
        outShape.triangles[baseTri + i] = ImWidgetsTriIdx(
            (ImDrawIdx)(tmp.triangles[i].a + baseVtx),
            (ImDrawIdx)(tmp.triangles[i].b + baseVtx),
            (ImDrawIdx)(tmp.triangles[i].c + baseVtx));
    if (tmp.bb.Min.x < FLT_MAX)
        outShape.bb.Add(ImRect(
            tmp.bb.Min.x + baseX, tmp.bb.Min.y + baselineY,
            tmp.bb.Max.x + baseX, tmp.bb.Max.y + baselineY));
}

// Tessellate a glyph scaled to targetH at (posX, posY=top-left) and merge.
static void TessLatexGlyphH(ImFont* font, float baseSz, ImWchar ch, float targetH,
                             float posX, float posY,
                             ImWidgetsShape& outShape, float tess_tol, int iterations)
{
    char utf8[8]; EncodeUTF8(ch, utf8);
    float asc = 0;
    ImVec2 refSz = CalcTextSize_Impl(font, baseSz, utf8, NULL, &asc);
    if (refSz.y < 1.0f) return;
    float fontSz    = baseSz * targetH / refSz.y;
    float baselineY = posY + targetH * (asc / refSz.y);
    TessLatexText(font, fontSz, utf8, posX, baselineY, outShape, tess_tol, iterations);
}

// Tessellate a glyph scaled to targetH, right-aligned to rightX, and merge.
static void TessLatexGlyphHR(ImFont* font, float baseSz, ImWchar ch, float targetH,
                              float rightX, float posY,
                              ImWidgetsShape& outShape, float tess_tol, int iterations)
{
    char utf8[8]; EncodeUTF8(ch, utf8);
    float asc = 0;
    ImVec2 refSz = CalcTextSize_Impl(font, baseSz, utf8, NULL, &asc);
    if (refSz.y < 1.0f) return;
    float fontSz    = baseSz * targetH / refSz.y;
    float scaledW   = refSz.x * targetH / refSz.y;
    float baselineY = posY + targetH * (asc / refSz.y);
    TessLatexText(font, fontSz, utf8, rightX - scaledW, baselineY, outShape, tess_tol, iterations);
}

// Walk the LaTeX box tree and tessellate all glyphs into outShape.
// x,y = baseline position (same convention as RenderBox).
static void TessellateBox(LaTeXBox* box, ImFont* mathFont, float fontSize, float x, float y,
                          ImWidgetsShape& outShape, float tess_tol, int iterations)
{
    if (!box) return;
    float px = x + box->shiftX;
    float py = y + box->shiftY;

    switch (box->type) {
    case LaTeXBox_Glyph: {
        float sz = fontSize * box->sizeFactor;
        if (box->text[0]) {
            TessLatexText(mathFont, sz, box->text, px, py, outShape, tess_tol, iterations);
        } else {
            ImWchar ch = box->codepoint;
            if (box->isMathItalic) ch = MathItalicize(ch);
            char utf8[8]; EncodeUTF8(ch, utf8);
            float offX = IsOperator(box->codepoint) ? sz * 0.17f : 0;
            TessLatexText(mathFont, sz, utf8, px + offX, py, outShape, tess_tol, iterations);
        }
        break;
    }
    case LaTeXBox_Space:
        break;
    case LaTeXBox_HBox: {
        for (int i = 0; i < box->children.Size; i++)
            TessellateBox(box->children[i], mathFont, fontSize, px, py, outShape, tess_tol, iterations);
        // Accent mark
        if (box->delimLeft && box->delimLeft < 0xFFF0) {
            float sz = fontSize * box->sizeFactor;
            float contentH  = box->height - sz * 0.25f;
            float accentBaseY = py - contentH - sz * 0.05f;
            ImWchar accentCh = 0;
            if      (box->delimLeft == 0x20D7) accentCh = 0x2192;
            else if (box->delimLeft == 0x02C6) accentCh = 0x02C6;
            else if (box->delimLeft == 0x00AF) accentCh = 0x00AF;
            else if (box->delimLeft == 0x02D9) accentCh = 0x02D9;
            else if (box->delimLeft == 0x00A8) accentCh = 0x00A8;
            else if (box->delimLeft == 0x02DC) accentCh = 0x02DC;
            if (accentCh) {
                char utf8[8]; EncodeUTF8(accentCh, utf8);
                TessLatexText(mathFont, sz * 0.7f, utf8, px, accentBaseY, outShape, tess_tol, iterations);
            }
        }
        // Skip boxed rect / cancel line decorations (AddRect/AddLine)
        break;
    }
    case LaTeXBox_Script:
        TessellateBox(box->base,        mathFont, fontSize, px, py, outShape, tess_tol, iterations);
        if (box->superscript) TessellateBox(box->superscript, mathFont, fontSize, px, py, outShape, tess_tol, iterations);
        if (box->subscript)   TessellateBox(box->subscript,   mathFont, fontSize, px, py, outShape, tess_tol, iterations);
        break;
    case LaTeXBox_Frac: {
        for (int i = 0; i < box->children.Size; i++)
            TessellateBox(box->children[i], mathFont, fontSize, px, py, outShape, tess_tol, iterations);
        // Fraction rule (mirrors dl->AddLine in RenderBox)
        if (box->ruleThickness > 0) {
            float sz = fontSize * box->sizeFactor;
            float axisY = py - sz * 0.25f;
            float half  = ImMax(0.5f, box->ruleThickness * 0.5f);
            AddRuleToShape(outShape, px, axisY - half, px + box->width, axisY + half);
        }
        if (box->delimLeft || box->delimRight) {
            float sz = fontSize * box->sizeFactor;
            float topY = py - box->height, botY = py + box->depth;
            float delimH = botY - topY;
            if (box->delimLeft)
                TessLatexGlyphH( mathFont, sz, box->delimLeft,  delimH, px,              topY, outShape, tess_tol, iterations);
            if (box->delimRight)
                TessLatexGlyphHR(mathFont, sz, box->delimRight, delimH, px + box->width, topY, outShape, tess_tol, iterations);
        }
        break;
    }
    case LaTeXBox_Sqrt: {
        float sz    = fontSize * box->sizeFactor;
        float topY  = py - box->height;
        float radH  = box->height + box->depth;
        float radW  = GlyphWidthAtH(mathFont, sz, 0x221A, radH);
        float thick = ImMax(0.5f, sz * 0.04f);
        TessLatexGlyphH(mathFont, sz, 0x221A, radH, px, topY, outShape, tess_tol, iterations);
        AddRuleToShape(outShape, px + radW, topY, px + box->width, topY + thick);
        for (int i = 0; i < box->children.Size; i++)
            TessellateBox(box->children[i], mathFont, fontSize, px, py, outShape, tess_tol, iterations);
        if (box->base)
            TessellateBox(box->base, mathFont, fontSize, px, py, outShape, tess_tol, iterations);
        break;
    }
    case LaTeXBox_Matrix: {
        float sz    = fontSize * box->sizeFactor;
        float topY  = py - box->height, botY = py + box->depth;
        float delimH = botY - topY;
        if (box->delimLeft)
            TessLatexGlyphH( mathFont, sz, box->delimLeft,  delimH, px,              topY, outShape, tess_tol, iterations);
        if (box->delimRight)
            TessLatexGlyphHR(mathFont, sz, box->delimRight, delimH, px + box->width, topY, outShape, tess_tol, iterations);
        for (int i = 0; i < box->children.Size; i++)
            TessellateBox(box->children[i], mathFont, fontSize, px, py, outShape, tess_tol, iterations);
        break;
    }
    case LaTeXBox_VBox: {
        for (int i = 0; i < box->children.Size; i++)
            TessellateBox(box->children[i], mathFont, fontSize, px, py, outShape, tess_tol, iterations);
        // Extensible horizontal brace assembly — mirrors RenderBox exactly
        if (box->delimRight == '{') {
            float sz = fontSize * box->sizeFactor;
            LaTeXBox* main = box->children.Size > 0 ? box->children[0] : NULL;
            float cw = box->width;
            float targetW = cw / sz;
            ImWchar braceCh = (box->delimLeft == 'O') ? 0x23DE : 0x23DF;
            stbtt_fontinfo fi; float emSc = 0;
            bool hasMath = GetSlugFontInfo(mathFont, &fi, &emSc);
            int braceGI = hasMath ? stbtt_FindGlyphIndex(&fi, braceCh) : 0;
            MathGlyphAssembly assembly;
            bool hasAssembly = (braceGI > 0) && MathGetHAssembly(&fi, braceGI, emSc, &assembly) && assembly.partCount > 0;
            float baseY = (box->delimLeft == 'O')
                ? py - (main ? main->height : 0) - sz * 0.05f
                : py + (main ? main->depth : 0) + sz * 0.05f;
            if (hasAssembly) {
                float fixedW = 0; int extIdx = -1;
                for (int i = 0; i < assembly.partCount; i++) {
                    if (assembly.parts[i].isExtender) extIdx = i;
                    else fixedW += assembly.parts[i].fullAdvance;
                }
                float extAdv    = (extIdx >= 0) ? assembly.parts[extIdx].fullAdvance : 0;
                float overlap   = (extIdx >= 0) ? assembly.parts[extIdx].startConnector * 0.5f : 0;
                int extCopies = 0;
                if (extAdv > overlap && targetW > fixedW)
                    extCopies = (int)((targetW - fixedW) / (extAdv - overlap));
                float totalAsmW = 0; int joinCount = 0;
                for (int i2 = 0; i2 < assembly.partCount; i2++) {
                    int cp2 = assembly.parts[i2].isExtender ? extCopies : 1;
                    for (int c2 = 0; c2 < cp2; c2++) {
                        if (joinCount++ > 0) totalAsmW -= overlap;
                        totalAsmW += assembly.parts[i2].fullAdvance;
                    }
                }
                if (totalAsmW < 0.01f) totalAsmW = 1.0f;
                float braceSz = sz * targetW / totalAsmW;
                float braceOverlap = overlap * braceSz;
                float penX2 = px; int jc = 0;
                for (int i = 0; i < assembly.partCount; i++) {
                    int copies = assembly.parts[i].isExtender ? extCopies : 1;
                    ImWchar partKey = (ImWchar)(0x100000 + assembly.parts[i].glyphID);
                    char partUtf8[8]; EncodeUTF8(partKey, partUtf8);
                    for (int c = 0; c < copies; c++) {
                        float ov = (jc > 0) ? braceOverlap : 0;
                        TessLatexText(mathFont, braceSz, partUtf8, penX2 - ov, baseY, outShape, tess_tol, iterations);
                        penX2 += assembly.parts[i].fullAdvance * braceSz - ov;
                        jc++;
                    }
                }
            } else {
                char bUtf8[8]; EncodeUTF8(braceCh, bUtf8);
                ImVec2 bSz = CalcTextSize_Impl(mathFont, sz, bUtf8);
                TessLatexText(mathFont, sz, bUtf8, px + (cw - bSz.x) * 0.5f, baseY, outShape, tess_tol, iterations);
            }
        }
        break;
    }
    case LaTeXBox_Delim:
        break;
    }
}

// Tessellate a LaTeX expression into an ImWidgetsShape for gradient/image fills.
// pos = top-left corner of the expression (same convention as DrawLaTeX).
// The shape's bb reflects actual glyph bounds; use CalcLaTeXSize for layout space.
void TesselateLaTeX(float font_size, const char* latex, ImVec2 pos,
                    ImWidgetsShape& outShape, float tess_tol, int iterations)
{
    font_size = SlugLpToPx(font_size);
    outShape.vertices.resize(0);
    outShape.triangles.resize(0);
    outShape.bb = ImRect(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    if (!latex || !*latex) return;

    ImFont* mathFont = LaTeXGetMathFont();
    if (!mathFont) return;

    LaTeXBox* tree = NULL;
    ImVec4 bb = LaTeXBuildAndMeasure(latex, font_size, &tree);
    if (!tree) return;

    // Same coordinate transform as DrawLaTeX: pos is the top-left corner of the content bbox.
    float baseX = pos.x - bb.x;
    float baseY = pos.y - bb.y;
    TessellateBox(tree, mathFont, font_size, baseX, baseY, outShape, tess_tol, iterations);
    IM_DELETE(tree);
}

} // namespace ImWidgets
#endif // _DEAR_WIDGETS_LATEX_INCLUDED
