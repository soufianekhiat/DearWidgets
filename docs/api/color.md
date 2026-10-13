# Color Utilities

All color functions are in the `ImWidgets` namespace. All *types* are at global
scope (2.0.0). Inputs and outputs are normalized floats `[0, 1]` unless noted.

---

## Argument order

Every `ColorConvert*` in DearWidgets takes **outputs first**, then inputs --
"right-to-left, like `=`":

```cpp
ColorConvertAtoB( out_x, out_y, out_z, in_x, in_y, in_z );
```

> **This is the opposite of `ImGui::ColorConvertRGBtoHSV`.** DearWidgets is
> internally consistent instead, which is what lets any conversion be used
> directly as a `pfColorModelToRGB` / `pfColorModelFromRGB` /
> `pfSpace2sRGB` / `pfsRGB2Space` callback.
>
> **Changed in 2.0.0:** the HSL / HSY / HSP / HSPLog pairs used to take inputs
> first and now follow the rule. See [CHANGELOG.md](../../CHANGELOG.md) breaking
> change #5.

---

## Color Space Conversions

### sRGB <-> Linear
```cpp
void ColorConvertRGBtoLinear( float& out_lr, float& out_lg, float& out_lb, float r, float g, float b );
void ColorConvertLineartoRGB( float& out_r, float& out_g, float& out_b, float lr, float lg, float lb );
```

### sRGB identity
```cpp
void ColorConvertsRGBtosRGB( float& out_r, float& out_g, float& out_b, float r, float g, float b );
```
Identity. It exists so the callback map has an entry for sRGB; you never need to
call it directly.

### sRGB <-> HSV
```cpp
void ColorConvertRGBtoHSV( float& out_h, float& out_s, float& out_v, float r, float g, float b );
void ColorConvertHSVtoRGB( float& out_r, float& out_g, float& out_b, float h, float s, float v );
```

### sRGB <-> HSL
```cpp
void ColorConvertRGBtoHSL( float& out_h, float& out_s, float& out_l, float r, float g, float b );
void ColorConvertHSLtoRGB( float& out_r, float& out_g, float& out_b, float h, float s, float l );
```

### sRGB <-> HSY (BT.709 luma)
```cpp
void ColorConvertRGBtoHSY( float& out_h, float& out_s, float& out_y, float r, float g, float b );
void ColorConvertHSYtoRGB( float& out_r, float& out_g, float& out_b, float h, float s, float y );
```

### sRGB <-> HSP (perceived brightness)
```cpp
void ColorConvertRGBtoHSP   ( float& out_h, float& out_s, float& out_p, float r, float g, float b );
void ColorConvertHSPtoRGB   ( float& out_r, float& out_g, float& out_b, float h, float s, float p );
void ColorConvertRGBtoHSPLog( float& out_h, float& out_s, float& out_p, float r, float g, float b );
void ColorConvertHSPLogtoRGB( float& out_r, float& out_g, float& out_b, float h, float s, float pLog );
```

### sRGB <-> OkLab
```cpp
void ColorConvertRGBtoOKLAB( float& out_L, float& out_a, float& out_b, float r, float g, float b );
void ColorConvertOKLABtoRGB( float& out_r, float& out_g, float& out_b, float L, float a, float b );
```

### sRGB <-> OkLCH
```cpp
void ColorConvertsRGBtoOKLCH ( float& out_L, float& out_c, float& out_h, float r, float g, float b );
void ColorConvertOKLCHtosRGB ( float& out_r, float& out_g, float& out_b, float L, float c, float h );
void ColorConvertOKLCHtoOKLAB( float& out_L, float& out_a, float& out_b, float L_in, float c, float h );
void ColorConvertOKLABtoOKLCH( float& out_L, float& out_c, float& out_h, float L, float a, float b );
```

### sRGB <-> XYZ (D65)
```cpp
void ColorConvertsRGBtoXYZ( float& out_X, float& out_Y, float& out_Z, float r, float g, float b );
void ColorConvertXYZtosRGB( float& out_r, float& out_g, float& out_b, float X, float Y, float Z );
```

### XYZ <-> CIE L\*a\*b\* (D65)
```cpp
void ColorConvertXYZtoCIELab ( float& out_L, float& out_a, float& out_b, float X, float Y, float Z );
void ColorConvertCIELabtoXYZ ( float& out_X, float& out_Y, float& out_Z, float L, float a, float b );
void ColorConvertsRGBtoCIELab( float& out_L, float& out_a, float& out_b, float r, float g, float b );
void ColorConvertCIELabtosRGB( float& out_r, float& out_g, float& out_b, float L, float a, float b );
```

### XYZ <-> xyY
```cpp
void ColorConvertXYZtoxyY( float& out_x, float& out_y, float& out_Y, float X, float Y, float Z );
void ColorConvertxyYtoXYZ( float& out_X, float& out_Y, float& out_Z, float x, float y, float Yval );
```

---

## Color Blending

```cpp
ImU32 ImColorBlendsRGB( ImU32 col0, ImU32 col1, float t );
ImU32 ColorBlend( ImU32 col0, ImU32 col1, float t, ImWidgetsGradientInterp interp );
```

`ImColorBlendsRGB` is a straight componentwise lerp in sRGB. `ColorBlend`
interpolates in the space named by `interp` and takes the shortest hue arc for
the cylindrical spaces.

> **Changed in 2.0.0.** `ImColorBlendLinear`, `ImColorBlendHSL`,
> `ImColorBlendHSLa`, `ImColorBlendHWB`, `ImColorBlendLCH`, `ImColorBlendLab`,
> `ImColorBlendOklab` and `ImColorBlendOkLCH` were declared but never defined
> (link error on any call). They are removed; use `ColorBlend` instead.

### Gradient sampling
```cpp
ImVec4 GradientSample     ( ImGradientData const& gradient, float t );
float  GradientAlphaSample( ImGradientData const& gradient, float t );
```
Sample at `t in [0,1]`. Interpolation mode comes from `gradient.Interpolation`.
`GradientAlphaSample` reads the independent alpha track when
`gradient.SplitAlpha` is set, and the colour stops' alpha otherwise.

### Gradient interpolation modes
```cpp
enum ImWidgetsGradientInterp_
{
    ImWidgetsGradientInterp_sRGB = 0,
    ImWidgetsGradientInterp_LinearSRGB,
    ImWidgetsGradientInterp_OkLab,
    ImWidgetsGradientInterp_OkLCH,
    ImWidgetsGradientInterp_HSV,
    ImWidgetsGradientInterp_COUNT
};
```

---

## Temperature and spectra

```cpp
ImU32 KelvinTemperatureTosRGBColors    ( float temperature );                        // [1000 K, 12000 K]
void  KelvinTemperatureToLinearRGBColorsF( float temperature, float* out_rgb_linear ); // 3 floats
void  WavelengthToRGB( float lambda, ImWidgetsObserver observer,
                       float& out_r, float& out_g, float& out_b );
void  SpectrumToXYZ  ( ImSpectrumData const& spectrum, ImWidgetsObserver observer,
                       float& out_X, float& out_Y, float& out_Z, bool normalize = true );
```

```cpp
enum ImWidgetsObserver_ { ImWidgetsObserver_CIE1931_2deg, ImWidgetsObserver_CIE1964_10deg, ImWidgetsObserver_COUNT };
```

---

## Working Color Spaces

```cpp
enum ImWidgetsColorSpace_
{
    ImWidgetsColorSpace_AdobeRGB,  ImWidgetsColorSpace_AppleRGB, ImWidgetsColorSpace_Best,
    ImWidgetsColorSpace_Beta,      ImWidgetsColorSpace_Bruce,    ImWidgetsColorSpace_CIERGB,
    ImWidgetsColorSpace_ColorMatch,ImWidgetsColorSpace_Don_RGB_4,ImWidgetsColorSpace_ECI,
    ImWidgetsColorSpace_Ekta_Space_PS5, ImWidgetsColorSpace_NTSC, ImWidgetsColorSpace_PAL_SECAM,
    ImWidgetsColorSpace_ProPhoto,  ImWidgetsColorSpace_SMPTE_C,  ImWidgetsColorSpace_sRGB,
    ImWidgetsColorSpace_WideGamutRGB, ImWidgetsColorSpace_Rec2020,
    ImWidgetsColorSpace_COUNT
};
```

### Introspection

| Signature | Description |
|---|---|
| `char const* ColorSpaceName( ImWidgetsColorSpace space )` | Display name ("ProPhoto", "Rec. 2020", ...). |
| `void ColorSpaceGetPrimaries( ImWidgetsColorSpace space, ImVec2& out_r, ImVec2& out_g, ImVec2& out_b )` | The three primaries as CIE 1931 `xy` chromaticities. |
| `void ColorSpaceGetTransfer( ImWidgetsColorSpace space, pfColorTransfer* out_encode, pfColorTransfer* out_decode, float* out_gamma )` | Transfer function pair and nominal gamma. Any out param may be `NULL`. |
| `float ColorSpaceEncode( ImWidgetsColorSpace space, float linear )` | Linear -> encoded, one channel. |
| `float ColorSpaceDecode( ImWidgetsColorSpace space, float encoded )` | Encoded -> linear, one channel. |
| `void ColorConvertRGBSpacetoXYZ( ImWidgetsColorSpace space, float& out_X, float& out_Y, float& out_Z, float r, float g, float b )` | Encoded space RGB -> XYZ (space's own white point). |
| `void ColorConvertXYZtoRGBSpace( ImWidgetsColorSpace space, float& out_r, float& out_g, float& out_b, float X, float Y, float Z )` | XYZ -> encoded space RGB. |
| `void ColorAdaptSpaceToD65( ImWidgetsColorSpace space, float& X, float& Y, float& Z )` | Bradford-adapt from the space's white point to D65, in place. |
| `void ColorAdaptD65ToSpace( ImWidgetsColorSpace space, float& X, float& Y, float& Z )` | The inverse. |
| `bool ChromaticityInsideGamut( ImWidgetsColorSpace space, float x, float y )` | Is the `xy` chromaticity inside the space's primary triangle? |
| `bool ColorInsideSpaceGamut( ImWidgetsColorSpace space, float r, float g, float b, bool adapt_white_point = true )` | Is the display colour representable in `space`? |

> D50-white spaces (ProPhoto, ECI, Best, ...) render their own white as a warm
> cast unless adapted. Every path that displays a working-space colour takes an
> `adapt_white_point` flag defaulting to `true`.

---

## Color Models

A *model* is a coordinate system (HSV, OkLab, xyY, ...); a *space* is a set of
primaries plus a transfer function. Most DearWidgets colour APIs take one of each.

```cpp
enum ImWidgetsColorModel_
{
    ImWidgetsColorModel_RGB, ImWidgetsColorModel_LinearRGB,
    ImWidgetsColorModel_HSV, ImWidgetsColorModel_HSL,
    ImWidgetsColorModel_HSY, ImWidgetsColorModel_HSP,
    ImWidgetsColorModel_OkLab, ImWidgetsColorModel_OkLCH,
    ImWidgetsColorModel_CIELab, ImWidgetsColorModel_CIELCh,
    ImWidgetsColorModel_XYZ, ImWidgetsColorModel_xyY,
    ImWidgetsColorModel_COUNT
};

enum ImWidgetsColorAxis_
{
    ImWidgetsColorAxis_None, ImWidgetsColorAxis_Hue, ImWidgetsColorAxis_Chroma,
    ImWidgetsColorAxis_Lightness,
    ImWidgetsColorAxis_Comp0, ImWidgetsColorAxis_Comp1, ImWidgetsColorAxis_Comp2,
    ImWidgetsColorAxis_COUNT
};
```

| Signature | Description |
|---|---|
| `bool ColorModelIsWorkingSpace( ImWidgetsColorModel model )` | True when the model's coordinates are space-relative (RGB / LinearRGB) rather than absolute. |
| `void ColorModelGetFunctions( ImWidgetsColorModel model, pfColorModelToRGB* out_to_rgb, pfColorModelFromRGB* out_from_rgb )` | Conversion pair for a model; either out param may be `NULL`. **Both** come back `NULL` for `LinearRGB`, which has no fixed pair. |
| `char const* ColorModelName( ImWidgetsColorModel model )` | Display name. |
| `char const* ColorModelCompName( ImWidgetsColorModel model, int comp )` | Name of component 0..2 ("H", "S", "V", "L", "a", "b", ...). |
| `int ColorModelAxisIndex( ImWidgetsColorModel model, ImWidgetsColorAxis axis )` | Which component index a semantic axis maps to, or -1. |
| `void ColorModelCompRange( ImWidgetsColorModel model, int comp, float* out_min, float* out_max )` | Natural range of one component. |
| `void ColorModelDefaultTriple( ImWidgetsColorModel model, float out_c[3] )` | A neutral starting triple for the model. |
| `void ColorModelToDisplay( ImWidgetsColorModel model, ImWidgetsColorSpace space, float c0, float c1, float c2, float& out_r, float& out_g, float& out_b, bool adapt_white_point = true )` | Model coordinates -> display sRGB. |
| `void ColorDisplayToModel( ImWidgetsColorModel model, ImWidgetsColorSpace space, float r, float g, float b, float& out_c0, float& out_c1, float& out_c2, bool adapt_white_point = true )` | The inverse. |
| `char const* ColorAxisName( ImWidgetsColorAxis axis )` | Display name. |

---

## Color Harmony

```cpp
enum ImWidgetsColorHarmony_
{
    ImWidgetsColorHarmony_None, ImWidgetsColorHarmony_Complement,
    ImWidgetsColorHarmony_SplitComplement, ImWidgetsColorHarmony_Triad,
    ImWidgetsColorHarmony_Tetrad, ImWidgetsColorHarmony_Square,
    ImWidgetsColorHarmony_Analogous, ImWidgetsColorHarmony_Monochromatic,
    ImWidgetsColorHarmony_COUNT
};

int  ColorHarmonyOffsets( ImWidgetsColorHarmony harmony, float spread_deg,
                          float* out_deg, int cap );
char const* ColorHarmonyName( ImWidgetsColorHarmony harmony );
```

`ColorHarmonyOffsets` writes up to `cap` hue offsets in degrees (the first is
always `0`, the base colour) and returns how many were written. `spread_deg` is
only used by `SplitComplement` and `Analogous`.

---

## Scalar -> Color

One uniform `float -> ImU32` mapping, exposed once per colour space. Every
entry point has the same signature as `ImWidgetsColor1DCallback`:

```cpp
typedef ImU32 ( *ImWidgetsColor1DCallback )( float t, void* pUserData );
```

so any of them drops straight into `DrawProceduralColor1D*`, `DrawColorRing`,
`DrawProceduralColorArcBilinear`, `DrawShapeProceduralColor*`,
`ShapeFillProceduralColor1D`, ... with no adapter.

`pUserData` is an optional `ImWidgetsColorScalar const*` saying *what the scalar
means* -- a hue, a lightness, a chroma, a colour temperature, a wavelength --
what the other two components are pinned to, how `t` is shaped, and which colour
harmony is applied on top. `NULL` gives the default: a plain hue sweep at full
saturation, i.e. the classic hue bar.

### Options

See [data-types.md -- ImWidgetsColorScalar](data-types.md#imwidgetscolorscalar)
for the full field list.

```cpp
void ColorScalarPreset( ImWidgetsColorScalar* opt, ImWidgetsColorRamp ramp );

enum ImWidgetsColorRamp_
{
    ImWidgetsColorRamp_Hue, ImWidgetsColorRamp_Lightness, ImWidgetsColorRamp_Chroma,
    ImWidgetsColorRamp_LightnessToHue, ImWidgetsColorRamp_Grey,
    ImWidgetsColorRamp_Red, ImWidgetsColorRamp_Green, ImWidgetsColorRamp_Blue,
    ImWidgetsColorRamp_Kelvin, ImWidgetsColorRamp_Wavelength,
    ImWidgetsColorRamp_COUNT
};
```

### Resolve once, sample many

Every enum in the options is resolved to a **function pointer** once, up front,
by `ColorScalarResolve` -- never re-dispatched per sample.

```cpp
void   ColorScalarResolve( ImWidgetsColorScalarResolved* out_resolved,
                           ImWidgetsColorScalar const& opt,
                           ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
ImVec4 ColorFromScalarResolved        ( float t, ImWidgetsColorScalarResolved const& resolved );
ImU32  ColorFromScalarResolvedCallback( float t, void* pUserData );  // pUserData: ImWidgetsColorScalarResolved const*
```

`ImWidgetsColorSpace_FromOptions` (`= -1`) means "keep whatever
`ImWidgetsColorScalar::Space` already holds".

### Convenience (resolves internally, per call)

```cpp
ImVec4 ColorFromScalar        ( float t, ImWidgetsColorScalar const& opt );
ImU32  ColorFromScalarU32     ( float t, ImWidgetsColorScalar const& opt );
ImU32  ColorFromScalarCallback( float t, void* pUserData );  // pUserData: ImWidgetsColorScalar const*
ImU32  ColorScalarInSpace( ImWidgetsColorSpace space, float t, ImWidgetsColorScalar const* opt );
```

### Per-space callback

```cpp
ImWidgetsColor1DCallback ColorScalarCallback( ImWidgetsColorSpace space );
```

The single entry point for "give me a `float -> ImU32` callback pinned to this
working space". The returned callback has the standard
`ImU32 ( float t, void* pUserData )` signature, so it drops into any of the
procedural-colour drawing functions; pass an `ImWidgetsColorScalar const*` as
`pUserData`, or `NULL` for the default hue sweep.

```cpp
ImWidgets::DrawColorRing( dl, pos, size, 12.0f,
                          ImWidgets::ColorScalarCallback( ImWidgetsColorSpace_Rec2020 ),
                          &opt, 128, 0.0f, true );
```

> **Changed in 2.0.0.** The 17 individual `ColorScalar<Space>` functions
> (`ColorScalarsRGB`, `ColorScalarAdobeRGB`, `ColorScalarProPhoto`,
> `ColorScalarRec2020`, ...) are no longer exported -- they differed only by the
> space they pinned. `ColorScalarCallback` is unchanged and replaces all of them.

### Space-aware drawing overloads

These take the options struct directly and pick the right entry point
themselves. Each has a plain callback sibling documented in
[drawlist.md](drawlist.md).

```cpp
void ShapeFillProceduralColor1D( ImWidgetsShape& shape, ImWidgetsColorScalar const& opt,
                                 bool sample_v,
                                 ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );

void DrawShapeProceduralColorVerticalBand  ( ImDrawList*, ImRect const& bb, int divisions,
                                             ImWidgetsColorScalar const& opt, ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
void DrawShapeProceduralColorHorizontalBand( ImDrawList*, ImRect const& bb, int divisions,
                                             ImWidgetsColorScalar const& opt, ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
void DrawShapeProceduralColorAnnulus       ( ImDrawList*, ImVec2 center, float innerRadius, float outerRadius,
                                             int numSectors, ImWidgetsColorScalar const& opt,
                                             ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );

void DrawProceduralColor1DNearestHorizontal ( ImDrawList*, ImWidgetsColorScalar const& opt, float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX, ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
void DrawProceduralColor1DNearestVertical   ( ImDrawList*, ImWidgetsColorScalar const& opt, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionY, ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
void DrawProceduralColor1DBilinearHorizontal( ImDrawList*, ImWidgetsColorScalar const& opt, float minX, float maxX, ImVec2 position, ImVec2 size, int resolutionX, ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
void DrawProceduralColor1DBilinearVertical  ( ImDrawList*, ImWidgetsColorScalar const& opt, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionY, ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );

void DrawProceduralColorArcBilinear   ( ImDrawList*, ImVec2 center, float innerRadius, float outerRadius,
                                        float startAngle, float sweepAngle, ImWidgetsColorScalar const& opt,
                                        int division, bool bilinear, ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
void DrawProceduralColorSplineBilinear( ImDrawList*, ImVec2 const* points, int points_count, float thickness,
                                        ImWidgetsColorScalar const& opt, int resolution, bool closed,
                                        ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
void DrawColorRing                    ( ImDrawList*, ImVec2 const curPos, ImVec2 const size, float thickness_,
                                        ImWidgetsColorScalar const& opt, int division, float colorOffset,
                                        bool bIsBilinear, ImWidgetsColorSpace space = ImWidgetsColorSpace_FromOptions );
```

### Shaping and output enums

```cpp
enum ImWidgetsColorSource_ { Model, Kelvin, Wavelength, Gradient, COUNT };  // ImWidgetsColorSource_*
enum ImWidgetsColorWrap_   { Clamp, Repeat, Mirror, COUNT };                // ImWidgetsColorWrap_*
enum ImWidgetsColorEase_   { Linear, Smoothstep, Gamma, Exponential, Logarithmic, Sine, COUNT };
enum ImWidgetsColorGamut_  { Clip, Desaturate, Mark, Keep, COUNT };         // out-of-gamut policy
```

Name helpers:

```cpp
char const* ColorSourceName( ImWidgetsColorSource source );
char const* ColorWrapName  ( ImWidgetsColorWrap   wrap   );
char const* ColorEaseName  ( ImWidgetsColorEase   ease   );
char const* ColorGamutName ( ImWidgetsColorGamut  gamut  );
char const* ColorRampName  ( ImWidgetsColorRamp   ramp   );
```

---

## Color Difference (Delta-E)

```cpp
float ColorDeltaE76  ( ImVec4 a, ImVec4 b );
float ColorDeltaE94  ( ImVec4 a, ImVec4 b, float kL = 1.0f, float kC = 1.0f, float kH = 1.0f );
float ColorDeltaE2000( ImVec4 a, ImVec4 b, float kL = 1.0f, float kC = 1.0f, float kH = 1.0f );
float ColorDeltaEOK  ( ImVec4 a, ImVec4 b );

char const* DeltaEFormulaName    ( ImDeltaEFormula f );
char const* DeltaEPerceptualLabel( float de );   // "Imperceptible" / "Just noticeable" / ...
```

```cpp
enum ImDeltaEFormula_
{
    ImDeltaEFormula_E76, ImDeltaEFormula_E94, ImDeltaEFormula_E2000, ImDeltaEFormula_EOK,
    ImDeltaEFormula_COUNT
};
```

> **Changed in 2.0.0.** `ImDeltaEFormulaShader_`, which used different ordinals
> for the same four formulas, is gone. `ImDeltaECompareState::Formula` is typed
> `ImDeltaEFormula`; the GPU path remaps internally.

The interactive tools that consume these -- `ColorDifferenceVisualizer` and
`ColorDifferenceImageViewer` -- are in [widgets.md](widgets.md).

---

## Grading gradients

```cpp
ImGradientData const* GradingGradient( ImWidgetsGradingGradient which );

enum ImWidgetsGradingGradient_
{
    ImWidgetsGradingGradient_Temperature, ImWidgetsGradingGradient_Tint,
    ImWidgetsGradingGradient_Contrast,    ImWidgetsGradingGradient_Pivot,
    ImWidgetsGradingGradient_MidDetail,   ImWidgetsGradingGradient_Saturation,
    ImWidgetsGradingGradient_Hue,         ImWidgetsGradingGradient_LumMix,
    ImWidgetsGradingGradient_COUNT
};
```

Returns a library-owned, never-freed `ImGradientData` for the named grading
control -- pass it straight to `SliderGradient*`, `DragFloatGradientUnderline` or
`DrawGradientBar`.

---

## Internal helpers

> `ImsRGBToLinear`, `ImLinearTosRGB`, `ImNormalize01`, `ImRescale`,
> `ImLinearSample`, `ImFunctionFromData` and `ImColorFrom_xyz` live in
> `src/api/dear_widgets_internal.h`. They are **not public API** and are not
> covered by the 2.0 stability promise -- they can change or disappear in any
> release. They used to be listed here by mistake.
