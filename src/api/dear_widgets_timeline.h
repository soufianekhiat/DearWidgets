// dear_widgets_timeline.h
// Public types and entry point for the Timeline widget (The Flow M5): the
// authoritative keys/cache visualization for a temporal document. Draws the
// playable tick range with playhead, authored keys, events, cache coverage,
// checkpoints, INVALID ranges and stale retained branches distinctly; supports
// scrubbing and key insertion at arbitrary ticks INCLUDING before computed
// frames. Included by dear_widgets.h so downstream callers only need
// dear_widgets.h.

#pragma once

struct ImVec2;
template<typename T> struct ImVector;
typedef unsigned int ImU32;

//////////////////////////////////////////////////////////////////////////
// Timeline data types (ticks are exact 64-bit; the host owns rational rate)
//////////////////////////////////////////////////////////////////////////

// One authored key on a named track row.
struct ImTimelineKey
{
	long long	Tick;
	int			TrackIndex;   // row in ImTimelineData::Tracks
	unsigned	Id;           // host identity (hit-testing / edits)
	bool		Selected;
};

// A time-stamped event marker (drawn on the ruler, not on a track row).
struct ImTimelineEvent
{
	long long	Tick;
	unsigned	Id;
	ImU32		Color;        // 0 = default
};

// Half-open [BeginTick, EndTick) band drawn across all rows.
// Kind selects the style: cache coverage (subtle fill), invalid (hatched
// warning), stale retained branch (dimmed), checkpoint (thin pin line at
// BeginTick; EndTick ignored).
enum ImTimelineBandKind
{
	ImTimelineBand_CacheCoverage = 0,
	ImTimelineBand_Invalid,
	ImTimelineBand_StaleBranch,
	ImTimelineBand_Checkpoint,
};

struct ImTimelineBand
{
	long long			BeginTick;
	long long			EndTick;
	ImTimelineBandKind	Kind;
};

// A named key row (e.g. one Time Value state, one keyed register).
struct ImTimelineTrack
{
	char const* Label;
};

struct ImTimelineData
{
	long long					BeginTick;     // playable range [Begin, End)
	long long					EndTick;
	long long					PlayheadTick;
	char const*					DeterminismLabel; // active profile (nullable)
	ImVector<ImTimelineTrack>	Tracks;
	ImVector<ImTimelineKey>		Keys;
	ImVector<ImTimelineEvent>	Events;
	ImVector<ImTimelineBand>	Bands;
};

// What the user did this frame (host applies the edit and re-feeds data —
// the widget never mutates ImTimelineData).
enum ImTimelineActionKind
{
	ImTimelineAction_None = 0,
	ImTimelineAction_Scrub,        // TargetTick = requested playhead
	ImTimelineAction_InsertKey,    // TargetTick + TrackIndex (past OK)
	ImTimelineAction_SelectKey,    // KeyId
	ImTimelineAction_DeleteKey,    // KeyId
};

struct ImTimelineAction
{
	ImTimelineActionKind Kind;
	long long			 TargetTick;
	int					 TrackIndex;
	unsigned			 KeyId;
};

namespace ImWidgets
{
	// Draws the timeline in the current window filling `size` (0,0 = avail).
	// Returns true when `out_action` carries a user action for the host.
	bool ImTimeline( char const* label,
					 ImTimelineData const& data,
					 ImVec2 const& size,
					 ImTimelineAction* out_action );
}
