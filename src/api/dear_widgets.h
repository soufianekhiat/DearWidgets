#include <imgui.h>

namespace ImWidgets {

#define ImWidget_Kibi (1024ull)
#define ImWidget_Mibi (ImWidget_Kibi*1024ull)
#define ImWidget_Gibi (ImWidget_Mibi*1024ull)
#define ImWidget_Tebi (ImWidget_Gibi*1024ull)
#define ImWidget_Pebi (ImWidget_Tebi*1024ull)

	typedef int ImWidgetsLengthUnit;
	typedef int ImWidgetsChromaticPlot;

	enum ImWidgetsLengthUnit_
	{
		ImWidgetsLengthUnit_Metric = 0,
		ImWidgetsLengthUnit_Imperial,
		ImWidgetsLengthUnit_COUNT
	};

	enum ImWidgetsChromaticPlot_
	{
		// White Points
		ImWidgetsChromaticPlot_C,
		ImWidgetsChromaticPlot_D50,
		ImWidgetsChromaticPlot_D65,
		ImWidgetsChromaticPlot_E,

		// Standard
		ImWidgetsChromaticPlot_1931_2deg,
		ImWidgetsChromaticPlot_1964_10deg,

		// Color Spaces
		ImWidgetsChromaticPlot_NTSC,		// C
		ImWidgetsChromaticPlot_EBU,			// D65
		ImWidgetsChromaticPlot_SMPTE,		// D65
		ImWidgetsChromaticPlot_HDTV,		// D65
		ImWidgetsChromaticPlot_CIE,			// E
		ImWidgetsChromaticPlot_sRGB,		// D65
		ImWidgetsChromaticPlot_Adobe,		// D65
		ImWidgetsChromaticPlot_ColorMatch,	// D50
		ImWidgetsChromaticPlot_ProPhoto,	// D50

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

	// Color Processing
	IMGUI_API bool HueToHue(const char* label);
	IMGUI_API bool LumToSat(const char* label);
	IMGUI_API bool ColorRing(const char* label, float thickness, int split);

	// xyzToRGB: a rowMajorMatrix
	IMGUI_API bool ChromaticPlotInternalBilinear(const char* label, ImVec2 primR, ImVec2 primG, ImVec2 primB, ImVec2 whitePoint, float* xyzToRGB, float gamma, int resX, int resY, float minX = 0.0f, float maxX = 0.8f, float minY = 0.0f, float maxY = 0.9f);
	//IMGUI_API bool ChromaticPlot(const char* label, int resX, int resY, ImWidgetsChromaticPlot flags);

	IMGUI_API bool DensityPlotBilinear(const char* label, float(*sample)(float x, float y), int resX, int resY, float minX, float maxX, float minY, float maxY);
	IMGUI_API bool DensityPlotNearest(const char* label, float(*sample)(float x, float y), int resX, int resY, float minX, float maxX, float minY, float maxY);

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
