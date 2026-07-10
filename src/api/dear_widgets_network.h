// dear_widgets_network.h
// Public types and entry points for the Network Graph widget: a read-only,
// hierarchically grouped DAG illustration (neural networks, dataflow graphs).
// Included by dear_widgets.h so downstream callers only need dear_widgets.h.

#pragma once

// dear_widgets.h must have been included first (this header is strictly
// an extraction of declarations that would otherwise live inside it).
struct ImVec2;
template<typename T> struct ImVector;
typedef unsigned int ImU32;

//////////////////////////////////////////////////////////////////////////
// Network Graph data types
//////////////////////////////////////////////////////////////////////////

// One operation box. GroupId indexes ImNetworkGraphData::Groups (-1 = root).
struct ImNetworkGraphNode
{
	char	Label[ 64 ];	// primary text (e.g. op type: "Conv")
	char	Sublabel[ 64 ];	// secondary text (e.g. instance name: "conv1_1")
	ImU32	Color;			// box fill tint
	int		GroupId;		// containing group, -1 = root

	ImNetworkGraphNode() : Color( 0 ), GroupId( -1 ) { Label[ 0 ] = 0; Sublabel[ 0 ] = 0; }
};

// Groups form a tree (Parent = -1 for top-level groups). A group renders as
// a single bigger box at its parent's level; double-click dives into it.
struct ImNetworkGraphGroup
{
	char	Label[ 64 ];
	int		Parent;		// parent group index, -1 = root
	int		OpCount;	// transitive op-node count (badge text "N ops")

	ImNetworkGraphGroup() : Parent( -1 ), OpCount( 0 ) { Label[ 0 ] = 0; }
};

// Directed edge between two op nodes (indices into Nodes). At a given level,
// endpoints inside child groups are re-routed to the child group's box.
struct ImNetworkGraphEdge
{
	int	From;
	int	To;

	ImNetworkGraphEdge() : From( -1 ), To( -1 ) {}
	ImNetworkGraphEdge( int from, int to ) : From( from ), To( to ) {}
};

// One laid-out box at a level: either an op node (IsGroup=false, Id indexes
// Nodes) or a child group (IsGroup=true, Id indexes Groups).
struct ImNetworkGraphLayoutItem
{
	int		Id;
	bool	IsGroup;
	int		Layer;
	ImVec2	Pos;	// top-left, layout space
	ImVec2	Size;

	ImNetworkGraphLayoutItem() : Id( -1 ), IsGroup( false ), Layer( 0 ), Pos( 0, 0 ), Size( 0, 0 ) {}
};

// Level-local edge between two layout items (already re-routed + deduped).
struct ImNetworkGraphLayoutEdge
{
	int	FromItem;
	int	ToItem;

	ImNetworkGraphLayoutEdge() : FromItem( -1 ), ToItem( -1 ) {}
	ImNetworkGraphLayoutEdge( int from, int to ) : FromItem( from ), ToItem( to ) {}
};

// Cached layout of ONE level (the direct children of GroupId).
struct ImNetworkGraphLevelLayout
{
	int									GroupId;	// -1 = root level
	bool								Valid;
	ImVector<ImNetworkGraphLayoutItem>	Items;
	ImVector<ImNetworkGraphLayoutEdge>	Edges;
	ImVec2								Min;		// layout-space bounds
	ImVec2								Max;

	ImNetworkGraphLevelLayout() : GroupId( -1 ), Valid( false ), Min( 0, 0 ), Max( 0, 0 ) {}
};

struct ImNetworkGraphData
{
	ImVector<ImNetworkGraphNode>	Nodes;
	ImVector<ImNetworkGraphGroup>	Groups;
	ImVector<ImNetworkGraphEdge>	Edges;

	// View state
	int		CurrentGroup;	// level being displayed, -1 = root
	ImVec2	Scroll;			// layout-space point at the view's top-left
	float	Zoom;			// layout -> screen multiplier
	int		HoveredNode;	// per-frame, -1 = none
	int		HoveredGroup;	// per-frame, -1 = none
	int		ActivatedNode;	// double-clicked op node this frame, -1 = none
	int		ActivatedGroup;	// double-clicked group box this frame, -1 = none
	bool	FitRequest;		// zoom-to-fit the current level on next draw
	int		LastDrawnGroup;	// internal: level change detection (auto-fit)

	// Per-level computed layout cache (lazily filled by NetworkGraphLayout).
	ImVector<ImNetworkGraphLevelLayout>	LayoutCache;

	ImNetworkGraphData()
		: CurrentGroup( -1 ), Scroll( 0, 0 ), Zoom( 1.0f ),
		  HoveredNode( -1 ), HoveredGroup( -1 ),
		  ActivatedNode( -1 ), ActivatedGroup( -1 ),
		  FitRequest( true ), LastDrawnGroup( -2 ) {}

	void Clear()
	{
		Nodes.clear();
		Groups.clear();
		Edges.clear();
		// ImVector never destructs its elements — free the per-level
		// buffers explicitly (same manual deep-clear pattern as imgui's
		// ImDrawListSplitter channels).
		for ( int i = 0; i < LayoutCache.Size; ++i )
		{
			LayoutCache[ i ].Items.clear();
			LayoutCache[ i ].Edges.clear();
		}
		LayoutCache.clear();
		CurrentGroup   = -1;
		Scroll         = ImVec2( 0, 0 );
		Zoom           = 1.0f;
		HoveredNode    = -1;
		HoveredGroup   = -1;
		ActivatedNode  = -1;
		ActivatedGroup = -1;
		FitRequest     = true;
		LastDrawnGroup = -2;
	}

	// Builder helpers (truncating copies). Return the new element's index.
	int  AddNode ( char const* label, char const* sublabel, ImU32 color, int group_id );
	int  AddGroup( char const* label, int parent_group );
	void AddEdge ( int from, int to );

	// Cached layout for a level; nullptr when not yet computed.
	ImNetworkGraphLevelLayout*			FindLayout( int group_id );
	ImNetworkGraphLevelLayout const*	FindLayout( int group_id ) const;
};

namespace ImWidgets {
	// Compute (and cache) the layered DAG layout of ONE level: the direct
	// child nodes and child groups of `group_id` (-1 = root). Edges whose
	// endpoints lie inside different children are re-routed to the child
	// containing each endpoint and deduped. Longest-path layering +
	// barycenter ordering sweeps + per-layer centered x assignment.
	IMGUI_API void NetworkGraphLayout( ImNetworkGraphData& data, int group_id );

	// Read-only network illustration canvas: rounded node boxes, bezier
	// edges with arrowheads, wheel zoom-to-cursor, left/middle-drag pan,
	// visible-rect culling, LOD (text skipped below ~0.4 zoom, dot boxes
	// below ~0.15), hover highlight + tooltip. Double-click on a group box
	// sets data.ActivatedGroup; on an op node sets data.ActivatedNode (the
	// host pushes/handles levels). Auto-fits when the displayed level
	// changes or data.FitRequest is set. Returns true when something was
	// activated this frame.
	IMGUI_API bool NetworkGraph( char const* str_id, ImNetworkGraphData& data, ImVec2 size = ImVec2( 0, 0 ) );
}
