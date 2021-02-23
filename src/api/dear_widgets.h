#include <imgui.h>

//////////////////////////////////////////////////////////////////////////
// Style TODO:
//	* Expose Style
//	* Line Thickness for Slider2D
//	* Line Color for Slider2D
//
// Known issue:
//	* Slider2DInt must take into account the half pixel
//	* Move 2D must store state "IsDrag"
//
// Optim TODO:
//	* ChromaticPlot draw only the internal MultiColorQuad where its needed "inside" or at least leave the option for style "transparency etc"
//	* ChromaticPlot: Bake some as much as possible values: provide different version: ChromaticPlotDynamic {From enum and compute info at each frame}, ChromaticPlotFromData {From Baked data}
//
// Write use case for:
//	* HueToHue:
//		- Color Remap
//	* LumToSat:
//		- Color Remap
//	* ColorRing:
//		- HDR Color Management {Shadow, MidTone, Highlight}
//	* Grid2D_AoS_Float:
//		- Color Remap:
//////////////////////////////////////////////////////////////////////////

namespace ImWidgets {

#define ImWidget_Kibi (1024ull)
#define ImWidget_Mibi (ImWidget_Kibi*1024ull)
#define ImWidget_Gibi (ImWidget_Mibi*1024ull)
#define ImWidget_Tebi (ImWidget_Gibi*1024ull)
#define ImWidget_Pebi (ImWidget_Tebi*1024ull)

	typedef int ImWidgetsLengthUnit;
	typedef int ImWidgetsChromaticPlot;
	typedef int ImWidgetsObserver;
	typedef int ImWidgetsIlluminance;
	typedef int ImWidgetsColorSpace;

	enum ImWidgetsLengthUnit_
	{
		ImWidgetsLengthUnit_Metric = 0,
		ImWidgetsLengthUnit_Imperial,
		ImWidgetsLengthUnit_COUNT
	};

	enum ImWidgetsObserver_
	{
		// Standard
		ImWidgetsObserverChromaticPlot_1931_2deg = 0,
		ImWidgetsObserverChromaticPlot_1964_10deg,
		ImWidgetsObserverChromaticPlot_COUNT
	};

	enum ImWidgetsIlluminance_
	{
		// White Points
		ImWidgetsWhitePointChromaticPlot_A = 0,
		ImWidgetsWhitePointChromaticPlot_B,
		ImWidgetsWhitePointChromaticPlot_C,
		ImWidgetsWhitePointChromaticPlot_D50,
		ImWidgetsWhitePointChromaticPlot_D55,
		ImWidgetsWhitePointChromaticPlot_D65,
		ImWidgetsWhitePointChromaticPlot_D75,
		ImWidgetsWhitePointChromaticPlot_D93,
		ImWidgetsWhitePointChromaticPlot_E,
		ImWidgetsWhitePointChromaticPlot_F1,
		ImWidgetsWhitePointChromaticPlot_F2,
		ImWidgetsWhitePointChromaticPlot_F3,
		ImWidgetsWhitePointChromaticPlot_F4,
		ImWidgetsWhitePointChromaticPlot_F5,
		ImWidgetsWhitePointChromaticPlot_F6,
		ImWidgetsWhitePointChromaticPlot_F7,
		ImWidgetsWhitePointChromaticPlot_F8,
		ImWidgetsWhitePointChromaticPlot_F9,
		ImWidgetsWhitePointChromaticPlot_F10,
		ImWidgetsWhitePointChromaticPlot_F11,
		ImWidgetsWhitePointChromaticPlot_F12,
		ImWidgetsWhitePointChromaticPlot_COUNT
	};

	enum ImWidgetsColorSpace_
	{
		// Color Spaces
		ImWidgetsColorSpace_AdobeRGB = 0,	// D65
		ImWidgetsColorSpace_AppleRGB,		// D65
		ImWidgetsColorSpace_Best,			// D50
		ImWidgetsColorSpace_Beta,			// D50
		ImWidgetsColorSpace_Bruce,			// D65
		ImWidgetsColorSpace_CIERGB,			// E
		ImWidgetsColorSpace_ColorMatch,		// D50
		ImWidgetsColorSpace_Don_RGB_4,		// D50
		ImWidgetsColorSpace_ECI,			// D50
		ImWidgetsColorSpace_Ekta_Space_PS5,	// D50
		ImWidgetsColorSpace_NTSC,			// C
		ImWidgetsColorSpace_PAL_SECAM,		// D65
		ImWidgetsColorSpace_ProPhoto,		// D50
		ImWidgetsColorSpace_SMPTE_C,		// D65
		ImWidgetsColorSpace_sRGB,			// D65
		ImWidgetsColorSpace_WideGamutRGB,	// D50
		ImWidgetsColorSpace_Rec2020,		// D65
		ImWidgetsColorSpace_COUNT
	};

	enum ImWidgetsChromaticPlot_
	{

		// Style
		ImWidgetsChromaticPlot_ShowWavelength,
		ImWidgetsChromaticPlot_ShowGrid,
		ImWidgetsChromaticPlot_ShowPrimaries,
		ImWidgetsChromaticPlot_ShowWhitePoint,

		ImWidgetsChromaticPlot_COUNT
	};

	// Layout
	IMGUI_API void CenterNextItem(ImVec2 nextItemSize);

	// Numbers, Scalar
	IMGUI_API bool DragFloatLog(const char* label, float* data, float v_speed, float log_basis = 10.0f, const void* p_min = NULL, const void* p_max = NULL, ImGuiSliderFlags flags = 0);

	IMGUI_API bool DragLengthScalar(const char* label, ImGuiDataType data_type, void* p_data, ImWidgetsLengthUnit* p_defaultUnit, float v_speed, const void* p_min = NULL, const void* p_max = NULL, ImGuiSliderFlags flags = 0);

	IMGUI_API bool Slider2DScalar(char const* pLabel, ImGuiDataType data_type, void* pValueX, void* pValueY, void* p_minX, void* p_maxX, void* p_minY, void* p_maxY, float const fScale = 1.0f);

	IMGUI_API bool Slider2DInt(char const* pLabel, int* pValueX, int* pValueY, int* p_minX, int* p_maxX, int* p_minY, int* p_maxY, float const fScale = 1.0f);
	IMGUI_API bool Slider2DFloat(char const* pLabel, float* pValueX, float* pValueY, float* p_minX, float* p_maxX, float* p_minY, float* p_maxY, float const fScale = 1.0f);
	IMGUI_API bool Slider2DDouble(char const* pLabel, double* pValueX, double* pValueY, double* p_minX, double* p_maxX, double* p_minY, double* p_maxY, float const fScale = 1.0f);

	IMGUI_API bool SliderScalar3D(char const* pLabel, float* pValueX, float* pValueY, float* pValueZ, float const fMinX, float const fMaxX, float const fMinY, float const fMaxY, float const fMinZ, float const fMaxZ, float const fScale = 1.0f);

	IMGUI_API bool InputVec2(const char* label, ImVec2* value, ImVec2* p_vMinValue, ImVec2* p_vMaxValue, float const fScale = 1.0f);
	IMGUI_API bool InputVec3(const char* label, ImVec4* value, ImVec4 const vMinValue, ImVec4 const vMaxValue, float const fScale = 1.0f);

	// Grid
	// Default behavior: AoS & RowMajor
	IMGUI_API bool Grid2D_AoS_Float(const char* label, float* buffer, int rows, int columns, float minX, float maxX, float minY, float maxY);

	// Core
	IMGUI_API bool PlaneMovePoint2D(const char* label, float* buffer_aot, int float2_count, float minX, float maxX, float minY, float maxY);
	IMGUI_API bool MoveLine2D(const char* label, float* buffer_aot, int float2_count, float minX, float maxX, float minY, float maxY);

	// Widgets
	// Density Plot
	IMGUI_API bool DensityPlotBilinear(const char* label, float(*sample)(float x, float y), int resX, int resY, float minX, float maxX, float minY, float maxY);
	IMGUI_API bool DensityPlotNearest(const char* label, float(*sample)(float x, float y), int resX, int resY, float minX, float maxX, float minY, float maxY);

	IMGUI_API bool DensityIsolinePlotBilinear(const char* label, float(*sample)(float x, float y), float* isoValue, int isoLinesCount, ImU32* isoLinesColors, int isolinesColorsCount, int resX, int resY, float minX, float maxX, float minY, float maxY);

	// Plots
	IMGUI_API void	AnalyticalPlot(char const* label, float(*func)(float const x), float const minX, float const maxX, int const minSamples = 64);

	// Draws
	// Mask
	IMGUI_API void DrawConvexMaskMesh(ImDrawList* pDrawList, ImVec2 curPos, float* buffer_aot, int float2_count, ImVec2 size);

	// xyzToRGB: a rowMajorMatrix
	IMGUI_API void DrawChromaticPlotNearest(ImDrawList* pDrawList,
		ImVec2 const vPos,
		float width, float heigth,
		int const chromeLineSamplesCount,
		ImWidgetsColorSpace const colorspace,
		ImWidgetsObserver const observer,
		ImWidgetsIlluminance const illum,
		int resX, int resY,
		float wavelengthMin = 400.0f, float wavelengthMax = 700.0f,
		float minX = 0.0f, float maxX = 0.8f,
		float minY = 0.0f, float maxY = 0.9f);
	IMGUI_API void DrawChromaticPlotBilinear(ImDrawList* pDrawList,
		ImVec2 const vPos,
		float width, float heigth,
		int const chromeLineSamplesCount,
		ImWidgetsColorSpace const colorspace,
		ImWidgetsObserver const observer,
		ImWidgetsIlluminance const illum,
		int resX, int resY,
		float wavelengthMin = 400.0f, float wavelengthMax = 700.0f,
		float minX = 0.0f, float maxX = 0.8f,
		float minY = 0.0f, float maxY = 0.9f);
	IMGUI_API void DrawChromaticPoint(ImDrawList* pDrawList, ImVec2 const vpos, ImU32 col);
	IMGUI_API void DrawChromaticLine(ImDrawList* pDrawList, ImVec2 const* vpos, int const pts_counts, ImU32 col, bool closed, float thickness);

	// Color Processing
	IMGUI_API bool HueToHue(const char* label);
	IMGUI_API bool LumToSat(const char* label);
	IMGUI_API bool ColorRing(const char* label, float thickness, int split);

	//////////////////////////////////////////////////////////////////////////
	// External
	//////////////////////////////////////////////////////////////////////////
	// By @thedmd: https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353
	void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(-1.0f, -1.0f));
	void EndGroupPanel();

	// By @r-lyeh: https://github.com/ocornut/imgui/issues/786#issuecomment-479539045
	void ShowBezierDemo();

	// By @nem0: https://github.com/ocornut/imgui/issues/786#issuecomment-358106252
	enum class CurveEditorFlags
	{
		NO_TANGENTS = 1 << 0,
		SHOW_GRID = 1 << 1
	};
	int CurveEditor(const char* label, float* values, int points_count, const ImVec2& editor_size, ImU32 flags, int* new_count);
}
