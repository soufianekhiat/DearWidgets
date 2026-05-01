# Expand-to-Window

Each widget has a small expand button in its top-right corner. Clicking it opens a resizable, non-modal ImGui window.

Both the inline and expanded versions share the same data, so changes propagate both ways.

## Layout

All expanded windows follow a consistent layout:

```
+---------------------------+---------+
|                           | Label   |
|                           | [ctrl]  |
|        Widget (3:2)       |         |
|                           | Label   |
|                           | [ctrl]  |
+---------------------------+---------+
|<-------- 4:2 window ratio -------->|
```

- **Window** aspect ratio: **4:2** (default 800x400)
- **Left panel** (75% width): widget at **3:2** aspect ratio
- **Right panel** (25% width): bordered child window, vertical label + control pairs

### Right panel convention

All labels and controls are stacked vertically -- label on top, full-width control below:

```cpp
ImGui::TextUnformatted( "Position" );
ImGui::SetNextItemWidth( -FLT_MIN );
ImGui::DragFloat( "##pos", &value, 0.001f, 0.0f, 1.0f, "%.3f" );
```

## Supported Widgets

| Widget | Right Panel Contents |
|---|---|
| GradientEditor | Stops (count), Interpolation (Combo), Selected: Position (DragFloat), Color (ColorEdit4) |
| CurveEditor | Keys (count), Range X/Y, Selected: X/Y (DragFloat), Segment (Combo) |
| ColorWheel | Mode, R/G/B/A (DragFloat, editable), HDR max, color preview swatch |
| HDRWheel ^A | Y Value (DragFloat), Right Arc (DragFloat, when `rightValue != NULL`), Left Arc (DragFloat, when `leftValue != NULL`), R/G/B (DragFloat) |
| ColorWarper | Mode, Space, Third Axis, Selected point: Offset X/Y (DragFloat), Pinned |
| ColorCurve | Mode, Keys (count), Selected: Position/Value (DragFloat) |
| ParadeScope | Overlay (Checkbox), Scale (Combo) |
| VectorScope | Skin Tone Line (Checkbox) |
| Histogram | Layout (Combo), X Scale (Combo), Y Scale (Combo) |
| CIEChromaticity | Gamut (Combo), Background (Checkbox), Signal Color (Combo) |
| ToneCurve | Mode, Channel (name + index), Keys (count), Selected: Input/Output (DragFloat) |

**^A `HDRWheel`** -- replaces the former separate `PrimariesWheel`. When called with `rightValue = leftValue = NULL`, the widget renders without arc sliders (tighter footprint matching the old PrimariesWheel layout) and the right panel hides the corresponding arc DragFloats. With one or both arc pointers non-null, the matching DragFloat appears.

## Architecture

### Infrastructure (in `dear_widgets.cpp`)

- `s_InsideExpandedWidget` -- static bool flag preventing recursive expand buttons
- `WidgetExpandButton(widget_id, bb)` -- draws the expand icon at the top-right of `bb`, returns `bool*` to the open state (stored in `window->StateStorage`), returns `NULL` when inside an expanded window
- `IsMouseOverExpandButton(widget_id, bb)` -- hit-test helper to suppress widget interaction when mouse is over the button
- `BeginExpandedWindow(label, widget_id, pOpen, defaultSize)` -- opens a resizable `ImGui::Begin` window with a stable ID (`###ExpandWdg_XXXXXXXX`), sets `s_InsideExpandedWidget = true`
- `EndExpandedWindow()` -- calls `ImGui::End()`, resets `s_InsideExpandedWidget = false`

### Per-widget pattern

```cpp
bool* pExpanded = WidgetExpandButton( id, bb );
if ( pExpanded && *pExpanded )
{
    if ( BeginExpandedWindow( label, id, pExpanded, ImVec2( 800, 400 ) ) )
    {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float widgetW = avail.x * 0.75f;
        // Left: widget
        Widget( "##exp", data, ..., ImVec2( widgetW, avail.y ) );
        ImGui::SameLine();
        // Right: info / options (vertical label + control)
        ImGui::BeginChild( "##info", ImVec2( 0, avail.y ), ImGuiChildFlags_Borders );
        ImGui::TextUnformatted( "Keys" );
        ImGui::Text( "%d", data->Keys.Size );
        ImGui::Spacing();
        ImGui::TextUnformatted( "Position" );
        ImGui::SetNextItemWidth( -FLT_MIN );
        ImGui::DragFloat( "##pos", &key.Position, 0.001f );
        ImGui::EndChild();
    }
    EndExpandedWindow();
}
```

### Display-only widgets (ParadeScope, VectorScope, Histogram, CIEChromaticity)

The right panel stores option overrides in `ImGui::GetStateStorage()`. Values are initialized from the caller's parameters on first open and persist across frames.

### Interactive widgets (GradientEditor, CurveEditor, ColorWheel, ColorWarper, ColorCurve, ToneCurve)

The right panel shows editable controls for the selected key/point (DragFloat for position, value, offsets; Combo for segment type). Changes made via the right panel propagate immediately through the shared data pointer.
