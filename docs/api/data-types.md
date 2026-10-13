# Data Types

Data structs used as inputs and outputs for widgets and drawing functions.

> **All public types are at global scope in 2.0.0.** Around 100 declarations
> that used to live inside `namespace ImWidgets` were moved out, so every
> DearWidgets type is spelled the same way (as `ImVec2` / `ImDrawList` are in
> ImGui). Only *functions* remain in `namespace ImWidgets`. Drop the
> `ImWidgets::` prefix from type names.

---

## Gradient

### `ImGradientStop` / `ImGradientAlphaStop`
```cpp
struct ImGradientStop {
    float  Position;   // [0, 1]
    ImVec4 Color;      // RGBA in sRGB, float [0,1]
};

struct ImGradientAlphaStop {
    float Position;    // [0, 1]
    float Alpha;       // [0, 1]
};
```

### `ImGradientData`
```cpp
struct ImGradientData {
    ImVector<ImGradientStop>      Stops;
    ImVector<ImGradientAlphaStop> AlphaStops;        // used when SplitAlpha == true
    ImWidgetsGradientInterp       Interpolation;     // sRGB, LinearSRGB, OkLab, OkLCH, HSV
    int                           SelectedIdx;       // editor state, -1 = none
    int                           SelectedAlphaIdx;  // editor state for the alpha track, -1 = none
    bool                          SplitAlpha;        // colour and alpha have independent keys
};
```
A default-constructed gradient has two colour stops (black -> white) and two
alpha stops (both 1.0).

Methods:

| Method | Description |
|---|---|
| `int AddStop( float pos, ImVec4 col )` | Add a colour stop; returns its sorted index. |
| `bool RemoveStop( int idx )` | Remove a colour stop (minimum 2 enforced). |
| `void SortStops()` | Sort colour stops by position (called by `AddStop`). |
| `int AddAlphaStop( float pos, float a )` | Add an alpha stop; returns its sorted index. |
| `bool RemoveAlphaStop( int idx )` | Remove an alpha stop (minimum 2 enforced). |
| `void SortAlphaStops()` | Sort alpha stops by position. |

Sampling:
```cpp
ImVec4 GradientSample     ( ImGradientData const& gradient, float t );
float  GradientAlphaSample( ImGradientData const& gradient, float t );
```
`GradientAlphaSample` reads `AlphaStops` when `SplitAlpha` is set, and the
colour stops' alpha channel otherwise.

---

## Animation Curve

### `ImCurveEditorKey`
```cpp
struct ImCurveEditorKey {
    ImVec2                   Pos;           // (x = time, y = value)
    ImCurveEditorSeg         Segment;       // interpolation from this key to the next
    ImVec2                   TangentLeft;   // incoming handle offset (typically negative x)
    ImVec2                   TangentRight;  // outgoing handle offset (typically positive x)
    ImCurveEditorTangentMode TangentMode;   // Free, Aligned, Mirrored
};
```

### `ImCurveEditorData`
```cpp
struct ImCurveEditorData {
    ImVector<ImCurveEditorKey> Keys;
    ImVec2                     RangeMin;    // visible range (x = time, y = value)
    ImVec2                     RangeMax;
    int                        SelectedIdx; // -1 = none
    int                        DragTarget;  // 0 = key, -1 = left tangent, 1 = right tangent
};
```
Methods: `int AddKey( ImVec2 pos, ImCurveEditorSeg seg )` (inserts a key,
splitting the existing Bezier at that time via De Casteljau),
`bool RemoveKey( int idx )` (minimum 2 keys), `void SortKeys()`.

**Segment types (`ImCurveEditorSeg`):** `StepStart`, `StepEnd`, `StepCenter`,
`Linear`, `InQuad` ... `InOutBounce`, `CubicBezier` (15+ easing types).

**Tangent modes (`ImCurveEditorTangentMode`):** `Free`, `Aligned`, `Mirrored`.

Helpers:
```cpp
float       CurveEditorEvalEasing( ImCurveEditorSeg seg, float t );   // t in [0,1]
float       CurveEditorSample    ( ImCurveEditorData const& curve, float x );
char const* CurveEditorSegName   ( ImCurveEditorSeg seg );
char const* CurveEditorTangentModeName( ImCurveEditorTangentMode mode );
```

> **Removed in 2.0.0:** the unfinished third curve model `ImCurveKey`,
> `ImCurveData`, `ImCurveInterp` and `ImCurveEval` / `ImCurveAdd` /
> `ImCurveRemove` / `ImCurveSort`. The shipping curve types are
> `ImCurveEditorData`, `ImColorCurveData` and `ImToneCurveData`.

---

## Color Curves

### `ImColorCurveKey` / `ImColorCurveData`
```cpp
struct ImColorCurveKey {
    float                    Position;      // X position [0,1]
    float                    Value;         // Y value (range depends on mode)
    ImCurveEditorSeg         Segment;
    ImVec2                   TangentLeft, TangentRight;
    ImCurveEditorTangentMode TangentMode;
};

struct ImColorCurveData {
    ImVector<ImColorCurveKey> Keys;
    int                       SelectedIdx;
};
```
Methods: `AddKey`, `RemoveKey`, `SortKeys`.

### Color curve modes (`ImColorCurveMode`)

| Mode | X axis | Y axis | Y range |
|---|---|---|---|
| `ImColorCurveMode_HueVsHue` | Hue [0,1] | Hue shift | [-0.5, 0.5] |
| `ImColorCurveMode_HueVsSat` | Hue [0,1] | Saturation multiplier | [0, 2] |
| `ImColorCurveMode_HueVsLum` | Hue [0,1] | Luminance offset | [-1, 1] |
| `ImColorCurveMode_LumVsSat` | Luminance [0,1] | Saturation multiplier | [0, 2] |
| `ImColorCurveMode_SatVsSat` | Saturation [0,1] | Saturation multiplier | [0, 2] |

```cpp
float       ColorCurveDefaultValue( ImColorCurveMode mode );
void        ColorCurveRange       ( ImColorCurveMode mode, float* out_min, float* out_max );
char const* ColorCurveModeName    ( ImColorCurveMode mode );
float       ColorCurveSample      ( ImColorCurveData const& curve, ImColorCurveMode mode,
                                    float x, bool advancedSegments = false );
```

---

## Tone Curve

### `ImToneCurveData`
```cpp
struct ImToneCurveData {
    ImColorCurveData Channels[ 4 ];   // per-channel curves (up to 4)
    int              ActiveChannel;
};
```
Method: `Reset( int channelCount )`.

```cpp
int         ToneCurveChannelCount( ImHistogramMode mode );
char const* ToneCurveChannelName ( ImHistogramMode mode, int channel );
float       ToneCurveSample      ( ImColorCurveData const& curve, float x,
                                   bool advancedSegments = false );
```

---

## Pixel Buffers

Every `Accumulate(...)` below reads a raw CPU image described by three values:

```cpp
enum ImPixelBitDepth_ { ImPixelBitDepth_UInt8, ImPixelBitDepth_UInt10, ImPixelBitDepth_UInt16, ImPixelBitDepth_COUNT };
enum ImPixelLayout_   { ImPixelLayout_Interleaved, ImPixelLayout_Planar, ImPixelLayout_COUNT };
```

| Bit depth | Range |
|---|---|
| `ImPixelBitDepth_UInt8` | [0, 255] |
| `ImPixelBitDepth_UInt10` | [0, 1023] |
| `ImPixelBitDepth_UInt16` | [0, 65535] |

`ImPixelLayout_Interleaved` is `RGBRGB...`; `_Planar` is `RRR...GGG...BBB...`.

> **Renamed in 2.0.0:** `ImParadeBitDepth` -> `ImPixelBitDepth`,
> `ImParadeLayout` -> `ImPixelLayout`. They describe any pixel buffer, not just
> the parade scope. `ImParadeMode` and `ImParadeScale` keep their names.

---

## Scopes

### `ImParadeScopeData`
```cpp
struct ImParadeScopeData {
    ImVector<ImU32>  Bins;          // [ch * XBins * YBins + x * YBins + y]
    int              XBins, YBins, ChannelCount;
    ImU32            PeakCount;
    ImParadeMode     Mode;
    ImPixelBitDepth  BitDepth;

    void Accumulate( void const* data, int width, int height, int channels,
                     ImPixelBitDepth bitDepth, ImPixelLayout layout, ImParadeMode mode,
                     int xBins = 128, int yBins = 128, int maxSamples = 1000000 );
};
```
**Parade modes:** `ImParadeMode_Luma`, `_RGB`, `_YRGB`, `_YCbCr`.
**Scales:** `ImParadeScale_Linear`, `_Log`, `_InvLog`.

### `ImVectorScopeData`
```cpp
struct ImVectorScopeData {
    ImVector<ImU32>  Bins;          // [xBin * Resolution + yBin]
    int              Resolution;
    ImU32            PeakCount;
    ImPixelBitDepth  BitDepth;

    void Accumulate( void const* data, int width, int height, int channels,
                     ImPixelBitDepth bitDepth, ImPixelLayout layout,
                     int resolution = 256, int maxSamples = 1000000 );
};
```
Accumulates Cb/Cr chrominance (BT.709) into a 2D histogram.

### `ImHistogramData`
```cpp
struct ImHistogramData {
    ImVector<ImU32>  Bins;          // [ch * BinCount + bin]
    int              BinCount, ChannelCount;
    ImU32            PeakCount;
    ImHistogramMode  Mode;
    ImPixelBitDepth  BitDepth;

    void Accumulate( void const* data, int width, int height, int channels,
                     ImPixelBitDepth bitDepth, ImPixelLayout layout, ImHistogramMode mode,
                     int binCount = 256, int maxSamples = 1000000 );
};
```
**Histogram modes:** `ImHistogramMode_Luma`, `_RGB`, `_YRGB`, `_YCbCr`, `_HSV`,
`_OkLCH`.
**Layouts:** `ImHistogramLayout_Overlapped`, `_Stacked`.

### `ImCIEChromaticityData`
```cpp
struct ImCIEChromaticityData {
    ImVector<float> SampledRGB;     // [i*3+0..2] = r, g, b in [0,1]
    int             SampleCount;

    void Accumulate( void const* data, int width, int height, int channels,
                     ImPixelBitDepth bitDepth, ImPixelLayout layout,
                     int maxSamples = 50000 );
};
```
**CIE gamuts:** `ImCIEChromaticityGamut_sRGB_Rec709`, `_Rec2020`, `_DCI_P3`,
`_ACEScg`, `_AdobeRGB`, `_ProPhoto`.

---

## Color Warper

### `ImColorWarperData`
```cpp
struct ImColorWarperData {
    int              HueDivisions;   // divisions around hue (6, 12 or 24)
    int              SatDivisions;   // divisions along saturation
    ImVector<ImVec2> Offsets;        // per-point (hue_shift, sat_shift)
    ImVector<bool>   Pinned;         // per-point pin state
    int              SelectedIdx;    // -1 = none
};
```
Index layout: `satIdx * HueDivisions + hueIdx`. `satIdx = 0` is the centre,
`satIdx = SatDivisions` the outer edge.

Methods: `Init( int hueDivs, int satDivs )`, `Reset()`, `PointCount()`,
`PointIndex( hueIdx, satIdx )`, `GetIdentityPos(...)`, `GetWarpedPos(...)`.

### `ImColorWarperOverlay`
```cpp
struct ImColorWarperOverlay {
    ImVector<float> SampledRGB;     // [i*3+0..2] = r, g, b in [0,1]
    int             SampleCount;

    void Accumulate( void const* data, int width, int height, int channels,
                     ImPixelBitDepth bitDepth, ImPixelLayout layout,
                     int maxSamples = 50000 );
};
```

---

## Grading

### `ImColorSliceData`
```cpp
struct ImColorSliceData {
    ImVector<ImColorSliceVector> Slices;
    float       Density;          // luminance of saturated colours (subtractive)
    float       DensityDepth;     // how far Density reaches into highlights
    float       Saturation;
    float       SatBalance;       // luminance balance at medium saturation
    float       SatDepth;         // how far Saturation reaches into highlights
    float       Hue;
    int         HighlightSlice;   // slice whose highlight button is held, -1 = none
    char const* DensityLabel;     // default "D"
    char const* SaturationLabel;  // default "S"
};
```
Methods: `ResolveVectors()`, `ResetSlices()` (per-slice controls to 0, wedges
untouched), `ResetGlobals()`, `ResetAll()`.

### `ImChromaWarpData`
```cpp
struct ImChromaWarpData {
    ImVector<ImChromaWarpStroke> Strokes;
    int                          Selected;        // -1 = none
    ImChromaWarpTool             Tool;
    float                        TonalRangeLow;
    float                        TonalRangeHigh;
    float                        TonalRangePivot;
    ImVec2                       WhitePoint;      // diagram space; neutrals live here
};
```
Methods: `ResetGlobals()`, `ResetAll()`.

### `ImSpectrumData`
```cpp
struct ImSpectrumData {
    ImVector<float> Samples;    // relative power per sample; 0 = no emission
    float           LambdaMin;  // nm at Samples[0]        (default 380)
    float           LambdaMax;  // nm at Samples[Size-1]   (default 730)
};
```
Method: `Resize( int n )` (minimum 2; resampling preserves the existing curve).
Default is 64 samples.

### `ImGridWarpData`
```cpp
struct ImGridWarpData {
    int              Cols, Rows;
    ImVector<ImVec2> Offsets;     // Cols*Rows, row-major; per-node offset
    int              SelectedIdx; // -1 = none
};
```
Methods: `Init( int cols, int rows )` (minimum 2x2, default 4x4), `Reset()`,
`At( c, r )`, `Base( c, r )` (the un-warped normalized position).

---

## Scalar -> Color

### `ImWidgetsColorScalar`

The options struct describing a `float -> colour` mapping.

| Group | Fields |
|---|---|
| What the scalar means | `ImWidgetsColorSource Source`, `ImWidgetsColorModel Model`, `ImWidgetsColorSpace Space`, `ImWidgetsColorAxis Axis`, `float Fixed[3]` (the other components, index-aligned to `Model`), `float RangeMin`, `float RangeMax` (swap to reverse) |
| Second driven axis | `ImWidgetsColorAxis Axis2` (`_None` disables), `float Range2Min`, `float Range2Max` -- this is what makes "luminance to hue" one mapping rather than two |
| Shaping of `t` | `float DomainMin`, `float DomainMax`, `float Repeats`, `ImWidgetsColorWrap Wrap`, `ImWidgetsColorEase Ease`, `float EaseParam` |
| Harmony | `ImWidgetsColorHarmony Harmony`, `int HarmonyIndex` (0 = the base), `float HarmonySpread` (degrees), `ImWidgetsColorModel HarmonyModel` (the cylinder the rotation happens in) |
| Source-specific | `float KelvinMin/KelvinMax`, `float LambdaMin/LambdaMax`, `ImWidgetsObserver Observer`, `ImGradientData const* Gradient` |
| Output | `ImWidgetsColorGamut Gamut`, `ImU32 OutOfGamutColor`, `float Alpha`, `bool AdaptWhitePoint` |

Defaults: `Source_Model`, `Model_HSV`, `Space_sRGB`, `Axis_Hue`, range `[0,1]`,
no second axis, no harmony, `Wrap_Clamp`, `Ease_Linear` -- i.e. the classic hue
bar.

> `AdaptWhitePoint` Bradford-adapts the working-space white point to D65 before
> display. Without it a D50 space (ProPhoto, ECI, Best, ...) renders its own
> white as a warm cast, which is a conversion error rather than a preview.

### `ImWidgetsColorScalarResolved`

The pre-resolved form produced by `ColorScalarResolve`: the original `Options`
plus a function pointer and index for every enum in it (`ModelToRGB`,
`ModelFromRGB`, `Encode`, `Decode`, `Gamma`, `AxisIndex`, `Axis2Index`,
`AxisApply`, `Wrap`, `Ease`, `Source`, `Harmony`, `Gamut`, the harmony
conversion pair, and up to 5 `HarmonyOffsets`). Build it once, sample it many
times with `ColorFromScalarResolved`.

---

## Transform

### `ImTransformData`
```cpp
struct ImTransformData {
    ImVec2 Translation;  // offset from canvas centre (lp)
    float  Rotation;     // radians
    ImVec2 Scale;        // 1,1 = fit to canvas
};
```

### `ImTransformGizmoDrawParams` / `ImTransformGizmoCallbacks`
```cpp
struct ImTransformGizmoDrawParams {
    ImDrawList* DrawList;
    ImVec2      Center;        // screen-space centre
    ImVec2      Corners[ 4 ];  // screen-space corners: TL, TR, BR, BL
    float       HalfW, HalfH;  // half-extents after scale
    float       CosR, SinR;    // rotation basis
    int         Index;         // which item is being drawn
};

struct ImTransformGizmoCallbacks {
    ImTransformGizmoDrawFn DrawFn   = nullptr;   // draw item `Index`
    void*                  DrawData = nullptr;
    ImTransformGizmoSwapFn SwapFn   = nullptr;   // reorder two items
    void*                  SwapData = nullptr;
};
```

> **Replaces `ImTransformImage`** (2.0.0). `TransformGizmo` no longer owns
> textures: the host draws each item through `DrawFn` and can back it with a
> texture, a shape or anything else.

---

## Paint Canvas

### `ImPaintCanvasData`
```cpp
struct ImPaintCanvasData {
    void*                  Pixels;       // user-owned pixel buffer
    int                    Width, Height;
    ImPlatform_PixelFormat Format;       // e.g. ImPlatform_PixelFormat_RGBA8
    ImPaintMode            Mode;         // Mask, Grayscale, Color
    ImPaintBrush           Brush;        // Hard, Soft
    ImPaintTool            Tool;         // Brush, Eraser
    float                  BrushSize;
    float                  BrushHardness;
    float                  BrushOpacity;
    ImVec4                 BrushColor;
};
```
`DestroyTexture()` releases the internal GPU texture. Call it before freeing
`Pixels` or resizing.

---

## Image Viewing

### `ImImageViewerState`
```cpp
struct ImImageViewerState {
    float                  Zoom;         // 1.0 = fit image to widget
    ImVec2                 Pan;          // image-space pan from the image centre
    void const*            Pixels;       // optional CPU buffer for the pixel inspector
    ImVec2                 PixelSize;    // dimensions of Pixels
    ImPlatform_PixelFormat PixelFormat;
    // _Last* members are internal, filled by ImageViewer each frame.
};
```
Read-only helpers, valid after `ImageViewer` has run at least once:

| Method | Description |
|---|---|
| `ImVec2 UVToCanvas( ImVec2 uv ) const` | Image UV -> screen position. |
| `ImVec2 CanvasToUV( ImVec2 screen ) const` | Screen position -> image UV. |
| `ImRect GetCanvasRect() const` | The widget's screen rect. |
| `float GetZoomScale() const` | Total scale (fit scale x `Zoom`). |

The overlays use these, which is why they can be called with nothing but the
state.

### Overlay inputs
```cpp
struct ImDetectionBox {
    float x, y;      // top-left corner, UV
    float w, h;      // size, UV
    float score;     // confidence [0,1]; < 0 = no score shown
    int   class_id;  // index into a class-name table; -1 = unlabeled
    int   user_id;   // app-defined ID for selection tracking; -1 = none
};

struct ImKeypoint {
    float x, y;        // UV position
    float confidence;  // [0,1]; 0 = skip, < 0.3 = drawn faded
    int   part_id;     // index into a part-name table
};

struct ImSkeletonEdge { int part_a; int part_b; };

struct ImTextLabel {
    float       x, y;        // UV anchor
    char const* text;
    ImU32       color;       // 0 = white
    float       font_scale;  // 1.0 = default
    int         anchor;      // ImTextLabelAnchor_*
};

struct ImOverlayStyle {
    float box_thickness;     // detection-box outline width      (2.0)
    float label_font_scale;  // 1.0 = default UI font size
    float label_bg_alpha;    // label chip background            (0.6)
    bool  show_score;        //                                  (true)
    bool  show_class_label;  //                                  (true)
    ImU32 default_color;     // 0 = auto palette by class_id
    float point_radius;      // keypoint dot radius              (5.0)
    float edge_thickness;    // skeleton edge width              (2.0)
};
```

### `ImAnnotationResult`
```cpp
struct ImAnnotationResult {
    int            action;           // ImAnnotationAction_*
    int            user_id;          // which box was acted on (-1 for Create)
    ImDetectionBox new_value;        // resulting box for Move / Resize / Create
    char           new_label[ 128 ]; // filled on LabelEdit
};
```
`AnnotationEditor` returns an *intent*; it never mutates the boxes you pass in.

### `ImImageInspectorState` and `ImImageBuffer`

`ImageInspector` takes an **`ImImageBuffer`** -- a Halide-style strided buffer
descriptor defined in `ImPlatform.h`, not in DearWidgets. The widget uploads its
bytes once into a packed RGBA32F GPU texture and re-uploads only when
`ImImageBuffer.version` changes.

`ImImageInspectorState` holds the view transform (gamma / sRGB / log curve,
gamut, exposure, white point, tonemap, false colour, NaN highlight, mosaic
decode) plus pan / zoom and the cached GPU texture. All transforms run in the
shader; per-frame CPU cost is uniform updates only.

The CPU-side loupe walks your host pointer directly at full precision
(`double[4]` plus `int64[4]` lanes for exact integer display), so **the buffer
must stay alive while the widget is on screen**. Call
`ImageInspectorReleaseState( state )` to free the GPU resources.

Mosaic patterns (`ImMosaicPattern`): `None`, `BayerRGGB`, `BayerGRBG`,
`BayerGBRG`, `BayerBGGR`, `XTrans6x6`.

---

## LUTs, LookDev and Volumes

### `ImColorLUT3D`
```cpp
struct ImColorLUT3D {
    int              Size;           // edge length (17, 33, 65 typical)
    ImVector<ImVec4> Entries;        // Size^3 RGBA
    float            DomainMin[ 3 ]; // default 0,0,0
    float            DomainMax[ 3 ]; // default 1,1,1
};
```

### `ImDeltaECompareState`
```cpp
struct ImDeltaECompareState {
    ImDeltaEFormula   Formula;       // E76 / E94 / E2000 / EOK
    ImDeltaEColorRamp Ramp;          // e.g. ImDeltaEColorRamp_Viridis
    float             MaxDeltaE;     // dE == MaxDeltaE maps to the top of the ramp (20)
    float             Threshold;     // overlay red above this dE (5)
    bool              ShowThreshold;
    float             MixA, MixB, MixDeltaE;   // layer weights
};
```
> **Changed in 2.0.0.** `Formula` was typed by the now-removed
> `ImDeltaEFormulaShader_`, which numbered the formulas differently from the
> public enum. It is `ImDeltaEFormula` now; the GPU path remaps internally.

### `ImVolumeViewerState` / `ImVolumeSliceState`
```cpp
struct ImVolumeViewerState {
    float const*       Voxels;
    int                Width, Height, Depth;
    ImVolumeViewerMode Mode;
    float              SliceX, SliceY, SliceZ;   // 0..1
    float              WindowMin, WindowMax, Gamma;
    float              CamAzimuth, CamElevation; // radians
    float              CamDistance;              // world units
    float              StepSize;
    float              IsoThreshold;
    ImVolumeSliceState CacheX, CacheY, CacheZ;   // internal slice textures
};
```
`ImVolumeSliceState` is the lighter descriptor used by `VolumeSliceViewer`
(voxels, dimensions, window, gamma, current slice). `Voxels` is caller-owned and
must outlive the widget.

### `ImLookDevState` / `ImLookDevInspectorState`
Pan / zoom / wipe position plus the comparison mode for `LookDevCompare`, and
additionally exposure, false-colour and channel-isolation settings plus the
orbit state for `LookDevInspector`.

---

## Unit Field

### `ImUnitDef`
```cpp
struct ImUnitDef {
    char const*           name;          // e.g. "meter"
    char const*           abbreviation;  // e.g. "m"
    float                 mul;           // base * mul + add = display
    float                 add;
    ImUnitConvertCallback toDisplay;     // custom callback (overrides mul/add)
    ImUnitConvertCallback toBase;
    void*                 pUserData;
};
```
Factory helpers:
```cpp
ImUnitDef ImUnitDef_Simple( char const* name, char const* abbr, float mul, float add = 0.0f );
ImUnitDef ImUnitDef_Custom( char const* name, char const* abbr,
                            ImUnitConvertCallback toDisplay,
                            ImUnitConvertCallback toBase, void* pUserData = NULL );
```

---

## Canvas Toolkit

```cpp
struct ImWidgetsCanvasState {
    ImVec2 Pan;            // screen-space offset for the world origin
    float  Zoom;           // screen pixels per world unit
    bool   PanActive;
    ImVec2 PanLastMouse;
    ImRect Bounds;         // last-known screen viewport (filled by BeginCanvas)
};

struct ImWidgetsSelectionState {
    ImWidgetsSelectionMode Mode;
    ImVector<ImVec2>       Path;     // Marquee: [0] = anchor, [1] = cursor. Lasso: drag polyline.
    bool                   Active;   // currently dragging
    bool                   Closed;   // released this frame; selection is final
};

struct ImWidgetsCropState {
    ImRect Rect;
    int    Active;          // -1 idle; 0..7 grip (TL,T,TR,R,BR,B,BL,L); 8 inside
    ImVec2 DragOriginMs;
    ImRect DragOriginRect;
    ImVec2 LastBoundsMin;   // internal: tracks bounds movement between frames
    float  LastAspect;      // internal: last aspect ratio the rect was snapped to
};

struct ImWidgetsEyedropperResult {
    bool   Hovered;    // mouse is inside screen_rect this frame
    bool   Picked;     // LMB released inside the area
    ImVec2 PosScreen;  // cursor position when sampled
    ImVec2 UV;         // 0..1 within the source surface
    ImU32  Color;      // centre sample
};

struct ImWidgetsGradientDropState {
    ImVector<ImVec2> Path;    // sampled drag path, screen coords
    ImVector<ImU32>  Colors;  // sampled colour per path point (parallel arrays)
    bool             Active;  // true while the user is dragging
};

struct ImWidgetsOnionFrame {
    ImTextureID Tex;
    ImVec2      UV0, UV1;
    ImU32       Tint;   // RGBA; A is multiplied with Alpha
    float       Alpha;  // 0..1
};

struct ImWidgetsGradientMesh {
    int   CountX, CountY;
    ImVector<ImWidgetsGradientMeshNode> Nodes;  // CountX*CountY, row-major
    int   SelectedNode;                         // -1 idle
    bool  ShowMesh;
};

struct ImWidgetsToonRamp {
    ImVector<ImWidgetsToonBand> Bands;
    int                         SelectedBand;
};
```

These are POD state structs with `ImVector` members and no constructor -- zero
initialize them (`ImWidgetsCanvasState s = {};`) and set `Zoom = 1.0f` before
first use.

---

## Drawing Descriptors

```cpp
struct ImCoonsPatch {
    ImVec2 bottom[ 4 ];   // four cubic Bezier edges, 4 control points each
    ImVec2 top[ 4 ];
    ImVec2 left[ 4 ];
    ImVec2 right[ 4 ];
};

struct ImGregoryPatch {
    ImVec2 cp[ 20 ];      // 12 boundary + 8 interior control points
};

struct ImIsoContourTier {
    float spacing;      // world-value interval (or log-step multiplier)
    ImU32 color;
    float thickness;    // pixels
    bool  log_spacing;  // iso-values at k*spacing in the log domain
};

struct ImWidgetsVarFontAxis {
    char const* Tag;      // e.g. "wght"
    char const* Name;     // human label
    float       Min, Max, Default;
    float       Value;    // in/out
};
```

`ImWidgetsThickLineDesc` carries everything `DrawThickLine` needs: mode, colour,
thickness, cap, join, miter limit, `dashed` plus dash length / gap length / dash
offset, and tolerance. See
[compatibility.md](../compatibility.md#5-drawthickline--runtime-fallback-details).

---

## Vector Drawing Tool

### `ImVectorDrawingNode`
```cpp
struct ImVectorDrawingNode {
    ImVec2 Anchor;      // world-space anchor position
    ImVec2 InTangent;   // offset toward the incoming cubic control point
    ImVec2 OutTangent;  // offset toward the outgoing cubic control point
    bool   Broken;      // when false, InTangent mirrors OutTangent through the anchor
};
```
Consecutive nodes form a cubic Bezier segment using the left node's
`OutTangent` as P1 and the right node's `InTangent` as P2 (relative offsets
added to the respective anchors).

### `ImVectorDrawingStyle_`

| Value | Rendering | Notes |
|---|---|---|
| `ImVectorDrawingStyle_Polyline` | `ImDrawList::AddPolyline` | Cap/Join ignored |
| `ImVectorDrawingStyle_PolylineAA` | `DrawPolylineAA` (SDF-AA) | Cap/Join applied |
| `ImVectorDrawingStyle_StrokedBezier` | `DrawStrokedBezierPath` (Euler spiral) | Cap/Join applied |
| `ImVectorDrawingStyle_StrokedDashedBezier` | `DrawStrokedDashedBezierPath` | Cap/Join applied |
| `ImVectorDrawingStyle_DashedPolyline` | `DrawDashedPolylineAA` | Cap/Join applied |

### `ImVectorDrawingPath`
```cpp
struct ImVectorDrawingPath {
    ImVector<ImVectorDrawingNode> Nodes;
    bool                          Closed;     // join last node back to first
    ImU32                         Color;      // IM_COL32 packed RGBA
    float                         Thickness;  // stroke width (lp)
    ImVectorDrawingStyle          Style;
    float                         DashLen;    // dash length (lp), dashed styles only
    float                         GapLen;     // gap length (lp); 0 = flush, < 0 = overlap
    int                           Cap;        // ImWidgetsCap_*
    int                           Join;       // ImWidgetsJoin_*
};
```
Default: `Color` = amber, `Thickness` = 2, `Style` = `StrokedBezier`,
`Cap` = `Butt`, `Join` = `Miter`.

### `ImVectorDrawingData`
```cpp
struct ImVectorDrawingData {
    ImVector<ImVectorDrawingPath> Paths;
    ImVec2        PanOffset;      // world offset of the view origin (screen pixels)
    float         Zoom;           // world-to-screen multiplier (1.0 = 1:1)
    int           SelectedPath;   // index into Paths (-1 = none)
    int           SelectedNode;   // index into SelectedPath.Nodes (-1 = none)
    int           SelectedHandle; // 0 = anchor, 1 = InTangent, 2 = OutTangent
    ImVector<int> SelectedNodes;  // multi-node indices within SelectedPath
    int           ActivePath;     // path currently being drawn (-1 = idle)
};
```
All indices default to `-1`; `Zoom` defaults to `1.0`. Paths are owned by this
struct.

> **Fixed in 2.0.0.** `ImVectorDrawingData` now has `Clear()`, a destructor and
> deep copy / assignment. `ImVector` never destructs its elements, so before this
> every path leaked its node buffer at teardown.

```cpp
bool VectorDrawingTool( char const* label, ImVectorDrawingData& data,
                        ImVec2 size = ImVec2( 0, 0 ) );
```
Returns `true` when `data` was modified this frame.

---

## Network Graph

```cpp
struct ImNetworkGraphNode {
    char  Label[ 64 ];     // primary text (op type: "Conv")
    char  Sublabel[ 64 ];  // secondary text (instance name: "conv1_1")
    ImU32 Color;           // box fill tint
    int   GroupId;         // containing group, -1 = root
};

struct ImNetworkGraphGroup {
    char Label[ 64 ];
    int  Parent;    // parent group index, -1 = root
    int  OpCount;   // transitive op-node count (the "N ops" badge)
};

struct ImNetworkGraphEdge { int From, To; };   // indices into Nodes

struct ImNetworkGraphData {
    ImVector<ImNetworkGraphNode>  Nodes;
    ImVector<ImNetworkGraphGroup> Groups;
    ImVector<ImNetworkGraphEdge>  Edges;

    int    CurrentGroup;    // level being displayed, -1 = root
    ImVec2 Scroll;          // layout-space point at the view's top-left
    float  Zoom;            // layout -> screen multiplier
    int    HoveredNode, HoveredGroup;      // per-frame, -1 = none
    int    ActivatedNode, ActivatedGroup;  // double-clicked this frame, -1 = none
    bool   FitRequest;      // zoom-to-fit the current level on the next draw
    int    LastDrawnGroup;  // internal: level-change detection

    ImVector<ImNetworkGraphLevelLayout> LayoutCache;
};
```

| Method | Description |
|---|---|
| `int AddNode( char const* label, char const* sublabel, ImU32 color, int group_id )` | Truncating copy; returns the new index. |
| `int AddGroup( char const* label, int parent_group )` | Groups form a tree (`Parent = -1` for top level). |
| `void AddEdge( int from, int to )` | Directed edge between two op nodes. |
| `void Clear()` | Deep-clears everything, including the per-level layout buffers. |
| `void InvalidateLayout()` | Mark every cached level stale. |
| `ImNetworkGraphLevelLayout* FindLayout( int group_id )` | Cached layout, `nullptr` when not yet computed. |
| `void RecomputeOpCounts()` | Recompute every group's transitive `OpCount`. Call once after building -- `AddGroup` cannot know its future contents, so without this the badges read 0. |

The three builders invalidate the layout cache for you. **Any other mutation of
`Nodes` / `Groups` / `Edges` must be followed by `InvalidateLayout()`**: a cached
layout stores raw indices and the draw path dereferences them unguarded, so a
shrunk vector turns a stale cache into an out-of-bounds read, not merely a stale
picture.

Layout internals (`ImNetworkGraphLayoutItem`, `ImNetworkGraphLayoutEdge`,
`ImNetworkGraphLevelLayout`) are public so hosts can read back box positions;
they are filled by `NetworkGraphLayout` and should be treated as read-only.

---

## Timeline

Ticks are exact 64-bit integers; the host owns the rational frame rate.

```cpp
struct ImTimelineKey {
    long long Tick;
    int       TrackIndex;   // row in ImTimelineData::Tracks
    unsigned  Id;           // host identity (hit-testing / edits)
    bool      Selected;
};

struct ImTimelineEvent {
    long long Tick;
    unsigned  Id;
    ImU32     Color;        // 0 = default
};

enum ImTimelineBandKind {
    ImTimelineBand_CacheCoverage = 0,  // subtle fill
    ImTimelineBand_Invalid,            // hatched warning
    ImTimelineBand_StaleBranch,        // dimmed
    ImTimelineBand_Checkpoint,         // thin pin line at BeginTick; EndTick ignored
    ImTimelineBand_CommittedHistory,   // retained versions -- NOT cache residency
};

struct ImTimelineBand {
    long long          BeginTick;   // half-open [BeginTick, EndTick)
    long long          EndTick;
    ImTimelineBandKind Kind;
};

struct ImTimelineTrack { char const* Label; };

struct ImTimelineData {
    long long                 BeginTick;         // playable range [Begin, End)
    long long                 EndTick;
    long long                 PlayheadTick;
    char const*               DeterminismLabel;  // active profile (nullable)
    ImVector<ImTimelineTrack> Tracks;
    ImVector<ImTimelineKey>   Keys;
    ImVector<ImTimelineEvent> Events;
    ImVector<ImTimelineBand>  Bands;
};

enum ImTimelineActionKind {
    ImTimelineAction_None = 0,
    ImTimelineAction_Scrub,       // TargetTick = requested playhead
    ImTimelineAction_InsertKey,   // TargetTick + TrackIndex (past OK)
    ImTimelineAction_SelectKey,   // KeyId
    ImTimelineAction_DeleteKey,   // KeyId
};

struct ImTimelineAction {
    ImTimelineActionKind Kind;
    long long            TargetTick;
    int                  TrackIndex;
    unsigned             KeyId;
};
```

`ImTimeline` never mutates `ImTimelineData`: the host applies the reported action
and re-feeds the data.

> `ImTimelineTrack::Label` is a raw `char const*` with implicit lifetime -- it
> must outlive the call.
