#pragma once
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>
// imgui_internal.h is currently required by the public header for a small surface
// of by-value internal types: ImRect (function params), ImGuiButtonFlags,
// ImGuiNavRenderCursorFlags, ImGuiItemFlags. ImWidgetsAABB is now used in public
// struct members and inline ImTrunc was replaced with truncf, so the surface is
// shrinking â€” but these particular signatures still pull it in. Removing it
// fully requires either replacing the param types or a typedef-only shim header.
#include <imgui_internal.h>
#include <math.h>

//-----------------------------------------------------------------------------
// [SECTION] Shape Caching Configuration
//-----------------------------------------------------------------------------
// Enable shape caching to improve performance by caching generated geometry
// Shapes cache only positions and topology - UVs are set fresh each frame
// This avoids font atlas timing issues and keeps cache simple
//#define DEAR_WIDGETS_SHAPE_CACHING // Done by the build system cf. Common.cs

// Map project-specific graphics API defines to ImPlatform defines
// This must be done BEFORE including ImPlatform.h so it can properly set IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
#if defined(__DEAR_GFX_DX9__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_DIRECTX9
#elif defined(__DEAR_GFX_DX10__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_DIRECTX10
#elif defined(__DEAR_GFX_DX11__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_DIRECTX11
#elif defined(__DEAR_GFX_DX12__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_DIRECTX12
#elif defined(__DEAR_GFX_OGL3__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_OPENGL3
#elif defined(__DEAR_GFX_VULKAN__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_VULKAN
#endif

#include <ImPlatform.h>

// Note: top-of-file TODO blocks have been removed. Open work items live in the
// project issue tracker; bugfix-tier items are tracked as `// FIXME` comments
// adjacent to the relevant declarations.

// Compatibility shim for pre-C++17 compilers: `constexpr if` -> regular branches.
// Drop this when the minimum supported C++ standard is bumped to C++17.
#if !__cpp_if_constexpr
#define constexpr
#endif

// ImPlatform shader types
struct ImDrawShader
{
	ImPlatform_Shader vs; // Vertex shader handle
	ImPlatform_Shader ps; // Pixel shader handle
	ImPlatform_ShaderProgram program; // Shader program handle
	// Note: New ImPlatform API handles constant buffers internally via ImPlatform_SetShaderUniform
	// The old vs_cst/ps_cst/cpu_*_data fields are no longer needed
};

enum ImWidgetsFeatures_
{
	ImWidgetsFeatures_None     = 0,
	ImWidgetsFeatures_Markers  = 1 << 0,
	ImWidgetsFeatures_RichFont = 1 << 1,  // Slug GPU font rendering (color fonts, gradients, ligatures)
	ImWidgetsFeatures_LaTeX    = 1 << 2,  // LaTeX math rendering via Slug

	ImWidgetsFeatures_COUNT
};
typedef int ImWidgetsFeatures;

struct ImWidgetsVertex
{
	ImVec2 pos;
	ImVec2 uv;
	ImU32 col;
};
struct ImWidgetsVertexLine
{
	ImVec2 pos;
	ImVec4 tangent;
	ImVec2 segment;
	ImVec2 uv;
	ImVec2 angle;
	ImU32 col;
};
struct ImWidgetsTriIdx
{
	ImDrawIdx a, b, c;
	constexpr ImWidgetsTriIdx() : a( ( ImDrawIdx )( -1 ) ), b( ( ImDrawIdx )( -1 ) ), c( ( ImDrawIdx )( -1 ) )
	{}
	constexpr ImWidgetsTriIdx( ImDrawIdx _a, ImDrawIdx _b, ImDrawIdx _c ) : a( _a ), b( _b ), c( _c )
	{}
	ImDrawIdx& operator[] ( size_t idx )
	{
		IM_ASSERT( idx == 0 || idx == 1 || idx == 2 );
		return ( ( ImDrawIdx* )( void* )( char* )this )[ idx ];
	}
	ImDrawIdx operator[] ( size_t idx ) const
	{
		IM_ASSERT( idx == 0 || idx == 1 || idx == 2 );
		return ( ( ImDrawIdx const* )( void const* )( char const* )this )[ idx ];
	}
	ImWidgetsTriIdx& operator= ( ImWidgetsTriIdx const& rhs )
	{
		a = rhs.a;
		b = rhs.b;
		c = rhs.c;

		return *this;
	}
};

struct ImWidgetsShape
{
	ImVector<ImWidgetsVertex>	vertices;
	ImVector<ImWidgetsTriIdx>	triangles;
	ImRect						bb;
};
struct ImWidgetsShapeLine
{
	ImVector<ImWidgetsVertexLine>	vertices;
	ImVector<ImWidgetsTriIdx>		triangles;
	float							total_length;
	ImRect							bb;
};

typedef ImU32( *ImWidgetsColor1DCallback )( float x, void* );
typedef ImU32( *ImWidgetsColor2DCallback )( float x, float y, void* );

// Opaque Slug font rendering state (defined in dear_widgets.cpp, inside namespace ImWidgets)
namespace ImWidgets { struct ImWidgetsSlugState; }

struct ImWidgetsContext; // Opaque context -- full definition in dear_widgets_internal.h

enum ImWidgetsStyleColor
{
	StyleColor_Slider2D_CursorX,    // Color for Slider2D X-axis cursor
	StyleColor_Slider2D_CursorY,    // Color for Slider2D Y-axis cursor

	// Slider2DRange
	StyleColor_Slider2DRange_MinHandle,  // Min-corner handle
	StyleColor_Slider2DRange_MaxHandle,  // Max-corner handle
	StyleColor_Slider2DRange_Fill,       // Selection rectangle fill

	// Slider2DDisc
	StyleColor_Slider2DDisc_Background,  // Disc fill (transparent = use FrameBg)
	StyleColor_Slider2DDisc_Cursor,      // Handle dot
	StyleColor_Slider2DDisc_CursorOutline,  // Handle outline
	StyleColor_Slider2DDisc_Ring,        // Outer ring
	StyleColor_Slider2DDisc_Grid,        // Crosshair and concentric rings

	// SliderRing
	StyleColor_SliderRing_Track,           // Track arc color
	StyleColor_SliderRing_TrackActive,     // Track arc fill from min to current value
	StyleColor_SliderRing_Grab,            // Grab handle fill
	StyleColor_SliderRing_GrabActive,      // Grab handle fill while dragging

	// SliderSpline
	StyleColor_SliderSpline_Track,         // Spline track color
	StyleColor_SliderSpline_TrackActive,   // Spline track fill from min to current value
	StyleColor_SliderSpline_Grab,          // Grab handle fill
	StyleColor_SliderSpline_GrabActive,    // Grab handle fill while dragging

	// Gradient Editor
	StyleColor_Gradient_MarkerOutline,
	StyleColor_Gradient_MarkerOutlineHovered,
	StyleColor_Gradient_MarkerOutlineSelected,
	StyleColor_Gradient_AlphaIndicator,
	StyleColor_Gradient_Checkerboard1,
	StyleColor_Gradient_Checkerboard2,

	// Curve Editor
	StyleColor_CurveEditor_Line,
	StyleColor_CurveEditor_GridMinor,
	StyleColor_CurveEditor_GridMajor,
	StyleColor_CurveEditor_ZeroLine,
	StyleColor_CurveEditor_Key,
	StyleColor_CurveEditor_KeyHovered,
	StyleColor_CurveEditor_KeySelected,
	StyleColor_CurveEditor_KeyOutline,
	StyleColor_CurveEditor_KeyOutlineHovered,
	StyleColor_CurveEditor_KeyOutlineSelected,
	StyleColor_CurveEditor_TangentLine,
	StyleColor_CurveEditor_TangentHovered,
	StyleColor_CurveEditor_Crosshair,
	StyleColor_CurveEditor_AddIndicator,
	StyleColor_CurveEditor_AxisLabel,

	// Color Wheel
	StyleColor_ColorWheel_DotOutline,
	StyleColor_ColorWheel_DotOutlineActive,
	StyleColor_ColorWheel_Crosshair,
	StyleColor_ColorWheel_SliderOutline,

	// Color Warper
	StyleColor_ColorWarper_GridLine,
	StyleColor_ColorWarper_GridLineHovered,
	StyleColor_ColorWarper_PointOutline,
	StyleColor_ColorWarper_PointOutlineActive,
	StyleColor_ColorWarper_PointFill,
	StyleColor_ColorWarper_PointFillMoved,
	StyleColor_ColorWarper_Crosshair,

	// Color Curve
	StyleColor_ColorCurve_Background,
	StyleColor_ColorCurve_Grid,
	StyleColor_ColorCurve_Line,
	StyleColor_ColorCurve_NeutralLine,
	StyleColor_ColorCurve_Key,
	StyleColor_ColorCurve_KeyHovered,
	StyleColor_ColorCurve_KeySelected,
	StyleColor_ColorCurve_KeyOutline,
	StyleColor_ColorCurve_Crosshair,
	StyleColor_ColorCurve_HistogramOverlay,

	// Parade Scope
	StyleColor_ParadeScope_Background,
	StyleColor_ParadeScope_Grid,
	StyleColor_ParadeScope_ChannelR,
	StyleColor_ParadeScope_ChannelG,
	StyleColor_ParadeScope_ChannelB,
	StyleColor_ParadeScope_ChannelLuma,
	StyleColor_ParadeScope_ChannelCb,
	StyleColor_ParadeScope_ChannelCr,
	StyleColor_ParadeScope_GradTick,
	StyleColor_ParadeScope_GradLabel,

	// Vector Scope
	StyleColor_VectorScope_Background,
	StyleColor_VectorScope_Grid,
	StyleColor_VectorScope_Graticule,
	StyleColor_VectorScope_Signal,
	StyleColor_VectorScope_SkinToneLine,
	StyleColor_VectorScope_TargetR,
	StyleColor_VectorScope_TargetG,
	StyleColor_VectorScope_TargetB,
	StyleColor_VectorScope_TargetCy,
	StyleColor_VectorScope_TargetMg,
	StyleColor_VectorScope_TargetYl,

	// Histogram
	StyleColor_Histogram_Background,
	StyleColor_Histogram_Grid,
	StyleColor_Histogram_ChannelR,
	StyleColor_Histogram_ChannelG,
	StyleColor_Histogram_ChannelB,
	StyleColor_Histogram_ChannelLuma,
	StyleColor_Histogram_ChannelCb,
	StyleColor_Histogram_ChannelCr,
	StyleColor_Histogram_ChannelH,
	StyleColor_Histogram_ChannelS,
	StyleColor_Histogram_ChannelV,
	StyleColor_Histogram_ChannelOkL,
	StyleColor_Histogram_ChannelOkC,
	StyleColor_Histogram_ChannelOkH,
	StyleColor_Histogram_GradTick,
	StyleColor_Histogram_GradLabel,

	// CIE Chromaticity
	StyleColor_CIEChromaticity_Background,
	StyleColor_CIEChromaticity_Grid,
	StyleColor_CIEChromaticity_Signal,
	StyleColor_CIEChromaticity_GamutLine,
	StyleColor_CIEChromaticity_WhitePoint,
	StyleColor_CIEChromaticity_PrimaryR,
	StyleColor_CIEChromaticity_PrimaryG,
	StyleColor_CIEChromaticity_PrimaryB,
	StyleColor_CIEChromaticity_GradTick,
	StyleColor_CIEChromaticity_GradLabel,

	// Chromaticity Plot / Line / Point (standalone helpers)
	StyleColor_ChromaticityPlot_Triangle,       // Color space triangle outline
	StyleColor_ChromaticityPlot_WhitePoint,     // White point marker fill
	StyleColor_ChromaticityLine_Default,        // Default fallback line color
	StyleColor_ChromaticityPoint_Default,       // Default fallback point color

	// Hue Selector
	StyleColor_HueSelector_ZeroWidthLine,       // Vertical line when width=0
	StyleColor_HueSelector_Cursor,              // Triangle cursor fill

	// Tone Curve
	StyleColor_ToneCurve_Background,
	StyleColor_ToneCurve_Grid,
	StyleColor_ToneCurve_NeutralLine,
	StyleColor_ToneCurve_Key,
	StyleColor_ToneCurve_KeySelected,
	StyleColor_ToneCurve_KeyOutline,
	StyleColor_ToneCurve_Crosshair,
	StyleColor_ToneCurve_HistogramOverlay,
	StyleColor_ToneCurve_GradTick,
	StyleColor_ToneCurve_GradLabel,

	// HDR Wheel
	StyleColor_HDRWheel_RightArc,			// Right arc fill color (default red)
	StyleColor_HDRWheel_LeftArcMin,			// Left arc gradient start color (black)
	StyleColor_HDRWheel_LeftArcMax,			// Left arc gradient end color (white)

	// Transform Gizmo
	StyleColor_Gizmo_Canvas,				// Canvas background
	StyleColor_Gizmo_Outline,				// Bounding box outline
	StyleColor_Gizmo_Handle,				// Handle fill
	StyleColor_Gizmo_HandleActive,			// Handle fill when active
	StyleColor_Gizmo_DimOutline,			// Unselected image outline (low alpha)

	// Color Picker
	StyleColor_ColorPicker_DotOutline,		// Dot outline (white)
	StyleColor_ColorPicker_DotOutlineActive,// Dot outline when active (yellow)
	StyleColor_ColorPicker_Crosshair,		// Crosshair lines (white, low alpha)
	StyleColor_ColorPicker_SliderOutline,	// Vertical slider outline (black, medium alpha)
	StyleColor_ColorPicker_SliderHandle,	// Vertical slider handle (white)

	StyleColor_Count
};

enum ImWidgetsStyleVar
{
	StyleVar_HueSelector_Thickness_ZeroWidth,

	// Slider2D
	StyleVar_Slider2D_DragThickness,
	StyleVar_Slider2D_BorderThickness,
	StyleVar_Slider2D_LineThickness,
	StyleVar_Slider2D_CursorRadius,
	StyleVar_Slider2D_CursorOffset,
	StyleVar_Slider2D_CornerRadius,

	// Slider2DRange
	StyleVar_Slider2DRange_FillAlpha,
	StyleVar_Slider2DRange_HandleRadius,

	// Slider2DDisc
	StyleVar_Slider2DDisc_CursorRadius,
	StyleVar_Slider2DDisc_CursorOutlineThickness,
	StyleVar_Slider2DDisc_RingThickness,
	// (Slider2DDisc_GridRings is an int count â€” accessed directly, not via PushStyleVar)

	// SliderRing
	StyleVar_SliderRing_TrackThickness,
	StyleVar_SliderRing_GrabRadius,

	// SliderSpline
	StyleVar_SliderSpline_TrackThickness,
	StyleVar_SliderSpline_GrabRadius,

	// General
	StyleVar_NavCursor_Thickness,
	StyleVar_NavCursor_Distance,
	StyleVar_WhitePoint_Radius,
	StyleVar_PrecisionDrag_BlockSize,

	// Gradient Editor
	StyleVar_Gradient_MarkerHeight,
	StyleVar_Gradient_CheckerboardCellSize,
	StyleVar_Gradient_MarkerThickness,

	// Curve Editor
	StyleVar_CurveEditor_DefaultHeight,
	StyleVar_CurveEditor_KeyRadius,
	StyleVar_CurveEditor_TangentRadius,
	StyleVar_CurveEditor_HitRadius,
	StyleVar_CurveEditor_LineThickness,
	StyleVar_CurveEditor_KeyOutlineThickness,
	StyleVar_CurveEditor_ZeroLineThickness,

	// Color Wheel
	StyleVar_ColorWheel_DotRadius,
	StyleVar_ColorWheel_RingThickness,
	// (ColorWheel_DiscSectors / DiscRings are int counts â€” accessed directly, not via PushStyleVar)
	StyleVar_ColorWheel_SliderHeight,

	// Color Warper
	StyleVar_ColorWarper_PointRadius,
	StyleVar_ColorWarper_HitRadius,
	StyleVar_ColorWarper_GridThickness,
	// (ColorWarper_DiscSectors / DiscRings are int counts â€” accessed directly, not via PushStyleVar)
	StyleVar_ColorWarper_DiscScale,

	// Color Curve (Hue vs, Lum vs, Sat vs)
	StyleVar_ColorCurve_DefaultHeight,
	StyleVar_ColorCurve_KeyRadius,
	StyleVar_ColorCurve_LineThickness,

	// Parade Scope
	StyleVar_ParadeScope_DefaultHeight,
	StyleVar_ParadeScope_OverlayAlpha,
	StyleVar_ParadeScope_GradTickLength,
	StyleVar_ParadeScope_GradTickThickness,
	StyleVar_ParadeScope_GradMargin,

	// Vector Scope
	StyleVar_VectorScope_DefaultSize,
	StyleVar_VectorScope_SignalAlpha,
	StyleVar_VectorScope_GraticuleThickness,
	StyleVar_VectorScope_DiscScale,

	// Histogram
	StyleVar_Histogram_DefaultHeight,
	StyleVar_Histogram_OverlayAlpha,
	StyleVar_Histogram_GradTickLength,
	StyleVar_Histogram_GradTickThickness,
	StyleVar_Histogram_GradMarginLeft,
	StyleVar_Histogram_GradMarginBottom,

	// CIE Chromaticity
	StyleVar_CIEChromaticity_DefaultSize,
	StyleVar_CIEChromaticity_SignalRadius,
	StyleVar_CIEChromaticity_GamutLineThickness,
	StyleVar_CIEChromaticity_WhitePointRadius,
	StyleVar_CIEChromaticity_GradMargin,
	StyleVar_CIEChromaticity_SignalAlpha,
	StyleVar_CIEChromaticity_GradMarginRight,
	StyleVar_CIEChromaticity_GradMarginTop,

	// Chromaticity Plot / Line / Point
	// (ChromaticityPlot_Resolution / LineSamples are int counts â€” accessed directly, not via PushStyleVar)
	StyleVar_ChromaticityPlot_BorderThickness,  // Horseshoe outline thickness (lp)
	StyleVar_ChromaticityPlot_TriangleThickness,// Color space triangle thickness (lp)
	StyleVar_ChromaticityLine_Thickness,        // Polyline thickness (lp)
	StyleVar_ChromaticityPoint_Radius,          // Point radius (lp)
	// (ChromaticityPoint_Segments is an int count â€” accessed directly, not via PushStyleVar)

	// Tone Curve
	StyleVar_ToneCurve_DefaultHeight,
	StyleVar_ToneCurve_KeyRadius,
	StyleVar_ToneCurve_LineThickness,
	StyleVar_ToneCurve_GradMarginLeft,
	StyleVar_ToneCurve_GradMarginBottom,
	StyleVar_ToneCurve_BandThickness,
	StyleVar_ToneCurve_BandGap,
	StyleVar_ToneCurve_GradMarginRight,
	StyleVar_ToneCurve_GradMarginTop,

	// HDR Wheel
	StyleVar_HDRWheel_RingThickness,
	StyleVar_HDRWheel_RingGap,

	// Transform Gizmo
	StyleVar_Gizmo_HandleSize,
	StyleVar_Gizmo_RotationHandleOffset,
	StyleVar_Gizmo_OutlineThickness,
	StyleVar_Gizmo_RotationLineThickness,
	StyleVar_Gizmo_CenterDotRadius,
	StyleVar_Gizmo_EdgeHandleScale,
	StyleVar_Gizmo_DefaultCanvasHeight,

	// Color Picker
	StyleVar_ColorPicker_DotRadius,
	// (ColorPicker_PlaneResolution is an int count â€” accessed directly, not via PushStyleVar)
	StyleVar_ColorPicker_SliderWidth,
	// (ColorPicker_SliderResolution is an int count â€” accessed directly, not via PushStyleVar)
	StyleVar_ColorPicker_ComponentSliderHeight,
	StyleVar_ColorPicker_DotOutlineThickness,
	StyleVar_ColorPicker_SliderHandleHeight,
	StyleVar_ColorPicker_ComponentHandleWidth,

	// Hatching
	StyleVar_Hatch_DefaultSpacing,
	StyleVar_Hatch_DefaultThickness,
	// Bezier curve editor
	StyleVar_BezierCurve_KeyRadius,
	StyleVar_BezierCurve_TangentRadius,
	StyleVar_BezierCurve_LineThickness,
	StyleVar_BezierCurve_SnapAngleDeg,
	// Font inspector
	StyleVar_FontInspector_GlyphCell,
	StyleVar_FontInspector_MetricAlpha,
	// Notched dial
	StyleVar_NotchedDial_TickLength,
	StyleVar_NotchedDial_TickThickness,
	// Delta-E
	StyleVar_DeltaE_SwatchSize,
	// Equation input
	StyleVar_Equation_BlockPadding,
	StyleVar_Equation_InlineBaselineOffset,
	// Iso-contour tiers
	StyleVar_IsoContour_MajorThickness,
	StyleVar_IsoContour_MediumThickness,
	StyleVar_IsoContour_MinorThickness,
	// LookDev
	StyleVar_LookDev_DividerThickness,
	StyleVar_LookDev_HandleRadius,
	// Volume slice viewer
	StyleVar_VolumeSlice_HandleRadius,
	// Vector drawing tool
	StyleVar_VectorDrawing_AnchorRadius,
	StyleVar_VectorDrawing_TangentRadius,
	StyleVar_VectorDrawing_GridSpacing,
	StyleVar_VectorDrawing_TangentLineThickness,
	StyleVar_VectorDrawing_AnchorOutlineThickness,
	StyleVar_VectorDrawing_ClosingRingRadius,
	StyleVar_VectorDrawing_ClosingRingThickness,
	StyleVar_VectorDrawing_HoverRingGap,
	StyleVar_VectorDrawing_HoverRingThickness,
	StyleVar_VectorDrawing_GhostRingThickness,
	StyleVar_VectorDrawing_CrosshairArmLength,
	StyleVar_VectorDrawing_CrosshairThickness,
	// Precision popup input width
	StyleVar_PrecisionPopup_InputWidth,

	StyleVar_Count
};

// Unified line-rendering mode for DrawThickLine() â€” declared early so ImWidgetsStyle's
// constructor can reference enum values. Every mode exists in every build; the only
// mode that depends on the GPU custom-shader path is PolylineAA. When that path is
// not compiled in (IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER undefined) PolylineAA silently
// renders as AddPolyline at runtime â€” the caller-visible enum is unchanged.
enum ImWidgetsThickLineMode_
{
	ImWidgetsThickLineMode_AddPolyline,         // ImGui default polyline
	ImWidgetsThickLineMode_PolylineAA,          // Rougier SDF; silent runtime fallback to AddPolyline if shader unavailable
	ImWidgetsThickLineMode_StrokedPolyline,     // Euler spirals on raw polyline (via "stroke" shader; CPU polyline fallback if unavailable)
	ImWidgetsThickLineMode_StrokedBezierPath,   // Euler spirals on Catmull-Rom -> cubic path (via "stroke" shader; CPU polyline fallback if unavailable)
	ImWidgetsThickLineMode_ImGuiBezier,         // ImGui AddBezierCubic per Catmull-Rom segment (always available)

	ImWidgetsThickLineMode_COUNT
};
typedef int ImWidgetsThickLineMode;

struct ImWidgetsStyle
{
	float	HueSelector_Thickness_ZeroWidth;

	// Slider2D
	float	Slider2D_DragThickness;
	float	Slider2D_BorderThickness;
	float	Slider2D_LineThickness;
	float	Slider2D_CursorRadius;
	float	Slider2D_CursorOffset;
	float	Slider2D_CornerRadius;

	// Slider2DRange
	float	Slider2DRange_FillAlpha;
	float	Slider2DRange_HandleRadius;     // corner handle size (lp)

	// Slider2DDisc
	float	Slider2DDisc_CursorRadius;
	float	Slider2DDisc_CursorOutlineThickness;
	float	Slider2DDisc_RingThickness;
	int		Slider2DDisc_GridRings;             // Number of concentric grid rings (count)

	// SliderRing
	float	SliderRing_TrackThickness;			// Track arc thickness (lp)
	float	SliderRing_GrabRadius;				// Grab handle radius (lp)

	// SliderSpline
	float	SliderSpline_TrackThickness;		// Spline track thickness (lp)
	float	SliderSpline_GrabRadius;			// Grab handle radius (lp)

	// General
	float	NavCursor_Thickness;
	float	NavCursor_Distance;
	float	WhitePoint_Radius;
	float	PrecisionDrag_BlockSize;

	// Gradient Editor
	float	Gradient_MarkerHeight;
	float	Gradient_CheckerboardCellSize;
	float	Gradient_MarkerThickness;

	// Curve Editor
	float	CurveEditor_DefaultHeight;
	float	CurveEditor_KeyRadius;
	float	CurveEditor_TangentRadius;
	float	CurveEditor_HitRadius;
	float	CurveEditor_LineThickness;
	float	CurveEditor_KeyOutlineThickness;
	float	CurveEditor_ZeroLineThickness;
	ImWidgetsThickLineMode	CurveEditor_LineMode;

	// Color Wheel
	float	ColorWheel_DotRadius;
	float	ColorWheel_RingThickness;
	int		ColorWheel_DiscSectors;				// Disc angular resolution (count)
	int		ColorWheel_DiscRings;				// Disc radial resolution (count)
	float	ColorWheel_SliderHeight;			// Master slider height (lp)

	// Color Warper
	float	ColorWarper_PointRadius;			// Control point dot radius (lp)
	float	ColorWarper_HitRadius;				// Hit testing radius for points (lp)
	float	ColorWarper_GridThickness;			// Mesh grid line thickness (lp)
	int		ColorWarper_DiscSectors;			// Background disc angular resolution (count)
	int		ColorWarper_DiscRings;				// Background disc radial resolution (count)
	float	ColorWarper_DiscScale;				// Disc radius as fraction of half-side (0..1)

	// Color Curve
	float	ColorCurve_DefaultHeight;
	float	ColorCurve_KeyRadius;
	float	ColorCurve_LineThickness;
	ImWidgetsThickLineMode	ColorCurve_LineMode;

	// Parade Scope
	float	ParadeScope_DefaultHeight;
	float	ParadeScope_OverlayAlpha;		// Max alpha in overlay mode (0..1)
	float	ParadeScope_GradTickLength;		// Graduation tick mark length (lp)
	float	ParadeScope_GradTickThickness;	// Graduation tick line thickness
	float	ParadeScope_GradMargin;			// Left margin for graduation labels (lp)

	// Vector Scope
	float	VectorScope_DefaultSize;		// Default square side length (lp)
	float	VectorScope_SignalAlpha;		// Max alpha for signal dots (0..1)
	float	VectorScope_GraticuleThickness;	// Graticule line thickness (lp)
	float	VectorScope_DiscScale;			// Disc/graticule radius as fraction of half-side (0..1)

	// Histogram
	float	Histogram_DefaultHeight;		// Default widget height (lp)
	float	Histogram_OverlayAlpha;			// Max alpha in overlay mode (0..1)
	float	Histogram_GradTickLength;		// Graduation tick mark length (lp)
	float	Histogram_GradTickThickness;	// Graduation tick line thickness
	float	Histogram_GradMarginLeft;		// Left margin for Y-axis labels (lp)
	float	Histogram_GradMarginBottom;		// Bottom margin for X-axis labels (lp)

	// CIE Chromaticity
	float	CIEChromaticity_DefaultSize;		// Default square side (lp)
	float	CIEChromaticity_SignalRadius;		// Signal dot radius (lp)
	float	CIEChromaticity_GamutLineThickness;	// Gamut triangle line thickness (lp)
	float	CIEChromaticity_WhitePointRadius;	// White point marker radius (lp)
	float	CIEChromaticity_GradMargin;			// Left/bottom margin for graduation labels (lp)
	float	CIEChromaticity_SignalAlpha;		// Signal point alpha (0..1)
	float	CIEChromaticity_GradMarginRight;	// Right margin (lp)
	float	CIEChromaticity_GradMarginTop;		// Top margin (lp)

	// Chromaticity Plot / Line / Point (standalone helpers)
	int		ChromaticityPlot_Resolution;        // Procedural fill resolution per axis (count)
	int		ChromaticityPlot_LineSamples;       // Horseshoe sample count
	float	ChromaticityPlot_BorderThickness;   // Horseshoe outline thickness (lp)
	float	ChromaticityPlot_TriangleThickness; // Color space triangle thickness (lp)
	float	ChromaticityLine_Thickness;         // Polyline thickness (lp)
	float	ChromaticityPoint_Radius;           // Point radius (lp)
	int		ChromaticityPoint_Segments;         // Point circle segment count
	ImWidgetsThickLineMode	ChromaticityLine_Mode;
	int		ChromaticityLine_SmoothIterations;  // binomial smoothing passes on the locus polyline (0 = raw data)

	// Tone Curve
	float	ToneCurve_DefaultHeight;			// Default widget height (lp)
	float	ToneCurve_KeyRadius;				// Key dot radius (lp)
	float	ToneCurve_LineThickness;			// Curve line thickness (lp)
	float	ToneCurve_GradMarginLeft;			// Left margin for Y-axis labels (lp)
	float	ToneCurve_GradMarginBottom;			// Bottom margin for X-axis labels (lp)
	float	ToneCurve_BandThickness;			// Gradient band thickness (lp)
	float	ToneCurve_BandGap;					// Gap between scope and band (lp)
	float	ToneCurve_GradMarginRight;			// Right margin (lp)
	float	ToneCurve_GradMarginTop;			// Top margin (lp)
	ImWidgetsThickLineMode	ToneCurve_LineMode;

	// HDR Wheel / Primaries Wheel
	float	HDRWheel_RightArcStart;				// Right arc start angle in degrees (0=right, CW)
	float	HDRWheel_RightArcEnd;				// Right arc end angle in degrees
	float	HDRWheel_LeftArcStart;				// Left arc start angle in degrees
	float	HDRWheel_LeftArcEnd;				// Left arc end angle in degrees
	float	HDRWheel_ArcGap;					// Gap between indicator ring and arc sliders (lp)
	float	HDRWheel_ArcThickness;				// Arc slider track thickness (lp)
	float	HDRWheel_ArcGrabRadius;				// Arc slider grab handle radius (lp)
	float	HDRWheel_RingThickness;				// Gradient indicator ring thickness (lp)
	float	HDRWheel_RingGap;					// Gap between indicator ring and inner ColorWheel (lp)

	// Transform Gizmo
	float	Gizmo_HandleSize;					// Corner handle half-size (lp)
	float	Gizmo_RotationHandleOffset;			// Distance from top edge to rotation handle (lp)
	float	Gizmo_OutlineThickness;				// Bounding box outline thickness (lp)
	float	Gizmo_RotationLineThickness;		// Rotation stem line thickness (lp)
	float	Gizmo_CenterDotRadius;				// Selected-object center dot radius (lp)
	float	Gizmo_EdgeHandleScale;				// Edge handle size as fraction of corner handle size
	float	Gizmo_DefaultCanvasHeight;			// Canvas height when no size given (lp)

	// Color Picker
	float	ColorPicker_DotRadius;				// Dot indicator radius (lp)
	int		ColorPicker_PlaneResolution;		// 2D plane grid resolution per axis (count)
	float	ColorPicker_SliderWidth;			// Vertical slider width (lp)
	int		ColorPicker_SliderResolution;		// Vertical slider segment count
	float	ColorPicker_ComponentSliderHeight;	// Component slider height (lp)
	float	ColorPicker_DotOutlineThickness;	// Dot outline ring thickness (lp)
	float	ColorPicker_SliderHandleHeight;		// Vertical slider handle height (lp)
	float	ColorPicker_ComponentHandleWidth;	// Component slider handle width (lp)

	// Hatching
	float	Hatch_DefaultSpacing;
	float	Hatch_DefaultThickness;
	// Bezier curve editor (new)
	float	BezierCurve_KeyRadius;
	float	BezierCurve_TangentRadius;
	float	BezierCurve_LineThickness;
	float	BezierCurve_SnapAngleDeg;
	// Font inspector
	float	FontInspector_GlyphCell;
	float	FontInspector_MetricAlpha;
	// Notched dial
	float	NotchedDial_TickLength;
	float	NotchedDial_TickThickness;
	// Delta-E visualizer
	float	DeltaE_SwatchSize;
	// Equation input
	float	Equation_BlockPadding;
	float	Equation_InlineBaselineOffset;
	// Iso-contour default tier thicknesses
	float	IsoContour_MajorThickness;
	float	IsoContour_MediumThickness;
	float	IsoContour_MinorThickness;
	// LookDev
	float	LookDev_DividerThickness;
	float	LookDev_HandleRadius;
	// Volume slice viewer
	float	VolumeSlice_HandleRadius;
	// Vector drawing tool
	float	VectorDrawing_AnchorRadius;
	float	VectorDrawing_TangentRadius;
	float	VectorDrawing_GridSpacing;
	float	VectorDrawing_TangentLineThickness;		// (lp)
	float	VectorDrawing_AnchorOutlineThickness;	// (lp)
	float	VectorDrawing_ClosingRingRadius;		// First-node closing-ring radius (lp)
	float	VectorDrawing_ClosingRingThickness;		// (lp)
	float	VectorDrawing_HoverRingGap;				// Gap added to anchor radius for hover highlight (lp)
	float	VectorDrawing_HoverRingThickness;		// (lp)
	float	VectorDrawing_GhostRingThickness;		// Empty-canvas ghost ring thickness (lp)
	float	VectorDrawing_CrosshairArmLength;		// Ghost crosshair arm length (lp)
	float	VectorDrawing_CrosshairThickness;		// Ghost crosshair line thickness (lp)
	// Precision popup input width (in lp)
	float	PrecisionPopup_InputWidth;

	// All bool flags grouped at the end so each one only contributes trailing padding
	// rather than 3-byte holes between mid-struct floats. (Mode fields are typed enums
	// and pack as int = 4 bytes so they stay grouped with the floats above.)
	bool	CurveEditor_LineDashed;
	bool	ColorCurve_LineDashed;
	bool	ChromaticityLine_Dashed;
	bool	ToneCurve_LineDashed;

	ImVec4  Colors[ StyleColor_Count ];

	ImWidgetsStyle()
	{
		HueSelector_Thickness_ZeroWidth = 5.0f;

		Slider2D_DragThickness   = 8.0f;
		Slider2D_BorderThickness = 2.0f;
		Slider2D_LineThickness   = 2.0f;
		Slider2D_CursorRadius    = 7.048f;
		Slider2D_CursorOffset    = 12.0f;
		Slider2D_CornerRadius    = 4.0f;

		Slider2DRange_FillAlpha            = 0.15f;
		Slider2DRange_HandleRadius         = 5.0f;

		Slider2DDisc_CursorRadius          = 5.0f;
		Slider2DDisc_CursorOutlineThickness = 1.5f;
		Slider2DDisc_RingThickness         = 2.0f;
		Slider2DDisc_GridRings             = 2;

		NavCursor_Thickness      = 5.0f;
		NavCursor_Distance       = 7.0f;
		WhitePoint_Radius        = 5.0f;
		PrecisionDrag_BlockSize  = 70.0f;

		// Gradient Editor
		Gradient_MarkerHeight        = 30.0f;
		Gradient_CheckerboardCellSize = 15.0f;
		Gradient_MarkerThickness     = 2.0f;

		// Curve Editor
		CurveEditor_DefaultHeight    = 500.0f;
		CurveEditor_KeyRadius        = 6.0f;
		CurveEditor_TangentRadius    = 5.0f;
		CurveEditor_HitRadius        = 22.0f;
		CurveEditor_LineThickness    = 4.0f;
		CurveEditor_KeyOutlineThickness = 2.0f;
		CurveEditor_ZeroLineThickness = 2.0f;
		CurveEditor_LineMode         = ImWidgetsThickLineMode_StrokedBezierPath;
		CurveEditor_LineDashed       = false;

		// Color Wheel
		ColorWheel_DotRadius     = 8.0f;
		ColorWheel_RingThickness = 16.0f;
		ColorWheel_DiscSectors   = 64;
		ColorWheel_DiscRings     = 16;
		ColorWheel_SliderHeight  = 32.0f;

		// Slider Ring
		SliderRing_TrackThickness = 12.0f;
		SliderRing_GrabRadius     = 8.0f;

		// Slider Spline
		SliderSpline_TrackThickness = 12.0f;
		SliderSpline_GrabRadius     = 12.0f;

		// Color Warper
		ColorWarper_PointRadius  = 8.0f;
		ColorWarper_HitRadius    = 18.0f;
		ColorWarper_GridThickness = 2.0f;
		ColorWarper_DiscSectors  = 64;
		ColorWarper_DiscRings    = 16;
		ColorWarper_DiscScale    = 1.0f;  // extra disc shrink inside widget bbox (bbox already auto-shrinks to 75% of requested)

		// Color Curve
		ColorCurve_DefaultHeight = 375.0f;
		ColorCurve_KeyRadius     = 8.0f;
		ColorCurve_LineThickness = 4.0f;
		ColorCurve_LineMode      = ImWidgetsThickLineMode_AddPolyline; // CPU: stroked (shader) mode silently no-draws if the stroke shader fails/rebuilds
		ColorCurve_LineDashed    = false;

		// Parade Scope
		ParadeScope_DefaultHeight    = 500.0f;
		ParadeScope_OverlayAlpha     = 0.75f;
		ParadeScope_GradTickLength   = 12.0f;
		ParadeScope_GradTickThickness = 3.27f;
		ParadeScope_GradMargin       = 40.0f;

		// Vector Scope
		VectorScope_DefaultSize        = 500.0f;
		VectorScope_SignalAlpha        = 0.75f;
		VectorScope_GraticuleThickness = 1.0f;
		VectorScope_DiscScale          = 1.0f;  // extra disc shrink inside widget bbox (bbox already auto-shrinks to 75% of requested)

		// Histogram
		Histogram_DefaultHeight      = 500.0f;
		Histogram_OverlayAlpha       = 0.5f;
		Histogram_GradTickLength     = 10.0f;
		Histogram_GradTickThickness  = 3.0f;
		Histogram_GradMarginLeft     = 42.0f;
		Histogram_GradMarginBottom   = 24.0f;

		// CIE Chromaticity
		CIEChromaticity_DefaultSize        = 750.0f;
		CIEChromaticity_SignalRadius       = 3.0f;
		CIEChromaticity_GamutLineThickness = 3.0f;
		CIEChromaticity_WhitePointRadius   = 10.0f;
		CIEChromaticity_GradMargin         = 24.0f;  // left & bottom (lp)
		CIEChromaticity_SignalAlpha        = 0.6f;
		CIEChromaticity_GradMarginRight    = 64.0f;  // right (lp)
		CIEChromaticity_GradMarginTop      = 16.0f;  // top (lp)

		// Chromaticity Plot / Line / Point
		ChromaticityPlot_Resolution        = 128;
		ChromaticityPlot_LineSamples       = 128;
		ChromaticityPlot_BorderThickness   = 2.0f;
		ChromaticityPlot_TriangleThickness = 3.0f;
		ChromaticityLine_Thickness         = 3.0f;
		ChromaticityPoint_Radius           = 8.0f;
		ChromaticityPoint_Segments         = 24;
		ChromaticityLine_Mode              = ImWidgetsThickLineMode_StrokedBezierPath;
		ChromaticityLine_Dashed            = false;
		ChromaticityLine_SmoothIterations  = 2;

		// Tone Curve
		ToneCurve_DefaultHeight     = 500.0f;
		ToneCurve_KeyRadius         = 8.0f;
		ToneCurve_LineThickness     = 4.0f;
		ToneCurve_GradMarginLeft    = 24.0f;
		ToneCurve_GradMarginBottom  = 24.0f;
		ToneCurve_BandThickness     = 16.0f;
		ToneCurve_BandGap           = 4.0f;
		ToneCurve_GradMarginRight   = 0.0f;
		ToneCurve_GradMarginTop     = 4.0f;
		ToneCurve_LineMode          = ImWidgetsThickLineMode_AddPolyline; // CPU: stroked (shader) mode silently no-draws if the stroke shader fails/rebuilds
		ToneCurve_LineDashed        = false;

		// HDR Wheel / Primaries Wheel
		HDRWheel_RightArcStart   = -45.0f;
		HDRWheel_RightArcEnd     = 45.0f;
		HDRWheel_LeftArcStart    = 135.0f;
		HDRWheel_LeftArcEnd      = 225.0f;
		HDRWheel_ArcGap          = 2.0f;
		HDRWheel_ArcThickness    = 8.0f;
		HDRWheel_ArcGrabRadius   = 4.0f;
		HDRWheel_RingThickness   = 8.0f;
		HDRWheel_RingGap         = 2.0f;

		// Transform Gizmo
		Gizmo_HandleSize              = 6.0f;
		Gizmo_RotationHandleOffset    = 62.0f;
		Gizmo_OutlineThickness        = 3.0f;
		Gizmo_RotationLineThickness   = 2.0f;
		Gizmo_CenterDotRadius         = 4.0f;
		Gizmo_EdgeHandleScale         = 0.8f;
		Gizmo_DefaultCanvasHeight     = 400.0f;

		Colors[ StyleColor_Slider2D_CursorX ] = ImVec4( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 1.0f ); // Blue
		Colors[ StyleColor_Slider2D_CursorY ] = ImVec4( 255.0f / 255.0f, 128.0f / 255.0f, 64.0f / 255.0f, 1.0f ); // Orange

		Colors[ StyleColor_Slider2DRange_MinHandle ]    = ImVec4(  91.0f/255.0f, 194.0f/255.0f, 231.0f/255.0f, 1.00f );
		Colors[ StyleColor_Slider2DRange_MaxHandle ]    = ImVec4( 255.0f/255.0f, 128.0f/255.0f,  64.0f/255.0f, 1.00f );
		Colors[ StyleColor_Slider2DRange_Fill      ]    = ImVec4(  91.0f/255.0f, 194.0f/255.0f, 231.0f/255.0f, 0.20f );
		Colors[ StyleColor_Slider2DDisc_Background ]    = ImVec4( 0.0f, 0.0f, 0.0f, 0.0f );     // transparent â†’ falls back to FrameBg
		Colors[ StyleColor_Slider2DDisc_Cursor     ]    = ImVec4(  91.0f/255.0f, 194.0f/255.0f, 231.0f/255.0f, 1.00f );
		Colors[ StyleColor_Slider2DDisc_CursorOutline ] = ImVec4(  20.0f/255.0f,  20.0f/255.0f,  20.0f/255.0f, 1.00f );
		Colors[ StyleColor_Slider2DDisc_Ring       ]    = ImVec4( 255.0f/255.0f, 128.0f/255.0f,  64.0f/255.0f, 1.00f );
		Colors[ StyleColor_Slider2DDisc_Grid       ]    = ImVec4( 1.0f,          1.0f,           1.0f,          0.15f );

		// Gradient Editor Colors
		Colors[ StyleColor_Gradient_MarkerOutline ]         = ImVec4( 40.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 1.0f );
		Colors[ StyleColor_Gradient_MarkerOutlineHovered ]  = ImVec4( 200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 1.0f );
		Colors[ StyleColor_Gradient_MarkerOutlineSelected ] = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
		Colors[ StyleColor_Gradient_AlphaIndicator ]        = ImVec4( 0.0f, 0.0f, 0.0f, 180.0f / 255.0f );
		Colors[ StyleColor_Gradient_Checkerboard1 ]         = ImVec4( 204.0f / 255.0f, 204.0f / 255.0f, 204.0f / 255.0f, 1.0f );
		Colors[ StyleColor_Gradient_Checkerboard2 ]         = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );

		// Curve Editor Colors
		Colors[ StyleColor_CurveEditor_Line ]               = ImVec4( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 1.0f );
		Colors[ StyleColor_CurveEditor_GridMinor ]          = ImVec4( 200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 40.0f / 255.0f );
		Colors[ StyleColor_CurveEditor_GridMajor ]          = ImVec4( 200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 80.0f / 255.0f );
		Colors[ StyleColor_CurveEditor_ZeroLine ]           = ImVec4( 1.0f, 1.0f, 1.0f, 60.0f / 255.0f );
		Colors[ StyleColor_CurveEditor_Key ]                = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_CurveEditor_KeyHovered ]         = ImVec4( 230.0f / 255.0f, 230.0f / 255.0f, 230.0f / 255.0f, 1.0f );
		Colors[ StyleColor_CurveEditor_KeySelected ]        = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
		Colors[ StyleColor_CurveEditor_KeyOutline ]         = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_CurveEditor_KeyOutlineHovered ]  = ImVec4( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 1.0f );
		Colors[ StyleColor_CurveEditor_KeyOutlineSelected ] = ImVec4( 180.0f / 255.0f, 180.0f / 255.0f, 0.0f, 1.0f );
		Colors[ StyleColor_CurveEditor_TangentLine ]        = ImVec4( 1.0f, 180.0f / 255.0f, 50.0f / 255.0f, 200.0f / 255.0f );
		Colors[ StyleColor_CurveEditor_TangentHovered ]     = ImVec4( 1.0f, 220.0f / 255.0f, 100.0f / 255.0f, 1.0f );
		Colors[ StyleColor_CurveEditor_Crosshair ]          = ImVec4( 1.0f, 1.0f, 1.0f, 100.0f / 255.0f );
		Colors[ StyleColor_CurveEditor_AddIndicator ]       = ImVec4( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 220.0f / 255.0f );
		Colors[ StyleColor_CurveEditor_AxisLabel ]          = ImVec4( 200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 180.0f / 255.0f );

		// Color Wheel Colors
		Colors[ StyleColor_ColorWheel_DotOutline ]          = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_ColorWheel_DotOutlineActive ]    = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ColorWheel_Crosshair ]           = ImVec4( 1.0f, 1.0f, 1.0f, 40.0f / 255.0f );
		Colors[ StyleColor_ColorWheel_SliderOutline ]       = ImVec4( 0.0f, 0.0f, 0.0f, 150.0f / 255.0f );

		// Slider Ring Colors
		Colors[ StyleColor_SliderRing_Track ]               = ImVec4( 1.0f, 1.0f, 1.0f, 60.0f / 255.0f );
		Colors[ StyleColor_SliderRing_TrackActive ]         = ImVec4( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 200.0f / 255.0f );
		Colors[ StyleColor_SliderRing_Grab ]                = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_SliderRing_GrabActive ]          = ImVec4( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 1.0f );

		// Slider Spline Colors
		Colors[ StyleColor_SliderSpline_Track ]              = ImVec4( 1.0f, 1.0f, 1.0f, 60.0f / 255.0f );
		Colors[ StyleColor_SliderSpline_TrackActive ]        = ImVec4( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 200.0f / 255.0f );
		Colors[ StyleColor_SliderSpline_Grab ]               = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_SliderSpline_GrabActive ]         = ImVec4( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 1.0f );

		// Color Warper Colors
		Colors[ StyleColor_ColorWarper_GridLine ]            = ImVec4( 1.0f, 1.0f, 1.0f, 40.0f / 255.0f );
		Colors[ StyleColor_ColorWarper_GridLineHovered ]     = ImVec4( 1.0f, 1.0f, 1.0f, 100.0f / 255.0f );
		Colors[ StyleColor_ColorWarper_PointOutline ]        = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_ColorWarper_PointOutlineActive ]  = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ColorWarper_PointFill ]           = ImVec4( 0.5f, 0.5f, 0.5f, 0.8f );
		Colors[ StyleColor_ColorWarper_PointFillMoved ]      = ImVec4( 1.0f, 0.6f, 0.0f, 0.9f );
		Colors[ StyleColor_ColorWarper_Crosshair ]           = ImVec4( 1.0f, 1.0f, 1.0f, 40.0f / 255.0f );

		// Color Curve Colors
		Colors[ StyleColor_ColorCurve_Background ]          = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ColorCurve_Grid ]                = ImVec4( 200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 25.0f / 255.0f );
		Colors[ StyleColor_ColorCurve_Line ]                = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_ColorCurve_NeutralLine ]         = ImVec4( 1.0f, 1.0f, 1.0f, 80.0f / 255.0f );
		Colors[ StyleColor_ColorCurve_Key ]                 = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_ColorCurve_KeyHovered ]          = ImVec4( 230.0f / 255.0f, 230.0f / 255.0f, 230.0f / 255.0f, 1.0f );
		Colors[ StyleColor_ColorCurve_KeySelected ]         = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ColorCurve_KeyOutline ]          = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ColorCurve_Crosshair ]           = ImVec4( 1.0f, 1.0f, 1.0f, 50.0f / 255.0f );
		Colors[ StyleColor_ColorCurve_HistogramOverlay ]    = ImVec4( 1.0f, 1.0f, 1.0f, 40.0f / 255.0f );

		// Parade Scope Colors
		Colors[ StyleColor_ParadeScope_Background ]         = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ParadeScope_Grid ]               = ImVec4( 1.0f, 1.0f, 1.0f, 0.12f );
		Colors[ StyleColor_ParadeScope_ChannelR ]           = ImVec4( 1.0f, 0.25f, 0.25f, 1.0f );
		Colors[ StyleColor_ParadeScope_ChannelG ]           = ImVec4( 0.25f, 1.0f, 0.25f, 1.0f );
		Colors[ StyleColor_ParadeScope_ChannelB ]           = ImVec4( 0.35f, 0.35f, 1.0f, 1.0f );
		Colors[ StyleColor_ParadeScope_ChannelLuma ]        = ImVec4( 0.9f, 0.9f, 0.9f, 1.0f );
		Colors[ StyleColor_ParadeScope_ChannelCb ]          = ImVec4( 0.3f, 0.5f, 1.0f, 1.0f );
		Colors[ StyleColor_ParadeScope_ChannelCr ]          = ImVec4( 1.0f, 0.4f, 0.3f, 1.0f );
		Colors[ StyleColor_ParadeScope_GradTick ]           = ImVec4( 1.0f, 1.0f, 1.0f, 0.4f );
		Colors[ StyleColor_ParadeScope_GradLabel ]          = ImVec4( 1.0f, 1.0f, 1.0f, 0.7f );

		// Vector Scope Colors
		Colors[ StyleColor_VectorScope_Background ]         = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_VectorScope_Grid ]               = ImVec4( 1.0f, 1.0f, 1.0f, 0.12f );
		Colors[ StyleColor_VectorScope_Graticule ]          = ImVec4( 1.0f, 1.0f, 1.0f, 0.35f );
		Colors[ StyleColor_VectorScope_Signal ]             = ImVec4( 0.2f, 1.0f, 0.3f, 1.0f );
		Colors[ StyleColor_VectorScope_SkinToneLine ]       = ImVec4( 1.0f, 0.7f, 0.3f, 0.5f );
		Colors[ StyleColor_VectorScope_TargetR ]            = ImVec4( 1.0f, 64.0f / 255.0f, 64.0f / 255.0f, 200.0f / 255.0f );
		Colors[ StyleColor_VectorScope_TargetG ]            = ImVec4( 64.0f / 255.0f, 1.0f, 64.0f / 255.0f, 200.0f / 255.0f );
		Colors[ StyleColor_VectorScope_TargetB ]            = ImVec4( 80.0f / 255.0f, 80.0f / 255.0f, 1.0f, 200.0f / 255.0f );
		Colors[ StyleColor_VectorScope_TargetCy ]           = ImVec4( 64.0f / 255.0f, 1.0f, 1.0f, 200.0f / 255.0f );
		Colors[ StyleColor_VectorScope_TargetMg ]           = ImVec4( 1.0f, 64.0f / 255.0f, 1.0f, 200.0f / 255.0f );
		Colors[ StyleColor_VectorScope_TargetYl ]           = ImVec4( 1.0f, 1.0f, 64.0f / 255.0f, 200.0f / 255.0f );

		// Histogram Colors
		Colors[ StyleColor_Histogram_Background ]           = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_Histogram_Grid ]                 = ImVec4( 1.0f, 1.0f, 1.0f, 0.12f );
		Colors[ StyleColor_Histogram_ChannelR ]             = ImVec4( 1.0f, 0.25f, 0.25f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelG ]             = ImVec4( 0.25f, 1.0f, 0.25f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelB ]             = ImVec4( 0.35f, 0.35f, 1.0f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelLuma ]          = ImVec4( 0.9f, 0.9f, 0.9f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelCb ]            = ImVec4( 0.3f, 0.5f, 1.0f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelCr ]            = ImVec4( 1.0f, 0.4f, 0.3f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelH ]             = ImVec4( 1.0f, 0.5f, 0.8f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelS ]             = ImVec4( 0.3f, 0.9f, 0.9f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelV ]             = ImVec4( 0.85f, 0.85f, 0.85f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelOkL ]           = ImVec4( 0.9f, 0.9f, 0.9f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelOkC ]           = ImVec4( 0.8f, 0.4f, 1.0f, 1.0f );
		Colors[ StyleColor_Histogram_ChannelOkH ]           = ImVec4( 1.0f, 0.6f, 0.3f, 1.0f );
		Colors[ StyleColor_Histogram_GradTick ]             = ImVec4( 1.0f, 1.0f, 1.0f, 0.4f );
		Colors[ StyleColor_Histogram_GradLabel ]            = ImVec4( 1.0f, 1.0f, 1.0f, 0.7f );

		// CIE Chromaticity Colors
		Colors[ StyleColor_CIEChromaticity_Background ]     = ImVec4( 0.06f, 0.06f, 0.06f, 1.0f );
		Colors[ StyleColor_CIEChromaticity_Grid ]           = ImVec4( 1.0f, 1.0f, 1.0f, 0.12f );
		Colors[ StyleColor_CIEChromaticity_Signal ]         = ImVec4( 1.0f, 1.0f, 1.0f, 0.6f );
		Colors[ StyleColor_CIEChromaticity_GamutLine ]      = ImVec4( 1.0f, 1.0f, 1.0f, 0.8f );
		Colors[ StyleColor_CIEChromaticity_WhitePoint ]     = ImVec4( 1.0f, 1.0f, 1.0f, 0.9f );
		Colors[ StyleColor_CIEChromaticity_PrimaryR ]       = ImVec4( 1.0f, 100.0f / 255.0f, 100.0f / 255.0f, 220.0f / 255.0f );
		Colors[ StyleColor_CIEChromaticity_PrimaryG ]       = ImVec4( 100.0f / 255.0f, 1.0f, 100.0f / 255.0f, 220.0f / 255.0f );
		Colors[ StyleColor_CIEChromaticity_PrimaryB ]       = ImVec4( 100.0f / 255.0f, 100.0f / 255.0f, 1.0f, 220.0f / 255.0f );
		Colors[ StyleColor_CIEChromaticity_GradTick ]       = ImVec4( 1.0f, 1.0f, 1.0f, 0.4f );
		Colors[ StyleColor_CIEChromaticity_GradLabel ]      = ImVec4( 1.0f, 1.0f, 1.0f, 0.7f );

		// Chromaticity Plot / Line / Point (standalone helpers)
		Colors[ StyleColor_ChromaticityPlot_Triangle ]      = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_ChromaticityPlot_WhitePoint ]    = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ChromaticityLine_Default ]       = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ChromaticityPoint_Default ]      = ImVec4( 1.0f, 0.0f, 0.0f, 1.0f );

		Colors[ StyleColor_HueSelector_ZeroWidthLine ]      = ImVec4( 0.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_HueSelector_Cursor ]             = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );

		// Tone Curve Colors
		Colors[ StyleColor_ToneCurve_Background ]           = ImVec4( 0.06f, 0.06f, 0.06f, 1.0f );
		Colors[ StyleColor_ToneCurve_Grid ]                 = ImVec4( 1.0f, 1.0f, 1.0f, 0.12f );
		Colors[ StyleColor_ToneCurve_NeutralLine ]          = ImVec4( 0.5f, 0.5f, 0.5f, 0.5f );
		Colors[ StyleColor_ToneCurve_Key ]                  = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_ToneCurve_KeySelected ]          = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_ToneCurve_KeyOutline ]           = ImVec4( 0.0f, 0.0f, 0.0f, 200.0f / 255.0f );
		Colors[ StyleColor_ToneCurve_Crosshair ]            = ImVec4( 1.0f, 1.0f, 1.0f, 50.0f / 255.0f );
		Colors[ StyleColor_ToneCurve_HistogramOverlay ]     = ImVec4( 1.0f, 1.0f, 1.0f, 40.0f / 255.0f );
		Colors[ StyleColor_ToneCurve_GradTick ]             = ImVec4( 1.0f, 1.0f, 1.0f, 0.4f );
		Colors[ StyleColor_ToneCurve_GradLabel ]            = ImVec4( 1.0f, 1.0f, 1.0f, 0.7f );

		// HDR Wheel Colors
		Colors[ StyleColor_HDRWheel_RightArc ]              = ImVec4( 1.0f, 0.0f, 0.0f, 0.8f );
		Colors[ StyleColor_HDRWheel_LeftArcMin ]            = ImVec4( 0.0f, 0.0f, 0.0f, 0.8f );
		Colors[ StyleColor_HDRWheel_LeftArcMax ]            = ImVec4( 1.0f, 1.0f, 1.0f, 0.8f );

		// Transform Gizmo Colors
		Colors[ StyleColor_Gizmo_Canvas ]                  = ImVec4( 30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 1.0f );
		Colors[ StyleColor_Gizmo_Outline ]                 = ImVec4( 1.0f, 1.0f, 1.0f, 180.0f / 255.0f );
		Colors[ StyleColor_Gizmo_Handle ]                  = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_Gizmo_HandleActive ]            = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
		Colors[ StyleColor_Gizmo_DimOutline ]              = ImVec4( 1.0f, 1.0f, 1.0f, 60.0f / 255.0f );

		// Color Picker Colors
		Colors[ StyleColor_ColorPicker_DotOutline ]        = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_ColorPicker_DotOutlineActive ]  = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ColorPicker_Crosshair ]         = ImVec4( 1.0f, 1.0f, 1.0f, 40.0f / 255.0f );
		Colors[ StyleColor_ColorPicker_SliderOutline ]     = ImVec4( 0.0f, 0.0f, 0.0f, 150.0f / 255.0f );
		Colors[ StyleColor_ColorPicker_SliderHandle ]      = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );

		// Color Picker Vars
		ColorPicker_DotRadius              = 8.0f;
		ColorPicker_PlaneResolution        = 24;
		ColorPicker_SliderWidth            = 24.0f;
		ColorPicker_SliderResolution       = 16;
		ColorPicker_ComponentSliderHeight  = 24.0f;
		ColorPicker_DotOutlineThickness    = 1.0f;
		ColorPicker_SliderHandleHeight     = 6.0f;
		ColorPicker_ComponentHandleWidth   = 6.0f;

		// Hatching
		Hatch_DefaultSpacing      = 20.0f;
		Hatch_DefaultThickness    = 2.0f;
		// Bezier curve editor
		BezierCurve_KeyRadius     = 12.0f;
		BezierCurve_TangentRadius = 10.0f;
		BezierCurve_LineThickness = 5.0f;
		BezierCurve_SnapAngleDeg  = 5.0f;
		// Font inspector
		FontInspector_GlyphCell   = 180.0f;
		FontInspector_MetricAlpha = 0.6f;
		// Notched dial
		NotchedDial_TickLength    = 16.0f;
		NotchedDial_TickThickness = 2.0f;
		// Delta-E
		DeltaE_SwatchSize         = 128.0f;
		// Equation input
		Equation_BlockPadding         = 20.0f;
		Equation_InlineBaselineOffset = 0.0f;

		// Iso-contour default tier thicknesses (lp).
		IsoContour_MajorThickness  = 5.0f;
		IsoContour_MediumThickness = 2.0f;
		IsoContour_MinorThickness  = 1.0f;
		// LookDev.
		LookDev_DividerThickness = 2.0f;
		LookDev_HandleRadius     = 8.0f;
		// Volume slice viewer.
		VolumeSlice_HandleRadius = 15.0f;
		// Vector drawing tool.
		VectorDrawing_AnchorRadius           = 8.0f;
		VectorDrawing_TangentRadius          = 5.0f;
		VectorDrawing_GridSpacing            = 64.0f;
		VectorDrawing_TangentLineThickness   = 2.0f;
		VectorDrawing_AnchorOutlineThickness = 1.0f;
		VectorDrawing_ClosingRingRadius      = 12.0f;
		VectorDrawing_ClosingRingThickness   = 2.0f;
		VectorDrawing_HoverRingGap           = 6.0f;
		VectorDrawing_HoverRingThickness     = 4.0f;
		VectorDrawing_GhostRingThickness     = 2.0f;
		VectorDrawing_CrosshairArmLength     = 12.0f;
		VectorDrawing_CrosshairThickness     = 1.0f;
		// Precision popup input width (in lp).
		PrecisionPopup_InputWidth = 300.0f;
	}

	void ScaleAllSizes( float scale_factor )
	{
		HueSelector_Thickness_ZeroWidth = truncf( HueSelector_Thickness_ZeroWidth * scale_factor );
		Slider2D_DragThickness   = truncf( Slider2D_DragThickness * scale_factor );
		Slider2D_BorderThickness = truncf( Slider2D_BorderThickness * scale_factor );
		Slider2D_LineThickness   = truncf( Slider2D_LineThickness * scale_factor );
		Slider2D_CursorRadius    = truncf( Slider2D_CursorRadius * scale_factor );
		Slider2D_CursorOffset    = truncf( Slider2D_CursorOffset * scale_factor );
		Slider2D_CornerRadius    = truncf( Slider2D_CornerRadius * scale_factor );
		NavCursor_Thickness      = truncf( NavCursor_Thickness * scale_factor );
		NavCursor_Distance       = truncf( NavCursor_Distance * scale_factor );
		WhitePoint_Radius        = truncf( WhitePoint_Radius * scale_factor );
		PrecisionDrag_BlockSize  = truncf( PrecisionDrag_BlockSize * scale_factor );

		Gradient_MarkerHeight         = truncf( Gradient_MarkerHeight * scale_factor );
		Gradient_CheckerboardCellSize = truncf( Gradient_CheckerboardCellSize * scale_factor );
		Gradient_MarkerThickness      = truncf( Gradient_MarkerThickness * scale_factor );

		CurveEditor_DefaultHeight     = truncf( CurveEditor_DefaultHeight * scale_factor );
		CurveEditor_KeyRadius         = truncf( CurveEditor_KeyRadius * scale_factor );
		CurveEditor_TangentRadius     = truncf( CurveEditor_TangentRadius * scale_factor );
		CurveEditor_HitRadius         = truncf( CurveEditor_HitRadius * scale_factor );
		CurveEditor_LineThickness     = truncf( CurveEditor_LineThickness * scale_factor );
		CurveEditor_KeyOutlineThickness = truncf( CurveEditor_KeyOutlineThickness * scale_factor );
		CurveEditor_ZeroLineThickness = truncf( CurveEditor_ZeroLineThickness * scale_factor );

		ColorWheel_DotRadius     = truncf( ColorWheel_DotRadius * scale_factor );
		ColorWheel_RingThickness = truncf( ColorWheel_RingThickness * scale_factor );
		ColorWheel_SliderHeight  = truncf( ColorWheel_SliderHeight * scale_factor );

		SliderRing_TrackThickness = truncf( SliderRing_TrackThickness * scale_factor );
		SliderRing_GrabRadius     = truncf( SliderRing_GrabRadius * scale_factor );

		SliderSpline_TrackThickness = truncf( SliderSpline_TrackThickness * scale_factor );
		SliderSpline_GrabRadius     = truncf( SliderSpline_GrabRadius * scale_factor );

		ColorWarper_PointRadius   = truncf( ColorWarper_PointRadius * scale_factor );
		ColorWarper_HitRadius    = truncf( ColorWarper_HitRadius * scale_factor );
		ColorWarper_GridThickness = truncf( ColorWarper_GridThickness * scale_factor );

		ColorCurve_DefaultHeight = truncf( ColorCurve_DefaultHeight * scale_factor );
		ColorCurve_KeyRadius     = truncf( ColorCurve_KeyRadius * scale_factor );
		ColorCurve_LineThickness = truncf( ColorCurve_LineThickness * scale_factor );

		ParadeScope_DefaultHeight     = truncf( ParadeScope_DefaultHeight * scale_factor );
		ParadeScope_GradTickLength    = truncf( ParadeScope_GradTickLength * scale_factor );
		ParadeScope_GradTickThickness = truncf( ParadeScope_GradTickThickness * scale_factor );
		ParadeScope_GradMargin        = truncf( ParadeScope_GradMargin * scale_factor );

		VectorScope_DefaultSize        = truncf( VectorScope_DefaultSize * scale_factor );
		VectorScope_GraticuleThickness = truncf( VectorScope_GraticuleThickness * scale_factor );
		// VectorScope_DiscScale is a ratio â€” not scaled

		Histogram_DefaultHeight      = truncf( Histogram_DefaultHeight * scale_factor );
		Histogram_GradTickLength     = truncf( Histogram_GradTickLength * scale_factor );
		Histogram_GradTickThickness  = truncf( Histogram_GradTickThickness * scale_factor );
		Histogram_GradMarginLeft     = truncf( Histogram_GradMarginLeft * scale_factor );
		Histogram_GradMarginBottom   = truncf( Histogram_GradMarginBottom * scale_factor );

		CIEChromaticity_DefaultSize        = truncf( CIEChromaticity_DefaultSize * scale_factor );
		CIEChromaticity_SignalRadius       = truncf( CIEChromaticity_SignalRadius * scale_factor );
		CIEChromaticity_GamutLineThickness = truncf( CIEChromaticity_GamutLineThickness * scale_factor );
		CIEChromaticity_WhitePointRadius   = truncf( CIEChromaticity_WhitePointRadius * scale_factor );
		CIEChromaticity_GradMargin         = truncf( CIEChromaticity_GradMargin * scale_factor );
		CIEChromaticity_GradMarginRight    = truncf( CIEChromaticity_GradMarginRight * scale_factor );
		CIEChromaticity_GradMarginTop      = truncf( CIEChromaticity_GradMarginTop * scale_factor );
		// Chromaticity Plot / Line / Point â€” only lp fields are scaled; counts/segments are not.
		ChromaticityPlot_BorderThickness   = truncf( ChromaticityPlot_BorderThickness * scale_factor );
		ChromaticityPlot_TriangleThickness = truncf( ChromaticityPlot_TriangleThickness * scale_factor );
		ChromaticityLine_Thickness         = truncf( ChromaticityLine_Thickness * scale_factor );
		ChromaticityPoint_Radius           = truncf( ChromaticityPoint_Radius * scale_factor );

		ToneCurve_DefaultHeight     = truncf( ToneCurve_DefaultHeight * scale_factor );
		ToneCurve_KeyRadius         = truncf( ToneCurve_KeyRadius * scale_factor );
		ToneCurve_LineThickness     = truncf( ToneCurve_LineThickness * scale_factor );
		ToneCurve_GradMarginLeft    = truncf( ToneCurve_GradMarginLeft * scale_factor );
		ToneCurve_GradMarginBottom  = truncf( ToneCurve_GradMarginBottom * scale_factor );
		ToneCurve_BandThickness     = truncf( ToneCurve_BandThickness * scale_factor );
		ToneCurve_BandGap           = truncf( ToneCurve_BandGap * scale_factor );
		ToneCurve_GradMarginRight   = truncf( ToneCurve_GradMarginRight * scale_factor );
		ToneCurve_GradMarginTop     = truncf( ToneCurve_GradMarginTop * scale_factor );

		HDRWheel_ArcGap             = truncf( HDRWheel_ArcGap * scale_factor );
		HDRWheel_ArcThickness       = truncf( HDRWheel_ArcThickness * scale_factor );
		HDRWheel_ArcGrabRadius      = truncf( HDRWheel_ArcGrabRadius * scale_factor );
		HDRWheel_RingThickness      = truncf( HDRWheel_RingThickness * scale_factor );
		HDRWheel_RingGap            = truncf( HDRWheel_RingGap * scale_factor );

		Gizmo_HandleSize             = truncf( Gizmo_HandleSize * scale_factor );
		Gizmo_RotationHandleOffset   = truncf( Gizmo_RotationHandleOffset * scale_factor );
		Gizmo_OutlineThickness       = truncf( Gizmo_OutlineThickness * scale_factor );
		Gizmo_RotationLineThickness  = truncf( Gizmo_RotationLineThickness * scale_factor );
		Gizmo_CenterDotRadius        = truncf( Gizmo_CenterDotRadius * scale_factor );
		// Gizmo_EdgeHandleScale is a ratio â€” not scaled
		Gizmo_DefaultCanvasHeight    = truncf( Gizmo_DefaultCanvasHeight * scale_factor );

		ColorPicker_DotRadius              = truncf( ColorPicker_DotRadius * scale_factor );
		ColorPicker_SliderWidth            = truncf( ColorPicker_SliderWidth * scale_factor );
		ColorPicker_ComponentSliderHeight  = truncf( ColorPicker_ComponentSliderHeight * scale_factor );
		ColorPicker_DotOutlineThickness    = truncf( ColorPicker_DotOutlineThickness * scale_factor );
		ColorPicker_SliderHandleHeight     = truncf( ColorPicker_SliderHandleHeight * scale_factor );
		ColorPicker_ComponentHandleWidth   = truncf( ColorPicker_ComponentHandleWidth * scale_factor );
		Hatch_DefaultSpacing      = truncf( Hatch_DefaultSpacing * scale_factor );
		Hatch_DefaultThickness    = truncf( Hatch_DefaultThickness * scale_factor );
		BezierCurve_KeyRadius     = truncf( BezierCurve_KeyRadius * scale_factor );
		BezierCurve_TangentRadius = truncf( BezierCurve_TangentRadius * scale_factor );
		BezierCurve_LineThickness = truncf( BezierCurve_LineThickness * scale_factor );
		FontInspector_GlyphCell   = truncf( FontInspector_GlyphCell * scale_factor );
		NotchedDial_TickLength    = truncf( NotchedDial_TickLength * scale_factor );
		DeltaE_SwatchSize         = truncf( DeltaE_SwatchSize * scale_factor );
		Equation_BlockPadding     = truncf( Equation_BlockPadding * scale_factor );
		IsoContour_MajorThickness  = truncf( IsoContour_MajorThickness * scale_factor );
		IsoContour_MediumThickness = truncf( IsoContour_MediumThickness * scale_factor );
		IsoContour_MinorThickness  = truncf( IsoContour_MinorThickness * scale_factor );
		LookDev_DividerThickness  = truncf( LookDev_DividerThickness * scale_factor );
		LookDev_HandleRadius      = truncf( LookDev_HandleRadius * scale_factor );
		VolumeSlice_HandleRadius  = truncf( VolumeSlice_HandleRadius * scale_factor );
		VectorDrawing_AnchorRadius           = truncf( VectorDrawing_AnchorRadius * scale_factor );
		VectorDrawing_TangentRadius          = truncf( VectorDrawing_TangentRadius * scale_factor );
		VectorDrawing_GridSpacing            = truncf( VectorDrawing_GridSpacing * scale_factor );
		VectorDrawing_TangentLineThickness   = truncf( VectorDrawing_TangentLineThickness * scale_factor );
		VectorDrawing_AnchorOutlineThickness = truncf( VectorDrawing_AnchorOutlineThickness * scale_factor );
		VectorDrawing_ClosingRingRadius      = truncf( VectorDrawing_ClosingRingRadius * scale_factor );
		VectorDrawing_ClosingRingThickness   = truncf( VectorDrawing_ClosingRingThickness * scale_factor );
		VectorDrawing_HoverRingGap           = truncf( VectorDrawing_HoverRingGap * scale_factor );
		VectorDrawing_HoverRingThickness     = truncf( VectorDrawing_HoverRingThickness * scale_factor );
		VectorDrawing_GhostRingThickness     = truncf( VectorDrawing_GhostRingThickness * scale_factor );
		VectorDrawing_CrosshairArmLength     = truncf( VectorDrawing_CrosshairArmLength * scale_factor );
		VectorDrawing_CrosshairThickness     = truncf( VectorDrawing_CrosshairThickness * scale_factor );
		PrecisionPopup_InputWidth            = truncf( PrecisionPopup_InputWidth * scale_factor );
	}

	void PushColor( ImWidgetsStyleColor colorIndex, ImVec4 const& color )
	{
		ColorModifier modifier;
		modifier.Index = colorIndex;
		modifier.Value = Colors[ colorIndex ];
		m_ColorStack.push_back( modifier );
		Colors[ colorIndex ] = color;
	}
	void PopColor( int count = 1 )
	{
		while ( count > 0 )
		{
			auto& modifier = m_ColorStack.back();
			Colors[ modifier.Index ] = modifier.Value;
			m_ColorStack.pop_back();
			--count;
		}
	}

	void PushVar( ImWidgetsStyleVar varIndex, float value )
	{
		auto* var = GetVarFloatAddr( varIndex );
		IM_ASSERT( var != nullptr );
		VarModifier modifier;
		modifier.Index = varIndex;
		modifier.Value = ImVec4( *var, 0, 0, 0 );
		*var = value;
		m_VarStack.push_back( modifier );
	}
	void PushVar( ImWidgetsStyleVar varIndex, ImVec2 const& value )
	{
		auto* var = GetVarVec2Addr( varIndex );
		IM_ASSERT( var != nullptr );
		VarModifier modifier;
		modifier.Index = varIndex;
		modifier.Value = ImVec4( var->x, var->y, 0, 0 );
		*var = value;
		m_VarStack.push_back( modifier );
	}
	void PushVar( ImWidgetsStyleVar varIndex, ImVec4 const& value )
	{
		auto* var = GetVarVec4Addr( varIndex );
		IM_ASSERT( var != nullptr );
		VarModifier modifier;
		modifier.Index = varIndex;
		modifier.Value = *var;
		*var = value;
		m_VarStack.push_back( modifier );
	}
	void PopVar( int count = 1 )
	{
		while ( count > 0 )
		{
			auto& modifier = m_VarStack.back();
			if ( auto floatValue = GetVarFloatAddr( modifier.Index ) )
				*floatValue = modifier.Value.x;
			else if ( auto vec2Value = GetVarVec2Addr( modifier.Index ) )
				*vec2Value = ImVec2( modifier.Value.x, modifier.Value.y );
			else if ( auto vec4Value = GetVarVec4Addr( modifier.Index ) )
				*vec4Value = modifier.Value;
			m_VarStack.pop_back();
			--count;
		}
	}

	char const* GetColorName( ImWidgetsStyleColor colorIndex ) const
	{
		switch ( colorIndex )
		{
		case StyleColor_Slider2D_CursorX: return "Slider2DCursorX";
		case StyleColor_Slider2D_CursorY: return "Slider2DCursorY";
		case StyleColor_Slider2DRange_MinHandle: return "Slider2DRangeMinHandle";
		case StyleColor_Slider2DRange_MaxHandle: return "Slider2DRangeMaxHandle";
		case StyleColor_Slider2DRange_Fill:      return "Slider2DRangeFill";
		case StyleColor_Slider2DDisc_Background:      return "Slider2DDiscBackground";
		case StyleColor_Slider2DDisc_Cursor:          return "Slider2DDiscCursor";
		case StyleColor_Slider2DDisc_CursorOutline:   return "Slider2DDiscCursorOutline";
		case StyleColor_Slider2DDisc_Ring:            return "Slider2DDiscRing";
		case StyleColor_Slider2DDisc_Grid:            return "Slider2DDiscGrid";
		case StyleColor_Gradient_MarkerOutline: return "GradientMarkerOutline";
		case StyleColor_Gradient_MarkerOutlineHovered: return "GradientMarkerOutlineHovered";
		case StyleColor_Gradient_MarkerOutlineSelected: return "GradientMarkerOutlineSelected";
		case StyleColor_Gradient_AlphaIndicator: return "GradientAlphaIndicator";
		case StyleColor_Gradient_Checkerboard1: return "GradientCheckerboard1";
		case StyleColor_Gradient_Checkerboard2: return "GradientCheckerboard2";
		case StyleColor_CurveEditor_Line: return "CurveEditorLine";
		case StyleColor_CurveEditor_GridMinor: return "CurveEditorGridMinor";
		case StyleColor_CurveEditor_GridMajor: return "CurveEditorGridMajor";
		case StyleColor_CurveEditor_ZeroLine: return "CurveEditorZeroLine";
		case StyleColor_CurveEditor_Key: return "CurveEditorKey";
		case StyleColor_CurveEditor_KeyHovered: return "CurveEditorKeyHovered";
		case StyleColor_CurveEditor_KeySelected: return "CurveEditorKeySelected";
		case StyleColor_CurveEditor_KeyOutline: return "CurveEditorKeyOutline";
		case StyleColor_CurveEditor_KeyOutlineHovered: return "CurveEditorKeyOutlineHovered";
		case StyleColor_CurveEditor_KeyOutlineSelected: return "CurveEditorKeyOutlineSelected";
		case StyleColor_CurveEditor_TangentLine: return "CurveEditorTangentLine";
		case StyleColor_CurveEditor_TangentHovered: return "CurveEditorTangentHovered";
		case StyleColor_CurveEditor_Crosshair: return "CurveEditorCrosshair";
		case StyleColor_CurveEditor_AddIndicator: return "CurveEditorAddIndicator";
		case StyleColor_CurveEditor_AxisLabel: return "CurveEditorAxisLabel";
		case StyleColor_ColorWheel_DotOutline: return "ColorWheelDotOutline";
		case StyleColor_ColorWheel_DotOutlineActive: return "ColorWheelDotOutlineActive";
		case StyleColor_ColorWheel_Crosshair: return "ColorWheelCrosshair";
		case StyleColor_ColorWheel_SliderOutline: return "ColorWheelSliderOutline";
		case StyleColor_SliderRing_Track: return "SliderRingTrack";
		case StyleColor_SliderRing_TrackActive: return "SliderRingTrackActive";
		case StyleColor_SliderRing_Grab: return "SliderRingGrab";
		case StyleColor_SliderRing_GrabActive: return "SliderRingGrabActive";
		case StyleColor_SliderSpline_Track: return "SliderSplineTrack";
		case StyleColor_SliderSpline_TrackActive: return "SliderSplineTrackActive";
		case StyleColor_SliderSpline_Grab: return "SliderSplineGrab";
		case StyleColor_SliderSpline_GrabActive: return "SliderSplineGrabActive";
		case StyleColor_ColorWarper_GridLine: return "ColorWarperGridLine";
		case StyleColor_ColorWarper_GridLineHovered: return "ColorWarperGridLineHovered";
		case StyleColor_ColorWarper_PointOutline: return "ColorWarperPointOutline";
		case StyleColor_ColorWarper_PointOutlineActive: return "ColorWarperPointOutlineActive";
		case StyleColor_ColorWarper_PointFill: return "ColorWarperPointFill";
		case StyleColor_ColorWarper_PointFillMoved: return "ColorWarperPointFillMoved";
		case StyleColor_ColorWarper_Crosshair: return "ColorWarperCrosshair";
		case StyleColor_ColorCurve_Background: return "ColorCurveBackground";
		case StyleColor_ColorCurve_Grid: return "ColorCurveGrid";
		case StyleColor_ColorCurve_Line: return "ColorCurveLine";
		case StyleColor_ColorCurve_NeutralLine: return "ColorCurveNeutralLine";
		case StyleColor_ColorCurve_Key: return "ColorCurveKey";
		case StyleColor_ColorCurve_KeyHovered: return "ColorCurveKeyHovered";
		case StyleColor_ColorCurve_KeySelected: return "ColorCurveKeySelected";
		case StyleColor_ColorCurve_KeyOutline: return "ColorCurveKeyOutline";
		case StyleColor_ColorCurve_Crosshair: return "ColorCurveCrosshair";
		case StyleColor_ColorCurve_HistogramOverlay: return "ColorCurveHistogramOverlay";
		case StyleColor_ParadeScope_Background: return "ParadeScopeBackground";
		case StyleColor_ParadeScope_Grid: return "ParadeScopeGrid";
		case StyleColor_ParadeScope_ChannelR: return "ParadeScopeChannelR";
		case StyleColor_ParadeScope_ChannelG: return "ParadeScopeChannelG";
		case StyleColor_ParadeScope_ChannelB: return "ParadeScopeChannelB";
		case StyleColor_ParadeScope_ChannelLuma: return "ParadeScopeChannelLuma";
		case StyleColor_ParadeScope_ChannelCb: return "ParadeScopeChannelCb";
		case StyleColor_ParadeScope_ChannelCr: return "ParadeScopeChannelCr";
		case StyleColor_ParadeScope_GradTick: return "ParadeScopeGradTick";
		case StyleColor_ParadeScope_GradLabel: return "ParadeScopeGradLabel";
		case StyleColor_VectorScope_Background: return "VectorScopeBackground";
		case StyleColor_VectorScope_Grid: return "VectorScopeGrid";
		case StyleColor_VectorScope_Graticule: return "VectorScopeGraticule";
		case StyleColor_VectorScope_Signal: return "VectorScopeSignal";
		case StyleColor_VectorScope_SkinToneLine: return "VectorScopeSkinToneLine";
		case StyleColor_VectorScope_TargetR: return "VectorScopeTargetR";
		case StyleColor_VectorScope_TargetG: return "VectorScopeTargetG";
		case StyleColor_VectorScope_TargetB: return "VectorScopeTargetB";
		case StyleColor_VectorScope_TargetCy: return "VectorScopeTargetCy";
		case StyleColor_VectorScope_TargetMg: return "VectorScopeTargetMg";
		case StyleColor_VectorScope_TargetYl: return "VectorScopeTargetYl";
		case StyleColor_Histogram_Background: return "HistogramBackground";
		case StyleColor_Histogram_Grid: return "HistogramGrid";
		case StyleColor_Histogram_ChannelR: return "HistogramChannelR";
		case StyleColor_Histogram_ChannelG: return "HistogramChannelG";
		case StyleColor_Histogram_ChannelB: return "HistogramChannelB";
		case StyleColor_Histogram_ChannelLuma: return "HistogramChannelLuma";
		case StyleColor_Histogram_ChannelCb: return "HistogramChannelCb";
		case StyleColor_Histogram_ChannelCr: return "HistogramChannelCr";
		case StyleColor_Histogram_ChannelH: return "HistogramChannelH";
		case StyleColor_Histogram_ChannelS: return "HistogramChannelS";
		case StyleColor_Histogram_ChannelV: return "HistogramChannelV";
		case StyleColor_Histogram_ChannelOkL: return "HistogramChannelOkL";
		case StyleColor_Histogram_ChannelOkC: return "HistogramChannelOkC";
		case StyleColor_Histogram_ChannelOkH: return "HistogramChannelOkH";
		case StyleColor_Histogram_GradTick: return "HistogramGradTick";
		case StyleColor_Histogram_GradLabel: return "HistogramGradLabel";
		case StyleColor_CIEChromaticity_Background: return "CIEChromaticityBackground";
		case StyleColor_CIEChromaticity_Grid: return "CIEChromaticityGrid";
		case StyleColor_CIEChromaticity_Signal: return "CIEChromaticitySignal";
		case StyleColor_CIEChromaticity_GamutLine: return "CIEChromaticityGamutLine";
		case StyleColor_CIEChromaticity_WhitePoint: return "CIEChromaticityWhitePoint";
		case StyleColor_CIEChromaticity_PrimaryR: return "CIEChromaticityPrimaryR";
		case StyleColor_CIEChromaticity_PrimaryG: return "CIEChromaticityPrimaryG";
		case StyleColor_CIEChromaticity_PrimaryB: return "CIEChromaticityPrimaryB";
		case StyleColor_CIEChromaticity_GradTick: return "CIEChromaticityGradTick";
		case StyleColor_CIEChromaticity_GradLabel: return "CIEChromaticityGradLabel";
		case StyleColor_ChromaticityPlot_Triangle:   return "ChromaticityPlotTriangle";
		case StyleColor_ChromaticityPlot_WhitePoint: return "ChromaticityPlotWhitePoint";
		case StyleColor_ChromaticityLine_Default:    return "ChromaticityLineDefault";
		case StyleColor_ChromaticityPoint_Default:   return "ChromaticityPointDefault";
		case StyleColor_HueSelector_ZeroWidthLine:   return "HueSelectorZeroWidthLine";
		case StyleColor_HueSelector_Cursor:          return "HueSelectorCursor";
		case StyleColor_ToneCurve_Background: return "ToneCurveBackground";
		case StyleColor_ToneCurve_Grid: return "ToneCurveGrid";
		case StyleColor_ToneCurve_NeutralLine: return "ToneCurveNeutralLine";
		case StyleColor_ToneCurve_Key: return "ToneCurveKey";
		case StyleColor_ToneCurve_KeySelected: return "ToneCurveKeySelected";
		case StyleColor_ToneCurve_KeyOutline: return "ToneCurveKeyOutline";
		case StyleColor_ToneCurve_Crosshair: return "ToneCurveCrosshair";
		case StyleColor_ToneCurve_HistogramOverlay: return "ToneCurveHistogramOverlay";
		case StyleColor_ToneCurve_GradTick: return "ToneCurveGradTick";
		case StyleColor_ToneCurve_GradLabel: return "ToneCurveGradLabel";
		case StyleColor_HDRWheel_RightArc: return "HDRWheelRightArc";
		case StyleColor_HDRWheel_LeftArcMin: return "HDRWheelLeftArcMin";
		case StyleColor_HDRWheel_LeftArcMax: return "HDRWheelLeftArcMax";
		case StyleColor_Gizmo_Canvas: return "GizmoCanvas";
		case StyleColor_Gizmo_Outline: return "GizmoOutline";
		case StyleColor_Gizmo_Handle: return "GizmoHandle";
		case StyleColor_Gizmo_HandleActive: return "GizmoHandleActive";
		case StyleColor_Gizmo_DimOutline: return "GizmoDimOutline";
		case StyleColor_ColorPicker_DotOutline: return "ColorPickerDotOutline";
		case StyleColor_ColorPicker_DotOutlineActive: return "ColorPickerDotOutlineActive";
		case StyleColor_ColorPicker_Crosshair: return "ColorPickerCrosshair";
		case StyleColor_ColorPicker_SliderOutline: return "ColorPickerSliderOutline";
		case StyleColor_ColorPicker_SliderHandle: return "ColorPickerSliderHandle";
		case StyleColor_Count: break;
		}

		IM_ASSERT( 0 );
		return "Unknown";
	}

private:
	struct ColorModifier
	{
		ImWidgetsStyleColor  Index;
		ImVec4      Value;
	};

	struct VarModifier
	{
		ImWidgetsStyleVar Index;
		ImVec4   Value;
	};

	float* GetVarFloatAddr( ImWidgetsStyleVar idx )
	{
		switch ( idx )
		{
		case StyleVar_HueSelector_Thickness_ZeroWidth:	return &HueSelector_Thickness_ZeroWidth;
		case StyleVar_Slider2D_DragThickness:			return &Slider2D_DragThickness;
		case StyleVar_Slider2D_BorderThickness:			return &Slider2D_BorderThickness;
		case StyleVar_Slider2D_LineThickness:			return &Slider2D_LineThickness;
		case StyleVar_Slider2D_CursorRadius:			return &Slider2D_CursorRadius;
		case StyleVar_Slider2D_CursorOffset:			return &Slider2D_CursorOffset;
		case StyleVar_Slider2D_CornerRadius:			return &Slider2D_CornerRadius;
		case StyleVar_Slider2DRange_FillAlpha:				return &Slider2DRange_FillAlpha;
		case StyleVar_Slider2DRange_HandleRadius:			return &Slider2DRange_HandleRadius;
		case StyleVar_Slider2DDisc_CursorRadius:			return &Slider2DDisc_CursorRadius;
		case StyleVar_Slider2DDisc_CursorOutlineThickness:	return &Slider2DDisc_CursorOutlineThickness;
		case StyleVar_Slider2DDisc_RingThickness:			return &Slider2DDisc_RingThickness;
		case StyleVar_NavCursor_Thickness:				return &NavCursor_Thickness;
		case StyleVar_NavCursor_Distance:				return &NavCursor_Distance;
		case StyleVar_WhitePoint_Radius:				return &WhitePoint_Radius;
		case StyleVar_PrecisionDrag_BlockSize:			return &PrecisionDrag_BlockSize;
		case StyleVar_Gradient_MarkerHeight:			return &Gradient_MarkerHeight;
		case StyleVar_Gradient_CheckerboardCellSize:	return &Gradient_CheckerboardCellSize;
		case StyleVar_Gradient_MarkerThickness:			return &Gradient_MarkerThickness;
		case StyleVar_CurveEditor_DefaultHeight:		return &CurveEditor_DefaultHeight;
		case StyleVar_CurveEditor_KeyRadius:			return &CurveEditor_KeyRadius;
		case StyleVar_CurveEditor_TangentRadius:		return &CurveEditor_TangentRadius;
		case StyleVar_CurveEditor_HitRadius:			return &CurveEditor_HitRadius;
		case StyleVar_CurveEditor_LineThickness:		return &CurveEditor_LineThickness;
		case StyleVar_CurveEditor_KeyOutlineThickness:	return &CurveEditor_KeyOutlineThickness;
		case StyleVar_CurveEditor_ZeroLineThickness:	return &CurveEditor_ZeroLineThickness;
		case StyleVar_ColorWheel_DotRadius:				return &ColorWheel_DotRadius;
		case StyleVar_ColorWheel_RingThickness:			return &ColorWheel_RingThickness;
		case StyleVar_ColorWheel_SliderHeight:			return &ColorWheel_SliderHeight;
		case StyleVar_SliderRing_TrackThickness:		return &SliderRing_TrackThickness;
		case StyleVar_SliderRing_GrabRadius:			return &SliderRing_GrabRadius;
		case StyleVar_SliderSpline_TrackThickness:		return &SliderSpline_TrackThickness;
		case StyleVar_SliderSpline_GrabRadius:			return &SliderSpline_GrabRadius;
		case StyleVar_ColorWarper_PointRadius:			return &ColorWarper_PointRadius;
		case StyleVar_ColorWarper_HitRadius:			return &ColorWarper_HitRadius;
		case StyleVar_ColorWarper_GridThickness:		return &ColorWarper_GridThickness;
		case StyleVar_ColorWarper_DiscScale:			return &ColorWarper_DiscScale;
		case StyleVar_ColorCurve_DefaultHeight:			return &ColorCurve_DefaultHeight;
		case StyleVar_ColorCurve_KeyRadius:				return &ColorCurve_KeyRadius;
		case StyleVar_ColorCurve_LineThickness:			return &ColorCurve_LineThickness;
		case StyleVar_ParadeScope_DefaultHeight:		return &ParadeScope_DefaultHeight;
		case StyleVar_ParadeScope_OverlayAlpha:			return &ParadeScope_OverlayAlpha;
		case StyleVar_ParadeScope_GradTickLength:		return &ParadeScope_GradTickLength;
		case StyleVar_ParadeScope_GradTickThickness:	return &ParadeScope_GradTickThickness;
		case StyleVar_ParadeScope_GradMargin:			return &ParadeScope_GradMargin;
		case StyleVar_VectorScope_DefaultSize:			return &VectorScope_DefaultSize;
		case StyleVar_VectorScope_SignalAlpha:			return &VectorScope_SignalAlpha;
		case StyleVar_VectorScope_GraticuleThickness:	return &VectorScope_GraticuleThickness;
		case StyleVar_VectorScope_DiscScale:			return &VectorScope_DiscScale;
		case StyleVar_Histogram_DefaultHeight:			return &Histogram_DefaultHeight;
		case StyleVar_Histogram_OverlayAlpha:			return &Histogram_OverlayAlpha;
		case StyleVar_Histogram_GradTickLength:			return &Histogram_GradTickLength;
		case StyleVar_Histogram_GradTickThickness:		return &Histogram_GradTickThickness;
		case StyleVar_Histogram_GradMarginLeft:			return &Histogram_GradMarginLeft;
		case StyleVar_Histogram_GradMarginBottom:		return &Histogram_GradMarginBottom;
		case StyleVar_CIEChromaticity_DefaultSize:		return &CIEChromaticity_DefaultSize;
		case StyleVar_CIEChromaticity_SignalRadius:		return &CIEChromaticity_SignalRadius;
		case StyleVar_CIEChromaticity_GamutLineThickness: return &CIEChromaticity_GamutLineThickness;
		case StyleVar_CIEChromaticity_WhitePointRadius:	return &CIEChromaticity_WhitePointRadius;
		case StyleVar_CIEChromaticity_GradMargin:		return &CIEChromaticity_GradMargin;
		case StyleVar_CIEChromaticity_SignalAlpha:		return &CIEChromaticity_SignalAlpha;
		case StyleVar_CIEChromaticity_GradMarginRight:	return &CIEChromaticity_GradMarginRight;
		case StyleVar_CIEChromaticity_GradMarginTop:	return &CIEChromaticity_GradMarginTop;
		case StyleVar_ChromaticityPlot_BorderThickness:   return &ChromaticityPlot_BorderThickness;
		case StyleVar_ChromaticityPlot_TriangleThickness: return &ChromaticityPlot_TriangleThickness;
		case StyleVar_ChromaticityLine_Thickness:         return &ChromaticityLine_Thickness;
		case StyleVar_ChromaticityPoint_Radius:           return &ChromaticityPoint_Radius;
		case StyleVar_ToneCurve_DefaultHeight:			return &ToneCurve_DefaultHeight;
		case StyleVar_ToneCurve_KeyRadius:				return &ToneCurve_KeyRadius;
		case StyleVar_ToneCurve_LineThickness:			return &ToneCurve_LineThickness;
		case StyleVar_ToneCurve_GradMarginLeft:			return &ToneCurve_GradMarginLeft;
		case StyleVar_ToneCurve_GradMarginBottom:		return &ToneCurve_GradMarginBottom;
		case StyleVar_ToneCurve_BandThickness:			return &ToneCurve_BandThickness;
		case StyleVar_ToneCurve_BandGap:				return &ToneCurve_BandGap;
		case StyleVar_ToneCurve_GradMarginRight:		return &ToneCurve_GradMarginRight;
		case StyleVar_ToneCurve_GradMarginTop:			return &ToneCurve_GradMarginTop;
		case StyleVar_HDRWheel_RingThickness:			return &HDRWheel_RingThickness;
		case StyleVar_HDRWheel_RingGap:					return &HDRWheel_RingGap;
		case StyleVar_Gizmo_HandleSize:					return &Gizmo_HandleSize;
		case StyleVar_Gizmo_RotationHandleOffset:		return &Gizmo_RotationHandleOffset;
		case StyleVar_Gizmo_OutlineThickness:			return &Gizmo_OutlineThickness;
		case StyleVar_Gizmo_RotationLineThickness:		return &Gizmo_RotationLineThickness;
		case StyleVar_Gizmo_CenterDotRadius:			return &Gizmo_CenterDotRadius;
		case StyleVar_Gizmo_EdgeHandleScale:			return &Gizmo_EdgeHandleScale;
		case StyleVar_Gizmo_DefaultCanvasHeight:		return &Gizmo_DefaultCanvasHeight;
		case StyleVar_ColorPicker_DotRadius:			return &ColorPicker_DotRadius;
		case StyleVar_ColorPicker_SliderWidth:			return &ColorPicker_SliderWidth;
		case StyleVar_ColorPicker_ComponentSliderHeight:	return &ColorPicker_ComponentSliderHeight;
		case StyleVar_ColorPicker_DotOutlineThickness:		return &ColorPicker_DotOutlineThickness;
		case StyleVar_ColorPicker_SliderHandleHeight:		return &ColorPicker_SliderHandleHeight;
		case StyleVar_ColorPicker_ComponentHandleWidth:		return &ColorPicker_ComponentHandleWidth;
		case StyleVar_Hatch_DefaultSpacing:				return &Hatch_DefaultSpacing;
		case StyleVar_Hatch_DefaultThickness:			return &Hatch_DefaultThickness;
		case StyleVar_BezierCurve_KeyRadius:			return &BezierCurve_KeyRadius;
		case StyleVar_BezierCurve_TangentRadius:		return &BezierCurve_TangentRadius;
		case StyleVar_BezierCurve_LineThickness:		return &BezierCurve_LineThickness;
		case StyleVar_BezierCurve_SnapAngleDeg:			return &BezierCurve_SnapAngleDeg;
		case StyleVar_FontInspector_GlyphCell:			return &FontInspector_GlyphCell;
		case StyleVar_FontInspector_MetricAlpha:		return &FontInspector_MetricAlpha;
		case StyleVar_NotchedDial_TickLength:			return &NotchedDial_TickLength;
		case StyleVar_NotchedDial_TickThickness:		return &NotchedDial_TickThickness;
		case StyleVar_DeltaE_SwatchSize:				return &DeltaE_SwatchSize;
		case StyleVar_Equation_BlockPadding:			return &Equation_BlockPadding;
		case StyleVar_Equation_InlineBaselineOffset:	return &Equation_InlineBaselineOffset;
		case StyleVar_IsoContour_MajorThickness:		return &IsoContour_MajorThickness;
		case StyleVar_IsoContour_MediumThickness:		return &IsoContour_MediumThickness;
		case StyleVar_IsoContour_MinorThickness:		return &IsoContour_MinorThickness;
		case StyleVar_LookDev_DividerThickness:			return &LookDev_DividerThickness;
		case StyleVar_LookDev_HandleRadius:				return &LookDev_HandleRadius;
		case StyleVar_VolumeSlice_HandleRadius:			return &VolumeSlice_HandleRadius;
		case StyleVar_VectorDrawing_AnchorRadius:			return &VectorDrawing_AnchorRadius;
		case StyleVar_VectorDrawing_TangentRadius:			return &VectorDrawing_TangentRadius;
		case StyleVar_VectorDrawing_GridSpacing:			return &VectorDrawing_GridSpacing;
		case StyleVar_VectorDrawing_TangentLineThickness:	return &VectorDrawing_TangentLineThickness;
		case StyleVar_VectorDrawing_AnchorOutlineThickness:	return &VectorDrawing_AnchorOutlineThickness;
		case StyleVar_VectorDrawing_ClosingRingRadius:		return &VectorDrawing_ClosingRingRadius;
		case StyleVar_VectorDrawing_ClosingRingThickness:	return &VectorDrawing_ClosingRingThickness;
		case StyleVar_VectorDrawing_HoverRingGap:			return &VectorDrawing_HoverRingGap;
		case StyleVar_VectorDrawing_HoverRingThickness:		return &VectorDrawing_HoverRingThickness;
		case StyleVar_VectorDrawing_GhostRingThickness:		return &VectorDrawing_GhostRingThickness;
		case StyleVar_VectorDrawing_CrosshairArmLength:		return &VectorDrawing_CrosshairArmLength;
		case StyleVar_VectorDrawing_CrosshairThickness:		return &VectorDrawing_CrosshairThickness;
		case StyleVar_PrecisionPopup_InputWidth:			return &PrecisionPopup_InputWidth;
		default:										return nullptr;
		}
	}
	ImVec2* GetVarVec2Addr( ImWidgetsStyleVar idx )
	{
		IM_UNUSED( idx );

		return NULL;
	}
	ImVec4* GetVarVec4Addr( ImWidgetsStyleVar idx )
	{
		IM_UNUSED( idx );

		return NULL;
	}

	ImVector<ColorModifier>	m_ColorStack;
	ImVector<VarModifier>	m_VarStack;
};

typedef int ImWidgetsLengthUnit;
typedef int ImWidgetsChromaticPlot;
typedef int ImWidgetsObserver;
typedef int ImWidgetsIlluminant;
typedef int ImWidgetsColorSpace;
typedef int ImWidgetsPointer;

enum ImWidgetsLengthUnit_
{
	ImWidgetsLengthUnit_Metric = 0,
	ImWidgetsLengthUnit_Imperial,
	ImWidgetsLengthUnit_COUNT
};

enum ImWidgetsObserver_
{
	// Standard
	ImWidgetsObserver_CIE1931_2deg = 0,
	ImWidgetsObserver_CIE1964_10deg,
	ImWidgetsObserver_COUNT
};

enum ImWidgetsIlluminant_
{
	// White Points
	ImWidgetsIlluminant_A = 0,
	ImWidgetsIlluminant_B,
	ImWidgetsIlluminant_C,
	ImWidgetsIlluminant_D50,
	ImWidgetsIlluminant_D55,
	ImWidgetsIlluminant_D65,
	ImWidgetsIlluminant_D75,
	ImWidgetsIlluminant_D93,
	ImWidgetsIlluminant_E,
	ImWidgetsIlluminant_F1,
	ImWidgetsIlluminant_F2,
	ImWidgetsIlluminant_F3,
	ImWidgetsIlluminant_F4,
	ImWidgetsIlluminant_F5,
	ImWidgetsIlluminant_F6,
	ImWidgetsIlluminant_F7,
	ImWidgetsIlluminant_F8,
	ImWidgetsIlluminant_F9,
	ImWidgetsIlluminant_F10,
	ImWidgetsIlluminant_F11,
	ImWidgetsIlluminant_F12,
	ImWidgetsIlluminant_COUNT
};

enum ImWidgetsColorSpace_
{
	// Color Spaces
	ImWidgetsColorSpace_AdobeRGB = 0,	// D65
	ImWidgetsColorSpace_AppleRGB,		// D65
	ImWidgetsColorSpace_Best,			// D50
	ImWidgetsColorSpace_Beta,			// D50
	ImWidgetsColorSpace_Bruce,			// D65
	ImWidgetsColorSpace_CIERGB,			// E
	ImWidgetsColorSpace_ColorMatch,		// D50
	ImWidgetsColorSpace_Don_RGB_4,		// D50
	ImWidgetsColorSpace_ECI,			// D50
	ImWidgetsColorSpace_Ekta_Space_PS5,	// D50
	ImWidgetsColorSpace_NTSC,			// C
	ImWidgetsColorSpace_PAL_SECAM,		// D65
	ImWidgetsColorSpace_ProPhoto,		// D50
	ImWidgetsColorSpace_SMPTE_C,		// D65
	ImWidgetsColorSpace_sRGB,			// D65
	ImWidgetsColorSpace_WideGamutRGB,	// D50
	ImWidgetsColorSpace_Rec2020,		// D65
	ImWidgetsColorSpace_COUNT
};

enum ImWidgetsChromaticPlot_
{
	// Style
	ImWidgetsChromaticPlot_ShowWavelength,
	ImWidgetsChromaticPlot_ShowGrid,
	ImWidgetsChromaticPlot_ShowPrimaries,
	ImWidgetsChromaticPlot_ShowWhitePoint,

	ImWidgetsChromaticPlot_COUNT
};

enum ImWidgetsMarker_
{
	// Style
	ImWidgetsMarker_Disc,
	ImWidgetsMarker_Square,
	ImWidgetsMarker_Triangle,
	ImWidgetsMarker_Diamond,
	ImWidgetsMarker_Heart,
	ImWidgetsMarker_Spade,
	ImWidgetsMarker_Club,
	ImWidgetsMarker_Chevron,
	ImWidgetsMarker_Clover,
	ImWidgetsMarker_Ring,
	ImWidgetsMarker_Tag,
	ImWidgetsMarker_Cross,
	ImWidgetsMarker_Asterisk,
	ImWidgetsMarker_Infinity,
	ImWidgetsMarker_Pin,
	ImWidgetsMarker_Arrow,
	ImWidgetsMarker_Ellipse,
	ImWidgetsMarker_EllipseApprox,

	ImWidgetsMarker_COUNT
};
typedef int ImWidgetsMarker;

enum ImWidgetsDrawType_
{
	// Style
	ImWidgetsDrawType_Filled,
	ImWidgetsDrawType_Stroke,
	ImWidgetsDrawType_Outline,
	ImWidgetsDrawType_SignedDistanceField,
	ImWidgetsDrawType_CutOff,

	ImWidgetsDrawType_COUNT
};
typedef int ImWidgetsDrawType;

enum ImWidgetsCap_
{
	ImWidgetsCap_None,
	ImWidgetsCap_Butt,
	ImWidgetsCap_Square,
	ImWidgetsCap_Round,
	ImWidgetsCap_TriangleOut,
	ImWidgetsCap_TriangleIn,

	ImWidgetsCap_COUNT
};
typedef int ImWidgetsCap;

enum ImWidgetsJoin_
{
    ImWidgetsJoin_Round,
    ImWidgetsJoin_Mitter,
    ImWidgetsJoin_Bevel,

    ImWidgetsJoin_COUNT
};
typedef int ImWidgetsJoin;

enum ImWidgetsPrimitive_
{
    ImWidgetsPrimitive_Line,
    ImWidgetsPrimitive_Arc,

    ImWidgetsPrimitive_COUNT
};
typedef int ImWidgetsPrimitive;

enum ImWidgetsCorrectness_
{
    ImWidgetsCorrectness_Weak,
    ImWidgetsCorrectness_Strong,

    ImWidgetsCorrectness_COUNT
};
typedef int ImWidgetsCorrectness;

// (ImWidgetsThickLineMode_ enum is declared before ImWidgetsStyle so the constructor
//  can reference its values; the descriptor struct lives here next to ImWidgetsCap_/Join_.)

// Descriptor passed to DrawThickLine. 32-bit fields first (no alignment holes); bool last.
struct ImWidgetsThickLineDesc
{
    ImU32                  color       = IM_COL32_WHITE;
    ImWidgetsThickLineMode mode        = ImWidgetsThickLineMode_AddPolyline;
    ImWidgetsCap           cap         = ImWidgetsCap_Butt;
    ImWidgetsJoin          join        = ImWidgetsJoin_Round;
    float                  thickness   = 1.0f;     // px
    float                  miter_limit = 4.0f;
    float                  dash_len    = 8.0f;     // px (used only when dashed=true)
    float                  gap_len     = 4.0f;     // px
    float                  dash_offset = 0.0f;     // px
    float                  tolerance   = 0.25f;    // px max chord error (Bezier modes)
    bool                   dashed      = false;
};

typedef int ImWidgetsGradientInterp;
enum ImWidgetsGradientInterp_
{
	ImWidgetsGradientInterp_sRGB = 0,
	ImWidgetsGradientInterp_LinearSRGB,
	ImWidgetsGradientInterp_OkLab,
	ImWidgetsGradientInterp_OkLCH,
	ImWidgetsGradientInterp_HSV,

	ImWidgetsGradientInterp_COUNT
};

struct ImGradientStop
{
	float	Position;	// [0, 1]
	ImVec4	Color;		// RGBA in sRGB, float [0,1]

	ImGradientStop() : Position( 0.0f ), Color( 1.0f, 1.0f, 1.0f, 1.0f ) {}
	ImGradientStop( float pos, ImVec4 col ) : Position( pos ), Color( col ) {}
};

struct ImGradientAlphaStop
{
	float	Position;	// [0, 1]
	float	Alpha;		// [0, 1]

	ImGradientAlphaStop() : Position( 0.0f ), Alpha( 1.0f ) {}
	ImGradientAlphaStop( float pos, float a ) : Position( pos ), Alpha( a ) {}
};

struct ImGradientData
{
	ImVector<ImGradientStop>		Stops;
	ImVector<ImGradientAlphaStop>	AlphaStops;		// Used when SplitAlpha == true
	ImWidgetsGradientInterp			Interpolation;
	int								SelectedIdx;		// Runtime state for editor, -1 = none
	int								SelectedAlphaIdx;	// Runtime state for alpha track, -1 = none
	bool							SplitAlpha;			// When true, color and alpha have independent keys

	ImGradientData() : Interpolation( ImWidgetsGradientInterp_sRGB ), SelectedIdx( -1 ), SelectedAlphaIdx( -1 ), SplitAlpha( false )
	{
		Stops.resize( 2 );
		Stops[ 0 ] = ImGradientStop( 0.0f, ImVec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
		Stops[ 1 ] = ImGradientStop( 1.0f, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
		AlphaStops.resize( 2 );
		AlphaStops[ 0 ] = ImGradientAlphaStop( 0.0f, 1.0f );
		AlphaStops[ 1 ] = ImGradientAlphaStop( 1.0f, 1.0f );
	}

	void SortStops()
	{
		// Insertion sort (small N, no STL)
		for ( int i = 1; i < Stops.Size; ++i )
		{
			ImGradientStop key = Stops[ i ];
			int j = i - 1;
			while ( j >= 0 && Stops[ j ].Position > key.Position )
			{
				Stops[ j + 1 ] = Stops[ j ];
				--j;
			}
			Stops[ j + 1 ] = key;
		}
	}

	void SortAlphaStops()
	{
		for ( int i = 1; i < AlphaStops.Size; ++i )
		{
			ImGradientAlphaStop key = AlphaStops[ i ];
			int j = i - 1;
			while ( j >= 0 && AlphaStops[ j ].Position > key.Position )
			{
				AlphaStops[ j + 1 ] = AlphaStops[ j ];
				--j;
			}
			AlphaStops[ j + 1 ] = key;
		}
	}

	int AddStop( float pos, ImVec4 col )
	{
		Stops.push_back( ImGradientStop( pos, col ) );
		SortStops();
		for ( int i = 0; i < Stops.Size; ++i )
		{
			if ( Stops[ i ].Position == pos )
				return i;
		}
		return Stops.Size - 1;
	}

	int AddAlphaStop( float pos, float a )
	{
		AlphaStops.push_back( ImGradientAlphaStop( pos, a ) );
		SortAlphaStops();
		for ( int i = 0; i < AlphaStops.Size; ++i )
		{
			if ( AlphaStops[ i ].Position == pos )
				return i;
		}
		return AlphaStops.Size - 1;
	}

	bool RemoveStop( int idx )
	{
		if ( Stops.Size <= 2 || idx < 0 || idx >= Stops.Size )
			return false;
		Stops.erase( Stops.Data + idx );
		return true;
	}

	bool RemoveAlphaStop( int idx )
	{
		if ( AlphaStops.Size <= 2 || idx < 0 || idx >= AlphaStops.Size )
			return false;
		AlphaStops.erase( AlphaStops.Data + idx );
		return true;
	}
};

typedef int ImCurveEditorSeg;
enum ImCurveEditorSeg_
{
	// Step (piecewise constant)
	ImCurveEditorSeg_StepStart = 0,	// Hold left value, jump at right key
	ImCurveEditorSeg_StepEnd,		// Jump to right value immediately
	ImCurveEditorSeg_StepCenter,	// Jump at midpoint

	// Linear (piecewise linear)
	ImCurveEditorSeg_Linear,

	// Quadratic
	ImCurveEditorSeg_InQuad,
	ImCurveEditorSeg_OutQuad,
	ImCurveEditorSeg_InOutQuad,

	// Cubic
	ImCurveEditorSeg_InCubic,
	ImCurveEditorSeg_OutCubic,
	ImCurveEditorSeg_InOutCubic,

	// Quartic
	ImCurveEditorSeg_InQuart,
	ImCurveEditorSeg_OutQuart,
	ImCurveEditorSeg_InOutQuart,

	// Quintic
	ImCurveEditorSeg_InQuint,
	ImCurveEditorSeg_OutQuint,
	ImCurveEditorSeg_InOutQuint,

	// Sine
	ImCurveEditorSeg_InSine,
	ImCurveEditorSeg_OutSine,
	ImCurveEditorSeg_InOutSine,

	// Exponential
	ImCurveEditorSeg_InExpo,
	ImCurveEditorSeg_OutExpo,
	ImCurveEditorSeg_InOutExpo,

	// Circular
	ImCurveEditorSeg_InCirc,
	ImCurveEditorSeg_OutCirc,
	ImCurveEditorSeg_InOutCirc,

	// Back (overshoot)
	ImCurveEditorSeg_InBack,
	ImCurveEditorSeg_OutBack,
	ImCurveEditorSeg_InOutBack,

	// Elastic
	ImCurveEditorSeg_InElastic,
	ImCurveEditorSeg_OutElastic,
	ImCurveEditorSeg_InOutElastic,

	// Bounce
	ImCurveEditorSeg_InBounce,
	ImCurveEditorSeg_OutBounce,
	ImCurveEditorSeg_InOutBounce,

	// Cubic Bezier (with tangent handles)
	ImCurveEditorSeg_CubicBezier,

	ImCurveEditorSeg_COUNT
};

typedef int ImCurveEditorTangentMode;
enum ImCurveEditorTangentMode_
{
	ImCurveEditorTangentMode_Free = 0,		// Left and right handles move independently
	ImCurveEditorTangentMode_Aligned,		// Handles stay collinear but can have different lengths
	ImCurveEditorTangentMode_Mirrored,		// Handles stay collinear and same length (symmetric)
	ImCurveEditorTangentMode_COUNT
};

struct ImCurveEditorKey
{
	ImVec2						Pos;			// (x=time, y=value)
	ImCurveEditorSeg			Segment;		// Interpolation to next key
	ImVec2						TangentLeft;	// Incoming tangent handle offset (typically negative x)
	ImVec2						TangentRight;	// Outgoing tangent handle offset (typically positive x)
	ImCurveEditorTangentMode	TangentMode;	// How left/right handles relate

	ImCurveEditorKey() : Pos( 0.0f, 0.0f ), Segment( ImCurveEditorSeg_CubicBezier ),
		TangentLeft( -0.1f, 0.0f ), TangentRight( 0.1f, 0.0f ), TangentMode( ImCurveEditorTangentMode_Mirrored ) {}
	ImCurveEditorKey( ImVec2 pos, ImCurveEditorSeg seg = ImCurveEditorSeg_CubicBezier )
		: Pos( pos ), Segment( seg ),
		TangentLeft( -0.1f, 0.0f ), TangentRight( 0.1f, 0.0f ), TangentMode( ImCurveEditorTangentMode_Mirrored ) {}
};

struct ImCurveEditorData
{
	ImVector<ImCurveEditorKey>	Keys;
	ImVec2						RangeMin;		// Visible range min (x=time, y=value)
	ImVec2						RangeMax;		// Visible range max
	int							SelectedIdx;	// Selected key, -1 = none
	int							DragTarget;		// 0 = key, -1 = left tangent, 1 = right tangent

	ImCurveEditorData()
		: RangeMin( 0.0f, 0.0f ), RangeMax( 1.0f, 1.0f ),
		SelectedIdx( -1 ), DragTarget( 0 )
	{
		Keys.resize( 2 );
		Keys[ 0 ] = ImCurveEditorKey( ImVec2( 0.0f, 0.0f ) );
		Keys[ 1 ] = ImCurveEditorKey( ImVec2( 1.0f, 1.0f ) );
	}

	void SortKeys()
	{
		for ( int i = 1; i < Keys.Size; ++i )
		{
			ImCurveEditorKey key = Keys[ i ];
			int j = i - 1;
			while ( j >= 0 && Keys[ j ].Pos.x > key.Pos.x )
			{
				Keys[ j + 1 ] = Keys[ j ];
				--j;
			}
			Keys[ j + 1 ] = key;
		}
	}

	int AddKey( ImVec2 pos, ImCurveEditorSeg seg = ImCurveEditorSeg_CubicBezier )
	{
		// Capture neighbors before insertion (push_back may reallocate)
		ImCurveEditorKey prevKey, nextKey;
		bool hasPrev = false, hasNext = false;
		for ( int i = 0; i < Keys.Size; ++i )
		{
			if ( Keys[ i ].Pos.x < pos.x )
				{ prevKey = Keys[ i ]; hasPrev = true; }
			else if ( Keys[ i ].Pos.x > pos.x && !hasNext )
				{ nextKey = Keys[ i ]; hasNext = true; }
		}

		Keys.push_back( ImCurveEditorKey( pos, seg ) );
		SortKeys();
		int newIdx = Keys.Size - 1;
		for ( int i = 0; i < Keys.Size; ++i )
		{
			if ( Keys[ i ].Pos.x == pos.x && Keys[ i ].Pos.y == pos.y )
				{ newIdx = i; break; }
		}

		// Evaluate the existing Bezier tangent at the insertion point via De Casteljau
		if ( seg == ImCurveEditorSeg_CubicBezier && hasPrev && hasNext )
		{
			float segW = nextKey.Pos.x - prevKey.Pos.x;
			if ( segW > 1e-6f )
			{
				float t    = ( pos.x - prevKey.Pos.x ) / segW;
				// Control points
				float P0x = prevKey.Pos.x,                          P0y = prevKey.Pos.y;
				float P1x = P0x + prevKey.TangentRight.x,           P1y = P0y + prevKey.TangentRight.y;
				float P2x = nextKey.Pos.x + nextKey.TangentLeft.x,  P2y = nextKey.Pos.y + nextKey.TangentLeft.y;
				float P3x = nextKey.Pos.x,                          P3y = nextKey.Pos.y;
				// De Casteljau level 1
				float P01x  = P0x  + t*(P1x-P0x),   P01y  = P0y  + t*(P1y-P0y);
				float P12x  = P1x  + t*(P2x-P1x),   P12y  = P1y  + t*(P2y-P1y);
				float P23x  = P2x  + t*(P3x-P2x),   P23y  = P2y  + t*(P3y-P2y);
				// De Casteljau level 2
				float P012x = P01x + t*(P12x-P01x), P012y = P01y + t*(P12y-P01y);
				float P123x = P12x + t*(P23x-P12x), P123y = P12y + t*(P23y-P12y);
				// De Casteljau level 3 -- split point
				float P0123x = P012x + t*(P123x-P012x), P0123y = P012y + t*(P123y-P012y);
				// Tangent handles relative to the split point
				Keys[ newIdx ].TangentLeft  = ImVec2( P012x - P0123x, P012y - P0123y );
				Keys[ newIdx ].TangentRight = ImVec2( P123x - P0123x, P123y - P0123y );
				Keys[ newIdx ].TangentMode  = ImCurveEditorTangentMode_Aligned;
			}
		}
		return newIdx;
	}

	bool RemoveKey( int idx )
	{
		if ( Keys.Size <= 2 || idx < 0 || idx >= Keys.Size )
			return false;
		Keys.erase( Keys.Data + idx );
		return true;
	}
};

// Control mesh for the GridWarp widget: a Cols x Rows lattice of 2-D offsets
// (normalized canvas units, [-1,1]) applied on top of the uniform base grid.
// Offsets are stored row-major; (0,0) at every node = identity warp.
struct ImGridWarpData
{
	int					Cols;
	int					Rows;
	ImVector<ImVec2>	Offsets;	// size Cols*Rows, row-major; node offset
	int					SelectedIdx;	// dragged/selected node, -1 = none

	ImGridWarpData() : Cols( 0 ), Rows( 0 ), SelectedIdx( -1 ) { Init( 4, 4 ); }

	void Init( int cols, int rows )
	{
		Cols = cols < 2 ? 2 : cols;
		Rows = rows < 2 ? 2 : rows;
		Offsets.resize( Cols * Rows );
		for ( int i = 0; i < Offsets.Size; ++i )
			Offsets[ i ] = ImVec2( 0.0f, 0.0f );
		SelectedIdx = -1;
	}

	void Reset()
	{
		for ( int i = 0; i < Offsets.Size; ++i )
			Offsets[ i ] = ImVec2( 0.0f, 0.0f );
	}

	ImVec2&			At( int c, int r )			{ return Offsets[ r * Cols + c ]; }
	ImVec2 const&	At( int c, int r ) const	{ return Offsets[ r * Cols + c ]; }

	// Uniform base position of node (c,r) in [0,1]^2.
	ImVec2 Base( int c, int r ) const
	{
		return ImVec2( Cols > 1 ? (float)c / (float)( Cols - 1 ) : 0.5f,
		               Rows > 1 ? (float)r / (float)( Rows - 1 ) : 0.5f );
	}
};

struct ImTransformData
{
	ImVec2	Translation;	// Offset from canvas center (lp)
	float	Rotation;		// Rotation angle (radians)
	ImVec2	Scale;			// Scale factor (1,1 = fit to canvas)

	ImTransformData() : Translation( 0.0f, 0.0f ), Rotation( 0.0f ), Scale( 1.0f, 1.0f ) {}
};

typedef int ImTransformGizmoFlags;
enum ImTransformGizmoFlags_
{
	ImTransformGizmoFlags_None           = 0,
	ImTransformGizmoFlags_NonUniformScale = 1 << 0,		// Show edge midpoint handles for non-uniform scaling
};

// Params passed to the draw callback for each object
struct ImTransformGizmoDrawParams
{
	ImDrawList*	DrawList;
	ImVec2		Center;			// screen-space center
	ImVec2		Corners[4];		// screen-space corners: TL, TR, BR, BL
	float		HalfW, HalfH;	// half-extents after scale
	float		CosR, SinR;		// rotation basis
	int			Index;
};
typedef void (*ImTransformGizmoDrawFn)( ImTransformGizmoDrawParams const& params, void* user_data );

// Optional: called when the expanded panel reorders two objects
typedef void (*ImTransformGizmoSwapFn)( int indexA, int indexB, void* user_data );

struct ImTransformGizmoCallbacks
{
	ImTransformGizmoDrawFn	DrawFn   = nullptr;
	void*					DrawData = nullptr;
	ImTransformGizmoSwapFn	SwapFn   = nullptr;
	void*					SwapData = nullptr;
};

typedef int ImColorWheelMode;
enum ImColorWheelMode_
{
	ImColorWheelMode_HSV = 0,	// Hue-Saturation disc, Value on master slider
	ImColorWheelMode_OkLCH,		// Perceptually uniform: Hue-Chroma disc, Lightness on master slider
	ImColorWheelMode_COUNT
};

typedef int ImColorPickerSpace;
enum ImColorPickerSpace_
{
	ImColorPickerSpace_sRGB = 0,
	ImColorPickerSpace_HSV,
	ImColorPickerSpace_OkLab,
	ImColorPickerSpace_OkLCH,
	ImColorPickerSpace_CIELab,
	ImColorPickerSpace_XYZ,
	ImColorPickerSpace_COUNT
};

typedef int ImColorWarperMode;
enum ImColorWarperMode_
{
	ImColorWarperMode_Circular = 0,		// Hue/Sat disc layout (polar mesh)
	ImColorWarperMode_Square,			// Square grid layout (cartesian mesh)
	ImColorWarperMode_ChromaLuma,		// Two squares: Yellow-Blue vs Luma + Green-Red vs Luma
	ImColorWarperMode_COUNT
};

typedef int ImColorWarperSpace;
enum ImColorWarperSpace_
{
	ImColorWarperSpace_HSV = 0,			// Hue-Saturation-Value
	ImColorWarperSpace_HSL,				// Hue-Saturation-Lightness
	ImColorWarperSpace_HSY,				// Hue-Saturation-Luma (BT.709)
	ImColorWarperSpace_HSP,				// Hue-Saturation-Perceived brightness
	ImColorWarperSpace_HSPLog,			// Hue-Saturation-Perceived brightness (log scale)
	ImColorWarperSpace_OkLab,			// OkLab (perceptually uniform, L on third axis)
	ImColorWarperSpace_OkLCH,			// OkLCH (perceptually uniform, L on third axis)
	ImColorWarperSpace_COUNT
};

typedef int ImColorWarperSignalColor;
enum ImColorWarperSignalColor_
{
	ImColorWarperSignalColor_Flat = 0,			// Use style color for all points
	ImColorWarperSignalColor_PixelColor,		// Use each pixel's own RGB color
	ImColorWarperSignalColor_COUNT
};

struct ImColorWarperOverlay;

struct ImColorWarperData
{
	int		HueDivisions;		// Number of divisions around hue (6, 12, or 24)
	int		SatDivisions;		// Number of divisions along saturation
	ImVector<ImVec2> Offsets;	// Per-point (hue_shift, sat_shift) offsets
								// Size = (SatDivisions+1) * HueDivisions
								// Index: satIdx * HueDivisions + hueIdx
								// satIdx=0 is center, satIdx=SatDivisions is outer edge
	ImVector<bool>   Pinned;	// Per-point pin state (locked in place)
	int		SelectedIdx;		// Currently selected/dragged point, -1 = none

	ImColorWarperData() : HueDivisions( 12 ), SatDivisions( 6 ), SelectedIdx( -1 ) {}

	void Init( int hueDivs = 12, int satDivs = 6 )
	{
		HueDivisions = hueDivs;
		SatDivisions = satDivs;
		int count = ( SatDivisions + 1 ) * HueDivisions;
		Offsets.resize( count );
		Pinned.resize( count );
		Reset();
	}

	void Reset()
	{
		for ( int i = 0; i < Offsets.Size; ++i )
		{
			Offsets[ i ] = ImVec2( 0.0f, 0.0f );
			Pinned[ i ] = false;
		}
		SelectedIdx = -1;
	}

	int PointCount() const { return ( SatDivisions + 1 ) * HueDivisions; }

	int PointIndex( int hueIdx, int satIdx ) const
	{
		return satIdx * HueDivisions + ( hueIdx % HueDivisions );
	}

	// Identity (unwarped) position: (hue 0..1, sat 0..1)
	ImVec2 GetIdentityPos( int hueIdx, int satIdx ) const
	{
		float h = ( float )( hueIdx % HueDivisions ) / ( float )HueDivisions;
		float s = ( float )satIdx / ( float )SatDivisions;
		return ImVec2( h, s );
	}

	// Warped position for a grid point
	ImVec2 GetWarpedPos( int hueIdx, int satIdx ) const
	{
		int idx = PointIndex( hueIdx, satIdx );
		ImVec2 id = GetIdentityPos( hueIdx, satIdx );
		float h = id.x + Offsets[ idx ].x;
		float s = ImClamp( id.y + Offsets[ idx ].y, 0.0f, 1.0f );
		// Wrap hue
		h = h - ImFloor( h );
		if ( h < 0.0f ) h += 1.0f;
		return ImVec2( h, s );
	}
};

typedef int ImColorCurveMode;
enum ImColorCurveMode_
{
	ImColorCurveMode_HueVsHue = 0,	// X=Hue, Y=Hue shift (-0.5..0.5)
	ImColorCurveMode_HueVsSat,		// X=Hue, Y=Saturation multiplier (0..2)
	ImColorCurveMode_HueVsLum,		// X=Hue, Y=Luminance offset (-1..1)
	ImColorCurveMode_LumVsSat,		// X=Luminance, Y=Saturation multiplier (0..2)
	ImColorCurveMode_SatVsSat,		// X=Saturation, Y=Saturation multiplier (0..2)
	ImColorCurveMode_COUNT
};

struct ImColorCurveKey
{
	float Position;		// X position (0-1)
	float Value;		// Y value (meaning depends on mode)

	// Advanced segment fields (used only when advancedSegments is enabled)
	ImCurveEditorSeg			Segment;		// Interpolation to next key
	ImVec2						TangentLeft;	// Left tangent handle (for CubicBezier)
	ImVec2						TangentRight;	// Right tangent handle (for CubicBezier)
	ImCurveEditorTangentMode	TangentMode;	// How left/right handles relate

	ImColorCurveKey() : Position( 0.0f ), Value( 0.0f ), Segment( ImCurveEditorSeg_CubicBezier ),
		TangentLeft( -0.1f, 0.0f ), TangentRight( 0.1f, 0.0f ), TangentMode( ImCurveEditorTangentMode_Mirrored ) {}
	ImColorCurveKey( float pos, float val ) : Position( pos ), Value( val ), Segment( ImCurveEditorSeg_CubicBezier ),
		TangentLeft( -0.1f, 0.0f ), TangentRight( 0.1f, 0.0f ), TangentMode( ImCurveEditorTangentMode_Mirrored ) {}
};

struct ImColorCurveData
{
	ImVector<ImColorCurveKey>	Keys;
	int							SelectedIdx;

	ImColorCurveData() : SelectedIdx( -1 ) {}

	void SortKeys()
	{
		for ( int i = 1; i < Keys.Size; ++i )
		{
			ImColorCurveKey key = Keys[ i ];
			int j = i - 1;
			while ( j >= 0 && Keys[ j ].Position > key.Position )
			{
				Keys[ j + 1 ] = Keys[ j ];
				--j;
			}
			Keys[ j + 1 ] = key;
		}
	}

	int AddKey( float pos, float val )
	{
		// Capture neighbors before insertion (push_back may reallocate)
		ImColorCurveKey prevKey, nextKey;
		bool hasPrev = false, hasNext = false;
		for ( int i = 0; i < Keys.Size; ++i )
		{
			if ( Keys[ i ].Position < pos )
				{ prevKey = Keys[ i ]; hasPrev = true; }
			else if ( Keys[ i ].Position > pos && !hasNext )
				{ nextKey = Keys[ i ]; hasNext = true; }
		}

		Keys.push_back( ImColorCurveKey( pos, val ) );
		SortKeys();
		int newIdx = Keys.Size - 1;
		for ( int i = 0; i < Keys.Size; ++i )
		{
			if ( Keys[ i ].Position == pos )
				{ newIdx = i; break; }
		}

		// Evaluate the existing Bezier tangent at the insertion point via De Casteljau
		if ( hasPrev && hasNext )
		{
			float segW = nextKey.Position - prevKey.Position;
			if ( segW > 1e-6f )
			{
				float t    = ( pos - prevKey.Position ) / segW;
				// Control points
				float P0x = prevKey.Position,                            P0y = prevKey.Value;
				float P1x = P0x + prevKey.TangentRight.x,               P1y = P0y + prevKey.TangentRight.y;
				float P2x = nextKey.Position + nextKey.TangentLeft.x,   P2y = nextKey.Value + nextKey.TangentLeft.y;
				float P3x = nextKey.Position,                            P3y = nextKey.Value;
				// De Casteljau level 1
				float P01x  = P0x  + t*(P1x-P0x),   P01y  = P0y  + t*(P1y-P0y);
				float P12x  = P1x  + t*(P2x-P1x),   P12y  = P1y  + t*(P2y-P1y);
				float P23x  = P2x  + t*(P3x-P2x),   P23y  = P2y  + t*(P3y-P2y);
				// De Casteljau level 2
				float P012x = P01x + t*(P12x-P01x), P012y = P01y + t*(P12y-P01y);
				float P123x = P12x + t*(P23x-P12x), P123y = P12y + t*(P23y-P12y);
				// De Casteljau level 3 -- split point
				float P0123x = P012x + t*(P123x-P012x), P0123y = P012y + t*(P123y-P012y);
				// Tangent handles relative to the split point
				Keys[ newIdx ].TangentLeft  = ImVec2( P012x - P0123x, P012y - P0123y );
				Keys[ newIdx ].TangentRight = ImVec2( P123x - P0123x, P123y - P0123y );
				Keys[ newIdx ].TangentMode  = ImCurveEditorTangentMode_Aligned;
			}
		}
		return newIdx;
	}

	bool RemoveKey( int idx )
	{
		if ( Keys.Size <= 0 || idx < 0 || idx >= Keys.Size )
			return false;
		Keys.erase( Keys.Data + idx );
		return true;
	}
};

// ---- Parade Scope ----

typedef int ImPixelBitDepth;
enum ImPixelBitDepth_
{
	ImPixelBitDepth_UInt8 = 0,		// 8-bit  [0, 255]
	ImPixelBitDepth_UInt10,		// 10-bit stored in uint16 [0, 1023]
	ImPixelBitDepth_UInt16,		// 16-bit [0, 65535]
	ImPixelBitDepth_COUNT
};

typedef int ImParadeMode;
enum ImParadeMode_
{
	ImParadeMode_Luma = 0,			// Single luminance waveform
	ImParadeMode_RGB,				// R, G, B side by side
	ImParadeMode_YRGB,				// Y, R, G, B side by side
	ImParadeMode_YCbCr,			// Y, Cb, Cr side by side
	ImParadeMode_COUNT
};

typedef int ImPixelLayout;
enum ImPixelLayout_
{
	ImPixelLayout_Interleaved = 0,	// RGBRGB... or RGBARGBA...
	ImPixelLayout_Planar,			// RRR...GGG...BBB...
	ImPixelLayout_COUNT
};

typedef int ImParadeScale;
enum ImParadeScale_
{
	ImParadeScale_Linear = 0,		// Linear mapping
	ImParadeScale_Log,				// Log2 -- expands shadows / darks
	ImParadeScale_InvLog,			// Inverse log2 -- expands highlights / brights
	ImParadeScale_COUNT
};

struct ImParadeScopeData
{
	ImVector<ImU32>	Bins;			// [ch * XBins * YBins + x * YBins + y]
	int				XBins;
	int				YBins;
	int				ChannelCount;	// Display channels (1..4 depending on mode)
	ImU32			PeakCount;		// Max bin value (for normalization)
	ImParadeMode	Mode;
	ImPixelBitDepth BitDepth;

	ImParadeScopeData() : XBins( 0 ), YBins( 0 ), ChannelCount( 0 ), PeakCount( 0 ), Mode( ImParadeMode_RGB ), BitDepth( ImPixelBitDepth_UInt8 ) {}

	void Clear()
	{
		Bins.clear();
		XBins = YBins = ChannelCount = 0;
		PeakCount = 0;
	}

	void Accumulate( void const* data, int width, int height, int channels,
					 ImPixelBitDepth bitDepth, ImPixelLayout layout, ImParadeMode mode,
					 int xBins = 128, int yBins = 128, int maxSamples = 1000000 );
};

// ---- Vector Scope ----

struct ImVectorScopeData
{
	ImVector<ImU32>	Bins;			// [xBin * Resolution + yBin] -- 2D chrominance histogram
	int				Resolution;		// Square grid resolution (N x N)
	ImU32			PeakCount;		// Max bin value (for normalization)
	ImPixelBitDepth BitDepth;

	ImVectorScopeData() : Resolution( 0 ), PeakCount( 0 ), BitDepth( ImPixelBitDepth_UInt8 ) {}

	void Clear()
	{
		Bins.clear();
		Resolution = 0;
		PeakCount = 0;
	}

	// Accumulate chrominance (Cb/Cr BT.709) from raw image data into 2D histogram.
	void Accumulate( void const* data, int width, int height, int channels,
					 ImPixelBitDepth bitDepth, ImPixelLayout layout,
					 int resolution = 256, int maxSamples = 1000000 );
};

// ---- Color Warper Overlay ----

struct ImColorWarperOverlay
{
	ImVector<float>	SampledRGB;		// [i*3 + 0..2] = r, g, b (normalized [0,1])
	int				SampleCount;

	ImColorWarperOverlay() : SampleCount( 0 ) {}

	void Clear() { SampledRGB.clear(); SampleCount = 0; }

	void Accumulate( void const* data, int width, int height, int channels,
					 ImPixelBitDepth bitDepth, ImPixelLayout layout,
					 int maxSamples = 50000 );
};

// ---- Histogram ----

typedef int ImHistogramMode;
enum ImHistogramMode_
{
	ImHistogramMode_Luma = 0,		// Single luminance channel
	ImHistogramMode_RGB,			// R, G, B
	ImHistogramMode_YRGB,			// Y, R, G, B
	ImHistogramMode_YCbCr,			// Y, Cb, Cr
	ImHistogramMode_HSV,			// H, S, V
	ImHistogramMode_OkLCH,			// Lightness, Chroma, Hue
	ImHistogramMode_COUNT
};

typedef int ImHistogramLayout;
enum ImHistogramLayout_
{
	ImHistogramLayout_Overlapped = 0,	// All channels in same area
	ImHistogramLayout_Stacked,			// Channels stacked vertically
	ImHistogramLayout_COUNT
};

struct ImHistogramData
{
	ImVector<ImU32>	Bins;			// [ch * BinCount + bin]
	int				BinCount;		// Number of bins per channel
	int				ChannelCount;	// Display channels (1..4 depending on mode)
	ImU32			PeakCount;		// Max bin value (for normalization)
	ImHistogramMode	Mode;
	ImPixelBitDepth BitDepth;

	ImHistogramData() : BinCount( 0 ), ChannelCount( 0 ), PeakCount( 0 ), Mode( ImHistogramMode_RGB ), BitDepth( ImPixelBitDepth_UInt8 ) {}

	void Clear()
	{
		Bins.clear();
		BinCount = ChannelCount = 0;
		PeakCount = 0;
	}

	void Accumulate( void const* data, int width, int height, int channels,
					 ImPixelBitDepth bitDepth, ImPixelLayout layout, ImHistogramMode mode,
					 int binCount = 256, int maxSamples = 1000000 );
};

// ---- CIE Chromaticity ----

typedef int ImCIEChromaticitySignalColor;
enum ImCIEChromaticitySignalColor_
{
	ImCIEChromaticitySignalColor_Flat = 0,		// Use style color for all points
	ImCIEChromaticitySignalColor_PixelColor,	// Use each pixel's own RGB color
	ImCIEChromaticitySignalColor_COUNT
};

typedef int ImCIEChromaticityGamut;
enum ImCIEChromaticityGamut_
{
	ImCIEChromaticityGamut_sRGB_Rec709 = 0,	// sRGB / Rec.709 (D65)
	ImCIEChromaticityGamut_Rec2020,				// Rec.2020 / UHDTV (D65)
	ImCIEChromaticityGamut_DCI_P3,				// DCI-P3 (D65 variant)
	ImCIEChromaticityGamut_ACEScg,				// ACEScg (D60)
	ImCIEChromaticityGamut_AdobeRGB,			// Adobe RGB (D65)
	ImCIEChromaticityGamut_ProPhoto,			// ProPhoto / ROMM (D50)
	ImCIEChromaticityGamut_COUNT
};

struct ImCIEChromaticityData
{
	ImVector<float>	SampledRGB;		// [i*3 + 0..2] = r, g, b (normalized [0,1])
	int				SampleCount;

	ImCIEChromaticityData() : SampleCount( 0 ) {}

	void Clear()
	{
		SampledRGB.clear();
		SampleCount = 0;
	}

	// Sample RGB pixels from raw image data for chromaticity plotting.
	void Accumulate( void const* data, int width, int height, int channels,
					 ImPixelBitDepth bitDepth, ImPixelLayout layout,
					 int maxSamples = 50000 );
};

// ---- Tone Curve ----

struct ImToneCurveData
{
	ImColorCurveData	Channels[ 4 ];	// Per-channel curves (up to 4)
	int					ActiveChannel;	// Which channel is being edited (0..N-1)

	ImToneCurveData() : ActiveChannel( 0 )
	{
		for ( int i = 0; i < 4; ++i )
		{
			Channels[ i ].AddKey( 0.0f, 0.0f );
			Channels[ i ].AddKey( 1.0f, 1.0f );
		}
	}

	void Reset( int channelCount )
	{
		IM_UNUSED( channelCount );
		for ( int i = 0; i < 4; ++i )
		{
			Channels[ i ].Keys.clear();
			Channels[ i ].SelectedIdx = -1;
			Channels[ i ].AddKey( 0.0f, 0.0f );
			Channels[ i ].AddKey( 1.0f, 1.0f );
		}
		ActiveChannel = 0;
	}
};

struct ImCircle
{
	ImVec2 center;
	float radius;
};
struct ImCapsule
{
	ImVec2 pos;
	float length;
	float thickness;
};
struct ImPolyShapeData
{
	ImVec2* pts;
	int pts_count;
};
struct ImPolyHoleShapeData
{
	ImVec2* pts;
	ImRect* p_bb;
	int pts_count;
	float gap;
	float strokeWidth;
};

// Unit Field
typedef float (*ImUnitConvertCallback)(float value, void* pUserData);
struct ImUnitDef
{
	char const* name;            // Full name: "meter", "foot", "inch"
	char const* abbreviation;    // Short: "m", "ft", "in"
	float       mul;             // base_value * mul + add = display_value
	float       add;             // (for Fahrenheit: mul=9/5, add=32)
	ImUnitConvertCallback toDisplay;  // Custom: base -> display (if non-NULL, mul/add ignored)
	ImUnitConvertCallback toBase;     // Custom: display -> base
	void*       pUserData;       // Passed to callbacks
};

inline ImUnitDef ImUnitDef_Simple( char const* name, char const* abbr, float mul, float add = 0.0f )
{
	ImUnitDef u = {};
	u.name = name;
	u.abbreviation = abbr;
	u.mul = mul;
	u.add = add;
	u.toDisplay = NULL;
	u.toBase = NULL;
	u.pUserData = NULL;
	return u;
}

inline ImUnitDef ImUnitDef_Custom( char const* name, char const* abbr, ImUnitConvertCallback toDisplay, ImUnitConvertCallback toBase, void* pUserData = NULL )
{
	ImUnitDef u = {};
	u.name = name;
	u.abbreviation = abbr;
	u.mul = 1.0f;
	u.add = 0.0f;
	u.toDisplay = toDisplay;
	u.toBase = toBase;
	u.pUserData = pUserData;
	return u;
}

// Paint Canvas
typedef int ImPaintMode;
enum ImPaintMode_
{
	ImPaintMode_BinaryMask = 0, // Binary mask: on/off
	ImPaintMode_Grayscale,      // Black & white intensity
	ImPaintMode_Color,          // Full RGBA
	ImPaintMode_COUNT
};

typedef int ImPaintBrush;
enum ImPaintBrush_
{
	ImPaintBrush_Hard = 0,      // Solid circle
	ImPaintBrush_Soft,          // Feathered circle
	ImPaintBrush_COUNT
};

typedef int ImPaintTool;
enum ImPaintTool_
{
	ImPaintTool_Brush = 0,
	ImPaintTool_Eraser,
	ImPaintTool_COUNT
};

struct ImPaintCanvasData
{
	// User-owned pixel buffer
	void*                  Pixels;         // Pointer to user pixel data (user allocates/frees)
	int                    Width;          // Canvas width in pixels
	int                    Height;         // Canvas height in pixels
	ImPlatform_PixelFormat Format;         // Pixel format (ImPlatform_PixelFormat_RGBA8, _R8, _RGBA32F, etc.)
	ImPaintMode            Mode;           // Brush behavior: Mask, Grayscale, Color

	// Brush settings
	ImPaintBrush Brush;
	ImPaintTool  Tool;
	float        BrushSize;
	float        BrushHardness;
	float        BrushOpacity;
	ImVec4       BrushColor;

	// Internal (widget-owned)
	ImTextureID  _TexID;                   // GPU texture (managed by widget)
	int          _TexW, _TexH;             // Current texture dimensions
	ImPlatform_PixelFormat _TexFmt;        // Current texture format
	ImVec2       _LastPos;
	bool         _TexDirty;

	ImPaintCanvasData()  { Pixels = NULL; Width = Height = 0; Format = ImPlatform_PixelFormat_RGBA8; Mode = ImPaintMode_Color; Brush = ImPaintBrush_Hard; Tool = ImPaintTool_Brush; BrushSize = 4.0f; BrushHardness = 0.5f; BrushOpacity = 1.0f; BrushColor = ImVec4( 1, 1, 1, 1 ); _TexID = ImTextureID_Invalid; _TexW = _TexH = 0; _TexFmt = ImPlatform_PixelFormat_RGBA8; _LastPos = ImVec2( -1, -1 ); _TexDirty = true; }
	void DestroyTexture() { if ( _TexID != ImTextureID_Invalid ) { ImPlatform_DestroyTexture( _TexID ); _TexID = ImTextureID_Invalid; } _TexW = _TexH = 0; }
};

// Image Viewer: persistent pan/zoom state + optional CPU pixel buffer for inspector readback
struct ImImageViewerState
{
	float  Zoom;         // Display zoom factor: 1.0 = fit image to widget
	ImVec2 Pan;          // Pan offset in image-space pixels from image centre

	// Optional: provide a CPU copy of the texture pixels to show RGBA values in the inspector.
	// If NULL, the inspector only shows (x, y) coordinates.
	void const*            Pixels;      // CPU pixel buffer
	ImVec2                 PixelSize;   // Dimensions of the Pixels buffer (should match texture)
	ImPlatform_PixelFormat PixelFormat;

	// --- Last-frame transform cache (written by ImageViewer, read by overlay widgets) ---
	// Overlays (DetectionOverlay, KeypointOverlay, TextLabelOverlay, AnnotationEditor)
	// must be called AFTER ImageViewer in the same frame with the same state.
	ImRect _LastCanvasRect;  // widget bounding box in screen coords
	ImVec2 _LastImageSize;   // image extents used at last call
	float  _LastFitScale;    // fit-to-widget scale (imageSize -> widget rect); totalScale = _LastFitScale * Zoom
	bool   _LastValid;       // true if ImageViewer has run at least once this frame

	ImImageViewerState()
		: Zoom( 1.0f ), Pan( 0.0f, 0.0f ), Pixels( NULL ), PixelSize( 0.0f, 0.0f ), PixelFormat( ImPlatform_PixelFormat_RGBA8 ),
		  _LastCanvasRect(), _LastImageSize( 0.0f, 0.0f ), _LastFitScale( 1.0f ), _LastValid( false ) {}

	// UV [0,1]^2 (image-relative) -> screen pixels inside the canvas.
	ImVec2 UVToCanvas( ImVec2 uv ) const
	{
		if ( !_LastValid ) return ImVec2( 0.0f, 0.0f );
		ImVec2 rectCenter = ( _LastCanvasRect.Min + _LastCanvasRect.Max ) * 0.5f;
		ImVec2 imgCenter  = ImVec2( _LastImageSize.x * 0.5f + Pan.x, _LastImageSize.y * 0.5f + Pan.y );
		float  ts         = _LastFitScale * Zoom;
		return ImVec2( rectCenter.x + ( uv.x * _LastImageSize.x - imgCenter.x ) * ts,
					   rectCenter.y + ( uv.y * _LastImageSize.y - imgCenter.y ) * ts );
	}

	// Screen pixels -> UV [0,1]^2 (image-relative).
	ImVec2 CanvasToUV( ImVec2 screen ) const
	{
		if ( !_LastValid || _LastImageSize.x <= 0.0f || _LastImageSize.y <= 0.0f )
			return ImVec2( 0.0f, 0.0f );
		ImVec2 rectCenter = ( _LastCanvasRect.Min + _LastCanvasRect.Max ) * 0.5f;
		ImVec2 imgCenter  = ImVec2( _LastImageSize.x * 0.5f + Pan.x, _LastImageSize.y * 0.5f + Pan.y );
		float  ts         = _LastFitScale * Zoom;
		if ( ts <= 0.0f ) return ImVec2( 0.0f, 0.0f );
		return ImVec2( ( imgCenter.x + ( screen.x - rectCenter.x ) / ts ) / _LastImageSize.x,
					   ( imgCenter.y + ( screen.y - rectCenter.y ) / ts ) / _LastImageSize.y );
	}

	// Widget canvas bounding rect in screen coords (from the last ImageViewer call).
	ImRect GetCanvasRect() const { return _LastCanvasRect; }

	// Total scale factor from image pixels -> screen pixels (fit * zoom).
	float  GetZoomScale()  const { return _LastFitScale * Zoom; }
};

// Invoked by ImageViewer right after it draws the image -- once for the normal
// (compact) widget, and again inside its built-in "expand to window" modal when
// that's open, each time with `state` reflecting that instance's own transform.
// Pass overlay-drawing calls (DetectionOverlay, KeypointOverlay, TextLabelOverlay,
// AnnotationEditor, ...) through this callback instead of calling them manually
// after ImageViewer so they also render inside the modal, not just the compact view.
typedef void ( *ImImageViewerOverlayCallback )( ImImageViewerState& state, void* user_data );

// ============================================================================
// ImageInspector: color-managed raw-buffer viewer
// ============================================================================
// A more advanced sibling of ImageViewer that displays any of 11 sample types x
// 1..4 channels via an HLSL uber-shader. The user supplies an ImImageBuffer
// (Halide-style strided descriptor, defined in ImPlatform.h) and the widget
// uploads its bytes ONCE into a packed RGBA32F GPU texture (re-uploading only
// when ImImageBuffer.version changes). All view transforms (gamma, sRGB, log
// curves, gamut, exposure, white point, tonemap, false color, NaN highlight,
// mosaic decode) run in the shader; per-frame CPU cost is uniform updates only.
//
// The CPU-side inspector loupe walks the user's host pointer directly with
// full precision (double[4] + int64[4] lanes for exact integer display) -- the
// user must keep the buffer alive while the widget is open.
//
// Not supported on backends without custom shader support (DX9). On those
// backends the widget renders a "not supported" message and returns false.

// Bayer / X-Trans color filter array patterns for raw photography.
enum ImMosaicPattern
{
	ImMosaicPattern_None = 0,
	ImMosaicPattern_BayerRGGB,
	ImMosaicPattern_BayerGRBG,
	ImMosaicPattern_BayerGBRG,
	ImMosaicPattern_BayerBGGR,
	ImMosaicPattern_XTrans6x6,
	ImMosaicPattern_COUNT
};

// Whether mosaic data is shown raw (single channel passthrough) or demosaiced.
enum ImMosaicMode
{
	ImMosaicMode_RawPassthrough = 0,    // Show as gray-with-tint (hot-pixel inspection)
	ImMosaicMode_BilinearDemosaic,      // Cheap in-shader demosaic
	ImMosaicMode_COUNT
};

// Input transfer (inverse OETF / EOTF): how raw values map to scene-linear.
enum ImImageInspector_Transfer
{
	ImImageInspector_Transfer_Linear = 0,   // Pass-through
	ImImageInspector_Transfer_Gamma,        // Power gamma (single float exponent)
	ImImageInspector_Transfer_sRGB,         // Real piecewise sRGB curve
	ImImageInspector_Transfer_Rec709,       // BT.709 OETF
	ImImageInspector_Transfer_Rec1886,      // BT.1886 (display-referred)
	ImImageInspector_Transfer_Cineon,       // Cineon log
	ImImageInspector_Transfer_SLog2,        // Sony S-Log2
	ImImageInspector_Transfer_SLog3,        // Sony S-Log3
	ImImageInspector_Transfer_LogC3,        // ARRI LogC3 (EI 800)
	ImImageInspector_Transfer_LogC4,        // ARRI LogC4
	ImImageInspector_Transfer_CanonLog,     // Canon Log
	ImImageInspector_Transfer_CanonLog2,    // Canon Log 2
	ImImageInspector_Transfer_CanonLog3,    // Canon Log 3
	ImImageInspector_Transfer_VLog,         // Panasonic V-Log
	ImImageInspector_Transfer_RedLog3G10,   // RED Log3G10
	ImImageInspector_Transfer_BMFilmGen5,   // Blackmagic Film Gen 5
	ImImageInspector_Transfer_AppleLog,     // Apple Log
	ImImageInspector_Transfer_FLog,         // Fujifilm F-Log
	ImImageInspector_Transfer_DLog,         // DJI D-Log
	ImImageInspector_Transfer_PQ,           // SMPTE ST.2084 (HDR10 EOTF)
	ImImageInspector_Transfer_HLG,          // BT.2100 Hybrid Log-Gamma
	ImImageInspector_Transfer_COUNT
};

// Output transfer (forward OETF): how working space maps to display.
enum ImImageInspector_OutputTransfer
{
	ImImageInspector_OutputTransfer_Linear = 0,
	ImImageInspector_OutputTransfer_Gamma,
	ImImageInspector_OutputTransfer_sRGB,
	ImImageInspector_OutputTransfer_PQ,
	ImImageInspector_OutputTransfer_HLG,
	ImImageInspector_OutputTransfer_COUNT
};

// Color primaries (gamut). Used for both input gamut and output gamut.
// Picking the same value for both is a no-op (3x3 identity through working space).
enum ImImageInspector_Gamut
{
	ImImageInspector_Gamut_Rec709 = 0,    // sRGB / BT.709 primaries
	ImImageInspector_Gamut_Rec2020,       // BT.2020 / BT.2100
	ImImageInspector_Gamut_DCIP3,         // DCI-P3 (D65)
	ImImageInspector_Gamut_DisplayP3,     // Apple Display P3
	ImImageInspector_Gamut_AdobeRGB,
	ImImageInspector_Gamut_ProPhoto,      // ROMM RGB
	ImImageInspector_Gamut_ACES_AP0,      // ACES AP0 (wide gamut)
	ImImageInspector_Gamut_ACES_AP1,      // ACES AP1 / ACEScg
	ImImageInspector_Gamut_COUNT
};

// HDR -> SDR tonemap operators.
enum ImImageInspector_Tonemap
{
	ImImageInspector_Tonemap_None = 0,    // Clip
	ImImageInspector_Tonemap_Reinhard,
	ImImageInspector_Tonemap_ReinhardExt, // Extended Reinhard with white point
	ImImageInspector_Tonemap_ACES,        // ACES Filmic (Narkowicz approximation)
	ImImageInspector_Tonemap_AGX,         // Troy Sobotka's AGX
	ImImageInspector_Tonemap_PBRNeutral,  // Khronos PBR Neutral
	ImImageInspector_Tonemap_Hable,       // Uncharted 2 / Hable
	ImImageInspector_Tonemap_COUNT
};

// Spatial reconstruction filter. All filters run manually in shader (no HW sampler).
enum ImImageInspector_Filter
{
	ImImageInspector_Filter_Nearest = 0,
	ImImageInspector_Filter_Bilinear,
	ImImageInspector_Filter_BicubicMitchell,   // Mitchell-Netravali (B=1/3, C=1/3)
	ImImageInspector_Filter_BicubicCatmullRom, // Catmull-Rom (B=0, C=1/2)
	ImImageInspector_Filter_Lanczos2,
	ImImageInspector_Filter_Lanczos3,
	ImImageInspector_Filter_COUNT
};

// False color palette (or off).
enum ImImageInspector_FalseColor
{
	ImImageInspector_FalseColor_Off = 0,
	ImImageInspector_FalseColor_Viridis,
	ImImageInspector_FalseColor_Magma,
	ImImageInspector_FalseColor_Inferno,
	ImImageInspector_FalseColor_Plasma,
	ImImageInspector_FalseColor_Cividis,
	ImImageInspector_FalseColor_Turbo,
	ImImageInspector_FalseColor_Cinematographer, // Exposure-scope colors
	ImImageInspector_FalseColor_OutOfGamut,      // Highlight pixels with channels < 0 or > 1
	ImImageInspector_FalseColor_COUNT
};

// Persistent state for the ImageInspector widget.
// User-controllable fields are at the top; the GPU cache fields below should
// not be touched directly -- they're managed by the widget. Call
// ImWidgets::ImageInspectorReleaseState() before destroying the state to free
// the GPU texture.
struct ImImageInspectorState
{
	// ---- View ----
	float  Zoom;                  // 1.0 = fit image to widget
	ImVec2 Pan;                   // Pan offset in image-space pixels

	// ---- Color pipeline ----
	int    InputTransfer;         // ImImageInspector_Transfer
	int    InputGamut;            // ImImageInspector_Gamut
	int    WorkingGamut;          // ImImageInspector_Gamut
	int    OutputGamut;           // ImImageInspector_Gamut
	int    OutputTransfer;        // ImImageInspector_OutputTransfer
	int    Tonemap;               // ImImageInspector_Tonemap
	int    Filter;                // ImImageInspector_Filter
	int    FalseColor;            // ImImageInspector_FalseColor
	float  Exposure;              // Stops (multiply by 2^Exposure)
	float  Black, White;          // Black/white point in working space
	float  Temperature, Tint;     // Color temperature adjustment (Kelvin offset, magenta/green)
	float  Gamma;                 // Used when InputTransfer == _Gamma (e.g. 2.2)
	float  ChannelMask[4];        // R/G/B/A multipliers (0..1)
	ImVec4 NaNColor;              // Highlight color for NaN/Inf pixels (default magenta)

	// ---- Mosaic (raw photography) ----
	int    MosaicPattern;         // ImMosaicPattern (overrides any inferred pattern)
	int    MosaicMode;            // ImMosaicMode (raw vs bilinear demosaic)

	// ---- GPU cache (managed by widget -- do not touch) ----
	ImTextureID PackedTexture;
	int         PackedTexW, PackedTexH;
	ImU64       LastUploadedVersion;
	int         LastUploadedWidth, LastUploadedHeight;
	int         LastUploadedChannels;
	int         LastUploadedSampleType;
	int         GpuXStride, GpuYStride, GpuCStride;   // On-GPU strides (always tight)
	ImVector<unsigned char> Scratch;                  // Persistent scratch for tight-pack
	bool        BufferTooLarge;                       // (placed last so the bool's trailing padding doesn't sit inside the struct)

	ImImageInspectorState()
		: Zoom( 1.0f ), Pan( 0.0f, 0.0f )
		, InputTransfer( ImImageInspector_Transfer_sRGB )
		, InputGamut( ImImageInspector_Gamut_Rec709 )
		, WorkingGamut( ImImageInspector_Gamut_Rec709 )
		, OutputGamut( ImImageInspector_Gamut_Rec709 )
		, OutputTransfer( ImImageInspector_OutputTransfer_sRGB )
		, Tonemap( ImImageInspector_Tonemap_None )
		, Filter( ImImageInspector_Filter_Bilinear )
		, FalseColor( ImImageInspector_FalseColor_Off )
		, Exposure( 0.0f )
		, Black( 0.0f ), White( 1.0f )
		, Temperature( 0.0f ), Tint( 0.0f )
		, Gamma( 2.2f )
		, NaNColor( 1.0f, 0.0f, 1.0f, 1.0f )
		, MosaicPattern( ImMosaicPattern_None )
		, MosaicMode( ImMosaicMode_BilinearDemosaic )
		, PackedTexture( ImTextureID_Invalid )
		, PackedTexW( 0 ), PackedTexH( 0 )
		, LastUploadedVersion( 0 )
		, LastUploadedWidth( 0 ), LastUploadedHeight( 0 )
		, LastUploadedChannels( 0 ), LastUploadedSampleType( 0 )
		, GpuXStride( 0 ), GpuYStride( 0 ), GpuCStride( 0 )
		, BufferTooLarge( false )
	{
		ChannelMask[ 0 ] = ChannelMask[ 1 ] = ChannelMask[ 2 ] = ChannelMask[ 3 ] = 1.0f;
	}
};

//////////////////////////////////////////////////////////////////////////
// Extended primitives, interactions & widgets (2026-04 refacto batch)
//////////////////////////////////////////////////////////////////////////

// Hatching / stippling patterns
enum ImWidgetsHatchPattern_
{
	ImWidgetsHatchPattern_Parallel = 0,
	ImWidgetsHatchPattern_Cross,
	ImWidgetsHatchPattern_Diagonal,
	ImWidgetsHatchPattern_DiagonalCross,
	ImWidgetsHatchPattern_Dots,
	ImWidgetsHatchPattern_ConcentricRings,
	ImWidgetsHatchPattern_BenDay,        // Comic-book grid of uniform dots, offset on alternate rows.
	ImWidgetsHatchPattern_Screentone,    // Halftone -- dot radius modulated across the pattern.
	ImWidgetsHatchPattern_COUNT
};
typedef int ImWidgetsHatchPattern;

// Cross-cutting slider flags (angle wrap, delta overlay, proportional drag, stepping)
enum ImWidgetsSliderFlags_
{
	ImWidgetsSliderFlags_None             = 0,
	ImWidgetsSliderFlags_AngleWrap        = 1 << 0, // angle wraps around 2pi
	ImWidgetsSliderFlags_AngleClamp       = 1 << 1, // clamp to [amin, amax] (default if neither set)
	ImWidgetsSliderFlags_ShowDelta        = 1 << 2, // show floating drag-delta overlay
	ImWidgetsSliderFlags_ProportionalDrag = 1 << 3, // participate in PushDragGroup
	ImWidgetsSliderFlags_Stepped          = 1 << 4  // snap to nearest stop
};
typedef int ImWidgetsSliderFlags;

// Falloff kernel for proportional multi-drag
enum ImWidgetsFalloff_
{
	ImWidgetsFalloff_Linear = 0,
	ImWidgetsFalloff_Gaussian,
	ImWidgetsFalloff_Smoothstep,
	ImWidgetsFalloff_COUNT
};
typedef int ImWidgetsFalloff;

// Scalar 2D callback (distinct from color ones; returns a float)
typedef float ( *ImWidgetsScalar2DCallback )( float x, float y, void* user );

// Tier descriptor for multi-level iso-contour rendering (topographic maps, etc).
struct ImIsoContourTier
{
	float spacing;      // world-value interval (or log-step multiplier when log_spacing=true)
	ImU32 color;        // line color
	float thickness;    // stroke thickness in pixels
	bool  log_spacing;  // when true, iso-values are at k*spacing in log domain (10^k*spacing style)

	ImIsoContourTier() : spacing( 1.0f ), color( IM_COL32( 255, 255, 255, 255 ) ), thickness( 1.0f ), log_spacing( false ) {}
	ImIsoContourTier( float s, ImU32 c, float t, bool lg = false ) : spacing( s ), color( c ), thickness( t ), log_spacing( lg ) {}
};

//////////////////////////////////////////////////////////////////////////
// Color grading (Phase D)
//////////////////////////////////////////////////////////////////////////

struct ImColorLUT3D
{
	int              Size;          // edge length (17, 33, 65 typical)
	ImVector<ImVec4> Entries;       // Size^3 RGBA
	float            DomainMin[ 3 ];
	float            DomainMax[ 3 ];

	ImColorLUT3D() : Size( 0 )
	{
		DomainMin[ 0 ] = DomainMin[ 1 ] = DomainMin[ 2 ] = 0.0f;
		DomainMax[ 0 ] = DomainMax[ 1 ] = DomainMax[ 2 ] = 1.0f;
	}
};

//////////////////////////////////////////////////////////////////////////
// LookDev (Phase E)
//////////////////////////////////////////////////////////////////////////

struct ImLookDevState
{
	// Pivot position, normalized to the canvas (0,0 = top-left corner, 1,1 = bottom-right).
	// The divider line always passes through this point, so the yellow slide handle
	// is anchored here -- and rotation naturally pivots around it.
	ImVec2 PivotNormalized;
	float  DividerAngleRad;  // 0 = vertical divider, CW positive
	bool   Swap;

	ImLookDevState() : PivotNormalized( 0.5f, 0.5f ), DividerAngleRad( 0.0f ), Swap( false ) {}
};

// Per-side tonemap controls for LookDevInspector.
struct ImLookDevInspectorSide
{
	float ExposureStops;
	float Black;
	float White;
	float Gamma;

	ImLookDevInspectorSide() : ExposureStops( 0.0f ), Black( 0.0f ), White( 1.0f ), Gamma( 2.2f ) {}
};

enum ImLookDevInspectorControls_
{
	ImLookDevInspectorControls_Independent = 0,
	ImLookDevInspectorControls_Unified     = 1,
};
typedef int ImLookDevInspectorControls;

struct ImLookDevInspectorState
{
	ImLookDevState               Divider;     // shared pivot + angle + swap
	ImLookDevInspectorSide       A;
	ImLookDevInspectorSide       B;
	ImLookDevInspectorControls   Controls;    // Independent vs Unified

	ImLookDevInspectorState() : Controls( ImLookDevInspectorControls_Independent ) {}
};

// Bezier surface patches
struct ImCoonsPatch
{
	// Four cubic Bezier boundaries, each 4 control points.
	// Order: bottom (u: 0->1), top (u: 0->1), left (v: 0->1), right (v: 0->1).
	// Corners must match: bottom[0]==left[0], bottom[3]==right[0], top[0]==left[3], top[3]==right[3].
	ImVec2 bottom[ 4 ];
	ImVec2 top[ 4 ];
	ImVec2 left[ 4 ];
	ImVec2 right[ 4 ];
};

struct ImGregoryPatch
{
	// 20 control points Gregory patch. Corners[4] + two tangent CPs per edge (8) + two twist CPs per corner (8) = 20.
	// Layout: [c00,c10,c11,c01, e_bot0,e_bot1, e_right0,e_right1, e_top0,e_top1, e_left0,e_left1,
	//          t00a,t00b, t10a,t10b, t11a,t11b, t01a,t01b]
	ImVec2 cp[ 20 ];
};

// Vector Drawing Tool types moved to dear_widgets_vector_drawing.h so the
// widget implementation can live in its own translation unit. Downstream code
// that includes dear_widgets.h keeps working with no changes.
#include "dear_widgets_vector_drawing.h"

// Network Graph widget (read-only hierarchically grouped DAG illustration) --
// own translation unit, same pattern as the Vector Drawing Tool.
#include "dear_widgets_network.h"
#include "dear_widgets_timeline.h"

// Font inspector mode
enum ImFontInspectorMode_
{
	ImFontInspectorMode_Grid = 0,
	ImFontInspectorMode_Metrics,
	ImFontInspectorMode_Curves,
	ImFontInspectorMode_Kerning,
	ImFontInspectorMode_COUNT
};
typedef int ImFontInspectorMode;

// Delta-E formulas
enum ImDeltaEFormula_
{
	ImDeltaEFormula_E76 = 0,
	ImDeltaEFormula_E94,
	ImDeltaEFormula_E2000,
	ImDeltaEFormula_EOK,
	ImDeltaEFormula_COUNT
};
typedef int ImDeltaEFormula;

// Equation input flags
enum ImWidgetsEquationFlags_
{
	ImWidgetsEquationFlags_None             = 0,
	ImWidgetsEquationFlags_ReadOnly         = 1 << 0,
	ImWidgetsEquationFlags_ShowParseErrors  = 1 << 1,
	ImWidgetsEquationFlags_InlineOnly       = 1 << 2,
	ImWidgetsEquationFlags_BlockOnly        = 1 << 3,
	// Inline vertical alignment (mutually exclusive; default is baseline/center).
	ImWidgetsEquationFlags_AlignInlineTop    = 1 << 4,
	ImWidgetsEquationFlags_AlignInlineCenter = 1 << 5,
	ImWidgetsEquationFlags_AlignInlineBottom = 1 << 6,
	// Align surrounding plain text instead of the math block (applied to the line).
	ImWidgetsEquationFlags_AlignInlineBaseline = 1 << 7 // fallback if no flag given
};
typedef int ImWidgetsEquationFlags;

enum ImWidgetsBgEffect_
{
	ImWidgetsBgEffect_Blur = 0,
	ImWidgetsBgEffect_GlassRefraction,
	ImWidgetsBgEffect_FrostedGlass,
	ImWidgetsBgEffect_Pixelate,
	ImWidgetsBgEffect_ChromaticAberration,
	ImWidgetsBgEffect_LiquidGlass,
	ImWidgetsBgEffect_HeatHaze,
	ImWidgetsBgEffect_Voronoi,
	ImWidgetsBgEffect_EdgeGlow,
	ImWidgetsBgEffect_Halftone,
	ImWidgetsBgEffect_MouseEdge,
	ImWidgetsBgEffect_CRT,
	ImWidgetsBgEffect_DotMatrix,
	ImWidgetsBgEffect_Glitch,
	ImWidgetsBgEffect_StainedGlass,
	ImWidgetsBgEffect_Rain,
	ImWidgetsBgEffect_Kaleidoscope,
	ImWidgetsBgEffect_COUNT,
};
typedef int ImWidgetsBgEffect;

namespace ImWidgets{
	ImWidgetsStyle& GetStyle();
	IMGUI_API void  ShowStyleEditor( ImWidgetsStyle* ref = NULL );
	IMGUI_API void  SaveStyleToFile( char const* filename, ImWidgetsStyle const* style = NULL );
	IMGUI_API bool  LoadStyleFromFile( char const* filename, ImWidgetsStyle* style = NULL );

	inline
	char const* GetStyleColorName( ImWidgetsStyleColor colorIndex )
	{
		return GetStyle().GetColorName( colorIndex );
	}
	inline
	void PushStyleColor( ImWidgetsStyleColor colorIndex, ImVec4 const& color )
	{
		GetStyle().PushColor( colorIndex, color );
	}
	inline
	void PopStyleColor( int count = 1 )
	{
		GetStyle().PopColor( count );
	}
	inline
	void PushStyleVar( ImWidgetsStyleVar varIndex, float value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PushStyleVar( ImWidgetsStyleVar varIndex, ImVec2 const& value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PushStyleVar( ImWidgetsStyleVar varIndex, ImVec4 const& value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PopStyleVar( int count = 1 )
	{
		GetStyle().PopVar( count );
	}

	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////
	inline
	float ImRound(float x)
	{
		return roundf(x);
	}
	inline
	float ImSmoothStep(float edge0, float edge1, float x)
	{
		// Scale, bias and saturate x to 0..1 range
		x = ImClamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		// Evaluate polynomial
		return x * x * (3.0f - 2.0f * x);
	}
	inline
	float ImLength(ImVec2 v)
	{
		return ImSqrt( ImLengthSqr( v ) );
	}

	// Right-to-Left, like operator '=' or like standard function in C, memcpy, ...
	IMGUI_API void	ColorConvertsRGBtosRGB( float& out_r, float& out_g, float& out_b, float r, float g, float b );
	IMGUI_API void	ColorConvertRGBtoLinear( float& out_L, float& out_a, float& out_b, float r, float g, float b );
	IMGUI_API void	ColorConvertLineartoRGB( float& out_r, float& out_g, float& out_b, float L, float a, float b );
	IMGUI_API void	ColorConvertRGBtoOKLAB( float& out_L, float& out_a, float& out_b, float r, float g, float b );
	IMGUI_API void	ColorConvertOKLABtoRGB( float& out_r, float& out_g, float& out_b, float L, float a, float b );
	IMGUI_API void	ColorConvertOKLCHtoOKLAB( float& out_L, float& out_a, float& out_b, float r, float g, float b );
	IMGUI_API void	ColorConvertOKLABtoOKLCH( float& out_r, float& out_g, float& out_b, float L, float a, float b );
	IMGUI_API void	ColorConvertsRGBtoOKLCH( float& out_L, float& out_c, float& out_h, float r, float g, float b );
	IMGUI_API void	ColorConvertOKLCHtosRGB( float& out_r, float& out_g, float& out_b, float L, float c, float h );
	IMGUI_API void	ColorConvertRGBtoHSV( float& out_h, float& out_s, float& out_v, float r, float g, float b );
	IMGUI_API void	ColorConvertHSVtoRGB( float& out_r, float& out_g, float& out_b, float h, float s, float v );

	// sRGB <-> XYZ (D65)
	IMGUI_API void	ColorConvertsRGBtoXYZ( float& out_X, float& out_Y, float& out_Z, float r, float g, float b );
	IMGUI_API void	ColorConvertXYZtosRGB( float& out_r, float& out_g, float& out_b, float X, float Y, float Z );
	// CIE Lab (L*a*b* D65)
	IMGUI_API void	ColorConvertXYZtoCIELab( float& out_L, float& out_a, float& out_b, float X, float Y, float Z );
	IMGUI_API void	ColorConvertCIELabtoXYZ( float& out_X, float& out_Y, float& out_Z, float L, float a, float b );
	IMGUI_API void	ColorConvertsRGBtoCIELab( float& out_L, float& out_a, float& out_b, float r, float g, float b );
	IMGUI_API void	ColorConvertCIELabtosRGB( float& out_r, float& out_g, float& out_b, float L, float a, float b );
	// CIE xyY <-> XYZ
	IMGUI_API void	ColorConvertXYZtoxyY( float& out_x, float& out_y, float& out_Y, float X, float Y, float Z );
	IMGUI_API void	ColorConvertxyYtoXYZ( float& out_X, float& out_Y, float& out_Z, float x, float y, float Yval );

	ImU32	KelvinTemperatureTosRGBColors( float temperature ); // [ 1000 K; 12000 K ]
	// Float-precision variant: linear RGB in [0,1], no 8-bit quantization (use for
	// chromaticity mapping -- quantized colors make the recovered locus zigzag).
	IMGUI_API void	KelvinTemperatureToLinearRGBColorsF( float temperature, float* out_rgb_linear ); // [ 1000 K; 12000 K ]

	//////////////////////////////////////////////////////////////////////////
	// Color Functions
	//////////////////////////////////////////////////////////////////////////

	//IMGUI_API void ColorConvertHWBtoRGB( float h, float w, float b, float& r, float& g, float& b );

	IMGUI_API ImU32 ImColorBlendsRGB( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendLinear( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendHSL( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendHSLa( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendHWB( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendLCH( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendLab( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendOklab( ImU32 col0, ImU32 col1, float t );
	IMGUI_API ImU32 ImColorBlendOkLCH( ImU32 col0, ImU32 col1, float t );

	//////////////////////////////////////////////////////////////////////////
	// Geometry Generation
	//////////////////////////////////////////////////////////////////////////
#ifdef DEAR_WIDGETS_TESSELATION
	void	ShapeTesselationUniform( ImWidgetsShape& shape );
#endif

	void	ShapeTranslate( ImWidgetsShape& shape, ImVec2 t );

	void	ShapeSetDefaultUV( ImWidgetsShape& shape );
	void	ShapeSetDefaultUVCol( ImWidgetsShape& shape );
	void	ShapeSetDefaultBoundUV( ImWidgetsShape& shape );
	void	ShapeSetDefaultBoundUVWhiteCol( ImWidgetsShape& shape );
	void	ShapeSetDefaultWhiteCol( ImWidgetsShape& shape );
	void	ClearShapeCache();
	void	ShapeSetBound( ImWidgetsShape& shape );
	void	ShapeLineSetBound( ImWidgetsShapeLine& shape );

	void	GenShapeRect( ImWidgetsShape& shape, ImRect const& r );
	void	GenShapeCircle( ImWidgetsShape& shape, ImVec2 center, float radius, int side_count );
	void	GenShapeCircleArc( ImWidgetsShape& shape, ImVec2 center, float radius, float angle_min, float angle_max, int side_count );
	void	GenShapeRegularNGon( ImWidgetsShape& shape, ImVec2 center, float radius, int side_count );
	void	GenShapeSquircle( ImWidgetsShape& shape, ImVec2 center, float radius, int side_count, float n = 4.0f );
	// S-1: Tessellate a concave polygon and cache the result. pts should be in local/normalized coords;
	// origin is added back after tessellation (same pattern as GenShapeRect with r.Min).
	void	GenShapeConcavePoly( ImWidgetsShape& shape, ImVec2 const* pts, int pts_count, ImVec2 origin = ImVec2( 0.0f, 0.0f ) );

	// Tessellated bands and discs (UVs laid down for procedural-colour fills).
	//   VerticalBand   : rectangle Ã— N horizontal stripes        (uv.y = topâ†’bottom, uv.x = 0â†’1 across)
	//   HorizontalBand : rectangle Ã— N vertical stripes          (uv.x = leftâ†’right, uv.y = 0â†’1 across)
	//   RectGrid       : rectangle Ã— Nx Ã— Ny grid                (uv = [0,1]Â²)
	//   DiscRings      : full disc, polar grid sectors Ã— rings   (uv.x = angle/2Ï€, uv.y = r/R)
	//   Annulus        : ring (annulus), one radial division     (same polar UVs, v = (r-rIn)/(rOut-rIn))
	//   AnnulusRings   : ring with multiple sub-rings
	void	GenShapeVerticalBand  ( ImWidgetsShape& shape, ImRect const& r, int divisions );
	void	GenShapeHorizontalBand( ImWidgetsShape& shape, ImRect const& r, int divisions );
	void	GenShapeRectGrid      ( ImWidgetsShape& shape, ImRect const& r, int divisionsX, int divisionsY );
	void	GenShapeDiscRings     ( ImWidgetsShape& shape, ImVec2 center, float radius, int numSectors, int numRings );
	void	GenShapeAnnulus       ( ImWidgetsShape& shape, ImVec2 center, float innerRadius, float outerRadius, int numSectors );
	void	GenShapeAnnulusRings  ( ImWidgetsShape& shape, ImVec2 center, float innerRadius, float outerRadius, int numSectors, int numRings );
	// Partial annulus arc (no caching â€” startAngle/sweepAngle usually animate per frame).
	void	GenShapeAnnulusArc    ( ImWidgetsShape& shape, ImVec2 center, float innerRadius, float outerRadius, float startAngle, float sweepAngle, int divisions );
	// Barycentric-subdivided triangle (A, B, C). Vertex.uv = (w_A, w_B); w_C is implicit = 1 - w_A - w_B.
	// Use ShapeFillProceduralColor2D with a callback that derives w_C and computes the triangle mix.
	void	GenShapeTriangleSubdiv( ImWidgetsShape& shape, ImVec2 A, ImVec2 B, ImVec2 C, int subdivisions );

	// Pour a procedural-colour callback into a shape's vertex colours.
	// 1D variant samples uv.y (sample_v=true) or uv.x (sample_v=false).
	// 2D variant samples both axes.
	void	ShapeFillProceduralColor1D( ImWidgetsShape& shape, ImWidgetsColor1DCallback func, void* pUserData, bool sample_v );
	void	ShapeFillProceduralColor2D( ImWidgetsShape& shape, ImWidgetsColor2DCallback func, void* pUserData );
	void	ShapeFillSolidColor       ( ImWidgetsShape& shape, ImU32 col );

	// One-shot draws: GenShape â†’ fill colour â†’ DrawShape (no caller boilerplate).
	void	DrawShapeProceduralColorVerticalBand  ( ImDrawList* pDrawList, ImRect const& bb, int divisions,
	                                                 ImWidgetsColor1DCallback func, void* pUserData );
	void	DrawShapeProceduralColorHorizontalBand( ImDrawList* pDrawList, ImRect const& bb, int divisions,
	                                                 ImWidgetsColor1DCallback func, void* pUserData );
	void	DrawShapeProceduralColorRectGrid      ( ImDrawList* pDrawList, ImRect const& bb, int divisionsX, int divisionsY,
	                                                 ImWidgetsColor2DCallback func, void* pUserData );
	void	DrawShapeProceduralColorDiscRings     ( ImDrawList* pDrawList, ImVec2 center, float radius,
	                                                 int numSectors, int numRings,
	                                                 ImWidgetsColor2DCallback func, void* pUserData );
	void	DrawShapeProceduralColorAnnulus       ( ImDrawList* pDrawList, ImVec2 center, float innerRadius, float outerRadius,
	                                                 int numSectors, ImWidgetsColor1DCallback func, void* pUserData );
	void	DrawShapeProceduralColorAnnulusRings  ( ImDrawList* pDrawList, ImVec2 center, float innerRadius, float outerRadius,
	                                                 int numSectors, int numRings,
	                                                 ImWidgetsColor2DCallback func, void* pUserData );

	// TODO
	//void	GenShapeFromBezierCubicCurve( ImShape& shape, ImVector<ImVec2>& path, float thickness, int num_segments = 0 );
	//void	GenShapeFromBezierQuadraticCurve( ImShape& sshape, ImVector<ImVec2>& path, float thickness, int num_segments = 0 );

	// TODO Add Color Blend Option (Linear, sRGB, ...) cf. W3C rules
	typedef void	( *pfSpace2sRGB )( float&, float&, float&, float, float, float );
	typedef void	( *pfsRGB2Space )( float&, float&, float&, float, float, float );
	void	ShapeLinearGradientGeneric( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB, pfsRGB2Space sRGB2Space );
	void	ShapeRadialGradientGeneric( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB, pfsRGB2Space sRGB2Space );
	void	ShapeDiamondGradientGeneric( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB, pfsRGB2Space sRGB2Space );

	void	ShapeSRGBLinearGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeSRGBRadialGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeSRGBDiamondGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLabLinearGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLabRadialGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLabDiamondGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLchLinearGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLchRadialGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeOkLchDiamondGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeLinearSRGBLinearGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeLinearSRGBRadialGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
    void	ShapeLinearSRGBDiamondGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
    void	ShapeHSVLinearGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeHSVRadialGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );
	void	ShapeHSVDiamondGradient( ImWidgetsShape& shape, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1 );

	//////////////////////////////////////////////////////////////////////////
	// ImWidgets Context
	//////////////////////////////////////////////////////////////////////////

	// Had to be called after ImPlatform::InitGfx or after ImPlatform::SimpleInitialize

	IMGUI_API void SetFeatures( ImWidgetsFeatures features );
	IMGUI_API void AddFeatures( ImWidgetsFeatures features );
	IMGUI_API void RemoveFeature( ImWidgetsFeatures features );
	IMGUI_API ImWidgetsContext*	CreateContext();
	IMGUI_API void DestroyContext( ImWidgetsContext* );

	IMGUI_API void SetCurrentContext( ImWidgetsContext* );
	IMGUI_API ImWidgetsContext* GetCurrentContext();
	IMGUI_API ImTextureID GetWhiteTexture();

	IMGUI_API void OwnTexture( ImTextureID tex );

	//////////////////////////////////////////////////////////////////////////
	// Slug GPU Text Rendering
	// Renders resolution-independent text using the Slug algorithm (Eric Lengyel, public domain 2026).
	// Uses the current ImGui font by extracting its TTF outline data for GPU Bezier rendering.
	// Produces crisp results at any scale or viewing angle without texture atlases or distance fields.
	//
	// FONT SIZE UNIT -- Logical Pixels (lp):
	//   All font_size parameters are in logical pixels (DPI-independent).
	//   1 lp = 1 physical pixel at 96 DPI / 1.0x scale.
	//   At 2x DPI (192 DPI / Windows 200%), 1 lp = 2 physical pixels.
	//   The library converts lp -> physical pixels internally using ImGui::GetStyle().FontScaleDpi.
	//   Set FontScaleDpi at startup: ImGui::GetStyle().FontScaleDpi = ImPlatform_GetDpiScale();
	//   Return values (CalcTextSize, CalcShapedTextWidth, CalcLaTeXSize) are in PHYSICAL pixels,
	//   consistent with ImGui cursor positions and layout.
	//////////////////////////////////////////////////////////////////////////
	// Guard against Win32's DrawText/DrawTextA macro collision (winuser.h)
#ifdef DrawText
#undef DrawText
#endif
#ifdef DrawTextA
#undef DrawTextA
#endif
	// Use current ImGui font/size at pos (uses GetFontSize() which is already DPI-scaled)
	IMGUI_API void DrawText( ImDrawList* pDrawList, ImVec2 pos, ImU32 col, char const* text, char const* text_end = nullptr );
	// Explicit font and size in logical pixels (pass nullptr/0 to use current ImGui font/size)
	IMGUI_API void DrawText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, ImU32 col, char const* text, char const* text_end = nullptr );
	// Measure text. font_size in lp. Returns (width, height) in physical pixels.
	// out_ascent: if non-null, receives ascent in physical pixels (pass cursor.y + ascent as baseline to DrawText).
	IMGUI_API ImVec2 CalcTextSize( ImFont* font, float font_size, char const* text, char const* text_end = nullptr, float* out_ascent = nullptr );
	// Like CalcTextSize but runs the OpenType shaper (kb_text_shape) for accurate width of shaped text
	// (Arabic contextual forms, ligatures). Slower than CalcTextSize -- use only when alignment precision matters.
	IMGUI_API float CalcShapedTextWidth( ImFont* font, float font_size, char const* text, char const* text_end = nullptr );
	// Horizontal linear gradient: col_left at text start, col_right at text end. font_size in lp.
	IMGUI_API void DrawTextGradient( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, ImU32 col_left, ImU32 col_right, char const* text, char const* text_end = nullptr );
	// ---- Typography: tesselated text for gradient/image fills ----
	// font_size in lp. tess_tol: curve flattening tolerance (lower = more segments, smoother). 0 = auto.
	// Convert text to CPU-tesselated geometry (ImWidgetsShape) for use with gradient/image fill functions.
	IMGUI_API void TesselateText( ImFont* font, float font_size, char const* text, ImWidgetsShape& outShape, char const* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	// Same as TesselateText but returns per-glyph shapes (preserves ligatures/calt from full text shaping).
	IMGUI_API void TesselateTextPerGlyph( ImFont* font, float font_size, char const* text, ImVector<ImWidgetsShape>& outShapes, char const* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	// Pre-tessellate every glyph in the font and populate the cache. Call once at startup (after the
	// first ImGui::NewFrame) to eliminate stalls when TesselateText/TesselateTextPerGlyph first runs.
	IMGUI_API void PrewarmTessellationCache( ImFont* font, float tess_tol = 0.0f );
	// Extract raw contour points (explicitly closed, for DrawShapeWithHole). font_size in lp. Debug/internal use.
	IMGUI_API void ExtractTextContours( ImFont* font, float font_size, char const* text, char const* text_end, ImVec2 offset, ImVector<ImVec2>& outPoly, ImRect& outBB, float tess_tol = 0.0f );
	// Debug: draw the tessellation algorithm steps for a single character. font_size in lp.
	IMGUI_API void DrawTesselateDebug( ImDrawList* dl, ImFont* font, float font_size, char const* text, ImVec2 pos, float tess_tol, float spacing, float rowH );
	// Text filled with an image texture. font_size in lp.
	IMGUI_API void DrawImageText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, ImTextureID tex, char const* text, char const* text_end = nullptr, ImU32 tint = IM_COL32_WHITE, ImVec2 uv_offset = ImVec2(0,0), ImVec2 uv_scale = ImVec2(1,1), float tess_tol = 0.0f, int iterations = 0 );
	// Text filled with gradients (supports all color spaces via function pointers). font_size in lp.
	IMGUI_API void DrawLinearGradientText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB = nullptr, pfsRGB2Space sRGB2Space = nullptr, char const* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	IMGUI_API void DrawRadialGradientText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB = nullptr, pfsRGB2Space sRGB2Space = nullptr, char const* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	IMGUI_API void DrawDiamondGradientText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB = nullptr, pfsRGB2Space sRGB2Space = nullptr, char const* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	// GPU gradient text rendering (requires ImPlatform + SLUG_FILL shader). No CPU tessellation.
	// Gradient is computed per-pixel on GPU. Falls back to CPU tessellation if shader unavailable. font_size in lp.
	// colorSpace: 0=sRGB, 1=Linear, 2=OkLab, 3=OkLch, 4=HSV (matches ImWidgetsGradientInterp_)
	IMGUI_API void DrawLinearGradientTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, char const* text_end = nullptr, bool perChar = false, int colorSpace = 0 );
	IMGUI_API void DrawRadialGradientTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, char const* text_end = nullptr, bool perChar = false, int colorSpace = 0 );
	IMGUI_API void DrawDiamondGradientTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, char const* text_end = nullptr, bool perChar = false, int colorSpace = 0 );
	// GPU image text rendering. Image is sampled per-pixel within glyph coverage. font_size in lp.
	IMGUI_API void DrawImageTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, ImTextureID tex, ImU32 tint = IM_COL32_WHITE, ImVec2 uv_offset = ImVec2(0,0), ImVec2 uv_scale = ImVec2(1,1), char const* text_end = nullptr, bool perChar = false );
	// GPU image text with per-character texture cycling. textures[i % nTextures] is used for glyph i.
	IMGUI_API void DrawImageTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, ImTextureID const* textures, int nTextures, ImU32 tint = IM_COL32_WHITE, ImVec2 uv_offset = ImVec2(0,0), ImVec2 uv_scale = ImVec2(1,1), char const* text_end = nullptr );
	// Debug: draw curve outlines, control points, and bounding boxes for Slug glyphs. font_size in lp.
	// flags: 1=curves, 2=control points, 4=bounding boxes, 8=band grid, 0xFF=all
	IMGUI_API void DrawTextDebugCurves( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, char const* text_end = nullptr, int flags = 0xFF );
	// Debug: draw each color layer's quad as a flat semi-transparent rectangle (no Slug shader). font_size in lp.
	IMGUI_API void DrawTextDebugLayers( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, char const* text, char const* text_end = nullptr );
	// Set to true to use the debug shader (xcov=R, ycov=G, coverage=B) instead of normal rendering.
	IMGUI_API extern bool g_SlugDebugShader;
	// ImFontLoader backend: rasterizes Slug glyphs (including color/gradient) into ImGui's bitmap atlas.
	// Use with: cfg.FontLoader = ImWidgets::GetSlugFontLoader();
	IMGUI_API ImFontLoader const* GetSlugFontLoader();
	IMGUI_API bool GetSlugFontInfo(ImFont* font, void* outStbttFontInfo, float* outEmScale); // outStbttFontInfo = stbtt_fontinfo*
	IMGUI_API void SlugBuildGlyphByID(ImFont* font, int glyphID);
	// LaTeX math rendering via Slug GPU fonts. font_size in lp.
	// Requires ImWidgetsFeatures_LaTeX to be set before CreateContext().
	// latex: LaTeX math string (e.g. "x^2 + \\frac{\\alpha}{\\beta} = 0")
	// Call during font loading phase (before CreateContext) to load the Latin Modern Math font.
	IMGUI_API void LoadLaTeXFont();
	IMGUI_API void DrawLaTeX( ImDrawList* pDrawList, float font_size, ImVec2 pos, ImU32 col, char const* latex );
	// Measure the bounding box of a LaTeX expression. font_size in lp. Returns size in physical pixels.
	IMGUI_API ImVec2 CalcLaTeXSize( float font_size, char const* latex );
	// Debug: draw bounding boxes for each glyph/box in a LaTeX expression. font_size in lp.
	IMGUI_API void DrawLaTeXDebug( ImDrawList* pDrawList, float font_size, ImVec2 pos, char const* latex );
	// Tessellate a LaTeX expression into an ImWidgetsShape for gradient/image fills. font_size in lp.
	// pos = top-left corner (same convention as DrawLaTeX). Use CalcLaTeXSize for layout size.
	IMGUI_API void TesselateLaTeX( float font_size, char const* latex, ImVec2 pos, ImWidgetsShape& outShape, float tess_tol = 0.25f, int iterations = 0 );

	//////////////////////////////////////////////////////////////////////////
	// DrawList
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API void DrawShapeDebugEx( ImDrawList* pDrawList, ImTextureID tex, ImWidgetsShape& shape, float edge_thickness, ImU32 edge_col, ImU32 triangle_col, float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1 );
	IMGUI_API void DrawShapeEx( ImDrawList* pDrawList, ImTextureID tex, ImWidgetsShape& shape );
	IMGUI_API void DrawShapeDebug( ImDrawList* pDrawList, ImWidgetsShape& shape, float edge_thickness, ImU32 edge_col, ImU32 triangle_col, float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1 );
	IMGUI_API void DrawShape( ImDrawList* pDrawList, ImWidgetsShape& shape );
	IMGUI_API void DrawImageShapeDebug( ImDrawList* pDrawList, ImTextureID tex, ImWidgetsShape& shape, float edge_thickness, ImU32 edge_col, ImU32 triangle_col, float vrtx_radius, ImU32 vrtx_col, int tri_idx = -1 );
	IMGUI_API void DrawImageShape( ImDrawList* pDrawList, ImTextureID tex, ImWidgetsShape& shape );

	IMGUI_API void DrawTriangleCursor( ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, float thickness, ImU32 col );
	IMGUI_API void DrawTriangleCursorFilled( ImDrawList* pDrawList, ImVec2 targetPoint, float angle, float size, ImU32 col );

	IMGUI_API void DrawSignetCursor( ImDrawList* pDrawList, ImVec2 targetPoint, float width, float height, float height_ratio, float align01, float angle, float thickness, ImU32 col );
	IMGUI_API void DrawSignetFilledCursor( ImDrawList* pDrawList, ImVec2 targetPoint, float width, float height, float height_ratio, float align01, float angle, ImU32 col );

	IMGUI_API void DrawProceduralColor1DNearestHorizontal( ImDrawList* pDrawList, ImWidgetsColor1DCallback func, void* pUserData, float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX );
	IMGUI_API void DrawProceduralColor1DNearestVertical( ImDrawList* pDrawList, ImWidgetsColor1DCallback func, void* pUserData, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionY );
	IMGUI_API void DrawProceduralColor1DBilinearHorizontal( ImDrawList* pDrawList, ImWidgetsColor1DCallback func, void* pUserData, float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX );
	IMGUI_API void DrawProceduralColor1DBilinearVertical( ImDrawList* pDrawList, ImWidgetsColor1DCallback func, void* pUserData, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionY );

	IMGUI_API void DrawProceduralColor2DNearest( ImDrawList* pDrawList, ImWidgetsColor2DCallback func, void* pUserData, float minX, float maxX, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );
	IMGUI_API void DrawProceduralColor2DBilinear( ImDrawList* pDrawList, ImWidgetsColor2DCallback func, void* pUserData, float minX, float maxX, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionX, int resolutionY );

	IMGUI_API void DrawHueBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float alpha, float gamma, float offset );
	IMGUI_API void DrawHueBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float colorStartRGB[ 3 ], float alpha, float gamma );
	IMGUI_API void DrawLumianceBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma );
	IMGUI_API void DrawSaturationBand( ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma );

	IMGUI_API void DrawProceduralColorArcBilinear( ImDrawList* pDrawList, ImVec2 center, float innerRadius, float outerRadius, float startAngle, float sweepAngle, ImWidgetsColor1DCallback func, void* pUserData, int division, bool bilinear );
	IMGUI_API void DrawProceduralColorSplineBilinear( ImDrawList* pDrawList, ImVec2 const* points, int points_count, float thickness, ImWidgetsColor1DCallback func, void* pUserData, int resolution, bool closed );

	IMGUI_API void DrawColorRing( ImDrawList* pDrawList, ImVec2 const curPos, ImVec2 const size, float thickness_, ImWidgetsColor1DCallback func, void* pUserData, int division, float colorOffset, bool bIsBilinear );

	IMGUI_API ImVec4 GradientSample( ImGradientData const& gradient, float t );
	IMGUI_API float  GradientAlphaSample( ImGradientData const& gradient, float t );
	IMGUI_API void DrawCheckerboard( ImDrawList* pDrawList, ImVec2 position, ImVec2 size, float cellSize, ImU32 col1, ImU32 col2 );
	IMGUI_API void DrawGradientBar( ImDrawList* pDrawList, ImGradientData const& gradient, ImVec2 position, ImVec2 size, int resolution );
	IMGUI_API void DrawSplineGradient( ImDrawList* pDrawList, ImGradientData const& gradient, ImVec2 const* points, int points_count, float thickness, int resolution, bool closed = false );
	IMGUI_API void DrawSplineGradientCut( ImDrawList* pDrawList, ImGradientData const& gradient, float const min, float const max, ImVec2 const* points, int points_count, float thickness, int resolution, bool closed = false );

	IMGUI_API float CurveEditorEvalEasing( ImCurveEditorSeg seg, float t );
	IMGUI_API float CurveEditorSample( ImCurveEditorData const& curve, float x );
	IMGUI_API char const* CurveEditorSegName( ImCurveEditorSeg seg );
	IMGUI_API char const* CurveEditorTangentModeName( ImCurveEditorTangentMode mode );

	IMGUI_API void DrawOkLabQuad( ImDrawList* pDrawList, ImVec2 start, ImVec2 size, float L, int resX = 16, int resY = 16 );
	IMGUI_API void DrawOkLchQuad( ImDrawList* pDrawList, ImVec2 start, ImVec2 size, float L, int resX = 16, int resY = 16 );

	// poly: Clockwise: Positive shape & Counter-clockwise for hole
	IMGUI_API void DrawShapeWithHole( ImDrawList* draw, ImVec2* poly, int points_count, ImU32 color, ImRect* p_bb = NULL, int gap = 1, int strokeWidth = 1 );

	// Solid-colour counterparts of the DrawImage*Shape pair below: same poly
	// format, no texture. DrawConcaveShape ear-clips, so it accepts any simple
	// polygon (self-intersection is still undefined) and -- unlike
	// DrawImageConcaveShape -- honours ImDrawListFlags_AntiAliasedFill.
	IMGUI_API void DrawConvexShape( ImDrawList* draw, ImVec2* poly, int points_count, ImU32 color );
	IMGUI_API void DrawConcaveShape( ImDrawList* draw, ImVec2* poly, int points_count, ImU32 color );

	IMGUI_API void DrawImageConvexShape( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count, ImU32 tint,
										 ImVec2 uv_offset = ImVec2( 0.0f, 0.0f ), ImVec2 uv_scale = ImVec2( 1.0f, 1.0f ) );
	IMGUI_API void DrawImageConcaveShape( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count, ImU32 tint,
										  ImVec2 uv_offset = ImVec2( 0.0f, 0.0f ), ImVec2 uv_scale = ImVec2( 1.0f, 1.0f ) );
	// poly: Clockwise outer contour & Counter-clockwise for holes (same format as DrawShapeWithHole)
	IMGUI_API void DrawImageShapeWithHole( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count, ImU32 tint,
										   ImVec2 uv_offset = ImVec2( 0.0f, 0.0f ), ImVec2 uv_scale = ImVec2( 1.0f, 1.0f ),
										   int gap = 3, int strokeWidth = 3 );

#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
	// compile_flags: IMPLATFORM_SHADER_COMPILE_* bitmask. Default is the
	// backend's moderate optimization. Lower it (OPTIMIZATION_LOW or
	// SKIP_OPTIMIZATION) for large uber-shaders where the optimizer goes
	// polynomial â€” e.g. image_inspector with its many [branch] switches over
	// 11 sample types x 7 tonemaps x 8 false-colour palettes takes ~19 min
	// at the default level but seconds with LOW.
	IMGUI_API void CreateInternalShader( ImDrawShader* shaders_out, char const* shader_name, int sizeof_vs_const_buffer, void *vs_const_buffer, int sizeof_ps_const_buffer, void *ps_const_buffer, char const* extra_define = nullptr, char const* cache_suffix = nullptr, unsigned int compile_flags = IMPLATFORM_SHADER_COMPILE_DEFAULT );

	// Eagerly compile/load every internal shader used by Dear Widgets.
	// Optional: shaders are lazily compiled on first use otherwise. Call this once
	// after CreateContext() to pay the compile cost up-front (e.g. at app startup,
	// before showing a UI) and avoid frame hitches on first widget draw.
	// Idempotent: already-loaded shaders are skipped. Slug shaders are only loaded
	// if ImWidgetsFeatures_RichFont or ImWidgetsFeatures_LaTeX is enabled.
	IMGUI_API void PrebuildShaders();

	IMGUI_API void DrawMarker( ImDrawList* pDrawList, ImVec2 start, ImVec2 size,
							   ImU32 fg_color,
							   ImU32 bg_color,
							   float rot_angle_rad,
							   float shape_size,
							   float linewidth,
							   float antialiasing,
							   ImWidgetsMarker marker,
							   ImWidgetsDrawType draw_type );
#endif

	IMGUI_API void DrawChromaticityPlot( ImDrawList* draw,
										 ImWidgetsIlluminant illuminance,
										 ImWidgetsObserver observer,
										 ImWidgetsColorSpace colorSpace,
										 int chromeLineSamplesCount,
										 ImVec2 const vpos, ImVec2 const size,
										 int resolutionX, int resolutionY,
										 ImU32 maskColor,
										 float wavelengthMin = 400.0f, float wavelengthMax = 700.0f,
										 float minX = 0.0f, float maxX = 0.8f,
										 float minY = 0.0f, float maxY = 0.9f,
										 bool showColorSpaceTriangle = true,
										 bool showWhitePoint = true,
										 bool showBorder = true,
										 ImU32 borderColor = IM_COL32( 0, 0, 0, 255 ),
										 float borderThickness = 1.0f );
	IMGUI_API void DrawChromaticityPoints( ImDrawList* pDrawList,
										   ImVec2 curPos,
										   ImVec2 size,
										   ImU32* colors4,
										   int color_count,
										   float minX, float maxX,
										   float minY, float maxY,
										   ImU32 plotColor, float radius, int num_segments );
	IMGUI_API void DrawChromaticityPoints( ImDrawList* pDrawList,
										  ImVec2 curPos,
										  ImVec2 size,
										  ImWidgetsColorSpace colorSpace,
										  float* colors4, // AoS
										  int color_count,
										  float minX, float maxX,
										  float minY, float maxY,
										  ImU32 plotColor, float radius, int num_segments,
										  int colorStride = 4 ); // 4 for rgba,rgba,rgba,...; 3 for rgb,rgb,rgb,... or anything else );
	IMGUI_API void DrawChromaticityLines( ImDrawList* pDrawList,
										  ImVec2 curPos,
										  ImVec2 size,
										  ImU32* color,
										  int color_count,
										  float minX, float maxX,
										  float minY, float maxY,
										  ImU32 plotColor, ImDrawFlags flags, float thickness );
	IMGUI_API void DrawChromaticityLines( ImDrawList* pDrawList,
										  ImVec2 curPos,
										  ImVec2 size,
										  ImWidgetsColorSpace colorSpace,
										  float* colors4, // AoS
										  int color_count,
										  float minX, float maxX,
										  float minY, float maxY,
										  ImU32 plotColor, ImDrawFlags flags, float thickness,
										  int colorStride = 4 ); // 4 for rgba,rgba,rgba,...; 3 for rgb,rgb,rgb,... or anything else );

	IMGUI_API void DrawLinearLineGraduation( ImDrawList* drawlist, ImVec2 start, ImVec2 end,
										 float mainLineThickness, ImU32 mainCol,
										 int division0, float height0, float thickness0, float angle0, ImU32 col0,
										 int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u,
										 int division2 = -1, float height2 = -1.0f, float thickness2 = -1.0f, float angle2 = -1.0f, ImU32 col2 = 0u );
	IMGUI_API void DrawLinearCircularGraduation( ImDrawList* drawlist, ImVec2 center, float radius, float start_angle, float end_angle, int num_segments,
												 float mainLineThickness, ImU32 mainCol,
												 int division0, float height0, float thickness0, float angle0, ImU32 col0,
												 int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u,
												 int division2 = -1, float height2 = -1.0f, float thickness2 = -1.0f, float angle2 = -1.0f, ImU32 col2 = 0u );

	IMGUI_API void DrawLogLineGraduation( ImDrawList *drawlist, ImVec2 start, ImVec2 end,
										  float mainLineThickness, ImU32 mainCol,
										  int division0, float height0, float thickness0, float angle0, ImU32 col0,
										  int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u );

	IMGUI_API void DrawLogCircularGraduation( ImDrawList *drawlist, ImVec2 center, float radius, float start_angle, float end_angle, int num_segments,
											  float mainLineThickness, ImU32 mainCol,
											  int division0, float height0, float thickness0, float angle0, ImU32 col0,
											  int division1 = -1, float height1 = -1.0f, float thickness1 = -1.0f, float angle1 = -1.0f, ImU32 col1 = 0u );

	// Schematic ephemerides (draw-only, no state/interaction).
	IMGUI_API void DrawMoonEphemeris( ImDrawList* pDrawList, ImVec2 center, float radius,
	                                  int year, int month, int day, float lonDeg, float latDeg,
	                                  ImU32 litCol = IM_COL32( 190, 185, 172, 255 ),  // dimmed lunar albedo
	                                  ImU32 darkCol = IM_COL32( 28, 36, 56, 255 ),    // earthshine tint
	                                  ImU32 outlineCol = IM_COL32( 170, 175, 200, 255 ) );
	IMGUI_API void DrawEarthSunEphemeris( ImDrawList* pDrawList, ImVec2 areaMin, ImVec2 areaSize,
	                                      int year, int month, int day, int hour, int minute,
	                                      float obsLon = 0.0f, float obsLat = 0.0f,
	                                      ImU32 sunCol = IM_COL32( 255, 220, 80, 255 ),
	                                      ImU32 dayCol = IM_COL32( 110, 165, 220, 255 ),
	                                      ImU32 nightCol = IM_COL32( 25, 35, 60, 255 ),
	                                      ImU32 outlineCol = IM_COL32( 180, 190, 210, 255 ) );

	// Sky cultures supported by DrawStarChart. Each defines its own
	// constellation figure lines + native star names + solar-system body names.
	enum ImWidgetsSkyCulture
	{
		ImWidgetsSkyCulture_Western = 0,    // IAU modern, 88 constellations
		ImWidgetsSkyCulture_Chinese,        // Han-dynasty 28 lunar mansions
		ImWidgetsSkyCulture_Arabic,         // Manazil al-Qamar (Arabic right-to-left text)
		ImWidgetsSkyCulture_Polynesian,     // Maori navigation
		ImWidgetsSkyCulture_COUNT
	};

	IMGUI_API char const* GetSkyCultureName( ImWidgetsSkyCulture culture );
	IMGUI_API int         GetSkyCultureCount();

	// Observer-centred sky chart. Stars projected via equatorial-to-horizon
	// (alt/az). Culture-specific constellation figure lines + native star
	// names + solar-system body names. Solar system overlay shows Moon
	// (with phase), naked-eye planets, and Sun when above horizon.
	IMGUI_API void DrawStarChart( ImDrawList* pDrawList, ImVec2 center, float radius,
	                              int year, int month, int day, int hour, int minute,
	                              float obsLon = 0.0f, float obsLat = 48.85f,
	                              float magLimit = 5.5f, float starScale = 1.0f,
	                              ImWidgetsSkyCulture culture = ImWidgetsSkyCulture_Western,
	                              bool showSolarSystem = true,
	                              ImU32 skyCol = IM_COL32( 8, 11, 22, 255 ),
	                              ImU32 outlineCol = IM_COL32( 180, 195, 225, 230 ),
	                              // Optional label font. When the culture uses non-Latin
	                              // glyphs (Arabic, CJK), pass a font with the matching
	                              // ranges; otherwise the active ImGui font is used.
	                              ImFont* labelFont = nullptr );

	// Sun-path diagram + analemma. Polar mode = zenith-centred dome (sun arcs
	// are radial curves from sunrise to sunset); Cartesian = azimuth x altitude.
	// Yellow analemma figure-8 loops are drawn at five fixed local clock hours
	// (06h/09h/12h/15h/18h). Equation of time + longitude offset from time-zone
	// meridian produce the east-west spread of the analemma.
	enum ImWidgetsSunPathMode
	{
		ImWidgetsSunPathMode_Polar = 0,
		ImWidgetsSunPathMode_Cartesian
	};
	IMGUI_API void DrawSunPath( ImDrawList* pDrawList, ImVec2 areaMin, ImVec2 areaSize,
	                            float obsLat = 48.85f, float obsLon = 0.0f,
	                            int tzOffset = 0, int year = 2026,
	                            ImWidgetsSunPathMode mode = ImWidgetsSunPathMode_Polar,
	                            ImU32 bgCol = IM_COL32( 12, 18, 30, 255 ),
	                            ImU32 outlineCol = IM_COL32( 190, 205, 230, 240 ) );

	typedef void ( *ImInlineOffset )( void* data, ImVec2 offset );
	typedef void ( *ImDrawShape )( ImDrawList* drawlist, ImU32 col, float thickness, void* data );
	typedef void ( *ImDrawShapeFilled )( ImDrawList* drawlist, ImU32 col, void* data );
	typedef void ( *ImDrawShapeFilledTex )( ImDrawList* drawlist, ImU32 col, void* data, ImTextureID tex, ImVec2 uv_min, ImVec2 uv_max );
	typedef bool ( *IsContains )( ImVec2 p, void* data );
	typedef void ( *FromRect )( ImRect r, void* data );

	IMGUI_API void RenderNavCursorEx( ImGuiID id, ImDrawShape func, void* data, ImRect display_rect, ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );
	IMGUI_API void RenderNavCursorCircle( ImVec2 center, float radius, ImGuiID id, ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );
	IMGUI_API void RenderNavCursorConvex( ImVec2* pts, int pts_count, ImGuiID id, ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );
	IMGUI_API void RenderNavCursorConcave( ImVec2* pts, int pts_count, ImGuiID id, ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );
	IMGUI_API void RenderCursorWithHole( ImVec2* pts, int pts_count, ImGuiID id, ImGuiNavRenderCursorFlags flags = ImGuiNavRenderCursorFlags_None );

	IMGUI_API void RenderFrameEx( ImU32 fill_col, bool border, ImDrawShape outline, ImDrawShapeFilled fill, ImDrawShapeFilledTex fill_tex, void* data, ImTextureID* tex = NULL, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API void RenderFrameCircle( ImVec2 center, float radius, ImU32 fill_col, bool border );
	IMGUI_API void RenderFrameConcave( ImVec2* pts, int pts_count, ImU32 fill_col, bool border );
	IMGUI_API void RenderFrameConvex( ImVec2* pts, int pts_count, ImU32 fill_col, bool border );
	IMGUI_API void RenderFrameWithHole( ImVec2* pts, int pts_count, ImU32 fill_col, bool border );

	//////////////////////////////////////////////////////////////////////////
	// Interactions
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API bool IsBoundingBoxWellFormed( ImVec2 const& r_min, ImVec2 const& r_max, ImVec2* pts, int pts_count );

	IMGUI_API bool Im_IsCircleContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsCapsuleHContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsCapsuleVContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsPolyConvexContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsPolyConcaveContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsPolyWithHoleContains( ImVec2 p, void* data );

	IMGUI_API bool IsMouseHovering( ImVec2 const& r_min, ImVec2 const& r_max, IsContains contains, void* data, bool clip = true );
	IMGUI_API bool ItemHoverable( ImRect const& bb, ImGuiID id, ImGuiItemFlags item_flags, IsContains isContains, void* extra_data );

	//typedef bool ( *ImItemHoverableFunc )( ImRect const& bb, ImGuiID id, ImGuiItemFlags item_flags, void* extra_data );
	IMGUI_API bool ButtonBehaviorEx( ImRect const& bb, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags, IsContains isContains, void* extra_data );
	IMGUI_API bool ButtonBehaviorCircle( ImVec2 center, float radius, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorCapsuleH( ImVec2 pos, float length, float radius, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorCapsuleV( ImVec2 pos, float length, float radius, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorConvex( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorConcave( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorWithHole( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );

	//////////////////////////////////////////////////////////////////////////
	// Widgets
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API bool ButtonEx( char const* label, ImVec2 const& size_arg, ImRect bb, ImVec2 text_offset, ImGuiButtonFlags flags,
							 IsContains isContains, ImDrawShape outline, ImDrawShapeFilled fill, ImDrawShapeFilledTex fill_tex, ImInlineOffset offset, FromRect from_rect,
							 void* extra_data,
							 ImTextureID* tex = NULL, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ButtonExCircle( char const* label, float radius, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExCapsuleH( char const* label, float length, float thickness, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExCapsuleV( char const* label, float length, float thickness, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExConvex( char const* label, ImVec2 const& size_arg, ImVec2* pts, int pts_count, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExConcave( char const* label, ImVec2 const& size_arg, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExWithHole( char const* label, ImVec2 const& size_arg, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags );

	IMGUI_API bool ImageButtonExCircle( char const* label, ImTextureID tex, float radius, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ImageButtonExCapsuleH( char const* label, ImTextureID tex, float length, float thickness, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ImageButtonExCapsuleV( char const* label, ImTextureID tex, float length, float thickness, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ImageButtonExConvex( char const* label, ImTextureID tex, ImVec2 const& size_arg, ImVec2* pts, int pts_count, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ImageButtonExConcave( char const* label, ImTextureID tex, ImVec2 const& size_arg, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );

	IMGUI_API bool HueSelector( char const* label, float hueHeight, float cursorHeight, float* hueCenter, float* hueWidth, float* featherLeft, float* featherRight, int division = 32, float alpha = 1.0f, float hideHueAlpha = 0.75f, float offset = 0.0f );
	IMGUI_API bool SliderNScalar( char const* label, ImGuiDataType data_type, void* ordered_value, int value_count, void* p_min, void* p_max, float cursor_width, bool show_hover_by_region );
	IMGUI_API bool SliderNFloat( char const* label, float* ordered_value, int value_count, float v_min, float v_max, float cursor_width, bool show_hover_by_region );
	IMGUI_API bool SliderNInt( char const* label, int* ordered_value, int value_count, int v_min, int v_max, float cursor_width, bool show_hover_by_region );
	IMGUI_API bool SliderNVerticalScalar( char const* label, ImGuiDataType data_type, void* ordered_value, int value_count, void* p_min, void* p_max, float cursor_height, bool show_hover_by_region, ImVec2 size = ImVec2( 0.0f, 0.0f ) );
	IMGUI_API bool SliderNVerticalFloat( char const* label, float* ordered_value, int value_count, float v_min, float v_max, float cursor_height, bool show_hover_by_region, ImVec2 size = ImVec2( 0.0f, 0.0f ) );
	IMGUI_API bool SliderNVerticalInt( char const* label, int* ordered_value, int value_count, int v_min, int v_max, float cursor_height, bool show_hover_by_region, ImVec2 size = ImVec2( 0.0f, 0.0f ) );
	// TODO: Add bool flipY
	IMGUI_API bool Slider2DScalar( char const* pLabel, ImGuiDataType data_type, void* pValueX, void* pValueY, void* p_minX, void* p_maxX, void* p_minY, void* p_maxY );
	IMGUI_API bool Slider2DFloat( char const* pLabel, float* pValueX, float* pValueY, float v_minX, float v_maxX, float v_minY, float v_maxY );
	IMGUI_API bool Slider2DInt( char const* pLabel, int* pValueX, void* pValueY, int v_minX, int v_maxX, int v_minY, int v_maxY );
	IMGUI_API bool Slider2DRangeScalar( char const* label, ImGuiDataType data_type, void* pMinX, void* pMinY, void* pMaxX, void* pMaxY, void* bMinX, void* bMaxX, void* bMinY, void* bMaxY );
	IMGUI_API bool Slider2DRangeFloat( char const* label, float* pMinX, float* pMinY, float* pMaxX, float* pMaxY, float bMinX, float bMaxX, float bMinY, float bMaxY );
	IMGUI_API bool Slider2DRangeInt( char const* label, int* pMinX, int* pMinY, int* pMaxX, int* pMaxY, int bMinX, int bMaxX, int bMinY, int bMaxY );
	IMGUI_API bool Slider2DDiscScalar( char const* label, ImGuiDataType data_type, void* pValueX, void* pValueY, void* p_min, void* p_max );
	IMGUI_API bool Slider2DDiscFloat( char const* label, float* pValueX, float* pValueY, float v_min, float v_max );
	IMGUI_API bool Slider2DDiscInt( char const* label, int* pValueX, int* pValueY, int v_min, int v_max );

	// SliderGradient: 1D slider with an ImGradientData-backed background. Color space comes from gradient->Interpolation.
	// `fill_up_to_cursor` (default false): when true, the gradient renders only
	// for t in [0, current value]. Past the cursor the ImGui FrameBg shows
	// through -- same idea / same callback trick as SliderSplineGradient.
	// `right_to_left` (default false): mirror the cursor direction. t=0 places
	// the grab at the right edge, t=1 at the left edge. Combined with
	// `fill_up_to_cursor`, the fill grows from the right. Gradient colors keep
	// their natural positions (blue at 0, orange at 1 for a blue->orange grad).
	IMGUI_API bool SliderGradientScalar( char const* label, ImGuiDataType data_type, void* p_value, void const* p_min, void const* p_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ), bool fill_up_to_cursor = false, bool right_to_left = false );
	IMGUI_API bool SliderGradientFloat( char const* label, float* v, float v_min, float v_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ), bool fill_up_to_cursor = false, bool right_to_left = false );
	IMGUI_API bool SliderGradientInt( char const* label, int* v, int v_min, int v_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ), bool fill_up_to_cursor = false, bool right_to_left = false );

	// How much of the gradient track is revealed, relative to the grab.
	//  - None:       the whole track is always visible.
	//  - FromMin:    revealed from the v_min edge up to the grab (Resolve's RGB
	//                Mixer bars, and Gain, grow this way from the bottom).
	//  - FromCenter: revealed from the track centre out to the grab, in either
	//                direction (Resolve's Lift / Gamma / Offset bars, where the
	//                centre is the neutral value and the fill shows the signed
	//                departure from it).
	enum ImWidgetsSliderFill_
	{
		ImWidgetsSliderFill_None = 0,
		ImWidgetsSliderFill_FromMin,
		ImWidgetsSliderFill_FromCenter
	};
	typedef int ImWidgetsSliderFill;

	// SliderGradientVertical: vertical twin of SliderGradient. v_min sits at the
	// BOTTOM (set `inverted` to put it at the top). `fill` selects how much of the
	// gradient is revealed. Building block of the colour-grading bar panels.
	IMGUI_API bool SliderGradientVerticalScalar( char const* label, ImGuiDataType data_type, void* p_value, void const* p_min, void const* p_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ), ImWidgetsSliderFill fill = ImWidgetsSliderFill_None, bool inverted = false );
	IMGUI_API bool SliderGradientVerticalFloat( char const* label, float* v, float v_min, float v_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ), ImWidgetsSliderFill fill = ImWidgetsSliderFill_None, bool inverted = false );
	IMGUI_API bool SliderGradientVerticalInt( char const* label, int* v, int v_min, int v_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ), ImWidgetsSliderFill fill = ImWidgetsSliderFill_None, bool inverted = false );

	//////////////////////////////////////////////////////////////////////////
	// DragFloat with a colour / gradient underline (the Resolve grading-panel
	// numeric field): a borderless drag value with a thin coloured bar beneath
	// it hinting at what the parameter does. Double-click the value resets it to
	// `v_default` -- so ImGuiSliderFlags_NoInput is forced internally, since
	// ImGui would otherwise consume the double-click for text entry.
	// `width` / `underline_thickness` are logical px (0 = default).
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API bool DragFloatColorUnderline( char const* label, float* v, float v_speed,
											float v_min, float v_max, float v_default,
											ImU32 underline_col, char const* format = "%.2f",
											float width = 0.0f, float underline_thickness = 0.0f );
	IMGUI_API bool DragFloatGradientUnderline( char const* label, float* v, float v_speed,
											   float v_min, float v_max, float v_default,
											   ImGradientData const* gradient, char const* format = "%.2f",
											   float width = 0.0f, float underline_thickness = 0.0f );

	// Ready-made underline gradients matching the Resolve grading controls.
	// Returned pointers are owned by the library and stay valid.
	enum ImWidgetsGradingGradient_
	{
		ImWidgetsGradingGradient_Temperature = 0,	// blue -> orange
		ImWidgetsGradingGradient_Tint,				// green -> magenta
		ImWidgetsGradingGradient_Contrast,			// black -> white
		ImWidgetsGradingGradient_Pivot,				// black -> white -> black
		ImWidgetsGradingGradient_MidDetail,			// white -> black
		ImWidgetsGradingGradient_Saturation,		// grey -> grey -> hue sweep
		ImWidgetsGradingGradient_Hue,				// full hue sweep
		ImWidgetsGradingGradient_LumMix,			// hue sweep -> white -> white
		ImWidgetsGradingGradient_COUNT
	};
	typedef int ImWidgetsGradingGradient;
	IMGUI_API ImGradientData const* GradingGradient( ImWidgetsGradingGradient which );

	// SliderGradientRange: two-handle horizontal range slider with gradient cut between handles.
	// Picks nearest handle on click. Lower is clamped to <= upper; upper to >= lower.
	IMGUI_API bool SliderGradientRangeScalar( char const* label, ImGuiDataType data_type, void* p_lower, void* p_upper, void const* p_min, void const* p_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ), bool right_to_left = false );
	IMGUI_API bool SliderGradientRangeFloat( char const* label, float* v_lower, float* v_upper, float v_min, float v_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ), bool right_to_left = false );
	IMGUI_API bool SliderGradientRangeInt( char const* label, int* v_lower, int* v_upper, int v_min, int v_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ), bool right_to_left = false );

	// SliderGradientRing: interactive ring/arc slider with gradient background.
	// Full-circle overload: value wraps at boundaries (natural for hue).
	// `fill_up_to_cursor` has identical semantics to SliderGradient above.
	IMGUI_API bool SliderGradientRingScalar( char const* label, ImGuiDataType data_type, void* p_value, void const* p_min, void const* p_max, ImGradientData const* gradient, float outerRadius, float thickness, bool fill_up_to_cursor = false );
	IMGUI_API bool SliderGradientRingFloat( char const* label, float* v, float v_min, float v_max, ImGradientData const* gradient, float outerRadius, float thickness, bool fill_up_to_cursor = false );
	IMGUI_API bool SliderGradientRingInt( char const* label, int* v, int v_min, int v_max, ImGradientData const* gradient, float outerRadius, float thickness, bool fill_up_to_cursor = false );
	// Arc overload: caller specifies startAngle + sweepAngle (radians). Value is clamped (no wrap).
	IMGUI_API bool SliderGradientRingScalar( char const* label, ImGuiDataType data_type, void* p_value, void const* p_min, void const* p_max, ImGradientData const* gradient, float outerRadius, float thickness, float startAngle, float sweepAngle, bool fill_up_to_cursor = false );
	IMGUI_API bool SliderGradientRingFloat( char const* label, float* v, float v_min, float v_max, ImGradientData const* gradient, float outerRadius, float thickness, float startAngle, float sweepAngle, bool fill_up_to_cursor = false );
	IMGUI_API bool SliderGradientRingInt( char const* label, int* v, int v_min, int v_max, ImGradientData const* gradient, float outerRadius, float thickness, float startAngle, float sweepAngle, bool fill_up_to_cursor = false );

	// SliderGradientRingRange: two-handle arc range slider with gradient cut between handles.
	// Non-wrap arcs only (caller supplies startAngle + signed sweepAngle). For full hue rings
	// prefer two separate SliderGradientRing calls (wrap semantics are ambiguous for range).
	IMGUI_API bool SliderGradientRingRangeScalar( char const* label, ImGuiDataType data_type, void* p_lower, void* p_upper, void const* p_min, void const* p_max, ImGradientData const* gradient, float outerRadius, float thickness, float startAngle, float sweepAngle );
	IMGUI_API bool SliderGradientRingRangeFloat( char const* label, float* v_lower, float* v_upper, float v_min, float v_max, ImGradientData const* gradient, float outerRadius, float thickness, float startAngle, float sweepAngle );
	IMGUI_API bool SliderGradientRingRangeInt( char const* label, int* v_lower, int* v_upper, int v_min, int v_max, ImGradientData const* gradient, float outerRadius, float thickness, float startAngle, float sweepAngle );

	// Unit Field: DragFloat with built-in unit selector
	IMGUI_API bool UnitField( char const* label, float* pValue, ImUnitDef* units, int unitCount, int* pSelectedUnit, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, char const* format = NULL );

	// Paint Canvas
	IMGUI_API bool PaintCanvas( char const* label, ImPaintCanvasData* canvas, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API bool GradientEditor( char const* label, ImGradientData* gradient, bool alpha = true, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool CurveEditor( char const* label, ImCurveEditorData* curve, ImVec2 size = ImVec2( 0, 0 ) );

	// Free-form deformation grid editor: drag the lattice nodes to warp. When
	// `background` is non-zero it is drawn behind the grid (e.g. the node's
	// input image) so the deformation can be judged against real content.
	// Edits ImGridWarpData::Offsets; returns true while a node is being moved.
	IMGUI_API bool GridWarp( char const* label, ImGridWarpData* grid, ImTextureID background = 0, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API void DrawColorDisc( ImDrawList* pDrawList, ImVec2 center, float radius, ImColorWheelMode mode, float thirdAxis, int numSectors = 64, int numRings = 16 );
	IMGUI_API void DrawCircularGradientIndicator( ImDrawList* pDrawList, ImVec2 center, float outerRadius, float innerRadius, float t );
	IMGUI_API bool ColorWheel( char const* label, ImVec4* color, ImColorWheelMode mode = ImColorWheelMode_HSV, float hdr_max = 1.0f, bool fixedIntensity = false, ImVec2 size = ImVec2( 0, 0 ) );
	// HDRWheel: unified ColorWheel + gradient indicator ring + optional right/left arc sliders.
	// Pass rightValue = leftValue = NULL to disable arc sliders (compact layout: indicator
	// ring + ColorWheel only). With either pointer non-null, the corresponding side arc
	// slider is drawn â€” the widget grows by 2*(ArcGrabRadius + ArcThickness + ArcGap) lp.
	IMGUI_API bool HDRWheel( char const* label, ImVec4* color, float* yValue, float yMin, float yMax, float* rightValue = NULL, float rightMin = 0.0f, float rightMax = 0.0f, float* leftValue = NULL, float leftMin = 0.0f, float leftMax = 0.0f, ImColorWheelMode mode = ImColorWheelMode_HSV, ImVec2 size = ImVec2( 0, 0 ) );

	//////////////////////////////////////////////////////////////////////////
	// Colour Bars (a.k.a. "Primaries Bars"): the bar counterpart of a primaries
	// wheel -- four vertical gradient sliders, Y (master) + R + G + B, each on a
	// black->channel-tint track. One call is ONE grading block (Lift, Gamma,
	// Gain or Offset); stack four side by side to get the full Resolve panel.
	//
	// `yrgb` is a 4-float array [Y, R, G, B] shared in/out. When `link_master`
	// is set, dragging Y applies the same delta to R/G/B (master luminance
	// move); the channel bars always stay independent. Double-click a channel
	// bar to reset it to `v_default`, or the Y bar to reset all four.
	// `size` is one bar's width x height in logical px (0 = default).
	//
	// `fill` picks the track style, and it is the parameter that distinguishes
	// the two Resolve usages:
	//   - FromCenter -> Lift / Gamma / Offset (signed move away from neutral)
	//   - FromMin    -> Gain, and every RGB Mixer bar (grows from the bottom)
	// When `show_values` is set each bar gets a DragFloat underneath, underlined
	// in the channel colour (white for Y); double-click a value to reset it.
	// Returns true the frame any value changed.
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API bool ColorBars( char const* label, float* yrgb,
							  float v_min = -1.0f, float v_max = 1.0f, float v_default = 0.0f,
							  bool link_master = true, bool show_values = true,
							  ImVec2 size = ImVec2( 0, 0 ),
							  ImWidgetsSliderFill fill = ImWidgetsSliderFill_FromCenter );

	//////////////////////////////////////////////////////////////////////////
	// ColorSlice: a row of "vectorspace slice" modules, each qualifying one
	// wedge of the hue circle and offering Hue / Density / Saturation on it,
	// plus a global row. Modelled on DaVinci Resolve's ColorSlice palette.
	//
	// Design notes (from the BMD reference manual, Ch.135):
	//  - The wedge WIDTH is fixed per slice; `Center` is the only qualifying
	//    control -- it slides the weighting centre inside the wedge and cannot
	//    leave it. There is no width or falloff control.
	//  - Slices deliberately OVERLAP so their qualifiers blend into each other;
	//    a clean matte is explicitly not the goal.
	//  - There is intentionally NO per-slice luminance control: brightness is
	//    handled subtractively inside Saturation so saturated colours don't
	//    become unnaturally bright.
	//  - Density is not Saturation: it adjusts luminance in proportion to how
	//    saturated a pixel already is (dye/ink density), leaving chroma
	//    magnitude alone. Both effects scale with saturation, so near-neutral
	//    pixels are barely touched.
	//
	// This widget is a CONTROL SURFACE: it owns no pixels and applies nothing.
	// The host reads the values and does the grading.
	//////////////////////////////////////////////////////////////////////////
	struct ImColorSliceVector
	{
		char const*	Label;
		float		HueCenterDeg;		// wedge centre on the hue circle (0 = red)
		float		HueHalfWidthDeg;	// FIXED half-width of the qualifying wedge
		float		Center;				// -1..1, weighting centre inside the wedge
		float		Hue;				// -1..1
		float		Density;			// -1..1
		float		Saturation;			// -1..1

		ImColorSliceVector()
			: Label( "" ), HueCenterDeg( 0.0f ), HueHalfWidthDeg( 30.0f ),
			  Center( 0.0f ), Hue( 0.0f ), Density( 0.0f ), Saturation( 0.0f ) {}
	};

	struct ImColorSliceData
	{
		ImVector<ImColorSliceVector>	Slices;

		// Global row (Resolve's labels are abbreviated exactly like this).
		float	Density;		// luminance of saturated colours (subtractive)
		float	DensityDepth;	// "Den.Depth" -- how much Density reaches highlights
		float	Saturation;
		float	SatBalance;		// luminance balance at medium saturation
		float	SatDepth;		// how much Saturation reaches highlights
		float	Hue;

		int		HighlightSlice;	// slice whose highlight button is held, -1 = none

		// Captions above the Density / Saturation bars. Default to "D" / "S";
		// point them at icon-font glyphs (or any UTF-8 string) to override.
		char const*	DensityLabel;
		char const*	SaturationLabel;

		ImColorSliceData()
			: Density( 0.0f ), DensityDepth( 0.0f ), Saturation( 0.0f ),
			  SatBalance( 0.0f ), SatDepth( 0.0f ), Hue( 0.0f ), HighlightSlice( -1 ),
			  DensityLabel( "D" ), SaturationLabel( "S" ) {}

		// Populate the 7 Resolve vectors. NOTE: BMD documents the slice set but
		// NOT its hue angles, so these are hue-wheel angles chosen to match the
		// labels (and to overlap, as Resolve's do). Tune freely.
		void ResolveVectors();
		void ResetSlices();		// per-slice controls -> 0, wedges untouched
		void ResetGlobals();
		void ResetAll();
	};

	// Returns true the frame any value changed. `size` is the whole panel in
	// logical px (0 = fit the content).
	IMGUI_API bool ColorSlice( char const* label, ImColorSliceData& data, ImVec2 size = ImVec2( 0, 0 ) );

	//////////////////////////////////////////////////////////////////////////
	// ChromaWarp: strokes on a chromaticity diagram that warp one colour into
	// another. This is DaVinci Resolve 20's "Chroma Warp" mode of the Color
	// Warper -- a DIFFERENT tool from the Hue-Saturation / Chroma-Luma mesh
	// warper (ColorWarper above); in Resolve the two are mutually exclusive per
	// node.
	//
	// A Normal stroke drags a source colour to a destination and affects every
	// colour along the way -- its influence region is a CAPSULE of constant
	// radius around the whole vector. A Point-to-Point stroke maps the source
	// straight onto the destination without travelling through the hues in
	// between. A Pin marks a colour range to be EXCLUDED (drop one on the white
	// point to keep neutrals from tinting).
	//
	// Like ColorSlice this is a control surface only: it owns no pixels.
	//////////////////////////////////////////////////////////////////////////
	enum ImChromaWarpKind_
	{
		ImChromaWarpKind_Normal = 0,
		ImChromaWarpKind_PointToPoint,
		ImChromaWarpKind_Pin
	};
	typedef int ImChromaWarpKind;

	enum ImChromaWarpTool_
	{
		ImChromaWarpTool_AddNormal = 0,
		ImChromaWarpTool_AddPointToPoint,
		ImChromaWarpTool_AddPin,
		ImChromaWarpTool_Select
	};
	typedef int ImChromaWarpTool;

	struct ImChromaWarpStroke
	{
		ImChromaWarpKind	Kind;
		ImVec2				Source;			// diagram space [0,1]^2, y UP
		ImVec2				Dest;			// unused for pins
		float				ChromaRange;	// 0..0.2 (Resolve's slider range)
		float				Exposure;		// bipolar; destination end only

		ImChromaWarpStroke()
			: Kind( ImChromaWarpKind_Normal ), Source( 0.5f, 0.5f ), Dest( 0.5f, 0.5f ),
			  ChromaRange( 0.04f ), Exposure( 0.0f ) {}
	};

	struct ImChromaWarpData
	{
		ImVector<ImChromaWarpStroke>	Strokes;
		int								Selected;	// -1 = none
		ImChromaWarpTool				Tool;

		// Global -- these apply to every stroke, not just the selected one.
		float	TonalRangeLow;
		float	TonalRangeHigh;
		float	TonalRangePivot;

		ImVec2	WhitePoint;		// diagram space; neutrals live here

		ImChromaWarpData()
			: Selected( -1 ), Tool( ImChromaWarpTool_AddNormal ),
			  TonalRangeLow( 1.0f ), TonalRangeHigh( 1.0f ), TonalRangePivot( 0.5f ),
			  WhitePoint( 0.5f, 0.5f ) {}

		void ResetGlobals() { TonalRangeLow = TonalRangeHigh = 1.0f; TonalRangePivot = 0.5f; }
		void ResetAll()     { Strokes.clear(); Selected = -1; ResetGlobals(); }
	};

	IMGUI_API bool ChromaWarp( char const* label, ImChromaWarpData& data, ImVec2 size = ImVec2( 0, 0 ) );

	//////////////////////////////////////////////////////////////////////////
	// Spectrum editor: draw a spectral power distribution by hand and integrate
	// it against the CIE observer to get a colour.
	//
	// The canvas is wavelength (x) against relative power (y). Drag to paint the
	// curve; the samples between the previous and current mouse position are
	// interpolated, so a fast sweep still lays down a continuous line instead of
	// isolated spikes. The result is XYZ = k * sum( S(lambda) * cmf(lambda) ),
	// normalised so a flat unit SPD lands at Y = 1.
	//
	// This is the hand-drawn counterpart to the physical pickers: those compute
	// a spectrum from a model, this one lets you invent it.
	//////////////////////////////////////////////////////////////////////////
	struct ImSpectrumData
	{
		ImVector<float>	Samples;	// relative power per sample; 0 = no emission
		float			LambdaMin;	// nm at Samples[0]
		float			LambdaMax;	// nm at Samples[Size-1]

		ImSpectrumData() : LambdaMin( 380.0f ), LambdaMax( 730.0f ) { Resize( 64 ); }

		void Resize( int n )
		{
			if ( n < 2 ) n = 2;
			int old = Samples.Size;
			Samples.resize( n );
			for ( int i = old; i < n; ++i ) Samples[ i ] = 0.0f;
		}
		void Fill( float v )
		{
			for ( int i = 0; i < Samples.Size; ++i ) Samples[ i ] = v;
		}
		float LambdaAt( int i ) const
		{
			if ( Samples.Size < 2 ) return LambdaMin;
			return LambdaMin + ( LambdaMax - LambdaMin ) * ( float )i / ( float )( Samples.Size - 1 );
		}
		// Linearly interpolated power at `lambda` nm; 0 outside the stored range.
		float SampleAt( float lambda ) const
		{
			if ( Samples.Size < 2 || lambda < LambdaMin || lambda > LambdaMax ) return 0.0f;
			float t  = ( lambda - LambdaMin ) / ( LambdaMax - LambdaMin ) * ( float )( Samples.Size - 1 );
			int   i0 = ( int )t;
			if ( i0 < 0 ) i0 = 0;
			if ( i0 > Samples.Size - 2 ) i0 = Samples.Size - 2;
			float f = t - ( float )i0;
			return Samples[ i0 ] + ( Samples[ i0 + 1 ] - Samples[ i0 ] ) * f;
		}
		// Gaussian emission line, handy as a starting point. Defined in the .cpp
		// so this header stays free of <math.h> (every TU includes it).
		void SetGaussian( float center_nm, float sigma_nm, float amplitude = 1.0f );
	};

	// Integrate an SPD against a CIE observer. `normalize` scales by
	// 1 / sum(ybar) so a flat unit spectrum yields Y = 1.
	IMGUI_API void SpectrumToXYZ( ImSpectrumData const& spectrum, ImWidgetsObserver observer,
								  float& out_X, float& out_Y, float& out_Z, bool normalize = true );

	// Colour of a monochromatic stimulus at `lambda`, normalised to full
	// brightness and faded where the observer response dies out. Used for the
	// spectrum ruler; also useful on its own.
	IMGUI_API void WavelengthToRGB( float lambda, ImWidgetsObserver observer,
									float& out_r, float& out_g, float& out_b );

	// Editable SPD canvas. Returns true the frame the curve was edited.
	IMGUI_API bool SpectrumEditor( char const* label, ImSpectrumData& spectrum,
								   ImVec2 size = ImVec2( 0, 0 ), float y_max = 1.0f,
								   ImWidgetsObserver observer = ImWidgetsObserver_CIE1931_2deg );

	//////////////////////////////////////////////////////////////////////////
	// Colour space introspection: primaries, per-space conversion and gamut
	// testing, so a caller can answer "does this colour fit in X?" for any of
	// the ImWidgetsColorSpace entries rather than assuming sRGB.
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API char const* ColorSpaceName( ImWidgetsColorSpace space );

	// xy chromaticities of the space's R, G and B primaries.
	IMGUI_API void ColorSpaceGetPrimaries( ImWidgetsColorSpace space,
										   ImVec2& out_r, ImVec2& out_g, ImVec2& out_b );

	// XYZ -> LINEAR RGB in the given space (no transfer function applied).
	IMGUI_API void ColorConvertXYZtoRGBSpace( ImWidgetsColorSpace space,
											  float& out_r, float& out_g, float& out_b,
											  float X, float Y, float Z );

	// Is the chromaticity (x, y) inside the space's primary triangle? This is a
	// pure CHROMATICITY test -- a colour can pass it and still clip on
	// luminance, so check the converted RGB against [0,1] as well.
	IMGUI_API bool ChromaticityInsideGamut( ImWidgetsColorSpace space, float x, float y );

	// Overlay gamut triangles on a chromaticity plot, one per entry in `spaces`,
	// drawn GREEN when (test_x, test_y) falls inside that space and RED when it
	// does not. The min/max args must match the plot the overlay sits on.
	IMGUI_API void DrawChromaticityGamuts( ImDrawList* pDrawList, ImVec2 pos, ImVec2 size,
										   ImWidgetsColorSpace const* spaces, int space_count,
										   float test_x, float test_y,
										   float minX = 0.0f, float maxX = 0.8f,
										   float minY = 0.0f, float maxY = 0.9f,
										   bool show_labels = true,
										   float thickness = 1.5f );

	//////////////////////////////////////////////////////////////////////////
	// Scalar -> Colour
	//
	// One uniform "float -> colour" mapping, exposed once per colour space Dear
	// Widgets knows about. Every one of them has the SAME definition, namely
	// ImWidgetsColor1DCallback:
	//
	//     ImU32 ( * )( float t, void* pUserData )
	//
	// so any of them drops straight into DrawProceduralColor1D*, DrawColorRing,
	// DrawProceduralColorArcBilinear, DrawShapeProceduralColor*,
	// ShapeFillProceduralColor1D, ... with no adapter. Those helpers also have
	// colour-space-aware overloads at the bottom of this block, which take the
	// options directly and remap the space enum to the matching entry point
	// themselves.
	//
	// `pUserData` is an optional `ImWidgetsColorScalar const*` saying WHAT the
	// scalar means -- a hue, a lightness, a chroma, a colour temperature, a
	// wavelength -- what the other two components are pinned to, how t is
	// shaped, and which colour harmony (complement / triad / tetrad / ...) is
	// applied on top. NULL uses the defaults, which is a plain hue sweep at
	// full saturation, i.e. the classic hue bar.
	//
	// Every enum in the options is resolved to a FUNCTION POINTER once, up
	// front, by ColorScalarResolve -- never re-dispatched per sample. See
	// ImWidgetsColorScalarResolved below.
	//////////////////////////////////////////////////////////////////////////

	// Pass to any of the colour-space-aware overloads to keep whatever
	// ImWidgetsColorScalar::Space already holds instead of overriding it.
	enum { ImWidgetsColorSpace_FromOptions = -1 };

	// Where the scalar comes from.
	enum ImWidgetsColorSource_
	{
		ImWidgetsColorSource_Model = 0,		// drives a component of Model (default)
		ImWidgetsColorSource_Kelvin,		// colour temperature along the Planckian locus
		ImWidgetsColorSource_Wavelength,	// monochromatic stimulus, LambdaMin..LambdaMax
		ImWidgetsColorSource_Gradient,		// samples the ImGradientData in Gradient
		ImWidgetsColorSource_COUNT
	};
	typedef int ImWidgetsColorSource;

	// How a triple is parameterised. ORTHOGONAL to ImWidgetsColorSpace_, which
	// supplies the primaries and the transfer function ("what RGB means").
	//   RGB / LinearRGB / HSV / HSL / HSY / HSP are built ON the working space.
	//   OkLab / OkLCH / CIELab / CIELCh / XYZ / xyY are CIE-referred and absolute;
	//   for those the working space only decides the gamut test.
	enum ImWidgetsColorModel_
	{
		ImWidgetsColorModel_RGB = 0,	// (R, G, B)		encoded, [0,1]
		ImWidgetsColorModel_LinearRGB,	// (R, G, B)		linear,  [0,1]
		ImWidgetsColorModel_HSV,		// (H, S, V)		all [0,1]
		ImWidgetsColorModel_HSL,		// (H, S, L)		all [0,1]
		ImWidgetsColorModel_HSY,		// (H, S, Y)		BT.709 luma
		ImWidgetsColorModel_HSP,		// (H, S, P)		perceived brightness
		ImWidgetsColorModel_OkLab,		// (L, a, b)		L [0,1], a/b ~ [-0.4,0.4]
		ImWidgetsColorModel_OkLCH,		// (L, C, H)		C ~ [0,0.37], H [0,1]
		ImWidgetsColorModel_CIELab,		// (L*, a*, b*)		L* [0,100], a*/b* ~ [-128,127]
		ImWidgetsColorModel_CIELCh,		// (L*, C*, h)		C* [0,150], h [0,1]
		ImWidgetsColorModel_XYZ,		// (X, Y, Z)
		ImWidgetsColorModel_xyY,		// (x, y, Y)
		ImWidgetsColorModel_COUNT
	};
	typedef int ImWidgetsColorModel;

	// Semantic component, resolved per model by ColorModelAxisIndex(). A model
	// with no such component (Hue in plain RGB, say) still honours the axis --
	// it is then applied as a post-step in the HSV cylinder instead of being
	// silently dropped. Comp0/1/2 always mean the raw 1st/2nd/3rd component.
	enum ImWidgetsColorAxis_
	{
		ImWidgetsColorAxis_None = -1,
		ImWidgetsColorAxis_Hue = 0,		// H in HS*, H in OkLCH / CIELCh
		ImWidgetsColorAxis_Chroma,		// S in HS*, C in OkLCH / CIELCh
		ImWidgetsColorAxis_Lightness,	// V/L/Y/P, L in Ok*, L* in CIE*, Y in XYZ/xyY
		ImWidgetsColorAxis_Comp0,
		ImWidgetsColorAxis_Comp1,
		ImWidgetsColorAxis_Comp2,
		ImWidgetsColorAxis_COUNT
	};
	typedef int ImWidgetsColorAxis;

	// Classical hue-wheel schemes. The scalar produces ONE colour; the harmony
	// rotates it, and HarmonyIndex picks which member of the scheme comes out --
	// so a single ramp can drive a whole palette by being evaluated N times.
	enum ImWidgetsColorHarmony_
	{
		ImWidgetsColorHarmony_None = 0,			// 1 : the colour itself
		ImWidgetsColorHarmony_Complement,		// 2 : H, H+180
		ImWidgetsColorHarmony_SplitComplement,	// 3 : H, H+180-s, H+180+s
		ImWidgetsColorHarmony_Triad,			// 3 : H, H+120, H+240
		ImWidgetsColorHarmony_Tetrad,			// 4 : H, H+60, H+180, H+240
		ImWidgetsColorHarmony_Square,			// 4 : H, H+90, H+180, H+270
		ImWidgetsColorHarmony_Analogous,		// 5 : H + k*spread, k = -2..+2
		ImWidgetsColorHarmony_Monochromatic,	// 5 : same H, chroma fanning out
		ImWidgetsColorHarmony_COUNT
	};
	typedef int ImWidgetsColorHarmony;

	// What happens to t outside [0,1] once the domain remap and Repeats are in.
	enum ImWidgetsColorWrap_
	{
		ImWidgetsColorWrap_Clamp = 0,
		ImWidgetsColorWrap_Repeat,
		ImWidgetsColorWrap_Mirror,
		ImWidgetsColorWrap_COUNT
	};
	typedef int ImWidgetsColorWrap;

	// Shaping applied to t after the domain remap. EaseParam is the gamma /
	// exponent / steepness; Linear and Smoothstep ignore it.
	enum ImWidgetsColorEase_
	{
		ImWidgetsColorEase_Linear = 0,
		ImWidgetsColorEase_Smoothstep,
		ImWidgetsColorEase_Gamma,
		ImWidgetsColorEase_Exponential,
		ImWidgetsColorEase_Logarithmic,
		ImWidgetsColorEase_Sine,
		ImWidgetsColorEase_COUNT
	};
	typedef int ImWidgetsColorEase;

	// What to do when the colour falls outside the working space gamut.
	enum ImWidgetsColorGamut_
	{
		ImWidgetsColorGamut_Clip = 0,	// saturate each channel (the usual)
		ImWidgetsColorGamut_Desaturate,	// blend toward its own luminance until it fits
		ImWidgetsColorGamut_Mark,		// replace with OutOfGamutColor (debug / QC)
		ImWidgetsColorGamut_Keep,		// leave it; ColorFromScalar returns it unclamped
		ImWidgetsColorGamut_COUNT
	};
	typedef int ImWidgetsColorGamut;

	// Ready-made mappings for ColorScalarPreset(). These are the named things
	// people actually reach for; anything else is reachable by hand.
	enum ImWidgetsColorRamp_
	{
		ImWidgetsColorRamp_Hue = 0,			// t -> hue at full chroma      ("hue to colour")
		ImWidgetsColorRamp_Lightness,		// t -> lightness, hue/chroma held
		ImWidgetsColorRamp_Chroma,			// t -> chroma, hue/lightness held
		ImWidgetsColorRamp_LightnessToHue,	// t -> lightness AND hue       ("luminance to hue")
		ImWidgetsColorRamp_Grey,			// t -> neutral ramp
		ImWidgetsColorRamp_Red,				// t -> the working space R channel
		ImWidgetsColorRamp_Green,
		ImWidgetsColorRamp_Blue,
		ImWidgetsColorRamp_Kelvin,			// t -> 1000 K .. 12000 K
		ImWidgetsColorRamp_Wavelength,		// t -> 380 nm .. 730 nm
		ImWidgetsColorRamp_COUNT
	};
	typedef int ImWidgetsColorRamp;

	//////////////////////////////////////////////////////////////////////////
	// Enum -> function pointer
	//
	// Colour conversion sits on the hot path: a 128-division bar fires the
	// callback 258 times, and each evaluation would otherwise re-enter a switch
	// for the model, again for the harmony model, and once per channel for the
	// transfer function. All of it is resolved to pointers up front instead --
	// same shape as the existing ImGradientInterpGetFunctions.
	//
	// The RGB triple these map to and from is:
	//   working-space models (RGB..HSP)  -> ENCODED RGB in the working space
	//   CIE-referred models (OkLab..xyY) -> display sRGB
	// ColorModelIsWorkingSpace() tells you which.
	//////////////////////////////////////////////////////////////////////////
	typedef void  ( *pfColorModelToRGB   )( float& out_r,  float& out_g,  float& out_b,  float c0, float c1, float c2 );
	typedef void  ( *pfColorModelFromRGB )( float& out_c0, float& out_c1, float& out_c2, float r,  float g,  float b  );
	// Transfer function of a working space. `gamma` is the value ColorSpaceGetTransfer
	// hands back alongside; the sRGB piecewise curve and the identity ignore it.
	typedef float ( *pfColorTransfer     )( float value, float gamma );

	// The remaining pipeline stages, one pointer per enum, so that NOTHING in the
	// per-sample path is a switch. Wrap and Ease shape t; Source produces the
	// colour; Harmony and Gamut post-process it.
	struct ImWidgetsColorScalarResolved;	// defined below -- the stages take it
	typedef float ( *pfColorScalarWrap   )( float u );
	typedef float ( *pfColorScalarEase   )( float u, float param );
	// Sources report their own alpha (a gradient carries one); the evaluator
	// multiplies it by ImWidgetsColorScalar::Alpha.
	typedef void  ( *pfColorScalarSource )( float u, ImWidgetsColorScalarResolved const& resolved,
											float& out_r, float& out_g, float& out_b, float& out_a );
	typedef void  ( *pfColorScalarStage  )( ImWidgetsColorScalarResolved const& resolved,
											float& r, float& g, float& b );
	// Applies a semantic axis the model could not express, in the HSV cylinder.
	// NULL when the axis IS native to the model (the usual case).
	typedef void  ( *pfColorScalarAxisApply )( float value, float& r, float& g, float& b );

	IMGUI_API bool	ColorModelIsWorkingSpace( ImWidgetsColorModel model );

	// Resolve a model to its conversion pair; either out param may be NULL.
	// BOTH come back NULL for ImWidgetsColorModel_LinearRGB, which has no fixed
	// conversion -- its transfer depends on the working space, so go through
	// ColorSpaceGetTransfer for that one.
	IMGUI_API void	ColorModelGetFunctions( ImWidgetsColorModel model,
											pfColorModelToRGB* out_to_rgb,
											pfColorModelFromRGB* out_from_rgb );

	// Resolve a working space to its transfer pair plus the gamma to feed them.
	// Any out param may be NULL. A space with gamma 1.0 (ECI) resolves to the
	// identity, so no ImPow runs at all.
	IMGUI_API void	ColorSpaceGetTransfer( ImWidgetsColorSpace space,
										   pfColorTransfer* out_encode,
										   pfColorTransfer* out_decode,
										   float* out_gamma );

	// The options block behind every scalar -> colour function. Default-
	// constructed it is a full hue sweep at full saturation in sRGB.
	struct ImWidgetsColorScalar
	{
		// --- what the scalar means -----------------------------------------
		ImWidgetsColorSource	Source;
		ImWidgetsColorModel		Model;
		ImWidgetsColorSpace		Space;
		ImWidgetsColorAxis		Axis;		// component the scalar drives
		float					Fixed[ 3 ];	// the other components, index-aligned to Model
		float					RangeMin;	// Axis value at t = 0
		float					RangeMax;	// Axis value at t = 1  (swap them to reverse)

		// A SECOND component driven by the same scalar. This is what makes
		// "luminance to hue" a single mapping rather than two: Axis = Lightness,
		// Axis2 = Hue. ImWidgetsColorAxis_None disables it.
		ImWidgetsColorAxis		Axis2;
		float					Range2Min;
		float					Range2Max;

		// --- shaping of t ---------------------------------------------------
		float					DomainMin;	// t is normalised from [DomainMin, DomainMax]
		float					DomainMax;	// before anything else happens
		float					Repeats;	// cycles across the domain (1 = a single pass)
		ImWidgetsColorWrap		Wrap;
		ImWidgetsColorEase		Ease;
		float					EaseParam;

		// --- harmony, applied to the colour produced above -------------------
		ImWidgetsColorHarmony	Harmony;
		int						HarmonyIndex;	// which member comes out (0 = the base)
		float					HarmonySpread;	// degrees; split-complement / analogous
		ImWidgetsColorModel		HarmonyModel;	// cylinder the rotation happens in

		// --- source-specific --------------------------------------------------
		float					KelvinMin, KelvinMax;
		float					LambdaMin, LambdaMax;
		ImWidgetsObserver		Observer;
		ImGradientData const*	Gradient;

		// --- output -----------------------------------------------------------
		ImWidgetsColorGamut		Gamut;
		ImU32					OutOfGamutColor;
		float					Alpha;
		// Bradford-adapt the working space white point to D65 before display.
		// Without it a D50 space (ProPhoto, ECI, Best, ...) renders its own white
		// as a warm cast, which is a conversion error rather than a preview.
		bool					AdaptWhitePoint;

		ImWidgetsColorScalar()
			: Source( ImWidgetsColorSource_Model )
			, Model( ImWidgetsColorModel_HSV )
			, Space( ImWidgetsColorSpace_sRGB )
			, Axis( ImWidgetsColorAxis_Hue )
			, RangeMin( 0.0f ), RangeMax( 1.0f )
			, Axis2( ImWidgetsColorAxis_None )
			, Range2Min( 0.0f ), Range2Max( 1.0f )
			, DomainMin( 0.0f ), DomainMax( 1.0f )
			, Repeats( 1.0f )
			, Wrap( ImWidgetsColorWrap_Clamp )
			, Ease( ImWidgetsColorEase_Linear )
			, EaseParam( 1.0f )
			, Harmony( ImWidgetsColorHarmony_None )
			, HarmonyIndex( 0 )
			, HarmonySpread( 30.0f )
			, HarmonyModel( ImWidgetsColorModel_HSV )
			, KelvinMin( 1000.0f ), KelvinMax( 12000.0f )
			, LambdaMin( 380.0f ), LambdaMax( 730.0f )
			, Observer( ImWidgetsObserver_CIE1931_2deg )
			, Gradient( NULL )
			, Gamut( ImWidgetsColorGamut_Clip )
			, OutOfGamutColor( IM_COL32( 255, 0, 255, 255 ) )
			, Alpha( 1.0f )
			, AdaptWhitePoint( true )
		{
			Fixed[ 0 ] = 0.0f; Fixed[ 1 ] = 1.0f; Fixed[ 2 ] = 1.0f;	// HSV: full sat, full value
		}
	};

	// An ImWidgetsColorScalar with every enum already turned into a pointer or an
	// index. Build one per draw with ColorScalarResolve, then evaluate with
	// ColorFromScalarResolved / ColorFromScalarResolvedCallback. The plain
	// ColorFromScalar is exactly "resolve, then evaluate once".
	//
	// It holds Options BY VALUE, so the copy handed through the callback void*
	// stays alive for the whole draw -- but ImGradientData is still referenced by
	// pointer, so a Gradient source must outlive the draw.
	struct ImWidgetsColorScalarResolved
	{
		ImWidgetsColorScalar	Options;

		// model / working space
		pfColorModelToRGB		ModelToRGB;			// NULL == LinearRGB, use Encode
		pfColorModelFromRGB		ModelFromRGB;		// NULL == LinearRGB, use Decode
		bool					ModelIsWorkingSpace;
		pfColorTransfer			Encode;
		pfColorTransfer			Decode;
		float					Gamma;

		// axes, already mapped to component indices (-1 = not native to the model,
		// in which case the matching *Apply pointer below is non-NULL and does it
		// in the HSV cylinder instead)
		int						AxisIndex;
		int						Axis2Index;
		pfColorScalarAxisApply	AxisApply;
		pfColorScalarAxisApply	Axis2Apply;
		// Hue is periodic: the driven value is wrapped into [0,1) before it
		// reaches the conversion, so a range like [-0.2, 0.8] (a hue sweep with
		// an offset) is legal rather than feeding a negative hue to HSVtoRGB.
		bool					AxisWraps;
		bool					Axis2Wraps;

		// harmony
		pfColorModelToRGB		HarmonyToRGB;
		pfColorModelFromRGB		HarmonyFromRGB;
		bool					HarmonyIsWorkingSpace;
		int						HarmonyHueIndex;
		int						HarmonyChromaIndex;
		int						HarmonyCount;
		float					HarmonyOffsets[ 5 ];

		// pipeline stages -- evaluating a sample is exactly these five calls,
		// with no enum left to test
		pfColorScalarWrap		Wrap;
		pfColorScalarEase		Ease;
		pfColorScalarSource		Source;
		pfColorScalarStage		Harmony;	// a no-op when Harmony_None
		pfColorScalarStage		Gamut;

		ImWidgetsColorScalarResolved()
			: ModelToRGB( NULL ), ModelFromRGB( NULL ), ModelIsWorkingSpace( true )
			, Encode( NULL ), Decode( NULL ), Gamma( 2.2f )
			, AxisIndex( -1 ), Axis2Index( -1 )
			, AxisApply( NULL ), Axis2Apply( NULL )
			, AxisWraps( false ), Axis2Wraps( false )
			, HarmonyToRGB( NULL ), HarmonyFromRGB( NULL ), HarmonyIsWorkingSpace( true )
			, HarmonyHueIndex( 0 ), HarmonyChromaIndex( 1 ), HarmonyCount( 1 )
			, Wrap( NULL ), Ease( NULL ), Source( NULL ), Harmony( NULL ), Gamut( NULL )
		{
			HarmonyOffsets[ 0 ] = HarmonyOffsets[ 1 ] = HarmonyOffsets[ 2 ] =
			HarmonyOffsets[ 3 ] = HarmonyOffsets[ 4 ] = 0.0f;
		}
	};

	// ---- Introspection -------------------------------------------------------
	IMGUI_API char const*	ColorModelName    ( ImWidgetsColorModel model );
	IMGUI_API char const*	ColorModelCompName( ImWidgetsColorModel model, int comp );
	IMGUI_API char const*	ColorAxisName     ( ImWidgetsColorAxis axis );
	IMGUI_API char const*	ColorSourceName   ( ImWidgetsColorSource source );
	IMGUI_API char const*	ColorHarmonyName  ( ImWidgetsColorHarmony harmony );
	IMGUI_API char const*	ColorWrapName     ( ImWidgetsColorWrap wrap );
	IMGUI_API char const*	ColorEaseName     ( ImWidgetsColorEase ease );
	IMGUI_API char const*	ColorGamutName    ( ImWidgetsColorGamut gamut );
	IMGUI_API char const*	ColorRampName     ( ImWidgetsColorRamp ramp );

	// Component index (0..2) the semantic axis maps to in `model`, or -1 when the
	// model has no such component.
	IMGUI_API int	ColorModelAxisIndex( ImWidgetsColorModel model, ImWidgetsColorAxis axis );
	// Natural range of component `comp` of `model` (H in [0,1], L* in [0,100], ...).
	IMGUI_API void	ColorModelCompRange( ImWidgetsColorModel model, int comp, float* out_min, float* out_max );
	// A vivid, in-gamut starting triple in `model` -- what the presets pin the
	// components the scalar is NOT driving to.
	IMGUI_API void	ColorModelDefaultTriple( ImWidgetsColorModel model, float out_c[ 3 ] );
	// Hue offsets, in DEGREES, of every member of a harmony scheme. Returns the
	// member count (<= cap). Monochromatic reports its count with all offsets 0 --
	// it fans chroma out instead, which ColorFromScalar applies.
	IMGUI_API int	ColorHarmonyOffsets( ImWidgetsColorHarmony harmony, float spread_deg, float* out_deg, int cap );

	// ---- Conversion ----------------------------------------------------------
	// Transfer function of a working space. sRGB uses its piecewise curve; every
	// other space uses the pure gamma from its table entry. Both are odd-symmetric
	// so out-of-gamut negatives survive a round trip instead of turning into NaN.
	// These are the one-shot forms; resolve the pointer with ColorSpaceGetTransfer
	// when converting more than a couple of values.
	IMGUI_API float	ColorSpaceEncode( ImWidgetsColorSpace space, float linear );
	IMGUI_API float	ColorSpaceDecode( ImWidgetsColorSpace space, float encoded );
	// LINEAR RGB in `space` -> XYZ, at the space own white point. The missing
	// counterpart of ColorConvertXYZtoRGBSpace.
	IMGUI_API void	ColorConvertRGBSpacetoXYZ( ImWidgetsColorSpace space,
											   float& out_X, float& out_Y, float& out_Z,
											   float r, float g, float b );
	// Bradford chromatic adaptation between the space white point and D65.
	IMGUI_API void	ColorAdaptSpaceToD65( ImWidgetsColorSpace space, float& X, float& Y, float& Z );
	IMGUI_API void	ColorAdaptD65ToSpace( ImWidgetsColorSpace space, float& X, float& Y, float& Z );
	// A triple in `model` -> DISPLAY sRGB, i.e. what ImGui wants to be handed.
	IMGUI_API void	ColorModelToDisplay( ImWidgetsColorModel model, ImWidgetsColorSpace space,
										 float c0, float c1, float c2,
										 float& out_r, float& out_g, float& out_b,
										 bool adapt_white_point = true );
	// Display sRGB -> a triple in `model`. Round-trips ColorModelToDisplay.
	IMGUI_API void	ColorDisplayToModel( ImWidgetsColorModel model, ImWidgetsColorSpace space,
										 float r, float g, float b,
										 float& out_c0, float& out_c1, float& out_c2,
										 bool adapt_white_point = true );
	// Does this display-sRGB colour fit inside `space`? Unlike
	// ChromaticityInsideGamut this is the FULL test -- chromaticity AND level.
	IMGUI_API bool	ColorInsideSpaceGamut( ImWidgetsColorSpace space, float r, float g, float b,
										   bool adapt_white_point = true );

	// ---- Evaluation ----------------------------------------------------------
	// Turn every enum in `opt` into a pointer / index, once. `space` overrides
	// opt.Space unless it is ImWidgetsColorSpace_FromOptions.
	IMGUI_API void		ColorScalarResolve( ImWidgetsColorScalarResolved* out_resolved,
											ImWidgetsColorScalar const& opt,
											ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
	IMGUI_API ImVec4	ColorFromScalarResolved( float t, ImWidgetsColorScalarResolved const& resolved );
	// ImWidgetsColor1DCallback. pUserData = ImWidgetsColorScalarResolved const*.
	IMGUI_API ImU32		ColorFromScalarResolvedCallback( float t, void* pUserData );

	// Convenience one-shots: these resolve internally, so prefer the resolved
	// form when you are about to evaluate the same mapping many times.
	IMGUI_API ImVec4	ColorFromScalar   ( float t, ImWidgetsColorScalar const& opt );
	IMGUI_API ImU32		ColorFromScalarU32( float t, ImWidgetsColorScalar const& opt );
	// ImWidgetsColor1DCallback. pUserData = ImWidgetsColorScalar const*, NULL = defaults.
	IMGUI_API ImU32		ColorFromScalarCallback( float t, void* pUserData );
	// Same mapping, but the working space is given rather than read from `opt`.
	IMGUI_API ImU32		ColorScalarInSpace( ImWidgetsColorSpace space, float t, ImWidgetsColorScalar const* opt );

	// Fill `opt` with a ready-made mapping, keeping its Space (and its Model when
	// the ramp can express itself in it -- Grey and the R/G/B ramps need a model
	// that has the component they drive, so those may switch Model).
	IMGUI_API void		ColorScalarPreset( ImWidgetsColorScalar* opt, ImWidgetsColorRamp ramp );

	// ---- One function per colour space, all with the SAME definition ---------
	// Identical apart from the working space they pin. pUserData is an optional
	// ImWidgetsColorScalar const*: its Space field is overridden by whichever
	// function you picked, every other field is honoured. Pass NULL for a plain
	// hue sweep rendered through that space.
	IMGUI_API ImU32	ColorScalarAdobeRGB    ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarAppleRGB    ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarBest        ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarBeta        ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarBruce       ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarCIERGB      ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarColorMatch  ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarDonRGB4     ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarECI         ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarEktaSpacePS5( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarNTSC        ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarPALSECAM    ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarProPhoto    ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarSMPTEC      ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarsRGB        ( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarWideGamutRGB( float t, void* pUserData );
	IMGUI_API ImU32	ColorScalarRec2020     ( float t, void* pUserData );
	// The same 17, chosen at runtime.
	IMGUI_API ImWidgetsColor1DCallback	ColorScalarCallback( ImWidgetsColorSpace space );

	//////////////////////////////////////////////////////////////////////////
	// Colour-space-aware overloads of the 1D procedural helpers
	//
	// The same helpers declared earlier in this header, but taking the options
	// block instead of a raw callback + void*, and choosing the working space
	// HERE rather than at the call site: the space enum is remapped to the
	// matching entry point internally (ColorScalarResolve / ColorScalarCallback),
	// resolved once for the whole draw rather than per sample.
	//
	// `space` defaults to ImWidgetsColorSpace_FromOptions, which keeps opt.Space.
	//////////////////////////////////////////////////////////////////////////
	void	ShapeFillProceduralColor1D( ImWidgetsShape& shape, ImWidgetsColorScalar const& opt, bool sample_v,
										ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );

	void	DrawShapeProceduralColorVerticalBand  ( ImDrawList* pDrawList, ImRect const& bb, int divisions,
	                                                ImWidgetsColorScalar const& opt,
	                                                ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
	void	DrawShapeProceduralColorHorizontalBand( ImDrawList* pDrawList, ImRect const& bb, int divisions,
	                                                ImWidgetsColorScalar const& opt,
	                                                ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
	void	DrawShapeProceduralColorAnnulus       ( ImDrawList* pDrawList, ImVec2 center, float innerRadius, float outerRadius,
	                                                int numSectors, ImWidgetsColorScalar const& opt,
	                                                ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );

	IMGUI_API void	DrawProceduralColor1DNearestHorizontal( ImDrawList* pDrawList, ImWidgetsColorScalar const& opt,
															float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX,
															ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
	IMGUI_API void	DrawProceduralColor1DNearestVertical  ( ImDrawList* pDrawList, ImWidgetsColorScalar const& opt,
															float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionY,
															ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
	IMGUI_API void	DrawProceduralColor1DBilinearHorizontal( ImDrawList* pDrawList, ImWidgetsColorScalar const& opt,
															 float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX,
															 ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
	IMGUI_API void	DrawProceduralColor1DBilinearVertical  ( ImDrawList* pDrawList, ImWidgetsColorScalar const& opt,
															 float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionY,
															 ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );

	IMGUI_API void	DrawProceduralColorArcBilinear( ImDrawList* pDrawList, ImVec2 center, float innerRadius, float outerRadius,
													float startAngle, float sweepAngle, ImWidgetsColorScalar const& opt,
													int division, bool bilinear,
													ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
	IMGUI_API void	DrawProceduralColorSplineBilinear( ImDrawList* pDrawList, ImVec2 const* points, int points_count,
													   float thickness, ImWidgetsColorScalar const& opt,
													   int resolution, bool closed,
													   ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );

	IMGUI_API void	DrawColorRing( ImDrawList* pDrawList, ImVec2 const curPos, ImVec2 const size, float thickness_,
								   ImWidgetsColorScalar const& opt, int division, float colorOffset, bool bIsBilinear,
								   ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );

	// Color Picker
	IMGUI_API bool ColorPickerSRGB( char const* label, ImVec4* color, int fixedAxis = 2, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerHSV( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerOkLab( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerOkLCH( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerCIELab( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerXYZ( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPicker( char const* label, ImVec4* color, ImColorPickerSpace space = ImColorPickerSpace_sRGB, int fixedAxis = 2, ImVec2 size = ImVec2( 0, 0 ) );

	// Physically-based skin color picker (forward-only biophysical model).
	// Drives *color from melanin / eu-pheo blend / hemoglobin parameters; the
	// input color is not read back (the param->color mapping is not invertible).
	IMGUI_API bool ColorPickerSkin( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// Physically-based hair color picker (forward-only). Drives *color from
	// melanin amount / pheomelanin redness / azimuthal roughness using the
	// Chiang et al. 2016 melanin absorption model (PBRT/Blender/Unreal). The
	// input color is not read back (the param->color mapping is not invertible).
	IMGUI_API bool ColorPickerHair( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// Physically-based leaf / vegetation color picker (forward-only). Drives *color
	// from chlorophyll / carotenoid / anthocyanin (+brown) pigment concentrations
	// using the PROSPECT-D leaf optical-properties model (Feret et al. 2017). The
	// input color is not read back (the param->color mapping is not invertible).
	IMGUI_API bool ColorPickerLeaf( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// Physically-based forward color pickers (all forward-only: they drive *color
	// from physical parameters and do not read it back). References are in the
	// implementation comments (dear_widgets.cpp).
	//   Blackbody : Planck's law along the Planckian locus (color temperature).
	//   Pigment   : subtractive paint mixing via Kubelka-Munk theory.
	//   Gem       : crystal-field (Beer-Lambert) gemstone body color.
	//   Water     : bio-optical ocean/water color (chlorophyll/CDOM/turbidity).
	//   Iris      : eye color from melanin absorption + Tyndall scattering.
	//   Flame     : emission spectrum (blackbody continuum + atomic lines).
	IMGUI_API bool ColorPickerBlackbody( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerPigment( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerGem( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerWater( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerIris( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerFlame( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// More forward-only physical pickers (references in dear_widgets.cpp).
	//   Bruise     : healing hematoma chromophores (hemoglobin->biliverdin->bilirubin).
	//   Nebula     : ionized-gas emission lines (Halpha/[OIII]/[NII]/[SII]).
	//   Maillard   : food browning (Maillard/caramel, Arrhenius time-temperature).
	//   Patina     : copper atmospheric weathering (metal->cuprite->green patina).
	//   Subsurface : translucent material color (Jensen 2001 dipole diffusion).
	IMGUI_API bool ColorPickerBruise( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerNebula( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerMaillard( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerPatina( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerSubsurface( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// More forward-only physical pickers (references in dear_widgets.cpp).
	//   Discharge : gas-discharge / neon-tube emission (noble gas + Hg + phosphor).
	//   Ice       : glacier/sea-ice color (pure-ice absorption + grain scattering).
	//   Ochre     : earth-pigment (iron-oxide) Kubelka-Munk mixing.
	IMGUI_API bool ColorPickerDischarge( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerIce( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerOchre( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	// Sky: Bruneton single-scatter atmosphere (Rayleigh + Mie + Ozone), baked
	// transmittance LUT. Plane = elevation Ã— time-of-day; sliders = view-az
	// (sun-rel), day-of-year, observer latitude.
	IMGUI_API bool ColorPickerSky( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// Stellar photosphere colour (Planck blackbody + line blanketing + TiO bands).
	// Plane = log Teff Ã— log g (dwarfâ†’supergiant); slider = metallicity [Fe/H].
	// Reference: Mamajek 2022 dwarf colour-temperature sequence.
	IMGUI_API bool ColorPickerStar( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// Vascular tissue colour (Beer-Lambert oxy/deoxy haemoglobin + melanin layer).
	// Plane = SpO2 Ã— dermal blood-volume fraction; slider = melanin density.
	// Reference: Prahl haemoglobin extinction coefficient tables (OMLC).
	IMGUI_API bool ColorPickerHemoglobin( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// Volumetric cloud lighting (Schneider & Vos 2015 Beer-Powder, dual HG).
	// Plane = optical depth Ã— cos(view, sun); slider = extinction coefficient.
	// Reference: "Real-Time Volumetric Cloudscapes of Horizon Zero Dawn" SIGGRAPH 2015.
	IMGUI_API bool ColorPickerCloud( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// Streetlight gas-discharge spectrum â€” mercury vapour + sodium D-line (LPS/HPS)
	// + tri-phosphor fluorescent overlay. Plane = Hgâ†”Na mix Ã— pressure; slider = phosphor coating.
	IMGUI_API bool ColorPickerStreetlight( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );

	// === Artist-oriented colour pickers (for asset creation) =================

	// Harmony palette â€” pick anchor + scheme (complement/split/triad/tetrad/square/analogous).
	// `color` = currently-active swatch. `out_palette` (up to 5 entries) and `out_count`
	// expose the full palette to the caller.
	IMGUI_API bool ColorPickerPaletteHarmony( char const* label, ImVec4* color,
	                                          ImVec4* out_palette = nullptr, int* out_count = nullptr );

	// Trichromatic mixer — barycentric mix of 3 artist primaries (defaults to Y/M/C).
	// Subtractive (absorbance-space) mixing. The vertical slider is either a bipolar
	// white<->black tint (enable_k = false, i.e. plain "CMY") or, when enable_k =
	// true, a unipolar 0..1 "K" (black ink) channel applying the standard CMYK
	// composition channel *= (1-K) -- a real 4th key/black ink, not a white/black
	// tint. Same function either way; only the slider's range/behaviour changes.
	IMGUI_API bool ColorPickerTrichromaticMixer( char const* label, ImVec4* color, bool enable_k = false );

	// Weathered metal â€” base metal (Steel/Iron/Copper/Brass/Aluminum/Gold) + patina
	// coverage + roughness + grime. Outputs sRGB albedo; F0 follows the metal preset.
	IMGUI_API bool ColorPickerWeatheredMetal( char const* label, ImVec4* color );

	// Fabric dye â€” substrate (linen / cotton / wool / silk) tinted by a dye colour
	// via Beer-Lambert. Plane = dye hue Ã— saturation; slider = dye intensity.
	IMGUI_API bool ColorPickerFabricDye( char const* label, ImVec4* color );

	// Mood-palette picker â€” curated 5-swatch palettes by mood word (warm/cool/melancholy/
	// fresh/vintage/pastel/neon/earth/sunset/ocean). Plane = palette position Ã— lightness shift;
	// slider = saturation crush.
	IMGUI_API bool ColorPickerMoodPalette( char const* label, ImVec4* color, ImVec4 out_palette[ 5 ] = nullptr );

	// Toon ramp â€” pick a midtone, get back a 3-stop shadow/mid/highlight ramp with
	// warm-cool hue shift. `color` = midtone; `out_ramp[3]` = full ramp.
	IMGUI_API bool ColorPickerToonRamp( char const* label, ImVec4* color, ImVec4 out_ramp[ 3 ] = nullptr );

	// Adobe-style harmony wheel: circular disc (hue around angle, saturation along radius)
	// with multiple handles arranged by the chosen harmony scheme. Combos let the user
	// pick the colour-space cylinder (HSV/HSL/HSY/HSP/OkLCH) and the scheme (mono/analogous/
	// complement/split-complement/triad/tetrad/square/compound). `color` = currently
	// active swatch; optional `out_palette` (up to 5 entries) + `out_count` expose all.
	IMGUI_API bool ColorPickerHarmonyWheel( char const* label, ImVec4* color,
	                                        ImVec4* out_palette = nullptr, int* out_count = nullptr );

	// Transform Gizmo
	IMGUI_API bool TransformGizmo( char const* label, ImTransformData* transforms, ImVec2* sizes, int count, int* selectedIndex, ImTransformGizmoCallbacks const* callbacks = nullptr, ImTransformGizmoFlags flags = ImTransformGizmoFlags_None, ImVec2 canvasSize = ImVec2( 0, 0 ) );

	// Color Warper
	IMGUI_API void ColorConvertRGBtoHSL( float r, float g, float b, float& out_h, float& out_s, float& out_l );
	IMGUI_API void ColorConvertHSLtoRGB( float h, float s, float l, float& out_r, float& out_g, float& out_b );
	IMGUI_API void ColorConvertRGBtoHSY( float r, float g, float b, float& out_h, float& out_s, float& out_y );
	IMGUI_API void ColorConvertHSYtoRGB( float h, float s, float y, float& out_r, float& out_g, float& out_b );
	IMGUI_API void ColorConvertRGBtoHSP( float r, float g, float b, float& out_h, float& out_s, float& out_p );
	IMGUI_API void ColorConvertHSPtoRGB( float h, float s, float p, float& out_r, float& out_g, float& out_b );
	IMGUI_API void ColorConvertRGBtoHSPLog( float r, float g, float b, float& out_h, float& out_s, float& out_p );
	IMGUI_API void ColorConvertHSPLogtoRGB( float h, float s, float pLog, float& out_r, float& out_g, float& out_b );
	IMGUI_API bool ColorWarper( char const* label, ImColorWarperData* data, ImColorWarperMode mode = ImColorWarperMode_Circular, ImColorWarperSpace space = ImColorWarperSpace_HSV, float thirdAxis = 1.0f, ImColorWarperOverlay const* signalOverlay = NULL, ImColorWarperSignalColor signalColor = ImColorWarperSignalColor_PixelColor, float axisAngle = 0.0f, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API float ColorCurveDefaultValue( ImColorCurveMode mode );
	IMGUI_API void  ColorCurveRange( ImColorCurveMode mode, float* out_min, float* out_max );
	IMGUI_API char const* ColorCurveModeName( ImColorCurveMode mode );
	IMGUI_API float ColorCurveSample( ImColorCurveData const& curve, ImColorCurveMode mode, float x, bool advancedSegments = false );
	IMGUI_API bool  ColorCurve( char const* label, ImColorCurveData* curve, ImColorCurveMode mode, ImHistogramData const* histogramOverlay = NULL, bool advancedSegments = false, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API char const* ParadeModeName( ImParadeMode mode );
	IMGUI_API void  ParadeScope( char const* label, ImParadeScopeData const& data, bool overlay = false, ImParadeScale scale = ImParadeScale_Linear, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API void  VectorScope( char const* label, ImVectorScopeData const& data, bool showSkinToneLine = true, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API char const* HistogramModeName( ImHistogramMode mode );
	IMGUI_API void  Histogram( char const* label, ImHistogramData const& data, ImHistogramLayout layout = ImHistogramLayout_Overlapped, ImParadeScale xScale = ImParadeScale_Linear, ImParadeScale yScale = ImParadeScale_Linear, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API char const* CIEChromaticityGamutName( ImCIEChromaticityGamut gamut );
	IMGUI_API void  CIEChromaticity( char const* label, ImCIEChromaticityData const& data, ImCIEChromaticityGamut gamut = ImCIEChromaticityGamut_sRGB_Rec709, bool showBackground = false, ImCIEChromaticitySignalColor signalColor = ImCIEChromaticitySignalColor_Flat, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API int   ToneCurveChannelCount( ImHistogramMode mode );
	IMGUI_API char const* ToneCurveChannelName( ImHistogramMode mode, int channel );
	IMGUI_API float ToneCurveSample( ImColorCurveData const& curve, float x, bool advancedSegments = false );
	IMGUI_API bool  ToneCurve( char const* label, ImToneCurveData* curve, ImHistogramMode mode, ImHistogramData const* histogramOverlay = NULL, bool advancedSegments = false, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API bool SliderRingScalar( char const* label, ImGuiDataType data_type, void* p_value, void* p_min, void* p_max,
									float v_angle_min = -0.75f * IM_PI, float v_angle_max = 0.75f * IM_PI,
									float v_thickness = 0.0f, char const* format = NULL, ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderRingFloat( char const* label, float* value, float v_min, float v_max,
									float v_angle_min = -0.75f * IM_PI, float v_angle_max = 0.75f * IM_PI,
									float v_thickness = 0.0f, char const* format = "%.3f", ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderRingInt( char const* label, int* value, int v_min, int v_max,
								  float v_angle_min = -0.75f * IM_PI, float v_angle_max = 0.75f * IM_PI,
								  float v_thickness = 0.0f, char const* format = "%d", ImGuiSliderFlags flags = 0 );

	// Spline Slider: slider whose track follows cubic bezier curve(s).
	// control_points: ImVec2 array in normalized [0,1]x[0,1] space mapped to widget rect. NULL = default S-curve.
	// num_points: 4 for single bezier, 7 for two chained segments, 3N+1 for N segments.
	IMGUI_API bool SliderSplineScalar( char const* label, ImGuiDataType data_type, void* p_value, void* p_min, void* p_max,
									   ImVec2 const* control_points = NULL, int num_points = 4, float v_height = 0.0f,
									   float v_thickness = 0.0f, char const* format = NULL, ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineFloat( char const* label, float* value, float v_min, float v_max,
									  ImVec2 const* control_points = NULL, int num_points = 4, float v_height = 0.0f,
									  float v_thickness = 0.0f, char const* format = "%.3f", ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineInt( char const* label, int* value, int v_min, int v_max,
									ImVec2 const* control_points = NULL, int num_points = 4, float v_height = 0.0f,
									float v_thickness = 0.0f, char const* format = "%d", ImGuiSliderFlags flags = 0 );

	// SliderSplineGradient: spline-track slider with a gradient background rendered along the bezier path.
	// `fill_up_to_cursor` (default false) compresses the full gradient into the [0, cursor] range
	// along the spline and renders a plain track color from the cursor to 1. When false, the full
	// gradient is painted across the entire spline.
	IMGUI_API bool SliderSplineGradientScalar( char const* label, ImGuiDataType data_type, void* p_value, void* p_min, void* p_max,
											   ImGradientData const* gradient,
											   ImVec2 const* control_points = NULL, int num_points = 4, float v_height = 0.0f,
											   float v_thickness = 0.0f, char const* format = NULL,
											   bool fill_up_to_cursor = false,
											   ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineGradientFloat( char const* label, float* value, float v_min, float v_max,
											  ImGradientData const* gradient,
											  ImVec2 const* control_points = NULL, int num_points = 4, float v_height = 0.0f,
											  float v_thickness = 0.0f, char const* format = "%.3f",
											  bool fill_up_to_cursor = false,
											  ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineGradientInt( char const* label, int* value, int v_min, int v_max,
											ImGradientData const* gradient,
											ImVec2 const* control_points = NULL, int num_points = 4, float v_height = 0.0f,
											float v_thickness = 0.0f, char const* format = "%d",
											bool fill_up_to_cursor = false,
											ImGuiSliderFlags flags = 0 );

	// SliderSplineGradientRange: two-handle range slider along a bezier spline. Gradient renders
	// only on [tLower, tUpper] via DrawSplineGradientCut. FrameBg stroke shows elsewhere.
	IMGUI_API bool SliderSplineGradientRangeScalar( char const* label, ImGuiDataType data_type, void* p_lower, void* p_upper, void* p_min, void* p_max,
	                                                ImGradientData const* gradient,
	                                                ImVec2 const* control_points = NULL, int num_points = 4, float v_height = 0.0f,
	                                                float v_thickness = 0.0f, char const* format = NULL,
	                                                ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineGradientRangeFloat( char const* label, float* v_lower, float* v_upper, float v_min, float v_max,
	                                               ImGradientData const* gradient,
	                                               ImVec2 const* control_points = NULL, int num_points = 4, float v_height = 0.0f,
	                                               float v_thickness = 0.0f, char const* format = "%.3f",
	                                               ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineGradientRangeInt( char const* label, int* v_lower, int* v_upper, int v_min, int v_max,
	                                             ImGradientData const* gradient,
	                                             ImVec2 const* control_points = NULL, int num_points = 4, float v_height = 0.0f,
	                                             float v_thickness = 0.0f, char const* format = "%d",
	                                             ImGuiSliderFlags flags = 0 );

	IMGUI_API bool DragFloatPrecise( char const* label, float* value, float v_min = 0.0f, float v_max = 0.0f, char const* format = NULL, ImGuiSliderFlags flags = 0 );

	// Up Vector selector (hemisphere picker)
	IMGUI_API bool UpVector( char const* label, float* direction, int defaultUpAxis = 1, ImVec2 size = ImVec2( 0, 0 ) );

	// Image Carousel
	IMGUI_API bool ImageCarousel( char const* label, ImTextureID* images, ImVec2* imageSizes, int imageCount, int* pSelectedIndex, ImVec2 size = ImVec2( 0, 0 ) );

	// Image Bento Grid: displays images in a uniform grid, cropping to a target cell aspect ratio (center crop).
	//
	// Drag-to-reorder support (optional):
	//   - pItemIds   : array of `imageCount` C-strings. When non-null, each cell's ImGui ID is derived
	//                  from its string instead of its position. This is required for clean drag-reorder
	//                  -- positional IDs cause a one-frame flicker when adjacent cells swap mid-drag
	//                  because ImGui's ActiveId stays pinned at the old index rather than following
	//                  the moved content. Strings must be unique per grid (duplicates across grids in
	//                  other windows are fine).
	//   - pReorderFrom / pReorderTo : out-params. When a swap is requested, the function writes the
	//                  source and target indices (both in [0, imageCount)) and returns true. The caller
	//                  is responsible for actually swapping the data -- ImageBento itself never mutates
	//                  the images/imageSizes arrays. Set to -1 on frames where no swap fires.
	//                  Swaps are reported as ADJACENT only (|from-to| == 1 horizontal, or == columnsPerRow
	//                  vertical). Multi-cell drags accumulate as multiple single-frame reports.
	// Returns true if *pSelectedIndex changed OR a reorder was requested this frame.
	IMGUI_API bool ImageBento( char const* label, ImTextureID* images, ImVec2* imageSizes, int imageCount, int* pSelectedIndex, int columnsPerRow = 4, float cellAspect = 1.0f, float spacing = 4.0f, char const* const* pItemIds = nullptr, int* pReorderFrom = nullptr, int* pReorderTo = nullptr );

	// Image Viewer: pan (left-drag), zoom (scroll wheel), double-click to reset.
	// Right-click shows a pixel-inspector loupe with RGBA values (requires state.Pixels).
	//
	// shaderProgram (optional): an ImPlatform custom shader program bound around the
	// image draw. When non-NULL, the viewer wraps its AddImage with the shader so the
	// PS colors/decodes `image` (sampled as texture0; ImGui's b0 ProjMtx and the
	// default ImGui VS interface still apply â€” author the VS as a standard ImGui VS).
	// Because the shader binds INSIDE the widget, it applies in BOTH the inline draw
	// and the expand/modal draw (which re-enters ImageViewer recursively). Pass NULL
	// (default) for a plain textured draw. The image draw in the right-click loupe is
	// left un-shaded (raw texels) so the pixel inspector reflects source data.
	//
	// ImageViewer also has a built-in "expand to window" button that re-renders the
	// same viewer inside a modal at a larger size. If overlay_callback is non-null, it
	// is invoked once per presentation (compact widget, and again inside the modal
	// when open) right after the image is drawn -- pass overlay-drawing calls through
	// it (rather than calling them manually after ImageViewer returns) so overlays
	// show up in the modal too, not just the compact view.
	IMGUI_API bool ImageViewer( char const* label, ImTextureID image, ImVec2 imageSize, ImImageViewerState& state, ImVec2 widgetSize = ImVec2( 0, 0 ), ImPlatform_ShaderProgram shaderProgram = nullptr, ImImageViewerOverlayCallback overlay_callback = NULL, void* overlay_user_data = NULL );

	// ============================================================================
	// [SECTION] Image overlays â€” composed on top of ImageViewer
	// ============================================================================
	// All coordinates are UV [0,1]^2 relative to the image, so overlays compose
	// naturally with ImageViewer's pan/zoom. Call ImageViewer FIRST, then the
	// overlay(s) with the same ImImageViewerState reference. Read-only display
	// overlays (Detection / Keypoint / TextLabel) never mutate their inputs;
	// AnnotationEditor is the only interactive one and returns edit intents via
	// ImAnnotationResult. To also support ImageViewer's built-in expand-to-window
	// modal, drive the overlay calls from an ImImageViewerOverlayCallback passed to
	// ImageViewer instead of calling them manually right after -- see the callback's
	// doc comment above ImImageViewerState.

	// A single detected object -- all coordinates normalized UV [0,1].
	struct ImDetectionBox
	{
		float x, y;      // top-left corner in UV
		float w, h;      // width/height in UV
		float score;     // confidence [0,1]; < 0 = no score to show
		int   class_id;  // index into a class-name table; -1 = unlabeled
		int   user_id;   // app-defined ID for selection tracking; -1 = none
	};

	// A single 2D landmark.
	struct ImKeypoint
	{
		float x, y;         // UV position
		float confidence;   // [0,1]; 0 = skip, <0.3 = drawn faded
		int   part_id;      // index into a part-name table
	};

	// Skeleton edge connecting two keypoints by their part_id.
	struct ImSkeletonEdge { int part_a; int part_b; };

	// Options shared by display overlays. Sizes in logical pixels (HiDPI scaled internally).
	struct ImOverlayStyle
	{
		float box_thickness;     // outline width for detection boxes
		float label_font_scale;  // 1.0 = default UI font size
		float label_bg_alpha;    // 0..1 for label chip background
		bool  show_score;
		bool  show_class_label;
		ImU32 default_color;     // 0 = auto palette by class_id
		float point_radius;      // keypoint dot radius
		float edge_thickness;    // skeleton edge line width

		ImOverlayStyle()
			: box_thickness( 2.0f ), label_font_scale( 1.0f ), label_bg_alpha( 0.6f ),
			  show_score( true ), show_class_label( true ), default_color( 0 ),
			  point_radius( 5.0f ), edge_thickness( 2.0f ) {}
	};

	// Text label anchor modes.
	enum ImTextLabelAnchor_
	{
		ImTextLabelAnchor_TopLeft    = 0,
		ImTextLabelAnchor_TopCenter  = 1,
		ImTextLabelAnchor_TopRight   = 2,
		ImTextLabelAnchor_MidLeft    = 3,
		ImTextLabelAnchor_Center     = 4,
		ImTextLabelAnchor_MidRight   = 5,
		ImTextLabelAnchor_BotLeft    = 6,
		ImTextLabelAnchor_BotCenter  = 7,
		ImTextLabelAnchor_BotRight   = 8,
		ImTextLabelAnchor_COUNT
	};

	struct ImTextLabel
	{
		float       x, y;       // UV anchor
		const char* text;
		ImU32       color;      // 0 = white
		float       font_scale; // 1.0 = default
		int         anchor;     // ImTextLabelAnchor_*
	};

	// Read-only bounding-box overlay. Returns the user_id of the hovered box (-1 = none).
	// Call after ImageViewer with the same state. class_names may be null.
	IMGUI_API int DetectionOverlay(
		const char*                str_id,
		const ImImageViewerState&  viewer_state,
		const ImDetectionBox*      boxes,
		int                        box_count,
		const char* const*         class_names,
		int                        class_name_count,
		const ImOverlayStyle*      style = NULL );

	// Read-only landmark / pose overlay. Skeleton edges are optional.
	IMGUI_API void KeypointOverlay(
		const char*                str_id,
		const ImImageViewerState&  viewer_state,
		const ImKeypoint*          keypoints,
		int                        keypoint_count,
		const ImSkeletonEdge*      edges,
		int                        edge_count,
		const char* const*         part_names,
		int                        part_name_count,
		const ImOverlayStyle*      style = NULL );

	// Floating text labels anchored at UV positions. Font size is zoom-independent.
	IMGUI_API void TextLabelOverlay(
		const char*                str_id,
		const ImImageViewerState&  viewer_state,
		const ImTextLabel*         labels,
		int                        label_count );

	// -------- AnnotationEditor: interactive bbox authoring --------
	enum ImAnnotationAction_
	{
		ImAnnotationAction_None      = 0,
		ImAnnotationAction_Move      = 1,
		ImAnnotationAction_Resize    = 2,
		ImAnnotationAction_Create    = 3,
		ImAnnotationAction_Delete    = 4,
		ImAnnotationAction_LabelEdit = 5,
	};

	struct ImAnnotationEditorState
	{
		int    SelectedId;         // user_id of selected box; -1 = none
		int    HoveredId;          // user_id under cursor
		bool   Creating;           // rubber-band new box in progress

		// Internal drag state -- do not read/write directly.
		int    _ActiveMode;        // 0=none, 1=move, 2=resize, 3=create
		int    _ActiveIndex;       // index into boxes[] for move/resize; -1 otherwise
		int    _ActiveHandle;      // 0..7 for resize handles; -1 otherwise
		ImVec2 _DragStartUV;       // rubber-band start
		ImVec2 _DragCurrUV;
		ImDetectionBox _OrigBox;   // pre-drag snapshot for Escape revert
		bool   _EditingLabel;      // inline label editor open
		int    _EditingId;         // user_id being label-edited

		ImAnnotationEditorState()
			: SelectedId( -1 ), HoveredId( -1 ), Creating( false ),
			  _ActiveMode( 0 ), _ActiveIndex( -1 ), _ActiveHandle( -1 ),
			  _DragStartUV( 0, 0 ), _DragCurrUV( 0, 0 ),
			  _EditingLabel( false ), _EditingId( -1 )
		{
			_OrigBox.x = _OrigBox.y = _OrigBox.w = _OrigBox.h = 0.0f;
			_OrigBox.score = -1.0f; _OrigBox.class_id = -1; _OrigBox.user_id = -1;
		}
	};

	struct ImAnnotationResult
	{
		int             action;             // ImAnnotationAction_*
		int             user_id;            // which box was acted on (-1 for Create)
		ImDetectionBox  new_value;          // resulting box for Move/Resize/Create
		char            new_label[ 128 ];   // filled on LabelEdit
	};

	// Returns one action per call (Action_None if nothing happened).
	// boxes[] is not mutated -- caller applies the returned action so undo stacks stay their concern.
	IMGUI_API ImAnnotationResult AnnotationEditor(
		const char*                str_id,
		const ImImageViewerState&  viewer_state,
		ImAnnotationEditorState&   editor_state,
		const ImDetectionBox*      boxes,
		int                        box_count,
		const char* const*         class_names,
		int                        class_name_count,
		const ImOverlayStyle*      style = NULL );

	// Image Inspector: color-managed raw-buffer viewer with shader-side decode of any of the 11
	// sample types x 1..4 channels described by ImImageBuffer. View transforms (gamma, sRGB,
	// camera log curves, gamut, exposure, white point, tonemap, false color, NaN highlight,
	// mosaic decode) are applied in a single HLSL uber-shader. Pan/zoom/double-click/inspector
	// loupe UX matches ImageViewer. The widget uploads buffer.host to a packed RGBA32F texture
	// once per buffer.version change; per-frame CPU cost is uniform updates only.
	//
	// The CPU-side inspector loupe reads buffer.host directly with full precision -- the user
	// must keep the buffer alive while the widget is open.
	//
	// Returns true if the user interacted with the widget. On backends without custom shader
	// support (DX9), draws a "not supported" message and returns false.
	IMGUI_API bool ImageInspector( char const* label, ImImageBuffer const& buffer, ImImageInspectorState& state, ImVec2 widgetSize = ImVec2( 0, 0 ) );

	// Returns true if the current backend supports ImageInspector (i.e. has custom shader support).
	IMGUI_API bool ImageInspectorSupported();

	// Free GPU resources owned by an ImImageInspectorState (the packed texture). Call before
	// destroying the state if you no longer need it. Safe to call multiple times.
	IMGUI_API void ImageInspectorReleaseState( ImImageInspectorState& state );

	//////////////////////////////////////////////////////////////////////////
	// Window Customization
	//////////////////////////////////////////////////////////////////////////
    // Note: it will break the rounding.
    IMGUI_API void SetCurrentWindowBackgroundImage( ImTextureID id, ImVec2 imgSize, bool fixedSize = false, ImU32 col = IM_COL32( 255, 255, 255, 255 ) );
    IMGUI_API void SetCurrentWindowBlurBackground( ImWidgetsBgEffect effect, float param0 = 0.0f, float param1 = 0.0f, float param2 = 0.0f, ImU32 tint = IM_COL32( 255, 255, 255, 255 ) );
    IMGUI_API void BlurBackgroundNewFrame();

    // Config
    IMGUI_API void SetDashedLinesUseGPU(bool enable);
    IMGUI_API bool GetDashedLinesUseGPU();
    IMGUI_API void SetDashedLinesDebugJoins(bool enable);
    IMGUI_API bool GetDashedLinesDebugJoins();

    //////////////////////////////////////////////////////////////////////////
    // Polylines (Dashed/Stroked)
    //////////////////////////////////////////////////////////////////////////

    // Unified line-drawing entry point. Selects between AddPolyline / PolylineAA /
    // StrokedPolyline / StrokedBezierPath based on desc.mode; honors desc.dashed
    // when the mode supports it (silent fallback to solid otherwise).
    IMGUI_API void DrawThickLine(ImDrawList* dl,
                                 ImVec2 const* points, int point_count,
                                 ImWidgetsThickLineDesc const& desc);

    // Human-readable mode name for combos / save files.
    IMGUI_API char const* GetThickLineModeName(ImWidgetsThickLineMode mode);

    // Draw a solid, anti-aliased polyline (Rougier 2013 SDF; selectable CPU/GPU
    // path via Set/GetDashedLinesUseGPU). Equivalent to DrawDashedPolylineAA
    // with no dashes -- uses the same code paths but skips dash math.
    IMGUI_API void DrawPolylineAA(
        ImDrawList* drawlist,
        ImVec2 const* points, int points_count,
        ImU32 col, float thickness,
        bool closed = false,
        ImWidgetsCap cap = ImWidgetsCap_Butt,
        ImWidgetsJoin join = ImWidgetsJoin_Mitter,
        float miter_limit = 4.0f);

    // Draw a dashed, anti-aliased polyline. Pattern alternates on/off lengths starting with ON.
    // - points: polyline vertices
    // - dashes: array of lengths [on, off, on, off, ...] in pixels; repeats
    // - dash_offset: initial offset into the pattern in pixels (positive shifts start forward)
    // - closed: whether to close the path (last connects to first)
    // Caps and joins are approximations using ImDrawList path stroking.
    IMGUI_API void DrawDashedPolylineAA(
        ImDrawList* drawlist,
        ImVec2 const* points, int points_count,
        ImU32 col, float thickness,
        float const* dashes, int dashes_count, float dash_offset,
        bool closed = false,
        ImWidgetsCap cap = ImWidgetsCap_Butt,
        ImWidgetsJoin join = ImWidgetsJoin_Mitter,
        float miter_limit = 4.0f);

    // Convenience: single on/off lengths.
    IMGUI_API void DrawDashedPolylineAA(
        ImDrawList* drawlist,
        ImVec2 const* points, int points_count,
        ImU32 col, float thickness,
        float dash_len, float gap_len, float dash_offset,
        bool closed = false,
        ImWidgetsCap cap = ImWidgetsCap_Butt,
        ImWidgetsJoin join = ImWidgetsJoin_Mitter,
        float miter_limit = 4.0f);

    //////////////////////////////////////////////////////////////////////////
    // Stroke Expansion (Euler Spiral) -- based on Linebender HPG 2024 paper
    //////////////////////////////////////////////////////////////////////////
    // Stroke a cubic Bezier path with high-quality parallel curves via Euler spirals.
    // points: 3*N+1 control points for N cubics [p0,p1,p2,p3, p4,p5,p6, ...]
    IMGUI_API void DrawStrokedBezierPath(
        ImDrawList* drawlist,
        ImVec2 const* points, int points_count,
        ImU32 col, float thickness,
        ImWidgetsCap cap = ImWidgetsCap_Round,
        ImWidgetsJoin join = ImWidgetsJoin_Round,
        float miter_limit = 4.0f,
        float tolerance = 0.25f,
        bool closed = false,
        ImWidgetsPrimitive primitive = ImWidgetsPrimitive_Line,
        ImWidgetsCorrectness correctness = ImWidgetsCorrectness_Weak);

    // Stroke a single cubic Bezier curve.
    IMGUI_API void DrawStrokedCubicBezier(
        ImDrawList* drawlist,
        ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3,
        ImU32 col, float thickness,
        ImWidgetsCap cap = ImWidgetsCap_Round,
        float tolerance = 0.25f,
        ImWidgetsPrimitive primitive = ImWidgetsPrimitive_Line,
        ImWidgetsCorrectness correctness = ImWidgetsCorrectness_Weak);

    // Stroke a polyline with Euler spiral offset curves (high-quality thick joins).
    IMGUI_API void DrawStrokedPolyline(
        ImDrawList* drawlist,
        ImVec2 const* points, int points_count,
        ImU32 col, float thickness,
        ImWidgetsCap cap = ImWidgetsCap_Round,
        ImWidgetsJoin join = ImWidgetsJoin_Round,
        float miter_limit = 4.0f,
        bool closed = false);

    // Stroke a dashed cubic Bezier path. dash_array is alternating dash,gap lengths.
    IMGUI_API void DrawStrokedDashedBezierPath(
        ImDrawList* drawlist,
        ImVec2 const* points, int points_count,
        ImU32 col, float thickness,
        float const* dash_array, int dash_count, float dash_offset = 0.0f,
        ImWidgetsCap cap = ImWidgetsCap_Round,
        ImWidgetsJoin join = ImWidgetsJoin_Round,
        float miter_limit = 4.0f,
        float tolerance = 0.25f,
        bool closed = false,
        ImWidgetsPrimitive primitive = ImWidgetsPrimitive_Line,
        ImWidgetsCorrectness correctness = ImWidgetsCorrectness_Weak);

    // Stroke a dashed polyline.
    IMGUI_API void DrawStrokedDashedPolyline(
        ImDrawList* drawlist,
        ImVec2 const* points, int points_count,
        ImU32 col, float thickness,
        float const* dash_array, int dash_count, float dash_offset = 0.0f,
        ImWidgetsCap cap = ImWidgetsCap_Round,
        ImWidgetsJoin join = ImWidgetsJoin_Round,
        float miter_limit = 4.0f,
        bool closed = false);

    // Debug
    IMGUI_API void SetStrokeDebugWireframe(bool enable);
    IMGUI_API bool GetStrokeDebugWireframe();

    //////////////////////////////////////////////////////////////////////////
    // Extended primitives (D1-D6)
    //////////////////////////////////////////////////////////////////////////

    // D1. Hatching & stippling fills (scanline-clip against arbitrary polygon)
    IMGUI_API void DrawHatchFill(ImDrawList* drawlist,
                                 ImVec2 const* poly, int poly_count,
                                 ImWidgetsHatchPattern pattern,
                                 float spacing, float angle_rad,
                                 float thickness, ImU32 col);
    IMGUI_API void DrawStippleFill(ImDrawList* drawlist,
                                   ImVec2 const* poly, int poly_count,
                                   float density, float jitter,
                                   float radius, ImU32 col,
                                   unsigned int seed = 1u);

    // D2. Bezier surface patches
    IMGUI_API void DrawCoonsPatch(ImDrawList* drawlist,
                                  ImCoonsPatch const& patch,
                                  ImU32 col, int resU = 16, int resV = 16);
    IMGUI_API void DrawCoonsPatchGradient(ImDrawList* drawlist,
                                          ImCoonsPatch const& patch,
                                          ImU32 c00, ImU32 c10, ImU32 c11, ImU32 c01,
                                          int resU = 16, int resV = 16);
    IMGUI_API void DrawCoonsPatchWireframe(ImDrawList* drawlist,
                                           ImCoonsPatch const& patch,
                                           ImU32 col, float thickness,
                                           int resU = 16, int resV = 16);
    IMGUI_API void DrawGregoryPatch(ImDrawList* drawlist,
                                    ImGregoryPatch const& patch,
                                    ImU32 col, int resU = 16, int resV = 16);
    IMGUI_API void DrawGregoryPatchGradient(ImDrawList* drawlist,
                                            ImGregoryPatch const& patch,
                                            ImU32 col_bl, ImU32 col_br,
                                            ImU32 col_tr, ImU32 col_tl,
                                            int resU = 16, int resV = 16);

    // D3. Rounded path offsetting
    IMGUI_API void OffsetPolyline(ImVec2 const* in_pts, int n,
                                  float offset, ImWidgetsJoin join,
                                  float miter_limit,
                                  ImVector<ImVec2>& out_pts, bool closed);
    IMGUI_API void DrawOffsetOutline(ImDrawList* drawlist,
                                     ImVec2 const* pts, int n,
                                     float offset, ImWidgetsJoin join,
                                     float miter_limit, float thickness,
                                     ImU32 col, bool closed);
    IMGUI_API void DrawOffsetFilled(ImDrawList* drawlist,
                                    ImVec2 const* pts, int n,
                                    float inset, ImU32 col, bool closed);

    // D4. Iso-contours (marching squares)
    IMGUI_API void DrawIsoContour(ImDrawList* drawlist,
                                  ImVec2 pos, ImVec2 size,
                                  int resX, int resY,
                                  ImWidgetsScalar2DCallback f, void* user,
                                  float const* iso_values, int iso_count,
                                  ImU32 col, float thickness);
    // Multi-tier iso-contour (major/medium/minor in one pass).
    // Draws tiers in array order; put finest/faintest first, major last so it paints on top.
    // pre_log_transform: replace each sample v with log(max(v, eps)) before contouring.
    // min_value/max_value: limit iso enumeration to this value range.
    // half_pixel_sample: sample each cell at (i+0.5)*dx instead of i*dx.
    // sample_subdiv: internal sub-sampling per cell (1=off, 2=2x density via bilinear).
    IMGUI_API void DrawIsoContourTiered(ImDrawList* drawlist,
                                        ImVec2 pos, ImVec2 size,
                                        int resX, int resY,
                                        ImWidgetsScalar2DCallback f, void* user,
                                        ImIsoContourTier const* tiers, int tier_count,
                                        float min_value = -FLT_MAX, float max_value = FLT_MAX,
                                        bool pre_log_transform = false,
                                        bool half_pixel_sample = true,
                                        int sample_subdiv = 1);

    IMGUI_API void DrawIsoFilled(ImDrawList* drawlist,
                                 ImVec2 pos, ImVec2 size,
                                 int resX, int resY,
                                 ImWidgetsScalar2DCallback f, void* user,
                                 float const* band_edges, int band_edge_count,
                                 ImU32 const* band_cols);

    // D5. Conic gradient fill
    IMGUI_API void DrawConicGradient(ImDrawList* drawlist,
                                     ImVec2 center, float radius,
                                     ImGradientData const& gradient,
                                     float start_angle_rad,
                                     int resolution = 128);
    IMGUI_API void DrawConicGradientRect(ImDrawList* drawlist,
                                         ImVec2 min, ImVec2 max,
                                         ImVec2 center_uv,
                                         ImGradientData const& gradient,
                                         float start_angle_rad,
                                         int resolution = 128);

    // D6. Superellipse (generalization of GenShapeSquircle)
    IMGUI_API void GenShapeSuperellipse(ImWidgetsShape& shape,
                                        ImVec2 center,
                                        float rx, float ry,
                                        float nx, float ny,
                                        int sides);

    //////////////////////////////////////////////////////////////////////////
    // Interaction helpers (I1-I3)
    //////////////////////////////////////////////////////////////////////////

    // I1. Angle-wrap/clamp-aware ring slider (thin wrapper exposing flags).
    IMGUI_API bool SliderRingFloatEx(char const* label, float* value,
                                     float v_min, float v_max,
                                     float v_angle_min, float v_angle_max,
                                     float v_thickness,
                                     char const* format,
                                     ImWidgetsSliderFlags widgets_flags,
                                     ImGuiSliderFlags imgui_flags = 0);

    // I2. Delta overlays
    IMGUI_API void PushDeltaOverlay(ImGuiID id, float start_value);
    IMGUI_API void RenderDeltaOverlay(ImGuiID id, float current,
                                      char const* fmt = nullptr);

    // I3. Proportional multi-drag group
    IMGUI_API void PushDragGroup(ImGuiID group_id,
                                 float falloff_radius = 2.0f,
                                 ImWidgetsFalloff kernel = ImWidgetsFalloff_Gaussian);
    IMGUI_API void PopDragGroup();
    // Reports the additive delta this frame for the given slider's group position.
    // Returns 0 if not in a group or group is inactive.
    IMGUI_API float GetDragGroupDelta(int group_slot);
    IMGUI_API void  SetDragGroupActive(int group_slot, float drag_delta);

    //////////////////////////////////////////////////////////////////////////
    // New widgets (W1-W7)
    //////////////////////////////////////////////////////////////////////////

    // W2. Vector drawing tool -- declaration lives in dear_widgets_vector_drawing.h
    // (included above). Left here as a breadcrumb so navigation by section still
    // lands at the right spot.

    // W4. Font inspector (4 modes in one widget; mode is user-controlled)
    IMGUI_API void FontInspector(char const* label,
                                 ImFont* font,
                                 ImFontInspectorMode mode = ImFontInspectorMode_Grid,
                                 float display_size = 64.0f,
                                 ImVec2 size = ImVec2(0, 0));

    // W5. Notched dial
    IMGUI_API bool NotchedDial(char const* label, float* v,
                               float const* stops, int stop_count,
                               char const* const* stop_labels = nullptr,
                               ImVec2 size = ImVec2(0, 0));

    // Continuous angle dial. Value is in DEGREES (the caller converts to/from
    // radians if its data is stored that way). 0 deg points East (right), and
    // positive degrees sweep counter-clockwise. When [v_min,v_max] spans a full
    // turn (>= 360 deg) the dial wraps freely; a smaller span clamps to an arc.
    // Drag to set, scroll wheel to nudge (+/-1 deg, +/-10 with Shift), hold
    // Ctrl to snap to 15 deg, double-click for a type-exact precision popup.
    // The current value is shown as a "NN deg" readout under the dial.
    IMGUI_API bool AngleDial(char const* label, float* v_deg,
                             float v_min_deg = -180.0f, float v_max_deg = 180.0f,
                             ImVec2 size = ImVec2(0, 0));

    // W6. Color-difference visualizer
    IMGUI_API float ColorDeltaE76   (ImVec4 a, ImVec4 b);
    IMGUI_API float ColorDeltaE94   (ImVec4 a, ImVec4 b, float kL = 1.0f, float kC = 1.0f, float kH = 1.0f);
    IMGUI_API float ColorDeltaE2000 (ImVec4 a, ImVec4 b, float kL = 1.0f, float kC = 1.0f, float kH = 1.0f);
    IMGUI_API float ColorDeltaEOK   (ImVec4 a, ImVec4 b);
    IMGUI_API char const* DeltaEFormulaName(ImDeltaEFormula f);
    IMGUI_API char const* DeltaEPerceptualLabel(float de); // "Imperceptible" / "Just noticeable" / ...
    IMGUI_API void ColorDifferenceVisualizer(char const* label,
                                             ImVec4* a, ImVec4* b,
                                             ImDeltaEFormula formula = ImDeltaEFormula_E2000);

    // W7. Equation input box (markdown-style $ / $$ delimited LaTeX)
    IMGUI_API bool EquationInput(char const* label,
                                 char* buf, size_t buf_size,
                                 ImVec2 size = ImVec2(0, 0),
                                 ImWidgetsEquationFlags flags = 0);

    // DPI / scaling helpers have moved to ImPlatform:
    //   ImPlatform_LpToPx(float | ImVec2)
    //   ImPlatform_PxToLp(float | ImVec2)
    //   ImPlatform_LpPxScale()
    // (See extern/ImPlatform/ImPlatform/ImPlatform.h.)

    //////////////////////////////////////////////////////////////////////////
    // Shared interaction helpers (used across the polish-pass widgets)
    //////////////////////////////////////////////////////////////////////////

    // Precision popup: shared "double-click a handle to type exact values" widget.
    // Typical flow:
    //   if (hover && IsMouseDoubleClicked(0)) ImGui::OpenPopup("##prec");
    //   if (ImWidgets::BeginPrecisionPopup("##prec", anchor_screen_pos)) {
    //       ImWidgets::PrecisionFloat("Value", &v);
    //       ImWidgets::EndPrecisionPopup();
    //   }
    IMGUI_API bool BeginPrecisionPopup(char const* str_id, ImVec2 anchor_screen_pos);
    IMGUI_API void EndPrecisionPopup();
    IMGUI_API bool PrecisionFloat(char const* label, float* v, float step = 0.0f,
                                  float v_min = -FLT_MAX, float v_max = FLT_MAX,
                                  char const* fmt = "%.4f");
    IMGUI_API bool PrecisionInt  (char const* label, int*   v, int step = 1,
                                  int v_min = -INT_MAX, int v_max = INT_MAX);

    // Keyboard nudge: returns the delta in value units when an arrow key is
    // pressed while the caller's widget is hovered or active. Scales by:
    //   Shift   -> x10
    //   Ctrl    -> x0.1
    // Use SetKeyOwner at call site if the widget is active to suppress other consumers.
    IMGUI_API ImVec2 KeyboardNudgeXY();
    IMGUI_API float  KeyboardNudge1D(bool horizontal = true);

    // Axis-constrain: when Shift is held, snap drag delta to the dominant axis.
    IMGUI_API ImVec2 AxisConstrain(ImVec2 drag_delta);

    // ------------------------------------------------------------------
    // Shared curve data model (consolidation target -- new widgets should
    // prefer this over the older ImCurveEditorData / ImColorCurveData types,
    // which remain supported for existing ColorCurve/ToneCurve/CurveEditor
    // widgets and will be ported onto this type in a follow-up pass).
    // ------------------------------------------------------------------
    enum ImCurveInterp_
    {
        ImCurveInterp_Linear = 0,
        ImCurveInterp_Step,          // piecewise constant (right-hold)
        ImCurveInterp_CubicBezier,   // uses TangentL / TangentR of adjacent keys
        ImCurveInterp_COUNT
    };
    typedef int ImCurveInterp;

    // Rubber-band / box selection helper.
    struct ImBoxSelectState
    {
        ImVec2 Start;
        ImVec2 End;
        bool   Active;
        ImBoxSelectState() : Start( 0, 0 ), End( 0, 0 ), Active( false ) {}
    };
    // Call each frame. Starts a box select when the user clicks on empty area
    // inside the canvas (caller passes hovered_item as "nothing hit" predicate).
    // Updates state in-place. Returns true while the selection rect is active.
    IMGUI_API bool BoxSelectUpdate(ImBoxSelectState* state, ImVec2 canvas_min,
                                   ImVec2 canvas_max, bool hovered_empty);
    IMGUI_API void BoxSelectDraw  (ImBoxSelectState const* state, ImDrawList* dl,
                                   ImU32 fill = IM_COL32( 120, 220, 255, 40 ),
                                   ImU32 border = IM_COL32( 120, 220, 255, 200 ));
    // Returns the sorted screen-space rect that the current/just-ended box covers.
    IMGUI_API ImRect BoxSelectRect(ImBoxSelectState const* state);

    // ------------------------------------------------------------------
    // Shared curve data (ImCurveKey / ImCurveData / ImCurveEval)
    // ------------------------------------------------------------------
    struct ImCurveKey
    {
        ImVec2        Pos;          // (x, y) in user space
        ImVec2        TangentL;     // Bezier handle into previous segment (relative)
        ImVec2        TangentR;     // Bezier handle into next segment (relative)
        ImCurveInterp InterpToNext; // interpolation style used for segment (this_key -> next_key)
        int           Flags;        // bit 0 = broken tangents, bit 1 = selected

        ImCurveKey() : Pos( 0, 0 ), TangentL( -0.1f, 0.0f ), TangentR( 0.1f, 0.0f ),
                       InterpToNext( ImCurveInterp_CubicBezier ), Flags( 0 ) {}
        ImCurveKey( ImVec2 p ) : Pos( p ), TangentL( -0.1f, 0.0f ), TangentR( 0.1f, 0.0f ),
                                 InterpToNext( ImCurveInterp_CubicBezier ), Flags( 0 ) {}
    };

    struct ImCurveData
    {
        ImVector<ImCurveKey> Keys;
        int                  SelectedIdx;  // -1 if none

        ImCurveData() : SelectedIdx( -1 ) {}
    };

    // Evaluate Y at a given X. Returns the endpoint value when x is outside
    // [Keys[0].Pos.x, Keys.back().Pos.x]. Requires Keys sorted by Pos.x.
    IMGUI_API float ImCurveEval(ImCurveData const& c, float x);
    // Insert a key at (p.x, p.y) keeping Keys sorted by Pos.x; returns its index.
    IMGUI_API int   ImCurveAdd (ImCurveData& c, ImVec2 p);
    IMGUI_API void  ImCurveRemove(ImCurveData& c, int idx);
    IMGUI_API void  ImCurveSort(ImCurveData& c);

    //////////////////////////////////////////////////////////////////////////
    // Phase D -- Color grading
    //////////////////////////////////////////////////////////////////////////

    IMGUI_API bool LoadCubeLUT(ImColorLUT3D& out, char const* filename);
    IMGUI_API bool SaveCubeLUT(ImColorLUT3D const& in, char const* filename);
    IMGUI_API ImVec4 SampleLUT3D(ImColorLUT3D const& lut, ImVec4 rgb_in);
    IMGUI_API void  InitIdentityLUT3D(ImColorLUT3D& out, int size);
    IMGUI_API bool  ColorLUT3DViewer(char const* label, ImColorLUT3D* lut,
                                     ImVec2 size = ImVec2(0, 0));

    //////////////////////////////////////////////////////////////////////////
    // Phase E -- LookDev A/B compare
    //////////////////////////////////////////////////////////////////////////

    IMGUI_API bool LookDevCompare(char const* label,
                                  ImTextureID tex_a, ImTextureID tex_b,
                                  ImVec2 a_uv_min, ImVec2 a_uv_max,
                                  ImVec2 b_uv_min, ImVec2 b_uv_max,
                                  ImLookDevState* state,
                                  ImVec2 size = ImVec2(0, 0));

    // LookDevInspector -- A/B compare with per-side exposure/black/white/gamma via
    // a dedicated shader (`lookdev_inspector.hlsl`). The shader samples both
    // textures across the widget rect, selects A or B based on the divider line,
    // and applies side-specific tonemap. Falls back to LookDevCompare if the
    // backend does not support custom shaders.
    IMGUI_API bool LookDevInspector(char const* label,
                                    ImTextureID tex_a, ImTextureID tex_b,
                                    ImLookDevInspectorState* state,
                                    ImVec2 size = ImVec2(0, 0));

    // Shader-based dE compare.
    enum ImDeltaEFormulaShader_
    {
        ImDeltaEFormulaShader_E76    = 0,
        ImDeltaEFormulaShader_EOK    = 1,
        ImDeltaEFormulaShader_E94    = 2,
        ImDeltaEFormulaShader_E2000  = 3,
        ImDeltaEFormulaShader_COUNT
    };
    typedef int ImDeltaEFormulaShader;

    enum ImDeltaEColorRamp_
    {
        ImDeltaEColorRamp_Gray = 0,
        ImDeltaEColorRamp_Viridis,
        ImDeltaEColorRamp_Turbo,
        ImDeltaEColorRamp_Magma,
        ImDeltaEColorRamp_COUNT
    };
    typedef int ImDeltaEColorRamp;

    struct ImDeltaECompareState
    {
        ImDeltaEFormulaShader Formula;
        ImDeltaEColorRamp     Ramp;
        float                 MaxDeltaE;     // maps dE=MaxDeltaE -> ramp top
        float                 Threshold;     // overlay red above this dE
        bool                  ShowThreshold;
        float                 MixA;          // 0..1 layer of raw A underneath
        float                 MixB;
        float                 MixDeltaE;

        ImDeltaECompareState()
            : Formula( ImDeltaEFormulaShader_E76 ), Ramp( ImDeltaEColorRamp_Viridis ),
              MaxDeltaE( 20.0f ), Threshold( 5.0f ), ShowThreshold( false ),
              MixA( 0.0f ), MixB( 0.0f ), MixDeltaE( 1.0f ) {}
    };

    IMGUI_API bool ColorDifferenceImageViewer(char const* label,
                                              ImTextureID tex_a, ImTextureID tex_b,
                                              ImDeltaECompareState* state,
                                              ImVec2 size = ImVec2(0, 0));

    //////////////////////////////////////////////////////////////////////////
    // Phase G -- Volume Slice Viewer (CPU-side slice extraction path).
    // Uses ImPlatform_CreateTexture3D for voxel upload (when the backend
    // supports it); the widget itself extracts a 2D slice on CPU each frame
    // and uploads it to a 2D texture for display. This keeps the widget
    // backend-agnostic while the full GPU volume_slice shader lands later.
    //////////////////////////////////////////////////////////////////////////

    enum ImVolumeSliceAxis_
    {
        ImVolumeSliceAxis_X = 0,
        ImVolumeSliceAxis_Y = 1,
        ImVolumeSliceAxis_Z = 2
    };
    typedef int ImVolumeSliceAxis;

    struct ImVolumeSliceState
    {
        // Source volume (caller-owned).
        float const* Voxels;  // w*h*d floats, 0..1 range recommended
        int Width, Height, Depth;
        // View parameters.
        ImVolumeSliceAxis Axis;
        float SliceT;         // 0..1
        float WindowMin, WindowMax;
        float Gamma;
        // Internal -- widget reuses a cached 2D texture for display.
        ImTextureID CachedTex;
        int CachedW, CachedH;

        ImVolumeSliceState()
            : Voxels(NULL), Width(0), Height(0), Depth(0),
              Axis(ImVolumeSliceAxis_Z), SliceT(0.5f),
              WindowMin(0.0f), WindowMax(1.0f), Gamma(1.0f),
              CachedTex(NULL), CachedW(0), CachedH(0) {}
    };

    IMGUI_API bool VolumeSliceViewer(char const* label, ImVolumeSliceState* state,
                                     ImVec2 size = ImVec2(0, 0));
    // Utility: generate a test 128x128x128 Perlin-ish volume (caller-owns buffer).
    IMGUI_API void VolumeGenerateTestField(float* out_voxels, int w, int h, int d, float seed = 0.0f);
    enum ImVolumeField_
    {
        ImVolumeField_Sines = 0,        // sum-of-sines (VolumeGenerateTestField)
        ImVolumeField_Sphere,           // centered radial falloff
        ImVolumeField_NestedSpheres,    // concentric shells, for MIP/iso demo
        ImVolumeField_Torus,            // ring around the Y axis
        ImVolumeField_Gyroid,           // Schoen gyroid (TPMS) implicit
        ImVolumeField_SphericalNoise,   // 3D value noise inside a radial window
        ImVolumeField_COUNT
    };
    typedef int ImVolumeField;
    IMGUI_API void VolumeGenerateField(float* out_voxels, int w, int h, int d,
                                       ImVolumeField mode, float seed = 0.0f);

    //////////////////////////////////////////////////////////////////////////
    // VolumeViewer -- combined volumetric widget with multiple rendering modes.
    //
    // Phase 1 (this session): 3-slice axial/sagittal/coronal layout using three
    // existing VolumeSliceViewer instances sharing the same voxel data.
    //
    // Phase 2 (follow-up): Raymarch (DDA), MIP, and Iso-Surface modes require a
    // Texture3D-sampling pixel shader (`volume_viewer.hlsl`) and are stubbed
    // today; the UI exposes them with a "Coming soon" message.
    //////////////////////////////////////////////////////////////////////////
    enum ImVolumeViewerMode_
    {
        ImVolumeViewerMode_ThreeSlice = 0,
        ImVolumeViewerMode_Raymarch   = 1,  // stub
        ImVolumeViewerMode_MIP        = 2,  // stub
        ImVolumeViewerMode_IsoSurface = 3,  // stub
        ImVolumeViewerMode_COUNT
    };
    typedef int ImVolumeViewerMode;

    struct ImVolumeViewerState
    {
        // Source volume (caller-owned) -- same contract as ImVolumeSliceState.
        float const* Voxels;
        int Width, Height, Depth;

        ImVolumeViewerMode Mode;

        // Per-axis slice positions (used by 3-slice mode).
        float SliceX;   // 0..1
        float SliceY;   // 0..1
        float SliceZ;   // 0..1

        // Shared display controls.
        float WindowMin;
        float WindowMax;
        float Gamma;

        // Orbital camera (raymarch modes; stubs today).
        float CamAzimuth;   // radians
        float CamElevation; // radians
        float CamDistance;  // world units

        // Raymarch-specific (stubs today).
        float StepSize;
        float IsoThreshold;

        // Internal per-axis slice caches so the three views don't alloc per frame.
        ImVolumeSliceState CacheX;
        ImVolumeSliceState CacheY;
        ImVolumeSliceState CacheZ;

        // Cached Texture3D (raymarch modes). Re-uploaded when Voxels pointer or
        // dims change. Leaked on widget destruction -- acceptable for runtime state.
        ImTextureID Tex3D;
        float const* Tex3DLastVoxels;
        int Tex3DLastW, Tex3DLastH, Tex3DLastD;

        // Color ramp (raymarch/MIP modes). 0 = gray, 1 = viridis.
        int ColorRamp;

        ImVolumeViewerState()
            : Voxels(NULL), Width(0), Height(0), Depth(0),
              Mode(ImVolumeViewerMode_Raymarch),   // DDA raymarch is the headline mode
              SliceX(0.5f), SliceY(0.5f), SliceZ(0.5f),
              WindowMin(0.0f), WindowMax(1.0f), Gamma(1.0f),
              CamAzimuth(0.5f), CamElevation(0.3f), CamDistance(2.0f),
              StepSize(0.01f), IsoThreshold(0.5f),
              Tex3D(NULL), Tex3DLastVoxels(NULL), Tex3DLastW(0), Tex3DLastH(0), Tex3DLastD(0),
              ColorRamp(1) {}
    };

    IMGUI_API bool VolumeViewer(char const* label, ImVolumeViewerState* state,
                                ImVec2 size = ImVec2(0, 0));

    //////////////////////////////////////////////////////////////////////////
    // Range Slider (1D two-handle min/max slider, no gradient).
    // Same pattern as SliderGradientRange but plain FrameBg + selection rect.
    //////////////////////////////////////////////////////////////////////////
    IMGUI_API bool RangeSliderScalar( char const* label, ImGuiDataType data_type,
                                      void* p_lower, void* p_upper,
                                      void const* p_min, void const* p_max,
                                      char const* format = NULL,
                                      ImVec2 size = ImVec2( 0, 0 ) );
    IMGUI_API bool RangeSliderFloat ( char const* label, float* v_lower, float* v_upper,
                                      float v_min, float v_max,
                                      char const* format = "%.3f",
                                      ImVec2 size = ImVec2( 0, 0 ) );
    IMGUI_API bool RangeSliderInt   ( char const* label, int* v_lower, int* v_upper,
                                      int v_min, int v_max,
                                      char const* format = "%d",
                                      ImVec2 size = ImVec2( 0, 0 ) );

    // Vertical two-handle range slider. Min at the bottom, max at the top.
    // `size` is (width, height) in lp; defaults to a tall 20x160 lp strip.
    IMGUI_API bool RangeSliderVerticalScalar( char const* label, ImGuiDataType data_type,
                                      void* p_lower, void* p_upper,
                                      void const* p_min, void const* p_max,
                                      char const* format = NULL,
                                      ImVec2 size = ImVec2( 0, 0 ) );
    IMGUI_API bool RangeSliderVerticalFloat ( char const* label, float* v_lower, float* v_upper,
                                      float v_min, float v_max,
                                      char const* format = "%.3f",
                                      ImVec2 size = ImVec2( 0, 0 ) );
    IMGUI_API bool RangeSliderVerticalInt   ( char const* label, int* v_lower, int* v_upper,
                                      int v_min, int v_max,
                                      char const* format = "%d",
                                      ImVec2 size = ImVec2( 0, 0 ) );

    //////////////////////////////////////////////////////////////////////////
    // Segmented Control (N exclusive options in one bar).
    // p_value stores the selected index in any integer ImGuiDataType
    // (S8/U8/S16/U16/S32/U32/S64/U64). Float/Double are not supported.
    //////////////////////////////////////////////////////////////////////////
    IMGUI_API bool SegmentedControlScalar( char const* label, ImGuiDataType data_type,
                                           void* p_value,
                                           char const* const* labels, int label_count,
                                           ImVec2 size = ImVec2( 0, 0 ) );
    IMGUI_API bool SegmentedControlInt   ( char const* label, int* p_value,
                                           char const* const* labels, int label_count,
                                           ImVec2 size = ImVec2( 0, 0 ) );

    //////////////////////////////////////////////////////////////////////////
    // Arrow primitives.
    //////////////////////////////////////////////////////////////////////////
    enum ImWidgetsArrowHead_
    {
        ImWidgetsArrowHead_None     = 0,
        ImWidgetsArrowHead_Triangle = 1,  // filled triangle
        ImWidgetsArrowHead_Open     = 2,  // two stroked lines (V shape)
        ImWidgetsArrowHead_Diamond  = 3,  // filled rhombus
        ImWidgetsArrowHead_Stealth  = 4,  // concave back arrow
        ImWidgetsArrowHead_Tick     = 5,  // short perpendicular line (CAD)
        ImWidgetsArrowHead_Dot      = 6,  // filled circle
        ImWidgetsArrowHead_Square   = 7,  // axis-aligned square block
        ImWidgetsArrowHead_COUNT
    };
    typedef int ImWidgetsArrowHead;

    IMGUI_API char const* GetArrowHeadName( ImWidgetsArrowHead head );

    // Draw an arrow shaft from `from` to `to` with optional heads at either end.
    // head_size is in pixels. The shaft is shortened so it doesn't poke through
    // a filled head; pass ImWidgetsArrowHead_None to draw a plain segment.
    IMGUI_API void DrawArrow( ImDrawList* draw, ImVec2 from, ImVec2 to,
                              ImU32 col, float thickness = 1.0f,
                              ImWidgetsArrowHead head_end   = ImWidgetsArrowHead_Triangle,
                              ImWidgetsArrowHead head_start = ImWidgetsArrowHead_None,
                              float head_size = 10.0f );

    // Draw a head primitive alone at `tip`, pointing along `dir` (need not be normalized).
    IMGUI_API void DrawArrowHead( ImDrawList* draw, ImVec2 tip, ImVec2 dir,
                                  ImU32 col, ImWidgetsArrowHead style,
                                  float size = 10.0f, float thickness = 1.0f );

    // Convenience: draw X (right) and Y (down on positive len_y, up on negative)
    // axis arrows from `origin` with short text labels at each tip.
    IMGUI_API void DrawAxisArrows( ImDrawList* draw, ImVec2 origin,
                                   float len_x, float len_y,
                                   ImU32 col_x = IM_COL32( 230, 80, 80, 255 ),
                                   ImU32 col_y = IM_COL32( 80, 200, 80, 255 ),
                                   float thickness = 1.0f, float head_size = 10.0f,
                                   char const* label_x = "x", char const* label_y = "y",
                                   ImU32 label_col = IM_COL32( 230, 230, 230, 255 ) );

    //////////////////////////////////////////////////////////////////////////
    // Dimension Line (CAD-style measurement with arrowheads, extension lines, label).
    //////////////////////////////////////////////////////////////////////////
    enum ImWidgetsDimensionTextOrient_
    {
        ImWidgetsDimensionTextOrient_FollowLine       = 0,  // rotate to the line direction
        ImWidgetsDimensionTextOrient_AlwaysHorizontal = 1,  // text horizontal regardless of angle
        ImWidgetsDimensionTextOrient_FollowReading    = 2,  // FollowLine but auto-flip so text reads L->R
        ImWidgetsDimensionTextOrient_Perpendicular    = 3,  // rotate 90deg to the line (also auto-flipped)
        ImWidgetsDimensionTextOrient_COUNT
    };
    typedef int ImWidgetsDimensionTextOrient;

    IMGUI_API char const* GetDimensionTextOrientName( ImWidgetsDimensionTextOrient orient );

    enum ImWidgetsDimensionFlags_
    {
        ImWidgetsDimensionFlags_None             = 0,
        ImWidgetsDimensionFlags_ExtensionLines   = 1 << 0,  // draw perpendicular extension lines from measured endpoints
        ImWidgetsDimensionFlags_HeadsInside      = 1 << 1,  // arrowheads point inward (default: outward)
        ImWidgetsDimensionFlags_TextAbove        = 1 << 2,  // text sits above the dim line (offset side)
        ImWidgetsDimensionFlags_TextBelow        = 1 << 3,  // text sits below the dim line
        ImWidgetsDimensionFlags_NoBreak          = 1 << 4,  // when text is centered, do NOT break the line under the text
        ImWidgetsDimensionFlags_Default          = ImWidgetsDimensionFlags_ExtensionLines | ImWidgetsDimensionFlags_TextAbove
    };
    typedef int ImWidgetsDimensionFlags;

    // CAD-style dimension annotation: arrowed dim line measuring `from` -> `to`,
    // perpendicular extension lines, and a centered label.
    // `offset` (pixels) displaces the dim line perpendicular to the measured
    // segment (positive = left of direction `to - from`).
    // `text` is the caller-formatted value, e.g. "1.24 m" or "42 px".
    // Pass font/font_size = NULL/0 to use the current ImGui font.
    IMGUI_API void DrawDimensionLine( ImDrawList* draw, ImVec2 from, ImVec2 to,
                                      float offset, char const* text,
                                      ImU32 line_col, ImU32 text_col,
                                      float line_thickness = 1.0f,
                                      ImWidgetsArrowHead head = ImWidgetsArrowHead_Triangle,
                                      float head_size = 8.0f,
                                      float ext_overshoot = 4.0f,
                                      float ext_gap = 2.0f,
                                      ImWidgetsDimensionTextOrient text_orient = ImWidgetsDimensionTextOrient_FollowReading,
                                      ImWidgetsDimensionFlags flags = ImWidgetsDimensionFlags_Default,
                                      ImFont* font = NULL, float font_size = 0.0f );

    //////////////////////////////////////////////////////////////////////////
    // Corner anchor used by widgets that place a sub-element inside a parent rect.
    //////////////////////////////////////////////////////////////////////////
    enum ImWidgetsCornerAnchor_
    {
        ImWidgetsCornerAnchor_TopLeft     = 0,
        ImWidgetsCornerAnchor_TopRight    = 1,
        ImWidgetsCornerAnchor_BottomLeft  = 2,
        ImWidgetsCornerAnchor_BottomRight = 3,
        ImWidgetsCornerAnchor_COUNT
    };
    typedef int ImWidgetsCornerAnchor;

    //////////////////////////////////////////////////////////////////////////
    // Eyedropper: sample a color from a user-supplied surface and (optionally)
    // show a magnified neighbor grid + crosshair tooltip under the cursor.
    //   - `screen_rect` is the area on screen the user can hover.
    //   - `sample_fn(uv, user_data)` returns the color at uv in [0,1]^2.
    //     A helper for raw 32-bit RGBA bitmaps is provided below.
    //////////////////////////////////////////////////////////////////////////
    struct ImWidgetsEyedropperBitmap
    {
        ImU32 const* Pixels;  // tightly-packed IM_COL32 array
        int          Width;
        int          Height;
    };

    typedef ImU32 ( *ImWidgetsEyedropperSampleFn )( ImVec2 uv, void* user_data );

    IMGUI_API ImU32 EyedropperSampleBitmap( ImVec2 uv, void* user_data );  // user_data: ImWidgetsEyedropperBitmap*

    struct ImWidgetsEyedropperResult
    {
        bool   Hovered;       // mouse is inside `screen_rect` this frame
        bool   Picked;        // user just released LMB inside the area (color is now locked by caller)
        ImVec2 PosScreen;     // cursor pos when sampled (screen coords)
        ImVec2 UV;             // 0..1 within the source surface
        ImU32  Color;         // center sample
    };

    IMGUI_API ImWidgetsEyedropperResult Eyedropper(
        char const* id_str, ImRect screen_rect,
        ImWidgetsEyedropperSampleFn sample_fn, void* user_data,
        int   neighbor_cells   = 7,                              // odd; 0 to disable loupe
        float neighbor_pixel   = 14.0f,                          // size of each loupe cell in screen pixels
        ImU32 outline_col      = IM_COL32( 0, 0, 0, 220 ),
        ImU32 highlight_col    = IM_COL32( 255, 255, 255, 220 ) );

    //////////////////////////////////////////////////////////////////////////
    // Crop rect: 8-grip resizable rectangle inside a bounds rect, with
    // optional rule-of-thirds / golden ratio / diagonal / center guides and
    // a darkened mask outside the crop region.
    //////////////////////////////////////////////////////////////////////////
    enum ImWidgetsCropGuides_
    {
        ImWidgetsCropGuides_None         = 0,
        ImWidgetsCropGuides_RuleOfThirds = 1 << 0,
        ImWidgetsCropGuides_GoldenRatio  = 1 << 1,
        ImWidgetsCropGuides_Diagonals    = 1 << 2,
        ImWidgetsCropGuides_Center       = 1 << 3,
        ImWidgetsCropGuides_Default      = ImWidgetsCropGuides_RuleOfThirds
    };
    typedef int ImWidgetsCropGuides;

    struct ImWidgetsCropState
    {
        ImRect Rect;
        int    Active;        // -1 idle; 0..7 grip (TL,T,TR,R,BR,B,BL,L); 8 inside
        ImVec2 DragOriginMs;
        ImRect DragOriginRect;
        ImVec2 LastBoundsMin; // tracks bounds.Min from the previous frame so the
                              // rect stays put when the host window scrolls;
                              // set to (FLT_MAX, FLT_MAX) to mean "uninitialised"
        float  LastAspect;    // last aspect_ratio the rect was snapped to; when the
                              // caller changes the constraint the rect is
                              // re-materialised to it (0 = none applied yet)
    };

    IMGUI_API bool CropRect( char const* id_str, ImWidgetsCropState& s, ImRect bounds,
                              float aspect_ratio = 0.0f,             // 0 = free; >0 = width/height locked
                              ImWidgetsCropGuides guides = ImWidgetsCropGuides_Default,
                              float grip_size = 6.0f,
                              ImU32 mask_col  = IM_COL32(  0,  0,  0, 140 ),
                              ImU32 line_col  = IM_COL32( 255, 255, 255, 220 ),
                              ImU32 grip_col  = IM_COL32( 255, 255, 255, 255 ),
                              ImU32 guide_col = IM_COL32( 255, 255, 255,  90 ) );

    //////////////////////////////////////////////////////////////////////////
    // Size / Resize control with an aspect-ratio lock.
    //
    // A compact dimension editor: an interactive proxy (a rectangle in 2D, an
    // isometric box in 3D) with drag handles, one numeric drag field per axis,
    // and a padlock button. While the lock is engaged, editing any one
    // dimension -- by dragging a handle or a field -- scales the others by the
    // same factor so every ratio is preserved (works identically for W:H and
    // W:H:D).
    //
    // width/height[/depth] are in/out. `lock` is caller-owned so it can be read,
    // set and persisted. v_min/v_max bound every axis (v_max <= 0 = unbounded;
    // values are always kept > 0 to protect the ratio math). `size` is the proxy
    // canvas size in logical px (0 = a sensible default). `preview` chooses
    // whether the proxy is drawn and where (above / below the fields, or omitted
    // for a fields-only editor). Returns true the frame any value changed.
    //////////////////////////////////////////////////////////////////////////

    // Where the interactive proxy (rectangle / isometric box) sits relative to
    // the numeric fields, or whether it is drawn at all.
    enum ImWidgetsSizePreview_
    {
        ImWidgetsSizePreview_None   = 0,   // numeric fields only, no proxy
        ImWidgetsSizePreview_Top    = 1,   // proxy above the fields (default)
        ImWidgetsSizePreview_Bottom = 2    // proxy below the fields
    };
    typedef int ImWidgetsSizePreview;

    IMGUI_API bool SizeControl2D( char const* label, float* width, float* height, bool* lock,
                                  float v_min = 0.0f, float v_max = 0.0f,
                                  char const* format = "%.1f", ImVec2 size = ImVec2( 0, 0 ),
                                  ImWidgetsSizePreview preview = ImWidgetsSizePreview_Top );
    IMGUI_API bool SizeControl3D( char const* label, float* width, float* height, float* depth, bool* lock,
                                  float v_min = 0.0f, float v_max = 0.0f,
                                  char const* format = "%.1f", ImVec2 size = ImVec2( 0, 0 ),
                                  ImWidgetsSizePreview preview = ImWidgetsSizePreview_Top );

    //////////////////////////////////////////////////////////////////////////
    // Snap lines / smart guides: given a moving rect and a list of static
    // candidate rects, compute the snap delta on each axis and (optionally)
    // draw extended alignment guide lines for matched edges.
    //////////////////////////////////////////////////////////////////////////
    enum ImWidgetsSnapFlags_
    {
        ImWidgetsSnapFlags_None       = 0,
        ImWidgetsSnapFlags_LeftEdge   = 1 << 0,
        ImWidgetsSnapFlags_RightEdge  = 1 << 1,
        ImWidgetsSnapFlags_TopEdge    = 1 << 2,
        ImWidgetsSnapFlags_BottomEdge = 1 << 3,
        ImWidgetsSnapFlags_CenterH    = 1 << 4,
        ImWidgetsSnapFlags_CenterV    = 1 << 5,
        ImWidgetsSnapFlags_Edges      = 0xF,
        ImWidgetsSnapFlags_Centers    = ImWidgetsSnapFlags_CenterH | ImWidgetsSnapFlags_CenterV,
        ImWidgetsSnapFlags_All        = 0x3F
    };
    typedef int ImWidgetsSnapFlags;

    // Returns the delta you should add to `moving` (Min and Max) to snap it.
    // Returns (0,0) if nothing snapped.
    IMGUI_API ImVec2 ComputeSnapAndDraw( ImDrawList* draw,
                                          ImRect moving,
                                          ImRect const* targets, int target_count,
                                          float snap_radius_px = 6.0f,
                                          ImWidgetsSnapFlags flags = ImWidgetsSnapFlags_All,
                                          ImU32 guide_col = IM_COL32( 255,  80, 220, 220 ),
                                          float guide_extend_px = 16.0f );

    //////////////////////////////////////////////////////////////////////////
    // Onion-skin overlay: ghost back/forward frames over a current frame.
    // Per-frame variant lets the caller author each ghost; Auto variant
    // attenuates alpha by 0.5^distance from the current index.
    //////////////////////////////////////////////////////////////////////////
    struct ImWidgetsOnionFrame
    {
        ImTextureID Tex;
        ImVec2      UV0, UV1;
        ImU32       Tint;   // RGBA; A is multiplied with `Alpha` below
        float       Alpha;  // 0..1
    };

    IMGUI_API void DrawOnionSkin( ImDrawList* draw, ImRect bounds,
                                   ImWidgetsOnionFrame const* prev, int prev_count,
                                   ImWidgetsOnionFrame const* next, int next_count,
                                   ImTextureID current_tex,
                                   ImVec2 cur_uv0 = ImVec2( 0, 0 ),
                                   ImVec2 cur_uv1 = ImVec2( 1, 1 ) );

    IMGUI_API void DrawOnionSkinAuto( ImDrawList* draw, ImRect bounds,
                                       ImTextureID const* frames, int frame_count,
                                       int current_idx,
                                       int back = 1, int forward = 1,
                                       ImU32 back_tint    = IM_COL32(  80, 160, 255, 255 ),
                                       ImU32 forward_tint = IM_COL32( 255, 100,  80, 255 ),
                                       float base_alpha = 0.4f );

    //////////////////////////////////////////////////////////////////////////
    // Lasso & marquee selection.
    //////////////////////////////////////////////////////////////////////////
    enum ImWidgetsSelectionMode_
    {
        ImWidgetsSelectionMode_Marquee = 0,
        ImWidgetsSelectionMode_Lasso   = 1
    };
    typedef int ImWidgetsSelectionMode;

    struct ImWidgetsSelectionState
    {
        ImWidgetsSelectionMode Mode;
        ImVector<ImVec2>       Path;     // Marquee: Path[0]=anchor, Path[1]=cursor. Lasso: drag polyline.
        bool                   Active;   // currently dragging
        bool                   Closed;   // user released this frame; selection is finalized
    };

    // Begin a selection over `bounds`. The InvisibleButton is placed under any
    // visual content the caller draws after this call. Returns true if a
    // selection is either active or finalized this frame.
    IMGUI_API bool BeginSelection( char const* id_str, ImRect bounds,
                                    ImWidgetsSelectionState& s,
                                    ImU32 fill_col = IM_COL32(  80, 160, 255,  60 ),
                                    ImU32 line_col = IM_COL32(  80, 160, 255, 220 ),
                                    float thickness = 1.0f );

    IMGUI_API void TestSelectionPoints( ImWidgetsSelectionState const& s,
                                         ImVec2 const* pts, int count,
                                         bool* inside_out );

    IMGUI_API void TestSelectionRects( ImWidgetsSelectionState const& s,
                                        ImRect const* rects, int count,
                                        bool* inside_out,
                                        bool fully_contained_only = false );

    //////////////////////////////////////////////////////////////////////////
    // Pan-zoom canvas + minimap.
    //   - BeginCanvas paints background + dotted grid + clips drawing to the
    //     viewport. The caller draws content using CanvasWorldToScreen().
    //   - Pan: middle drag, or Shift+left drag.
    //   - Zoom: mouse wheel (anchored to cursor).
    //   - DrawCanvasMinimap shows the viewport rect within a content extent
    //     and lets the user drag it to re-center.
    //////////////////////////////////////////////////////////////////////////
    struct ImWidgetsCanvasState
    {
        ImVec2 Pan;             // screen-space offset for world origin
        float  Zoom;            // screen pixels per world unit
        bool   PanActive;
        ImVec2 PanLastMouse;
        ImRect Bounds;          // last-known screen-space viewport (filled by BeginCanvas)
    };

    IMGUI_API bool BeginCanvas( char const* id_str, ImVec2 size, ImWidgetsCanvasState& s,
                                 ImU32 bg_col       = IM_COL32(  30,  30,  30, 255 ),
                                 ImU32 grid_col     = IM_COL32(  80,  80,  80, 110 ),
                                 float grid_step_world = 32.0f );
    IMGUI_API void EndCanvas();

    IMGUI_API ImVec2 CanvasWorldToScreen( ImWidgetsCanvasState const& s, ImVec2 w );
    IMGUI_API ImVec2 CanvasScreenToWorld( ImWidgetsCanvasState const& s, ImVec2 p );
    IMGUI_API ImRect CanvasViewportWorld( ImWidgetsCanvasState const& s );

    IMGUI_API void DrawCanvasMinimap( ImWidgetsCanvasState& s,
                                       ImRect world_content,
                                       ImVec2 minimap_size = ImVec2( 160, 110 ),
                                       ImWidgetsCornerAnchor anchor = ImWidgetsCornerAnchor_TopRight,
                                       float margin_px = 8.0f,
                                       ImU32 bg_col       = IM_COL32(  20,  20,  20, 200 ),
                                       ImU32 content_col  = IM_COL32(  80,  80,  80, 200 ),
                                       ImU32 viewport_col = IM_COL32( 255, 200,  60, 230 ) );


    //////////////////////////////////////////////////////////////////////////
    // Crosshair / reticle primitive. Sized by `radius` (outer extent in px).
    //////////////////////////////////////////////////////////////////////////
    enum ImWidgetsCrosshairStyle_
    {
        ImWidgetsCrosshairStyle_Plus        = 0,  // simple + going through center
        ImWidgetsCrosshairStyle_GappedPlus  = 1,  // + with center gap
        ImWidgetsCrosshairStyle_MilDot      = 2,  // GappedPlus with mil-dots along arms
        ImWidgetsCrosshairStyle_T           = 3,  // top + horizontal (no bottom arm)
        ImWidgetsCrosshairStyle_Dot         = 4,  // single dot only
        ImWidgetsCrosshairStyle_CircleDot   = 5,  // outer circle + center dot + gapped plus
        ImWidgetsCrosshairStyle_COUNT
    };
    typedef int ImWidgetsCrosshairStyle;

    IMGUI_API char const* GetCrosshairStyleName( ImWidgetsCrosshairStyle style );

    IMGUI_API void DrawCrosshair( ImDrawList* draw, ImVec2 center, float radius,
                                   ImWidgetsCrosshairStyle style,
                                   ImU32 col, float thickness = 1.0f,
                                   float gap_radius = 0.0f );

    // Full-rect crosshair: arms extend to the edges of `bounds` through `center`.
    // Used by color pickers and similar widgets that previously inlined this.
    IMGUI_API void DrawCrosshairInRect( ImDrawList* draw, ImVec2 center, ImRect bounds,
                                         ImU32 col, float thickness = 1.0f );

    //////////////////////////////////////////////////////////////////////////
    // Soft drop-shadow / inner-glow on a rect (stacked-alpha approximation;
    // no shader). `radius` is the spread of the falloff in px.
    //////////////////////////////////////////////////////////////////////////
    IMGUI_API void DrawDropShadowRect( ImDrawList* draw, ImRect r,
                                        float radius, ImVec2 offset,
                                        ImU32 col, float corner_round = 0.0f );

    IMGUI_API void DrawInnerGlowRect( ImDrawList* draw, ImRect r,
                                       float radius, ImU32 col, float corner_round = 0.0f );

    //////////////////////////////////////////////////////////////////////////
    // Rulers (top / bottom / left / right). Delegates ticks to
    // DrawLinearLineGraduation and draws numeric labels at major ticks.
    // Optional `p_mouse_world` indicates where the cursor sits on the axis;
    // when non-null the ruler draws a thin tracker line.
    //////////////////////////////////////////////////////////////////////////
    enum ImWidgetsRulerOrient_
    {
        ImWidgetsRulerOrient_Top    = 0,  // ticks point down  (ruler sits above content)
        ImWidgetsRulerOrient_Bottom = 1,  // ticks point up
        ImWidgetsRulerOrient_Left   = 2,  // ticks point right (ruler sits left of content)
        ImWidgetsRulerOrient_Right  = 3,
        ImWidgetsRulerOrient_COUNT
    };
    typedef int ImWidgetsRulerOrient;

    IMGUI_API void DrawRuler( ImDrawList* draw, ImRect bounds, ImWidgetsRulerOrient orient,
                               float world_min, float world_max,
                               float major_step, int minor_subdivs,
                               char const* label_format = "%g",
                               ImU32 line_col  = IM_COL32( 200, 200, 200, 220 ),
                               ImU32 label_col = IM_COL32( 230, 230, 230, 255 ),
                               float major_tick_height = 10.0f,
                               float minor_tick_height = 5.0f,
                               float thickness = 1.0f,
                               float const* p_mouse_world = NULL,
                               ImU32 tracker_col = IM_COL32( 255, 200,  60, 230 ),
                               ImFont* font = NULL, float font_size = 0.0f );

    //////////////////////////////////////////////////////////////////////////
    // Grid overlay (lines or dots, optional minor subdivision + origin highlight).
    //////////////////////////////////////////////////////////////////////////
    enum ImWidgetsGridFlags_
    {
        ImWidgetsGridFlags_None       = 0,
        ImWidgetsGridFlags_Major      = 1 << 0,
        ImWidgetsGridFlags_Minor      = 1 << 1,
        ImWidgetsGridFlags_Origin     = 1 << 2,  // emphasize the line(s) through `origin`
        ImWidgetsGridFlags_Dots       = 1 << 3,  // draw dots instead of lines
        ImWidgetsGridFlags_Default    = ImWidgetsGridFlags_Major | ImWidgetsGridFlags_Minor | ImWidgetsGridFlags_Origin
    };
    typedef int ImWidgetsGridFlags;

    IMGUI_API void DrawGridOverlay( ImDrawList* draw, ImRect bounds, ImVec2 origin,
                                     float major_step, int minor_subdivs,
                                     ImU32 major_col  = IM_COL32( 100, 100, 100, 180 ),
                                     ImU32 minor_col  = IM_COL32(  60,  60,  60, 120 ),
                                     ImU32 origin_col = IM_COL32( 220, 220, 220, 220 ),
                                     ImWidgetsGridFlags flags = ImWidgetsGridFlags_Default,
                                     float thickness = 1.0f );

    //////////////////////////////////////////////////////////////////////////
    // Gradient mesh editor (NxM control grid; bilinear color interpolation).
    //////////////////////////////////////////////////////////////////////////
    struct ImWidgetsGradientMeshNode { ImVec2 PosUV; ImU32 Color; };

    struct ImWidgetsGradientMesh
    {
        int   CountX;
        int   CountY;
        ImVector<ImWidgetsGradientMeshNode> Nodes;   // size CountX*CountY, row-major
        int   SelectedNode;                          // -1 idle
        bool  ShowMesh;
    };

    IMGUI_API void GradientMeshInit( ImWidgetsGradientMesh& m, int nx, int ny,
                                      ImU32 c00, ImU32 c10, ImU32 c01, ImU32 c11 );

    IMGUI_API bool GradientMeshEditor( char const* id_str, ImWidgetsGradientMesh& m,
                                        ImVec2 size, int sampleX = 32, int sampleY = 32 );

    //////////////////////////////////////////////////////////////////////////
    // Toon / cel ramp editor.
    //  - N bands ordered by Pos in [0,1].
    //  - Softness is the [0..1] blend width to the next band (0 = hard step).
    //////////////////////////////////////////////////////////////////////////
    struct ImWidgetsToonBand { float Pos; ImU32 Color; float Softness; };

    struct ImWidgetsToonRamp
    {
        ImVector<ImWidgetsToonBand> Bands;
        int  SelectedBand;
    };

    IMGUI_API bool  ToonRampEditor( char const* id_str, ImWidgetsToonRamp& r, ImVec2 size );
    IMGUI_API ImU32 ToonRampSample( ImWidgetsToonRamp const& r, float t01 );

    //////////////////////////////////////////////////////////////////////////
    // Matrix editor (DragFloat NxM in a table).
    //////////////////////////////////////////////////////////////////////////
    IMGUI_API bool MatrixEditor( char const* label, float* data, int rows, int cols,
                                  float speed = 0.1f, char const* format = "%.3f" );

    IMGUI_API bool MatrixEditorDouble( char const* label, double* data, int rows, int cols,
                                        float speed = 0.1f, char const* format = "%.3f" );

    //////////////////////////////////////////////////////////////////////////
    // Typography: text on path, font waterfall, var-axis sliders, kerning
    // pair editor, text inside rect / convex shape (basic word-wrap).
    //////////////////////////////////////////////////////////////////////////
    IMGUI_API void DrawTextOnPath( ImDrawList* draw, ImFont* font, float font_size,
                                    char const* text,
                                    ImVec2 const* path_pts, int path_count,
                                    ImU32 col,
                                    float offset_along = 0.0f,
                                    bool  baseline_above = false );

    IMGUI_API float DrawFontWaterfall( ImDrawList* draw, ImFont* font, ImVec2 pos,
                                        char const* text,
                                        float const* sizes, int size_count,
                                        ImU32 col, float gap = 4.0f );

    struct ImWidgetsVarFontAxis
    {
        char const* Tag;       // e.g., "wght"
        char const* Name;      // human label
        float Min, Max, Default;
        float Value;
    };

    IMGUI_API bool VarFontAxisSliders( char const* id_str,
                                        ImWidgetsVarFontAxis* axes, int axis_count,
                                        ImFont* preview_font = NULL,
                                        char const* preview_text = "Aa Bb Cc 0123" );

    IMGUI_API bool KerningPairEditor( char const* id_str, ImFont* font, float font_size,
                                       char left_glyph, char right_glyph,
                                       float* p_kern_px,
                                       ImVec2 size = ImVec2( 0, 0 ) );

    IMGUI_API void DrawTextInsideRect( ImDrawList* draw, ImFont* font, float font_size,
                                        ImRect bounds, char const* text, ImU32 col );

    IMGUI_API void DrawTextInsideConvex( ImDrawList* draw, ImFont* font, float font_size,
                                          ImVec2 const* convex_poly, int poly_count,
                                          char const* text, ImU32 col );

    //////////////////////////////////////////////////////////////////////////
    // GradientDrop: click-drag-collect colors into a gradient.
    //   - The user click-drags across `screen_rect`. Each ~`min_step_px` of
    //     cursor travel adds a sample (cursor pos + color from `sample_fn`).
    //   - While dragging the path is drawn live with the sampled colors so
    //     you can preview the gradient as you go.
    //   - On mouse release the raw samples are reduced via Douglas-Peucker
    //     simplification in OkLab (perceptual Î”E), then trimmed to at most
    //     `max_stops` by dropping the lowest-impact interior stops. Arc
    //     length along the stroke becomes the gradient Position (0..1).
    //   - Returns true exactly on the frame the gradient is finalized.
    //
    // State is caller-owned (so the path can persist between frames and
    // multiple GradientDrop widgets can coexist).
    //////////////////////////////////////////////////////////////////////////
    struct ImWidgetsGradientDropState
    {
        ImVector<ImVec2> Path;     // sample positions stored as UV in [0..1] inside screen_rect;
                                   //   the stroke stays attached to the source content even if the
                                   //   host window scrolls or resizes
        ImVector<ImU32>  Colors;   // sampled color per Path point (parallel arrays)
        bool             Active;   // true while user is dragging
    };

    IMGUI_API bool GradientDrop( char const* id_str, ImRect screen_rect,
                                  ImWidgetsEyedropperSampleFn sample_fn, void* user_data,
                                  ImWidgetsGradientDropState& state,
                                  ImGradientData* out_gradient,
                                  int   max_stops              = 8,
                                  float perceptual_threshold   = 0.04f,  // OkLab Î”E
                                  float min_step_px            = 3.0f,
                                  float path_thickness         = 3.0f,
                                  bool  show_live_preview      = true );
}

