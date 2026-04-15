#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>

#include <imgui_internal.h>

//#include <algorithm>
//#include <string>
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

//////////////////////////////////////////////////////////////////////////
// Style TODO:
//	* Expose Style
//	* Line Thickness for Slider2D
//	* Line Color for Slider2D
//
// Known issue:
//	* Slider2DInt must take into account the half pixel
//	* Move 2D must store state "IsDrag"
//
// Optim TODO:
//	* ChromaticPlot draw only the internal MultiColorQuad where its needed "inside" or at least leave the option for style "transparency etc"
//	* ChromaticPlot: Bake some as much as possible values: provide different version: ChromaticPlotDynamic {From enum and compute info at each frame}, ChromaticPlotFromData {From Baked data}
//
// Write use case for:
//	* ColorRing:
//		- HDR Color Management {Shadow, MidTone, Highlight}
//	* Grid2D_AoS_Float:
//		- Color Remap:
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Perf TODO:
//		Cache the triangulation for concave shape/poly
//		Find a way to cache shape with hole too
// Create "Shape" struct
//////////////////////////////////////////////////////////////////////////

#if !__cpp_if_constexpr
#define constexpr
#endif

#if 0
#define nullptr NULL
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
		return a == e.a && b == e.a;
	}
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
		return ( ( const ImDrawIdx* )( const void* )( const char* )this )[ idx ];
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
	//ImWidgetsVertexLine*			vertices;
	//int vertices_count;
	ImVector<ImWidgetsTriIdx>		triangles;
	//ImDrawIdx*						triangles;
	//int triangles_count;
	float total_length;
	ImRect							bb;
};

struct ImWidgetsShapeCacheEntry
{
	ImU64				key;
	ImWidgetsShape*		shape;  // Use pointer to avoid shallow copy issues with ImVector
};

struct ImWidgetsShapeCache
{
	ImVector<ImWidgetsShapeCacheEntry>	entries;
	ImVector<int>                       map;    // S-2: open-addressed hash map, slot → entries index+1 (0=empty)
};

typedef ImU32( *ImWidgetsColor1DCallback )( float x, void* );
typedef ImU32( *ImWidgetsColor2DCallback )( float x, float y, void* );

// Opaque Slug font rendering state (defined in dear_widgets.cpp, inside namespace ImWidgets)
namespace ImWidgets { struct ImWidgetsSlugState; }

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

	// Background blur / effects
	ImDrawShader					blurShader;
	ImTextureID						blurBackbufferCopy;
	ImTextureID						blurIntermediate;
	unsigned int					blurTexW, blurTexH;
};

enum ImWidgetsStyleColor
{
	StyleColor_Value,
	StyleColor_Slider2D_CursorX,    // Color for Slider2D X-axis cursor
	StyleColor_Slider2D_CursorY,    // Color for Slider2D Y-axis cursor

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
	StyleVar_ColorWheel_DiscSectors,
	StyleVar_ColorWheel_DiscRings,
	StyleVar_ColorWheel_SliderHeight,

	// Color Warper
	StyleVar_ColorWarper_PointRadius,
	StyleVar_ColorWarper_HitRadius,
	StyleVar_ColorWarper_GridThickness,
	StyleVar_ColorWarper_DiscSectors,
	StyleVar_ColorWarper_DiscRings,

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

	// Tone Curve
	StyleVar_ToneCurve_DefaultHeight,
	StyleVar_ToneCurve_KeyRadius,
	StyleVar_ToneCurve_LineThickness,
	StyleVar_ToneCurve_GradMarginLeft,
	StyleVar_ToneCurve_GradMarginBottom,
	StyleVar_ToneCurve_BandThickness,
	StyleVar_ToneCurve_BandGap,

	// Color Picker
	StyleVar_ColorPicker_DotRadius,
	StyleVar_ColorPicker_PlaneResolution,
	StyleVar_ColorPicker_SliderWidth,
	StyleVar_ColorPicker_SliderResolution,
	StyleVar_ColorPicker_ComponentSliderHeight,

	// Hatching
	StyleVar_Hatch_DefaultSpacing,
	StyleVar_Hatch_DefaultThickness,
	// Bezier curve editor
	StyleVar_BezierCurve_KeyRadius,
	StyleVar_BezierCurve_TangentRadius,
	StyleVar_BezierCurve_LineThickness,
	StyleVar_BezierCurve_SnapAngleDeg,
	// Envelope
	StyleVar_Envelope_StageHandleRadius,
	StyleVar_Envelope_GuideLineAlpha,
	StyleVar_Envelope_DefaultHeight,
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

	StyleVar_Count
};

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

	// SliderRing
	float	SliderRing_TrackThickness;			// Track arc thickness (px)
	float	SliderRing_GrabRadius;				// Grab handle radius (px)

	// SliderSpline
	float	SliderSpline_TrackThickness;		// Spline track thickness (px)
	float	SliderSpline_GrabRadius;			// Grab handle radius (px)

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

	// Color Wheel
	float	ColorWheel_DotRadius;
	float	ColorWheel_RingThickness;
	float	ColorWheel_DiscSectors;				// Disc angular resolution
	float	ColorWheel_DiscRings;				// Disc radial resolution
	float	ColorWheel_SliderHeight;			// Master slider height (px)

	// Color Warper
	float	ColorWarper_PointRadius;			// Control point dot radius (px)
	float	ColorWarper_HitRadius;				// Hit testing radius for points (px)
	float	ColorWarper_GridThickness;			// Mesh grid line thickness (px)
	float	ColorWarper_DiscSectors;			// Background disc angular resolution
	float	ColorWarper_DiscRings;				// Background disc radial resolution

	// Color Curve
	float	ColorCurve_DefaultHeight;
	float	ColorCurve_KeyRadius;
	float	ColorCurve_LineThickness;

	// Parade Scope
	float	ParadeScope_DefaultHeight;
	float	ParadeScope_OverlayAlpha;		// Max alpha in overlay mode (0..1)
	float	ParadeScope_GradTickLength;		// Graduation tick mark length (px)
	float	ParadeScope_GradTickThickness;	// Graduation tick line thickness
	float	ParadeScope_GradMargin;			// Left margin for graduation labels (px)

	// Vector Scope
	float	VectorScope_DefaultSize;		// Default square side length (px)
	float	VectorScope_SignalAlpha;		// Max alpha for signal dots (0..1)
	float	VectorScope_GraticuleThickness;	// Graticule line thickness (px)

	// Histogram
	float	Histogram_DefaultHeight;		// Default widget height (px)
	float	Histogram_OverlayAlpha;			// Max alpha in overlay mode (0..1)
	float	Histogram_GradTickLength;		// Graduation tick mark length (px)
	float	Histogram_GradTickThickness;	// Graduation tick line thickness
	float	Histogram_GradMarginLeft;		// Left margin for Y-axis labels (px)
	float	Histogram_GradMarginBottom;		// Bottom margin for X-axis labels (px)

	// CIE Chromaticity
	float	CIEChromaticity_DefaultSize;		// Default square side (px)
	float	CIEChromaticity_SignalRadius;		// Signal dot radius (px)
	float	CIEChromaticity_GamutLineThickness;	// Gamut triangle line thickness (px)
	float	CIEChromaticity_WhitePointRadius;	// White point marker radius (px)
	float	CIEChromaticity_GradMargin;			// Margin for graduation labels (px)
	float	CIEChromaticity_SignalAlpha;			// Signal point alpha (0..1)

	// Tone Curve
	float	ToneCurve_DefaultHeight;			// Default widget height (px)
	float	ToneCurve_KeyRadius;				// Key dot radius (px)
	float	ToneCurve_LineThickness;			// Curve line thickness (px)
	float	ToneCurve_GradMarginLeft;			// Left margin for Y-axis labels (px)
	float	ToneCurve_GradMarginBottom;			// Bottom margin for X-axis labels (px)
	float	ToneCurve_BandThickness;			// Gradient band thickness (px)
	float	ToneCurve_BandGap;					// Gap between scope and band (px)

	// HDR Wheel
	float	HDRWheel_RightArcStart;				// Right arc start angle in degrees (0=right, CW)
	float	HDRWheel_RightArcEnd;				// Right arc end angle in degrees
	float	HDRWheel_LeftArcStart;				// Left arc start angle in degrees
	float	HDRWheel_LeftArcEnd;				// Left arc end angle in degrees
	float	HDRWheel_ArcGap;					// Gap between indicator ring and arc sliders (px)
	float	HDRWheel_ArcThickness;				// Arc slider track thickness (px)
	float	HDRWheel_ArcGrabRadius;				// Arc slider grab handle radius (px)

	// Transform Gizmo
	float	Gizmo_HandleSize;					// Corner handle half-size (px)
	float	Gizmo_RotationHandleOffset;			// Distance from top edge to rotation handle (px)
	float	Gizmo_OutlineThickness;				// Bounding box outline thickness (px)

	// Color Picker
	float	ColorPicker_DotRadius;				// Dot indicator radius (px)
	float	ColorPicker_PlaneResolution;		// 2D plane grid resolution per axis
	float	ColorPicker_SliderWidth;			// Vertical slider width (px)
	float	ColorPicker_SliderResolution;		// Vertical slider segment count
	float	ColorPicker_ComponentSliderHeight;	// Component slider height (px)

	// Hatching
	float	Hatch_DefaultSpacing;
	float	Hatch_DefaultThickness;
	// Bezier curve editor (new)
	float	BezierCurve_KeyRadius;
	float	BezierCurve_TangentRadius;
	float	BezierCurve_LineThickness;
	float	BezierCurve_SnapAngleDeg;
	// Envelope editor
	float	Envelope_StageHandleRadius;
	float	Envelope_GuideLineAlpha;
	float	Envelope_DefaultHeight;
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

	ImVec4  Colors[ StyleColor_Count ];

	ImWidgetsStyle()
	{
		HueSelector_Thickness_ZeroWidth = 2.0f;

		Slider2D_DragThickness   = 8.0f;
		Slider2D_BorderThickness = 2.0f;
		Slider2D_LineThickness   = 2.0f;
		Slider2D_CursorRadius    = 4.0f;
		Slider2D_CursorOffset    = 16.0f;
		Slider2D_CornerRadius    = 2.0f;

		NavCursor_Thickness      = 2.0f;
		NavCursor_Distance       = 3.0f;
		WhitePoint_Radius        = 5.0f;
		PrecisionDrag_BlockSize  = 28.0f;

		// Gradient Editor
		Gradient_MarkerHeight        = 12.0f;
		Gradient_CheckerboardCellSize = 6.0f;
		Gradient_MarkerThickness     = 1.0f;

		// Curve Editor
		CurveEditor_DefaultHeight    = 200.0f;
		CurveEditor_KeyRadius        = 5.0f;
		CurveEditor_TangentRadius    = 4.0f;
		CurveEditor_HitRadius        = 8.0f;
		CurveEditor_LineThickness    = 2.0f;
		CurveEditor_KeyOutlineThickness = 1.5f;
		CurveEditor_ZeroLineThickness = 1.0f;

		// Color Wheel
		ColorWheel_DotRadius     = 6.0f;
		ColorWheel_RingThickness = 12.0f;
		ColorWheel_DiscSectors   = 96.0f;
		ColorWheel_DiscRings     = 24.0f;
		ColorWheel_SliderHeight  = 20.0f;

		// Slider Ring
		SliderRing_TrackThickness = 8.0f;
		SliderRing_GrabRadius     = 7.0f;

		// Slider Spline
		SliderSpline_TrackThickness = 4.0f;
		SliderSpline_GrabRadius     = 7.0f;

		// Color Warper
		ColorWarper_PointRadius  = 4.0f;
		ColorWarper_HitRadius    = 8.0f;
		ColorWarper_GridThickness = 1.0f;
		ColorWarper_DiscSectors  = 96.0f;
		ColorWarper_DiscRings    = 24.0f;

		// Color Curve
		ColorCurve_DefaultHeight = 150.0f;
		ColorCurve_KeyRadius     = 5.0f;
		ColorCurve_LineThickness = 2.0f;

		// Parade Scope
		ParadeScope_DefaultHeight    = 200.0f;
		ParadeScope_OverlayAlpha     = 0.6f;
		ParadeScope_GradTickLength   = 6.0f;
		ParadeScope_GradTickThickness = 1.0f;
		ParadeScope_GradMargin       = 40.0f;

		// Vector Scope
		VectorScope_DefaultSize        = 200.0f;
		VectorScope_SignalAlpha        = 0.8f;
		VectorScope_GraticuleThickness = 1.0f;

		// Histogram
		Histogram_DefaultHeight      = 200.0f;
		Histogram_OverlayAlpha       = 0.6f;
		Histogram_GradTickLength     = 6.0f;
		Histogram_GradTickThickness  = 1.0f;
		Histogram_GradMarginLeft     = 40.0f;
		Histogram_GradMarginBottom   = 20.0f;

		// CIE Chromaticity
		CIEChromaticity_DefaultSize        = 300.0f;
		CIEChromaticity_SignalRadius       = 1.5f;
		CIEChromaticity_GamutLineThickness = 1.5f;
		CIEChromaticity_WhitePointRadius   = 4.0f;
		CIEChromaticity_GradMargin         = 30.0f;
		CIEChromaticity_SignalAlpha        = 0.6f;

		// Tone Curve
		ToneCurve_DefaultHeight     = 200.0f;
		ToneCurve_KeyRadius         = 5.0f;
		ToneCurve_LineThickness     = 2.0f;
		ToneCurve_GradMarginLeft    = 30.0f;
		ToneCurve_GradMarginBottom  = 20.0f;
		ToneCurve_BandThickness     = 8.0f;
		ToneCurve_BandGap           = 2.0f;

		// HDR Wheel
		HDRWheel_RightArcStart   = -45.0f;
		HDRWheel_RightArcEnd     = 45.0f;
		HDRWheel_LeftArcStart    = 135.0f;
		HDRWheel_LeftArcEnd      = 225.0f;
		HDRWheel_ArcGap          = 4.0f;
		HDRWheel_ArcThickness    = 8.0f;
		HDRWheel_ArcGrabRadius   = 7.0f;

		// Transform Gizmo
		Gizmo_HandleSize              = 5.0f;
		Gizmo_RotationHandleOffset    = 25.0f;
		Gizmo_OutlineThickness        = 1.5f;

		Colors[ StyleColor_Value ] = ImVec4( 1.0f, 0.0f, 0.0f, 1.0f );
		Colors[ StyleColor_Slider2D_CursorX ] = ImVec4( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 1.0f ); // Blue
		Colors[ StyleColor_Slider2D_CursorY ] = ImVec4( 255.0f / 255.0f, 128.0f / 255.0f, 64.0f / 255.0f, 1.0f ); // Orange

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

		// Color Picker Colors
		Colors[ StyleColor_ColorPicker_DotOutline ]        = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
		Colors[ StyleColor_ColorPicker_DotOutlineActive ]  = ImVec4( 1.0f, 1.0f, 0.0f, 1.0f );
		Colors[ StyleColor_ColorPicker_Crosshair ]         = ImVec4( 1.0f, 1.0f, 1.0f, 40.0f / 255.0f );
		Colors[ StyleColor_ColorPicker_SliderOutline ]     = ImVec4( 0.0f, 0.0f, 0.0f, 150.0f / 255.0f );
		Colors[ StyleColor_ColorPicker_SliderHandle ]      = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );

		// Color Picker Vars
		ColorPicker_DotRadius            = 6.0f;
		ColorPicker_PlaneResolution      = 24.0f;
		ColorPicker_SliderWidth          = 20.0f;
		ColorPicker_SliderResolution     = 16.0f;
		ColorPicker_ComponentSliderHeight = 16.0f;

		// Hatching
		Hatch_DefaultSpacing      = 8.0f;
		Hatch_DefaultThickness    = 1.0f;
		// Bezier curve editor
		BezierCurve_KeyRadius     = 5.0f;
		BezierCurve_TangentRadius = 4.0f;
		BezierCurve_LineThickness = 2.0f;
		BezierCurve_SnapAngleDeg  = 5.0f;
		// Envelope editor
		Envelope_StageHandleRadius = 5.0f;
		Envelope_GuideLineAlpha    = 0.25f;
		Envelope_DefaultHeight     = 160.0f;
		// Font inspector
		FontInspector_GlyphCell   = 72.0f;
		FontInspector_MetricAlpha = 0.6f;
		// Notched dial
		NotchedDial_TickLength    = 6.0f;
		NotchedDial_TickThickness = 1.0f;
		// Delta-E
		DeltaE_SwatchSize         = 48.0f;
		// Equation input
		Equation_BlockPadding         = 8.0f;
		Equation_InlineBaselineOffset = 0.0f;
	}

	void ScaleAllSizes( float scale_factor )
	{
		HueSelector_Thickness_ZeroWidth = ImTrunc( HueSelector_Thickness_ZeroWidth * scale_factor );
		Slider2D_DragThickness   = ImTrunc( Slider2D_DragThickness * scale_factor );
		Slider2D_BorderThickness = ImTrunc( Slider2D_BorderThickness * scale_factor );
		Slider2D_LineThickness   = ImTrunc( Slider2D_LineThickness * scale_factor );
		Slider2D_CursorRadius    = ImTrunc( Slider2D_CursorRadius * scale_factor );
		Slider2D_CursorOffset    = ImTrunc( Slider2D_CursorOffset * scale_factor );
		Slider2D_CornerRadius    = ImTrunc( Slider2D_CornerRadius * scale_factor );
		NavCursor_Thickness      = ImTrunc( NavCursor_Thickness * scale_factor );
		NavCursor_Distance       = ImTrunc( NavCursor_Distance * scale_factor );
		WhitePoint_Radius        = ImTrunc( WhitePoint_Radius * scale_factor );
		PrecisionDrag_BlockSize  = ImTrunc( PrecisionDrag_BlockSize * scale_factor );

		Gradient_MarkerHeight         = ImTrunc( Gradient_MarkerHeight * scale_factor );
		Gradient_CheckerboardCellSize = ImTrunc( Gradient_CheckerboardCellSize * scale_factor );
		Gradient_MarkerThickness      = ImTrunc( Gradient_MarkerThickness * scale_factor );

		CurveEditor_DefaultHeight     = ImTrunc( CurveEditor_DefaultHeight * scale_factor );
		CurveEditor_KeyRadius         = ImTrunc( CurveEditor_KeyRadius * scale_factor );
		CurveEditor_TangentRadius     = ImTrunc( CurveEditor_TangentRadius * scale_factor );
		CurveEditor_HitRadius         = ImTrunc( CurveEditor_HitRadius * scale_factor );
		CurveEditor_LineThickness     = ImTrunc( CurveEditor_LineThickness * scale_factor );
		CurveEditor_KeyOutlineThickness = ImTrunc( CurveEditor_KeyOutlineThickness * scale_factor );
		CurveEditor_ZeroLineThickness = ImTrunc( CurveEditor_ZeroLineThickness * scale_factor );

		ColorWheel_DotRadius     = ImTrunc( ColorWheel_DotRadius * scale_factor );
		ColorWheel_RingThickness = ImTrunc( ColorWheel_RingThickness * scale_factor );
		ColorWheel_SliderHeight  = ImTrunc( ColorWheel_SliderHeight * scale_factor );

		SliderRing_TrackThickness = ImTrunc( SliderRing_TrackThickness * scale_factor );
		SliderRing_GrabRadius     = ImTrunc( SliderRing_GrabRadius * scale_factor );

		SliderSpline_TrackThickness = ImTrunc( SliderSpline_TrackThickness * scale_factor );
		SliderSpline_GrabRadius     = ImTrunc( SliderSpline_GrabRadius * scale_factor );

		ColorWarper_PointRadius   = ImTrunc( ColorWarper_PointRadius * scale_factor );
		ColorWarper_HitRadius    = ImTrunc( ColorWarper_HitRadius * scale_factor );
		ColorWarper_GridThickness = ImTrunc( ColorWarper_GridThickness * scale_factor );

		ColorCurve_DefaultHeight = ImTrunc( ColorCurve_DefaultHeight * scale_factor );
		ColorCurve_KeyRadius     = ImTrunc( ColorCurve_KeyRadius * scale_factor );
		ColorCurve_LineThickness = ImTrunc( ColorCurve_LineThickness * scale_factor );

		ParadeScope_DefaultHeight     = ImTrunc( ParadeScope_DefaultHeight * scale_factor );
		ParadeScope_GradTickLength    = ImTrunc( ParadeScope_GradTickLength * scale_factor );
		ParadeScope_GradTickThickness = ImTrunc( ParadeScope_GradTickThickness * scale_factor );
		ParadeScope_GradMargin        = ImTrunc( ParadeScope_GradMargin * scale_factor );

		VectorScope_DefaultSize        = ImTrunc( VectorScope_DefaultSize * scale_factor );
		VectorScope_GraticuleThickness = ImTrunc( VectorScope_GraticuleThickness * scale_factor );

		Histogram_DefaultHeight      = ImTrunc( Histogram_DefaultHeight * scale_factor );
		Histogram_GradTickLength     = ImTrunc( Histogram_GradTickLength * scale_factor );
		Histogram_GradTickThickness  = ImTrunc( Histogram_GradTickThickness * scale_factor );
		Histogram_GradMarginLeft     = ImTrunc( Histogram_GradMarginLeft * scale_factor );
		Histogram_GradMarginBottom   = ImTrunc( Histogram_GradMarginBottom * scale_factor );

		CIEChromaticity_DefaultSize        = ImTrunc( CIEChromaticity_DefaultSize * scale_factor );
		CIEChromaticity_SignalRadius       = ImTrunc( CIEChromaticity_SignalRadius * scale_factor );
		CIEChromaticity_GamutLineThickness = ImTrunc( CIEChromaticity_GamutLineThickness * scale_factor );
		CIEChromaticity_WhitePointRadius   = ImTrunc( CIEChromaticity_WhitePointRadius * scale_factor );
		CIEChromaticity_GradMargin         = ImTrunc( CIEChromaticity_GradMargin * scale_factor );

		ToneCurve_DefaultHeight     = ImTrunc( ToneCurve_DefaultHeight * scale_factor );
		ToneCurve_KeyRadius         = ImTrunc( ToneCurve_KeyRadius * scale_factor );
		ToneCurve_LineThickness     = ImTrunc( ToneCurve_LineThickness * scale_factor );
		ToneCurve_GradMarginLeft    = ImTrunc( ToneCurve_GradMarginLeft * scale_factor );
		ToneCurve_GradMarginBottom  = ImTrunc( ToneCurve_GradMarginBottom * scale_factor );
		ToneCurve_BandThickness     = ImTrunc( ToneCurve_BandThickness * scale_factor );
		ToneCurve_BandGap           = ImTrunc( ToneCurve_BandGap * scale_factor );

		HDRWheel_ArcGap             = ImTrunc( HDRWheel_ArcGap * scale_factor );
		HDRWheel_ArcThickness       = ImTrunc( HDRWheel_ArcThickness * scale_factor );
		HDRWheel_ArcGrabRadius      = ImTrunc( HDRWheel_ArcGrabRadius * scale_factor );

		Gizmo_HandleSize             = ImTrunc( Gizmo_HandleSize * scale_factor );
		Gizmo_RotationHandleOffset   = ImTrunc( Gizmo_RotationHandleOffset * scale_factor );
		Gizmo_OutlineThickness       = ImTrunc( Gizmo_OutlineThickness * scale_factor );

		ColorPicker_DotRadius            = ImTrunc( ColorPicker_DotRadius * scale_factor );
		ColorPicker_SliderWidth          = ImTrunc( ColorPicker_SliderWidth * scale_factor );
		ColorPicker_ComponentSliderHeight = ImTrunc( ColorPicker_ComponentSliderHeight * scale_factor );
	}

	void PushColor( ImWidgetsStyleColor colorIndex, const ImVec4& color )
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
	void PushVar( ImWidgetsStyleVar varIndex, const ImVec2& value )
	{
		auto* var = GetVarVec2Addr( varIndex );
		IM_ASSERT( var != nullptr );
		VarModifier modifier;
		modifier.Index = varIndex;
		modifier.Value = ImVec4( var->x, var->y, 0, 0 );
		*var = value;
		m_VarStack.push_back( modifier );
	}
	void PushVar( ImWidgetsStyleVar varIndex, const ImVec4& value )
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

	const char* GetColorName( ImWidgetsStyleColor colorIndex ) const
	{
		switch ( colorIndex )
		{
		case StyleColor_Value: return "Value";
		case StyleColor_Slider2D_CursorX: return "Slider2DCursorX";
		case StyleColor_Slider2D_CursorY: return "Slider2DCursorY";
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
		case StyleVar_ColorWheel_DiscSectors:			return &ColorWheel_DiscSectors;
		case StyleVar_ColorWheel_DiscRings:				return &ColorWheel_DiscRings;
		case StyleVar_ColorWheel_SliderHeight:			return &ColorWheel_SliderHeight;
		case StyleVar_SliderRing_TrackThickness:		return &SliderRing_TrackThickness;
		case StyleVar_SliderRing_GrabRadius:			return &SliderRing_GrabRadius;
		case StyleVar_SliderSpline_TrackThickness:		return &SliderSpline_TrackThickness;
		case StyleVar_SliderSpline_GrabRadius:			return &SliderSpline_GrabRadius;
		case StyleVar_ColorWarper_PointRadius:			return &ColorWarper_PointRadius;
		case StyleVar_ColorWarper_HitRadius:			return &ColorWarper_HitRadius;
		case StyleVar_ColorWarper_GridThickness:		return &ColorWarper_GridThickness;
		case StyleVar_ColorWarper_DiscSectors:			return &ColorWarper_DiscSectors;
		case StyleVar_ColorWarper_DiscRings:			return &ColorWarper_DiscRings;
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
		case StyleVar_ToneCurve_DefaultHeight:			return &ToneCurve_DefaultHeight;
		case StyleVar_ToneCurve_KeyRadius:				return &ToneCurve_KeyRadius;
		case StyleVar_ToneCurve_LineThickness:			return &ToneCurve_LineThickness;
		case StyleVar_ToneCurve_GradMarginLeft:			return &ToneCurve_GradMarginLeft;
		case StyleVar_ToneCurve_GradMarginBottom:		return &ToneCurve_GradMarginBottom;
		case StyleVar_ToneCurve_BandThickness:			return &ToneCurve_BandThickness;
		case StyleVar_ToneCurve_BandGap:				return &ToneCurve_BandGap;
		case StyleVar_ColorPicker_DotRadius:			return &ColorPicker_DotRadius;
		case StyleVar_ColorPicker_PlaneResolution:		return &ColorPicker_PlaneResolution;
		case StyleVar_ColorPicker_SliderWidth:			return &ColorPicker_SliderWidth;
		case StyleVar_ColorPicker_SliderResolution:		return &ColorPicker_SliderResolution;
		case StyleVar_ColorPicker_ComponentSliderHeight:	return &ColorPicker_ComponentSliderHeight;
		case StyleVar_Hatch_DefaultSpacing:				return &Hatch_DefaultSpacing;
		case StyleVar_Hatch_DefaultThickness:			return &Hatch_DefaultThickness;
		case StyleVar_BezierCurve_KeyRadius:			return &BezierCurve_KeyRadius;
		case StyleVar_BezierCurve_TangentRadius:		return &BezierCurve_TangentRadius;
		case StyleVar_BezierCurve_LineThickness:		return &BezierCurve_LineThickness;
		case StyleVar_BezierCurve_SnapAngleDeg:			return &BezierCurve_SnapAngleDeg;
		case StyleVar_Envelope_StageHandleRadius:		return &Envelope_StageHandleRadius;
		case StyleVar_Envelope_GuideLineAlpha:			return &Envelope_GuideLineAlpha;
		case StyleVar_Envelope_DefaultHeight:			return &Envelope_DefaultHeight;
		case StyleVar_FontInspector_GlyphCell:			return &FontInspector_GlyphCell;
		case StyleVar_FontInspector_MetricAlpha:		return &FontInspector_MetricAlpha;
		case StyleVar_NotchedDial_TickLength:			return &NotchedDial_TickLength;
		case StyleVar_NotchedDial_TickThickness:		return &NotchedDial_TickThickness;
		case StyleVar_DeltaE_SwatchSize:				return &DeltaE_SwatchSize;
		case StyleVar_Equation_BlockPadding:			return &Equation_BlockPadding;
		case StyleVar_Equation_InlineBaselineOffset:	return &Equation_InlineBaselineOffset;
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

// Shader constant buffer for dashed line rendering.
// Layout must match HLSL cbuffer packing (no field spans a 16-byte register boundary)
// and GLSL std140 layout (vec2 aligned to 8, vec4 aligned to 16).
// Registers: [0-15] p0+p1, [16-31] thickness+aa+dash, [32-47] dash_offset+cap+join+miter_limit,
//            [48-63] rect_min+rect_max, [64-79] color.  Total: 80 bytes.
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

// Stroke fill constant buffer — winding number shader.
// Holds directed line segments ("line soup") from Euler spiral stroke expansion.
#define IMGUI_STROKE_MAX_SEGMENTS 1024
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

typedef int ImWidgetsLengthUnit;
typedef int ImWidgetsChromaticPlot;
typedef int ImWidgetsObserver;
typedef int ImWidgetsIlluminance;
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
	ImWidgetsObserverChromaticPlot_1931_2deg = 0,
	ImWidgetsObserverChromaticPlot_1964_10deg,
	ImWidgetsObserverChromaticPlot_COUNT
};

enum ImWidgetsIlluminance_
{
	// White Points
	ImWidgetsWhitePointChromaticPlot_A = 0,
	ImWidgetsWhitePointChromaticPlot_B,
	ImWidgetsWhitePointChromaticPlot_C,
	ImWidgetsWhitePointChromaticPlot_D50,
	ImWidgetsWhitePointChromaticPlot_D55,
	ImWidgetsWhitePointChromaticPlot_D65,
	ImWidgetsWhitePointChromaticPlot_D75,
	ImWidgetsWhitePointChromaticPlot_D93,
	ImWidgetsWhitePointChromaticPlot_E,
	ImWidgetsWhitePointChromaticPlot_F1,
	ImWidgetsWhitePointChromaticPlot_F2,
	ImWidgetsWhitePointChromaticPlot_F3,
	ImWidgetsWhitePointChromaticPlot_F4,
	ImWidgetsWhitePointChromaticPlot_F5,
	ImWidgetsWhitePointChromaticPlot_F6,
	ImWidgetsWhitePointChromaticPlot_F7,
	ImWidgetsWhitePointChromaticPlot_F8,
	ImWidgetsWhitePointChromaticPlot_F9,
	ImWidgetsWhitePointChromaticPlot_F10,
	ImWidgetsWhitePointChromaticPlot_F11,
	ImWidgetsWhitePointChromaticPlot_F12,
	ImWidgetsWhitePointChromaticPlot_COUNT
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
				// De Casteljau level 3 — split point
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

struct ImTransformData
{
	ImVec2	Translation;	// Offset from canvas center (px)
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
typedef void (*ImTransformGizmoDrawFn)( const ImTransformGizmoDrawParams& params, void* user_data );

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
				// De Casteljau level 3 — split point
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
	ImParadeScale_Log,				// Log2 — expands shadows / darks
	ImParadeScale_InvLog,			// Inverse log2 — expands highlights / brights
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
	ImVector<ImU32>	Bins;			// [xBin * Resolution + yBin] — 2D chrominance histogram
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

struct ImGlobalData
{
    ImWidgetsFeatures features;
    bool             dashedLinesUseGPU;
    bool             dashedLinesDebugJoins;
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
	int gap;
	int strokeWidth;
};

// Unit Field
typedef float (*ImUnitConvertCallback)(float value, void* pUserData);
struct ImUnitDef
{
	const char* name;            // Full name: "meter", "foot", "inch"
	const char* abbreviation;    // Short: "m", "ft", "in"
	float       mul;             // base_value * mul + add = display_value
	float       add;             // (for Fahrenheit: mul=9/5, add=32)
	ImUnitConvertCallback toDisplay;  // Custom: base -> display (if non-NULL, mul/add ignored)
	ImUnitConvertCallback toBase;     // Custom: display -> base
	void*       pUserData;       // Passed to callbacks
};

inline ImUnitDef ImUnitDef_Simple( const char* name, const char* abbr, float mul, float add = 0.0f )
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

inline ImUnitDef ImUnitDef_Custom( const char* name, const char* abbr, ImUnitConvertCallback toDisplay, ImUnitConvertCallback toBase, void* pUserData = NULL )
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
	const void*            Pixels;      // CPU pixel buffer
	ImVec2                 PixelSize;   // Dimensions of the Pixels buffer (should match texture)
	ImPlatform_PixelFormat PixelFormat;

	ImImageViewerState() : Zoom( 1.0f ), Pan( 0.0f, 0.0f ), Pixels( NULL ), PixelSize( 0.0f, 0.0f ), PixelFormat( ImPlatform_PixelFormat_RGBA8 ) {}
};

// ============================================================================
// ImageInspector: color-managed raw-buffer viewer
// ============================================================================
// A more advanced sibling of ImageViewer that displays any of 11 sample types ×
// 1..4 channels via an HLSL uber-shader. The user supplies an ImImageBuffer
// (Halide-style strided descriptor, defined in ImPlatform.h) and the widget
// uploads its bytes ONCE into a packed RGBA32F GPU texture (re-uploading only
// when ImImageBuffer.version changes). All view transforms (gamma, sRGB, log
// curves, gamut, exposure, white point, tonemap, false color, NaN highlight,
// mosaic decode) run in the shader; per-frame CPU cost is uniform updates only.
//
// The CPU-side inspector loupe walks the user's host pointer directly with
// full precision (double[4] + int64[4] lanes for exact integer display) — the
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

// HDR → SDR tonemap operators.
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
// not be touched directly — they're managed by the widget. Call
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

	// ---- GPU cache (managed by widget — do not touch) ----
	ImTextureID PackedTexture;
	int         PackedTexW, PackedTexH;
	ImU64       LastUploadedVersion;
	int         LastUploadedWidth, LastUploadedHeight;
	int         LastUploadedChannels;
	int         LastUploadedSampleType;
	int         GpuXStride, GpuYStride, GpuCStride;   // On-GPU strides (always tight)
	bool        BufferTooLarge;
	ImVector<unsigned char> Scratch;                  // Persistent scratch for tight-pack

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
// Audio widgets (2026-04 batch B + C)
//////////////////////////////////////////////////////////////////////////

// B1. Parametric EQ
enum ImEQBandType_
{
	ImEQBandType_Peak = 0,
	ImEQBandType_LowShelf,
	ImEQBandType_HighShelf,
	ImEQBandType_LowPass,
	ImEQBandType_HighPass,
	ImEQBandType_Notch,
	ImEQBandType_BandPass,
	ImEQBandType_COUNT
};
typedef int ImEQBandType;

struct ImEQBand
{
	float freq;      // Hz
	float gain_db;   // dB (peak/shelf) — ignored by pass/notch types
	float Q;         // quality factor
	bool  enabled;
	ImEQBandType type;

	ImEQBand() : freq( 1000.0f ), gain_db( 0.0f ), Q( 1.0f ), enabled( true ), type( ImEQBandType_Peak ) {}
};

// B3. Spectrogram
struct ImSpectrogramData
{
	ImVector<float> History;     // size = FrameCount * BinCount
	int             FrameCount;  // columns (time)
	int             BinCount;    // rows (frequency)
	int             WriteIdx;    // next frame column to overwrite
	float           MinDb;
	float           MaxDb;
	ImU32           Colormap[ 256 ];  // precomputed; caller can fill or library fills with viridis

	ImSpectrogramData() : FrameCount( 0 ), BinCount( 0 ), WriteIdx( 0 ), MinDb( -96.0f ), MaxDb( 0.0f )
	{
		for ( int i = 0; i < 256; ++i ) Colormap[ i ] = 0;
	}
};

// B5. LUFS
struct ImLUFSMeter
{
	float momentary_400ms;
	float short_term_3s;
	float integrated;
	float loudness_range;
	float true_peak_dbtp;

	ImLUFSMeter() : momentary_400ms( -70.0f ), short_term_3s( -70.0f ),
		integrated( -70.0f ), loudness_range( 0.0f ), true_peak_dbtp( -70.0f ) {}
};

// B8. Compressor curve
struct ImCompressorCurve
{
	float threshold_db;
	float ratio;
	float knee_db;
	float makeup_db;

	ImCompressorCurve() : threshold_db( -18.0f ), ratio( 4.0f ), knee_db( 6.0f ), makeup_db( 0.0f ) {}
};

// B9. Oscilloscope
struct ImOscilloscope
{
	ImVector<float> Ring;
	int             WriteIdx;
	float           SampleRate;
	float           TriggerLevel;
	int             TriggerEdge;  // 0 = rising, 1 = falling, 2 = free-run
	float           TimeBase_s;
	float           YGain;

	ImOscilloscope() : WriteIdx( 0 ), SampleRate( 48000.0f ), TriggerLevel( 0.0f ),
		TriggerEdge( 0 ), TimeBase_s( 0.010f ), YGain( 1.0f ) {}
};

// B11. Pitch / chord wheel
enum ImPitchWheelMode_
{
	ImPitchWheelMode_FifthsCircle = 0,
	ImPitchWheelMode_Chromatic,
	ImPitchWheelMode_COUNT
};
typedef int ImPitchWheelMode;

// C1. Piano roll
struct ImMidiNote
{
	float start_beats;
	float length_beats;
	int   pitch_0_127;
	float velocity_0_1;

	ImMidiNote() : start_beats( 0.0f ), length_beats( 1.0f ), pitch_0_127( 60 ), velocity_0_1( 0.8f ) {}
	ImMidiNote( float s, float l, int p, float v ) : start_beats( s ), length_beats( l ), pitch_0_127( p ), velocity_0_1( v ) {}
};

struct ImPianoRollData
{
	ImVector<ImMidiNote> Notes;
	float PixelsPerBeat;
	float PixelsPerPitch;
	float ScrollX, ScrollY;
	int   SelectedIdx;
	float GridSnap_beats;
	float LengthBeats;
	int   LowestPitch, HighestPitch;

	ImPianoRollData() : PixelsPerBeat( 48.0f ), PixelsPerPitch( 10.0f ),
		ScrollX( 0 ), ScrollY( 0 ), SelectedIdx( -1 ), GridSnap_beats( 0.25f ),
		LengthBeats( 16.0f ), LowestPitch( 36 ), HighestPitch( 96 ) {}
};

// C2. Step sequencer
struct ImStepSequencerData
{
	int Tracks;
	int Steps;
	ImVector<float> Cells;   // Tracks*Steps, velocity 0..1 (0 = off)
	ImVector<const char*> TrackLabels;

	ImStepSequencerData() : Tracks( 0 ), Steps( 0 ) {}
	void Resize( int tracks, int steps )
	{
		Tracks = tracks; Steps = steps;
		Cells.resize( tracks * steps );
		for ( int i = 0; i < Cells.Size; ++i ) Cells[ i ] = 0.0f;
	}
	float& At( int track, int step ) { return Cells[ track * Steps + step ]; }
	float  At( int track, int step ) const { return Cells[ track * Steps + step ]; }
};

// C3. Modulation matrix
struct ImModMatrixData
{
	ImVector<const char*> Sources;
	ImVector<const char*> Destinations;
	ImVector<float>       Depth;   // Sources.Size * Destinations.Size, -1..+1

	ImModMatrixData() {}
	void Resize()
	{
		Depth.resize( Sources.Size * Destinations.Size );
		for ( int i = 0; i < Depth.Size; ++i ) Depth[ i ] = 0.0f;
	}
	float& At( int src, int dst ) { return Depth[ src * Destinations.Size + dst ]; }
};

// C4. LFO designer
struct ImLFODesignerData
{
	ImVector<ImVec2> Shape;  // (0..1, 0..1) anchor points; cyclic
	float Phase;
	float Rate_Hz;
	bool  Bipolar;

	ImLFODesignerData() : Phase( 0.0f ), Rate_Hz( 1.0f ), Bipolar( false )
	{
		// Default = sine-like
		for ( int i = 0; i <= 16; ++i )
		{
			float t = (float)i / 16.0f;
			Shape.push_back( ImVec2( t, 0.5f + 0.5f * ImSin( t * 2.0f * IM_PI ) ) );
		}
	}
};

// C5. Mixer channel strip
struct ImMixerChannel
{
	float volume_db;
	float pan_m1_to_p1;
	float peak_db;
	float peak_hold_db;
	bool  mute, solo, record;
	float sends[ 8 ];
	int   send_count;
	const char* name;

	ImMixerChannel() : volume_db( 0.0f ), pan_m1_to_p1( 0.0f ),
		peak_db( -96.0f ), peak_hold_db( -96.0f ),
		mute( false ), solo( false ), record( false ), send_count( 0 ), name( "Ch" )
	{
		for ( int i = 0; i < 8; ++i ) sends[ i ] = 0.0f;
	}
};

//////////////////////////////////////////////////////////////////////////
// Color grading (Phase D)
//////////////////////////////////////////////////////////////////////////

struct ImLiftGammaGain
{
	ImVec4 Lift;    // rgb + master in .w
	ImVec4 Gamma;
	ImVec4 Gain;

	ImLiftGammaGain()
		: Lift( 0, 0, 0, 0 ), Gamma( 1, 1, 1, 1 ), Gain( 1, 1, 1, 1 ) {}
};

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
	float  DividerT;         // 0..1 along perpendicular-to-divider axis
	float  DividerAngleRad;  // 0 = vertical divider, CW positive
	bool   Swap;

	ImLookDevState() : DividerT( 0.5f ), DividerAngleRad( 0.0f ), Swap( false ) {}
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

// Vector Drawing Tool: world-space bezier authoring canvas.
// Path = sequence of ImVectorDrawingNode. Each node holds an anchor point plus
// in/out tangents (relative to anchor). Successive nodes are joined with a cubic
// bezier segment using the left node's OutTangent and the right node's InTangent.
struct ImVectorDrawingNode
{
	ImVec2 Anchor;       // world-space position
	ImVec2 InTangent;    // relative offset from Anchor, applied to the incoming segment's P2
	ImVec2 OutTangent;   // relative offset from Anchor, applied to the outgoing segment's P1
	bool   Broken;       // when false, In/Out mirror each other through the anchor

	ImVectorDrawingNode() : Anchor( 0, 0 ), InTangent( 0, 0 ), OutTangent( 0, 0 ), Broken( false ) {}
	ImVectorDrawingNode( ImVec2 p ) : Anchor( p ), InTangent( 0, 0 ), OutTangent( 0, 0 ), Broken( false ) {}
};

enum ImVectorDrawingStyle_
{
	ImVectorDrawingStyle_Polyline = 0,          // AddPolyline
	ImVectorDrawingStyle_PolylineAA,            // DrawPolylineAA (SDF)
	ImVectorDrawingStyle_StrokedBezier,         // DrawStrokedBezierPath (Euler spiral)
	ImVectorDrawingStyle_StrokedDashedBezier,   // DrawStrokedDashedBezierPath
	ImVectorDrawingStyle_DashedPolyline,        // DrawDashedPolylineAA
	ImVectorDrawingStyle_COUNT
};
typedef int ImVectorDrawingStyle;

struct ImVectorDrawingPath
{
	ImVector<ImVectorDrawingNode> Nodes;
	bool                          Closed;
	ImU32                         Color;
	float                         Thickness;
	ImVectorDrawingStyle          Style;
	float                         DashLen;
	float                         GapLen;

	ImVectorDrawingPath()
		: Closed( false ), Color( IM_COL32( 255, 220, 100, 255 ) ), Thickness( 2.0f ),
		  Style( ImVectorDrawingStyle_StrokedBezier ), DashLen( 8.0f ), GapLen( 6.0f ) {}
};

struct ImVectorDrawingData
{
	ImVector<ImVectorDrawingPath> Paths;
	ImVec2 PanOffset;   // world offset of view origin (screen-space pixels)
	float  Zoom;        // world → screen multiplier
	int    SelectedPath;
	int    SelectedNode;
	int    SelectedHandle; // 0 = anchor, 1 = in, 2 = out
	// Authoring state: -1 if not currently creating a new path.
	int    ActivePath;

	ImVectorDrawingData()
		: PanOffset( 0, 0 ), Zoom( 1.0f ), SelectedPath( -1 ), SelectedNode( -1 ),
		  SelectedHandle( 0 ), ActivePath( -1 ) {}
};

// Envelope / ADSR
struct ImEnvelopeStage
{
	float Duration;   // seconds (relative)
	float Target;     // end value (0..1 typical)
	float Curvature;  // -1..+1 (negative = concave, 0 = linear, positive = convex)

	ImEnvelopeStage() : Duration( 0.1f ), Target( 0.0f ), Curvature( 0.0f ) {}
	ImEnvelopeStage( float d, float t, float c = 0.0f ) : Duration( d ), Target( t ), Curvature( c ) {}
};

struct ImEnvelopeData
{
	ImVector<ImEnvelopeStage> Stages;     // sequential stages; first implicitly starts at value 0
	float                     StartValue; // value before stage 0
	bool                      Loop;       // loop from end back to LoopStage
	int                       LoopStage;
	bool                      Sustain;    // hold SustainStage end value until release
	int                       SustainStage;
	int                       SelectedIdx;

	ImEnvelopeData()
		: StartValue( 0.0f ), Loop( false ), LoopStage( 0 ), Sustain( false ),
		  SustainStage( -1 ), SelectedIdx( -1 ) {}
};

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

namespace ImWidgets{
	enum ImWidgetsBgEffect
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
	extern ImGlobalData GlobalData;

	ImWidgetsStyle& GetStyle();
	IMGUI_API void  ShowStyleEditor( ImWidgetsStyle* ref = NULL );

	inline
	const char* GetStyleColorName( ImWidgetsStyleColor colorIndex )
	{
		GetStyle().GetColorName( colorIndex );
	}
	inline
	void PushStyleColor( ImWidgetsStyleColor colorIndex, const ImVec4& color )
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
	void PushStyleVar( ImWidgetsStyleVar varIndex, const ImVec2& value )
	{
		GetStyle().PushVar( varIndex, value );
	}
	inline
	void PushStyleVar( ImWidgetsStyleVar varIndex, const ImVec4& value )
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
	float ImLength(ImVec2 v)
	{
		return ImSqrt( ImLengthSqr( v ) );
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
		ImU32* current = colors;
		colorsConverted.resize( 3 * color_count );
		for ( int k = 0; k < color_count; ++k )
		{
			ImVec4 col = ( ImVec4 )ImColor( *current );
			colorsConverted[ 3 * k + 0 ] = col.x;
			colorsConverted[ 3 * k + 1 ] = col.y;
			colorsConverted[ 3 * k + 2 ] = col.z;
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

	//////////////////////////////////////////////////////////////////////////
	// Color Functions
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API ImU32	ImColorFrom_xyz( float x, float y, float z, float* xyzToRGB, float gamma );

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
	//void	MemoryString( std::string& sResult, ImU64 const uMemoryByte );
	//void	MemoryString( std::string& sResult, ImU64 const uMemoryByte );
	//void	MemoryString( std::string& sResult, ImU64 const uMemoryByte );

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
	void	GenShapeConcavePoly( ImWidgetsShape& shape, const ImVec2* pts, int pts_count, ImVec2 origin = ImVec2( 0.0f, 0.0f ) );

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
	// FONT SIZE UNIT — Logical Pixels (lp):
	//   All font_size parameters are in logical pixels (DPI-independent).
	//   1 lp = 1 physical pixel at 96 DPI / 1.0x scale.
	//   At 2x DPI (192 DPI / Windows 200%), 1 lp = 2 physical pixels.
	//   The library converts lp → physical pixels internally using ImGui::GetStyle().FontScaleDpi.
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
	IMGUI_API void DrawText( ImDrawList* pDrawList, ImVec2 pos, ImU32 col, const char* text, const char* text_end = nullptr );
	// Explicit font and size in logical pixels (pass nullptr/0 to use current ImGui font/size)
	IMGUI_API void DrawText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, ImU32 col, const char* text, const char* text_end = nullptr );
	// Measure text. font_size in lp. Returns (width, height) in physical pixels.
	// out_ascent: if non-null, receives ascent in physical pixels (pass cursor.y + ascent as baseline to DrawText).
	IMGUI_API ImVec2 CalcTextSize( ImFont* font, float font_size, const char* text, const char* text_end = nullptr, float* out_ascent = nullptr );
	// Like CalcTextSize but runs the OpenType shaper (HarfBuzz) for accurate width of shaped text
	// (Arabic contextual forms, ligatures). Slower than CalcTextSize — use only when alignment precision matters.
	IMGUI_API float CalcShapedTextWidth( ImFont* font, float font_size, const char* text, const char* text_end = nullptr );
	// Horizontal linear gradient: col_left at text start, col_right at text end. font_size in lp.
	IMGUI_API void DrawTextGradient( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, ImU32 col_left, ImU32 col_right, const char* text, const char* text_end = nullptr );
	// ---- Typography: tesselated text for gradient/image fills ----
	// font_size in lp. tess_tol: curve flattening tolerance (lower = more segments, smoother). 0 = auto.
	// Convert text to CPU-tesselated geometry (ImWidgetsShape) for use with gradient/image fill functions.
	IMGUI_API void TesselateText( ImFont* font, float font_size, const char* text, ImWidgetsShape& outShape, const char* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	// Same as TesselateText but returns per-glyph shapes (preserves ligatures/calt from full text shaping).
	IMGUI_API void TesselateTextPerGlyph( ImFont* font, float font_size, const char* text, ImVector<ImWidgetsShape>& outShapes, const char* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	// Pre-tessellate every glyph in the font and populate the cache. Call once at startup (after the
	// first ImGui::NewFrame) to eliminate stalls when TesselateText/TesselateTextPerGlyph first runs.
	IMGUI_API void PrewarmTessellationCache( ImFont* font, float tess_tol = 0.0f );
	// Extract raw contour points (explicitly closed, for DrawShapeWithHole). font_size in lp. Debug/internal use.
	IMGUI_API void ExtractTextContours( ImFont* font, float font_size, const char* text, const char* text_end, ImVec2 offset, ImVector<ImVec2>& outPoly, ImRect& outBB, float tess_tol = 0.0f );
	// Debug: draw the tessellation algorithm steps for a single character. font_size in lp.
	IMGUI_API void DrawTesselateDebug( ImDrawList* dl, ImFont* font, float font_size, const char* text, ImVec2 pos, float tess_tol, float spacing, float rowH );
	// Text filled with an image texture. font_size in lp.
	IMGUI_API void DrawImageText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, ImTextureID tex, const char* text, const char* text_end = nullptr, ImU32 tint = IM_COL32_WHITE, ImVec2 uv_offset = ImVec2(0,0), ImVec2 uv_scale = ImVec2(1,1), float tess_tol = 0.0f, int iterations = 0 );
	// Text filled with gradients (supports all color spaces via function pointers). font_size in lp.
	IMGUI_API void DrawLinearGradientText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB = nullptr, pfsRGB2Space sRGB2Space = nullptr, const char* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	IMGUI_API void DrawRadialGradientText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB = nullptr, pfsRGB2Space sRGB2Space = nullptr, const char* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	IMGUI_API void DrawDiamondGradientText( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, pfSpace2sRGB space2sRGB = nullptr, pfsRGB2Space sRGB2Space = nullptr, const char* text_end = nullptr, float tess_tol = 0.0f, int iterations = 0 );
	// GPU gradient text rendering (requires ImPlatform + SLUG_FILL shader). No CPU tessellation.
	// Gradient is computed per-pixel on GPU. Falls back to CPU tessellation if shader unavailable. font_size in lp.
	// colorSpace: 0=sRGB, 1=Linear, 2=OkLab, 3=OkLch, 4=HSV (matches ImWidgetsGradientInterp_)
	IMGUI_API void DrawLinearGradientTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, const char* text_end = nullptr, bool perChar = false, int colorSpace = 0 );
	IMGUI_API void DrawRadialGradientTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, const char* text_end = nullptr, bool perChar = false, int colorSpace = 0 );
	IMGUI_API void DrawDiamondGradientTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, ImVec2 uv_start, ImVec2 uv_end, ImU32 col0, ImU32 col1, const char* text_end = nullptr, bool perChar = false, int colorSpace = 0 );
	// GPU image text rendering. Image is sampled per-pixel within glyph coverage. font_size in lp.
	IMGUI_API void DrawImageTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, ImTextureID tex, ImU32 tint = IM_COL32_WHITE, ImVec2 uv_offset = ImVec2(0,0), ImVec2 uv_scale = ImVec2(1,1), const char* text_end = nullptr, bool perChar = false );
	// GPU image text with per-character texture cycling. textures[i % nTextures] is used for glyph i.
	IMGUI_API void DrawImageTextGPU( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, const ImTextureID* textures, int nTextures, ImU32 tint = IM_COL32_WHITE, ImVec2 uv_offset = ImVec2(0,0), ImVec2 uv_scale = ImVec2(1,1), const char* text_end = nullptr );
	// Debug: draw curve outlines, control points, and bounding boxes for Slug glyphs. font_size in lp.
	// flags: 1=curves, 2=control points, 4=bounding boxes, 8=band grid, 0xFF=all
	IMGUI_API void DrawTextDebugCurves( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, const char* text_end = nullptr, int flags = 0xFF );
	// Debug: draw each color layer's quad as a flat semi-transparent rectangle (no Slug shader). font_size in lp.
	IMGUI_API void DrawTextDebugLayers( ImDrawList* pDrawList, ImFont* font, float font_size, ImVec2 pos, const char* text, const char* text_end = nullptr );
	// Set to true to use the debug shader (xcov=R, ycov=G, coverage=B) instead of normal rendering.
	IMGUI_API extern bool g_SlugDebugShader;
	// ImFontLoader backend: rasterizes Slug glyphs (including color/gradient) into ImGui's bitmap atlas.
	// Use with: cfg.FontLoader = ImWidgets::GetSlugFontLoader();
	IMGUI_API const ImFontLoader* GetSlugFontLoader();
	IMGUI_API bool GetSlugFontInfo(ImFont* font, void* outStbttFontInfo, float* outEmScale); // outStbttFontInfo = stbtt_fontinfo*
	IMGUI_API void SlugBuildGlyphByID(ImFont* font, int glyphID);
	// LaTeX math rendering via Slug GPU fonts. font_size in lp.
	// Requires ImWidgetsFeatures_LaTeX to be set before CreateContext().
	// latex: LaTeX math string (e.g. "x^2 + \\frac{\\alpha}{\\beta} = 0")
	// Call during font loading phase (before CreateContext) to load the Latin Modern Math font.
	IMGUI_API void LoadLaTeXFont();
	IMGUI_API void DrawLaTeX( ImDrawList* pDrawList, float font_size, ImVec2 pos, ImU32 col, const char* latex );
	// Measure the bounding box of a LaTeX expression. font_size in lp. Returns size in physical pixels.
	IMGUI_API ImVec2 CalcLaTeXSize( float font_size, const char* latex );
	// Debug: draw bounding boxes for each glyph/box in a LaTeX expression. font_size in lp.
	IMGUI_API void DrawLaTeXDebug( ImDrawList* pDrawList, float font_size, ImVec2 pos, const char* latex );
	// Tessellate a LaTeX expression into an ImWidgetsShape for gradient/image fills. font_size in lp.
	// pos = top-left corner (same convention as DrawLaTeX). Use CalcLaTeXSize for layout size.
	IMGUI_API void TesselateLaTeX( float font_size, const char* latex, ImVec2 pos, ImWidgetsShape& outShape, float tess_tol = 0.25f, int iterations = 0 );

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
	IMGUI_API void DrawProceduralColorSplineBilinear( ImDrawList* pDrawList, const ImVec2* points, int points_count, float thickness, ImWidgetsColor1DCallback func, void* pUserData, int resolution, bool closed );

	IMGUI_API void DrawColorRing( ImDrawList* pDrawList, ImVec2 const curPos, ImVec2 const size, float thickness_, ImWidgetsColor1DCallback func, void* pUserData, int division, float colorOffset, bool bIsBilinear );

	IMGUI_API ImVec4 GradientSample( ImGradientData const& gradient, float t );
	IMGUI_API float  GradientAlphaSample( ImGradientData const& gradient, float t );
	IMGUI_API void DrawCheckerboard( ImDrawList* pDrawList, ImVec2 position, ImVec2 size, float cellSize, ImU32 col1, ImU32 col2 );
	IMGUI_API void DrawGradientBar( ImDrawList* pDrawList, ImGradientData const& gradient, ImVec2 position, ImVec2 size, int resolution );
	IMGUI_API void DrawSplineGradient( ImDrawList* pDrawList, ImGradientData const& gradient, const ImVec2* points, int points_count, float thickness, int resolution, bool closed = false );

	IMGUI_API float CurveEditorEvalEasing( ImCurveEditorSeg seg, float t );
	IMGUI_API float CurveEditorSample( ImCurveEditorData const& curve, float x );
	IMGUI_API const char* CurveEditorSegName( ImCurveEditorSeg seg );
	IMGUI_API const char* CurveEditorTangentModeName( ImCurveEditorTangentMode mode );

	IMGUI_API void DrawOkLabQuad( ImDrawList* pDrawList, ImVec2 start, ImVec2 size, float L, int resX = 16, int resY = 16 );
	IMGUI_API void DrawOkLchQuad( ImDrawList* pDrawList, ImVec2 start, ImVec2 size, float L, int resX = 16, int resY = 16 );

	// poly: Clockwise: Positive shape & Counter-clockwise for hole
	IMGUI_API void DrawShapeWithHole( ImDrawList* draw, ImVec2* poly, int points_count, ImU32 color, ImRect* p_bb = NULL, int gap = 1, int strokeWidth = 1 );

	IMGUI_API void DrawImageConvexShape( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count, ImU32 tint,
										 ImVec2 uv_offset = ImVec2( 0.0f, 0.0f ), ImVec2 uv_scale = ImVec2( 1.0f, 1.0f ) );
	IMGUI_API void DrawImageConcaveShape( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count, ImU32 tint,
										  ImVec2 uv_offset = ImVec2( 0.0f, 0.0f ), ImVec2 uv_scale = ImVec2( 1.0f, 1.0f ) );
	// poly: Clockwise outer contour & Counter-clockwise for holes (same format as DrawShapeWithHole)
	IMGUI_API void DrawImageShapeWithHole( ImDrawList* draw, ImTextureID img, ImVec2* poly, int points_count, ImU32 tint,
										   ImVec2 uv_offset = ImVec2( 0.0f, 0.0f ), ImVec2 uv_scale = ImVec2( 1.0f, 1.0f ),
										   int gap = 3, int strokeWidth = 3 );

#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
	IMGUI_API void CreateInternalShader( ImDrawShader* shaders_out, char const* shader_name, int sizeof_vs_const_buffer, void *vs_const_buffer, int sizeof_ps_const_buffer, void *ps_const_buffer );

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

	// TODO: find a clean way expose the style of the draws:
	// Triangle of ColorSpace
	// White PointDrawChromaticityPlotGeneric( ImDrawList* pDrawList,
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
	IMGUI_API void DrawChromaticityPlot( ImDrawList* draw,
										 ImWidgetsIlluminance illuminance,
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
	IMGUI_API bool IsBoundingBoxWellFormed( const ImVec2& r_min, const ImVec2& r_max, ImVec2* pts, int pts_count );

	IMGUI_API bool Im_IsCircleContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsCapsuleHContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsCapsuleVContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsPolyConvexContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsPolyConcaveContains( ImVec2 p, void* data );
	IMGUI_API bool Im_IsPolyWithHoleContains( ImVec2 p, void* data );

	IMGUI_API bool IsMouseHovering( const ImVec2& r_min, const ImVec2& r_max, IsContains contains, void* data, bool clip = true );
	IMGUI_API bool ItemHoverable( const ImRect& bb, ImGuiID id, ImGuiItemFlags item_flags, IsContains isContains, void* extra_data );

	//typedef bool ( *ImItemHoverableFunc )( const ImRect& bb, ImGuiID id, ImGuiItemFlags item_flags, void* extra_data );
	IMGUI_API bool ButtonBehaviorEx( const ImRect& bb, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags, IsContains isContains, void* extra_data );
	IMGUI_API bool ButtonBehaviorCircle( ImVec2 center, float radius, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorCapsuleH( ImVec2 pos, float length, float radius, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorCapsuleV( ImVec2 pos, float length, float radius, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorConvex( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorConcave( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonBehaviorWithHole( ImVec2* pts, int pts_count, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags );

	//////////////////////////////////////////////////////////////////////////
	// Widgets
	//////////////////////////////////////////////////////////////////////////
	IMGUI_API bool ButtonEx( const char* label, const ImVec2& size_arg, ImRect bb, ImVec2 text_offset, ImGuiButtonFlags flags,
							 IsContains isContains, ImDrawShape outline, ImDrawShapeFilled fill, ImDrawShapeFilledTex fill_tex, ImInlineOffset offset, FromRect from_rect,
							 void* extra_data,
							 ImTextureID* tex = NULL, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ButtonExCircle( const char* label, float radius, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExCapsuleH( const char* label, float length, float thickness, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExCapsuleV( const char* label, float length, float thickness, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExConvex( const char* label, const ImVec2& size_arg, ImVec2* pts, int pts_count, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExConcave( const char* label, const ImVec2& size_arg, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags );
	IMGUI_API bool ButtonExWithHole( const char* label, const ImVec2& size_arg, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags );

	IMGUI_API bool ImageButtonExCircle( const char* label, ImTextureID tex, float radius, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ImageButtonExCapsuleH( const char* label, ImTextureID tex, float length, float thickness, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ImageButtonExCapsuleV( const char* label, ImTextureID tex, float length, float thickness, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ImageButtonExConvex( const char* label, ImTextureID tex, const ImVec2& size_arg, ImVec2* pts, int pts_count, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );
	IMGUI_API bool ImageButtonExConcave( const char* label, ImTextureID tex, const ImVec2& size_arg, ImVec2* pts, int pts_count, ImVec2 text_offset, ImGuiButtonFlags flags, ImU32 col = IM_COL32_WHITE, ImVec2 uv_min = { 0.0f, 0.0f }, ImVec2 uv_max = { 1.0f, 1.0f } );

	IMGUI_API bool HueSelector( char const* label, float hueHeight, float cursorHeight, float* hueCenter, float* hueWidth, float* featherLeft, float* featherRight, int division = 32, float alpha = 1.0f, float hideHueAlpha = 0.75f, float offset = 0.0f );
	IMGUI_API bool SliderNScalar( char const* label, ImGuiDataType data_type, void* ordered_value, int value_count, void* p_min, void* p_max, float cursor_width, bool show_hover_by_region );
	IMGUI_API bool SliderNFloat( char const* label, float* ordered_value, int value_count, float v_min, float v_max, float cursor_width, bool show_hover_by_region );
	IMGUI_API bool SliderNInt( char const* label, int* ordered_value, int value_count, int v_min, int v_max, float cursor_width, bool show_hover_by_region );
	// TODO: Add bool flipY
	IMGUI_API bool Slider2DScalar( char const* pLabel, ImGuiDataType data_type, void* pValueX, void* pValueY, void* p_minX, void* p_maxX, void* p_minY, void* p_maxY );
	IMGUI_API bool Slider2DFloat( char const* pLabel, float* pValueX, float* pValueY, float v_minX, float v_maxX, float v_minY, float v_maxY );
	IMGUI_API bool Slider2DInt( char const* pLabel, int* pValueX, void* pValueY, int v_minX, int v_maxX, int v_minY, int v_maxY );

	// SliderGradient: 1D slider with an ImGradientData-backed background. Color space comes from gradient->Interpolation.
	IMGUI_API bool SliderGradientScalar( char const* label, ImGuiDataType data_type, void* p_value, const void* p_min, const void* p_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool SliderGradientFloat( char const* label, float* v, float v_min, float v_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool SliderGradientInt( char const* label, int* v, int v_min, int v_max, ImGradientData const* gradient, ImVec2 size = ImVec2( 0, 0 ) );

	// SliderGradientRing: interactive ring/arc slider with gradient background.
	// Full-circle overload: value wraps at boundaries (natural for hue).
	IMGUI_API bool SliderGradientRingScalar( char const* label, ImGuiDataType data_type, void* p_value, const void* p_min, const void* p_max, ImGradientData const* gradient, float outerRadius, float thickness );
	IMGUI_API bool SliderGradientRingFloat( char const* label, float* v, float v_min, float v_max, ImGradientData const* gradient, float outerRadius, float thickness );
	IMGUI_API bool SliderGradientRingInt( char const* label, int* v, int v_min, int v_max, ImGradientData const* gradient, float outerRadius, float thickness );
	// Arc overload: caller specifies startAngle + sweepAngle (radians). Value is clamped (no wrap).
	IMGUI_API bool SliderGradientRingScalar( char const* label, ImGuiDataType data_type, void* p_value, const void* p_min, const void* p_max, ImGradientData const* gradient, float outerRadius, float thickness, float startAngle, float sweepAngle );
	IMGUI_API bool SliderGradientRingFloat( char const* label, float* v, float v_min, float v_max, ImGradientData const* gradient, float outerRadius, float thickness, float startAngle, float sweepAngle );
	IMGUI_API bool SliderGradientRingInt( char const* label, int* v, int v_min, int v_max, ImGradientData const* gradient, float outerRadius, float thickness, float startAngle, float sweepAngle );

	// Unit Field: DragFloat with built-in unit selector
	IMGUI_API bool UnitField( char const* label, float* pValue, ImUnitDef* units, int unitCount, int* pSelectedUnit, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = NULL );

	// Paint Canvas
	IMGUI_API bool PaintCanvas( char const* label, ImPaintCanvasData* canvas, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API bool GradientEditor( char const* label, ImGradientData* gradient, bool alpha = true, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool CurveEditor( char const* label, ImCurveEditorData* curve, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API void DrawColorDisc( ImDrawList* pDrawList, ImVec2 center, float radius, ImColorWheelMode mode, float thirdAxis, int numSectors = 64, int numRings = 16 );
	IMGUI_API void DrawCircularGradientIndicator( ImDrawList* pDrawList, ImVec2 center, float outerRadius, float innerRadius, float t );
	IMGUI_API bool ColorWheel( char const* label, ImVec4* color, ImColorWheelMode mode = ImColorWheelMode_HSV, float hdr_max = 1.0f, bool fixedIntensity = false, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool PrimariesWheel( char const* label, ImVec4* color, float* yValue, float yMin, float yMax, ImColorWheelMode mode = ImColorWheelMode_HSV, float ringThickness = 12.0f, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool HDRWheel( char const* label, ImVec4* color, float* yValue, float yMin, float yMax, float* rightValue, float rightMin, float rightMax, float* leftValue, float leftMin, float leftMax, ImColorWheelMode mode = ImColorWheelMode_HSV, float ringThickness = 12.0f, ImVec2 size = ImVec2( 0, 0 ) );

	// Color Picker
	IMGUI_API bool ColorPickerSRGB( char const* label, ImVec4* color, int fixedAxis = 2, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerHSV( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerOkLab( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerOkLCH( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerCIELab( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPickerXYZ( char const* label, ImVec4* color, ImVec2 size = ImVec2( 0, 0 ) );
	IMGUI_API bool ColorPicker( char const* label, ImVec4* color, ImColorPickerSpace space = ImColorPickerSpace_sRGB, int fixedAxis = 2, ImVec2 size = ImVec2( 0, 0 ) );

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
	IMGUI_API const char* ColorCurveModeName( ImColorCurveMode mode );
	IMGUI_API float ColorCurveSample( ImColorCurveData const& curve, ImColorCurveMode mode, float x, bool advancedSegments = false );
	IMGUI_API bool  ColorCurve( char const* label, ImColorCurveData* curve, ImColorCurveMode mode, ImHistogramData const* histogramOverlay = NULL, bool advancedSegments = false, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API const char* ParadeModeName( ImParadeMode mode );
	IMGUI_API void  ParadeScope( char const* label, ImParadeScopeData const& data, bool overlay = false, ImParadeScale scale = ImParadeScale_Linear, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API void  VectorScope( char const* label, ImVectorScopeData const& data, bool showSkinToneLine = true, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API const char* HistogramModeName( ImHistogramMode mode );
	IMGUI_API void  Histogram( char const* label, ImHistogramData const& data, ImHistogramLayout layout = ImHistogramLayout_Overlapped, ImParadeScale xScale = ImParadeScale_Linear, ImParadeScale yScale = ImParadeScale_Linear, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API const char* CIEChromaticityGamutName( ImCIEChromaticityGamut gamut );
	IMGUI_API void  CIEChromaticity( char const* label, ImCIEChromaticityData const& data, ImCIEChromaticityGamut gamut = ImCIEChromaticityGamut_sRGB_Rec709, bool showBackground = false, ImCIEChromaticitySignalColor signalColor = ImCIEChromaticitySignalColor_Flat, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API int   ToneCurveChannelCount( ImHistogramMode mode );
	IMGUI_API const char* ToneCurveChannelName( ImHistogramMode mode, int channel );
	IMGUI_API float ToneCurveSample( ImColorCurveData const& curve, float x, bool advancedSegments = false );
	IMGUI_API bool  ToneCurve( char const* label, ImToneCurveData* curve, ImHistogramMode mode, ImHistogramData const* histogramOverlay = NULL, bool advancedSegments = false, ImVec2 size = ImVec2( 0, 0 ) );

	IMGUI_API bool SliderRingScalar( char const* label, ImGuiDataType data_type, void* p_value, void* p_min, void* p_max,
									float v_angle_min = -0.75f * IM_PI, float v_angle_max = 0.75f * IM_PI,
									float v_thickness = 0.0f, const char* format = NULL, ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderRingFloat( char const* label, float* value, float v_min, float v_max,
									float v_angle_min = -0.75f * IM_PI, float v_angle_max = 0.75f * IM_PI,
									float v_thickness = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderRingInt( char const* label, int* value, int v_min, int v_max,
								  float v_angle_min = -0.75f * IM_PI, float v_angle_max = 0.75f * IM_PI,
								  float v_thickness = 0.0f, const char* format = "%d", ImGuiSliderFlags flags = 0 );

	// Spline Slider: slider whose track follows cubic bezier curve(s).
	// control_points: ImVec2 array in normalized [0,1]x[0,1] space mapped to widget rect. NULL = default S-curve.
	// num_points: 4 for single bezier, 7 for two chained segments, 3N+1 for N segments.
	IMGUI_API bool SliderSplineScalar( char const* label, ImGuiDataType data_type, void* p_value, void* p_min, void* p_max,
									   const ImVec2* control_points = NULL, int num_points = 4, float v_height = 0.0f,
									   float v_thickness = 0.0f, const char* format = NULL, ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineFloat( char const* label, float* value, float v_min, float v_max,
									  const ImVec2* control_points = NULL, int num_points = 4, float v_height = 0.0f,
									  float v_thickness = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineInt( char const* label, int* value, int v_min, int v_max,
									const ImVec2* control_points = NULL, int num_points = 4, float v_height = 0.0f,
									float v_thickness = 0.0f, const char* format = "%d", ImGuiSliderFlags flags = 0 );

	// SliderSplineGradient: spline-track slider with a gradient background rendered along the bezier path.
	IMGUI_API bool SliderSplineGradientScalar( char const* label, ImGuiDataType data_type, void* p_value, void* p_min, void* p_max,
											   ImGradientData const* gradient,
											   const ImVec2* control_points = NULL, int num_points = 4, float v_height = 0.0f,
											   float v_thickness = 0.0f, const char* format = NULL, ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineGradientFloat( char const* label, float* value, float v_min, float v_max,
											  ImGradientData const* gradient,
											  const ImVec2* control_points = NULL, int num_points = 4, float v_height = 0.0f,
											  float v_thickness = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0 );
	IMGUI_API bool SliderSplineGradientInt( char const* label, int* value, int v_min, int v_max,
											ImGradientData const* gradient,
											const ImVec2* control_points = NULL, int num_points = 4, float v_height = 0.0f,
											float v_thickness = 0.0f, const char* format = "%d", ImGuiSliderFlags flags = 0 );

	IMGUI_API bool DragFloatPrecise( char const* label, float* value, float v_min = 0.0f, float v_max = 0.0f, const char* format = NULL, ImGuiSliderFlags flags = 0 );

	// Up Vector selector (hemisphere picker)
	IMGUI_API bool UpVector( char const* label, float* direction, int defaultUpAxis = 1, ImVec2 size = ImVec2( 0, 0 ) );

	// Image Carousel
	IMGUI_API bool ImageCarousel( char const* label, ImTextureID* images, ImVec2* imageSizes, int imageCount, int* pSelectedIndex, ImVec2 size = ImVec2( 0, 0 ) );

	// Image Bento Grid: displays images in a uniform grid, cropping to a target cell aspect ratio (center crop).
	//
	// Drag-to-reorder support (optional):
	//   - pItemIds   : array of `imageCount` C-strings. When non-null, each cell's ImGui ID is derived
	//                  from its string instead of its position. This is required for clean drag-reorder
	//                  — positional IDs cause a one-frame flicker when adjacent cells swap mid-drag
	//                  because ImGui's ActiveId stays pinned at the old index rather than following
	//                  the moved content. Strings must be unique per grid (duplicates across grids in
	//                  other windows are fine).
	//   - pReorderFrom / pReorderTo : out-params. When a swap is requested, the function writes the
	//                  source and target indices (both in [0, imageCount)) and returns true. The caller
	//                  is responsible for actually swapping the data — ImageBento itself never mutates
	//                  the images/imageSizes arrays. Set to -1 on frames where no swap fires.
	//                  Swaps are reported as ADJACENT only (|from-to| == 1 horizontal, or == columnsPerRow
	//                  vertical). Multi-cell drags accumulate as multiple single-frame reports.
	// Returns true if *pSelectedIndex changed OR a reorder was requested this frame.
	IMGUI_API bool ImageBento( char const* label, ImTextureID* images, ImVec2* imageSizes, int imageCount, int* pSelectedIndex, int columnsPerRow = 4, float cellAspect = 1.0f, float spacing = 4.0f, char const* const* pItemIds = nullptr, int* pReorderFrom = nullptr, int* pReorderTo = nullptr );

	// Image Viewer: pan (left-drag), zoom (scroll wheel), double-click to reset.
	// Right-click shows a pixel-inspector loupe with RGBA values (requires state.Pixels).
	IMGUI_API bool ImageViewer( char const* label, ImTextureID image, ImVec2 imageSize, ImImageViewerState& state, ImVec2 widgetSize = ImVec2( 0, 0 ) );

	// Image Inspector: color-managed raw-buffer viewer with shader-side decode of any of the 11
	// sample types × 1..4 channels described by ImImageBuffer. View transforms (gamma, sRGB,
	// camera log curves, gamut, exposure, white point, tonemap, false color, NaN highlight,
	// mosaic decode) are applied in a single HLSL uber-shader. Pan/zoom/double-click/inspector
	// loupe UX matches ImageViewer. The widget uploads buffer.host to a packed RGBA32F texture
	// once per buffer.version change; per-frame CPU cost is uniform updates only.
	//
	// The CPU-side inspector loupe reads buffer.host directly with full precision — the user
	// must keep the buffer alive while the widget is open.
	//
	// Returns true if the user interacted with the widget. On backends without custom shader
	// support (DX9), draws a "not supported" message and returns false.
	IMGUI_API bool ImageInspector( char const* label, const ImImageBuffer& buffer, ImImageInspectorState& state, ImVec2 widgetSize = ImVec2( 0, 0 ) );

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
    // Draw a solid, anti-aliased polyline (Rougier 2013 SDF; selectable CPU/GPU
    // path via Set/GetDashedLinesUseGPU). Equivalent to DrawDashedPolylineAA
    // with no dashes — uses the same code paths but skips dash math.
    IMGUI_API void DrawPolylineAA(
        ImDrawList* drawlist,
        const ImVec2* points, int points_count,
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
        const ImVec2* points, int points_count,
        ImU32 col, float thickness,
        const float* dashes, int dashes_count, float dash_offset,
        bool closed = false,
        ImWidgetsCap cap = ImWidgetsCap_Butt,
        ImWidgetsJoin join = ImWidgetsJoin_Mitter,
        float miter_limit = 4.0f);

    // Convenience: single on/off lengths.
    IMGUI_API void DrawDashedPolylineAA(
        ImDrawList* drawlist,
        const ImVec2* points, int points_count,
        ImU32 col, float thickness,
        float dash_len, float gap_len, float dash_offset,
        bool closed = false,
        ImWidgetsCap cap = ImWidgetsCap_Butt,
        ImWidgetsJoin join = ImWidgetsJoin_Mitter,
        float miter_limit = 4.0f);

    //////////////////////////////////////////////////////////////////////////
    // Stroke Expansion (Euler Spiral) — based on Linebender HPG 2024 paper
    //////////////////////////////////////////////////////////////////////////
    // Stroke a cubic Bezier path with high-quality parallel curves via Euler spirals.
    // points: 3*N+1 control points for N cubics [p0,p1,p2,p3, p4,p5,p6, ...]
    IMGUI_API void DrawStrokedBezierPath(
        ImDrawList* drawlist,
        const ImVec2* points, int points_count,
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
        const ImVec2* points, int points_count,
        ImU32 col, float thickness,
        ImWidgetsCap cap = ImWidgetsCap_Round,
        ImWidgetsJoin join = ImWidgetsJoin_Round,
        float miter_limit = 4.0f,
        bool closed = false);

    // Stroke a dashed cubic Bezier path. dash_array is alternating dash,gap lengths.
    IMGUI_API void DrawStrokedDashedBezierPath(
        ImDrawList* drawlist,
        const ImVec2* points, int points_count,
        ImU32 col, float thickness,
        const float* dash_array, int dash_count, float dash_offset = 0.0f,
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
        const ImVec2* points, int points_count,
        ImU32 col, float thickness,
        const float* dash_array, int dash_count, float dash_offset = 0.0f,
        ImWidgetsCap cap = ImWidgetsCap_Round,
        ImWidgetsJoin join = ImWidgetsJoin_Round,
        float miter_limit = 4.0f,
        bool closed = false);

    // Debug
    IMGUI_API void SetStrokeDebugWireframe(bool enable);
    IMGUI_API bool GetStrokeDebugWireframe();

    //////////////////////////////////////////////////////////////////////////
    // Extended primitives (D1–D6)
    //////////////////////////////////////////////////////////////////////////

    // D1. Hatching & stippling fills (scanline-clip against arbitrary polygon)
    IMGUI_API void DrawHatchFill(ImDrawList* drawlist,
                                 const ImVec2* poly, int poly_count,
                                 ImWidgetsHatchPattern pattern,
                                 float spacing, float angle_rad,
                                 float thickness, ImU32 col);
    IMGUI_API void DrawStippleFill(ImDrawList* drawlist,
                                   const ImVec2* poly, int poly_count,
                                   float density, float jitter,
                                   float radius, ImU32 col,
                                   unsigned int seed = 1u);

    // D2. Bezier surface patches
    IMGUI_API void DrawCoonsPatch(ImDrawList* drawlist,
                                  const ImCoonsPatch& patch,
                                  ImU32 col, int resU = 16, int resV = 16);
    IMGUI_API void DrawCoonsPatchGradient(ImDrawList* drawlist,
                                          const ImCoonsPatch& patch,
                                          ImU32 c00, ImU32 c10, ImU32 c11, ImU32 c01,
                                          int resU = 16, int resV = 16);
    IMGUI_API void DrawCoonsPatchWireframe(ImDrawList* drawlist,
                                           const ImCoonsPatch& patch,
                                           ImU32 col, float thickness,
                                           int resU = 16, int resV = 16);
    IMGUI_API void DrawGregoryPatch(ImDrawList* drawlist,
                                    const ImGregoryPatch& patch,
                                    ImU32 col, int resU = 16, int resV = 16);

    // D3. Rounded path offsetting
    IMGUI_API void OffsetPolyline(const ImVec2* in_pts, int n,
                                  float offset, ImWidgetsJoin join,
                                  float miter_limit,
                                  ImVector<ImVec2>& out_pts, bool closed);
    IMGUI_API void DrawOffsetOutline(ImDrawList* drawlist,
                                     const ImVec2* pts, int n,
                                     float offset, ImWidgetsJoin join,
                                     float miter_limit, float thickness,
                                     ImU32 col, bool closed);
    IMGUI_API void DrawOffsetFilled(ImDrawList* drawlist,
                                    const ImVec2* pts, int n,
                                    float inset, ImU32 col, bool closed);

    // D4. Iso-contours (marching squares)
    IMGUI_API void DrawIsoContour(ImDrawList* drawlist,
                                  ImVec2 pos, ImVec2 size,
                                  int resX, int resY,
                                  ImWidgetsScalar2DCallback f, void* user,
                                  const float* iso_values, int iso_count,
                                  ImU32 col, float thickness);
    // Multi-tier iso-contour (major/medium/minor in one pass).
    // Draws tiers in array order; put finest/faintest first, major last so it paints on top.
    // pre_log_transform: replace each sample v with log(max(v, eps)) before contouring.
    // min_value/max_value: limit iso enumeration to this value range.
    IMGUI_API void DrawIsoContourTiered(ImDrawList* drawlist,
                                        ImVec2 pos, ImVec2 size,
                                        int resX, int resY,
                                        ImWidgetsScalar2DCallback f, void* user,
                                        const ImIsoContourTier* tiers, int tier_count,
                                        float min_value = -FLT_MAX, float max_value = FLT_MAX,
                                        bool pre_log_transform = false);

    IMGUI_API void DrawIsoFilled(ImDrawList* drawlist,
                                 ImVec2 pos, ImVec2 size,
                                 int resX, int resY,
                                 ImWidgetsScalar2DCallback f, void* user,
                                 const float* band_edges, int band_edge_count,
                                 const ImU32* band_cols);

    // D5. Conic gradient fill
    IMGUI_API void DrawConicGradient(ImDrawList* drawlist,
                                     ImVec2 center, float radius,
                                     const ImGradientData& gradient,
                                     float start_angle_rad,
                                     int resolution = 128);
    IMGUI_API void DrawConicGradientRect(ImDrawList* drawlist,
                                         ImVec2 min, ImVec2 max,
                                         ImVec2 center_uv,
                                         const ImGradientData& gradient,
                                         float start_angle_rad,
                                         int resolution = 128);

    // D6. Superellipse (generalization of GenShapeSquircle)
    IMGUI_API void GenShapeSuperellipse(ImWidgetsShape& shape,
                                        ImVec2 center,
                                        float rx, float ry,
                                        float nx, float ny,
                                        int sides);

    //////////////////////////////////////////////////////////////////////////
    // Interaction helpers (I1–I3)
    //////////////////////////////////////////////////////////////////////////

    // I1. Angle-wrap/clamp-aware ring slider (thin wrapper exposing flags).
    IMGUI_API bool SliderRingFloatEx(const char* label, float* value,
                                     float v_min, float v_max,
                                     float v_angle_min, float v_angle_max,
                                     float v_thickness,
                                     const char* format,
                                     ImWidgetsSliderFlags widgets_flags,
                                     ImGuiSliderFlags imgui_flags = 0);

    // I2. Delta overlays
    IMGUI_API void PushDeltaOverlay(ImGuiID id, float start_value);
    IMGUI_API void RenderDeltaOverlay(ImGuiID id, float current,
                                      const char* fmt = nullptr);

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
    // New widgets (W1–W7)
    //////////////////////////////////////////////////////////////////////////

    // W2. Vector drawing tool (canvas with zoom/pan + bezier path authoring)
    // Forward declarations — data types are defined outside namespace (see ImVectorDrawingData).
    IMGUI_API bool VectorDrawingTool(const char* label,
                                     struct ImVectorDrawingData& data,
                                     ImVec2 size = ImVec2(0, 0));

    // W3. Envelope / ADSR editors
    IMGUI_API bool EnvelopeEditor(const char* label,
                                  ImEnvelopeData& data,
                                  ImVec2 size = ImVec2(0, 0));
    IMGUI_API bool ADSREditor(const char* label,
                              float* a, float* d, float* s, float* r,
                              ImVec2 size = ImVec2(0, 0));
    IMGUI_API float EnvelopeSample(const ImEnvelopeData& data, float t_seconds);

    // W4. Font inspector (4 modes in one widget; mode is user-controlled)
    IMGUI_API void FontInspector(const char* label,
                                 ImFont* font,
                                 ImFontInspectorMode mode = ImFontInspectorMode_Grid,
                                 float display_size = 64.0f,
                                 ImVec2 size = ImVec2(0, 0));

    // W5. Stepped / notched sliders
    IMGUI_API bool SliderRingSteppedFloat(const char* label, float* v,
                                          float v_min, float v_max,
                                          int step_count,
                                          const char* format = nullptr);
    IMGUI_API bool NotchedDial(const char* label, float* v,
                               const float* stops, int stop_count,
                               const char* const* stop_labels = nullptr);

    // W6. Color-difference visualizer
    IMGUI_API float ColorDeltaE76   (ImVec4 a, ImVec4 b);
    IMGUI_API float ColorDeltaE94   (ImVec4 a, ImVec4 b, float kL = 1.0f, float kC = 1.0f, float kH = 1.0f);
    IMGUI_API float ColorDeltaE2000 (ImVec4 a, ImVec4 b, float kL = 1.0f, float kC = 1.0f, float kH = 1.0f);
    IMGUI_API float ColorDeltaEOK   (ImVec4 a, ImVec4 b);
    IMGUI_API const char* DeltaEFormulaName(ImDeltaEFormula f);
    IMGUI_API const char* DeltaEPerceptualLabel(float de); // "Imperceptible" / "Just noticeable" / ...
    IMGUI_API void ColorDifferenceVisualizer(const char* label,
                                             ImVec4* a, ImVec4* b,
                                             ImDeltaEFormula formula = ImDeltaEFormula_E2000);

    // W7. Equation input box (markdown-style $ / $$ delimited LaTeX)
    IMGUI_API bool EquationInput(const char* label,
                                 char* buf, size_t buf_size,
                                 ImVec2 size = ImVec2(0, 0),
                                 ImWidgetsEquationFlags flags = 0);

    // DPI / scaling helpers have moved to ImPlatform:
    //   ImPlatform_LpToPx(float | ImVec2)
    //   ImPlatform_PxToLp(float | ImVec2)
    //   ImPlatform_LpPxScale()
    // (See extern/ImPlatform/ImPlatform/ImPlatform.h. The scale factor is
    // ImGui::GetStyle().FontScaleDpi so it stays in lockstep with Slug text scaling.)

    //////////////////////////////////////////////////////////////////////////
    // Phase B — Audio meters & scopes
    //////////////////////////////////////////////////////////////////////////

    // B1. Parametric EQ editor
    IMGUI_API bool ParametricEQEditor(const char* label,
                                      ImEQBand* bands, int band_count,
                                      float sample_rate,
                                      float freq_min = 20.0f, float freq_max = 20000.0f,
                                      float gain_min_db = -24.0f, float gain_max_db = 24.0f,
                                      ImVec2 size = ImVec2(0, 0));
    // Evaluate the summed biquad magnitude response (dB) at a given frequency.
    IMGUI_API float EvalParametricEQ_dB(const ImEQBand* bands, int band_count,
                                        float freq, float sample_rate);

    // B2. Spectrum analyzer (bars + peak hold)
    IMGUI_API void SpectrumAnalyzer(const char* label,
                                    const float* magnitudes_linear, int bin_count,
                                    float sample_rate,
                                    float min_db = -96.0f, float max_db = 0.0f,
                                    bool log_freq = true,
                                    ImVec2 size = ImVec2(0, 0));

    // B3. Spectrogram (scrolling waterfall)
    IMGUI_API void Spectrogram(const char* label,
                               ImSpectrogramData& data,
                               float sample_rate,
                               ImVec2 size = ImVec2(0, 0));
    IMGUI_API void SpectrogramPush(ImSpectrogramData& data,
                                   const float* magnitudes_linear, int bin_count);
    IMGUI_API void SpectrogramInitViridis(ImSpectrogramData& data);

    // B4. Audio vectorscope (Lissajous)
    IMGUI_API void AudioVectorscope(const char* label,
                                    const float* stereo_interleaved, int frame_count,
                                    float decay = 0.92f,
                                    ImVec2 size = ImVec2(0, 0));

    // B5. LUFS meter
    IMGUI_API void LUFSMeter(const char* label, const ImLUFSMeter& data,
                             ImVec2 size = ImVec2(0, 0));

    // B6. Phase correlation meter (-1..+1 horizontal needle)
    IMGUI_API void PhaseCorrelationMeter(const char* label, float correlation,
                                         ImVec2 size = ImVec2(0, 0));

    // B7. Goniometer (audio vectorscope rotated 45deg + M/S overlay)
    IMGUI_API void Goniometer(const char* label,
                              const float* stereo_interleaved, int frame_count,
                              float decay = 0.92f,
                              ImVec2 size = ImVec2(0, 0));

    // B8. Compressor transfer curve editor
    IMGUI_API bool CompressorCurveEditor(const char* label, ImCompressorCurve* c,
                                         ImVec2 size = ImVec2(0, 0));
    IMGUI_API float CompressorApply_dB(const ImCompressorCurve& c, float input_db);

    // B9. Oscilloscope with trigger
    IMGUI_API void Oscilloscope(const char* label, ImOscilloscope& o,
                                ImVec2 size = ImVec2(0, 0));
    IMGUI_API void OscilloscopePush(ImOscilloscope& o, const float* samples, int n);

    // B10. Modulated slider ring (mod-range arc overlay)
    IMGUI_API bool SliderRingModulated(const char* label, float* value,
                                       float v_min, float v_max,
                                       float mod_min, float mod_max,
                                       float v_angle_min = -0.75f * IM_PI,
                                       float v_angle_max =  0.75f * IM_PI,
                                       float v_thickness = 0.0f,
                                       const char* format = NULL);

    // B11. Pitch / chord wheel
    IMGUI_API bool PitchWheel(const char* label,
                              int* root_note_0_11,
                              unsigned int* chord_mask_12bits,
                              ImPitchWheelMode mode = ImPitchWheelMode_FifthsCircle,
                              ImVec2 size = ImVec2(0, 0));

    // B12. Tempo tap button
    IMGUI_API bool TempoTap(const char* label, float* bpm_out, float timeout_s = 2.0f);

    //////////////////////////////////////////////////////////////////////////
    // Phase C — Audio authoring
    //////////////////////////////////////////////////////////////////////////

    IMGUI_API bool PianoRoll(const char* label, ImPianoRollData& data,
                             ImVec2 size = ImVec2(0, 0));
    IMGUI_API bool StepSequencer(const char* label, ImStepSequencerData& data,
                                 int current_step = -1,
                                 ImVec2 size = ImVec2(0, 0));
    IMGUI_API bool ModMatrix(const char* label, ImModMatrixData& data,
                             ImVec2 size = ImVec2(0, 0));
    IMGUI_API bool LFODesigner(const char* label, ImLFODesignerData& data,
                               ImVec2 size = ImVec2(0, 0));
    IMGUI_API bool MixerChannelStrip(const char* label, ImMixerChannel& c,
                                     ImVec2 size = ImVec2(60, 240));

    //////////////////////////////////////////////////////////////////////////
    // Phase D — Color grading
    //////////////////////////////////////////////////////////////////////////

    IMGUI_API bool LiftGammaGainWheels(const char* label, ImLiftGammaGain* lgg,
                                       ImVec2 size = ImVec2(0, 0));
    IMGUI_API ImVec4 LiftGammaGainApply(ImVec4 rgb, const ImLiftGammaGain& lgg);

    IMGUI_API bool LoadCubeLUT(ImColorLUT3D& out, const char* filename);
    IMGUI_API bool SaveCubeLUT(const ImColorLUT3D& in, const char* filename);
    IMGUI_API ImVec4 SampleLUT3D(const ImColorLUT3D& lut, ImVec4 rgb_in);
    IMGUI_API void  InitIdentityLUT3D(ImColorLUT3D& out, int size);
    IMGUI_API bool  ColorLUT3DViewer(const char* label, ImColorLUT3D* lut,
                                     ImVec2 size = ImVec2(0, 0));

    //////////////////////////////////////////////////////////////////////////
    // Phase E — LookDev A/B compare
    //////////////////////////////////////////////////////////////////////////

    IMGUI_API bool LookDevCompare(const char* label,
                                  ImTextureID tex_a, ImTextureID tex_b,
                                  ImVec2 a_uv_min, ImVec2 a_uv_max,
                                  ImVec2 b_uv_min, ImVec2 b_uv_max,
                                  ImLookDevState* state,
                                  ImVec2 size = ImVec2(0, 0));

    //////////////////////////////////////////////////////////////////////////
    // Phase G — Volume Slice Viewer (CPU-side slice extraction path).
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
        const float* Voxels;  // w*h*d floats, 0..1 range recommended
        int Width, Height, Depth;
        // View parameters.
        ImVolumeSliceAxis Axis;
        float SliceT;         // 0..1
        float WindowMin, WindowMax;
        float Gamma;
        // Internal — widget reuses a cached 2D texture for display.
        ImTextureID CachedTex;
        int CachedW, CachedH;

        ImVolumeSliceState()
            : Voxels(NULL), Width(0), Height(0), Depth(0),
              Axis(ImVolumeSliceAxis_Z), SliceT(0.5f),
              WindowMin(0.0f), WindowMax(1.0f), Gamma(1.0f),
              CachedTex(NULL), CachedW(0), CachedH(0) {}
    };

    IMGUI_API bool VolumeSliceViewer(const char* label, ImVolumeSliceState* state,
                                     ImVec2 size = ImVec2(0, 0));
    // Utility: generate a test 128x128x128 Perlin-ish volume (caller-owns buffer).
    IMGUI_API void VolumeGenerateTestField(float* out_voxels, int w, int h, int d, float seed = 0.0f);
}
