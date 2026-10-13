# Widget Interactions

**Scope: the six classic editing widgets only.** This table covers
`GradientEditor`, `CurveEditor`, `ColorCurve`, `ToneCurve`, `ColorWheel` and
`ColorWarper`. The other ~50 widgets are not described here; their interactions
are documented inline with each function in
[api/widgets.md](api/widgets.md) and [api/drawlist.md](api/drawlist.md).

Shared conventions that are **not** repeated per widget:

- Every widget with a numeric value opens a typed-input precision popup on
  double-click (`BeginPrecisionPopup` / `PrecisionFloat` / `PrecisionInt`).
- Canvas-style widgets pan with a left or middle drag and zoom to the cursor
  with the wheel.
- `KeyboardNudgeXY` / `KeyboardNudge1D` provide the arrow-key delta (Shift =
  10x) and `AxisConstrain` snaps a drag to the dominant axis while Shift is
  held; widgets that support nudging use these.

> **Known gap.** Arrow / Delete handling in the widgets below is gated on
> `hovered` without `SetKeyOwner`, so the same keys can also drive ImGui
> navigation in that frame. Hold that in mind when embedding them in a
> keyboard-navigated layout.

| Feature | GradientEditor | CurveEditor | ColorCurve | ToneCurve | ColorWheel | ColorWarper |
|---|---|---|---|---|---|---|
| **Arrow key nudging** | L/R | L/R/U/D | L/R/U/D | L/R/U/D | - | - |
| **SetKeyOwner (arrows)** | L/R | L/R/U/D | L/R/U/D | L/R/U/D | - | - |
| **Shift = 10x speed** | Yes | Yes | Yes | Yes | - | - |
| **Double-click reset** | - | Reset Y to midpoint | Reset value to default | Reset value to position (identity) | Reset sat/value | - |
| **Delete key** | Remove stop | Remove key | Remove key | Remove key | - | Reset point offset to (0,0) |
| **Right-click context menu** | - | - | - | Reset to Identity / Remove Key | Reset Color / Copy Hex | - |
| **Tooltip on hover** | Stop index, position, RGBA | Key index, (x, y) | Key index, position, value | Key index, input, output | H/S/V or H/C/L + RGB + A | Point [hue,sat], pinned, shift values |
| **Non-uniform Catmull-Rom** | - | - | Yes | Yes | - | - |
