// dear_widgets_svg.cpp -- SVG documents drawn with plain ImDrawList geometry.
//
// Compiled as its own translation unit (standalone, resolved via the linker).
//
// A compact SVG 1.1 / SVG Tiny subset, enough for icons and illustrations:
//   elements    svg (viewBox, width, height), g, path, rect (rx / ry), circle,
//               ellipse, line, polyline, polygon, defs, linearGradient,
//               radialGradient, stop
//   painting    fill, stroke, fill-rule, opacity, fill-opacity, stroke-opacity,
//               stroke-width / -linecap / -linejoin / -miterlimit /
//               -dasharray / -dashoffset, presentation attributes and
//               style="..." declarations, inheritance through groups,
//               display:none, visibility:hidden
//   colours     #rgb #rgba #rrggbb #rrggbbaa, rgb() / rgba() (numbers or %),
//               CSS named colours, none, transparent, currentColor
//   gradients   url(#id) on fill / stroke, gradientUnits (objectBoundingBox /
//               userSpaceOnUse), gradientTransform, spreadMethod, stop-color /
//               stop-opacity / offset (number or %), href / xlink:href stop
//               inheritance
//   transforms  matrix, translate, scale, rotate (with centre), skewX, skewY
// Not supported: text, images, masks, clip paths, filters, patterns, markers,
// use / symbol, CSS stylesheets (<style>).

#include "dear_widgets.h"
#include "dear_widgets_internal.h"
#include "imgui_internal.h"
#include <math.h>
#include <ctype.h>   // isspace, tolower

namespace ImWidgets {

// ----------------------------------------------------------------------------
// Minimal XML tokenizer
// ----------------------------------------------------------------------------

// Attribute strings live in one pool per tag (no ImVector of ImVectors: its
// copies are deep and inner buffers would leak).
struct DW_SvgAttr { int name, value; };   // offsets into DW_SvgTag::pool

struct DW_SvgTag
{
    ImVector<char>       pool;
    int                  nameOff = 0;
    ImVector<DW_SvgAttr> attrs;
    bool                 closing = false;     // </name>
    bool                 selfClosing = false; // <name ... />

    const char* Name() const { return pool.Data + nameOff; }
    int AddStr(const char* b, const char* e)
    {
        int off = pool.Size;
        for (const char* q = b; q < e; ++q) pool.push_back(*q);
        pool.push_back(0);
        return off;
    }
    const char* Get(const char* n) const
    {
        for (int i = 0; i < attrs.Size; ++i)
            if (strcmp(pool.Data + attrs[i].name, n) == 0) return pool.Data + attrs[i].value;
        return nullptr;
    }
};

// Decodes the few entities that appear in attribute values, in place
// (the result is never longer than the source).
static void DW_SvgDecodeEntities(char* v)
{
    char* w = v;
    for (char* r = v; *r; )
    {
        if (*r == '&')
        {
            struct E { const char* s; char c; } ents[] = { { "&amp;", '&' }, { "&lt;", '<' }, { "&gt;", '>' }, { "&quot;", '"' }, { "&apos;", '\'' } };
            bool done = false;
            for (const E& e : ents)
            {
                size_t l = strlen(e.s);
                if (strncmp(r, e.s, l) == 0) { *w++ = e.c; r += l; done = true; break; }
            }
            if (done) continue;
        }
        *w++ = *r++;
    }
    *w = 0;
}

// Reads the next tag; returns false at the end of input.
static bool DW_SvgNextTag(const char*& s, DW_SvgTag& tag)
{
    for (;;)
    {
        while (*s && *s != '<') ++s;
        if (!*s) return false;
        if (strncmp(s, "<!--", 4) == 0) { const char* e = strstr(s + 4, "-->"); s = e ? e + 3 : s + strlen(s); continue; }
        if (strncmp(s, "<![CDATA[", 9) == 0) { const char* e = strstr(s + 9, "]]>"); s = e ? e + 3 : s + strlen(s); continue; }
        if (s[1] == '?' || s[1] == '!') { const char* e = strchr(s, '>'); s = e ? e + 1 : s + strlen(s); continue; }
        break;
    }
    ++s;
    tag.pool.resize(0);
    tag.attrs.resize(0);
    tag.closing = (*s == '/');
    if (tag.closing) ++s;
    const char* nb = s;
    while (*s && !isspace((unsigned char)*s) && *s != '>' && *s != '/') ++s;
    // drop a namespace prefix ("svg:rect")
    const char* colon = nb;
    for (const char* q = nb; q < s; ++q) if (*q == ':') colon = q + 1;
    tag.nameOff = tag.AddStr(colon, s);
    tag.selfClosing = false;
    for (;;)
    {
        while (*s && isspace((unsigned char)*s)) ++s;
        if (!*s) return true;
        if (*s == '>') { ++s; return true; }
        if (*s == '/' && s[1] == '>') { tag.selfClosing = true; s += 2; return true; }
        const char* an = s;
        while (*s && *s != '=' && !isspace((unsigned char)*s) && *s != '>' && *s != '/') ++s;
        if (s == an) { ++s; continue; } // stray character: skip it
        const char* ae = s;
        while (*s && isspace((unsigned char)*s)) ++s;
        DW_SvgAttr a;
        a.name = tag.AddStr(an, ae);
        if (*s == '=')
        {
            ++s;
            while (*s && isspace((unsigned char)*s)) ++s;
            char q = *s;
            const char* vb; const char* ve;
            if (q == '"' || q == '\'') { vb = ++s; while (*s && *s != q) ++s; ve = s; if (*s) ++s; }
            else { vb = s; while (*s && !isspace((unsigned char)*s) && *s != '>') ++s; ve = s; }
            a.value = tag.AddStr(vb, ve);
            DW_SvgDecodeEntities(tag.pool.Data + a.value);
        }
        else a.value = tag.AddStr(s, s);
        tag.attrs.push_back(a);
    }
}

// ----------------------------------------------------------------------------
// Value parsing
// ----------------------------------------------------------------------------

static const char* DW_SvgWs(const char* s) { while (*s && (isspace((unsigned char)*s) || *s == ',')) ++s; return s; }

static bool DW_SvgNum(const char*& s, float& out)
{
    s = DW_SvgWs(s);
    char* e = nullptr;
    // strtod is locale dependent for the decimal point; SVG numbers use '.'.
    // Parse manually.
    const char* p = s;
    double sign = 1.0, v = 0.0;
    if (*p == '+' || *p == '-') { if (*p == '-') sign = -1.0; ++p; }
    bool dig = false;
    while (*p >= '0' && *p <= '9') { v = v * 10.0 + (*p++ - '0'); dig = true; }
    if (*p == '.') { ++p; double f = 0.1; while (*p >= '0' && *p <= '9') { v += (*p++ - '0') * f; f *= 0.1; dig = true; } }
    if (!dig) return false;
    if (*p == 'e' || *p == 'E')
    {
        const char* q = p + 1; int es = 1, ex = 0;
        if (*q == '+' || *q == '-') { if (*q == '-') es = -1; ++q; }
        if (*q >= '0' && *q <= '9') { while (*q >= '0' && *q <= '9') { if (ex < 300) ex = ex * 10 + (*q - '0'); ++q; } v *= pow(10.0, es * ex); p = q; }
    }
    (void)e;
    if (v > 1e30) v = 1e30;  // keep geometry finite
    out = (float)(sign * v);
    s = p;
    return true;
}

// Length with optional unit; percentages resolve against `pct_ref`.
static float DW_SvgLength(const char* s, float def, float pct_ref = 0.0f)
{
    if (!s) return def;
    float v;
    if (!DW_SvgNum(s, v)) return def;
    if (*s == '%') return v * 0.01f * pct_ref;
    if (strncmp(s, "px", 2) == 0) return v;
    if (strncmp(s, "pt", 2) == 0) return v * 4.0f / 3.0f;
    if (strncmp(s, "pc", 2) == 0) return v * 16.0f;
    if (strncmp(s, "mm", 2) == 0) return v * 96.0f / 25.4f;
    if (strncmp(s, "cm", 2) == 0) return v * 96.0f / 2.54f;
    if (strncmp(s, "in", 2) == 0) return v * 96.0f;
    if (strncmp(s, "em", 2) == 0) return v * 16.0f;
    return v;
}

struct DW_SvgNamedColor { const char* name; ImU32 rgb; };
static const DW_SvgNamedColor kSvgColors[] = {
    { "aliceblue", 0xF0F8FF }, { "antiquewhite", 0xFAEBD7 }, { "aqua", 0x00FFFF }, { "aquamarine", 0x7FFFD4 },
    { "azure", 0xF0FFFF }, { "beige", 0xF5F5DC }, { "bisque", 0xFFE4C4 }, { "black", 0x000000 },
    { "blanchedalmond", 0xFFEBCD }, { "blue", 0x0000FF }, { "blueviolet", 0x8A2BE2 }, { "brown", 0xA52A2A },
    { "burlywood", 0xDEB887 }, { "cadetblue", 0x5F9EA0 }, { "chartreuse", 0x7FFF00 }, { "chocolate", 0xD2691E },
    { "coral", 0xFF7F50 }, { "cornflowerblue", 0x6495ED }, { "cornsilk", 0xFFF8DC }, { "crimson", 0xDC143C },
    { "cyan", 0x00FFFF }, { "darkblue", 0x00008B }, { "darkcyan", 0x008B8B }, { "darkgoldenrod", 0xB8860B },
    { "darkgray", 0xA9A9A9 }, { "darkgreen", 0x006400 }, { "darkgrey", 0xA9A9A9 }, { "darkkhaki", 0xBDB76B },
    { "darkmagenta", 0x8B008B }, { "darkolivegreen", 0x556B2F }, { "darkorange", 0xFF8C00 }, { "darkorchid", 0x9932CC },
    { "darkred", 0x8B0000 }, { "darksalmon", 0xE9967A }, { "darkseagreen", 0x8FBC8F }, { "darkslateblue", 0x483D8B },
    { "darkslategray", 0x2F4F4F }, { "darkslategrey", 0x2F4F4F }, { "darkturquoise", 0x00CED1 }, { "darkviolet", 0x9400D3 },
    { "deeppink", 0xFF1493 }, { "deepskyblue", 0x00BFFF }, { "dimgray", 0x696969 }, { "dimgrey", 0x696969 },
    { "dodgerblue", 0x1E90FF }, { "firebrick", 0xB22222 }, { "floralwhite", 0xFFFAF0 }, { "forestgreen", 0x228B22 },
    { "fuchsia", 0xFF00FF }, { "gainsboro", 0xDCDCDC }, { "ghostwhite", 0xF8F8FF }, { "gold", 0xFFD700 },
    { "goldenrod", 0xDAA520 }, { "gray", 0x808080 }, { "grey", 0x808080 }, { "green", 0x008000 },
    { "greenyellow", 0xADFF2F }, { "honeydew", 0xF0FFF0 }, { "hotpink", 0xFF69B4 }, { "indianred", 0xCD5C5C },
    { "indigo", 0x4B0082 }, { "ivory", 0xFFFFF0 }, { "khaki", 0xF0E68C }, { "lavender", 0xE6E6FA },
    { "lavenderblush", 0xFFF0F5 }, { "lawngreen", 0x7CFC00 }, { "lemonchiffon", 0xFFFACD }, { "lightblue", 0xADD8E6 },
    { "lightcoral", 0xF08080 }, { "lightcyan", 0xE0FFFF }, { "lightgoldenrodyellow", 0xFAFAD2 }, { "lightgray", 0xD3D3D3 },
    { "lightgreen", 0x90EE90 }, { "lightgrey", 0xD3D3D3 }, { "lightpink", 0xFFB6C1 }, { "lightsalmon", 0xFFA07A },
    { "lightseagreen", 0x20B2AA }, { "lightskyblue", 0x87CEFA }, { "lightslategray", 0x778899 }, { "lightslategrey", 0x778899 },
    { "lightsteelblue", 0xB0C4DE }, { "lightyellow", 0xFFFFE0 }, { "lime", 0x00FF00 }, { "limegreen", 0x32CD32 },
    { "linen", 0xFAF0E6 }, { "magenta", 0xFF00FF }, { "maroon", 0x800000 }, { "mediumaquamarine", 0x66CDAA },
    { "mediumblue", 0x0000CD }, { "mediumorchid", 0xBA55D3 }, { "mediumpurple", 0x9370DB }, { "mediumseagreen", 0x3CB371 },
    { "mediumslateblue", 0x7B68EE }, { "mediumspringgreen", 0x00FA9A }, { "mediumturquoise", 0x48D1CC }, { "mediumvioletred", 0xC71585 },
    { "midnightblue", 0x191970 }, { "mintcream", 0xF5FFFA }, { "mistyrose", 0xFFE4E1 }, { "moccasin", 0xFFE4B5 },
    { "navajowhite", 0xFFDEAD }, { "navy", 0x000080 }, { "oldlace", 0xFDF5E6 }, { "olive", 0x808000 },
    { "olivedrab", 0x6B8E23 }, { "orange", 0xFFA500 }, { "orangered", 0xFF4500 }, { "orchid", 0xDA70D6 },
    { "palegoldenrod", 0xEEE8AA }, { "palegreen", 0x98FB98 }, { "paleturquoise", 0xAFEEEE }, { "palevioletred", 0xDB7093 },
    { "papayawhip", 0xFFEFD5 }, { "peachpuff", 0xFFDAB9 }, { "peru", 0xCD853F }, { "pink", 0xFFC0CB },
    { "plum", 0xDDA0DD }, { "powderblue", 0xB0E0E6 }, { "purple", 0x800080 }, { "rebeccapurple", 0x663399 },
    { "red", 0xFF0000 }, { "rosybrown", 0xBC8F8F }, { "royalblue", 0x4169E1 }, { "saddlebrown", 0x8B4513 },
    { "salmon", 0xFA8072 }, { "sandybrown", 0xF4A460 }, { "seagreen", 0x2E8B57 }, { "seashell", 0xFFF5EE },
    { "sienna", 0xA0522D }, { "silver", 0xC0C0C0 }, { "skyblue", 0x87CEEB }, { "slateblue", 0x6A5ACD },
    { "slategray", 0x708090 }, { "slategrey", 0x708090 }, { "snow", 0xFFFAFA }, { "springgreen", 0x00FF7F },
    { "steelblue", 0x4682B4 }, { "tan", 0xD2B48C }, { "teal", 0x008080 }, { "thistle", 0xD8BFD8 },
    { "tomato", 0xFF6347 }, { "turquoise", 0x40E0D0 }, { "violet", 0xEE82EE }, { "wheat", 0xF5DEB3 },
    { "white", 0xFFFFFF }, { "whitesmoke", 0xF5F5F5 }, { "yellow", 0xFFFF00 }, { "yellowgreen", 0x9ACD32 },
};

enum DW_SvgPaintKind { DW_SvgPaint_None, DW_SvgPaint_Color, DW_SvgPaint_Current, DW_SvgPaint_Url, DW_SvgPaint_Unset };

struct DW_SvgPaintRef
{
    int      kind = DW_SvgPaint_Unset;
    ImU32    color = IM_COL32_BLACK; // straight alpha
    bool     hasFallback = false;    // url(#id) <color>
    char     url[64] = {};
};

static int DW_SvgHex(char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1; }

static bool DW_SvgParseColor(const char* s, ImU32& out)
{
    s = DW_SvgWs(s);
    if (*s == '#')
    {
        ++s;
        int n = 0; int d[8];
        while (n < 8 && DW_SvgHex(s[n]) >= 0) { d[n] = DW_SvgHex(s[n]); ++n; }
        int r, g, b, a = 255;
        if (n == 3 || n == 4) { r = d[0] * 17; g = d[1] * 17; b = d[2] * 17; if (n == 4) a = d[3] * 17; }
        else if (n == 6 || n == 8) { r = d[0] * 16 + d[1]; g = d[2] * 16 + d[3]; b = d[4] * 16 + d[5]; if (n == 8) a = d[6] * 16 + d[7]; }
        else return false;
        out = IM_COL32(r, g, b, a);
        return true;
    }
    if (strncmp(s, "rgb", 3) == 0)
    {
        const char* p = strchr(s, '(');
        if (!p) return false;
        ++p;
        float c[4] = { 0, 0, 0, 1 };
        for (int i = 0; i < 4; ++i)
        {
            p = DW_SvgWs(p);
            if (*p == '/') p = DW_SvgWs(p + 1);
            if (*p == ')') break;
            if (!DW_SvgNum(p, c[i])) break;
            if (*p == '%') { c[i] *= (i < 3) ? 2.55f : 0.01f; ++p; }
        }
        out = IM_COL32((int)ImClamp(c[0], 0.0f, 255.0f), (int)ImClamp(c[1], 0.0f, 255.0f), (int)ImClamp(c[2], 0.0f, 255.0f),
                       (int)(ImSaturate(c[3]) * 255.0f + 0.5f));
        return true;
    }
    char name[32]; int n = 0;
    while (n < 31 && ((s[n] >= 'a' && s[n] <= 'z') || (s[n] >= 'A' && s[n] <= 'Z'))) { name[n] = (char)tolower((unsigned char)s[n]); ++n; }
    name[n] = 0;
    if (strcmp(name, "transparent") == 0) { out = 0; return true; }
    for (const DW_SvgNamedColor& c : kSvgColors)
        if (strcmp(c.name, name) == 0)
        {
            out = IM_COL32((c.rgb >> 16) & 255, (c.rgb >> 8) & 255, c.rgb & 255, 255);
            return true;
        }
    return false;
}

static void DW_SvgParsePaint(const char* s, DW_SvgPaintRef& p)
{
    s = DW_SvgWs(s);
    if (strncmp(s, "none", 4) == 0) { p.kind = DW_SvgPaint_None; return; }
    if (ImStrnicmp(s, "currentColor", 12) == 0) { p.kind = DW_SvgPaint_Current; return; }
    if (strncmp(s, "inherit", 7) == 0) return; // the style already holds the parent's value
    if (strncmp(s, "url(", 4) == 0)
    {
        const char* b = s + 4;
        while (*b == ' ' || *b == '\'' || *b == '"') ++b;
        if (*b == '#') ++b;
        int n = 0;
        while (b[n] && b[n] != ')' && b[n] != '\'' && b[n] != '"' && n < 63) { p.url[n] = b[n]; ++n; }
        p.url[n] = 0;
        p.kind = DW_SvgPaint_Url;
        // fallback colour after the url: "url(#g) red"
        const char* after = strchr(s, ')');
        ImU32 fb;
        p.hasFallback = false;
        if (after && DW_SvgParseColor(after + 1, fb)) { p.color = fb; p.hasFallback = true; }
        return;
    }
    ImU32 c;
    if (DW_SvgParseColor(s, c)) { p.kind = DW_SvgPaint_Color; p.color = c; }
}

static ImWidgetsTransform2D DW_SvgParseTransform(const char* s)
{
    ImWidgetsTransform2D m;
    if (!s) return m;
    while (*s)
    {
        s = DW_SvgWs(s);
        const char* nb = s;
        while (*s && *s != '(') ++s;
        if (!*s) break;
        int nlen = (int)(s - nb);
        while (nlen > 0 && isspace((unsigned char)nb[nlen - 1])) --nlen;
        ++s;
        float a[6] = {}; int n = 0;
        while (n < 6) { const char* q = s; if (!DW_SvgNum(q, a[n])) break; s = q; ++n; }
        while (*s && *s != ')') ++s;
        if (*s) ++s;
        ImWidgetsTransform2D t;
        auto is = [&](const char* k) { return (int)strlen(k) == nlen && strncmp(nb, k, (size_t)nlen) == 0; };
        if (is("matrix") && n == 6) t = ImWidgetsTransform2D(a[0], a[1], a[2], a[3], a[4], a[5]);
        else if (is("translate")) t = ImWidgetsTransform2D::Translate(a[0], n > 1 ? a[1] : 0.0f);
        else if (is("scale")) t = ImWidgetsTransform2D::Scale(a[0], n > 1 ? a[1] : a[0]);
        else if (is("rotate"))
        {
            float r = a[0] * IM_PI / 180.0f;
            t = ImWidgetsTransform2D::Rotate(r);
            if (n >= 3) t = ImWidgetsTransform2D::Translate(a[1], a[2]) * t * ImWidgetsTransform2D::Translate(-a[1], -a[2]);
        }
        else if (is("skewX")) t = ImWidgetsTransform2D(1, 0, tanf(a[0] * IM_PI / 180.0f), 1, 0, 0);
        else if (is("skewY")) t = ImWidgetsTransform2D(1, tanf(a[0] * IM_PI / 180.0f), 0, 1, 0, 0);
        m = m * t; // the list applies right to left
    }
    return m;
}

// ----------------------------------------------------------------------------
// Document model
// ----------------------------------------------------------------------------

struct DW_SvgGradient
{
    char  id[64];
    int   type;                 // ImWidgetsPaintType_LinearGradient / _RadialGradient
    bool  userSpace;            // gradientUnits="userSpaceOnUse"
    bool  hasX1, hasY1, hasX2, hasY2, hasCx, hasCy, hasR, hasFx, hasFy, hasFr, hasUnits, hasSpread, hasTransform;
    float x1, y1, x2, y2, cx, cy, r, fx, fy, fr;     // raw numbers (see *Pct)
    bool  x1Pct, y1Pct, x2Pct, y2Pct, cxPct, cyPct, rPct, fxPct, fyPct, frPct;
    int   spread;
    ImWidgetsTransform2D transform;
    char  href[64];
    ImVector<ImWidgetsGradientStop> stops;
};

struct DW_SvgStyle
{
    DW_SvgPaintRef fill, stroke;
    float  fillOpacity = 1.0f, strokeOpacity = 1.0f, opacity = 1.0f; // opacity: ancestors x own
    float  ownOpacity = 1.0f;
    float  strokeWidth = 1.0f, miterLimit = 4.0f, dashOffset = 0.0f;
    int    fillRule = ImWidgetsFillRule_NonZero;
    int    cap = ImWidgetsCap_Butt, join = ImWidgetsJoin_Miter;
    float  dashes[16]; int dashCount = 0;
    bool   visible = true, display = true;
};

struct DW_SvgItem
{
    ImWidgetsPath        path;        // in document user space (element transforms applied)
    DW_SvgStyle          style;
    float                strokeScale; // scale of the element's transform (stroke width is in its local units)
    ImRect               bbox;        // object bounding box in the element's local space (objectBoundingBox gradients)
    ImWidgetsTransform2D xf;          // element user space -> document space (gradients live in user space)
};

} // namespace ImWidgets

struct ImWidgetsSvg
{
    ImVector<ImWidgets::DW_SvgItem*>      items;
    ImVector<ImWidgets::DW_SvgGradient*>  gradients;
    ImRect                                viewBox;
    ImVec2                                size;   // intrinsic size (width / height, else viewBox)
    ~ImWidgetsSvg()
    {
        for (int i = 0; i < items.Size; ++i) IM_DELETE(items[i]);
        for (int i = 0; i < gradients.Size; ++i) IM_DELETE(gradients[i]);
    }
};

namespace ImWidgets {

// Applies one property (presentation attribute or style declaration).
static void DW_SvgApplyProp(DW_SvgStyle& st, const char* name, const char* v)
{
    if (!v) return;
    if (!strcmp(name, "fill")) DW_SvgParsePaint(v, st.fill);
    else if (!strcmp(name, "stroke")) DW_SvgParsePaint(v, st.stroke);
    else if (!strcmp(name, "fill-opacity")) { float f; const char* q = v; if (DW_SvgNum(q, f)) st.fillOpacity = ImSaturate(*q == '%' ? f * 0.01f : f); }
    else if (!strcmp(name, "stroke-opacity")) { float f; const char* q = v; if (DW_SvgNum(q, f)) st.strokeOpacity = ImSaturate(*q == '%' ? f * 0.01f : f); }
    else if (!strcmp(name, "opacity")) { float f; const char* q = v; if (DW_SvgNum(q, f)) st.ownOpacity = ImSaturate(*q == '%' ? f * 0.01f : f); }
    else if (!strcmp(name, "stroke-width")) st.strokeWidth = DW_SvgLength(v, st.strokeWidth);
    else if (!strcmp(name, "stroke-miterlimit")) { float f; const char* q = v; if (DW_SvgNum(q, f)) st.miterLimit = ImMax(1.0f, f); }
    else if (!strcmp(name, "stroke-dashoffset")) st.dashOffset = DW_SvgLength(v, 0.0f);
    else if (!strcmp(name, "fill-rule")) st.fillRule = strstr(v, "evenodd") ? ImWidgetsFillRule_EvenOdd : ImWidgetsFillRule_NonZero;
    else if (!strcmp(name, "stroke-linecap")) st.cap = strstr(v, "round") ? ImWidgetsCap_Round : strstr(v, "square") ? ImWidgetsCap_Square : ImWidgetsCap_Butt;
    else if (!strcmp(name, "stroke-linejoin")) st.join = strstr(v, "round") ? ImWidgetsJoin_Round : strstr(v, "bevel") ? ImWidgetsJoin_Bevel : ImWidgetsJoin_Miter;
    else if (!strcmp(name, "stroke-dasharray"))
    {
        st.dashCount = 0;
        if (strstr(v, "none")) return;
        const char* q = v; float f; bool allZero = true;
        while (st.dashCount < 16 && DW_SvgNum(q, f)) { if (f < 0) { st.dashCount = 0; return; } st.dashes[st.dashCount++] = f; if (f > 0) allZero = false; if (*q == '%') ++q; while (*q && *q != ',' && !isspace((unsigned char)*q) && !(*q >= '0' && *q <= '9') && *q != '.' && *q != '-') ++q; }
        if (allZero) st.dashCount = 0; // "0" is solid per SVG
    }
    else if (!strcmp(name, "display")) { if (strstr(v, "none")) st.display = false; } // never re-shown below a hidden ancestor
    else if (!strcmp(name, "visibility")) st.visible = !(strstr(v, "hidden") || strstr(v, "collapse"));
}

// Inheritable presentation attributes of an element (attributes first, style= wins).
static void DW_SvgApplyStyle(DW_SvgStyle& st, const DW_SvgTag& tag)
{
    static const char* kProps[] = { "fill", "stroke", "fill-opacity", "stroke-opacity", "opacity", "stroke-width",
        "stroke-miterlimit", "stroke-dashoffset", "fill-rule", "stroke-linecap", "stroke-linejoin", "stroke-dasharray",
        "display", "visibility" };
    st.ownOpacity = 1.0f;
    for (const char* k : kProps) DW_SvgApplyProp(st, k, tag.Get(k));
    if (const char* css = tag.Get("style"))
    {
        const char* p = css;
        while (*p)
        {
            while (*p == ' ' || *p == ';' || *p == '\t' || *p == '\n') ++p;
            const char* nb = p;
            while (*p && *p != ':' && *p != ';') ++p;
            if (*p != ':') { while (*p && *p != ';') ++p; continue; } // malformed declaration: skip it
            const char* ne = p; while (ne > nb && isspace((unsigned char)ne[-1])) --ne;
            ++p;
            while (*p == ' ') ++p;
            const char* vb = p;
            while (*p && *p != ';') ++p;
            const char* ve = p; while (ve > vb && isspace((unsigned char)ve[-1])) --ve;
            char name[48], val[256];
            int nl = ImMin((int)(ne - nb), 47), vl = ImMin((int)(ve - vb), 255);
            memcpy(name, nb, (size_t)nl); name[nl] = 0;
            memcpy(val, vb, (size_t)vl); val[vl] = 0;
            // strip "!important"
            if (char* imp = strstr(val, "!important")) *imp = 0;
            DW_SvgApplyProp(st, name, val);
        }
    }
}

static float DW_SvgAttrF(const DW_SvgTag& t, const char* n, float def, float pct_ref = 0.0f) { return DW_SvgLength(t.Get(n), def, pct_ref); }

// Offset of a stop: number or percentage, clamped and monotonic (SVG rule).
static float DW_SvgStopOffset(const char* s)
{
    if (!s) return 0.0f;
    float v; const char* q = s;
    if (!DW_SvgNum(q, v)) return 0.0f;
    if (*q == '%') v *= 0.01f;
    return ImSaturate(v);
}

} // namespace ImWidgets

using namespace ImWidgets;

ImWidgetsSvg* ImWidgets::SvgLoad(const char* text)
{
    if (!text) return nullptr;
    ImWidgetsSvg* doc = IM_NEW(ImWidgetsSvg)();
    doc->viewBox = ImRect(0, 0, 0, 0);
    doc->size = ImVec2(0, 0);

    struct Frame { DW_SvgStyle style; ImWidgetsTransform2D xf; ImVector<char> name; };
    ImVector<Frame*> stack;
    Frame* root = IM_NEW(Frame)();
    root->style.fill.kind = DW_SvgPaint_Color; root->style.fill.color = IM_COL32_BLACK; // SVG initial fill
    root->style.stroke.kind = DW_SvgPaint_None;
    stack.push_back(root);

    DW_SvgGradient* curGrad = nullptr;
    int defsDepth = 0;
    const char* s = text;
    DW_SvgTag tag;
    bool sawSvg = false;
    while (DW_SvgNextTag(s, tag))
    {
        const char* name = tag.Name();
        if (tag.closing)
        {
            if (!strcmp(name, "linearGradient") || !strcmp(name, "radialGradient")) { curGrad = nullptr; continue; }
            if (!strcmp(name, "defs") || !strcmp(name, "symbol") || !strcmp(name, "clipPath") || !strcmp(name, "mask")
                || !strcmp(name, "pattern") || !strcmp(name, "marker")) { if (defsDepth > 0) --defsDepth; continue; }
            if ((!strcmp(name, "g") || !strcmp(name, "svg") || !strcmp(name, "a") || !strcmp(name, "switch")) && stack.Size > 1)
            {
                IM_DELETE(stack.back()); stack.pop_back();
            }
            continue;
        }

        // Gradients (may appear anywhere, usually in <defs>).
        if (!strcmp(name, "linearGradient") || !strcmp(name, "radialGradient"))
        {
            DW_SvgGradient* gdef = IM_NEW(DW_SvgGradient)();
            memset(gdef->id, 0, sizeof(gdef->id)); memset(gdef->href, 0, sizeof(gdef->href));
            gdef->type = !strcmp(name, "linearGradient") ? ImWidgetsPaintType_LinearGradient : ImWidgetsPaintType_RadialGradient;
            const char* id = tag.Get("id"); if (id) ImStrncpy(gdef->id, id, sizeof(gdef->id));
            const char* href = tag.Get("href"); if (!href) href = tag.Get("xlink:href");
            if (href) ImStrncpy(gdef->href, href[0] == '#' ? href + 1 : href, sizeof(gdef->href));
            const char* units = tag.Get("gradientUnits");
            gdef->hasUnits = units != nullptr; gdef->userSpace = units && strstr(units, "userSpaceOnUse");
            const char* spread = tag.Get("spreadMethod");
            gdef->hasSpread = spread != nullptr;
            gdef->spread = spread ? (strstr(spread, "reflect") ? ImWidgetsSpread_Reflect : strstr(spread, "repeat") ? ImWidgetsSpread_Repeat : ImWidgetsSpread_Pad) : ImWidgetsSpread_Pad;
            const char* gt = tag.Get("gradientTransform");
            gdef->hasTransform = gt != nullptr;
            gdef->transform = DW_SvgParseTransform(gt);
            // Keep the raw number and whether it was a percentage: the meaning
            // depends on gradientUnits, known only after href resolution.
            // Defaults are the spec's percentages (x2 = 100%, cx = cy = r = 50%).
            auto num = [&](const char* n, bool& has, float& out, bool& pct, float defPct) {
                const char* v = tag.Get(n); has = v != nullptr;
                out = defPct; pct = true;
                if (!v) return;
                float f; const char* q = v;
                if (!DW_SvgNum(q, f)) { has = false; return; }
                out = f; pct = (*q == '%');
            };
            num("x1", gdef->hasX1, gdef->x1, gdef->x1Pct, 0.0f);   num("y1", gdef->hasY1, gdef->y1, gdef->y1Pct, 0.0f);
            num("x2", gdef->hasX2, gdef->x2, gdef->x2Pct, 100.0f); num("y2", gdef->hasY2, gdef->y2, gdef->y2Pct, 0.0f);
            num("cx", gdef->hasCx, gdef->cx, gdef->cxPct, 50.0f);  num("cy", gdef->hasCy, gdef->cy, gdef->cyPct, 50.0f);
            num("r",  gdef->hasR,  gdef->r,  gdef->rPct,  50.0f);
            num("fx", gdef->hasFx, gdef->fx, gdef->fxPct, 50.0f);  num("fy", gdef->hasFy, gdef->fy, gdef->fyPct, 50.0f);
            num("fr", gdef->hasFr, gdef->fr, gdef->frPct, 0.0f);
            doc->gradients.push_back(gdef);
            curGrad = tag.selfClosing ? nullptr : gdef;
            continue;
        }
        if (!strcmp(name, "stop"))
        {
            if (curGrad)
            {
                DW_SvgStyle tmp;
                ImU32 col = IM_COL32_BLACK;
                float op = 1.0f;
                const char* sc = tag.Get("stop-color");
                const char* so = tag.Get("stop-opacity");
                // style="stop-color:..;stop-opacity:.."
                char scBuf[64] = {}, soBuf[32] = {};
                if (const char* css = tag.Get("style"))
                {
                    if (const char* p = strstr(css, "stop-color")) { p = strchr(p, ':'); if (p) { ++p; int n = 0; while (p[n] && p[n] != ';' && n < 63) { scBuf[n] = p[n]; ++n; } sc = scBuf; } }
                    if (const char* p = strstr(css, "stop-opacity")) { p = strchr(p, ':'); if (p) { ++p; int n = 0; while (p[n] && p[n] != ';' && n < 31) { soBuf[n] = p[n]; ++n; } so = soBuf; } }
                }
                if (sc) DW_SvgParseColor(sc, col);
                if (so) { float f; const char* q = so; if (DW_SvgNum(q, f)) op = ImSaturate(*q == '%' ? f * 0.01f : f); }
                float a = ((col >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f * op;
                col = (col & ~IM_COL32_A_MASK) | ((ImU32)(a * 255.0f + 0.5f) << IM_COL32_A_SHIFT);
                ImWidgetsGradientStop gs;
                gs.Offset = DW_SvgStopOffset(tag.Get("offset"));
                if (curGrad->stops.Size && gs.Offset < curGrad->stops.back().Offset) gs.Offset = curGrad->stops.back().Offset;
                gs.Color = col;
                curGrad->stops.push_back(gs);
            }
            continue;
        }
        // Not rendered directly: defs and the referenced-only containers
        // (clip paths, masks, symbols, patterns, markers are unsupported, so
        // their content must not show as plain shapes).
        if (!strcmp(name, "defs") || !strcmp(name, "symbol") || !strcmp(name, "clipPath") || !strcmp(name, "mask")
            || !strcmp(name, "pattern") || !strcmp(name, "marker")) { if (!tag.selfClosing) ++defsDepth; continue; }
        if (defsDepth > 0)
        {
            // Keep the group stack balanced: a group opened in a skipped region
            // still gets popped by its closing tag.
            if ((!strcmp(name, "g") || !strcmp(name, "svg") || !strcmp(name, "a") || !strcmp(name, "switch")) && !tag.selfClosing)
            {
                Frame* f = IM_NEW(Frame)(); f->style = stack.back()->style; f->xf = stack.back()->xf; stack.push_back(f);
            }
            continue;
        }

        // Inherited state for this element.
        Frame* parent = stack.back();
        DW_SvgStyle st = parent->style;
        DW_SvgApplyStyle(st, tag);
        st.opacity = parent->style.opacity * st.ownOpacity;
        ImWidgetsTransform2D xf = parent->xf * DW_SvgParseTransform(tag.Get("transform"));

        if (!strcmp(name, "svg"))
        {
            if (!sawSvg)
            {
                sawSvg = true;
                float w = DW_SvgAttrF(tag, "width", 0.0f), h = DW_SvgAttrF(tag, "height", 0.0f);
                if (const char* vb = tag.Get("viewBox"))
                {
                    float v[4]; const char* q = vb; int n = 0;
                    while (n < 4 && DW_SvgNum(q, v[n])) ++n;
                    if (n == 4 && v[2] > 0 && v[3] > 0) doc->viewBox = ImRect(v[0], v[1], v[0] + v[2], v[1] + v[3]);
                }
                if (doc->viewBox.GetWidth() <= 0.0f) doc->viewBox = ImRect(0, 0, w > 0 ? w : 100.0f, h > 0 ? h : 100.0f);
                doc->size = ImVec2(w > 0 ? w : doc->viewBox.GetWidth(), h > 0 ? h : doc->viewBox.GetHeight());
            }
            if (!tag.selfClosing) { Frame* f = IM_NEW(Frame)(); f->style = st; f->xf = xf; stack.push_back(f); }
            continue;
        }
        if (!strcmp(name, "g") || !strcmp(name, "a") || !strcmp(name, "switch"))
        {
            if (!tag.selfClosing) { Frame* f = IM_NEW(Frame)(); f->style = st; f->xf = xf; stack.push_back(f); }
            continue;
        }

        // Shapes -> path in local units.
        ImWidgetsPath p;
        const float vbw = doc->viewBox.GetWidth(), vbh = doc->viewBox.GetHeight();
        const float vbd = ImSqrt((vbw * vbw + vbh * vbh) * 0.5f);
        bool shape = true;
        if (!strcmp(name, "path"))
        {
            p.AddSvgPathData(tag.Get("d")); // keeps the prefix on errors, like browsers
        }
        else if (!strcmp(name, "rect"))
        {
            float x = DW_SvgAttrF(tag, "x", 0, vbw), y = DW_SvgAttrF(tag, "y", 0, vbh);
            float w = DW_SvgAttrF(tag, "width", 0, vbw), h = DW_SvgAttrF(tag, "height", 0, vbh);
            const char* rxs = tag.Get("rx"); const char* rys = tag.Get("ry");
            float rx = DW_SvgLength(rxs, -1.0f, vbw), ry = DW_SvgLength(rys, -1.0f, vbh);
            if (rx < 0 && ry >= 0) rx = ry;
            if (ry < 0 && rx >= 0) ry = rx;
            rx = ImClamp(rx, 0.0f, w * 0.5f); ry = ImClamp(ry, 0.0f, h * 0.5f);
            if (w > 0 && h > 0)
            {
                if (rx <= 0 || ry <= 0) p.Rect(ImVec2(x, y), ImVec2(x + w, y + h));
                else
                {
                    p.MoveTo(ImVec2(x + rx, y)).LineTo(ImVec2(x + w - rx, y)).ArcTo(rx, ry, 0, false, true, ImVec2(x + w, y + ry))
                     .LineTo(ImVec2(x + w, y + h - ry)).ArcTo(rx, ry, 0, false, true, ImVec2(x + w - rx, y + h))
                     .LineTo(ImVec2(x + rx, y + h)).ArcTo(rx, ry, 0, false, true, ImVec2(x, y + h - ry))
                     .LineTo(ImVec2(x, y + ry)).ArcTo(rx, ry, 0, false, true, ImVec2(x + rx, y)).Close();
                }
            }
        }
        else if (!strcmp(name, "circle"))
        {
            float r = DW_SvgAttrF(tag, "r", 0, vbd);
            if (r > 0) p.Circle(ImVec2(DW_SvgAttrF(tag, "cx", 0, vbw), DW_SvgAttrF(tag, "cy", 0, vbh)), r);
        }
        else if (!strcmp(name, "ellipse"))
        {
            float rx = DW_SvgAttrF(tag, "rx", 0, vbw), ry = DW_SvgAttrF(tag, "ry", 0, vbh);
            if (rx > 0 && ry > 0) p.Ellipse(ImVec2(DW_SvgAttrF(tag, "cx", 0, vbw), DW_SvgAttrF(tag, "cy", 0, vbh)), ImVec2(rx, ry));
        }
        else if (!strcmp(name, "line"))
        {
            p.MoveTo(ImVec2(DW_SvgAttrF(tag, "x1", 0, vbw), DW_SvgAttrF(tag, "y1", 0, vbh)))
             .LineTo(ImVec2(DW_SvgAttrF(tag, "x2", 0, vbw), DW_SvgAttrF(tag, "y2", 0, vbh)));
        }
        else if (!strcmp(name, "polyline") || !strcmp(name, "polygon"))
        {
            const char* q = tag.Get("points");
            float x, y; bool first = true;
            while (q && DW_SvgNum(q, x) && DW_SvgNum(q, y)) { if (first) p.MoveTo(ImVec2(x, y)); else p.LineTo(ImVec2(x, y)); first = false; }
            if (!strcmp(name, "polygon") && !first) p.Close();
        }
        else shape = false;

        if (!shape || p.Subpaths.Size == 0 || !st.display || !st.visible) continue;
        DW_SvgItem* item = IM_NEW(DW_SvgItem)();
        // Local bounding box from the flattened outline (control points overshoot curves).
        {
            ImVector<ImVec2> fp; ImVector<int> fc;
            p.Flatten(fp, fc, nullptr, 0.05f);
            ImRect bb(ImVec2(FLT_MAX, FLT_MAX), ImVec2(-FLT_MAX, -FLT_MAX));
            for (int k = 0; k < fp.Size; ++k) bb.Add(fp[k]);
            item->bbox = bb;
        }
        item->xf = xf;
        p.Transform(xf);
        item->path = p;
        p.Points.clear(); p.Subpaths.clear();
        item->style = st;
        item->strokeScale = ImSqrt(ImFabs(xf.m[0] * xf.m[3] - xf.m[1] * xf.m[2]));
        doc->items.push_back(item);
    }
    while (stack.Size) { IM_DELETE(stack.back()); stack.pop_back(); }
    if (!sawSvg && doc->viewBox.GetWidth() <= 0.0f) { doc->viewBox = ImRect(0, 0, 100, 100); doc->size = ImVec2(100, 100); }

    // Resolve gradient href chains (attributes and stops are inherited when missing).
    for (int pass = 0; pass < 4; ++pass)
        for (int i = 0; i < doc->gradients.Size; ++i)
        {
            DW_SvgGradient* g = doc->gradients[i];
            if (!g->href[0]) continue;
            for (int j = 0; j < doc->gradients.Size; ++j)
            {
                DW_SvgGradient* b = doc->gradients[j];
                if (b == g || strcmp(b->id, g->href) != 0) continue;
                if (g->stops.Size == 0 && b->stops.Size) g->stops = b->stops;
                if (!g->hasUnits && b->hasUnits) { g->userSpace = b->userSpace; g->hasUnits = true; }
                if (!g->hasSpread && b->hasSpread) { g->spread = b->spread; g->hasSpread = true; }
                if (!g->hasTransform && b->hasTransform) { g->transform = b->transform; g->hasTransform = true; }
                if (!g->hasX1 && b->hasX1) { g->x1 = b->x1; g->x1Pct = b->x1Pct; g->hasX1 = true; }
                if (!g->hasY1 && b->hasY1) { g->y1 = b->y1; g->y1Pct = b->y1Pct; g->hasY1 = true; }
                if (!g->hasX2 && b->hasX2) { g->x2 = b->x2; g->x2Pct = b->x2Pct; g->hasX2 = true; }
                if (!g->hasY2 && b->hasY2) { g->y2 = b->y2; g->y2Pct = b->y2Pct; g->hasY2 = true; }
                if (!g->hasCx && b->hasCx) { g->cx = b->cx; g->cxPct = b->cxPct; g->hasCx = true; }
                if (!g->hasCy && b->hasCy) { g->cy = b->cy; g->cyPct = b->cyPct; g->hasCy = true; }
                if (!g->hasR && b->hasR)   { g->r = b->r;   g->rPct = b->rPct;   g->hasR = true; }
                if (!g->hasFr && b->hasFr) { g->fr = b->fr; g->frPct = b->frPct; g->hasFr = true; }
                break;
            }
        }
    return doc;
}

void ImWidgets::SvgFree(ImWidgetsSvg* doc) { if (doc) IM_DELETE(doc); }

ImVec2 ImWidgets::SvgGetSize(const ImWidgetsSvg* doc) { return doc ? doc->size : ImVec2(0, 0); }

namespace ImWidgets {

// Builds the paint for an item (document space), then maps it to the screen.
static bool DW_SvgMakePaint(const ImWidgetsSvg* doc, const DW_SvgItem& it, const DW_SvgPaintRef& ref, float opacity,
                            ImU32 current, const ImWidgetsTransform2D& docToScreen, ImWidgetsPaint& out,
                            ImVector<ImWidgetsGradientStop>& stopsBuf)
{
    auto withAlpha = [](ImU32 c, float a) {
        float ca = ((c >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f * a;
        return (c & ~IM_COL32_A_MASK) | ((ImU32)(ImSaturate(ca) * 255.0f + 0.5f) << IM_COL32_A_SHIFT);
    };
    switch (ref.kind)
    {
    case DW_SvgPaint_None: case DW_SvgPaint_Unset: return false;
    case DW_SvgPaint_Color:   out = ImWidgetsPaint::Solid(withAlpha(ref.color, opacity)); return true;
    case DW_SvgPaint_Current: out = ImWidgetsPaint::Solid(withAlpha(current, opacity)); return true;
    default: break;
    }
    const DW_SvgGradient* g = nullptr;
    for (int i = 0; i < doc->gradients.Size; ++i)
        if (!strcmp(doc->gradients[i]->id, ref.url)) { g = doc->gradients[i]; break; }
    // Missing gradient (or no stops): the fallback colour if one was given, else nothing.
    if (!g || g->stops.Size == 0)
    {
        if (!ref.hasFallback) return false;
        out = ImWidgetsPaint::Solid(withAlpha(ref.color, opacity));
        return true;
    }
    if (g->stops.Size == 1) { out = ImWidgetsPaint::Solid(withAlpha(g->stops[0].Color, opacity)); return true; }

    // Stops with opacity applied; the caller's buffer outlives the draw call.
    stopsBuf.resize(0);
    for (int i = 0; i < g->stops.Size; ++i) { ImWidgetsGradientStop st = g->stops[i]; st.Color = withAlpha(st.Color, opacity); stopsBuf.push_back(st); }

    ImWidgetsPaint p;
    p.Type = g->type;
    p.Stops = stopsBuf.Data;
    p.StopCount = g->stops.Size;
    p.Spread = g->spread;
    // Gradient space lives in the element's user space (its transform
    // included): objectBoundingBox maps the unit square onto the element's
    // local bbox; then gradientTransform; then element -> document -> screen.
    ImWidgetsTransform2D unitToLocal;
    if (!g->userSpace)
    {
        // A zero-width or zero-height box has no gradient space: draw nothing (spec).
        if (it.bbox.GetWidth() <= 0.0f || it.bbox.GetHeight() <= 0.0f) return false;
        unitToLocal = ImWidgetsTransform2D(it.bbox.GetWidth(), 0, 0, it.bbox.GetHeight(), it.bbox.Min.x, it.bbox.Min.y);
    }
    p.Transform = docToScreen * it.xf * unitToLocal * g->transform;
    // Resolve coordinates: in bbox units percentages are fractions; in user
    // space they are percentages of the viewport (r: of its normalised diagonal).
    const float vw = doc->viewBox.GetWidth(), vh = doc->viewBox.GetHeight();
    const float vd = ImSqrt((vw * vw + vh * vh) * 0.5f);
    auto res = [&](float v, bool pct, float ref) { return pct ? (g->userSpace ? v * 0.01f * ref : v * 0.01f) : v; };
    if (g->type == ImWidgetsPaintType_LinearGradient)
    {
        p.P0 = ImVec2(res(g->x1, g->x1Pct, vw), res(g->y1, g->y1Pct, vh));
        p.P1 = ImVec2(res(g->x2, g->x2Pct, vw), res(g->y2, g->y2Pct, vh));
    }
    else
    {
        p.P0 = ImVec2(res(g->cx, g->cxPct, vw), res(g->cy, g->cyPct, vh)); // focal point not supported: uses the centre
        p.R0 = res(g->fr, g->frPct, vd);
        p.R1 = res(g->r, g->rPct, vd);
    }
    out = p;
    return true;
}

void SvgDraw(ImDrawList* dl, const ImWidgetsSvg* doc, ImVec2 pos, ImVec2 size, ImU32 current_color)
{
    if (!dl || !doc || doc->viewBox.GetWidth() <= 0.0f || doc->viewBox.GetHeight() <= 0.0f) return;
    if (size.x <= 0.0f) size.x = doc->size.x;
    if (size.y <= 0.0f) size.y = doc->size.y;
    // viewBox -> target rect, preserveAspectRatio="xMidYMid meet" (the SVG default).
    const ImRect& vb = doc->viewBox;
    const float sc = ImMin(size.x / vb.GetWidth(), size.y / vb.GetHeight());
    const float tx = pos.x + (size.x - vb.GetWidth() * sc) * 0.5f - vb.Min.x * sc;
    const float ty = pos.y + (size.y - vb.GetHeight() * sc) * 0.5f - vb.Min.y * sc;
    const ImWidgetsTransform2D docToScreen(sc, 0, 0, sc, tx, ty);

    ImVector<ImWidgetsGradientStop> stopsBuf;
    for (int i = 0; i < doc->items.Size; ++i)
    {
        const DW_SvgItem& it = *doc->items[i];
        ImWidgetsPath path = it.path;
        path.Transform(docToScreen);
        const DW_SvgStyle& st = it.style;
        ImWidgetsPaint paint;
        if (DW_SvgMakePaint(doc, it, st.fill, st.fillOpacity * st.opacity, current_color, docToScreen, paint, stopsBuf))
            PathFill(dl, path, paint, st.fillRule);
        if (st.strokeWidth > 0.0f && DW_SvgMakePaint(doc, it, st.stroke, st.strokeOpacity * st.opacity, current_color, docToScreen, paint, stopsBuf))
        {
            ImWidgetsStrokeStyle ss;
            const float k = sc * it.strokeScale;
            ss.Width = st.strokeWidth * k;
            ss.Cap = st.cap; ss.Join = st.join; ss.MiterLimit = st.miterLimit;
            float dashes[16];
            for (int d = 0; d < st.dashCount; ++d) dashes[d] = st.dashes[d] * k;
            ss.Dashes = st.dashCount ? dashes : nullptr;
            ss.DashCount = st.dashCount;
            ss.DashOffset = st.dashOffset * k;
            if (paint.Type == ImWidgetsPaintType_Solid) PathStroke(dl, path, paint.Color, ss);
            else                                         PathStroke(dl, path, paint, ss);
        }
        path.Points.clear(); path.Subpaths.clear();
    }
}

} // namespace ImWidgets
