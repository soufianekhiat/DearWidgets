#pragma once
// dear_widgets_text_shape.h -- text-shaping API.
//
// ─── Backend ─────────────────────────────────────────────────────────────────
//
// kb_text_shape
//   A single-header shaper bundled with DearWidgets (src/api/kb_text_shape.h).
//   No external dependency. This is the only shaping backend.
//
// ─────────────────────────────────────────────────────────────────────────────
//
// The implementation lives in dear_widgets_text_shape.cpp, compiled as its own
// translation unit.  dear_widgets_slug.cpp includes this header and the linker
// resolves the calls — no #include of the .cpp required.

// ─── Feature tag helpers ─────────────────────────────────────────────────────
// 4-byte big-endian FOURCC — identical layout to KBTS_FEATURE_TAG_* and HB_TAG().
#define DW_SHAPER_TAG(a,b,c,d) \
    ( (unsigned)(a)<<24 | (unsigned)(b)<<16 | (unsigned)(c)<<8 | (unsigned)(d) )

#define DW_SHAPER_FEATURE_liga  DW_SHAPER_TAG('l','i','g','a')
#define DW_SHAPER_FEATURE_calt  DW_SHAPER_TAG('c','a','l','t')
#define DW_SHAPER_FEATURE_ss01  DW_SHAPER_TAG('s','s','0','1')
#define DW_SHAPER_FEATURE_ss07  DW_SHAPER_TAG('s','s','0','7')

// ─── Common types ─────────────────────────────────────────────────────────────

// Opaque context — owns the font, accumulated features, and per-pass shaping state.
struct DwShaperCtx;

// A single shaped glyph returned by DwShaper_NextGlyph().
// All metrics are in font design units (same scale as kbts and hb with upem scale).
struct DwShaperGlyph
{
    unsigned int GlyphId;  // glyph index in the font
    int          AdvanceX; // horizontal advance
    int          OffsetX;  // horizontal placement offset
    int          OffsetY;  // vertical placement offset
    int          Cluster;  // byte offset of the first source character of this glyph's cluster
                           // (filled by DwShaper_ShapeBidi; -1 from the raw run iterator)
    int          Level;    // resolved bidi embedding level (odd = RTL); DwShaper_ShapeBidi only
};

// Paragraph direction for DwShaper_ShapeBidi (CSS `direction` / HTML `dir`).
enum DwShaperDirection
{
    DwShaperDirection_Auto = 0, // first strong character decides (HTML dir="auto"), LTR if none
    DwShaperDirection_LTR  = 1,
    DwShaperDirection_RTL  = 2,
};

// Opaque run handle — passed to DwShaper_NextGlyph() to iterate one script run.
struct DwShaperRun { void* _p; };

// ─── API ─────────────────────────────────────────────────────────────────────

// Lifecycle
DwShaperCtx* DwShaper_Create();
void         DwShaper_Destroy(DwShaperCtx* ctx);

// Returns true once a font has been loaded and is ready for shaping.
bool DwShaper_IsReady(DwShaperCtx* ctx);

// Font + features — call once per font before any shaping.
bool DwShaper_PushFontFromMemory(DwShaperCtx* ctx, void* data, int size);
void DwShaper_PushFeature(DwShaperCtx* ctx, unsigned tag, unsigned value);

// Per-string shaping — Begin / PushUtf8 / End sequence.
void DwShaper_Begin(DwShaperCtx* ctx);
void DwShaper_PushUtf8(DwShaperCtx* ctx, const char* text, int len);
void DwShaper_End(DwShaperCtx* ctx);

// Output iteration — call after DwShaper_End().
bool DwShaper_NextRun(DwShaperCtx* ctx, DwShaperRun* run);
bool DwShaper_NextGlyph(DwShaperRun* run, DwShaperGlyph* out);

// Whole-string shaping with Unicode bidi, as a browser does it: resolves the
// implicit embedding levels of UAX #9 (rules P2-P3, W1-W7, N1-N2, I1-I2, L1),
// shapes each level run with its own direction, then reorders the runs (L2).
// `emit` receives every glyph once, in final left-to-right visual order.
// Returns the resolved paragraph level (0 = LTR, 1 = RTL).
// Text segmentation (UAX #14 line breaks, UAX #29 graphemes / words), one
// flag word per byte of the UTF-8 input plus one for the end: flags[i] tells
// what kind of boundary sits *before* byte i. Same bits as kbts_break_flags.
enum DwShaperBreak
{
    DwShaperBreak_Grapheme = 1 << 2,
    DwShaperBreak_Word     = 1 << 3,
    DwShaperBreak_LineSoft = 1 << 4, // a line may wrap here
    DwShaperBreak_LineHard = 1 << 5, // a line must break here (after LF, CR, LS, PS, ...)
};
void DwShaper_TextBreaks(const char* text, int len, unsigned int* out_flags /* len + 1 entries */);

// Paragraph level per UAX #9 P2-P3 for `direction` (DwShaperDirection): 0 = LTR, 1 = RTL.
int  DwShaper_ParagraphLevel(const char* text, int len, int direction);

typedef void (*DwShaperEmitFn)(void* user, const DwShaperGlyph* glyph);
int  DwShaper_ShapeBidi(DwShaperCtx* ctx, const char* text, int len, int direction,
                        DwShaperEmitFn emit, void* user);
