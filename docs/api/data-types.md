# Data Types

Data structs used as inputs/outputs for widgets and drawing functions.

---

## Gradient

### `ImGradientStop`
```cpp
struct ImGradientStop {
    float  Position;  // [0, 1]
    ImVec4 Color;     // RGBA in sRGB, float [0,1]
};
```

### `ImGradientData`
```cpp
struct ImGradientData {
    ImVector<ImGradientStop>  Stops;
    ImWidgetsGradientInterp   Interpolation; // sRGB, LinearSRGB, OkLab, OkLCH, HSV
    int                       SelectedIdx;  // runtime editor state, -1 = none
};
```
Methods:
- `int  AddStop(float pos, ImVec4 col)` — add a stop and return its sorted index.
- `bool RemoveStop(int idx)` — remove stop (minimum 2 stops enforced).
- `void SortStops()` — sort by position (called automatically by `AddStop`).

---

## Animation Curve

### `ImCurveEditorKey`
```cpp
struct ImCurveEditorKey {
    ImVec2                 Pos;           // (x=time, y=value)
    ImCurveEditorSeg       Segment;       // Interpolation from this key to the next
    ImVec2                 TangentLeft;   // Incoming handle offset (typically negative x)
    ImVec2                 TangentRight;  // Outgoing handle offset (typically positive x)
    ImCurveEditorTangentMode TangentMode; // Free, Aligned, or Mirrored
};
```

### `ImCurveEditorData`
```cpp
struct ImCurveEditorData {
    ImVector<ImCurveEditorKey> Keys;
    ImVec2                     RangeMin;    // Visible range (x=time, y=value)
    ImVec2                     RangeMax;
    int                        SelectedIdx; // -1 = none
    int                        DragTarget;  // 0=key, -1=left tangent, 1=right tangent
};
```
Methods:
- `int  AddKey(ImVec2 pos, ImCurveEditorSeg seg)` — inserts a key, splitting the existing Bézier at that time via De Casteljau.
- `bool RemoveKey(int idx)` — removes key (minimum 2 keys enforced).
- `void SortKeys()` — sort by time.

**Segment types (`ImCurveEditorSeg`):** `StepStart`, `StepEnd`, `StepCenter`, `Linear`, `InQuad…InOutBounce`, `CubicBezier` (15+ easing types).

**Tangent modes (`ImCurveEditorTangentMode`):** `Free`, `Aligned`, `Mirrored`.

Helper:
```cpp
float CurveEditorEvalEasing(ImCurveEditorSeg seg, float t); // t ∈ [0,1]
float CurveEditorSample(const ImCurveEditorData& curve, float x);
const char* CurveEditorSegName(ImCurveEditorSeg seg);
const char* CurveEditorTangentModeName(ImCurveEditorTangentMode mode);
```

---

## Color Curves

### `ImColorCurveKey`
```cpp
struct ImColorCurveKey {
    float Position;       // X position [0,1]
    float Value;          // Y value (range depends on mode)
    ImCurveEditorSeg Segment;
    ImVec2 TangentLeft, TangentRight;
    ImCurveEditorTangentMode TangentMode;
};
```

### `ImColorCurveData`
```cpp
struct ImColorCurveData {
    ImVector<ImColorCurveKey> Keys;
    int                       SelectedIdx;
};
```
Methods: `AddKey`, `RemoveKey`, `SortKeys`.

### Color Curve Modes (`ImColorCurveMode`)

| Mode | X axis | Y axis | Y range |
|---|---|---|---|
| `HueVsHue` | Hue [0,1] | Hue shift | [-0.5, 0.5] |
| `HueVsSat` | Hue [0,1] | Saturation multiplier | [0, 2] |
| `HueVsLum` | Hue [0,1] | Luminance offset | [-1, 1] |
| `LumVsSat` | Luminance [0,1] | Saturation multiplier | [0, 2] |
| `SatVsSat` | Saturation [0,1] | Saturation multiplier | [0, 2] |

```cpp
float       ColorCurveDefaultValue(ImColorCurveMode mode);
void        ColorCurveRange(ImColorCurveMode mode, float* out_min, float* out_max);
const char* ColorCurveModeName(ImColorCurveMode mode);
float       ColorCurveSample(const ImColorCurveData& curve, ImColorCurveMode mode,
                              float x, bool advancedSegments = false);
```

---

## Tone Curve

### `ImToneCurveData`
```cpp
struct ImToneCurveData {
    ImColorCurveData Channels[4]; // Per-channel curves (up to 4)
    int              ActiveChannel;
};
```
Methods: `Reset(int channelCount)`.

```cpp
int         ToneCurveChannelCount(ImHistogramMode mode);
const char* ToneCurveChannelName(ImHistogramMode mode, int channel);
float       ToneCurveSample(const ImColorCurveData& curve, float x,
                             bool advancedSegments = false);
```

---

## Color Warper

### `ImColorWarperData`
```cpp
struct ImColorWarperData {
    int              HueDivisions;   // Divisions around hue (6, 12, or 24)
    int              SatDivisions;   // Divisions along saturation
    ImVector<ImVec2> Offsets;        // Per-point (hue_shift, sat_shift)
    ImVector<bool>   Pinned;         // Per-point pin state
    int              SelectedIdx;    // -1 = none
};
```
Index layout: `satIdx * HueDivisions + hueIdx`. `satIdx=0` = center, `satIdx=SatDivisions` = outer edge.

Methods: `Init(int hueDivs, int satDivs)`, `Reset()`, `PointCount()`, `PointIndex(hueIdx, satIdx)`, `GetIdentityPos(...)`, `GetWarpedPos(...)`.

### `ImColorWarperOverlay`
```cpp
struct ImColorWarperOverlay {
    ImVector<float> SampledRGB;  // [i*3+0..2] = r,g,b ∈ [0,1]
    int             SampleCount;
    void Accumulate(const void* data, int width, int height, int channels,
                    ImParadeBitDepth bitDepth, ImParadeLayout layout,
                    int maxSamples = 50000);
};
```

---

## Scopes

### `ImParadeScopeData`
```cpp
struct ImParadeScopeData {
    ImVector<ImU32> Bins;         // [ch * XBins * YBins + x * YBins + y]
    int             XBins, YBins, ChannelCount;
    ImU32           PeakCount;
    ImParadeMode    Mode;
    ImParadeBitDepth BitDepth;
    void Accumulate(const void* data, int width, int height, int channels,
                    ImParadeBitDepth bitDepth, ImParadeLayout layout,
                    ImParadeMode mode, int xBins = 128, int yBins = 128,
                    int maxSamples = 1000000);
};
```

**Parade modes:** `Luma`, `RGB`, `YRGB`, `YCbCr`
**Bit depths:** `UInt8` [0,255], `UInt10` [0,1023], `UInt16` [0,65535]
**Layouts:** `Interleaved` (RGBRGB…), `Planar` (RRR…GGG…)
**Scale:** `Linear`, `Log`, `InvLog`

### `ImVectorScopeData`
```cpp
struct ImVectorScopeData {
    ImVector<ImU32> Bins;       // [xBin * Resolution + yBin]
    int             Resolution;
    ImU32           PeakCount;
    ImParadeBitDepth BitDepth;
    void Accumulate(const void* data, int width, int height, int channels,
                    ImParadeBitDepth bitDepth, ImParadeLayout layout,
                    int resolution = 256, int maxSamples = 1000000);
};
```

### `ImHistogramData`
```cpp
struct ImHistogramData {
    ImVector<ImU32> Bins;           // [ch * BinCount + bin]
    int             BinCount, ChannelCount;
    ImU32           PeakCount;
    ImHistogramMode Mode;
    ImParadeBitDepth BitDepth;
    void Accumulate(const void* data, int width, int height, int channels,
                    ImParadeBitDepth bitDepth, ImParadeLayout layout,
                    ImHistogramMode mode, int binCount = 256,
                    int maxSamples = 1000000);
};
```
**Histogram modes:** `Luma`, `RGB`, `YRGB`, `YCbCr`, `HSV`, `OkLCH`
**Layouts:** `Overlapped`, `Stacked`

### `ImCIEChromaticityData`
```cpp
struct ImCIEChromaticityData {
    ImVector<float> SampledRGB;  // [i*3+0..2] = r,g,b ∈ [0,1]
    int             SampleCount;
    void Accumulate(const void* data, int width, int height, int channels,
                    ImParadeBitDepth bitDepth, ImParadeLayout layout,
                    int maxSamples = 50000);
};
```
**CIE gamuts:** `sRGB_Rec709`, `Rec2020`, `DCI_P3`, `ACEScg`, `AdobeRGB`, `ProPhoto`

---

## Transform

### `ImTransformData`
```cpp
struct ImTransformData {
    ImVec2 Translation;  // Offset from canvas center (px)
    float  Rotation;     // Rotation angle (radians)
    ImVec2 Scale;        // Scale factor (1,1 = fit to canvas)
};
```

### `ImTransformImage`
```cpp
struct ImTransformImage {
    ImTextureID     Texture;
    ImVec2          TexSize;    // Original image dimensions
    ImTransformData Transform;
};
```

---

## Paint Canvas

### `ImPaintCanvasData`
```cpp
struct ImPaintCanvasData {
    void*                  Pixels;       // User-owned pixel buffer
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
`DestroyTexture()` releases the internal GPU texture. Call before freeing `Pixels` or resizing.

---

## Image Viewer

### `ImImageViewerState`
```cpp
struct ImImageViewerState {
    float                  Zoom;         // Display zoom: 1.0 = fit image to widget
    ImVec2                 Pan;          // Image-space pan from image centre
    const void*            Pixels;       // Optional CPU buffer for pixel inspector
    ImVec2                 PixelSize;    // Dimensions of Pixels buffer
    ImPlatform_PixelFormat PixelFormat;
};
```

---

## Unit Field

### `ImUnitDef`
```cpp
struct ImUnitDef {
    const char*          name;          // e.g. "meter"
    const char*          abbreviation;  // e.g. "m"
    float                mul;           // base * mul + add = display
    float                add;
    ImUnitConvertCallback toDisplay;    // Custom callback (overrides mul/add)
    ImUnitConvertCallback toBase;
    void*                pUserData;
};
```
Factory helpers:
```cpp
ImUnitDef ImUnitDef_Simple(const char* name, const char* abbr, float mul, float add = 0.0f);
ImUnitDef ImUnitDef_Custom(const char* name, const char* abbr,
                            ImUnitConvertCallback toDisplay,
                            ImUnitConvertCallback toBase, void* pUserData = NULL);
```
