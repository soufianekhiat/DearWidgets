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

	// Key-drag state persists across frames in the window's state storage. Key
	// ids are host-positional and regenerate every frame, but the host applies
	// no edit while a drag is in flight (MoveKey fires on RELEASE only), so the
	// armed id stays meaningful for the drag's duration.
	ImGuiStorage* storage      = ImGui::GetStateStorage();
	ImGuiID const id_drag_key  = ImGui::GetID( "##tl_drag_key" );
	ImGuiID const id_dragging  = ImGui::GetID( "##tl_dragging" );
	unsigned      drag_key_id  = (unsigned)storage->GetInt( id_drag_key, 0 );
	bool          dragging     = storage->GetInt( id_dragging, 0 ) != 0;

	for ( int i = 0; i < data.Keys.Size; ++i )
	{
		ImTimelineKey const& k = data.Keys[ i ];
		if ( k.TrackIndex < 0 || k.TrackIndex >= data.Tracks.Size ) continue;
		float x = TickToX( k.Tick, data.BeginTick, data.EndTick, tx, tw );
		float y = ry1 + kTrackH * (float)k.TrackIndex + kTrackH * 0.5f;
		ImU32 c = k.Selected ? IM_COL32( 255, 210, 90, 255 ) : IM_COL32( 210, 210, 210, 255 );
		dl->AddNgonFilled( ImVec2( x, y ), kKeyR, c, 4 ); // diamond
		if ( hovered && out_action && !acted && drag_key_id == 0u &&
			 ImFabs( mouse.x - x ) <= kKeyR + 2.0f && ImFabs( mouse.y - y ) <= kKeyR + 2.0f )
		{
			if ( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
			{
				// Select immediately AND arm a potential drag-move.
				out_action->Kind = ImTimelineAction_SelectKey; out_action->KeyId = k.Id; acted = true;
				storage->SetInt( id_drag_key, (int)k.Id );
				drag_key_id = k.Id;
			}
			else if ( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
			{ out_action->Kind = ImTimelineAction_KeyContext; out_action->KeyId = k.Id; acted = true; }
		}
	}

	// Drag-move in flight: ghost the key at the candidate tick; emit MoveKey on
	// release. While armed, empty-space scrubbing is suppressed.
	if ( drag_key_id != 0u )
	{
		ImTimelineKey const* dk = 0;
		for ( int i = 0; i < data.Keys.Size; ++i )
			if ( data.Keys[ i ].Id == drag_key_id ) { dk = &data.Keys[ i ]; break; }
		long long const tick = XToTick( mouse.x, data.BeginTick, data.EndTick, tx, tw );
		if ( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) )
		{
			if ( dragging && dk && out_action && !acted && tick != dk->Tick )
			{
				out_action->Kind = ImTimelineAction_MoveKey;
				out_action->KeyId = drag_key_id;
				out_action->TargetTick = tick;
				out_action->TrackIndex = dk->TrackIndex;
				acted = true;
			}
			storage->SetInt( id_drag_key, 0 );
			storage->SetInt( id_dragging, 0 );
			drag_key_id = 0u; dragging = false;
		}
		else
		{
			if ( !dragging && ImGui::IsMouseDragging( ImGuiMouseButton_Left, 3.0f ) )
			{ dragging = true; storage->SetInt( id_dragging, 1 ); }
			if ( dragging && dk )
			{
				float gx = TickToX( tick, data.BeginTick, data.EndTick, tx, tw );
				float gy = ry1 + kTrackH * (float)dk->TrackIndex + kTrackH * 0.5f;
				dl->AddNgonFilled( ImVec2( gx, gy ), kKeyR, IM_COL32( 255, 210, 90, 160 ), 4 );
				dl->AddLine( ImVec2( gx, ry1 ), ImVec2( gx, p0.y + sz.y ), IM_COL32( 255, 210, 90, 90 ) );
				char buf[ 32 ]; snprintf( buf, sizeof( buf ), "%lld", tick );
				dl->AddText( ImVec2( gx + 6.0f, gy - 8.0f ), IM_COL32( 255, 210, 90, 255 ), buf );
			}
		}
	}

	// Del removes the selected key while the timeline is hovered.
	if ( hovered && out_action && !acted && drag_key_id == 0u &&
		 ImGui::IsKeyPressed( ImGuiKey_Delete ) )
	{
		for ( int i = 0; i < data.Keys.Size; ++i )
		{
			if ( !data.Keys[ i ].Selected ) continue;
			out_action->Kind = ImTimelineAction_DeleteKey;
			out_action->KeyId = data.Keys[ i ].Id;
			acted = true;
			break;
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
	// Suppressed while a key drag is armed — dragging a key must never scrub.
	if ( hovered && out_action && !acted && drag_key_id == 0u && mouse.x >= tx )
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

bool ImTimelineCurve( char const* label,
					  ImTimelineCurveData const& data,
					  ImVec2 const& size,
					  ImTimelineCurveAction* out_action )
{
	if ( out_action )
	{
		out_action->Kind = ImTimelineCurveAction_None;
		out_action->TargetTick = 0; out_action->TargetValue = 0.0; out_action->KeyId = 0u;
	}
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if ( window->SkipItems || data.EndTick <= data.BeginTick || !( data.MaxValue > data.MinValue ) )
		return false;

	ImVec2 avail = ImGui::GetContentRegionAvail();
	ImVec2 sz( size.x > 0.0f ? size.x : avail.x,
			   size.y > 0.0f ? size.y : 110.0f );
	if ( sz.x < kLabelW + 32.0f ) sz.x = kLabelW + 32.0f;
	if ( sz.y < 48.0f ) sz.y = 48.0f;
	ImVec2 p0 = window->DC.CursorPos;
	ImGui::InvisibleButton( label, sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight );
	bool const hovered = ImGui::IsItemHovered();
	ImDrawList* dl = window->DrawList;

	float const tx = p0.x + kLabelW + 4.0f;           // curve area origin
	float const tw = sz.x - kLabelW - 4.0f;
	float const ty = p0.y + 4.0f;                     // top padding
	float const th = sz.y - 8.0f;                     // curve area height

	auto ValueToY = [&]( double v ) -> float
	{
		double t = ( v - data.MinValue ) / ( data.MaxValue - data.MinValue );
		if ( t < 0.0 ) t = 0.0; if ( t > 1.0 ) t = 1.0;
		return ty + th * (float)( 1.0 - t );
	};
	auto YToValue = [&]( float y ) -> double
	{
		double t = 1.0 - (double)( y - ty ) / (double)th;
		if ( t < 0.0 ) t = 0.0; if ( t > 1.0 ) t = 1.0;
		return data.MinValue + t * ( data.MaxValue - data.MinValue );
	};

	// background + gutter + value grid (min / mid / max)
	dl->AddRectFilled( p0, ImVec2( p0.x + sz.x, p0.y + sz.y ), ImGui::GetColorU32( ImGuiCol_FrameBg ) );
	dl->AddRectFilled( p0, ImVec2( p0.x + kLabelW, p0.y + sz.y ), ImGui::GetColorU32( ImGuiCol_TableHeaderBg ) );
	if ( data.Label )
		dl->AddText( ImVec2( p0.x + 6.0f, p0.y + 4.0f ), ImGui::GetColorU32( ImGuiCol_Text ), data.Label );
	double const grid[ 3 ] = { data.MaxValue, ( data.MinValue + data.MaxValue ) * 0.5, data.MinValue };
	for ( int g = 0; g < 3; ++g )
	{
		float y = ValueToY( grid[ g ] );
		dl->AddLine( ImVec2( tx, y ), ImVec2( tx + tw, y ), IM_COL32( 255, 255, 255, 22 ) );
		char buf[ 32 ]; snprintf( buf, sizeof( buf ), "%.4g", grid[ g ] );
		ImVec2 ts = ImGui::CalcTextSize( buf );
		dl->AddText( ImVec2( tx - ts.x - 4.0f > p0.x + 4.0f ? tx - ts.x - 4.0f : p0.x + 4.0f, y - ts.y * 0.5f ),
					 ImGui::GetColorU32( ImGuiCol_TextDisabled ), buf );
	}

	// the curve: a host-sampled polyline when provided (exact for cubic
	// zones), else hold extrapolation before/after and step (hold) or ramp
	// (linear) between keys
	ImU32 const curve_col = IM_COL32( 120, 190, 255, 220 );
	if ( data.Samples.Size > 1 )
	{
		// sub-tick x precision (samples land between integral ticks)
		double const range = (double)( data.EndTick - data.BeginTick );
		for ( int i = 1; i < data.Samples.Size; ++i )
		{
			float ax = tx + tw * (float)( ( (double)data.Samples[ i - 1 ].x - (double)data.BeginTick ) / range );
			float bx = tx + tw * (float)( ( (double)data.Samples[ i ].x - (double)data.BeginTick ) / range );
			dl->AddLine( ImVec2( ax, ValueToY( data.Samples[ i - 1 ].y ) ),
						 ImVec2( bx, ValueToY( data.Samples[ i ].y ) ), curve_col, 1.5f );
		}
	}
	else if ( data.Keys.Size > 0 )
	{
		float x_prev = TickToX( data.BeginTick, data.BeginTick, data.EndTick, tx, tw );
		float y_prev = ValueToY( data.Keys[ 0 ].Value );
		for ( int i = 0; i < data.Keys.Size; ++i )
		{
			float x = TickToX( data.Keys[ i ].Tick, data.BeginTick, data.EndTick, tx, tw );
			float y = ValueToY( data.Keys[ i ].Value );
			if ( i == 0 )
				dl->AddLine( ImVec2( x_prev, y ), ImVec2( x, y ), curve_col, 1.5f );          // pre-hold
			else if ( data.Keys[ i - 1 ].Interp == 1 )
				dl->AddLine( ImVec2( x_prev, y_prev ), ImVec2( x, y ), curve_col, 1.5f );     // ramp
			else
			{
				dl->AddLine( ImVec2( x_prev, y_prev ), ImVec2( x, y_prev ), curve_col, 1.5f ); // step across
				dl->AddLine( ImVec2( x, y_prev ), ImVec2( x, y ), IM_COL32( 120, 190, 255, 110 ) ); // rise
			}
			x_prev = x; y_prev = y;
		}
		dl->AddLine( ImVec2( x_prev, y_prev ),
					 ImVec2( TickToX( data.EndTick - 1, data.BeginTick, data.EndTick, tx, tw ), y_prev ),
					 curve_col, 1.5f ); // post-hold
	}

	// keys + interactions (same drag machinery as ImTimeline, plus the value axis)
	bool acted = false;
	ImVec2 const mouse = ImGui::GetIO().MousePos;
	ImGuiStorage* storage     = ImGui::GetStateStorage();
	ImGuiID const id_drag_key = ImGui::GetID( "##tlc_drag_key" );
	ImGuiID const id_dragging = ImGui::GetID( "##tlc_dragging" );
	ImGuiID const id_tan_key  = ImGui::GetID( "##tlc_tan_key" );
	ImGuiID const id_tan_side = ImGui::GetID( "##tlc_tan_side" ); // 1 = in, 2 = out
	unsigned      drag_key_id = (unsigned)storage->GetInt( id_drag_key, 0 );
	bool          dragging    = storage->GetInt( id_dragging, 0 ) != 0;
	unsigned      tan_key_id  = (unsigned)storage->GetInt( id_tan_key, 0 );
	int           tan_side    = storage->GetInt( id_tan_side, 0 );

	auto XToTickF = [&]( float x ) -> double
	{
		double t = (double)( x - tx ) / (double)tw;
		if ( t < 0.0 ) t = 0.0; if ( t > 1.0 ) t = 1.0;
		return (double)data.BeginTick + t * (double)( data.EndTick - data.BeginTick );
	};

	// Tangent handles for the SELECTED cubic key: a handle each side at a
	// fixed tick offset along the slope. Clicking a handle arms a tangent
	// drag; release emits the new slope. Handles hit-test BEFORE keys.
	float const hdx_ticks = (float)( data.EndTick - data.BeginTick ) * 0.06f + 0.5f;
	for ( int i = 0; i < data.Keys.Size && tan_key_id == 0u && drag_key_id == 0u; ++i )
	{
		ImTimelineCurveKey const& k = data.Keys[ i ];
		if ( !k.Selected || k.Interp != 2 ) continue;
		float kx = TickToX( k.Tick, data.BeginTick, data.EndTick, tx, tw );
		float ky = ValueToY( k.Value );
		float hx_in  = tx + tw * (float)( ( (double)k.Tick - hdx_ticks - (double)data.BeginTick ) / (double)( data.EndTick - data.BeginTick ) );
		float hy_in  = ValueToY( k.Value - k.TanIn * hdx_ticks );
		float hx_out = tx + tw * (float)( ( (double)k.Tick + hdx_ticks - (double)data.BeginTick ) / (double)( data.EndTick - data.BeginTick ) );
		float hy_out = ValueToY( k.Value + k.TanOut * hdx_ticks );
		dl->AddLine( ImVec2( kx, ky ), ImVec2( hx_in, hy_in ), IM_COL32( 255, 160, 90, 180 ) );
		dl->AddLine( ImVec2( kx, ky ), ImVec2( hx_out, hy_out ), IM_COL32( 255, 160, 90, 180 ) );
		dl->AddCircleFilled( ImVec2( hx_in, hy_in ), 3.5f, IM_COL32( 255, 160, 90, 255 ) );
		dl->AddCircleFilled( ImVec2( hx_out, hy_out ), 3.5f, IM_COL32( 255, 160, 90, 255 ) );
		if ( hovered && out_action && !acted && ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
		{
			if ( ImFabs( mouse.x - hx_in ) <= 5.0f && ImFabs( mouse.y - hy_in ) <= 5.0f )
			{ storage->SetInt( id_tan_key, (int)k.Id ); storage->SetInt( id_tan_side, 1 ); tan_key_id = k.Id; tan_side = 1; }
			else if ( ImFabs( mouse.x - hx_out ) <= 5.0f && ImFabs( mouse.y - hy_out ) <= 5.0f )
			{ storage->SetInt( id_tan_key, (int)k.Id ); storage->SetInt( id_tan_side, 2 ); tan_key_id = k.Id; tan_side = 2; }
		}
	}

	// Tangent drag in flight: live slope preview; emit on release.
	if ( tan_key_id != 0u )
	{
		ImTimelineCurveKey const* tk = 0;
		for ( int i = 0; i < data.Keys.Size; ++i )
			if ( data.Keys[ i ].Id == tan_key_id ) { tk = &data.Keys[ i ]; break; }
		double slope = 0.0; bool slope_ok = false;
		if ( tk )
		{
			double const dtick = XToTickF( mouse.x ) - (double)tk->Tick;
			double const dval  = YToValue( mouse.y ) - tk->Value;
			double const mag   = dtick < 0.0 ? -dtick : dtick;
			if ( mag > 0.05 ) { slope = dval / dtick; slope_ok = true; }
		}
		if ( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) )
		{
			if ( tk && out_action && !acted && slope_ok )
			{
				out_action->Kind = tan_side == 1 ? ImTimelineCurveAction_TangentIn
												 : ImTimelineCurveAction_TangentOut;
				out_action->KeyId = tan_key_id;
				out_action->TargetValue = slope;
				acted = true;
			}
			storage->SetInt( id_tan_key, 0 );
			storage->SetInt( id_tan_side, 0 );
			tan_key_id = 0u; tan_side = 0;
		}
		else if ( tk && slope_ok )
		{
			float kx = TickToX( tk->Tick, data.BeginTick, data.EndTick, tx, tw );
			float ky = ValueToY( tk->Value );
			dl->AddLine( ImVec2( kx, ky ), mouse, IM_COL32( 255, 160, 90, 255 ), 1.5f );
			char buf[ 32 ]; snprintf( buf, sizeof( buf ), "%.3g /t", slope );
			dl->AddText( ImVec2( mouse.x + 6.0f, mouse.y - 14.0f ), IM_COL32( 255, 160, 90, 255 ), buf );
		}
	}

	for ( int i = 0; i < data.Keys.Size; ++i )
	{
		ImTimelineCurveKey const& k = data.Keys[ i ];
		float x = TickToX( k.Tick, data.BeginTick, data.EndTick, tx, tw );
		float y = ValueToY( k.Value );
		ImU32 c = k.Selected ? IM_COL32( 255, 210, 90, 255 ) : IM_COL32( 225, 225, 225, 255 );
		dl->AddNgonFilled( ImVec2( x, y ), kKeyR, c, 4 );
		if ( hovered && out_action && !acted && drag_key_id == 0u && tan_key_id == 0u &&
			 ImFabs( mouse.x - x ) <= kKeyR + 3.0f && ImFabs( mouse.y - y ) <= kKeyR + 3.0f )
		{
			if ( ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
			{
				out_action->Kind = ImTimelineCurveAction_SelectKey; out_action->KeyId = k.Id; acted = true;
				storage->SetInt( id_drag_key, (int)k.Id );
				drag_key_id = k.Id;
			}
			else if ( ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
			{ out_action->Kind = ImTimelineCurveAction_KeyContext; out_action->KeyId = k.Id; acted = true; }
		}
	}

	if ( drag_key_id != 0u )
	{
		ImTimelineCurveKey const* dk = 0;
		for ( int i = 0; i < data.Keys.Size; ++i )
			if ( data.Keys[ i ].Id == drag_key_id ) { dk = &data.Keys[ i ]; break; }
		long long const tick  = XToTick( mouse.x, data.BeginTick, data.EndTick, tx, tw );
		double const    value = YToValue( mouse.y );
		if ( !ImGui::IsMouseDown( ImGuiMouseButton_Left ) )
		{
			if ( dragging && dk && out_action && !acted &&
				 ( tick != dk->Tick || value != dk->Value ) )
			{
				out_action->Kind = ImTimelineCurveAction_MoveKey;
				out_action->KeyId = drag_key_id;
				out_action->TargetTick = tick;
				out_action->TargetValue = value;
				acted = true;
			}
			storage->SetInt( id_drag_key, 0 );
			storage->SetInt( id_dragging, 0 );
			drag_key_id = 0u; dragging = false;
		}
		else
		{
			if ( !dragging && ImGui::IsMouseDragging( ImGuiMouseButton_Left, 3.0f ) )
			{ dragging = true; storage->SetInt( id_dragging, 1 ); }
			if ( dragging && dk )
			{
				float gx = TickToX( tick, data.BeginTick, data.EndTick, tx, tw );
				float gy = ValueToY( value );
				dl->AddNgonFilled( ImVec2( gx, gy ), kKeyR, IM_COL32( 255, 210, 90, 160 ), 4 );
				dl->AddLine( ImVec2( gx, ty ), ImVec2( gx, ty + th ), IM_COL32( 255, 210, 90, 70 ) );
				char buf[ 48 ]; snprintf( buf, sizeof( buf ), "%lld : %.4g", tick, value );
				dl->AddText( ImVec2( gx + 6.0f, gy - 16.0f ), IM_COL32( 255, 210, 90, 255 ), buf );
			}
		}
	}

	if ( hovered && out_action && !acted && drag_key_id == 0u && tan_key_id == 0u &&
		 ImGui::IsKeyPressed( ImGuiKey_Delete ) )
	{
		for ( int i = 0; i < data.Keys.Size; ++i )
		{
			if ( !data.Keys[ i ].Selected ) continue;
			out_action->Kind = ImTimelineCurveAction_DeleteKey;
			out_action->KeyId = data.Keys[ i ].Id;
			acted = true;
			break;
		}
	}

	// playhead
	{
		float x = TickToX( data.PlayheadTick, data.BeginTick, data.EndTick, tx, tw );
		dl->AddLine( ImVec2( x, p0.y ), ImVec2( x, p0.y + sz.y ), IM_COL32( 90, 170, 255, 200 ), 2.0f );
	}

	// empty space: double-click inserts; drag scrubs (suppressed mid drag)
	if ( hovered && out_action && !acted && drag_key_id == 0u && tan_key_id == 0u && mouse.x >= tx )
	{
		long long tick = XToTick( mouse.x, data.BeginTick, data.EndTick, tx, tw );
		if ( ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
		{ out_action->Kind = ImTimelineCurveAction_InsertKey; out_action->TargetTick = tick; acted = true; }
		else if ( ImGui::IsMouseDown( ImGuiMouseButton_Left ) && ImGui::IsItemActive() )
		{ out_action->Kind = ImTimelineCurveAction_Scrub; out_action->TargetTick = tick; acted = true; }
	}
	return acted;
}

} // namespace ImWidgets
