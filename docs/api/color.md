# Color Utilities

All color conversion functions are in the `ImWidgets` namespace. Inputs and outputs are normalized floats `[0, 1]` unless noted.

---

## Color Space Conversions

All functions follow the convention: `ColorConvertAtoB(out_x, out_y, out_z, in_x, in_y, in_z)`.

### sRGB <-> Linear
```cpp
void ColorConvertRGBtoLinear(float& out_L, float& out_a, float& out_b, float r, float g, float b);
void ColorConvertLineartoRGB(float& out_r, float& out_g, float& out_b, float L, float a, float b);
```

### sRGB <-> HSV
```cpp
void ColorConvertRGBtoHSV(float& out_h, float& out_s, float& out_v, float r, float g, float b);
void ColorConvertHSVtoRGB(float& out_r, float& out_g, float& out_b, float h, float s, float v);
```

### sRGB <-> HSL
```cpp
void ColorConvertRGBtoHSL(float r, float g, float b, float& out_h, float& out_s, float& out_l);
void ColorConvertHSLtoRGB(float h, float s, float l, float& out_r, float& out_g, float& out_b);
```

### sRGB <-> HSY (BT.709 Luma)
```cpp
void ColorConvertRGBtoHSY(float r, float g, float b, float& out_h, float& out_s, float& out_y);
void ColorConvertHSYtoRGB(float h, float s, float y, float& out_r, float& out_g, float& out_b);
```

### sRGB <-> HSP (Perceived brightness)
```cpp
void ColorConvertRGBtoHSP(float r, float g, float b, float& out_h, float& out_s, float& out_p);
void ColorConvertHSPtoRGB(float h, float s, float p, float& out_r, float& out_g, float& out_b);
void ColorConvertRGBtoHSPLog(float r, float g, float b, float& out_h, float& out_s, float& out_p);
void ColorConvertHSPLogtoRGB(float h, float s, float pLog, float& out_r, float& out_g, float& out_b);
```

### sRGB <-> OkLab
```cpp
void ColorConvertRGBtoOKLAB(float& out_L, float& out_a, float& out_b, float r, float g, float b);
void ColorConvertOKLABtoRGB(float& out_r, float& out_g, float& out_b, float L, float a, float b);
```

### sRGB <-> OkLCH
```cpp
void ColorConvertsRGBtoOKLCH(float& out_L, float& out_c, float& out_h, float r, float g, float b);
void ColorConvertOKLCHtosRGB(float& out_r, float& out_g, float& out_b, float L, float c, float h);
void ColorConvertOKLCHtoOKLAB(float& out_L, float& out_a, float& out_b, float r, float g, float b);
void ColorConvertOKLABtoOKLCH(float& out_r, float& out_g, float& out_b, float L, float a, float b);
```

### sRGB <-> XYZ (D65)
```cpp
void ColorConvertsRGBtoXYZ(float& out_X, float& out_Y, float& out_Z, float r, float g, float b);
void ColorConvertXYZtosRGB(float& out_r, float& out_g, float& out_b, float X, float Y, float Z);
```

### XYZ <-> CIE L*a*b* (D65)
```cpp
void ColorConvertXYZtoCIELab(float& out_L, float& out_a, float& out_b, float X, float Y, float Z);
void ColorConvertCIELabtoXYZ(float& out_X, float& out_Y, float& out_Z, float L, float a, float b);
void ColorConvertsRGBtoCIELab(float& out_L, float& out_a, float& out_b, float r, float g, float b);
void ColorConvertCIELabtosRGB(float& out_r, float& out_g, float& out_b, float L, float a, float b);
```

### XYZ <-> xyY
```cpp
void ColorConvertXYZtoxyY(float& out_x, float& out_y, float& out_Y, float X, float Y, float Z);
void ColorConvertxyYtoXYZ(float& out_X, float& out_Y, float& out_Z, float x, float y, float Yval);
```

---

## Color Blending

All blend functions take two `ImU32` RGBA colors and a blend factor `t in [0,1]`, and return the interpolated `ImU32`.

```cpp
ImU32 ImColorBlendsRGB(ImU32 col0, ImU32 col1, float t);      // sRGB interpolation
ImU32 ImColorBlendLinear(ImU32 col0, ImU32 col1, float t);    // Linear (gamma-correct)
ImU32 ImColorBlendHSL(ImU32 col0, ImU32 col1, float t);       // HSL (shortest-hue path)
ImU32 ImColorBlendHSLa(ImU32 col0, ImU32 col1, float t);      // HSL (shortest-hue + alpha)
ImU32 ImColorBlendHWB(ImU32 col0, ImU32 col1, float t);       // HWB color model
ImU32 ImColorBlendLCH(ImU32 col0, ImU32 col1, float t);       // CIE LCH
ImU32 ImColorBlendLab(ImU32 col0, ImU32 col1, float t);       // CIE Lab
ImU32 ImColorBlendOklab(ImU32 col0, ImU32 col1, float t);     // OkLab (perceptual)
ImU32 ImColorBlendOkLCH(ImU32 col0, ImU32 col1, float t);     // OkLCH (perceptual polar)
```

### `GradientSample`
```cpp
ImVec4 GradientSample(const ImGradientData& gradient, float t);
```
Sample the gradient at position `t in [0,1]`. Interpolation mode is taken from `gradient.Interpolation`.

---

## Gradient Interpolation Modes

```cpp
enum ImWidgetsGradientInterp_
{
    ImWidgetsGradientInterp_sRGB       = 0,
    ImWidgetsGradientInterp_LinearSRGB,
    ImWidgetsGradientInterp_OkLab,
    ImWidgetsGradientInterp_OkLCH,
    ImWidgetsGradientInterp_HSV,
};
```

---

## Temperature

```cpp
ImU32 KelvinTemperatureTosRGBColors(float temperature);
```
Convert a color temperature in Kelvin `[1000, 12000]` to an sRGB `ImU32`.

---

## Color Space Enum

Used by `DrawChromaticityPoints` and related functions.

```cpp
enum ImWidgetsColorSpace_
{
    ImWidgetsColorSpace_AdobeRGB,
    ImWidgetsColorSpace_AppleRGB,
    ImWidgetsColorSpace_Best,
    ImWidgetsColorSpace_Beta,
    ImWidgetsColorSpace_Bruce,
    ImWidgetsColorSpace_CIERGB,
    ImWidgetsColorSpace_ColorMatch,
    ImWidgetsColorSpace_Don_RGB_4,
    ImWidgetsColorSpace_ECI,
    ImWidgetsColorSpace_Ekta_Space_PS5,
    ImWidgetsColorSpace_NTSC,
    ImWidgetsColorSpace_PAL_SECAM,
    ImWidgetsColorSpace_ProPhoto,
    ImWidgetsColorSpace_SMPTE_C,
    ImWidgetsColorSpace_sRGB,
    ImWidgetsColorSpace_WideGamutRGB,
    ImWidgetsColorSpace_Rec2020,
};
```

---

## Misc Math Helpers

```cpp
float ImsRGBToLinear(float x);
float ImLinearTosRGB(float x);
float ImNormalize01(float x, float min, float max);
float ImScaleFromNormalized(float x, float newMin, float newMax);
float ImRescale(float x, float min, float max, float newMin, float newMax);
float ImLinearSample(float t, float* buffer, int count);
float ImFunctionFromData(float x, float minX, float maxX, float* data, int count);
ImU32 ImColorFrom_xyz(float x, float y, float z, float* xyzToRGB, float gamma);
```
