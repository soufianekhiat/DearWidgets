// dear_widgets_network.cpp
// Network Graph widget -- read-only, hierarchically grouped DAG illustration.
// Own translation unit (like the other dear_widgets_* sub-widgets); compiled
// standalone and resolved via the linker (not unity-build #include).

#include "dear_widgets.h"
#include "dear_widgets_internal.h"
#include "imgui_internal.h"

#include <string.h>

//////////////////////////////////////////////////////////////////////////
// ImNetworkGraphData builder helpers
//////////////////////////////////////////////////////////////////////////

static void NG_CopyStr( char* dst, int dst_size, char const* src )
{
	if ( !src )
	{
		dst[ 0 ] = 0;
		return;
	}
	int i = 0;
	for ( ; i < dst_size - 1 && src[ i ]; ++i )
		dst[ i ] = src[ i ];
	dst[ i ] = 0;
}

int ImNetworkGraphData::AddNode( char const* label, char const* sublabel, ImU32 color, int group_id )
{
	ImNetworkGraphNode n;
	NG_CopyStr( n.Label,    IM_ARRAYSIZE( n.Label ),    label );
	NG_CopyStr( n.Sublabel, IM_ARRAYSIZE( n.Sublabel ), sublabel );
	n.Color   = color;
	n.GroupId = group_id;
	Nodes.push_back( n );
	return Nodes.Size - 1;
}

int ImNetworkGraphData::AddGroup( char const* label, int parent_group )
{
	ImNetworkGraphGroup g;
	NG_CopyStr( g.Label, IM_ARRAYSIZE( g.Label ), label );
	g.Parent  = parent_group;
	g.OpCount = 0;
	Groups.push_back( g );
	return Groups.Size - 1;
}

void ImNetworkGraphData::AddEdge( int from, int to )
{
	Edges.push_back( ImNetworkGraphEdge( from, to ) );
}

ImNetworkGraphLevelLayout* ImNetworkGraphData::FindLayout( int group_id )
{
	for ( int i = 0; i < LayoutCache.Size; ++i )
		if ( LayoutCache[ i ].GroupId == group_id )
			return &LayoutCache[ i ];
	return NULL;
}

ImNetworkGraphLevelLayout const* ImNetworkGraphData::FindLayout( int group_id ) const
{
	for ( int i = 0; i < LayoutCache.Size; ++i )
		if ( LayoutCache[ i ].GroupId == group_id )
			return &LayoutCache[ i ];
	return NULL;
}

namespace ImWidgets {

	//==================================================================
	// Layout constants
	//==================================================================
	static const ImVec2 NG_NodeSize   = ImVec2( 150.0f, 46.0f );
	static const ImVec2 NG_GroupSize  = ImVec2( 200.0f, 72.0f );
	static const float  NG_HGap       = 42.0f;
	static const float  NG_VGap       = 70.0f;
	static const float  NG_LodText    = 0.40f;	// below: skip node text
	static const float  NG_LodDots    = 0.15f;	// below: nodes as dots
	static const float  NG_ZoomMin    = 0.02f;
	static const float  NG_ZoomMax    = 4.0f;

	//==================================================================
	// Layout: layered DAG of ONE level
	//==================================================================

	// Resolve the layout item (at level `group_id`) that contains node
	// `node_idx`: the node itself when it sits directly at this level, or
	// the child group whose subtree contains it. -1 when outside the level.
	static int NG_ItemForNode( ImNetworkGraphData const& data, int group_id,
	                           int node_idx,
	                           ImVector<int> const& node_to_item,
	                           ImVector<int> const& group_to_item )
	{
		if ( node_idx < 0 || node_idx >= data.Nodes.Size )
			return -1;
		int g = data.Nodes[ node_idx ].GroupId;
		if ( g == group_id )
			return node_to_item[ node_idx ];
		// Walk up the group chain looking for a DIRECT child of group_id.
		int guard = 0;
		while ( g != -1 && guard++ < 256 )
		{
			if ( data.Groups[ g ].Parent == group_id )
				return group_to_item[ g ];
			g = data.Groups[ g ].Parent;
		}
		return -1;
	}

	void NetworkGraphLayout( ImNetworkGraphData& data, int group_id )
	{
		// Fetch-or-create the cache slot.
		ImNetworkGraphLevelLayout* cache = data.FindLayout( group_id );
		if ( !cache )
		{
			data.LayoutCache.push_back( ImNetworkGraphLevelLayout() );
			cache = &data.LayoutCache.back();
			cache->GroupId = group_id;
		}
		cache->Items.clear();
		cache->Edges.clear();
		cache->Valid = false;

		// ---- 1. Collect this level's items --------------------------------
		ImVector<int> node_to_item;		// node index  -> item index (-1)
		ImVector<int> group_to_item;	// group index -> item index (-1)
		node_to_item.resize( data.Nodes.Size, -1 );
		group_to_item.resize( data.Groups.Size, -1 );

		for ( int i = 0; i < data.Nodes.Size; ++i )
		{
			if ( data.Nodes[ i ].GroupId != group_id )
				continue;
			ImNetworkGraphLayoutItem it;
			it.Id      = i;
			it.IsGroup = false;
			it.Size    = NG_NodeSize;
			node_to_item[ i ] = cache->Items.Size;
			cache->Items.push_back( it );
		}
		for ( int g = 0; g < data.Groups.Size; ++g )
		{
			if ( data.Groups[ g ].Parent != group_id )
				continue;
			ImNetworkGraphLayoutItem it;
			it.Id      = g;
			it.IsGroup = true;
			it.Size    = NG_GroupSize;
			group_to_item[ g ] = cache->Items.Size;
			cache->Items.push_back( it );
		}

		int const n_items = cache->Items.Size;
		if ( n_items == 0 )
		{
			cache->Min = cache->Max = ImVec2( 0, 0 );
			cache->Valid = true;
			return;
		}

		// ---- 2. Re-route + dedup edges to level items ----------------------
		{
			ImVector<ImU64> keys;
			keys.reserve( data.Edges.Size );
			for ( int e = 0; e < data.Edges.Size; ++e )
			{
				int const a = NG_ItemForNode( data, group_id, data.Edges[ e ].From, node_to_item, group_to_item );
				int const b = NG_ItemForNode( data, group_id, data.Edges[ e ].To,   node_to_item, group_to_item );
				if ( a < 0 || b < 0 || a == b )
					continue;
				keys.push_back( ( (ImU64)(ImU32)a << 32 ) | (ImU64)(ImU32)b );
			}
			// Sort + unique
			if ( keys.Size > 1 )
			{
				ImQsort( keys.Data, (size_t)keys.Size, sizeof( ImU64 ),
					[]( void const* lhs, void const* rhs ) -> int {
						ImU64 const a = *(ImU64 const*)lhs;
						ImU64 const b = *(ImU64 const*)rhs;
						return ( a < b ) ? -1 : ( a > b ) ? 1 : 0;
					} );
			}
			ImU64 prev = ~(ImU64)0;
			for ( int k = 0; k < keys.Size; ++k )
			{
				if ( keys[ k ] == prev )
					continue;
				prev = keys[ k ];
				cache->Edges.push_back( ImNetworkGraphLayoutEdge(
					(int)( keys[ k ] >> 32 ), (int)( keys[ k ] & 0xFFFFFFFFu ) ) );
			}
		}

		// ---- 3. Longest-path layering (Kahn; cycle-safe) --------------------
		ImVector<int> layer;
		ImVector<int> in_deg;
		layer.resize( n_items, 0 );
		in_deg.resize( n_items, 0 );
		for ( int e = 0; e < cache->Edges.Size; ++e )
			in_deg[ cache->Edges[ e ].ToItem ]++;

		ImVector<int> queue;
		queue.reserve( n_items );
		for ( int i = 0; i < n_items; ++i )
			if ( in_deg[ i ] == 0 )
				queue.push_back( i );

		int  processed = 0;
		int  max_layer = 0;
		{
			ImVector<int> in_left = in_deg;
			int head = 0;
			while ( head < queue.Size )
			{
				int const cur = queue[ head++ ];
				processed++;
				for ( int e = 0; e < cache->Edges.Size; ++e )
				{
					if ( cache->Edges[ e ].FromItem != cur )
						continue;
					int const to = cache->Edges[ e ].ToItem;
					if ( layer[ to ] < layer[ cur ] + 1 )
						layer[ to ] = layer[ cur ] + 1;
					if ( --in_left[ to ] == 0 )
						queue.push_back( to );
				}
				if ( layer[ cur ] > max_layer )
					max_layer = layer[ cur ];
			}
			// Cycle leftovers (possible after group re-routing): park them one
			// layer below everything processed so they stay visible.
			if ( processed < n_items )
			{
				ImVector<bool> seen;
				seen.resize( n_items, false );
				for ( int q = 0; q < queue.Size; ++q )
					seen[ queue[ q ] ] = true;
				for ( int i = 0; i < n_items; ++i )
					if ( !seen[ i ] )
						layer[ i ] = max_layer + 1;
				max_layer += 1;
			}
		}
		for ( int i = 0; i < n_items; ++i )
			cache->Items[ i ].Layer = layer[ i ];

		// ---- 4. Order within layers: barycenter sweeps ----------------------
		// Rows are stored FLAT (ImVector must not nest: it neither constructs
		// nor destructs elements): rows[row_start[l] .. row_start[l]+count].
		int const n_layers = max_layer + 1;
		ImVector<int> row_count;
		ImVector<int> row_start;
		ImVector<int> rows;
		row_count.resize( n_layers, 0 );
		row_start.resize( n_layers, 0 );
		rows.resize( n_items, 0 );
		for ( int i = 0; i < n_items; ++i )
			row_count[ layer[ i ] ]++;
		for ( int l = 1; l < n_layers; ++l )
			row_start[ l ] = row_start[ l - 1 ] + row_count[ l - 1 ];
		{
			ImVector<int> fill = row_start;
			for ( int i = 0; i < n_items; ++i )
				rows[ fill[ layer[ i ] ]++ ] = i;
		}

		ImVector<float> order_pos;	// item -> position key within its layer
		order_pos.resize( n_items, 0.0f );
		for ( int l = 0; l < n_layers; ++l )
			for ( int k = 0; k < row_count[ l ]; ++k )
				order_pos[ rows[ row_start[ l ] + k ] ] = (float)k;

		int const n_sweeps = 5;
		for ( int sweep = 0; sweep < n_sweeps; ++sweep )
		{
			bool const down = ( sweep % 2 ) == 0;	// down: order by predecessors
			for ( int li = 0; li < n_layers; ++li )
			{
				int const l   = down ? li : ( n_layers - 1 - li );
				int* const row = rows.Data + row_start[ l ];
				int  const cnt_row = row_count[ l ];
				// Barycenter of each item's neighbors in the fixed adjacent layer.
				for ( int k = 0; k < cnt_row; ++k )
				{
					int const item = row[ k ];
					float sum = 0.0f;
					int   cnt = 0;
					for ( int e = 0; e < cache->Edges.Size; ++e )
					{
						int other = -1;
						if ( down && cache->Edges[ e ].ToItem == item )
							other = cache->Edges[ e ].FromItem;
						else if ( !down && cache->Edges[ e ].FromItem == item )
							other = cache->Edges[ e ].ToItem;
						if ( other < 0 )
							continue;
						sum += order_pos[ other ];
						cnt += 1;
					}
					if ( cnt > 0 )
						order_pos[ item ] = sum / (float)cnt;
				}
				// Re-sort the row by the new keys, then re-normalize keys.
				if ( cnt_row > 1 )
				{
					// insertion sort (rows are small; stable enough)
					for ( int a = 1; a < cnt_row; ++a )
					{
						int const v = row[ a ];
						int b = a - 1;
						while ( b >= 0 && ( order_pos[ row[ b ] ] > order_pos[ v ] ||
						        ( order_pos[ row[ b ] ] == order_pos[ v ] && row[ b ] > v ) ) )
						{
							row[ b + 1 ] = row[ b ];
							--b;
						}
						row[ b + 1 ] = v;
					}
				}
				for ( int k = 0; k < cnt_row; ++k )
					order_pos[ row[ k ] ] = (float)k;
			}
		}

		// ---- 5. Coordinates: per-layer centered x, cumulative y -------------
		float y = 0.0f;
		cache->Min = ImVec2( +FLT_MAX, +FLT_MAX );
		cache->Max = ImVec2( -FLT_MAX, -FLT_MAX );
		for ( int l = 0; l < n_layers; ++l )
		{
			int const* row = rows.Data + row_start[ l ];
			int const  cnt_row = row_count[ l ];
			float row_w = 0.0f;
			float row_h = 0.0f;
			for ( int k = 0; k < cnt_row; ++k )
			{
				row_w += cache->Items[ row[ k ] ].Size.x;
				if ( k > 0 )
					row_w += NG_HGap;
				if ( cache->Items[ row[ k ] ].Size.y > row_h )
					row_h = cache->Items[ row[ k ] ].Size.y;
			}
			float x = -0.5f * row_w;
			for ( int k = 0; k < cnt_row; ++k )
			{
				ImNetworkGraphLayoutItem& it = cache->Items[ row[ k ] ];
				it.Pos = ImVec2( x, y + 0.5f * ( row_h - it.Size.y ) );
				x += it.Size.x + NG_HGap;
				cache->Min = ImMin( cache->Min, it.Pos );
				cache->Max = ImMax( cache->Max, it.Pos + it.Size );
			}
			y += row_h + NG_VGap;
		}
		cache->Valid = true;
	}

	//==================================================================
	// Rendering + interaction
	//==================================================================

	static ImVec2 NG_ToScreen( ImVec2 const& origin, ImVec2 const& scroll, float zoom, ImVec2 const& p )
	{
		return ImVec2( origin.x + ( p.x - scroll.x ) * zoom,
		               origin.y + ( p.y - scroll.y ) * zoom );
	}

	static void NG_FitLevel( ImNetworkGraphData& data, ImNetworkGraphLevelLayout const& lvl, ImVec2 const& size )
	{
		float const pad = 60.0f;
		float const bw  = ImMax( lvl.Max.x - lvl.Min.x, 1.0f ) + 2.0f * pad;
		float const bh  = ImMax( lvl.Max.y - lvl.Min.y, 1.0f ) + 2.0f * pad;
		float zoom = ImMin( size.x / bw, size.y / bh );
		zoom = ImClamp( zoom, NG_ZoomMin, 1.5f );
		data.Zoom = zoom;
		ImVec2 const center = ImVec2( 0.5f * ( lvl.Min.x + lvl.Max.x ),
		                              0.5f * ( lvl.Min.y + lvl.Max.y ) );
		data.Scroll = ImVec2( center.x - 0.5f * size.x / zoom,
		                      center.y - 0.5f * size.y / zoom );
	}

	bool NetworkGraph( char const* str_id, ImNetworkGraphData& data, ImVec2 size )
	{
		data.HoveredNode    = -1;
		data.HoveredGroup   = -1;
		data.ActivatedNode  = -1;
		data.ActivatedGroup = -1;

		if ( size.x <= 0.0f )
			size.x = ImGui::GetContentRegionAvail().x;
		if ( size.y <= 0.0f )
			size.y = ImGui::GetContentRegionAvail().y;
		if ( size.x < 16.0f || size.y < 16.0f )
			return false;

		bool activated = false;

		ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
		bool const open = ImGui::BeginChild( str_id, size, ImGuiChildFlags_Borders,
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoMove );
		ImGui::PopStyleVar();
		if ( !open )
		{
			ImGui::EndChild();
			return false;
		}

		ImGuiIO&    io = ImGui::GetIO();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 const p0 = ImGui::GetCursorScreenPos();
		ImVec2 const p1 = ImVec2( p0.x + size.x, p0.y + size.y );

		// Whole-canvas interaction plate (pan / wheel / double-click on bg).
		ImGui::SetCursorScreenPos( p0 );
		ImGui::InvisibleButton( "##ng_canvas", size,
			ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonMiddle );
		bool const canvas_hovered = ImGui::IsItemHovered();
		bool const canvas_active  = ImGui::IsItemActive();

		// Layout (compute-on-demand for the displayed level).
		ImNetworkGraphLevelLayout* lvl = data.FindLayout( data.CurrentGroup );
		if ( !lvl || !lvl->Valid )
		{
			NetworkGraphLayout( data, data.CurrentGroup );
			lvl = data.FindLayout( data.CurrentGroup );
		}
		if ( !lvl )
		{
			ImGui::EndChild();
			return false;
		}

		// Auto-fit on first show of a level / explicit request.
		if ( data.FitRequest || data.LastDrawnGroup != data.CurrentGroup )
		{
			NG_FitLevel( data, *lvl, size );
			data.FitRequest    = false;
			data.LastDrawnGroup = data.CurrentGroup;
		}

		// Pan (left or middle drag).
		if ( canvas_active &&
		     ( ImGui::IsMouseDragging( ImGuiMouseButton_Left, 1.0f ) ||
		       ImGui::IsMouseDragging( ImGuiMouseButton_Middle, 1.0f ) ) )
		{
			data.Scroll.x -= io.MouseDelta.x / data.Zoom;
			data.Scroll.y -= io.MouseDelta.y / data.Zoom;
		}

		// Wheel zoom-to-cursor.
		if ( canvas_hovered && io.MouseWheel != 0.0f )
		{
			float const new_zoom = ImClamp( data.Zoom * ImPow( 1.15f, io.MouseWheel ), NG_ZoomMin, NG_ZoomMax );
			if ( new_zoom != data.Zoom )
			{
				ImVec2 const mouse_rel = ImVec2( io.MousePos.x - p0.x, io.MousePos.y - p0.y );
				// keep the layout point under the cursor fixed
				data.Scroll.x += mouse_rel.x / data.Zoom - mouse_rel.x / new_zoom;
				data.Scroll.y += mouse_rel.y / data.Zoom - mouse_rel.y / new_zoom;
				data.Zoom = new_zoom;
			}
		}

		float const zoom = data.Zoom;
		dl->PushClipRect( p0, p1, true );

		// Background
		ImU32 const col_bg     = ImGui::GetColorU32( ImGuiCol_ChildBg, 1.0f );
		ImU32 const col_grid   = ImGui::GetColorU32( ImGuiCol_Border, 0.30f );
		ImU32 const col_text   = ImGui::GetColorU32( ImGuiCol_Text );
		ImU32 const col_sub    = ImGui::GetColorU32( ImGuiCol_TextDisabled );
		ImU32 const col_edge   = ImGui::GetColorU32( ImGuiCol_Text, 0.35f );
		ImU32 const col_border = ImGui::GetColorU32( ImGuiCol_Border, 0.9f );
		ImU32 const col_hover  = ImGui::GetColorU32( ImGuiCol_NavCursor, 1.0f );
		IM_UNUSED( col_bg );

		// Sparse dot grid (cheap orientation cue), only at readable zooms.
		if ( zoom > 0.10f )
		{
			float const step = 64.0f * zoom;
			if ( step > 9.0f )
			{
				float const ox = p0.x - fmodf( data.Scroll.x * zoom, step );
				float const oy = p0.y - fmodf( data.Scroll.y * zoom, step );
				for ( float gy = oy; gy < p1.y; gy += step )
					for ( float gx = ox; gx < p1.x; gx += step )
						dl->AddCircleFilled( ImVec2( gx, gy ), 1.0f, col_grid, 4 );
			}
		}

		// ---- Edges (under the boxes) ---------------------------------------
		for ( int e = 0; e < lvl->Edges.Size; ++e )
		{
			ImNetworkGraphLayoutItem const& a = lvl->Items[ lvl->Edges[ e ].FromItem ];
			ImNetworkGraphLayoutItem const& b = lvl->Items[ lvl->Edges[ e ].ToItem ];
			ImVec2 const from = NG_ToScreen( p0, data.Scroll, zoom,
				ImVec2( a.Pos.x + 0.5f * a.Size.x, a.Pos.y + a.Size.y ) );
			ImVec2 const to   = NG_ToScreen( p0, data.Scroll, zoom,
				ImVec2( b.Pos.x + 0.5f * b.Size.x, b.Pos.y ) );
			// Cull: both endpoints far outside on the same side.
			if ( ( from.x < p0.x && to.x < p0.x ) || ( from.x > p1.x && to.x > p1.x ) ||
			     ( from.y < p0.y && to.y < p0.y ) || ( from.y > p1.y && to.y > p1.y ) )
				continue;
			if ( zoom < NG_LodDots )
			{
				dl->AddLine( from, to, col_edge, 1.0f );
				continue;
			}
			float const tang = ImClamp( ( to.y - from.y ) * 0.45f, 12.0f * zoom, 160.0f * zoom );
			dl->AddBezierCubic( from, ImVec2( from.x, from.y + tang ),
			                    ImVec2( to.x, to.y - tang ), to,
			                    col_edge, ImMax( 1.0f, 1.6f * zoom ) );
			// Arrowhead (entering the target from above).
			float const ah = ImMax( 3.0f, 6.0f * zoom );
			dl->AddTriangleFilled(
				ImVec2( to.x, to.y ),
				ImVec2( to.x - 0.55f * ah, to.y - ah ),
				ImVec2( to.x + 0.55f * ah, to.y - ah ),
				col_edge );
		}

		// ---- Boxes -----------------------------------------------------------
		ImFont* font = ImGui::GetFont();
		int hovered_item = -1;
		for ( int i = 0; i < lvl->Items.Size; ++i )
		{
			ImNetworkGraphLayoutItem const& it = lvl->Items[ i ];
			ImVec2 const bmin = NG_ToScreen( p0, data.Scroll, zoom, it.Pos );
			ImVec2 const bmax = NG_ToScreen( p0, data.Scroll, zoom, it.Pos + it.Size );
			// Visible-rect culling.
			if ( bmax.x < p0.x || bmin.x > p1.x || bmax.y < p0.y || bmin.y > p1.y )
				continue;

			bool const hovered = canvas_hovered &&
				io.MousePos.x >= bmin.x && io.MousePos.x <= bmax.x &&
				io.MousePos.y >= bmin.y && io.MousePos.y <= bmax.y;
			if ( hovered )
				hovered_item = i;

			if ( zoom < NG_LodDots )
			{
				// Dot LOD: tint-filled mini rect, no text.
				ImU32 fill = it.IsGroup ? ImGui::GetColorU32( ImGuiCol_Header, 0.9f )
				                        : data.Nodes[ it.Id ].Color;
				if ( fill == 0 )
					fill = ImGui::GetColorU32( ImGuiCol_Button, 0.9f );
				dl->AddRectFilled( bmin, bmax, fill, 1.0f );
				continue;
			}

			float const round = ( it.IsGroup ? 8.0f : 6.0f ) * ImClamp( zoom, 0.2f, 1.0f );
			if ( it.IsGroup )
			{
				ImNetworkGraphGroup const& grp = data.Groups[ it.Id ];
				// "Stack" hint: two offset ghost rects behind the box.
				ImVec2 const off = ImVec2( 4.0f * zoom, 4.0f * zoom );
				ImU32 const ghost = ImGui::GetColorU32( ImGuiCol_Header, 0.35f );
				dl->AddRectFilled( bmin + off + off, bmax + off + off, ghost, round );
				dl->AddRectFilled( bmin + off, bmax + off, ghost, round );
				dl->AddRectFilled( bmin, bmax, ImGui::GetColorU32( ImGuiCol_Header, 0.95f ), round );
				dl->AddRect( bmin, bmax, hovered ? col_hover : col_border, round, 0,
				             hovered ? 2.5f : 1.5f );
				if ( zoom >= NG_LodText )
				{
					float const fs  = ImClamp( 16.0f * zoom, 8.0f, 28.0f );
					float const fs2 = ImClamp( 12.0f * zoom, 7.0f, 20.0f );
					ImVec4 const clip = ImVec4( bmin.x + 2, bmin.y, bmax.x - 2, bmax.y );
					ImVec2 const ts = font->CalcTextSizeA( fs, FLT_MAX, 0.0f, grp.Label );
					dl->AddText( font, fs,
						ImVec2( 0.5f * ( bmin.x + bmax.x ) - 0.5f * ts.x,
						        bmin.y + 0.22f * ( bmax.y - bmin.y ) - 0.5f * fs ),
						col_text, grp.Label, NULL, 0.0f, &clip );
					char badge[ 32 ];
					ImFormatString( badge, IM_ARRAYSIZE( badge ), "%d ops", grp.OpCount );
					ImVec2 const bs = font->CalcTextSizeA( fs2, FLT_MAX, 0.0f, badge );
					dl->AddText( font, fs2,
						ImVec2( 0.5f * ( bmin.x + bmax.x ) - 0.5f * bs.x,
						        bmin.y + 0.68f * ( bmax.y - bmin.y ) - 0.5f * fs2 ),
						col_sub, badge, NULL, 0.0f, &clip );
				}
			}
			else
			{
				ImNetworkGraphNode const& node = data.Nodes[ it.Id ];
				ImU32 fill = node.Color;
				if ( fill == 0 )
					fill = ImGui::GetColorU32( ImGuiCol_Button, 0.9f );
				dl->AddRectFilled( bmin, bmax, fill, round );
				dl->AddRect( bmin, bmax, hovered ? col_hover : col_border, round, 0,
				             hovered ? 2.5f : 1.0f );
				if ( zoom >= NG_LodText )
				{
					float const fs  = ImClamp( 14.0f * zoom, 8.0f, 24.0f );
					float const fs2 = ImClamp( 10.5f * zoom, 7.0f, 18.0f );
					ImVec4 const clip = ImVec4( bmin.x + 2, bmin.y, bmax.x - 2, bmax.y );
					ImVec2 const ts = font->CalcTextSizeA( fs, FLT_MAX, 0.0f, node.Label );
					bool const has_sub = node.Sublabel[ 0 ] != 0;
					dl->AddText( font, fs,
						ImVec2( 0.5f * ( bmin.x + bmax.x ) - 0.5f * ts.x,
						        bmin.y + ( has_sub ? 0.30f : 0.50f ) * ( bmax.y - bmin.y ) - 0.5f * fs ),
						col_text, node.Label, NULL, 0.0f, &clip );
					if ( has_sub )
					{
						ImVec2 const ss = font->CalcTextSizeA( fs2, FLT_MAX, 0.0f, node.Sublabel );
						dl->AddText( font, fs2,
							ImVec2( 0.5f * ( bmin.x + bmax.x ) - 0.5f * ss.x,
							        bmin.y + 0.72f * ( bmax.y - bmin.y ) - 0.5f * fs2 ),
							col_sub, node.Sublabel, NULL, 0.0f, &clip );
					}
				}
			}
		}

		dl->PopClipRect();

		// ---- Hover feedback + activation ------------------------------------
		if ( hovered_item >= 0 )
		{
			ImNetworkGraphLayoutItem const& it = lvl->Items[ hovered_item ];
			if ( it.IsGroup )
			{
				data.HoveredGroup = it.Id;
				ImGui::SetTooltip( "%s\n%d ops -- double-click to open",
					data.Groups[ it.Id ].Label, data.Groups[ it.Id ].OpCount );
			}
			else
			{
				data.HoveredNode = it.Id;
				if ( data.Nodes[ it.Id ].Sublabel[ 0 ] )
					ImGui::SetTooltip( "%s\n%s", data.Nodes[ it.Id ].Label, data.Nodes[ it.Id ].Sublabel );
				else
					ImGui::SetTooltip( "%s", data.Nodes[ it.Id ].Label );
			}

			if ( canvas_hovered && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
			{
				if ( it.IsGroup )
					data.ActivatedGroup = it.Id;
				else
					data.ActivatedNode = it.Id;
				activated = true;
			}
		}

		ImGui::EndChild();
		return activated;
	}

} // namespace ImWidgets
