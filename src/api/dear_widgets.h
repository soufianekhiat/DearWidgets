#include <imgui.h>

namespace ImWidgets {

#define ImWidget_Kibi (1024ull)
#define ImWidget_Mibi (ImWidget_Kibi*1024ull)
#define ImWidget_Gibi (ImWidget_Mibi*1024ull)
#define ImWidget_Tebi (ImWidget_Gibi*1024ull)
#define ImWidget_Pebi (ImWidget_Tebi*1024ull)

	typedef int ImWidgetsLengthUnit;

	enum ImWidgetsLengthUnit_
	{
		ImWidgetsLengthUnit_Metric = 0,
		ImWidgetsLengthUnit_Imperial,
		ImWidgetsLengthUnit_COUNT
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

	// Color Processing
	IMGUI_API bool HueToHue(const char* label);
	IMGUI_API bool LumToSat(const char* label);

	//////////////////////////////////////////////////////////////////////////
	// External
	//////////////////////////////////////////////////////////////////////////
	// By @thedmd: https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353
	void BeginGroupPanel(const char* name, const ImVec2& size = ImVec2(-1.0f, -1.0f));
	void EndGroupPanel();
}
