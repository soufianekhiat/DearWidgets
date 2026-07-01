// dear_widgets_internal.h
// Cross-translation-unit helpers shared by dear_widgets.cpp and its split
// sibling files (dear_widgets_image_inspector.cpp, dear_widgets_stroke.cpp,
// dear_widgets_vector_drawing.cpp, ...).
//
// These declarations used to live as file-statics inside dear_widgets.cpp
// when the siblings were #include'd inline. Now that each sibling is its
// own translation unit they need external-linkage declarations.
// Not part of the public API -- downstream callers should not include this.

#pragma once

#include "dear_widgets.h"
#include "imgui_internal.h"  // for ImRect used by the expand helpers

// ---------------------------------------------------------------------------
// Internal-only structs (moved from dear_widgets.h)
// ---------------------------------------------------------------------------

struct ImWidgetsMarkerBuffer
{
	ImVec4	fg_color;
	ImVec4	bg_color;

	ImVec2	rotation;
	float	linewidth;
	float	size;

	float	type;
	float	antialiasing;
	float	draw_type;
	float	pad0;
};

struct ImWidgetsEdgeIdx
{
	ImDrawIdx a, b;
	constexpr ImWidgetsEdgeIdx() : a( ( ImDrawIdx )( -1 ) ), b( ( ImDrawIdx )( -1 ) )
	{}
	constexpr ImWidgetsEdgeIdx( ImDrawIdx _a, ImDrawIdx _b ) : a( _a ), b( _b )
	{}
	ImWidgetsEdgeIdx& operator[] ( size_t idx )
	{
		IM_ASSERT( idx == 0 || idx == 1 );
		return ( ( ImWidgetsEdgeIdx* )( void* )( char* )this )[ idx ];
	}
	ImWidgetsEdgeIdx operator[] ( size_t idx ) const
	{
		IM_ASSERT( idx == 0 || idx == 1 );
		return ( ( const ImWidgetsEdgeIdx* )( const void* )( const char* )this )[ idx ];
	}
	bool operator== ( ImWidgetsEdgeIdx e ) const
	{
		return a == e.a && b == e.b;
	}
};

struct ImWidgetsShapeCacheEntry
{
	ImU64				key;
	ImWidgetsShape*		shape;  // Use pointer to avoid shallow copy issues with ImVector
};

struct ImWidgetsShapeCache
{
	ImVector<ImWidgetsShapeCacheEntry>	entries;
	ImVector<int>                       map;    // S-2: open-addressed hash map, slot -> entries index+1 (0=empty)
};

// Shader constant buffer for dashed line rendering.
// Layout must match HLSL cbuffer packing (no field spans a 16-byte register boundary)
// and GLSL std140 layout (vec2 aligned to 8, vec4 aligned to 16).
// Registers: [0-15] p0+p1, [16-31] thickness+aa+dash, [32-47] dash_offset+cap+join+miter_limit,
//            [48-63] rect_min+rect_max, [64-79] color.  Total: 80 bytes.
#define IMGUI_STROKE_MAX_SEGMENTS 1024
struct ImWidgetsDashedLineBuffer
{
    ImVec2  p0;            // offset 0   - segment start (screen space)
    ImVec2  p1;            // offset 8   - segment end   (screen space)
    float   thickness;     // offset 16  - stroke width in pixels
    float   aa;            // offset 20  - aa fringe in pixels
    ImVec2  dash;          // offset 24  - x=dash length, y=gap length (pixels)
    float   dash_offset;   // offset 32  - offset along path (pixels)
    float   cap;           // offset 36  - cap type (ImWidgetsCap_)
    float   join;          // offset 40  - join type (ImWidgetsJoin_)
    float   miter_limit;   // offset 44  - miter limit ratio
    ImVec2  rect_min;      // offset 48  - bounding quad min (screen space)
    ImVec2  rect_max;      // offset 56  - bounding quad max (screen space)
    ImVec4  color;         // offset 64  - RGBA
    // --- Join/cap support (reg 5-6) ---
    ImVec2  prev_dir;      // offset 80  - tangent of previous segment (0,0 = cap at p0)
    ImVec2  next_dir;      // offset 88  - tangent of next segment (0,0 = cap at p1)
    float   seg_start;     // offset 96  - cumulative arc-length at p0
    float   seg_end;       // offset 100 - cumulative arc-length at p1
    float   total_length;  // offset 104 - total polyline length
    float   flags;         // offset 108 - bit flags (encoded as float, decoded
                           //              on GPU via (int)flags): 1=debug joins,
                           //              2=first segment of closed polyline,
                           //              4=last segment of closed polyline.
                           //              Must match lines.hlsl decoding.
};

// Stroke fill constant buffer -- winding number shader.
// Holds directed line segments ("line soup") from Euler spiral stroke expansion.
struct ImWidgetsStrokeBuffer
{
    float   params[4];                        // [0]=num_segs, [1]=unused, [2]=aa_width, [3]=unused
    float   color[4];                         // RGBA
    float   bounds[4];                        // rect_min.x, rect_min.y, rect_max.x, rect_max.y
    float   segments[IMGUI_STROKE_MAX_SEGMENTS * 4]; // p0.x, p0.y, p1.x, p1.y per segment
};

#define ImWidgets_Kibi (1024ull)
#define ImWidgets_Mibi (ImWidgets_Kibi*1024ull)
#define ImWidgets_Gibi (ImWidgets_Mibi*1024ull)
#define ImWidgets_Tebi (ImWidgets_Gibi*1024ull)
#define ImWidgets_Pebi (ImWidgets_Tebi*1024ull)

struct ImGlobalData
{
    ImWidgetsFeatures features;
    bool             dashedLinesUseGPU;
    bool             dashedLinesDebugJoins;
};

namespace ImWidgets { struct ImWidgetsSlugState; } // full definition lives in dear_widgets_slug.cpp

struct ImWidgetsContext
{
	ImTextureID						blackImg; // 4x4 RGBA UInt8 Black Image: { Linear, Clamp }
	ImTextureID						whiteImg; // 4x4 RGBA UInt8 White Image: { Linear, Clamp }
	ImVector<ImTextureID>			ressources;
	ImWidgetsFeatures				features;

	ImDrawShader					markerShader;
	ImDrawShader					lineShader;
	ImDrawShader					slugShader;         // Slug GPU font rendering shader (monochrome glyphs)
	ImDrawShader					slugColorShader;    // Slug GPU font rendering shader (COLR v0 color glyphs)
	ImDrawShader					slugGradientShader; // Slug GPU font gradient shader (COLR v1 linear gradients)
	ImDrawShader					slugDebugShader;    // Slug GPU font debug shader (xcov/ycov/coverage as RGB)
	ImDrawShader					slugFillShader;     // Slug GPU fill gradient shader (user linear/radial/diamond)
	ImWidgets::ImWidgetsSlugState*	slugState;        // Per-context Slug font atlas cache

	// Stroke expansion (Euler spiral, winding-number fill)
	ImDrawShader					strokeShader;

	// Image Inspector: color-managed raw-buffer viewer (uber-shader for 44 sample-type/channel combos)
	ImDrawShader					imageInspectorShader;

	// LookDev Inspector: A/B compare shader with per-side exposure/black/white/gamma
	ImDrawShader					lookDevInspectorShader;

	// dE compare: A/B image difference with false-color ramp
	ImDrawShader					deltaECompareShader;

	// VolumeViewer: Texture3D raymarching (DDA / MIP / iso-surface)
	ImDrawShader					volumeViewerShader;

	// Background blur / effects. One shader variant per effect, compiled lazily
	// (the blur.hlsl source is compiled once per BG_EFFECT value -- see
	// CreateInternalShader's extra_define param). Indexed by ImWidgetsBgEffect.
	ImDrawShader					bgEffectShaders[ ImWidgetsBgEffect_COUNT ];
	ImTextureID						blurBackbufferCopy;
	ImTextureID						blurIntermediate;
	unsigned int					blurTexW, blurTexH;
};

namespace ImWidgets {

// Expand-to-window helpers -- used by widgets that offer a "maximize" button.
// WidgetExpandButton returns a pointer to the widget's persistent expanded-state
// bool (stored in window state storage), or NULL when already inside an
// expanded window (to suppress nested expand buttons).
IMGUI_API bool* WidgetExpandButton( ImGuiID widget_id, ImRect const& bb );
IMGUI_API bool  BeginExpandedWindow( char const* label, ImGuiID widget_id, bool* pOpen,
                                     ImVec2 defaultSize = ImVec2( 600, 500 ) );
IMGUI_API void  EndExpandedWindow();

extern ImGlobalData GlobalData;

//////////////////////////////////////////////////////////////////////////
// Math / Color Helpers (internal -- not part of the public API)
//////////////////////////////////////////////////////////////////////////
inline
float ImCbrt( float x )
{
	return cbrtf( x );
	//return ImPow( x, 1.0f / 3.0f );
}

inline
float ImTan2( float x, float y )
{
	return atan2f( y, x );
}

inline
float ImFract(float x)
{
	return x - ImFloor( x );
}

inline
int LoadShaderFile( size_t* file_data_size, char** file_data, char const* filename )
{
	*file_data_size = 0;
	// Load with 1 padding byte so the buffer is NUL-terminated for APIs expecting C-strings
	*file_data = ( char* )ImFileLoadToMemory( filename, "rb", file_data_size, 1 );
	if ( !*file_data )
		return 0;

	return 1;
}

inline
float ImDot( ImVec4 const& a, ImVec4 const& b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
inline
float ImDot3( ImVec4 const& a, ImVec4 const& b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline
float ImDot3( float* a, float* b )
{
	return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
}
inline
float	ImNormalize01(float const x, float const _min, float const _max)
{
	return ( x - _min ) / ( _max - _min );
}
inline
float	ImScaleFromNormalized(float const x, float const newMin, float const newMax)
{
	return x * ( newMax - newMin ) + newMin;
}
inline
float	ImRescale(float const x, float const _min, float const _max, float const newMin, float const newMax)
{
	return ImScaleFromNormalized( ImNormalize01( x, _min, _max ), newMin, newMax );
}
template < typename Type >
inline
Type	Normalize01(Type const x, Type const _min, Type const _max)
{
	return ( x - _min ) / ( _max - _min );
}
template < typename Type >
inline
Type	ScaleFromNormalized(Type const x, Type const newMin, Type const newMax)
{
	return x * ( newMax - newMin ) + newMin;
}
template < typename Type >
inline
Type	Rescale(Type const x, Type const _min, Type const _max, Type const newMin, Type const newMax)
{
	return ScaleFromNormalized( Normalize01( x, _min, _max ), newMin, newMax );
}

inline
float  ImLengthSqr3( const ImVec4& lhs )
{
	return ( lhs.x * lhs.x ) + ( lhs.y * lhs.y ) + ( lhs.z * lhs.z );
}
inline
float ImLengthL1(ImVec2 v)
{
	return ImAbs( v.x ) + ImAbs( v.y );
}
inline
ImVec2 ImNormalized(ImVec2 v)
{
	return v / ImLength( v );
}
inline
ImVec2 ImHalfTurn(ImVec2 v)
{
	return ImVec2(-v.y, v.x);
}
inline
ImVec2 ImAntiHalfTurn(ImVec2 v)
{
	return ImVec2(v.y, -v.x);
}
inline
float ImLength(ImVec4 v)
{
	return ImSqrt( ImLengthSqr( v ) );
}
inline
float ImLength3(ImVec4 v)
{
	return ImSqrt( ImLengthSqr3( v ) );
}
float	ImLinearSample( float t, float* buffer, int count );
inline
float	ImFunctionFromData( float const x, float const minX, float const maxX, float* data, int const samples_count )
{
	float const t = ImSaturate( ImNormalize01( x, minX, maxX ) );

	return ImLinearSample( t, data, samples_count );
}

inline
float	ImsRGBToLinear( float x )
{
	if ( x <= 0.04045f )
		return x / 12.92f;
	else
		return ImPow( ( x + 0.055f ) / 1.055f, 2.4f);
}
inline
float	ImLinearTosRGB( float x )
{
	if ( x <= 0.0031308f )
		return 12.92f * x;
	else
		return 1.055f * ImPow( x, 1.0f / 2.4f) - 0.055f;
}

// Right-to-Left, like operator '=' or like standard function in C, memcpy, ...
inline
void Mat33RowMajorMulVec3( float& x, float& y, float& z, float* mat33RowMajor, float* vec3 )
{
	x = ImDot3( mat33RowMajor + 0, vec3 );
	y = ImDot3( mat33RowMajor + 3, vec3 );
	z = ImDot3( mat33RowMajor + 6, vec3 );
}

inline
void ImU32ColorToImRGBColor(ImVector<float>& colorsConverted, ImU32* colors, int color_count)
{
	// Linearize from sRGB-encoded 8-bit to linear [0,1] so downstream matrix conversions
	// (RGB->XYZ, chromaticity) are correct; without this the chromaticity line waves.
	ImU32* current = colors;
	colorsConverted.resize( 3 * color_count );
	for ( int k = 0; k < color_count; ++k )
	{
		ImVec4 col = ( ImVec4 )ImColor( *current );
		colorsConverted[ 3 * k + 0 ] = ImsRGBToLinear( col.x );
		colorsConverted[ 3 * k + 1 ] = ImsRGBToLinear( col.y );
		colorsConverted[ 3 * k + 2 ] = ImsRGBToLinear( col.z );
		++current;
	}
}

inline
void ImComputeRect( ImRect* p_bb, ImVec2* pts, int pts_count )
{
	ImRect& bb = *p_bb;
	bb.Min = ImVec2( FLT_MAX, FLT_MAX );
	bb.Max = ImVec2( -FLT_MAX, -FLT_MAX );
	for ( int k = 0; k < pts_count; ++k )
	{
		bb.Min.x = ImMin( bb.Min.x, pts[ k ].x );
		bb.Min.y = ImMin( bb.Min.y, pts[ k ].y );
		bb.Max.x = ImMax( bb.Max.x, pts[ k ].x );
		bb.Max.y = ImMax( bb.Max.y, pts[ k ].y );
	}
}

IMGUI_API ImU32	ImColorFrom_xyz( float x, float y, float z, float* xyzToRGB, float gamma );

//////////////////////////////////////////////////////////////////////////
// Scalar Helpers
//////////////////////////////////////////////////////////////////////////
void	ScaleData( ImGuiDataType data_type, void* p_data, double value );
void	ScaleData( ImGuiDataType data_type, void* p_data, ImU64 value );
bool	IsNegativeScalar( ImGuiDataType data_type, ImU64* src );
bool	IsPositiveScalar( ImGuiDataType data_type, ImU64* src );
void	EqualScalar( ImGuiDataType data_type, ImU64* p_target, ImU64* p_source );
void	SetScalarIndirect( ImGuiDataType data_type, void* p_source, int idx, ImU64* value );
float	ScalarToFloat( ImGuiDataType data_type, ImU64* p_source );
float	ScalarIndirectToFloat( ImGuiDataType data_type, void* p_source, int idx );
ImU64	ScalarIndirectToScalar( ImGuiDataType data_type, void* p_source, int idx );
ImU64	FloatToScalar( ImGuiDataType data_type, float f_value );
ImU64	AddScalar( ImGuiDataType data_type, void* p_a, void* p_b );
ImU64	SubScalar( ImGuiDataType data_type, void* p_a, void* p_b );
ImU64	MulScalar( ImGuiDataType data_type, void* p_a, void* p_b );
ImU64	DivScalar( ImGuiDataType data_type, void* p_a, void* p_b );
ImU64	ClampScalar( ImGuiDataType data_type, void* p_value, void* p_min, void* p_max );
ImU64	Normalize01( ImGuiDataType data_type, void* p_value, void const* p_min, void const* p_max );

//////////////////////////////////////////////////////////////////////////
// Chromaticity (Generic / internal overloads)
//////////////////////////////////////////////////////////////////////////
IMGUI_API
void	DrawChromaticityPlotGeneric( ImDrawList* pDrawList,
									 ImVec2 curPos,
									 ImVec2 size,
									 ImVec2 primR, ImVec2 primG, ImVec2 primB,
									 ImVec2 whitePoint,
									 float* xyzToRGB,
									 int const chromeLineSamplesCount,
									 float* observerX, float* observerY, float* observerZ,
									 int const observerSampleCount,
									 float const observerWavelengthMin, float const observerWavelengthMax,
									 float* standardCIE,
									 int const standardCIESampleCount,
									 float const standardCIEWavelengthMin, float const standardCIEWavelengthMax,
									 float gamma,
									 int resX, int resY,
									 ImU32 maskColor,
									 float wavelengthMin = 400.0f, float wavelengthMax = 700.0f,
									 float minX = 0.0f, float maxX = 0.8f,
									 float minY = 0.0f, float maxY = 0.9f,
									 bool showColorSpaceTriangle = true,
									 bool showWhitePoint = true,
									 bool showBorder = true,
									 ImU32 borderColor = IM_COL32( 0, 0, 0, 255 ),
									 float borderThickness = 1.0f );
IMGUI_API
void	DrawChromaticityPointsGeneric( ImDrawList* pDrawList,
									   ImVec2 curPos,
									   ImVec2 size,
									   float* rgbToXYZ,
									   float* colors4, // AoS
									   int color_count,
									   float minX, float maxX,
									   float minY, float maxY,
									   ImU32 plotColor, float radius, int num_segments,
									   int colorStride = 4 ); // 4 for rgba,rgba,rgba,...; 3 for rgb,rgb,rgb,... or anything else
IMGUI_API
void	DrawChromaticityLinesGeneric( ImDrawList* pDrawList,
									  ImVec2 curPos,
									  ImVec2 size,
									  float* rgbToXYZ,
									  float* colors4, // AoS
									  int color_count,
									  float minX, float maxX,
									  float minY, float maxY,
									  ImU32 plotColor, ImDrawFlags flags, float thickness,
									  int colorStride = 4 ); // 4 for rgba,rgba,rgba,...; 3 for rgb,rgb,rgb,... or anything else )

// ── Slug lifecycle helpers (defined in dear_widgets_slug.cpp) ────────────────
void SlugDestroyState(ImWidgetsSlugState* state);
void SlugDebugDrawCmdCallback(ImDrawList* overlay, const ImDrawList* draw_list,
                              const ImDrawCmd* cmd, bool show_mesh, bool show_aabb,
                              char* out_text, int text_size);

// ── Shared state from dear_widgets.cpp ───────────────────────────────────────
extern ImWidgetsContext* gs_pContext;
extern bool g_SlugDebugShader;

// ── Slug rendering helpers called by dear_widgets_latex.cpp ──────────────────
ImVec2 CalcTextSize_Impl(ImFont* font, float font_size, const char* text,
                         const char* text_end = nullptr, float* out_ascent = nullptr);
void   DrawText_Impl(ImDrawList* pDrawList, ImFont* font, float font_size,
                     ImVec2 pos, ImU32 col, const char* text, const char* text_end = nullptr);
void   TesselateText_Impl(ImFont* font, float font_size, const char* text,
                          ImWidgetsShape& outShape, const char* text_end,
                          float tess_tol, int iterations);
float  SlugLpToPx(float lp);

// ── Lp ↔ Px unit conversion wrappers ────────────────────────────────────────
// All DearWidgets internal code uses these instead of calling ImPlatform directly.
// When ImPlatform is available, delegates to it; otherwise replicates the same
// FontScaleDpi formula that SlugLpToPx uses.
#ifdef IMPLATFORM_API
inline float  LpToPx(float  lp) { return ImPlatform_LpToPx(lp); }
inline float  PxToLp(float  px) { return ImPlatform_PxToLp(px); }
inline ImVec2 LpToPx(ImVec2 lp) { return ImPlatform_LpToPx(lp); }
inline ImVec2 PxToLp(ImVec2 px) { return ImPlatform_PxToLp(px); }
#else
inline float  LpToPx(float  lp) { return lp * ImGui::GetStyle().FontScaleDpi; }
inline float  PxToLp(float  px) { float s = ImGui::GetStyle().FontScaleDpi; return s > 1e-3f ? px / s : px; }
inline ImVec2 LpToPx(ImVec2 lp) { float s = ImGui::GetStyle().FontScaleDpi; return ImVec2(lp.x * s, lp.y * s); }
inline ImVec2 PxToLp(ImVec2 px) { float s = ImGui::GetStyle().FontScaleDpi; return s > 1e-3f ? ImVec2(px.x / s, px.y / s) : px; }
#endif

// Catmull-Rom -> cubic Bezier path. Produces 3*(n-1)+1 control points so
// DrawStrokedBezierPath / DrawStrokedDashedBezierPath can consume it directly.
// Endpoints are clamped (P-1 = P0, Pn = Pn-1) so the curve still passes through them.
//   B0 = Pi
//   B1 = Pi   + (Pi+1 - Pi-1) / 6
//   B2 = Pi+1 - (Pi+2 - Pi)   / 6
//   B3 = Pi+1
// Adjacent segments share endpoints, so we emit B0 once at the start and only
// (B1, B2, B3) per segment afterward.
inline void CatmullRomToCubicBezierPath(ImVec2 const* pts, int n, ImVector<ImVec2>& out)
{
    if (n < 2) { out.clear(); return; }
    out.resize(3 * (n - 1) + 1);
    out[0] = pts[0];
    int w = 1;
    for (int i = 0; i < n - 1; ++i)
    {
        ImVec2 const Pm1 = (i == 0)     ? pts[i]   : pts[i - 1];
        ImVec2 const P0  = pts[i];
        ImVec2 const P1  = pts[i + 1];
        ImVec2 const P2  = (i + 2 < n)  ? pts[i + 2] : pts[i + 1];
        ImVec2 const B1(P0.x + (P1.x - Pm1.x) / 6.0f, P0.y + (P1.y - Pm1.y) / 6.0f);
        ImVec2 const B2(P1.x - (P2.x - P0.x)  / 6.0f, P1.y - (P2.y - P0.y)  / 6.0f);
        out[w++] = B1;
        out[w++] = B2;
        out[w++] = P1;
    }
}

// Force a fresh ImDrawCmd whose VtxOffset matches the current VtxBuffer tail.
//
// Some widgets (or some upstream sequence of widgets) end up with
// _CmdHeader.VtxOffset stuck at 0 while VtxBuffer.Size has grown well past 64K.
// When that happens the 16-bit indices we emit afterwards wrap inside the
// [0, 65535] window and resolve to old, unrelated vertices — visually the new
// geometry disappears or smears into garbage. Calling this at the top of a
// widget's draw block guarantees the cmd we're about to write into has a
// correct VtxOffset, so our indices are interpreted correctly regardless of
// upstream state.
//
// We deliberately avoid _OnChangedVtxOffset (asserts when the current cmd has
// a UserCallback). Manually bumping _CmdHeader.VtxOffset + resetting
// _VtxCurrentIdx + AddDrawCmd gives a fresh cmd that inherits the bumped
// offset, which is the same effect minus the assert hazard.
inline void DW_EnsureFreshVtxOffset(ImDrawList* dl)
{
    if (!dl) return;
    if (!(dl->Flags & ImDrawListFlags_AllowVtxOffset)) return;
    if (dl->VtxBuffer.Size <= 0) return;
    if ((int)dl->_CmdHeader.VtxOffset >= dl->VtxBuffer.Size) return;
    dl->_CmdHeader.VtxOffset = (unsigned)dl->VtxBuffer.Size;
    dl->_VtxCurrentIdx = 0;
    dl->AddDrawCmd();
}

} // namespace ImWidgets
