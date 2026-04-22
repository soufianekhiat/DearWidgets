// dear_widgets_text_shape.cpp -- text-shaping backend implementations.
// Compiled as its own translation unit; dear_widgets_slug.cpp includes the
// header and the linker resolves calls via namespace ImWidgets.
#include "imgui.h"   // IM_ALLOC / IM_FREE
#include <cstring>   // memset

// ── Backend-specific includes (outside namespace — they define global types) ──

#if defined(DW_SHAPER_BACKEND_HARFBUZZ)
#include <hb.h>
#else
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
#endif // DW_SHAPER_BACKEND_HARFBUZZ

// ─────────────────────────────────────────────────────────────────────────────
namespace ImWidgets {
// Include declarations inside the namespace so symbols match what callers
// (compiled inside namespace ImWidgets via dear_widgets.cpp) expect.
#include "dear_widgets_text_shape.h"

// ─────────────────────────────────────────────────────────────────────────────

#if defined(DW_SHAPER_BACKEND_HARFBUZZ)

struct DwShaperCtx
{
    hb_blob_t*              blob;
    hb_face_t*              face;
    hb_font_t*              font;
    bool                    ready;
    hb_feature_t            features[8];
    int                     feature_count;

    hb_buffer_t*            buf;
    hb_glyph_info_t*        infos;
    hb_glyph_position_t*    positions;
    unsigned int            glyph_count;
    unsigned int            glyph_cursor;
    bool                    run_consumed;
};

DwShaperCtx* DwShaper_Create()
{
    DwShaperCtx* ctx = (DwShaperCtx*)IM_ALLOC(sizeof(DwShaperCtx));
    memset(ctx, 0, sizeof(DwShaperCtx));
    return ctx;
}

void DwShaper_Destroy(DwShaperCtx* ctx)
{
    if (!ctx) return;
    if (ctx->buf)  hb_buffer_destroy(ctx->buf);
    if (ctx->font) hb_font_destroy(ctx->font);
    if (ctx->face) hb_face_destroy(ctx->face);
    if (ctx->blob) hb_blob_destroy(ctx->blob);
    IM_FREE(ctx);
}

bool DwShaper_IsReady(DwShaperCtx* ctx) { return ctx && ctx->ready; }

bool DwShaper_PushFontFromMemory(DwShaperCtx* ctx, void* data, int size)
{
    ctx->blob = hb_blob_create((const char*)data, (unsigned)size,
                               HB_MEMORY_MODE_DUPLICATE, nullptr, nullptr);
    ctx->face = hb_face_create(ctx->blob, 0);
    ctx->font = hb_font_create(ctx->face);
    unsigned int upem = hb_face_get_upem(ctx->face);
    hb_font_set_scale(ctx->font, (int)upem, (int)upem);
    ctx->ready = (ctx->font != nullptr);
    return ctx->ready;
}

void DwShaper_PushFeature(DwShaperCtx* ctx, unsigned tag, unsigned value)
{
    if (ctx->feature_count >= 8) return;
    hb_feature_t& f = ctx->features[ctx->feature_count++];
    f.tag   = (hb_tag_t)tag;
    f.value = value;
    f.start = HB_FEATURE_GLOBAL_START;
    f.end   = HB_FEATURE_GLOBAL_END;
}

void DwShaper_Begin(DwShaperCtx* ctx)
{
    if (ctx->buf) hb_buffer_reset(ctx->buf);
    else          ctx->buf = hb_buffer_create();
    ctx->glyph_count  = 0;
    ctx->glyph_cursor = 0;
    ctx->run_consumed = false;
}

void DwShaper_PushUtf8(DwShaperCtx* ctx, const char* text, int len)
{
    hb_buffer_add_utf8(ctx->buf, text, len, 0, len);
}

void DwShaper_End(DwShaperCtx* ctx)
{
    hb_buffer_guess_segment_properties(ctx->buf);
    hb_shape(ctx->font, ctx->buf, ctx->features, (unsigned)ctx->feature_count);
    ctx->infos     = hb_buffer_get_glyph_infos(ctx->buf, &ctx->glyph_count);
    ctx->positions = hb_buffer_get_glyph_positions(ctx->buf, nullptr);
    ctx->glyph_cursor = 0;
}

bool DwShaper_NextRun(DwShaperCtx* ctx, DwShaperRun* run)
{
    if (ctx->run_consumed || ctx->glyph_count == 0)
        return false;
    ctx->run_consumed = true;
    run->_p = ctx;
    return true;
}

bool DwShaper_NextGlyph(DwShaperRun* run, DwShaperGlyph* out)
{
    DwShaperCtx* ctx = (DwShaperCtx*)run->_p;
    if (ctx->glyph_cursor >= ctx->glyph_count)
        return false;
    unsigned int i = ctx->glyph_cursor++;
    out->GlyphId  = ctx->infos[i].codepoint;
    out->AdvanceX = ctx->positions[i].x_advance;
    out->OffsetX  = ctx->positions[i].x_offset;
    out->OffsetY  = ctx->positions[i].y_offset;
    return true;
}

#else // kb_text_shape backend ────────────────────────────────────────────────

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

#endif // DW_SHAPER_BACKEND_HARFBUZZ

} // namespace ImWidgets
