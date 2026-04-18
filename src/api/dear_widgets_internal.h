// dear_widgets_internal.h
// Cross-translation-unit helpers shared by dear_widgets.cpp and its split
// sibling files (dear_widgets_image_inspector.cpp, dear_widgets_stroke.cpp,
// dear_widgets_vector_drawing.cpp, …).
//
// These declarations used to live as file-statics inside dear_widgets.cpp
// when the siblings were #include'd inline. Now that each sibling is its
// own translation unit they need external-linkage declarations.
// Not part of the public API — downstream callers should not include this.

#pragma once

#include "dear_widgets.h"
#include "imgui_internal.h"  // for ImRect used by the expand helpers

namespace ImWidgets {

// Expand-to-window helpers — used by widgets that offer a "maximize" button.
// WidgetExpandButton returns a pointer to the widget's persistent expanded-state
// bool (stored in window state storage), or NULL when already inside an
// expanded window (to suppress nested expand buttons).
IMGUI_API bool* WidgetExpandButton( ImGuiID widget_id, ImRect const& bb );
IMGUI_API bool  BeginExpandedWindow( char const* label, ImGuiID widget_id, bool* pOpen,
                                     ImVec2 defaultSize = ImVec2( 600, 500 ) );
IMGUI_API void  EndExpandedWindow();

} // namespace ImWidgets
