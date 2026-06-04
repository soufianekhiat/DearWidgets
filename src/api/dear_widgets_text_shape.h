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
