// dear_widgets_path.cpp -- vector paths on a plain ImDrawList.
//
// Compiled as its own translation unit (standalone, resolved via the linker).
//
//   * FillPolygons / DW_FillSegments: anti-aliased polygon fill with the SVG /
//     Canvas2D fill rules (nonzero, evenodd) for any set of contours, including
//     self-intersecting ones and holes. Pure ImDrawList triangles, so it works
//     on stock Dear ImGui with no custom shader.
//   * ImWidgetsPath: a Canvas2D / SVG style path (moveTo, lineTo, quadratic and
//     cubic curves, SVG elliptical arcs, closePath, rect / ellipse / rounded
//     rect helpers, 2x3 transforms) and an SVG path-data ("d") parser.
//   * PathFill / PathStroke: draw such a path.
//
// Fill algorithm: trapezoid decomposition. The y range is cut into slabs at
// every vertex (and at every edge crossing, so self-intersecting outlines are
// exact); inside a slab the edges do not cross, so sorting them by x and
// walking the winding number gives the covered spans. Each span is a
// trapezoid. Anti-aliasing is a 1 px fringe (alpha ramp) drawn outward along
// every boundary piece: the left/right edges of the spans, and the horizontal
// pieces where coverage starts or stops between two slabs.

#include "dear_widgets.h"
#include "dear_widgets_internal.h"
#include "imgui_internal.h"

namespace ImWidgets {

// ----------------------------------------------------------------------------
// Polygon fill
// ----------------------------------------------------------------------------

struct DW_FillEdge
{
    float x0, y0, x1, y1; // y0 < y1 (top to bottom)
    int   dir;            // +1 if the original segment went downwards, -1 upwards
};

struct DW_SlabEdge
{
    float xa, xb;         // x at the slab top / bottom
    int   dir;
    int   id;             // index into the edge array
};

// A span between two edges that stays open over consecutive slabs: it is only
// emitted (one trapezoid + its side fringes) when its bounding edges change,
// so the output size follows the number of edges, not slabs x spans.
struct DW_OpenSpan
{
    int   L, R;           // edge ids
    float y0;             // where it started
};

static inline float DW_EdgeXAt(const DW_FillEdge& e, float y)
{
    float t = (y - e.y0) / (e.y1 - e.y0);
    return e.x0 + (e.x1 - e.x0) * t;
}

struct DW_FillWriter
{
    ImDrawList* dl;
    ImU32       col;      // full colour
    ImU32       colTrans; // same colour, alpha 0
    float       aa;       // fringe width in pixels (0 = no AA)
    ImVec2      uv;

    void Quad(ImVec2 a, ImVec2 b, ImVec2 c, ImVec2 d, ImU32 ca, ImU32 cb, ImU32 cc, ImU32 cd)
    {
        dl->PrimReserve(6, 4);
        ImDrawIdx base = (ImDrawIdx)dl->_VtxCurrentIdx;
        dl->PrimWriteVtx(a, uv, ca);
        dl->PrimWriteVtx(b, uv, cb);
        dl->PrimWriteVtx(c, uv, cc);
        dl->PrimWriteVtx(d, uv, cd);
        dl->PrimWriteIdx(base); dl->PrimWriteIdx((ImDrawIdx)(base + 1)); dl->PrimWriteIdx((ImDrawIdx)(base + 2));
        dl->PrimWriteIdx(base); dl->PrimWriteIdx((ImDrawIdx)(base + 2)); dl->PrimWriteIdx((ImDrawIdx)(base + 3));
    }

    // Fringe along a boundary piece p->q whose outward normal is n (unit).
    // Opaque half lies inside (already covered by the trapezoid shrink), the
    // transparent half outside, so the visual edge stays where the path is.
    void Fringe(ImVec2 p, ImVec2 q, ImVec2 n)
    {
        if (aa <= 0.0f) return;
        const float h = aa * 0.5f;
        ImVec2 pi(p.x - n.x * h, p.y - n.y * h), qi(q.x - n.x * h, q.y - n.y * h);
        ImVec2 po(p.x + n.x * h, p.y + n.y * h), qo(q.x + n.x * h, q.y + n.y * h);
        Quad(pi, qi, qo, po, col, col, colTrans, colTrans);
    }

    // Outer-half fringe: starts at the edge with half alpha and fades out over
    // half the fringe width. Used where the fill is not shrunk (horizontal and
    // near-horizontal boundaries), so it never overlaps the fill -- an
    // overlapping inner half would show as a darker band on translucent fills.
    void FringeOuter(ImVec2 p, ImVec2 q, ImVec2 n)
    {
        if (aa <= 0.0f) return;
        const float h = aa * 0.5f;
        ImVec2 po(p.x + n.x * h, p.y + n.y * h), qo(q.x + n.x * h, q.y + n.y * h);
        Quad(p, q, qo, po, colHalf, colHalf, colTrans, colTrans);
    }
    ImU32 colHalf;
};

static inline bool DW_FillInside(int winding, ImWidgetsFillRule rule)
{
    return rule == ImWidgetsFillRule_EvenOdd ? (winding & 1) != 0 : winding != 0;
}

// Spans of one horizontal line through the slab (at its top or bottom), used
// to find horizontal boundary pieces between consecutive slabs.
struct DW_Span { float l, r; };

// Appends the parts of `a` that are not covered by `b` (both sorted, disjoint).
static void DW_SpanDifference(const ImVector<DW_Span>& a, const ImVector<DW_Span>& b, ImVector<DW_Span>& out)
{
    out.resize(0);
    int j = 0;
    for (int i = 0; i < a.Size; ++i)
    {
        float l = a[i].l, r = a[i].r;
        while (j < b.Size && b[j].r <= l) ++j;
        int k = j;
        while (l < r)
        {
            if (k >= b.Size || b[k].l >= r) { DW_Span s = { l, r }; out.push_back(s); break; }
            if (b[k].l > l) { DW_Span s = { l, b[k].l }; out.push_back(s); }
            l = ImMax(l, b[k].r);
            ++k;
        }
    }
}

// Core: fill a soup of directed segments (pairs of points). Contours do not
// need to be explicit -- winding only depends on the directed edges.
void DW_FillSegments(ImDrawList* dl, const ImVec2* seg_pts, int seg_count, ImU32 col,
                     ImWidgetsFillRule rule, float aa)
{
    if (!dl || !seg_pts || seg_count <= 0 || (col & IM_COL32_A_MASK) == 0) return;

    ImVector<DW_FillEdge> edges;
    edges.reserve(seg_count);
    ImVector<float> ys;
    ys.reserve(seg_count * 2);
    for (int i = 0; i < seg_count; ++i)
    {
        ImVec2 a = seg_pts[i * 2], b = seg_pts[i * 2 + 1];
        // NaN or inf: skip (x - x is NaN for both).
        if (!(a.x - a.x == 0.0f) || !(a.y - a.y == 0.0f) || !(b.x - b.x == 0.0f) || !(b.y - b.y == 0.0f)) continue;
        if (a.y == b.y) continue;   // horizontal edges never change the winding
        DW_FillEdge e;
        if (a.y < b.y) { e.x0 = a.x; e.y0 = a.y; e.x1 = b.x; e.y1 = b.y; e.dir = +1; }
        else           { e.x0 = b.x; e.y0 = b.y; e.x1 = a.x; e.y1 = a.y; e.dir = -1; }
        edges.push_back(e);
        ys.push_back(e.y0);
        ys.push_back(e.y1);
    }
    if (edges.Size == 0) return;

    // Clip the work to the draw list's clip rect (vertically): slabs fully
    // outside it are skipped, which keeps huge off-screen paths cheap.
    const ImVec4 clip = dl->_CmdHeader.ClipRect;

    ImQsort(ys.Data, (size_t)ys.Size, sizeof(float), [](const void* a, const void* b) -> int {
        float fa = *(const float*)a, fb = *(const float*)b; return fa < fb ? -1 : fa > fb ? 1 : 0; });
    {
        int w = 0;
        // Exact dedupe: every edge endpoint must be a slab boundary, otherwise a
        // nearly horizontal edge (top of a flattened circle) spans no slab and
        // the slab sees an odd number of edges.
        for (int i = 0; i < ys.Size; ++i)
            if (w == 0 || ys[i] != ys[w - 1]) ys[w++] = ys[i];
        ys.resize(w);
    }
    ImQsort(edges.Data, (size_t)edges.Size, sizeof(DW_FillEdge), [](const void* a, const void* b) -> int {
        float fa = ((const DW_FillEdge*)a)->y0, fb = ((const DW_FillEdge*)b)->y0; return fa < fb ? -1 : fa > fb ? 1 : 0; });

    DW_FillWriter w;
    w.dl = dl;
    w.col = col;
    w.colTrans = col & ~IM_COL32_A_MASK;
    w.colHalf  = w.colTrans | ((((col >> IM_COL32_A_SHIFT) & 0xFF) / 2) << IM_COL32_A_SHIFT);
    w.aa = aa;
    w.uv = dl->_Data->TexUvWhitePixel;

    ImVector<int>         active;     // indices into edges spanning the current y
    ImVector<DW_SlabEdge> slab;
    ImVector<float>       subYs;
    ImVector<DW_Span>     prevBottom, curTop, curBottom, diff;
    float prevBottomY = -FLT_MAX;
    int next = 0;

    auto sortSlab = [](ImVector<DW_SlabEdge>& v) {
        // By midpoint x (no crossings inside a slab, so any x works).
        if (v.Size > 24)
        {
            ImQsort(v.Data, (size_t)v.Size, sizeof(DW_SlabEdge), [](const void* a, const void* b) -> int {
                float fa = ((const DW_SlabEdge*)a)->xa + ((const DW_SlabEdge*)a)->xb;
                float fb = ((const DW_SlabEdge*)b)->xa + ((const DW_SlabEdge*)b)->xb;
                return fa < fb ? -1 : fa > fb ? 1 : 0; });
            return;
        }
        for (int i = 1; i < v.Size; ++i)
        {
            DW_SlabEdge t = v[i];
            float key = t.xa + t.xb;
            int j = i - 1;
            while (j >= 0 && v[j].xa + v[j].xb > key) { v[j + 1] = v[j]; --j; }
            v[j + 1] = t;
        }
    };

    ImVector<DW_OpenSpan> open, nextOpen;

    // Emits the trapezoid of a span bounded by edges L and R between y0 and y1,
    // shrunk by half the fringe along each steep edge, plus its side fringes.
    auto flushSpan = [&](const DW_OpenSpan& sp, float y1)
    {
        const float y0 = sp.y0;
        if (y1 - y0 <= 0.0f) return;
        const DW_FillEdge& EL = edges[sp.L];
        const DW_FillEdge& ER = edges[sp.R];
        const float La = DW_EdgeXAt(EL, y0), Lb = DW_EdgeXAt(EL, y1);
        const float Ra = DW_EdgeXAt(ER, y0), Rb = DW_EdgeXAt(ER, y1);
        // Steep edges (|n.x| >= 0.35): shrink by h / |n.x| and use a centred
        // fringe. Shallow ones: no shrink (it would need a huge horizontal
        // shift) and an outer-half fringe instead.
        const float h = aa * 0.5f;
        auto edgeNx = [](const DW_FillEdge& e) -> float {
            float dx = e.x1 - e.x0, dy = e.y1 - e.y0;
            float len = ImSqrt(dx * dx + dy * dy);
            return len > 0.0f ? dy / len : 1.0f;
        };
        const float nxL = edgeNx(EL), nxR = edgeNx(ER);
        const bool steepL = nxL >= 0.35f, steepR = nxR >= 0.35f;
        const float shL = (aa > 0.0f && steepL) ? h / nxL : 0.0f;
        const float shR = (aa > 0.0f && steepR) ? h / nxR : 0.0f;
        float lta = La + shL, ltb = Lb + shL;
        float rta = Ra - shR, rtb = Rb - shR;
        if (rta < lta) { float c = (La + Ra) * 0.5f; lta = rta = c; }
        if (rtb < ltb) { float c = (Lb + Rb) * 0.5f; ltb = rtb = c; }
        w.Quad(ImVec2(lta, y0), ImVec2(rta, y0), ImVec2(rtb, y1), ImVec2(ltb, y1), col, col, col, col);
        if (aa > 0.0f)
        {
            // Outward normals: the left edge points to -x, the right one to +x.
            ImVec2 lp(La, y0), lq(Lb, y1);
            float ldx = lq.x - lp.x, ldy = lq.y - lp.y, ll = ImSqrt(ldx * ldx + ldy * ldy);
            if (ll > 0.0f) { ImVec2 n(-ldy / ll, ldx / ll); if (steepL) w.Fringe(lp, lq, n); else w.FringeOuter(lp, lq, n); }
            ImVec2 rp(Ra, y0), rq(Rb, y1);
            float rdx = rq.x - rp.x, rdy = rq.y - rp.y, rl = ImSqrt(rdx * rdx + rdy * rdy);
            if (rl > 0.0f) { ImVec2 n(rdy / rl, -rdx / rl); if (steepR) w.Fringe(rp, rq, n); else w.FringeOuter(rp, rq, n); }
        }
    };
    auto flushAll = [&](float y1)
    {
        for (int i = 0; i < open.Size; ++i) flushSpan(open[i], y1);
        open.resize(0);
    };

    // Processes one crossing-free slab [ya, yb]: continues or opens spans and
    // records its top / bottom x intervals for the horizontal boundaries.
    auto emitSlab = [&](float ya, float yb)
    {
        // Every active edge spans the whole parent slab (all endpoints are slab
        // boundaries), hence every sub-slab too.
        slab.resize(0);
        for (int k = 0; k < active.Size; ++k)
        {
            const DW_FillEdge& e = edges[active[k]];
            DW_SlabEdge s = { DW_EdgeXAt(e, ya), DW_EdgeXAt(e, yb), e.dir, active[k] };
            slab.push_back(s);
        }
        sortSlab(slab);
        // Coincident edges (shared edge of two shapes, zero-area spikes) must
        // cancel or add as one edge, whatever order the sort left them in.
        {
            int wi = 0;
            for (int k = 0; k < slab.Size; ++k)
            {
                if (wi > 0 && slab[wi - 1].xa == slab[k].xa && slab[wi - 1].xb == slab[k].xb)
                    slab[wi - 1].dir += slab[k].dir;
                else
                    slab[wi++] = slab[k];
            }
            slab.resize(wi);
            wi = 0;
            for (int k = 0; k < slab.Size; ++k)
                if (slab[k].dir != 0) slab[wi++] = slab[k];
            slab.resize(wi);
        }
        curTop.resize(0);
        curBottom.resize(0);
        nextOpen.resize(0);
        int openCursor = 0;
        int winding = 0;
        for (int k = 0; k < slab.Size; )
        {
            bool wasIn = DW_FillInside(winding, rule);
            winding += slab[k].dir;
            bool isIn = DW_FillInside(winding, rule);
            if (!wasIn && isIn)
            {
                // span starts at edge k; find where it ends
                const DW_SlabEdge& L = slab[k];
                int m = k + 1;
                for (; m < slab.Size; ++m)
                {
                    winding += slab[m].dir;
                    if (!DW_FillInside(winding, rule)) break;
                }
                if (m >= slab.Size) break; // malformed (open) input: nothing to close the span
                const DW_SlabEdge& R = slab[m];
                if (R.xa <= L.xa && R.xb <= L.xb) { k = m + 1; continue; } // zero width: nothing to draw

                DW_OpenSpan sp = { L.id, R.id, ya };
                // Both lists are in x order, so a moving cursor finds matches in linear time.
                for (int o = openCursor; o < open.Size; ++o)
                    if (open[o].L == L.id && open[o].R == R.id)
                    {
                        sp.y0 = open[o].y0;          // same edges: keep growing it
                        open[o].L = -1;              // consumed
                        openCursor = o + 1;
                        break;
                    }
                nextOpen.push_back(sp);

                DW_Span st = { L.xa, R.xa }, sb = { L.xb, R.xb };
                if (st.r > st.l) curTop.push_back(st);
                if (sb.r > sb.l) curBottom.push_back(sb);
                k = m + 1;
                continue;
            }
            ++k;
        }
        // Spans that did not continue end at ya.
        for (int o = 0; o < open.Size; ++o)
            if (open[o].L >= 0) flushSpan(open[o], ya);
        open.swap(nextOpen);

        // Horizontal boundaries against the previous slab (only if contiguous).
        if (aa > 0.0f)
        {
            const bool contiguous = ImFabs(prevBottomY - ya) < 1e-4f;
            static const ImVector<DW_Span> kEmpty;
            // Coverage that starts at ya: top edge, fringe upward.
            DW_SpanDifference(curTop, contiguous ? prevBottom : kEmpty, diff);
            for (int i = 0; i < diff.Size; ++i)
                w.FringeOuter(ImVec2(diff[i].l, ya), ImVec2(diff[i].r, ya), ImVec2(0.0f, -1.0f));
            // Coverage that stopped at ya: bottom edge of the previous slab, fringe downward.
            if (contiguous)
            {
                DW_SpanDifference(prevBottom, curTop, diff);
                for (int i = 0; i < diff.Size; ++i)
                    w.FringeOuter(ImVec2(diff[i].l, ya), ImVec2(diff[i].r, ya), ImVec2(0.0f, 1.0f));
            }
            else
            {
                for (int i = 0; i < prevBottom.Size; ++i)
                    w.FringeOuter(ImVec2(prevBottom[i].l, prevBottomY), ImVec2(prevBottom[i].r, prevBottomY), ImVec2(0.0f, 1.0f));
            }
        }
        prevBottom.swap(curBottom);
        prevBottomY = yb;
    };

    for (int yi = 0; yi + 1 < ys.Size; ++yi)
    {
        const float ya = ys[yi], yb = ys[yi + 1];

        // Update the active edge list: add edges starting at or above ya, drop finished ones.
        while (next < edges.Size && edges[next].y0 <= ya) active.push_back(next++);
        for (int k = 0; k < active.Size; )
        {
            if (edges[active[k]].y1 <= ya) { active[k] = active.back(); active.pop_back(); }
            else ++k;
        }
        if (ya > clip.w + aa) { flushAll(ya); break; }           // everything below is clipped
        if (yb < clip.y - aa) { flushAll(ya); continue; }

        // Edges that cross inside the slab change order between top and bottom:
        // split the slab at every crossing so each sub-slab is crossing-free.
        subYs.resize(0);
        subYs.push_back(ya);
        // Order the edges by x at the top, then insertion-sort them by x at the
        // bottom: every swap is exactly one pair of edges crossing inside the
        // slab, so this finds all crossings in O(n + crossings).
        int crossings = 0;
        if (active.Size > 1)
        {
            slab.resize(0);
            for (int k = 0; k < active.Size; ++k)
            {
                const DW_FillEdge& e = edges[active[k]];
                DW_SlabEdge se = { DW_EdgeXAt(e, ya), DW_EdgeXAt(e, yb), 0, active[k] };
                slab.push_back(se);
            }
            ImQsort(slab.Data, (size_t)slab.Size, sizeof(DW_SlabEdge), [](const void* a, const void* b) -> int {
                const DW_SlabEdge* ea = (const DW_SlabEdge*)a; const DW_SlabEdge* eb = (const DW_SlabEdge*)b;
                if (ea->xa != eb->xa) return ea->xa < eb->xa ? -1 : 1;
                return ea->xb < eb->xb ? -1 : ea->xb > eb->xb ? 1 : 0; });
            const int kMaxExact = 16;
            for (int i = 1; i < slab.Size; ++i)
            {
                DW_SlabEdge t = slab[i];
                int j = i - 1;
                // Relative epsilon: shared vertices far from the origin must not
                // count as crossings because of float rounding.
                const float eps = 1e-5f + 1e-6f * ImFabs(t.xb);
                while (j >= 0 && slab[j].xb > t.xb + eps)
                {
                    // slab[j] (left at the top) ends right of t at the bottom: they cross.
                    if (crossings < kMaxExact)
                    {
                        float da = slab[j].xa - t.xa, db = slab[j].xb - t.xb;
                        float tt = da / (da - db);
                        float yc = ya + (yb - ya) * tt;
                        if (yc > ya + 1e-4f && yc < yb - 1e-4f) subYs.push_back(yc);
                    }
                    ++crossings;
                    slab[j + 1] = slab[j];
                    --j;
                }
                slab[j + 1] = t;
            }
            // Many crossings in one slab (heavily self-intersecting input): cut
            // it into quarter-pixel sub-slabs instead of at every crossing.
            // Inside a sub-slab the crossing order is then off by at most 0.25 px.
            if (crossings > kMaxExact)
            {
                subYs.resize(1);
                // Quarter-pixel steps, but never more cuts than crossings would need.
                const int n = ImMax(2, ImMin((int)ImCeil((yb - ya) * 4.0f), crossings + 1));
                for (int k = 1; k < n; ++k) subYs.push_back(ya + (yb - ya) * (float)k / (float)n);
            }
        }
        subYs.push_back(yb);
        if (subYs.Size > 2)
        {
            ImQsort(subYs.Data, (size_t)subYs.Size, sizeof(float), [](const void* a, const void* b) -> int {
                float fa = *(const float*)a, fb = *(const float*)b; return fa < fb ? -1 : fa > fb ? 1 : 0; });
        }
        for (int s = 0; s + 1 < subYs.Size; ++s)
            if (subYs[s + 1] - subYs[s] > 1e-5f)
                emitSlab(subYs[s], subYs[s + 1]);
    }

    flushAll(prevBottomY);

    // Close the last bottom boundary.
    if (aa > 0.0f)
        for (int i = 0; i < prevBottom.Size; ++i)
            w.FringeOuter(ImVec2(prevBottom[i].l, prevBottomY), ImVec2(prevBottom[i].r, prevBottomY), ImVec2(0.0f, 1.0f));
}

void FillPolygons(ImDrawList* dl, const ImVec2* points, const int* contour_counts, int contour_count,
                  ImU32 col, ImWidgetsFillRule rule, float aa)
{
    if (!dl || !points || !contour_counts || contour_count <= 0) return;
    ImVector<ImVec2> segs;
    int base = 0;
    for (int c = 0; c < contour_count; ++c)
    {
        const int n = contour_counts[c];
        for (int i = 0; i < n && n >= 2; ++i)
        {
            segs.push_back(points[base + i]);
            segs.push_back(points[base + (i + 1) % n]);  // contours are implicitly closed
        }
        base += ImMax(n, 0);
    }
    DW_FillSegments(dl, segs.Data, segs.Size / 2, col, rule, aa);
}

} // namespace ImWidgets

// The path and transform types live at global scope (like every public
// DearWidgets type), so their members are defined outside the namespace.

// ----------------------------------------------------------------------------
// Transform
// ----------------------------------------------------------------------------

ImWidgetsTransform2D ImWidgetsTransform2D::Translate(float tx, float ty)
{
    ImWidgetsTransform2D m; m.m[4] = tx; m.m[5] = ty; return m;
}
ImWidgetsTransform2D ImWidgetsTransform2D::Scale(float sx, float sy)
{
    ImWidgetsTransform2D m; m.m[0] = sx; m.m[3] = sy; return m;
}
ImWidgetsTransform2D ImWidgetsTransform2D::Rotate(float radians)
{
    ImWidgetsTransform2D m;
    float c = ImCos(radians), s = ImSin(radians);
    m.m[0] = c; m.m[1] = s; m.m[2] = -s; m.m[3] = c;
    return m;
}
ImWidgetsTransform2D ImWidgetsTransform2D::operator*(const ImWidgetsTransform2D& o) const
{
    // (this * o)(p) = this(o(p)), SVG / Canvas2D order: the right one applies first.
    ImWidgetsTransform2D r;
    r.m[0] = m[0] * o.m[0] + m[2] * o.m[1];
    r.m[1] = m[1] * o.m[0] + m[3] * o.m[1];
    r.m[2] = m[0] * o.m[2] + m[2] * o.m[3];
    r.m[3] = m[1] * o.m[2] + m[3] * o.m[3];
    r.m[4] = m[0] * o.m[4] + m[2] * o.m[5] + m[4];
    r.m[5] = m[1] * o.m[4] + m[3] * o.m[5] + m[5];
    return r;
}

// ----------------------------------------------------------------------------
// Path
// ----------------------------------------------------------------------------
// Storage: every subpath is a chain of cubic Beziers (3N+1 points). Lines are
// stored as cubics with control points at 1/3 and 2/3, quadratics are degree
// elevated and arcs are approximated by cubics (<= 90 degrees each), so fill,
// stroke and transforms all work on one representation.

void ImWidgetsPath::Clear()
{
    Points.resize(0);
    Subpaths.resize(0);
}

void ImWidgetsPath::DW_EnsureSubpath()
{
    if (Subpaths.Size == 0 || Subpaths.back().Closed)
    {
        ImVec2 start = Subpaths.Size ? Points[Subpaths.back().First] : ImVec2(0, 0);
        MoveTo(start);
    }
}

ImWidgetsPath& ImWidgetsPath::MoveTo(ImVec2 p)
{
    // A moveTo right after another empty moveTo replaces it.
    if (Subpaths.Size && Subpaths.back().Count == 1 && !Subpaths.back().Closed)
    {
        Points.back() = p;
        return *this;
    }
    ImWidgetsSubpath sp;
    sp.First = Points.Size;
    sp.Count = 1;
    sp.Closed = false;
    Subpaths.push_back(sp);
    Points.push_back(p);
    return *this;
}

ImWidgetsPath& ImWidgetsPath::CubicTo(ImVec2 c1, ImVec2 c2, ImVec2 p)
{
    DW_EnsureSubpath();
    Points.push_back(c1);
    Points.push_back(c2);
    Points.push_back(p);
    Subpaths.back().Count += 3;
    return *this;
}

ImWidgetsPath& ImWidgetsPath::LineTo(ImVec2 p)
{
    if (Subpaths.Size == 0) return MoveTo(p); // Canvas2D: lineTo on an empty path is a moveTo
    DW_EnsureSubpath();
    ImVec2 a = Points.back();
    return CubicTo(ImLerp(a, p, 1.0f / 3.0f), ImLerp(a, p, 2.0f / 3.0f), p);
}

ImWidgetsPath& ImWidgetsPath::QuadTo(ImVec2 c, ImVec2 p)
{
    DW_EnsureSubpath();
    ImVec2 a = Points.back();
    return CubicTo(ImLerp(a, c, 2.0f / 3.0f), ImLerp(p, c, 2.0f / 3.0f), p);
}

ImWidgetsPath& ImWidgetsPath::Close()
{
    if (Subpaths.Size == 0 || Subpaths.back().Closed) return *this;
    ImWidgetsSubpath& sp = Subpaths.back();
    ImVec2 first = Points[sp.First], last = Points.back();
    if (sp.Count > 1 && (first.x != last.x || first.y != last.y))
        LineTo(first);
    Subpaths.back().Closed = true;
    return *this;
}

ImVec2 ImWidgetsPath::CurrentPoint() const
{
    return Points.Size ? Points.back() : ImVec2(0, 0);
}

// One cubic approximating the unit-circle arc from angle a0 to a1 (|a1-a0| <= pi/2),
// then mapped through the ellipse frame.
static void DW_ArcSegment(ImWidgetsPath& path, ImVec2 c, float rx, float ry, float cphi, float sphi, float a0, float a1)
{
    float k = 4.0f / 3.0f * tanf((a1 - a0) * 0.25f);
    float c0 = ImCos(a0), s0 = ImSin(a0), c1 = ImCos(a1), s1 = ImSin(a1);
    ImVec2 u1(c0 - k * s0, s0 + k * c0);
    ImVec2 u2(c1 + k * s1, s1 - k * c1);
    ImVec2 u3(c1, s1);
    auto map = [&](ImVec2 u) {
        float x = u.x * rx, y = u.y * ry;
        return ImVec2(c.x + cphi * x - sphi * y, c.y + sphi * x + cphi * y);
    };
    path.CubicTo(map(u1), map(u2), map(u3));
}

ImWidgetsPath& ImWidgetsPath::ArcTo(float rx, float ry, float x_axis_rotation_deg, bool large_arc, bool sweep, ImVec2 p)
{
    // SVG endpoint parameterisation -> centre parameterisation (SVG 1.1 F.6.5).
    DW_EnsureSubpath();
    ImVec2 p0 = Points.back();
    if (p0.x == p.x && p0.y == p.y) return *this;
    rx = ImFabs(rx); ry = ImFabs(ry);
    if (rx == 0.0f || ry == 0.0f) return LineTo(p);

    float phi = x_axis_rotation_deg * (IM_PI / 180.0f);
    float cphi = ImCos(phi), sphi = ImSin(phi);
    float dx2 = (p0.x - p.x) * 0.5f, dy2 = (p0.y - p.y) * 0.5f;
    float x1p =  cphi * dx2 + sphi * dy2;
    float y1p = -sphi * dx2 + cphi * dy2;

    // F.6.6: scale radii up if they cannot span the endpoints.
    float lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
    if (lambda > 1.0f) { float s = ImSqrt(lambda); rx *= s; ry *= s; }

    float rx2 = rx * rx, ry2 = ry * ry;
    float num = rx2 * ry2 - rx2 * y1p * y1p - ry2 * x1p * x1p;
    float den = rx2 * y1p * y1p + ry2 * x1p * x1p;
    float coef = den > 0.0f ? ImSqrt(ImMax(0.0f, num / den)) : 0.0f;
    if (large_arc == sweep) coef = -coef;
    float cxp =  coef * (rx * y1p / ry);
    float cyp = -coef * (ry * x1p / rx);
    ImVec2 c(cphi * cxp - sphi * cyp + (p0.x + p.x) * 0.5f,
             sphi * cxp + cphi * cyp + (p0.y + p.y) * 0.5f);

    auto angle = [](float ux, float uy, float vx, float vy) {
        float a = ImAtan2(ux * vy - uy * vx, ux * vx + uy * vy);
        return a;
    };
    float theta1 = angle(1.0f, 0.0f, (x1p - cxp) / rx, (y1p - cyp) / ry);
    float dtheta = angle((x1p - cxp) / rx, (y1p - cyp) / ry, (-x1p - cxp) / rx, (-y1p - cyp) / ry);
    if (!sweep && dtheta > 0.0f) dtheta -= 2.0f * IM_PI;
    else if (sweep && dtheta < 0.0f) dtheta += 2.0f * IM_PI;

    int segs = (int)ImCeil(ImFabs(dtheta) / (IM_PI * 0.5f) - 1e-4f);
    if (segs < 1) segs = 1;
    float step = dtheta / (float)segs;
    for (int i = 0; i < segs; ++i)
        DW_ArcSegment(*this, c, rx, ry, cphi, sphi, theta1 + step * i, theta1 + step * (i + 1));
    Points.back() = p; // land exactly on the endpoint
    return *this;
}

ImWidgetsPath& ImWidgetsPath::Arc(ImVec2 center, float radius, float a_min, float a_max, bool counter_clockwise)
{
    // Canvas2D arc(): lines to the arc start when a subpath is open.
    ImVec2 start(center.x + ImCos(a_min) * radius, center.y + ImSin(a_min) * radius);
    // Canvas2D: with no subpath, moveTo the arc start; otherwise (including right
    // after closePath, whose current point is the subpath start) lineTo it.
    if (Subpaths.Size == 0) MoveTo(start);
    else { DW_EnsureSubpath(); LineTo(start); }
    const float twoPi = 2.0f * IM_PI;
    float sweep;
    if (!counter_clockwise)
    {
        float d = a_max - a_min;
        if (d >= twoPi) sweep = twoPi;                              // full circle
        else { sweep = ImFmod(d, twoPi); if (sweep < 0.0f) sweep += twoPi; }
    }
    else
    {
        float d = a_min - a_max;
        if (d >= twoPi) sweep = -twoPi;
        else { sweep = ImFmod(d, twoPi); if (sweep < 0.0f) sweep += twoPi; sweep = -sweep; }
    }
    if (sweep == 0.0f) return *this;
    int segs = ImMax(1, (int)ImCeil(ImFabs(sweep) / (IM_PI * 0.5f) - 1e-4f));
    float step = sweep / (float)segs;
    for (int i = 0; i < segs; ++i)
        DW_ArcSegment(*this, center, radius, radius, 1.0f, 0.0f, a_min + step * i, a_min + step * (i + 1));
    return *this;
}

ImWidgetsPath& ImWidgetsPath::Rect(ImVec2 min, ImVec2 max)
{
    MoveTo(min).LineTo(ImVec2(max.x, min.y)).LineTo(max).LineTo(ImVec2(min.x, max.y)).Close();
    return *this;
}

ImWidgetsPath& ImWidgetsPath::RoundRect(ImVec2 min, ImVec2 max, float r)
{
    r = ImMin(r, ImMin((max.x - min.x) * 0.5f, (max.y - min.y) * 0.5f));
    if (r <= 0.0f) return Rect(min, max);
    MoveTo(ImVec2(min.x + r, min.y));
    LineTo(ImVec2(max.x - r, min.y));
    ArcTo(r, r, 0.0f, false, true, ImVec2(max.x, min.y + r));
    LineTo(ImVec2(max.x, max.y - r));
    ArcTo(r, r, 0.0f, false, true, ImVec2(max.x - r, max.y));
    LineTo(ImVec2(min.x + r, max.y));
    ArcTo(r, r, 0.0f, false, true, ImVec2(min.x, max.y - r));
    LineTo(ImVec2(min.x, min.y + r));
    ArcTo(r, r, 0.0f, false, true, ImVec2(min.x + r, min.y));
    return Close();
}

ImWidgetsPath& ImWidgetsPath::Ellipse(ImVec2 center, ImVec2 radii)
{
    const ImVec2 start(center.x + radii.x, center.y);
    MoveTo(start);
    for (int i = 0; i < 4; ++i)
        DW_ArcSegment(*this, center, radii.x, radii.y, 1.0f, 0.0f, IM_PI * 0.5f * i, IM_PI * 0.5f * (i + 1));
    Points.back() = start; // cos/sin(2*pi) in float is not exactly the start
    return Close();
}

ImWidgetsPath& ImWidgetsPath::Circle(ImVec2 center, float r)
{
    return Ellipse(center, ImVec2(r, r));
}

ImWidgetsPath& ImWidgetsPath::Transform(const ImWidgetsTransform2D& t)
{
    for (int i = 0; i < Points.Size; ++i)
        Points[i] = t.Apply(Points[i]);
    return *this;
}

ImRect ImWidgetsPath::GetBounds() const
{
    // Control-point hull: conservative.
    ImRect r(ImVec2(FLT_MAX, FLT_MAX), ImVec2(-FLT_MAX, -FLT_MAX));
    for (int i = 0; i < Points.Size; ++i) r.Add(Points[i]);
    return r;
}

// ---- SVG path data -------------------------------------------------------

static const char* DW_SvgSkipWsp(const char* s)
{
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r' || *s == ',') ++s;
    return s;
}

// Number per the SVG grammar ("1.5e-3", ".5", "-.5", "1-2" is two numbers, "0.5.5" too).
static bool DW_SvgNumber(const char*& s, float& out)
{
    s = DW_SvgSkipWsp(s);
    const char* p = s;
    if (*p == '+' || *p == '-') ++p;
    bool digits = false;
    while (*p >= '0' && *p <= '9') { ++p; digits = true; }
    if (*p == '.') { ++p; while (*p >= '0' && *p <= '9') { ++p; digits = true; } }
    if (!digits) return false;
    if (*p == 'e' || *p == 'E')
    {
        const char* q = p + 1;
        if (*q == '+' || *q == '-') ++q;
        if (*q >= '0' && *q <= '9') { p = q; while (*p >= '0' && *p <= '9') ++p; }
    }
    // Locale-independent conversion (atof would read "1.5" as 1 under a
    // comma-decimal locale).
    const char* q = s;
    double sign = 1.0;
    if (*q == '+' || *q == '-') { if (*q == '-') sign = -1.0; ++q; }
    double v = 0.0;
    while (*q >= '0' && *q <= '9') v = v * 10.0 + (*q++ - '0');
    if (*q == '.')
    {
        ++q;
        double f = 0.1;
        while (*q >= '0' && *q <= '9') { v += (*q++ - '0') * f; f *= 0.1; }
    }
    if (q < p && (*q == 'e' || *q == 'E'))
    {
        ++q;
        int es = 1, e = 0;
        if (*q == '+' || *q == '-') { if (*q == '-') es = -1; ++q; }
        while (*q >= '0' && *q <= '9') { if (e < 1000) e = e * 10 + (*q - '0'); ++q; }
        v *= pow(10.0, (double)(es * e));
    }
    out = (float)(sign * v);
    s = p;
    return true;
}

// Arc flags are single characters and may be packed: "a10 10 0 0110 10".
static bool DW_SvgFlag(const char*& s, bool& out)
{
    s = DW_SvgSkipWsp(s);
    if (*s == '0' || *s == '1') { out = (*s == '1'); ++s; return true; }
    return false;
}

bool ImWidgetsPath::AddSvgPathData(const char* d)
{
    if (!d) return false;
    const char* s = d;
    char cmd = 0;
    ImVec2 lastCtrl(0, 0);  // reflected control point for S / T
    char   lastCmd = 0;
    bool   ok = true;
    while (true)
    {
        s = DW_SvgSkipWsp(s);
        if (!*s) break;
        if ((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z'))
            cmd = *s++;
        else if (cmd == 0) { ok = false; break; }   // data must start with a command
        if (lastCmd == 0 && cmd != 'M' && cmd != 'm') { ok = false; break; } // ... and that command is a moveto

        const bool rel = (cmd >= 'a' && cmd <= 'z');
        const char C = (char)(rel ? cmd - 32 : cmd);
        ImVec2 cur = CurrentPoint();
        auto P = [&](float x, float y) { return rel ? ImVec2(cur.x + x, cur.y + y) : ImVec2(x, y); };
        float a, b, c2, d2, e, f;
        bool  fa, fs;

        switch (C)
        {
        case 'M':
            if (!DW_SvgNumber(s, a) || !DW_SvgNumber(s, b)) { ok = false; break; }
            MoveTo(P(a, b));
            cmd = rel ? 'l' : 'L';     // subsequent pairs are implicit lineTo
            break;
        case 'L':
            if (!DW_SvgNumber(s, a) || !DW_SvgNumber(s, b)) { ok = false; break; }
            LineTo(P(a, b));
            break;
        case 'H':
            if (!DW_SvgNumber(s, a)) { ok = false; break; }
            LineTo(ImVec2(rel ? cur.x + a : a, cur.y));
            break;
        case 'V':
            if (!DW_SvgNumber(s, a)) { ok = false; break; }
            LineTo(ImVec2(cur.x, rel ? cur.y + a : a));
            break;
        case 'C':
            if (!DW_SvgNumber(s, a) || !DW_SvgNumber(s, b) || !DW_SvgNumber(s, c2) ||
                !DW_SvgNumber(s, d2) || !DW_SvgNumber(s, e) || !DW_SvgNumber(s, f)) { ok = false; break; }
            CubicTo(P(a, b), P(c2, d2), P(e, f));
            lastCtrl = P(c2, d2);
            break;
        case 'S':
        {
            if (!DW_SvgNumber(s, c2) || !DW_SvgNumber(s, d2) || !DW_SvgNumber(s, e) || !DW_SvgNumber(s, f)) { ok = false; break; }
            ImVec2 c1 = (lastCmd == 'C' || lastCmd == 'S') ? ImVec2(2 * cur.x - lastCtrl.x, 2 * cur.y - lastCtrl.y) : cur;
            CubicTo(c1, P(c2, d2), P(e, f));
            lastCtrl = P(c2, d2);
            break;
        }
        case 'Q':
            if (!DW_SvgNumber(s, a) || !DW_SvgNumber(s, b) || !DW_SvgNumber(s, e) || !DW_SvgNumber(s, f)) { ok = false; break; }
            QuadTo(P(a, b), P(e, f));
            lastCtrl = P(a, b);
            break;
        case 'T':
        {
            if (!DW_SvgNumber(s, e) || !DW_SvgNumber(s, f)) { ok = false; break; }
            ImVec2 q = (lastCmd == 'Q' || lastCmd == 'T') ? ImVec2(2 * cur.x - lastCtrl.x, 2 * cur.y - lastCtrl.y) : cur;
            QuadTo(q, P(e, f));
            lastCtrl = q;
            break;
        }
        case 'A':
            if (!DW_SvgNumber(s, a) || !DW_SvgNumber(s, b) || !DW_SvgNumber(s, c2) ||
                !DW_SvgFlag(s, fa) || !DW_SvgFlag(s, fs) || !DW_SvgNumber(s, e) || !DW_SvgNumber(s, f)) { ok = false; break; }
            ArcTo(a, b, c2, fa, fs, P(e, f));
            break;
        case 'Z':
        {
            Close();
            // A command after Z without moveto starts at the closed subpath's start.
            if (Subpaths.Size) { ImVec2 st = Points[Subpaths.back().First]; MoveTo(st); }
            // Z takes no arguments: anything other than a command letter (or the
            // end) after it is an error. Treating it as an implicit repeat would
            // loop forever on input like "M0 0 Z 5".
            const char* q = DW_SvgSkipWsp(s);
            if (*q && !((*q >= 'A' && *q <= 'Z') || (*q >= 'a' && *q <= 'z'))) ok = false;
            break;
        }
        default:
            ok = false;
            break;
        }
        if (!ok) break;
        lastCmd = C;
    }
    // Drop a trailing lone moveTo (e.g. after the final Z).
    if (Subpaths.Size && Subpaths.back().Count == 1 && !Subpaths.back().Closed)
    {
        Points.resize(Subpaths.back().First);
        Subpaths.pop_back();
    }
    return ok;
}

// ---- Flattening ------------------------------------------------------------

static void DW_FlattenCubic(ImVector<ImVec2>& out, ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3, float tol)
{
    // Segment count from the second-difference bound (Wang's formula).
    float ddx = ImMax(ImFabs(p0.x - 2 * p1.x + p2.x), ImFabs(p1.x - 2 * p2.x + p3.x));
    float ddy = ImMax(ImFabs(p0.y - 2 * p1.y + p2.y), ImFabs(p1.y - 2 * p2.y + p3.y));
    float dd = ImSqrt(ddx * ddx + ddy * ddy);
    int n = (int)ImCeil(ImSqrt(dd * 0.75f / ImMax(tol, 1e-3f)));
    n = ImClamp(n, 1, 512);
    for (int i = 1; i <= n; ++i)
        out.push_back(ImBezierCubicCalc(p0, p1, p2, p3, (float)i / (float)n));
}

void ImWidgetsPath::Flatten(ImVector<ImVec2>& out_points, ImVector<int>& out_counts, ImVector<bool>* out_closed, float tolerance) const
{
    out_points.resize(0);
    out_counts.resize(0);
    if (out_closed) out_closed->resize(0);
    for (int si = 0; si < Subpaths.Size; ++si)
    {
        const ImWidgetsSubpath& sp = Subpaths[si];
        if (sp.Count < 1) continue;
        int start = out_points.Size;
        out_points.push_back(Points[sp.First]);
        for (int k = sp.First; k + 3 < sp.First + sp.Count; k += 3)
            DW_FlattenCubic(out_points, Points[k], Points[k + 1], Points[k + 2], Points[k + 3], tolerance);
        // A closed subpath ends on its start point; drop the duplicate.
        if (sp.Closed && out_points.Size - start > 1)
        {
            ImVec2 a = out_points[start], b = out_points.back();
            if (ImFabs(a.x - b.x) < 1e-5f && ImFabs(a.y - b.y) < 1e-5f) out_points.pop_back();
        }
        out_counts.push_back(out_points.Size - start);
        if (out_closed) out_closed->push_back(sp.Closed);
    }
}

// ----------------------------------------------------------------------------
// Drawing
// ----------------------------------------------------------------------------

namespace ImWidgets {

void PathFill(ImDrawList* dl, const ImWidgetsPath& path, ImU32 col, ImWidgetsFillRule rule)
{
    if (!dl || path.Subpaths.Size == 0) return;
    // SVG fills every subpath as if closed.
    ImVector<ImVec2> pts;
    ImVector<int> counts;
    path.Flatten(pts, counts, nullptr, 0.25f);
    FillPolygons(dl, pts.Data, counts.Data, counts.Size, col, rule, 1.0f);
}

void PathStroke(ImDrawList* dl, const ImWidgetsPath& path, ImU32 col, const ImWidgetsStrokeStyle& style)
{
    if (!dl || path.Subpaths.Size == 0 || style.Width <= 0.0f) return;
    for (int si = 0; si < path.Subpaths.Size; ++si)
    {
        const ImWidgetsSubpath& sp = path.Subpaths[si];
        if (sp.Count < 4)
        {
            // Zero-length subpath: SVG draws a dot for round / square caps.
            ImVec2 c = path.Points[sp.First];
            float h = style.Width * 0.5f;
            if (style.Cap == ImWidgetsCap_Round)
            {
                ImWidgetsPath dot; dot.Circle(c, h); PathFill(dl, dot, col);
            }
            else if (style.Cap == ImWidgetsCap_Square)
            {
                ImWidgetsPath dot; dot.Rect(ImVec2(c.x - h, c.y - h), ImVec2(c.x + h, c.y + h)); PathFill(dl, dot, col);
            }
            continue;
        }
        const ImVec2* pts = path.Points.Data + sp.First;
        // Closed subpaths already end on their start point, so the stroker adds
        // no extra closing segment -- it only joins the ends.
        const int count = sp.Count;
        if (style.DashCount > 0 && style.Dashes)
            DrawStrokedDashedBezierPath(dl, pts, count, col, style.Width,
                                        style.Dashes, style.DashCount, style.DashOffset,
                                        style.Cap, style.Join, style.MiterLimit, style.Tolerance, sp.Closed);
        else
            DrawStrokedBezierPath(dl, pts, count, col, style.Width,
                                  style.Cap, style.Join, style.MiterLimit, style.Tolerance, sp.Closed);
    }
}

} // namespace ImWidgets
