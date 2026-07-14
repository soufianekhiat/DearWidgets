// dear_widgets_timeline.cpp
// Timeline widget (The Flow M5) -- authoritative keys/cache visualization.
// Own translation unit (like the other dear_widgets_* sub-widgets); compiled
// standalone and resolved via the linker (not unity-build #include).

#include "dear_widgets.h"
#include "dear_widgets_internal.h"
#include "imgui_internal.h"

#include <stdio.h>

namespace
{
	constexpr float kRulerH   = 22.0f; // tick ruler + events strip
	constexpr float kTrackH   = 20.0f;
	constexpr float kLabelW   = 120.0f;
	constexpr float kKeyR     = 4.0f;

	float TickToX( long long tick, long long b, long long e, float x0, float w )
	{
		if ( e <= b ) return x0;
		return x0 + w * (float)( (double)( tick - b ) / (double)( e - b ) );
	}
	long long XToTick( float x, long long b, long long e, float x0, float w )
	{
		if ( w <= 0.0f || e <= b ) return b;
		double t = (double)( x - x0 ) / (double)w;
		if ( t < 0.0 ) t = 0.0;
		if ( t > 1.0 ) t = 1.0;
		long long tick = b + (long long)( t * (double)( e - b ) + 0.5 );
		return tick >= e ? e - 1 : tick;
	}
}

namespace ImWidgets
{

bool ImTimeline( char const* label,
				 ImTimelineData const& data,
				 ImVec2 const& size,
				 ImTimelineAction* out_action )
{
	if ( out_action )
	{
		out_action->Kind = ImTimelineAction_None;
		out_action->TargetTick = 0; out_action->TrackIndex = -1; out_action->KeyId = 0u;
	}
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if ( window->SkipItems || data.EndTick <= data.BeginTick )
		return false;

	ImVec2 avail = ImGui::GetContentRegionAvail();
	ImVec2 sz( size.x > 0.0f ? size.x : avail.x,
			   size.y > 0.0f ? size.y
							 : kRulerH + kTrackH * ( data.Tracks.Size > 0 ? data.Tracks.Size : 1 ) + 4.0f );
	// A collapsed/zero-width host region must not assert in InvisibleButton.
	if ( sz.x < kLabelW + 32.0f ) sz.x = kLabelW + 32.0f;
	if ( sz.y < kRulerH + kTrackH ) sz.y = kRulerH + kTrackH;
	ImVec2 p0 = window->DC.CursorPos;
	ImGui::InvisibleButton( label, sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight );
	bool const hovered = ImGui::IsItemHovered();
	ImDrawList* dl = window->DrawList;

	float const lx = p0.x, lw = kLabelW;              // label gutter
	float const tx = p0.x + kLabelW + 4.0f;           // timeline origin
	float const tw = sz.x - kLabelW - 4.0f;           // timeline width
	float const ry1 = p0.y + kRulerH;                 // ruler bottom

	// background + gutter
	dl->AddRectFilled( p0, ImVec2( p0.x + sz.x, p0.y + sz.y ), ImGui::GetColorU32( ImGuiCol_FrameBg ) );
	dl->AddRectFilled( ImVec2( lx, p0.y ), ImVec2( lx + lw, p0.y + sz.y ), ImGui::GetColorU32( ImGuiCol_TableHeaderBg ) );

	// bands (behind everything else); checkpoints draw as pins after
	for ( int i = 0; i < data.Bands.Size; ++i )
	{
		ImTimelineBand const& b = data.Bands[ i ];
		float xb = TickToX( b.BeginTick < data.BeginTick ? data.BeginTick : b.BeginTick, data.BeginTick, data.EndTick, tx, tw );
		float xe = TickToX( b.EndTick > data.EndTick ? data.EndTick : b.EndTick, data.BeginTick, data.EndTick, tx, tw );
		switch ( b.Kind )
		{
		case ImTimelineBand_CacheCoverage:
			dl->AddRectFilled( ImVec2( xb, ry1 ), ImVec2( xe, p0.y + sz.y ), IM_COL32( 64, 160, 96, 40 ) );
			break;
		case ImTimelineBand_CommittedHistory:
			dl->AddRectFilled( ImVec2( xb, ry1 ), ImVec2( xe, p0.y + sz.y ), IM_COL32( 96, 128, 192, 36 ) );
			break;
		case ImTimelineBand_Invalid:
			dl->AddRectFilled( ImVec2( xb, ry1 ), ImVec2( xe, p0.y + sz.y ), IM_COL32( 200, 80, 48, 46 ) );
			for ( float hx = xb; hx < xe; hx += 8.0f ) // hatching
				dl->AddLine( ImVec2( hx, p0.y + sz.y ), ImVec2( hx + 6.0f, ry1 ), IM_COL32( 200, 80, 48, 90 ) );
			break;
		case ImTimelineBand_StaleBranch:
			dl->AddRectFilled( ImVec2( xb, ry1 ), ImVec2( xe, p0.y + sz.y ), IM_COL32( 128, 128, 128, 36 ) );
			break;
		case ImTimelineBand_Checkpoint:
			dl->AddLine( ImVec2( xb, p0.y ), ImVec2( xb, p0.y + sz.y ), IM_COL32( 240, 200, 64, 200 ), 2.0f );
			dl->AddTriangleFilled( ImVec2( xb - 4.0f, p0.y ), ImVec2( xb + 4.0f, p0.y ), ImVec2( xb, p0.y + 6.0f ), IM_COL32( 240, 200, 64, 220 ) );
			break;
		}
	}

	// ruler line + coarse tick marks
	dl->AddLine( ImVec2( tx, ry1 ), ImVec2( tx + tw, ry1 ), ImGui::GetColorU32( ImGuiCol_Border ) );
	long long const range = data.EndTick - data.BeginTick;
	long long step = 1; while ( range / step > 20 ) step *= ( step % 3 == 0 ) ? 5 : 2;
	for ( long long t = data.BeginTick; t < data.EndTick; t += step )
	{
		float x = TickToX( t, data.BeginTick, data.EndTick, tx, tw );
		dl->AddLine( ImVec2( x, ry1 - 5.0f ), ImVec2( x, ry1 ), ImGui::GetColorU32( ImGuiCol_Border ) );
		char buf[ 32 ]; snprintf( buf, sizeof( buf ), "%lld", t );
		dl->AddText( ImVec2( x + 2.0f, p0.y + 2.0f ), ImGui::GetColorU32( ImGuiCol_TextDisabled ), buf );
	}

	// events on the ruler
	for ( int i = 0; i < data.Events.Size; ++i )
	{
		float x = TickToX( data.Events[ i ].Tick, data.BeginTick, data.EndTick, tx, tw );
		ImU32 c = data.Events[ i ].Color ? data.Events[ i ].Color : IM_COL32( 120, 180, 240, 255 );
		dl->AddTriangleFilled( ImVec2( x - 4.0f, ry1 - 1.0f ), ImVec2( x + 4.0f, ry1 - 1.0f ), ImVec2( x, ry1 - 8.0f ), c );
	}

	// track rows + keys
	bool acted = false;
	ImVec2 const mouse = ImGui::GetIO().MousePos;
	for ( int r = 0; r < data.Tracks.Size; ++r )
	{
		float y0 = ry1 + kTrackH * (float)r, y1 = y0 + kTrackH;
		if ( r & 1 )
			dl->AddRectFilled( ImVec2( tx, y0 ), ImVec2( tx + tw, y1 ), IM_COL32( 255, 255, 255, 8 ) );
		if ( data.Tracks[ r ].Label )
			dl->AddText( ImVec2( lx + 6.0f, y0 + 3.0f ), ImGui::GetColorU32( ImGuiCol_Text ), data.Tracks[ r ].Label );
	}
	for ( int i = 0; i < data.Keys.Size; ++i )
	{
		ImTimelineKey const& k = data.Keys[ i ];
		if ( k.TrackIndex < 0 || k.TrackIndex >= data.Tracks.Size ) continue;
		float x = TickToX( k.Tick, data.BeginTick, data.EndTick, tx, tw );
		float y = ry1 + kTrackH * (float)k.TrackIndex + kTrackH * 0.5f;
		ImU32 c = k.Selected ? IM_COL32( 255, 210, 90, 255 ) : IM_COL32( 210, 210, 210, 255 );
		dl->AddNgonFilled( ImVec2( x, y ), kKeyR, c, 4 ); // diamond
		if ( hovered && out_action && !acted &&
			 ImFabs( mouse.x - x ) <= kKeyR + 2.0f && ImFabs( mouse.y - y ) <= kKeyR + 2.0f )
		{
			if ( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
			{ out_action->Kind = ImTimelineAction_SelectKey; out_action->KeyId = k.Id; acted = true; }
			else if ( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
			{ out_action->Kind = ImTimelineAction_DeleteKey; out_action->KeyId = k.Id; acted = true; }
		}
	}

	// playhead (over everything)
	{
		float x = TickToX( data.PlayheadTick, data.BeginTick, data.EndTick, tx, tw );
		dl->AddLine( ImVec2( x, p0.y ), ImVec2( x, p0.y + sz.y ), IM_COL32( 90, 170, 255, 255 ), 2.0f );
	}

	// determinism profile label (top-right)
	if ( data.DeterminismLabel )
	{
		ImVec2 ts = ImGui::CalcTextSize( data.DeterminismLabel );
		dl->AddText( ImVec2( p0.x + sz.x - ts.x - 6.0f, p0.y + 2.0f ), ImGui::GetColorU32( ImGuiCol_TextDisabled ), data.DeterminismLabel );
	}

	// interactions on empty space: left drag/click on ruler or rows = scrub;
	// double-click on a row = insert key at that tick (PAST times included).
	if ( hovered && out_action && !acted && mouse.x >= tx )
	{
		long long tick = XToTick( mouse.x, data.BeginTick, data.EndTick, tx, tw );
		if ( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) && mouse.y >= ry1 )
		{
			int row = (int)( ( mouse.y - ry1 ) / kTrackH );
			if ( row >= 0 && row < data.Tracks.Size )
			{ out_action->Kind = ImTimelineAction_InsertKey; out_action->TargetTick = tick; out_action->TrackIndex = row; acted = true; }
		}
		else if ( ImGui::IsMouseDown( ImGuiMouseButton_Left ) && ImGui::IsItemActive() )
		{ out_action->Kind = ImTimelineAction_Scrub; out_action->TargetTick = tick; acted = true; }
	}
	return acted;
}

} // namespace ImWidgets
