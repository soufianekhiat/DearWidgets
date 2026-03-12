# Widget Interactions

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
