// dear_widgets_text_shape.cpp -- text-shaping backend implementations.
// Compiled as its own translation unit; dear_widgets_slug.cpp includes the
// header and the linker resolves calls via namespace ImWidgets.
#include "imgui.h"   // IM_ALLOC / IM_FREE
#include "imgui_internal.h" // ImTextCharFromUtf8, ImMax
#include <cstring>   // memset

// ── Backend include (outside namespace — it defines global types) ──
// kb_text_shape is a single-header shaper bundled with DearWidgets; no external
// dependency. It is the only shaping backend.

#define KB_TEXT_SHAPE_STATIC
#define KB_TEXT_SHAPE_IMPLEMENTATION
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4100 4319 4505 4701)
#elif defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-function"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wuninitialized"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-function"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wuninitialized"
#endif
#include "kb_text_shape.h"
#if defined(_MSC_VER)
#  pragma warning(pop)
#elif defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

// ─────────────────────────────────────────────────────────────────────────────
namespace ImWidgets {
// Include declarations inside the namespace so symbols match what callers
// (compiled inside namespace ImWidgets via dear_widgets.cpp) expect.
#include "dear_widgets_text_shape.h"

// ─────────────────────────────────────────────────────────────────────────────

struct DwShaperCtx
{
    kbts_shape_context* kbts;
    kbts_font*          font;
    kbts_run            current_run;
};

#ifdef _WIN32
static kbts_font* DwShaper_kbts_TryPushFont(kbts_shape_context* ctx, void* data, int size)
{
    kbts_font* f = nullptr;
    __try   { f = kbts_ShapePushFontFromMemory(ctx, data, size, 0); }
    __except(1) { f = nullptr; }
    return f;
}
#endif

DwShaperCtx* DwShaper_Create()
{
    DwShaperCtx* ctx = (DwShaperCtx*)IM_ALLOC(sizeof(DwShaperCtx));
    memset(ctx, 0, sizeof(DwShaperCtx));
    ctx->kbts = kbts_CreateShapeContext(0, 0);
    return ctx;
}

void DwShaper_Destroy(DwShaperCtx* ctx)
{
    if (!ctx) return;
    if (ctx->kbts) kbts_DestroyShapeContext(ctx->kbts);
    IM_FREE(ctx);
}

bool DwShaper_IsReady(DwShaperCtx* ctx)
{
    return ctx && ctx->kbts && ctx->font && ctx->font->Error == 0;
}

bool DwShaper_PushFontFromMemory(DwShaperCtx* ctx, void* data, int size)
{
#ifdef _WIN32
    ctx->font = DwShaper_kbts_TryPushFont(ctx->kbts, data, size);
#else
    ctx->font = kbts_ShapePushFontFromMemory(ctx->kbts, data, size, 0);
#endif
    if (!ctx->font || ctx->font->Error != 0)
    {
        kbts_DestroyShapeContext(ctx->kbts);
        ctx->kbts = nullptr;
        ctx->font = nullptr;
        return false;
    }
    return true;
}

void DwShaper_PushFeature(DwShaperCtx* ctx, unsigned tag, unsigned value)
{
    kbts_ShapePushFeature(ctx->kbts, (kbts_u32)tag, (kbts_u32)value);
}

void DwShaper_Begin(DwShaperCtx* ctx)
{
    kbts_ShapeBegin(ctx->kbts, KBTS_DIRECTION_DONT_KNOW, KBTS_LANGUAGE_DONT_KNOW);
}

void DwShaper_PushUtf8(DwShaperCtx* ctx, const char* text, int len)
{
    kbts_ShapeUtf8(ctx->kbts, text, (kbts_u32)len, KBTS_USER_ID_GENERATION_MODE_CODEPOINT_INDEX);
}

void DwShaper_End(DwShaperCtx* ctx)
{
    kbts_ShapeEnd(ctx->kbts);
}

bool DwShaper_NextRun(DwShaperCtx* ctx, DwShaperRun* run)
{
    if (!kbts_ShapeRun(ctx->kbts, &ctx->current_run))
        return false;
    run->_p = &ctx->current_run;
    return true;
}

bool DwShaper_NextGlyph(DwShaperRun* run, DwShaperGlyph* out)
{
    kbts_run*   kr = (kbts_run*)run->_p;
    kbts_glyph* g  = nullptr;
    if (!kbts_GlyphIteratorNext(&kr->Glyphs, &g))
        return false;
    out->GlyphId  = (unsigned int)g->Id;
    out->AdvanceX = (int)g->AdvanceX;
    out->OffsetX  = (int)g->OffsetX;
    out->OffsetY  = (int)g->OffsetY;
    out->Cluster  = -1;
    out->Level    = 0;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Unicode bidi (UAX #9, implicit levels) + per-run shaping + L2 reordering.
//
// kbts reduces the Unicode bidi classes to: NI (all neutrals: WS, ON, B, S and
// isolates), BN, L, R, NSM, AL, AN, EN, ES, ET, CS. That is exactly what the
// implicit part of the algorithm needs. Explicit embeddings/overrides/isolates
// (LRE, RLO, LRI, ...) are treated as neutrals, so the maximum level is 2.

enum DwBidiType : unsigned char
{
    DwBidi_NI = KBTS_UNICODE_BIDIRECTIONAL_CLASS_NI,
    DwBidi_BN = KBTS_UNICODE_BIDIRECTIONAL_CLASS_BN,
    DwBidi_L  = KBTS_UNICODE_BIDIRECTIONAL_CLASS_L,
    DwBidi_R  = KBTS_UNICODE_BIDIRECTIONAL_CLASS_R,
    DwBidi_NSM= KBTS_UNICODE_BIDIRECTIONAL_CLASS_NSM,
    DwBidi_AL = KBTS_UNICODE_BIDIRECTIONAL_CLASS_AL,
    DwBidi_AN = KBTS_UNICODE_BIDIRECTIONAL_CLASS_AN,
    DwBidi_EN = KBTS_UNICODE_BIDIRECTIONAL_CLASS_EN,
    DwBidi_ES = KBTS_UNICODE_BIDIRECTIONAL_CLASS_ES,
    DwBidi_ET = KBTS_UNICODE_BIDIRECTIONAL_CLASS_ET,
    DwBidi_CS = KBTS_UNICODE_BIDIRECTIONAL_CLASS_CS,
};

static inline bool DwBidi_IsWhitespace(unsigned int cp)
{
    // WS class plus segment / paragraph separators (L1). U+00A0 is CS, not WS.
    return cp == 0x20 || cp == 0x09 || cp == 0x0B || cp == 0x0C || cp == 0x3000
        || (cp >= 0x2000 && cp <= 0x200A) || cp == 0x1680 || cp == 0x205F
        || cp == 0x2028 || cp == 0x2029;
}

// Resolves one embedding level per codepoint. `types` is modified in place.
static int DwBidi_ResolveLevels(unsigned char* types, const unsigned int* cps, int n,
                                int direction, unsigned char* levels)
{
    // P2-P3: paragraph level.
    int para = 0;
    if (direction == DwShaperDirection_RTL) para = 1;
    else if (direction == DwShaperDirection_Auto)
    {
        for (int i = 0; i < n; ++i)
        {
            if (types[i] == DwBidi_L) { para = 0; break; }
            if (types[i] == DwBidi_R || types[i] == DwBidi_AL) { para = 1; break; }
        }
    }
    const unsigned char sos = (para & 1) ? DwBidi_R : DwBidi_L;
    const unsigned char eos = sos;

    // W1: NSM takes the type of the previous character (sos at the start).
    // BN is "removed" (X9): give it the previous type too so it never splits a run.
    for (int i = 0; i < n; ++i)
        if (types[i] == DwBidi_NSM || types[i] == DwBidi_BN)
            types[i] = (i == 0) ? sos : types[i - 1];

    // W2: EN preceded (through any non-strong) by AL becomes AN.  W3: AL -> R.
    {
        unsigned char lastStrong = sos;
        for (int i = 0; i < n; ++i)
        {
            unsigned char t = types[i];
            if (t == DwBidi_L || t == DwBidi_R || t == DwBidi_AL) lastStrong = t;
            else if (t == DwBidi_EN && lastStrong == DwBidi_AL) types[i] = DwBidi_AN;
        }
        for (int i = 0; i < n; ++i)
            if (types[i] == DwBidi_AL) types[i] = DwBidi_R;
    }

    // W4: a single ES between two EN -> EN; a single CS between two numbers of the same type -> that type.
    for (int i = 1; i + 1 < n; ++i)
    {
        unsigned char a = types[i - 1], b = types[i + 1];
        if (types[i] == DwBidi_ES && a == DwBidi_EN && b == DwBidi_EN) types[i] = DwBidi_EN;
        else if (types[i] == DwBidi_CS && a == b && (a == DwBidi_EN || a == DwBidi_AN)) types[i] = a;
    }

    // W5: a sequence of ET adjacent to an EN becomes EN.
    for (int i = 0; i < n; )
    {
        if (types[i] != DwBidi_ET) { ++i; continue; }
        int j = i;
        while (j < n && types[j] == DwBidi_ET) ++j;
        bool touchesEN = (i > 0 && types[i - 1] == DwBidi_EN) || (j < n && types[j] == DwBidi_EN);
        if (touchesEN) for (int k = i; k < j; ++k) types[k] = DwBidi_EN;
        i = j;
    }

    // W6: remaining separators and terminators become neutral.
    for (int i = 0; i < n; ++i)
        if (types[i] == DwBidi_ES || types[i] == DwBidi_ET || types[i] == DwBidi_CS) types[i] = DwBidi_NI;

    // W7: EN preceded by L (or sos L) becomes L.
    {
        unsigned char lastStrong = sos;
        for (int i = 0; i < n; ++i)
        {
            unsigned char t = types[i];
            if (t == DwBidi_L || t == DwBidi_R) lastStrong = t;
            else if (t == DwBidi_EN && lastStrong == DwBidi_L) types[i] = DwBidi_L;
        }
    }

    // N1-N2: neutrals take the direction of their surroundings when both sides
    // agree (EN/AN count as R), otherwise the embedding direction.
    for (int i = 0; i < n; )
    {
        if (types[i] != DwBidi_NI) { ++i; continue; }
        int j = i;
        while (j < n && types[j] == DwBidi_NI) ++j;
        auto strongOf = []( unsigned char t ) -> unsigned char
        {
            return ( t == DwBidi_EN || t == DwBidi_AN ) ? (unsigned char)DwBidi_R : t;
        };
        unsigned char before = (i == 0) ? sos : strongOf(types[i - 1]);
        unsigned char after  = (j == n) ? eos : strongOf(types[j]);
        unsigned char res    = (before == after) ? before : sos;
        for (int k = i; k < j; ++k) types[k] = res;
        i = j;
    }

    // I1-I2: implicit levels.
    for (int i = 0; i < n; ++i)
    {
        unsigned char t = types[i];
        int lv = para;
        if ((para & 1) == 0)
        {
            if (t == DwBidi_R) lv += 1;
            else if (t == DwBidi_AN || t == DwBidi_EN) lv += 2;
        }
        else
        {
            if (t == DwBidi_L || t == DwBidi_EN || t == DwBidi_AN) lv += 1;
        }
        levels[i] = (unsigned char)lv;
    }

    // L1: trailing whitespace goes back to the paragraph level.
    for (int i = n - 1; i >= 0 && DwBidi_IsWhitespace(cps[i]); --i)
        levels[i] = (unsigned char)para;

    return para;
}

void DwShaper_TextBreaks(const char* text, int len, unsigned int* out_flags)
{
    if (!out_flags) return;
    memset(out_flags, 0, sizeof(unsigned int) * (size_t)(len + 1));
    if (!text || len <= 0) return;
    int breakCount = 0, flagCount = 0;
    kbts_BreakEntireStringUtf8(KBTS_DIRECTION_DONT_KNOW, KBTS_JAPANESE_LINE_BREAK_STYLE_NORMAL, 0,
                               text, len, nullptr, 0, &breakCount,
                               (kbts_break_flags*)out_flags, len + 1, &flagCount);
}

int DwShaper_ParagraphLevel(const char* text, int len, int direction)
{
    if (direction == DwShaperDirection_LTR) return 0;
    if (direction == DwShaperDirection_RTL) return 1;
    const char* p = text;
    const char* e = text + len;
    while (p < e)
    {
        unsigned int cp = 0;
        int adv = ImTextCharFromUtf8(&cp, p, e);
        if (adv <= 0) break;
        unsigned char t = kbts__GetUnicodeBidirectionalClass(cp);
        if (t == DwBidi_L) return 0;
        if (t == DwBidi_R || t == DwBidi_AL) return 1;
        p += adv;
    }
    return 0;
}

bool DwShaper_IsStrong(unsigned int cp)
{
    unsigned char t = kbts__GetUnicodeBidirectionalClass(cp);
    return t == DwBidi_L || t == DwBidi_R || t == DwBidi_AL;
}

int DwShaper_ResolveLevels(const char* text, int len, int direction, unsigned char* out_levels)
{
    if (!text || len <= 0 || !out_levels) return 0;
    ImVector<unsigned int>  cps;
    ImVector<int>           offs;
    ImVector<unsigned char> types, levels;
    const char* p = text;
    const char* e = text + len;
    while (p < e)
    {
        unsigned int cp = 0;
        int adv = ImTextCharFromUtf8(&cp, p, e);
        if (adv <= 0) break;
        offs.push_back((int)(p - text));
        cps.push_back(cp);
        p += adv;
    }
    offs.push_back(len);
    const int n = cps.Size;
    if (n == 0) return 0;
    types.resize(n); levels.resize(n);
    for (int i = 0; i < n; ++i) types[i] = kbts__GetUnicodeBidirectionalClass(cps[i]);
    const int para = DwBidi_ResolveLevels(types.Data, cps.Data, n, direction, levels.Data);
    for (int i = 0; i < n; ++i)
        for (int b = offs[i]; b < offs[i + 1]; ++b) out_levels[b] = levels[i];
    return para;
}

int DwShaper_ShapeBidi(DwShaperCtx* ctx, const char* text, int len, int direction,
                       DwShaperEmitFn emit, void* user)
{
    if (!ctx || !ctx->kbts || !ctx->font || !text || len <= 0 || !emit)
        return 0;

    // Decode once: codepoints, their byte offsets and bidi types.
    ImVector<unsigned int>  cps;
    ImVector<int>           offs;
    ImVector<unsigned char> types, levels;
    cps.reserve(len); offs.reserve(len + 1);
    {
        const char* p = text;
        const char* e = text + len;
        while (p < e)
        {
            unsigned int cp = 0;
            int adv = ImTextCharFromUtf8(&cp, p, e);
            if (adv <= 0) break;
            offs.push_back((int)(p - text));
            cps.push_back(cp);
            p += adv;
        }
        offs.push_back(len);
    }
    const int n = cps.Size;
    if (n == 0) return 0;
    types.resize(n); levels.resize(n);
    for (int i = 0; i < n; ++i)
        types[i] = kbts__GetUnicodeBidirectionalClass(cps[i]);
    const int para = DwBidi_ResolveLevels(types.Data, cps.Data, n, direction, levels.Data);

    // Level runs (logical order).
    struct LevelRun { int cpBegin, cpEnd, level; int glyphBegin, glyphEnd; };
    ImVector<LevelRun> runs;
    for (int i = 0; i < n; )
    {
        int j = i + 1;
        while (j < n && levels[j] == levels[i]) ++j;
        LevelRun r = { i, j, levels[i], 0, 0 };
        runs.push_back(r);
        i = j;
    }

    // Shape every run with its own direction. kbts returns each run's glyphs
    // already in visual order (RTL runs flipped), which is what L2 needs for an
    // atomic run: a run at level L is reversed L times, i.e. once iff odd.
    ImVector<DwShaperGlyph> glyphs;
    for (int r = 0; r < runs.Size; ++r)
    {
        LevelRun& lr = runs[r];
        lr.glyphBegin = glyphs.Size;
        const int bBegin = offs[lr.cpBegin];
        const int bEnd   = offs[lr.cpEnd];
        kbts_ShapeBegin(ctx->kbts, (lr.level & 1) ? KBTS_DIRECTION_RTL : KBTS_DIRECTION_LTR, KBTS_LANGUAGE_DONT_KNOW);
        kbts_ShapeUtf8(ctx->kbts, text + bBegin, (kbts_u32)(bEnd - bBegin), KBTS_USER_ID_GENERATION_MODE_CODEPOINT_INDEX);
        kbts_ShapeEnd(ctx->kbts);
        // kbts may split one level run into several (script / font changes,
        // e.g. Hebrew next to Arabic). It returns them in logical order with
        // each one flipped internally; for an odd level the pieces themselves
        // must also be emitted last-to-first.
        ImVector<int> subStarts;
        kbts_run kr;
        while (kbts_ShapeRun(ctx->kbts, &kr))
        {
            subStarts.push_back(glyphs.Size);
            // kbts segments direction on its own and flips any run it considers
            // RTL (e.g. Arabic-Indic digits, class AN). Our levels are
            // authoritative: undo the flip when kbts disagrees with them.
            const int  subBegin = glyphs.Size;
            const bool kbtsRtl  = (kr.Direction == KBTS_DIRECTION_RTL);
            const bool wantRtl  = (lr.level & 1) != 0;
            kbts_glyph* g = nullptr;
            while (kbts_GlyphIteratorNext(&kr.Glyphs, &g))
            {
                DwShaperGlyph o;
                o.GlyphId  = (unsigned int)g->Id;
                o.AdvanceX = (int)g->AdvanceX;
                o.OffsetX  = (int)g->OffsetX;
                o.OffsetY  = (int)g->OffsetY;
                int ci = lr.cpBegin + g->UserIdOrCodepointIndex;
                if (ci < lr.cpBegin) ci = lr.cpBegin;
                if (ci >= lr.cpEnd)  ci = lr.cpEnd - 1;
                o.Cluster  = offs[ci];
                o.Level    = lr.level;
                glyphs.push_back(o);
            }
            if (kbtsRtl != wantRtl)
                for (int a = subBegin, b = glyphs.Size - 1; a < b; ++a, --b)
                {
                    DwShaperGlyph t = glyphs[a]; glyphs[a] = glyphs[b]; glyphs[b] = t;
                }
        }
        if ((lr.level & 1) && subStarts.Size > 1)
        {
            ImVector<DwShaperGlyph> tmp;
            tmp.reserve(glyphs.Size - lr.glyphBegin);
            for (int k = subStarts.Size - 1; k >= 0; --k)
            {
                int b0 = subStarts[k];
                int b1 = (k + 1 < subStarts.Size) ? subStarts[k + 1] : glyphs.Size;
                for (int gi = b0; gi < b1; ++gi) tmp.push_back(glyphs[gi]);
            }
            for (int k = 0; k < tmp.Size; ++k) glyphs[lr.glyphBegin + k] = tmp[k];
        }
        lr.glyphEnd = glyphs.Size;
    }

    // L2: from the highest level down to the lowest odd level, reverse every
    // maximal sequence of runs at that level or higher.
    ImVector<int> order;
    order.resize(runs.Size);
    int maxLevel = 0, minOdd = 255;
    for (int r = 0; r < runs.Size; ++r)
    {
        order[r] = r;
        maxLevel = ImMax(maxLevel, runs[r].level);
        if (runs[r].level & 1) minOdd = ImMin(minOdd, runs[r].level);
    }
    for (int lv = maxLevel; lv >= minOdd && lv > 0; --lv)
    {
        for (int i = 0; i < order.Size; )
        {
            if (runs[order[i]].level < lv) { ++i; continue; }
            int j = i;
            while (j < order.Size && runs[order[j]].level >= lv) ++j;
            for (int a = i, b = j - 1; a < b; ++a, --b) { int t = order[a]; order[a] = order[b]; order[b] = t; }
            i = j;
        }
    }

    for (int k = 0; k < order.Size; ++k)
    {
        const LevelRun& lr = runs[order[k]];
        for (int gi = lr.glyphBegin; gi < lr.glyphEnd; ++gi)
            emit(user, &glyphs[gi]);
    }
    return para;
}

} // namespace ImWidgets

