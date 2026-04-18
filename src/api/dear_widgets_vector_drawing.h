// dear_widgets_vector_drawing.h
// Public types and entry point for the Vector Drawing Tool widget.
// Included by dear_widgets.h so downstream callers only need dear_widgets.h.

#pragma once

// dear_widgets.h must have been included first (this header is strictly
// an extraction of declarations that used to live inside it).
struct ImVec2;
template<typename T> struct ImVector;
typedef unsigned int ImU32;

//////////////////////////////////////////////////////////////////////////
// Vector Drawing Tool data types
//////////////////////////////////////////////////////////////////////////

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
	// V3: per-path line styling. Cap applies to open endpoints; Join to corners.
	// Ignored by the plain Polyline style (ImGui::AddPolyline has no cap/join).
	// Values are ImWidgetsCap_* / ImWidgetsJoin_* from dear_widgets.h.
	int                           Cap;   // ImWidgetsCap_Butt
	int                           Join;  // ImWidgetsJoin_Mitter

	ImVectorDrawingPath()
		: Closed( false ), Color( IM_COL32( 255, 220, 100, 255 ) ), Thickness( 2.0f ),
		  Style( ImVectorDrawingStyle_StrokedBezier ), DashLen( 8.0f ), GapLen( 6.0f ),
		  Cap( 1 /*ImWidgetsCap_Butt*/ ), Join( 1 /*ImWidgetsJoin_Mitter*/ ) {}
};

struct ImVectorDrawingData
{
	ImVector<ImVectorDrawingPath> Paths;
	ImVec2 PanOffset;   // world offset of view origin (screen-space pixels)
	float  Zoom;        // world → screen multiplier
	int    SelectedPath;
	int    SelectedNode;
	int    SelectedHandle;       // 0 = anchor, 1 = in, 2 = out
	ImVector<int> SelectedNodes; // multi-node selection (indices into SelectedPath's Nodes)
	// Authoring state: -1 if not currently creating a new path.
	int    ActivePath;

	ImVectorDrawingData()
		: PanOffset( 0, 0 ), Zoom( 1.0f ), SelectedPath( -1 ), SelectedNode( -1 ),
		  SelectedHandle( 0 ), ActivePath( -1 ) {}
};

namespace ImWidgets {
	// W2. Vector drawing tool (canvas with zoom/pan + bezier path authoring)
	IMGUI_API bool VectorDrawingTool( const char* label,
	                                  struct ImVectorDrawingData& data,
	                                  ImVec2 size = ImVec2( 0, 0 ) );
}
