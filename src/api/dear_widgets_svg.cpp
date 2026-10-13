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
//   text        <text> (x, y, font-size, text-anchor, direction, fill paint;
//               nested <tspan> content is flattened into the text; one font,
//               no stroke):
//               shaped, bidi-ordered glyph outlines transformed like any
//               other shape (rotation, skew, gradients all apply)
//   clipping    <clipPath> (shapes, groups with transforms, clip-rule,
//               clipPathUnits) referenced by clip-path="url(#id)" on shapes and
//               groups (nested references intersect, up to 4)
// Not supported: images, masks, filters, patterns, markers, use / symbol,
// CSS stylesheets (<style>), tspan positioning.

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
    const char*          start = nullptr;     // the tag's '<' in the source

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
    tag.start = s;
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
    float  fontSize = 16.0f;   // inherited
    int    textAnchor = 0;     // 0 start, 1 middle, 2 end
    int    direction = ImWidgetsTextDirection_LTR; // SVG initial value: ltr
    int    clipRule = ImWidgetsFillRule_NonZero;    // inherited
};

// A clip-path reference: the clip's id and the user space it is used in
// (the referencing element's coordinate system, its transform included).
struct DW_SvgClipRef { char id[64]; ImWidgetsTransform2D xf; int serial; }; // serial: the referencing element
enum { DW_SvgMaxClipRefs = 4 };

struct DW_SvgClipDef
{
    char          id[64];
    ImWidgetsPath path;       // union of the child shapes (clip space, without the clipPath's own transform)
    ImWidgetsTransform2D xf;  // the clipPath element's transform
    bool          bboxUnits;  // clipPathUnits="objectBoundingBox"
    int           rule;       // clip-rule (one child: its rule; several: nonzero on normalised children)
    int           children;   // supported children added
    bool          unsupported;// had children we cannot use (<use>, <text>): then ignore the clip
    int           defsDepthAtStart;
    ImVector<int> childFirst; // first subpath of each child (for the union normalisation)
};

// Several clip children are combined as a nonzero union: orient each child's
// subpaths so the outer ones are positive and the nested ones (holes, by
// nesting depth parity) negative. A child's own region is then exactly
// winding 1, so overlapping children add up instead of cancelling.
static void DW_SvgNormaliseClipChild(ImWidgetsPath& path, int subFirst, int subEnd)
{
    ImWidgetsPath tmp;
    tmp.Points = path.Points;
    for (int k = subFirst; k < subEnd; ++k) tmp.Subpaths.push_back(path.Subpaths[k]);
    ImVector<ImVec2> fp; ImVector<int> fc;
    tmp.Flatten(fp, fc, nullptr, 0.5f);
    if (fc.Size != subEnd - subFirst) return; // degenerate subpaths: leave as is
    ImVector<int> start; start.resize(fc.Size);
    for (int c = 0, b = 0; c < fc.Size; b += fc[c], ++c) start[c] = b;
    for (int c = 0; c < fc.Size; ++c)
    {
        if (fc[c] < 3) continue;
        double area = 0.0;
        for (int k = 0; k < fc[c]; ++k)
        {
            ImVec2 a = fp[start[c] + k], b = fp[start[c] + (k + 1) % fc[c]];
            area += (double)a.x * b.y - (double)b.x * a.y;
        }
        // nesting depth: how many other contours contain this one's first point
        const ImVec2 q = fp[start[c]];
        int depth = 0;
        for (int o = 0; o < fc.Size; ++o)
        {
            if (o == c || fc[o] < 3) continue;
            bool in = false;
            for (int k = 0, j = fc[o] - 1; k < fc[o]; j = k++)
            {
                ImVec2 a = fp[start[o] + k], b = fp[start[o] + j];
                if ((a.y > q.y) != (b.y > q.y) && q.x < (b.x - a.x) * (q.y - a.y) / (b.y - a.y) + a.x) in = !in;
            }
            if (in) ++depth;
        }
        const bool wantPositive = (depth & 1) == 0;
        if ((area > 0.0) != wantPositive)
        {
            // reverse the cubic chain point by point (still a valid chain)
            const ImWidgetsSubpath& sub = path.Subpaths[subFirst + c];
            for (int a2 = sub.First, b2 = sub.First + sub.Count - 1; a2 < b2; ++a2, --b2) { ImVec2 t = path.Points[a2]; path.Points[a2] = path.Points[b2]; path.Points[b2] = t; }
        }
    }
}

struct DW_SvgItem
{
    DW_SvgClipRef clips[DW_SvgMaxClipRefs];
    int           clipCount = 0;
    ImWidgetsPath        path;        // in document user space (element transforms applied)
    DW_SvgStyle          style;
    float                strokeScale; // scale of the element's transform (stroke width is in its local units)
    ImRect               bbox;        // object bounding box in the element's local space (objectBoundingBox gradients)
    ImWidgetsTransform2D xf;          // element user space -> document space (gradients live in user space)
    ImVector<char>       text;        // <text> content (UTF-8, whitespace collapsed); empty for shapes
    ImVec2               textPos;     // x, y of the first character (element space)
};

} // namespace ImWidgets

struct ImWidgetsSvg
{
    ImVector<ImWidgets::DW_SvgItem*>      items;
    ImVector<ImWidgets::DW_SvgGradient*>  gradients;
    ImVector<ImWidgets::DW_SvgClipDef*>   clipDefs;
    ImVector<ImRect>                      refBoxes;     // by clip reference serial: bbox of the referencing element (its own space)
    ImRect                                viewBox;
    ImVec2                                size;   // intrinsic size (width / height, else viewBox)
    ~ImWidgetsSvg()
    {
        for (int i = 0; i < items.Size; ++i) IM_DELETE(items[i]);
        for (int i = 0; i < gradients.Size; ++i) IM_DELETE(gradients[i]);
        for (int i = 0; i < clipDefs.Size; ++i) IM_DELETE(clipDefs[i]);
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
    else if (!strcmp(name, "clip-rule")) st.clipRule = strstr(v, "evenodd") ? ImWidgetsFillRule_EvenOdd : ImWidgetsFillRule_NonZero;
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
    else if (!strcmp(name, "font-size")) st.fontSize = DW_SvgLength(v, st.fontSize, st.fontSize);
    else if (!strcmp(name, "text-anchor")) st.textAnchor = strstr(v, "middle") ? 1 : strstr(v, "end") ? 2 : 0;
    else if (!strcmp(name, "direction")) { if (strstr(v, "rtl")) st.direction = ImWidgetsTextDirection_RTL; else if (strstr(v, "ltr")) st.direction = ImWidgetsTextDirection_LTR; }
    else if (!strcmp(name, "visibility")) st.visible = !(strstr(v, "hidden") || strstr(v, "collapse"));
}

// ---- <style> sheets ----------------------------------------------------------
// Simple selectors only: type, .class, #id, their compounds (rect.a.b) and
// comma lists, with CSS specificity and source order. Rules with combinators,
// pseudo-classes or attribute selectors are ignored; at-rules are skipped.
struct DW_SvgCssRule
{
    char tag[32];        // "" or "*" = any
    char id[64];
    char classes[4][48];
    int  classCount;
    int  specificity;    // id 100, class 10, type 1
    int  declOff;        // into DW_SvgCss::decls (NUL-terminated block)
};
struct DW_SvgCss
{
    ImVector<DW_SvgCssRule> rules; // in source order
    ImVector<char>          decls;
};

static void DW_SvgParseCss(const char* b, const char* e, DW_SvgCss& css)
{
    // Copy without comments and CDATA markers.
    ImVector<char> src;
    for (const char* q = b; q < e; )
    {
        if (q + 1 < e && q[0] == '/' && q[1] == '*') { const char* c = strstr(q + 2, "*/"); q = (c && c < e) ? c + 2 : e; continue; }
        if (e - q >= 9 && strncmp(q, "<![CDATA[", 9) == 0) { q += 9; continue; }
        if (e - q >= 3 && strncmp(q, "]]>", 3) == 0) { q += 3; continue; }
        src.push_back(*q++);
    }
    src.push_back(0);
    const char* p = src.Data;
    while (*p)
    {
        while (*p && isspace((unsigned char)*p)) ++p;
        if (!*p) break;
        if (*p == '@')
        {
            // at-rule: up to ';' or a balanced block
            while (*p && *p != ';' && *p != '{') ++p;
            if (*p == '{') { int depth = 0; do { if (*p == '{') ++depth; else if (*p == '}') --depth; ++p; } while (*p && depth > 0); }
            else if (*p) ++p;
            continue;
        }
        const char* selB = p;
        while (*p && *p != '{') ++p;
        if (!*p) break;
        const char* selE = p++;
        const char* blkB = p;
        while (*p && *p != '}') ++p;
        const int declOff = css.decls.Size;
        for (const char* q = blkB; q < p; ++q) css.decls.push_back(*q);
        css.decls.push_back(0);
        if (*p) ++p;
        // each selector of the list
        for (const char* a = selB; a < selE; )
        {
            const char* c = a; while (c < selE && *c != ',') ++c;
            const char* x = a; const char* y = c;
            while (x < y && isspace((unsigned char)*x)) ++x;
            while (y > x && isspace((unsigned char)y[-1])) --y;
            a = c + 1;
            if (x == y) continue;
            bool ok = true;
            for (const char* q = x; q < y; ++q) if (isspace((unsigned char)*q) || strchr(">+~:[", *q)) { ok = false; break; }
            if (!ok) continue;
            DW_SvgCssRule r; memset(&r, 0, sizeof(r));
            r.declOff = declOff;
            const char* q = x;
            int n = 0;
            while (q < y && *q != '.' && *q != '#') { if (n < 31) r.tag[n++] = *q; ++q; }
            if (n && strcmp(r.tag, "*")) r.specificity += 1;
            while (q < y && ok)
            {
                const char kind = *q++;
                const char* nb = q; while (q < y && *q != '.' && *q != '#') ++q;
                const int len = (int)(q - nb);
                if (len <= 0) { ok = false; break; }
                if (kind == '#') { ImStrncpy(r.id, nb, (size_t)ImMin(len + 1, 64)); r.specificity += 100; }
                else if (r.classCount < 4) { ImStrncpy(r.classes[r.classCount++], nb, (size_t)ImMin(len + 1, 48)); r.specificity += 10; }
                else ok = false;
            }
            if (ok) css.rules.push_back(r);
        }
    }
}

static bool DW_SvgCssMatch(const DW_SvgCssRule& r, const DW_SvgTag& tag)
{
    if (r.tag[0] && strcmp(r.tag, "*") && strcmp(r.tag, tag.Name())) return false;
    if (r.id[0]) { const char* id = tag.Get("id"); if (!id || strcmp(id, r.id)) return false; }
    if (r.classCount)
    {
        const char* cls = tag.Get("class");
        if (!cls) return false;
        for (int k = 0; k < r.classCount; ++k)
        {
            const size_t l = strlen(r.classes[k]);
            bool found = false;
            for (const char* q = cls; *q && !found; )
            {
                while (*q && isspace((unsigned char)*q)) ++q;
                const char* nb = q; while (*q && !isspace((unsigned char)*q)) ++q;
                found = ((size_t)(q - nb) == l && strncmp(nb, r.classes[k], l) == 0);
            }
            if (!found) return false;
        }
    }
    return true;
}

// Start of the closing tag "</name>" (any namespace prefix, whitespace
// allowed before '>'), or null.
static const char* DW_SvgFindClose(const char* s, const char* name)
{
    const size_t nl = strlen(name);
    for (const char* e = strstr(s, "</"); e; e = strstr(e + 2, "</"))
    {
        const char* q = e + 2; while (isspace((unsigned char)*q)) ++q;
        const char* nb = q; while (*q && *q != '>' && !isspace((unsigned char)*q)) ++q;
        const char* local = nb; for (const char* r = nb; r < q; ++r) if (*r == ':') local = r + 1;
        if ((size_t)(q - local) == nl && strncmp(local, name, nl) == 0) return e;
    }
    return nullptr;
}

// Applies a declaration block ("fill:red; stroke:blue").
static void DW_SvgApplyDecls(DW_SvgStyle& st, const char* css)
{
    {
        const char* p = css;
        while (*p)
        {
            while (*p == ';' || isspace((unsigned char)*p)) ++p;
            const char* nb = p;
            while (*p && *p != ':' && *p != ';') ++p;
            if (*p != ':') { while (*p && *p != ';') ++p; continue; } // malformed declaration: skip it
            const char* ne = p; while (ne > nb && isspace((unsigned char)ne[-1])) --ne;
            ++p;
            while (isspace((unsigned char)*p)) ++p;
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

// Inheritable presentation properties of an element, in CSS cascade order:
// presentation attributes, then matching <style> rules (by specificity, then
// source order), then the style= attribute.
static void DW_SvgApplyStyle(DW_SvgStyle& st, const DW_SvgTag& tag, const DW_SvgCss* css = nullptr)
{
    static const char* kProps[] = { "fill", "stroke", "fill-opacity", "stroke-opacity", "opacity", "stroke-width",
        "stroke-miterlimit", "stroke-dashoffset", "fill-rule", "stroke-linecap", "stroke-linejoin", "stroke-dasharray",
        "display", "visibility", "font-size", "text-anchor", "direction", "clip-rule" };
    st.ownOpacity = 1.0f;
    for (const char* k : kProps) DW_SvgApplyProp(st, k, tag.Get(k));
    if (css && css->rules.Size)
    {
        ImVector<int> match;
        for (int i = 0; i < css->rules.Size; ++i)
            if (DW_SvgCssMatch(css->rules[i], tag))
            {
                // insertion by specificity, stable for source order
                match.push_back(i);
                int k = match.Size - 1;
                while (k > 0 && css->rules[match[k - 1]].specificity > css->rules[i].specificity) { match[k] = match[k - 1]; --k; }
                match[k] = i;
            }
        for (int k = 0; k < match.Size; ++k) DW_SvgApplyDecls(st, css->decls.Data + css->rules[match[k]].declOff);
    }
    if (const char* decl = tag.Get("style")) DW_SvgApplyDecls(st, decl);
}

static float DW_SvgAttrF(const DW_SvgTag& t, const char* n, float def, float pct_ref = 0.0f) { return DW_SvgLength(t.Get(n), def, pct_ref); }

// clip-path="url(#id)" (attribute or style declaration) -> id.
static bool DW_SvgClipPathRef(const DW_SvgTag& tag, char* out, int out_size)
{
    const char* v = tag.Get("clip-path");
    char buf[128] = {};
    if (!v)
        if (const char* css = tag.Get("style"))
            if (const char* p = strstr(css, "clip-path"))
                if ((p = strchr(p, ':')) != nullptr)
                {
                    ++p; int n = 0;
                    while (p[n] && p[n] != ';' && n < 127) { buf[n] = p[n]; ++n; }
                    v = buf;
                }
    if (!v) return false;
    const char* u = strstr(v, "url(");
    if (!u) return false;
    u += 4;
    while (*u == ' ' || *u == '\'' || *u == '"') ++u;
    if (*u == '#') ++u;
    int n = 0;
    while (u[n] && u[n] != ')' && u[n] != '\'' && u[n] != '"' && n < out_size - 1) { out[n] = u[n]; ++n; }
    out[n] = 0;
    return n > 0;
}

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

    struct Frame { DW_SvgStyle style; ImWidgetsTransform2D xf; DW_SvgClipRef clips[DW_SvgMaxClipRefs]; int clipCount = 0; bool isClipRoot = false; DW_SvgClipDef* outerClip = nullptr; };
    ImVector<Frame*> stack;
    Frame* root = IM_NEW(Frame)();
    root->style.fill.kind = DW_SvgPaint_Color; root->style.fill.color = IM_COL32_BLACK; // SVG initial fill
    root->style.stroke.kind = DW_SvgPaint_None;
    stack.push_back(root);

    // Pre-pass: source range of every element with an id (for <use>) and
    // the <style> sheets (rules apply to elements before them too).
    struct IdRange { const char* b; const char* content; const char* e; };
    ImVector<IdRange> idRanges;
    ImGuiStorage      idMap;   // ImHashStr(id) -> index + 1 (first definition wins)
    DW_SvgCss         css;
    {
        struct Open { char name[32]; int idx; };
        ImVector<Open> open;
        const char* q = text;
        DW_SvgTag t;
        while (DW_SvgNextTag(q, t))
        {
            if (t.closing)
            {
                // pop to the matching element (tolerates unclosed children)
                int k = open.Size - 1;
                while (k >= 0 && strcmp(open[k].name, t.Name())) --k;
                if (k < 0) continue;
                while (open.Size > k)
                {
                    if (open.back().idx >= 0) idRanges[open.back().idx].e = q;
                    open.pop_back();
                }
                continue;
            }
            int idx = -1;
            if (const char* id = t.Get("id"))
                if (*id && idMap.GetInt(ImHashStr(id), 0) == 0)
                {
                    IdRange r = { t.start, q, q };
                    idRanges.push_back(r);
                    idx = idRanges.Size - 1;
                    idMap.SetInt(ImHashStr(id), idx + 1);
                }
            // Shapes and other empty elements never have children: treat them
            // as closed even when written HTML-style without "/>".
            static const char* kEmpty[] = { "rect", "circle", "ellipse", "line", "polyline", "polygon", "path", "use", "stop", "image" };
            bool empty = t.selfClosing;
            for (const char* k : kEmpty) if (!strcmp(t.Name(), k)) empty = true;
            if (empty) continue;
            Open o; ImStrncpy(o.name, t.Name(), sizeof(o.name)); o.idx = idx;
            open.push_back(o);
            if (!strcmp(t.Name(), "style"))
            {
                // raw CSS up to </style> (the tokenizer would skip its CDATA);
                // an unclosed sheet is ignored rather than eating the document
                if (const char* e = DW_SvgFindClose(q, "style"))
                {
                    DW_SvgParseCss(q, e, css);
                    q = e;
                }
            }
        }
        // unclosed elements end with the document
        for (int k = 0; k < open.Size; ++k) if (open[k].idx >= 0) idRanges[open[k].idx].e = q;
    }

    DW_SvgGradient* curGrad = nullptr;
    DW_SvgClipDef*  curClip = nullptr;   // inside <clipPath>: shapes feed the clip, not the document
    int clipSerial = 0;
    int defsDepth = 0;
    const char* s = text;
    DW_SvgTag tag;
    bool sawSvg = false;
    // <use> expansion: the referenced element's source is parsed in place
    // (no recursion); at its end the parser resumes after the <use>.
    struct UseSrc { const char* resume; const char* end; int stackSize; int defsDepth; int idIdx; DW_SvgGradient* grad; DW_SvgClipDef* clip; };
    ImVector<UseSrc> srcs;
    // Total source re-parsed by <use> expansions: bounded so chains of uses
    // that multiply (a uses b ten times, b uses c ten times...) stay linear.
    const size_t textLen = strlen(text);
    size_t useBudget = ImMax(textLen * 16, (size_t)(4u << 20));
    for (;;)
    {
        if (srcs.Size && s >= srcs.back().end)
        {
            const UseSrc u = srcs.back();
            srcs.pop_back();
            while (stack.Size > u.stackSize) { IM_DELETE(stack.back()); stack.pop_back(); }
            s = u.resume; defsDepth = u.defsDepth; curGrad = u.grad; curClip = u.clip;
            continue;
        }
        if (!DW_SvgNextTag(s, tag))
        {
            if (srcs.Size) { s = srcs.back().end; continue; }
            break;
        }
        const char* name = tag.Name();
        if (!strcmp(name, "style"))
        {
            // parsed in the pre-pass: skip the sheet's text
            if (!tag.selfClosing)
            {
                if (const char* e = DW_SvgFindClose(s, "style")) s = e;
            }
            continue;
        }
        if (tag.closing)
        {
            if (!strcmp(name, "linearGradient") || !strcmp(name, "radialGradient")) { curGrad = nullptr; continue; }
            if (!strcmp(name, "clipPath"))
            {
                if (curClip)
                {
                    if (curClip->children > 1)
                        for (int c = 0; c < curClip->childFirst.Size; ++c)
                            DW_SvgNormaliseClipChild(curClip->path, curClip->childFirst[c],
                                                     c + 1 < curClip->childFirst.Size ? curClip->childFirst[c + 1] : curClip->path.Subpaths.Size);
                    DW_SvgClipDef* outer = nullptr; // a clipPath nested in another (or reached by <use> from one)
                    if (stack.Size > 1 && stack.back()->isClipRoot) { outer = stack.back()->outerClip; IM_DELETE(stack.back()); stack.pop_back(); }
                    curClip = outer;
                }
                continue;
            }
            if (!strcmp(name, "defs") || !strcmp(name, "symbol") || !strcmp(name, "mask")
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
                if (const char* decl = tag.Get("style"))
                {
                    if (const char* p = strstr(decl, "stop-color")) { p = strchr(p, ':'); if (p) { ++p; int n = 0; while (p[n] && p[n] != ';' && n < 63) { scBuf[n] = p[n]; ++n; } sc = scBuf; } }
                    if (const char* p = strstr(decl, "stop-opacity")) { p = strchr(p, ':'); if (p) { ++p; int n = 0; while (p[n] && p[n] != ';' && n < 31) { soBuf[n] = p[n]; ++n; } so = soBuf; } }
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
        // <clipPath>: its shapes are collected into a clip definition (in the
        // clip's own space; the user space of the referencing element is
        // applied at draw time). Works inside or outside <defs>.
        if (!strcmp(name, "clipPath"))
        {
            DW_SvgClipDef* cd = IM_NEW(DW_SvgClipDef)();
            memset(cd->id, 0, sizeof(cd->id));
            if (const char* id = tag.Get("id")) ImStrncpy(cd->id, id, sizeof(cd->id));
            const char* units = tag.Get("clipPathUnits");
            cd->bboxUnits = units && strstr(units, "objectBoundingBox");
            cd->rule = ImWidgetsFillRule_NonZero; cd->children = 0; cd->unsupported = false;
            cd->xf = DW_SvgParseTransform(tag.Get("transform"));
            cd->defsDepthAtStart = defsDepth;
            doc->clipDefs.push_back(cd);
            if (!tag.selfClosing)
            {
                Frame* f = IM_NEW(Frame)();
                f->outerClip = curClip;
                curClip = cd;
                f->style = stack.back()->style;
                DW_SvgApplyStyle(f->style, tag, &css);
                f->xf = ImWidgetsTransform2D(); // children in clip space; the clipPath transform is applied at draw time
                f->isClipRoot = true;
                stack.push_back(f);
            }
            continue;
        }
        // Not rendered directly: defs and the referenced-only containers
        // (masks, symbols, patterns, markers are unsupported, so their content
        // must not show as plain shapes).
        if (!strcmp(name, "defs") || !strcmp(name, "symbol") || !strcmp(name, "mask")
            || !strcmp(name, "pattern") || !strcmp(name, "marker")) { if (!tag.selfClosing) ++defsDepth; continue; }
        // Inside a clipPath: <text> is not supported (it falls through to its
        // parser, which consumes the content and drops it); nested defs /
        // masks do not feed the clip.
        if (curClip && !strcmp(name, "text")) curClip->unsupported = true;
        if (curClip && defsDepth > curClip->defsDepthAtStart)
        {
            if ((!strcmp(name, "g") || !strcmp(name, "svg") || !strcmp(name, "a") || !strcmp(name, "switch")) && !tag.selfClosing)
            {
                Frame* f = IM_NEW(Frame)(); f->style = stack.back()->style; f->xf = stack.back()->xf; stack.push_back(f); // balanced by the closing tag
            }
            continue;
        }
        if (defsDepth > 0 && !curClip)
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
        DW_SvgApplyStyle(st, tag, &css);
        st.opacity = parent->style.opacity * st.ownOpacity;
        ImWidgetsTransform2D xf = parent->xf * DW_SvgParseTransform(tag.Get("transform"));
        // clip-path references: inherited from the enclosing groups, plus this
        // element's own (in its user space, its transform included).
        DW_SvgClipRef clips[DW_SvgMaxClipRefs];
        int clipCount = curClip ? 0 : parent->clipCount;
        for (int c = 0; c < clipCount; ++c) clips[c] = parent->clips[c];
        if (!curClip && clipCount < DW_SvgMaxClipRefs)
        {
            char cid[64];
            if (DW_SvgClipPathRef(tag, cid, (int)sizeof(cid)))
            {
                ImStrncpy(clips[clipCount].id, cid, sizeof(clips[clipCount].id));
                clips[clipCount].xf = xf;
                clips[clipCount].serial = ++clipSerial;
                ++clipCount;
            }
        }
        auto pushFrame = [&]() {
            Frame* f = IM_NEW(Frame)(); f->style = st; f->xf = xf;
            for (int c = 0; c < clipCount; ++c) f->clips[c] = clips[c];
            f->clipCount = clipCount;
            stack.push_back(f);
        };

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
            if (!tag.selfClosing) pushFrame();
            continue;
        }
        if (!strcmp(name, "g") || !strcmp(name, "a") || !strcmp(name, "switch"))
        {
            if (!tag.selfClosing) pushFrame();
            continue;
        }
        // <use href="#id">: the referenced element rendered as a child of the
        // <use>, translated by x / y; a <symbol> is a viewport (viewBox,
        // preserveAspectRatio) sized by the use's width / height.
        if (!strcmp(name, "use"))
        {
            const char* href = tag.Get("href"); if (!href) href = tag.Get("xlink:href");
            if (!href || href[0] != '#' || !st.display) continue;
            const int idx = idMap.GetInt(ImHashStr(href + 1), 0) - 1;
            if (idx < 0 || srcs.Size >= 16) continue;
            const IdRange& r = idRanges[idx];
            // A reference to an element being expanded, or to an ancestor of
            // this <use> (its source contains the use), is a cycle: an error,
            // nothing is rendered (as browsers do).
            bool cycle = (r.b <= tag.start && tag.start < r.e);
            for (int k = 0; k < srcs.Size; ++k) if (srcs[k].idIdx == idx) cycle = true;
            if (cycle) continue;
            const size_t cost = (size_t)(r.e - r.b);
            if (cost > useBudget) continue;
            useBudget -= cost;
            const float vbw = doc->viewBox.GetWidth(), vbh = doc->viewBox.GetHeight();
            xf = xf * ImWidgetsTransform2D(1, 0, 0, 1, DW_SvgAttrF(tag, "x", 0, vbw), DW_SvgAttrF(tag, "y", 0, vbh));
            const char* b = r.b;
            DW_SvgTag rt; const char* rq = r.b;
            if (!DW_SvgNextTag(rq, rt) || rt.start != r.b) continue;
            if (!strcmp(rt.Name(), "symbol"))
            {
                b = r.content;
                // the symbol's own presentation attributes / rules
                DW_SvgStyle s2 = st; DW_SvgApplyStyle(s2, rt, &css);
                s2.opacity = st.opacity * s2.ownOpacity; st = s2;
                float w = DW_SvgAttrF(tag, "width", DW_SvgAttrF(rt, "width", -1.0f, vbw), vbw);
                float h = DW_SvgAttrF(tag, "height", DW_SvgAttrF(rt, "height", -1.0f, vbh), vbh);
                if (w < 0.0f) w = vbw;
                if (h < 0.0f) h = vbh;
                float v[4]; int n = 0;
                if (const char* vb = rt.Get("viewBox")) { const char* q = vb; while (n < 4 && DW_SvgNum(q, v[n])) ++n; }
                if (n == 4 && v[2] > 0.0f && v[3] > 0.0f)
                {
                    float sx = w / v[2], sy = h / v[3];
                    const char* par = rt.Get("preserveAspectRatio");
                    float ax = 0.5f, ay = 0.5f;
                    if (!(par && strstr(par, "none")))
                    {
                        const float u = (par && strstr(par, "slice")) ? ImMax(sx, sy) : ImMin(sx, sy);
                        sx = sy = u;
                        if (par && strstr(par, "xMin")) ax = 0.0f; else if (par && strstr(par, "xMax")) ax = 1.0f;
                        if (par && strstr(par, "YMin")) ay = 0.0f; else if (par && strstr(par, "YMax")) ay = 1.0f;
                    }
                    xf = xf * ImWidgetsTransform2D(sx, 0, 0, sy, ax * (w - v[2] * sx) - v[0] * sx, ay * (h - v[3] * sy) - v[1] * sy);
                }
            }
            if (b >= r.e) continue;
            UseSrc u = { s, r.e, stack.Size, defsDepth, idx, curGrad, curClip };
            pushFrame(); // the use's style, transform and clips for the content
            srcs.push_back(u);
            defsDepth = curClip ? curClip->defsDepthAtStart : 0; // a referenced element in <defs> renders
            s = b;
            continue;
        }

        // <text>: read its character content (tspan content flattened), then
        // keep it as a text item; glyph outlines are made at draw time with
        // the font passed to SvgDraw.
        if (!strcmp(name, "text"))
        {
            ImVector<char> content;
            if (!tag.selfClosing)
            {
                bool lastSpace = true;
                int depth = 1;
                while (*s && depth > 0)
                {
                    if (*s == '<')
                    {
                        if (strncmp(s, "<![CDATA[", 9) == 0)
                        {
                            // character data: kept as is
                            const char* e = strstr(s + 9, "]]>");
                            const char* ce = e ? e : s + strlen(s);
                            for (const char* q = s + 9; q < ce; ++q) content.push_back(*q);
                            if (ce > s + 9) lastSpace = false;
                            s = e ? e + 3 : ce;
                            continue;
                        }
                        if (strncmp(s, "<!--", 4) == 0) { const char* e = strstr(s + 4, "-->"); s = e ? e + 3 : s + strlen(s); continue; }
                        if (strncmp(s, "</", 2) == 0)
                        {
                            // </text>, </text >, </svg:text> ... (prefix and whitespace allowed)
                            const char* q = s + 2; while (isspace((unsigned char)*q)) ++q;
                            const char* nb = q; while (*q && *q != '>' && !isspace((unsigned char)*q)) ++q;
                            const char* local = nb; for (const char* r = nb; r < q; ++r) if (*r == ':') local = r + 1;
                            if (q - local == 4 && strncmp(local, "text", 4) == 0) depth = 0;
                        }
                        const char* e = strchr(s, '>');
                        s = e ? e + 1 : s + strlen(s);
                        continue;
                    }
                    // xml:space="default": collapse whitespace runs into one space
                    char c = *s++;
                    if (c == '\n' || c == '\r' || c == '\t' || c == ' ')
                    {
                        if (!lastSpace) { content.push_back(' '); lastSpace = true; }
                        continue;
                    }
                    if (c == '&')
                    {
                        struct E { const char* s; char c; } ents[] = { { "amp;", '&' }, { "lt;", '<' }, { "gt;", '>' }, { "quot;", '"' }, { "apos;", '\'' } };
                        bool done = false;
                        for (const E& en : ents)
                        {
                            size_t l = strlen(en.s);
                            if (strncmp(s, en.s, l) == 0) { content.push_back(en.c); s += l; done = true; break; }
                        }
                        if (!done && *s == '#')
                        {
                            // numeric character reference
                            const char* q = s + 1; unsigned int cp = 0;
                            if (*q == 'x' || *q == 'X') { ++q; while (DW_SvgHex(*q) >= 0) cp = cp * 16 + (unsigned)DW_SvgHex(*q++); }
                            else while (*q >= '0' && *q <= '9') cp = cp * 10 + (unsigned)(*q++ - '0');
                            if (*q == ';' && cp != 0) { char u[5] = {}; int n = ImTextCharToUtf8(u, cp); for (int k = 0; k < n; ++k) content.push_back(u[k]); s = q + 1; done = true; }
                        }
                        if (done) { lastSpace = false; continue; }
                    }
                    content.push_back(c);
                    lastSpace = false;
                }
                while (content.Size && content.back() == ' ') content.pop_back();
            }
            if (content.Size == 0 || !st.display || !st.visible || curClip) continue; // text in clip paths: not supported
            content.push_back(0);
            DW_SvgItem* item = IM_NEW(DW_SvgItem)();
            for (int c = 0; c < clipCount; ++c) item->clips[c] = clips[c];
            item->clipCount = clipCount;
            item->style = st;
            item->xf = xf;
            item->strokeScale = ImSqrt(ImFabs(xf.m[0] * xf.m[3] - xf.m[1] * xf.m[2]));
            const float vbw0 = doc->viewBox.GetWidth(), vbh0 = doc->viewBox.GetHeight();
            item->textPos = ImVec2(DW_SvgAttrF(tag, "x", 0.0f, vbw0), DW_SvgAttrF(tag, "y", 0.0f, vbh0));
            item->text = content;
            // Approximate bbox (for objectBoundingBox gradients): em box of the
            // run, from the codepoint count (not bytes).
            int ncp = 0;
            for (int k = 0; k < content.Size - 1; ++k) if (((unsigned char)content[k] & 0xC0) != 0x80) ++ncp;
            float w = 0.55f * st.fontSize * (float)ncp;
            const bool rtlRun = st.direction == ImWidgetsTextDirection_RTL;
            const int anchor = rtlRun ? (st.textAnchor == 0 ? 2 : st.textAnchor == 2 ? 0 : 1) : st.textAnchor;
            float x0 = item->textPos.x - (anchor == 1 ? w * 0.5f : anchor == 2 ? w : 0.0f);
            item->bbox = ImRect(x0, item->textPos.y - st.fontSize * 0.8f, x0 + w, item->textPos.y + st.fontSize * 0.2f);
            doc->items.push_back(item);
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

        if (!shape || p.Subpaths.Size == 0) continue;
        if (curClip)
        {
            // Clip children: union of their shapes (display:none ones excluded).
            if (!st.display || !st.visible) continue;
            p.Transform(xf);
            // The clip region is the union of the children, each with its own
            // clip-rule. With one child its rule is kept; with several, the
            // children are normalised when the clipPath closes and combined
            // as nonzero (holes are taken by nesting depth, i.e. even-odd).
            curClip->childFirst.push_back(curClip->path.Subpaths.Size);
            for (int sp = 0; sp < p.Subpaths.Size; ++sp)
            {
                ImWidgetsSubpath sub = p.Subpaths[sp];
                sub.First += curClip->path.Points.Size;
                curClip->path.Subpaths.push_back(sub);
            }
            for (int k = 0; k < p.Points.Size; ++k) curClip->path.Points.push_back(p.Points[k]);
            curClip->rule = (curClip->children == 0) ? st.clipRule : ImWidgetsFillRule_NonZero;
            ++curClip->children;
            continue;
        }
        if (!st.display || !st.visible) continue;
        DW_SvgItem* item = IM_NEW(DW_SvgItem)();
        for (int c = 0; c < clipCount; ++c) item->clips[c] = clips[c];
        item->clipCount = clipCount;
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

    // Bounding box of each clip-referencing element, for objectBoundingBox
    // clips: the union of all items under it, each item's box mapped straight
    // from its space into the referencing element's (exact when they differ
    // by translation / scale only, e.g. shapes inside a rotated group).
    for (int i = 0; i < doc->items.Size; ++i)
    {
        DW_SvgItem* it = doc->items[i];
        if (it->bbox.Min.x > it->bbox.Max.x) continue;
        for (int c = 0; c < it->clipCount; ++c)
        {
            const DW_SvgClipRef& r = it->clips[c];
            const float* m = r.xf.m;
            const float det = m[0] * m[3] - m[1] * m[2];
            if (ImFabs(det) < 1e-12f) continue;
            const float id2 = 1.0f / det;
            ImWidgetsTransform2D inv(m[3] * id2, -m[1] * id2, -m[2] * id2, m[0] * id2, 0, 0);
            inv.m[4] = -(inv.m[0] * m[4] + inv.m[2] * m[5]);
            inv.m[5] = -(inv.m[1] * m[4] + inv.m[3] * m[5]);
            const ImWidgetsTransform2D toRef = inv * it->xf;
            if (r.serial >= doc->refBoxes.Size)
            {
                const int old = doc->refBoxes.Size;
                doc->refBoxes.resize(r.serial + 1);
                for (int k = old; k < doc->refBoxes.Size; ++k) doc->refBoxes[k] = ImRect(ImVec2(FLT_MAX, FLT_MAX), ImVec2(-FLT_MAX, -FLT_MAX));
            }
            const ImVec2 cs[4] = { it->bbox.Min, ImVec2(it->bbox.Max.x, it->bbox.Min.y), it->bbox.Max, ImVec2(it->bbox.Min.x, it->bbox.Max.y) };
            for (int k = 0; k < 4; ++k) doc->refBoxes[r.serial].Add(toRef.Apply(cs[k]));
        }
    }

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

// Glyph outlines of a text item, in screen space (nonzero fill soup).
static bool DW_SvgTextSegments(const DW_SvgItem& it, ImFont* font, const ImWidgetsTransform2D& docToScreen, ImVector<ImVec2>& out)
{
    out.resize(0);
    const ImWidgetsTransform2D m = docToScreen * it.xf;
    // Outlines are produced at the on-screen size so flattening matches pixels.
    const float k = ImSqrt(ImFabs(m.m[0] * m.m[3] - m.m[1] * m.m[2]));
    if (k <= 0.0f || it.style.fontSize <= 0.0f) return false;
    const float sizePx = it.style.fontSize * k;
    ImVector<DW_GlyphOutline> glyphs;
    ImVector<ImVec2> segs;
    ImWidgetsTextDirection const saved = GetStyle().TextDirection;
    GetStyle().TextDirection = it.style.direction;
    bool ok = DW_ShapeTextOutlines(font, sizePx, it.text.Data, it.text.Data + it.text.Size - 1, glyphs, segs);
    GetStyle().TextDirection = saved;
    if (!ok || glyphs.Size == 0) return false;
    float width = 0.0f;
    for (int i = 0; i < glyphs.Size; ++i) width = ImMax(width, glyphs[i].penX + glyphs[i].advance);
    // text-anchor is relative to the writing direction: "start" is the right
    // end of an RTL run.
    int anchor = it.style.textAnchor;
    if (it.style.direction == ImWidgetsTextDirection_RTL) anchor = (anchor == 0) ? 2 : (anchor == 2) ? 0 : 1;
    float ax = anchor == 1 ? -width * 0.5f : anchor == 2 ? -width : 0.0f;
    out.reserve(segs.Size);
    for (int i = 0; i < segs.Size; ++i)
    {
        // local outline units are screen pixels at scale k: back to element units
        ImVec2 l((segs[i].x + ax) / k + it.textPos.x, segs[i].y / k + it.textPos.y);
        out.push_back(m.Apply(l));
    }
    return true;
}

void SvgDraw(ImDrawList* dl, const ImWidgetsSvg* doc, ImVec2 pos, ImVec2 size, ImU32 current_color, ImFont* font)
{
    if (!font) font = ImGui::GetFont();
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
        // clip-path: push every referenced clip (screen space) around the item.
        int pushed = 0;
        bool clippedAway = false;
        for (int c = 0; c < it.clipCount; ++c)
        {
            const DW_SvgClipDef* cd = nullptr;
            for (int k = 0; k < doc->clipDefs.Size; ++k) if (!strcmp(doc->clipDefs[k]->id, it.clips[c].id)) { cd = doc->clipDefs[k]; break; }
            if (!cd) continue;                                   // unknown reference: ignored (as browsers do)
            if (cd->path.Subpaths.Size == 0)
            {
                // Only unsupported children (<use>, <text>): ignore the clip rather
                // than hide the element. A truly empty clipPath hides it (spec).
                if (cd->unsupported) continue;
                clippedAway = true; break;
            }
            // ref space -> clipPath transform -> (objectBoundingBox) bbox mapping
            ImWidgetsTransform2D m = docToScreen * it.clips[c].xf * cd->xf;
            if (cd->bboxUnits)
            {
                const int serial = it.clips[c].serial;
                const ImRect bb = (serial < doc->refBoxes.Size) ? doc->refBoxes[serial] : it.bbox;
                if (bb.GetWidth() <= 0.0f || bb.GetHeight() <= 0.0f) { clippedAway = true; break; }
                m = m * ImWidgetsTransform2D(bb.GetWidth(), 0, 0, bb.GetHeight(), bb.Min.x, bb.Min.y);
            }
            if (DW_PathClipRoom() <= 0) continue; // clip stack full (with the caller's clips): ignored
            ImWidgetsPath cp = cd->path;
            cp.Transform(m);
            PushPathClip(cp, cd->rule);
            ++pushed;
        }
        struct PopGuard { int n; ~PopGuard() { while (n-- > 0) PopPathClip(); } } popGuard = { pushed };
        if (clippedAway) continue;
        if (it.text.Size > 0)
        {
            ImVector<ImVec2> segs;
            if (!DW_SvgTextSegments(it, font, docToScreen, segs)) continue;
            const DW_SvgStyle& st = it.style;
            ImWidgetsPaint paint;
            if (DW_SvgMakePaint(doc, it, st.fill, st.fillOpacity * st.opacity, current_color, docToScreen, paint, stopsBuf))
                DW_FillSegments(dl, segs.Data, segs.Size / 2, paint.Color, ImWidgetsFillRule_NonZero, 1.0f, &paint);
            continue;
        }
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
