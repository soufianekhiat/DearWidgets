// dear_widgets_text_shape.cpp -- text-shaping backend implementations.
// Compiled as its own translation unit; dear_widgets_slug.cpp includes the
// header and the linker resolves calls via namespace ImWidgets.
#include "imgui.h"   // IM_ALLOC / IM_FREE
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
    return true;
}

} // namespace ImWidgets
