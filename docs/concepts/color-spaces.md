# Color Spaces

DearWidgets works in multiple color spaces throughout its widget and drawing API. This document explains which spaces are used and when.

---

## Storage Convention

All widget output values (`ImVec4* color` parameters) are in **sRGB with premultiplied-alpha disabled** — i.e. straight-alpha sRGB, matching ImGui's standard `ImVec4` convention. Components are floats `[0, 1]` (HDR widgets may exceed 1.0 for the RGB components when `hdr_max > 1`).

---

## Spaces in Use

### sRGB

The default for all color inputs/outputs. Non-linear (gamma ≈ 2.2 piecewise). Human-visible gamut encoded for display devices.

### Linear sRGB

Linear RGB after gamma removal (`ImsRGBToLinear`). Used internally for physically-correct blending and gradient interpolation (`ImWidgetsGradientInterp_LinearSRGB`).

### HSV / HSL / HSY / HSP

Polar decompositions of sRGB. Useful for perceptual hue/saturation controls, but not perceptually uniform. HSY uses BT.709 luma weights; HSP uses perceived-brightness weights.

### OkLab

Perceptually uniform, device-independent. The `L` axis is lightness; `a/b` encode color direction. Distances in OkLab correlate well with human perceived color differences. Used by `ColorPickerOkLab`, `ColorWarperSpace_OkLab`, gradient mode `ImWidgetsGradientInterp_OkLab`.

### OkLCH

Polar form of OkLab: `L` (lightness), `C` (chroma = distance from neutral), `H` (hue angle in degrees). The `H` axis is perceptually meaningful for hue-based selection. Used by `ColorPickerOkLCH`, `ColorWarperSpace_OkLCH`, `ImColorWheelMode_OkLCH`, histogram mode `ImHistogramMode_OkLCH`.

### CIE L\*a\*b\* (CIELab / D65)

The CIE standard perceptual color space. Similar goal to OkLab but older and less uniform at saturated colors. Used by `ColorPickerCIELab`.

### XYZ (D65)

The linear CIE tristimulus space. Used internally for color-space conversions and chromaticity plots. Accessible via `ColorPickerXYZ` and `ColorConvertXYZ*` functions.

### CIE xyY

Derived from XYZ: chromaticity coordinates `(x, y)` plus luminance `Y`. Used by the chromaticity diagram (`CIEChromaticity` widget, `DrawChromaticityPlot`).

---

## Gradient Interpolation Modes

| Mode | Space | Notes |
|---|---|---|
| `sRGB` | sRGB | Default; matches browser/CSS behavior |
| `LinearSRGB` | Linear sRGB | Gamma-correct; avoids dark banding |
| `OkLab` | OkLab | Perceptually uniform lightness |
| `OkLCH` | OkLCH | Hue interpolation follows perceptual arc |
| `HSV` | HSV | Hue cycling; vivid saturated transitions |

---

## Color Warper Spaces

The `ColorWarper` widget supports the following spaces for its hue/saturation grid:

| Space | Description |
|---|---|
| `HSV` | Hue-Saturation-Value |
| `HSL` | Hue-Saturation-Lightness |
| `HSY` | Hue-Saturation-Luma (BT.709) |
| `HSP` | Hue-Saturation-Perceived brightness |
| `HSPLog` | HSP with log-scale perceived brightness |
| `OkLab` | Perceptually uniform `a/b` plane |
| `OkLCH` | Perceptually uniform `C/H` plane |

---

## Reference Color Spaces (Chromaticity Plots)

Used by `DrawChromaticityPlot` and `DrawChromaticityPoints`:

| Space | White Point |
|---|---|
| sRGB / Rec.709 | D65 |
| Rec.2020 | D65 |
| DCI-P3 | D65 variant |
| ACEScg | D60 |
| Adobe RGB | D65 |
| ProPhoto | D50 |
| AdobeRGB | D65 |
| NTSC | C |
| PAL/SECAM | D65 |
| Wide Gamut RGB | D50 |

---

## Illuminants / White Points

Available for `DrawChromaticityPlot` (`ImWidgetsIlluminance`): A, B, C, D50, D55, D65, D75, D93, E, F1–F12.

## Observers

- `ImWidgetsObserverChromaticPlot_1931_2deg` — CIE 1931 2° standard observer
- `ImWidgetsObserverChromaticPlot_1964_10deg` — CIE 1964 10° supplementary observer
