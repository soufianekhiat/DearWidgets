#include <demo.h>
#include <dear_widgets.h>

#include <vector>
#include <random>

static int grid_rows = 8;
static int grid_columns = 8;
static std::vector<float> grid_values;

static std::vector<float> linear_values;

std::random_device rd;
std::mt19937_64 gen(rd());
std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

class StaticInit
{
public:
	StaticInit()
	{
		for (int j = 0; j < grid_rows; ++j)
		{
			for (int i = 0; i < grid_columns; ++i)
			{
				float x = ((float)i) / ((float)(grid_columns - 1));
				float y = ((float)j) / ((float)(grid_rows - 1));

				grid_values.push_back(x);
				grid_values.push_back(y);
			}

			linear_values.push_back(dis(gen)*0.5f);
			linear_values.push_back(dis(gen));
		}

		ImVec2* pVec2Buffer = reinterpret_cast<ImVec2*>(&linear_values[0]);

		std::sort(pVec2Buffer, pVec2Buffer + linear_values.size() / 2,
			[](ImVec2 const& a, ImVec2 const& b)
			{
				return a.x < b.x;
			});
	}
};

ImVec2 TemperatureTo_xy(float TT)
{
	double T = (float)TT;
	double xc, yc;
	double const invT = 1.0 / T;
	double const invT2 = 1.0 / (T * T);
	double const invT3 = (1.0 / (T * T)) / T;
	double const _10_9 = 1e9 / (T * T * T);
	double const _10_6 = 1e6 / (T * T);
	double const _10_3 = 1e3 / (T);
	if (/*T >= 1667.0f &&*/ T <= 4000.0f)
		//xc = -0.2661239f * _10_9 * invT3 - 0.2343589f * _10_6 * invT2 + 0.8776956f * _10_3 * invT + 0.179910f;
		xc = -0.2661239 * _10_9 - 0.2343589 * _10_6 + 0.8776956 * _10_3 + 0.179910;
	else //if (x = 25000.0f)
		//xc = -3.0258469f * _10_9 * invT3 + 2.1070379 * _10_6 * invT2 + 0.2226347f * _10_3 * invT + 0.240390f;
		xc = -3.0258469 * _10_9 + 2.1070379 * _10_6 + 0.2226347 * _10_3 + 0.240390;

	double const xc2 = xc * xc;
	double const xc3 = xc2 * xc;

	if (/*T >= 1667.0f &&*/ T <= 2222.0f)
		yc = -1.1063814 * xc3 - 1.34811020 * xc2 + 2.18555832 * xc - 0.20219683;
	else if (T < 4000.0f)
		yc = -0.9549476 * xc3 - 1.37418593 * xc2 + 2.09137015 * xc - 0.16748867;
	else //if (T <= 25000.0f)
		yc = +3.0817580 * xc3 - 5.87338670 * xc2 + 3.75112997 * xc - 0.37001483;

	return ImVec2((float)xc, (float)yc);
}

namespace ImWidgets {
	void	ShowDemo()
	{
		static StaticInit s_StaticInit;
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Dear Widgets");

		static const float fZero = 0.0f;
		static float length = 16.0f;
		static ImWidgetsLengthUnit currentUnit = ImWidgetsLengthUnit_Metric;
		//DragLengthScalar("DragLengthScalar", ImGuiDataType_Float, &length, &currentUnit, 1.0f, &fZero, nullptr, ImGuiSliderFlags_None);

		float const width = ImGui::GetContentRegionAvailWidth() * 0.75;

		bool show_app_metrics = true;
		ImGui::ShowMetricsWindow(&show_app_metrics);

		//ShowBezierDemo();
		ImWidgets::BeginGroupPanel("Chromaticity Plot");
		{
			char const* observer[] = { "1931 2°", "1964 10°" };
			//char const* illum[] = { "A", "B", "C", "D50", "D55", "D65", "D75", "D93", "E", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12" };
			char const* illum[] = { "D50", "D65" };
			char const* colorSpace[] = { "AdobeRGB", "AppleRGB", "Best", "Beta", "Bruce", "CIERGB",
				"ColorMatch", "Don_RGB_4", "ECI","Ekta_Space_PS5", "NTSC",
				"PAL_SECAM", "ProPhoto", "SMPTE_C", "sRGB", "WideGamutRGB", "Rec2020" };
			static int curObserver = 0;
			static int curIllum = 1;
			static int curColorSpace = 0;
			ImGui::Combo("Observer", &curObserver, observer, IM_ARRAYSIZE(observer));
			ImGui::Combo("Illuminance", &curIllum, illum, IM_ARRAYSIZE(illum));
			ImGui::Combo("ColorSpace", &curColorSpace, colorSpace, IM_ARRAYSIZE(colorSpace));
			float const width = ImGui::GetContentRegionAvailWidth();
			DrawChromaticPlotBilinear(
				ImGui::GetWindowDrawList(),
				ImGui::GetCursorScreenPos(),
				width, width,
				256,
				curColorSpace,
				curObserver,
				curIllum, // == 0 ? ImWidgetsWhitePointChromaticPlot_D50 : ImWidgetsWhitePointChromaticPlot_D65,
				32, 32,
				400.0f, 700.0f,
				-0.1f, 0.9f,
				-0.1f, 0.9f);
			std::vector< ImVec2 > oPts;
			for (float T = 1667.0f; T <= 25000.0f; T += (25000.0f - 1667.0f)/128.0f)
			{
				oPts.push_back(TemperatureTo_xy(T));
				//DrawChromaticPoint(ImGui::GetWindowDrawList(), TemperatureTo_xy(T), IM_COL32(0, 0, 0, 255));
			}
			DrawChromaticLine(ImGui::GetWindowDrawList(), &oPts[0], oPts.size(), IM_COL32(0, 0, 0, 255), false, 1.0f);
			//DrawChromaticPoint(ImGui::GetWindowDrawList(), ImVec2(0.5f, 0.5f), IM_COL32(0, 0, 0, 255));
			ImGui::InvisibleButton("##Zone", ImVec2(width, width), 0);
		}
		ImWidgets::EndGroupPanel();
		ImWidgets::BeginGroupPanel("Density Plot Nearest");
		{
			DensityPlotNearest("Dense SS N", [](float x, float y) -> float { return std::sin(x) * std::sin(y); }, 32, 32, -4.0f, 4.0f, -3.0f, 3.0f);
			DensityPlotNearest("Dense S N", [](float x, float y) -> float { return std::sin(x * y); }, 32, 32, 0.0f, 4.0f, 0.0f, 4.0f);
		}
		ImWidgets::EndGroupPanel();
		ImWidgets::BeginGroupPanel("Density Plot Bilinear");
		{
			DensityPlotBilinear("Dense SS B", [](float x, float y) -> float { return std::sin(x) * std::sin(y); }, 32, 32, -4.0f, 4.0f, -3.0f, 3.0f);
			DensityPlotBilinear("Dense S B", [](float x, float y) -> float { return std::sin(x * y); }, 32, 32, 0.0f, 4.0f, 0.0f, 4.0f);
		}
		ImWidgets::EndGroupPanel();
		ImWidgets::BeginGroupPanel("Ring Color");
		{
			ColorRing("Ring Color", 8.0f, 64);
		}
		//ImWidgets::BeginGroupPanel("Curve Editor");
		//{
		//	CurveEditor("Edit Curve", &linear_values[0], linear_values.size() / 2, ImVec2(width, width * 0.5f), (ImU32)CurveEditorFlags::NO_TANGENTS | (ImU32)CurveEditorFlags::SHOW_GRID, nullptr);
		//}
		ImWidgets::EndGroupPanel();
		ImWidgets::BeginGroupPanel("2D Move");
		{
			MoveLine2D("Region", &linear_values[0], linear_values.size() / 2, -1.0f, 1.0f, -1.0f, 1.0f);
			//PlaneMovePoint2D("Region", &linear_values[0], linear_values.size()/2, -1.0f, 1.0f, -1.0f, 1.0f);
		}
		ImWidgets::EndGroupPanel();
		ImWidgets::BeginGroupPanel("Colors");
		{
			HueToHue("Wesh");
			LumToSat("Wesh2");
		}
		ImWidgets::EndGroupPanel();
		ImWidgets::BeginGroupPanel("Slider 2D");
		{
			static ImVec2 slider2D;
			ImVec2 boundMin(-1.0f, -1.0f);
			ImVec2 boundMax(1.0f, 1.0f);
			Slider2DFloat("Slider 2D Float", &slider2D.x, &slider2D.y, &boundMin.x, &boundMax.x, &boundMin.y, &boundMax.y, 0.75f);
		}
		{
			static int curX = 0;
			static int curY = 0;
			int minX = -5;
			int minY = -5;
			int maxX =  5;
			int maxY =  5;
			Slider2DInt("Slider 2D Int", &curX, &curY, &minX, &maxX, &minY, &maxY, 0.75f);
		}
		ImWidgets::EndGroupPanel();
		ImWidgets::BeginGroupPanel("Slider 3D");
		{
			static ImVec4 cur3D;
			ImVec4 boundMin(-1.0f, -1.0f, -1.0f, 0.0f);
			ImVec4 boundMax(1.0f, 1.0f, 1.0f, 0.0f);
			SliderScalar3D("Slider 3D Float", &cur3D.x, &cur3D.y, &cur3D.z,
											  boundMin.x, boundMax.x,
											  boundMin.y, boundMax.y,
											  boundMin.z, boundMax.z,
											0.75f);
		}
		ImWidgets::EndGroupPanel();
		ImWidgets::BeginGroupPanel("Grids");
		{
			ImVec2 boundMin(-1.0f, -1.0f);
			ImVec2 boundMax(1.0f, 1.0f);
			Grid2D_AoS_Float("Slider 3D Float", &grid_values[0], grid_rows, grid_columns, boundMin.x, boundMax.x, boundMin.y, boundMax.y);
		}
		ImWidgets::EndGroupPanel();

		//ImGui::Separator();
		//ImGui::Text("External");
		//ImWidgets::BeginGroupPanel( "GroupPanel" );
		//	ImGui::Text("First");
		//	ImGui::Text("Second");
		//ImWidgets::EndGroupPanel();
		//
		//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}
