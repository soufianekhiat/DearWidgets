#include <demo.h>
#include <dear_widgets.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <vector>
#include <random>

static int grid_rows = 8;
static int grid_columns = 8;
static std::vector<float> grid_values;

static std::vector<float> linear_values;
static std::vector<float> maskShape_values;

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

		constexpr int ptsCount = 16;
		float const radius = 0.5f;
		for (int i = 0; i < ptsCount; ++i)
		{
			float const angle = 2.0f * IM_PI * ((float)i)/((float)(ptsCount - 1));

			float x = radius * ImCos(angle);
			float y = radius * ImSin(angle);

			maskShape_values.push_back(x);
			maskShape_values.push_back(y);
		}

		//ImVec2* pVec2Buffer = reinterpret_cast<ImVec2*>(&linear_values[0]);
		//
		//std::sort(pVec2Buffer, pVec2Buffer + linear_values.size() / 2,
		//	[](ImVec2 const& a, ImVec2 const& b)
		//	{
		//		return a.x < b.x;
		//	});
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

static inline ImVec4 operator*(const ImVec4& lhs, const float rhs) { return ImVec4(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs); }

#pragma region ShaderToyHelper
// Ref: https://www.shadertoy.com/view/WlSGW1
float sdHorseshoe(ImVec2 p, ImVec2 c, float r, ImVec2 w)
{
	p.x = ImAbs(p.x);
	float l = ImWidgets::ImLength(p);
	//p = mat2(-c.x, c.y, c.y, c.x) * p;
	p = ImVec2(-c.x * p.x + p.y * c.y, c.y * p.x + p.y * c.x);
	p = ImVec2((p.y > 0.0f) ? p.x : l * ImSign(-c.x), (p.x > 0.0f) ? p.y : l);
	p = ImVec2(p.x, ImAbs(p.y - r)) - w;
	return ImWidgets::ImLength(ImMax(p, ImVec2(0.0f, 0.0f))) + ImMin(0.0f, ImMax(p.x, p.y));
}

ImU32 sdHorseshoeColor(ImVec2 p, float fTime)
{
	float t = IM_PI * (0.3f + 0.3f * ImCos(fTime * 0.5f));
	ImVec2 tmp = ImVec2(0.7f, 1.1f) * fTime + ImVec2(0.0f, 2.0f);
	ImVec2 w = ImVec2(0.750f, 0.25f) * (ImVec2(0.5f, 0.5f) + ImVec2(ImCos(tmp.x), ImCos(tmp.y)) * 0.5f);

	// distance
	float d = sdHorseshoe(p - ImVec2(0.0f, -0.1f), ImVec2(ImCos(t), ImSin(t)), 0.5f, w);

	// coloring
	ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f) - ImVec4(0.1f, 0.4f, 0.7f, 1.0f) * ImSign(d);
	col = col * (1.0f - exp(-2.0f * ImAbs(d)));
	col = col * (0.8f + 0.2f * ImCos(120.0 * ImAbs(d)));
	col = ImLerp(col, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f - ImWidgets::ImSmoothStep(0.0f, 0.02f, ImAbs(d)));

	return IM_COL32( 255 * col.x, 255 * col.y, 255 * col.z, 255);
}
#pragma endregion ShaderToyHelper

namespace ImWidgets {
	void	ShowDemo()
	{
		static StaticInit s_StaticInit;
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Dear Widgets");

		float const width = ImGui::GetContentRegionAvailWidth() * 0.75;

		bool show_app_metrics = true;
		ImGui::ShowMetricsWindow(&show_app_metrics);

		if (ImGui::TreeNode("Draw"))
		{
			if (ImGui::TreeNode("Triangles Pointers"))
			{
				float const width = ImGui::GetContentRegionAvailWidth();

				ImVec2 curPos = ImGui::GetCursorScreenPos();
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				ImGui::InvisibleButton("##Zone", ImVec2(width, 72.0f), 0);
				float const fPointerLine = 32.0f;
				pDrawList->AddLine(ImVec2(curPos.x + 0.5f * 32.0f, curPos.y + fPointerLine), ImVec2(curPos.x + 3.5f * 32.0f, curPos.y + fPointerLine), IM_COL32(0, 255, 0, 255), 2.0f);
				pDrawList->AddLine(ImVec2(curPos.x + 5.0f * 32.0f, curPos.y), ImVec2(curPos.x + 5.0f * 32.0f, curPos.y + 72.0f), IM_COL32(0, 255, 0, 255), 2.0f);
				pDrawList->AddLine(ImVec2(curPos.x + 7.0f * 32.0f, curPos.y), ImVec2(curPos.x + 7.0f * 32.0f, curPos.y + 72.0f), IM_COL32(0, 255, 0, 255), 2.0f);
				pDrawList->AddCircleFilled(ImVec2(curPos.x + 1.0f * 32.0f, curPos.y + fPointerLine), 4.0f, IM_COL32(255, 128, 0, 255), 16);
				pDrawList->AddCircleFilled(ImVec2(curPos.x + 3.0f * 32.0f, curPos.y + fPointerLine), 4.0f, IM_COL32(255, 128, 0, 255), 16);
				pDrawList->AddCircleFilled(ImVec2(curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine), 4.0f, IM_COL32(255, 128, 0, 255), 16);
				pDrawList->AddCircleFilled(ImVec2(curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine), 4.0f, IM_COL32(255, 128, 0, 255), 16);
				ImWidgets::DrawTrianglePointerFilled(pDrawList, ImVec2(curPos.x + 1.0f * 32.0f, curPos.y + fPointerLine), 32.0f, IM_COL32(255, 0, 0, 255), ImWidgetsPointer_Up);
				ImWidgets::DrawTrianglePointerFilled(pDrawList, ImVec2(curPos.x + 3.0f * 32.0f, curPos.y + fPointerLine), 32.0f, IM_COL32(255, 0, 0, 255), ImWidgetsPointer_Down);
				ImWidgets::DrawTrianglePointerFilled(pDrawList, ImVec2(curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine), 32.0f, IM_COL32(255, 0, 0, 255), ImWidgetsPointer_Right);
				ImWidgets::DrawTrianglePointerFilled(pDrawList, ImVec2(curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine), 32.0f, IM_COL32(255, 0, 0, 255), ImWidgetsPointer_Left);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Color Bands"))
			{
				static float col[4] = { 1, 0, 0, 1 };
				ImGui::ColorEdit4("Color", col);
				float const width = ImGui::GetContentRegionAvailWidth();
				float const height = 32.0f;
				static float gamma = 1.0f;
				ImGui::DragFloat("Gamma##Color", &gamma, 0.01f, 0.1f, 10.0f);
				static int division = 32;
				ImGui::DragInt("Division", &division, 1, 1, 128);

				ImGui::Text("HueBand");
				DrawHueBand(ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2(width, height), division, col, col[3], gamma);
				ImGui::InvisibleButton("##Zone", ImVec2(width, height), 0);

				ImGui::Text("LuminanceBand");
				DrawLumianceBand(ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2(width, height), division, ImVec4(col[0], col[1], col[2], col[3]), gamma);
				ImGui::InvisibleButton("##Zone", ImVec2(width, height), 0);

				ImGui::Text("SaturationBand");
				DrawSaturationBand(ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2(width, height), division, ImVec4(col[0], col[1], col[2], col[3]), gamma);
				ImGui::InvisibleButton("##Zone", ImVec2(width, height), 0);

				ImGui::Separator();
				ImGui::Text("Custom Color Band");
				static int frequency = 6;
				ImGui::SliderInt("Frequency", &frequency, 1, 32);
				static float alpha = 1.0f;
				ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f);
				// DrawColorBandEx(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, FuncType func, int division, float _alpha, float gamma, float offset)
				float const fFrequency = frequency;
				float const fAlpha = alpha;
				DrawColorBandEx< true >(ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2(width, height),
					[fFrequency, fAlpha](float const t)
					{
						float r = ImSign(ImSin(fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 0.0f / fFrequency)) * 0.5f + 0.5f;
						float g = ImSign(ImSin(fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 2.0f / fFrequency)) * 0.5f + 0.5f;
						float b = ImSign(ImSin(fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 4.0f / fFrequency)) * 0.5f + 0.5f;

						return IM_COL32(r * 255, g * 255, b * 255, fAlpha * 255);
					},
					division, gamma, 0.0f);
				ImGui::InvisibleButton("##Zone", ImVec2(width, height), 0);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Color Ring"))
			{
				static int division = 16;
				ImGui::SliderInt("Division", &division, 3, 128);
				static float colorOffset = 16;
				ImGui::SliderFloat("Color Offset", &colorOffset, 0.0f, 2.0f);
				static float thickness = 0.5f;
				ImGui::SliderFloat("Thickness", &thickness, 1.0f / width, 1.0f);

				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				{
					float const width = ImGui::GetContentRegionAvailWidth();
					ImVec2 curPos = ImGui::GetCursorScreenPos();
					ImGui::InvisibleButton("##Zone", ImVec2(width, width), 0);

					DrawColorRingEx< true >(pDrawList, curPos, ImVec2(width, width), thickness,
						[](float t)
						{
							float r, g, b;
							ImGui::ColorConvertHSVtoRGB(t, 1.0f, 1.0f, r, g, b);

							return IM_COL32(r * 255, g * 255, b * 255, 255);
						}, division, colorOffset);
				}
				static float center = 0.5f;
				ImGui::DragFloat("Center", &center, 0.01f, 0.0f, 1.0f);
				static float colorDotBound = 0.5f;
				ImGui::SliderFloat("Alpha Pow", &colorDotBound, -1.0f, 1.0f);
				static int frequency = 6;
				ImGui::SliderInt("Frequency", &frequency, 1, 32);
				{
					ImGui::Text("Nearest");
					float const width = ImGui::GetContentRegionAvailWidth();
					ImVec2 curPos = ImGui::GetCursorScreenPos();
					ImGui::InvisibleButton("##Zone", ImVec2(width, width) * 0.5f, 0);

					float fCenter = center;
					float fColorDotBound = colorDotBound;
					DrawColorRingEx< false >(pDrawList, curPos, ImVec2(width, width * 0.5f), thickness,
						[fCenter, fColorDotBound](float t)
						{
							float r, g, b;
							ImGui::ColorConvertHSVtoRGB(t, 1.0f, 1.0f, r, g, b);

							ImVec2 const v0(ImCos(t * 2.0f * IM_PI), ImSin(t * 2.0f * IM_PI));
							ImVec2 const v1(ImCos(fCenter * 2.0f * IM_PI), ImSin(fCenter * 2.0f * IM_PI));

							float const dot = ImWidgets::ImDot(v0, v1);
							float const angle = ImAcos(dot) / IM_PI;// / width;

							return IM_COL32(r * 255, g * 255, b * 255, (dot > fColorDotBound ? 1.0f : 0.0f) * 255);
						}, division, colorOffset);
				}
				{
					ImGui::Text("Custom");
					float const width = ImGui::GetContentRegionAvailWidth();
					ImVec2 curPos = ImGui::GetCursorScreenPos();
					ImGui::InvisibleButton("##Zone", ImVec2(width, width) * 0.5f, 0);

					float const fFreq = (float)frequency;
					DrawColorRingEx< true >(pDrawList, curPos, ImVec2(width, width) * 0.5f, thickness,
						[fFreq](float t)
						{
							float v = ImSign(ImCos(fFreq * 2.0f * IM_PI * t)) * 0.5f + 0.5f;

							return IM_COL32(v * 255, v * 255, v * 255, 255);
						}, division, colorOffset);
				}

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Color2D"))
			{
				float const width = ImGui::GetContentRegionAvailWidth();
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();

				float const fTime = static_cast<float>(ImGui::GetTime());

				static int resX = 124;
				static int resY = 124;
				static bool isBilinear = true;
				ImGui::SliderInt("ResX", &resX, 4, 512);
				ImGui::SliderInt("ResY", &resY, 4, 512);
				ImGui::Checkbox("Is Bilinear", &isBilinear);
				if (isBilinear)
				{
					DrawColorDensityPlotEx< true >(pDrawList,
						[fTime](float const x, float const y)
						{
							return sdHorseshoeColor(ImVec2(x, y), fTime);
						}, -1.0f, 1.0f, -1.0f, 1.0f, ImGui::GetCursorScreenPos(), ImVec2(width, width), resX, resY);
				}
				else
				{
					DrawColorDensityPlotEx< false >(pDrawList,
						[fTime](float const x, float const y)
						{
							return sdHorseshoeColor(ImVec2(x, y), fTime);
						}, -1.0f, 1.0f, -1.0f, 1.0f, ImGui::GetCursorScreenPos(), ImVec2(width, width), resX, resY);
				}
				ImGui::Dummy(ImVec2(width, width));

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Convex Mask"))
			{
				float const width = ImGui::GetWindowContentRegionWidth();
				ImVec2 const curPos = ImGui::GetCursorScreenPos();
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();

				DrawColorDensityPlotEx< true >(pDrawList,
					[](float const x, float const y)
					{
						float z = ImSaturate(std::sin(x) * std::sin(y) * 0.5f + 0.5f);

						return IM_COL32(255 * z, 255 * z, 255 * z, 255);
					}, -4.0f, 4.0f, -4.0f, 4.0f, curPos, ImVec2(width, width), 32, 32);

				ImWidgets::DrawConvexMaskMesh(pDrawList, curPos, &maskShape_values[0], maskShape_values.size() / 2, ImVec2(width, width));

				MoveLine2D("Shape", &maskShape_values[0], maskShape_values.size() / 2, -1.0f, 1.0f, -1.0f, 1.0f, true);

				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Widgets"))
		{
			if (ImGui::TreeNode("DragLengthScalar"))
			{
				static const float fZero = 0.0f;
				static float length = 16.0f;
				static ImWidgetsLengthUnit currentUnit = ImWidgetsLengthUnit_Metric;
				DragLengthScalar("DragLengthScalar", ImGuiDataType_Float, &length, &currentUnit, 1.0f, &fZero, nullptr, ImGuiSliderFlags_None);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Density Plot Nearest"))
			{
				DensityPlotNearest("Dense SS N", [](float x, float y) -> float { return std::sin(x) * std::sin(y); }, 32, 32, -4.0f, 4.0f, -3.0f, 3.0f);
				DensityPlotNearest("Dense S N", [](float x, float y) -> float { return std::sin(x * y); }, 32, 32, 0.0f, 4.0f, 0.0f, 4.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Density Plot Bilinear"))
			{
				DensityPlotBilinear("Dense SS B", [](float x, float y) -> float { return std::sin(x) * std::sin(y); }, 32, 32, -4.0f, 4.0f, -3.0f, 3.0f);
				DensityPlotBilinear("Dense S B", [](float x, float y) -> float { return std::sin(x * y); }, 32, 32, 0.0f, 4.0f, 0.0f, 4.0f);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Isoline"))
			{
				static int resolution = 128;
				static float isoLines[] = { 0.0f, 0.5f, 0.95f, 1.0f };
				ImGui::SliderInt("Resolution", &resolution, 4, 512);
				static bool bShowSurface = true;
				ImGui::Checkbox("Show Surface", &bShowSurface);
				ImGui::Text("Isoline Values");
				float const width = ImGui::GetContentRegionAvailWidth();
				ImGui::PushMultiItemsWidths(4, width);
				ImGui::DragFloat("##IsoLine0", &isoLines[0], 0.001f, -1.0f, 1.0f); ImGui::SameLine();
				ImGui::DragFloat("##IsoLine1", &isoLines[1], 0.001f, -1.0f, 1.0f); ImGui::SameLine();
				ImGui::DragFloat("##IsoLine2", &isoLines[2], 0.001f, -1.0f, 1.0f); ImGui::SameLine();
				ImGui::DragFloat("##IsoLine3", &isoLines[3], 0.001f, -1.0f, 1.0f);
				static ImU32 cols[] = { IM_COL32(255, 0, 0, 255), IM_COL32(255, 255, 0, 255), IM_COL32(255, 0, 255, 255), IM_COL32(0, 255, 0, 255) };
				ImGui::PushMultiItemsWidths(4, width);

				static ImVec4 col;
				col = ImGui::ColorConvertU32ToFloat4(cols[0]);
				if (ImGui::ColorEdit4("##IsoColor0", &col.x))
					cols[0] = ImGui::ColorConvertFloat4ToU32(col);
				ImGui::SameLine();

				col = ImGui::ColorConvertU32ToFloat4(cols[1]);
				if (ImGui::ColorEdit4("##IsoColor1", &col.x))
					cols[1] = ImGui::ColorConvertFloat4ToU32(col);
				ImGui::SameLine();

				col = ImGui::ColorConvertU32ToFloat4(cols[2]);
				if (ImGui::ColorEdit4("##IsoColor2", &col.x))
					cols[2] = ImGui::ColorConvertFloat4ToU32(col);
				ImGui::SameLine();

				col = ImGui::ColorConvertU32ToFloat4(cols[3]);
				if (ImGui::ColorEdit4("##IsoColor3", &col.x))
					cols[3] = ImGui::ColorConvertFloat4ToU32(col);

				DensityIsolinePlotBilinear("IsoLine 0", [](float x, float y) -> float { return 1.01f * std::sin(x) * std::sin(y); }, bShowSurface, &isoLines[0], 4, &cols[0], 4, resolution, resolution, -4.0f, 4.0f, -3.0f, 3.0f);
				DensityIsolinePlotBilinear("IsoLine 1", [](float x, float y) -> float { return 1.01f * std::sin(x * y); }, bShowSurface, &isoLines[0], 4, &cols[0], 4, resolution, resolution, -4.0f, 4.0f, -3.0f, 3.0f);

				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Analytic Plot"))
			{
				float const width = ImGui::GetContentRegionAvailWidth();
				ImGui::Text("ImGui::PlotLines: 128 samples");
				ImGui::Dummy(ImVec2(1.0f, ImGui::GetTextLineHeightWithSpacing()));
				ImGui::PlotLines("##PlotLines", [](void* data, int idx)
					{
						float const x = (((float)idx) / 127.0f) * 8.0f;
						return sin(x * x * x) * sin(x);
					}, nullptr, 128, 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(width, width));
				ImGui::Dummy(ImVec2(1.0f, ImGui::GetTextLineHeightWithSpacing()));

				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 128, 0, 255));
				ImGui::TextWrapped("/!\\ Use carefully and at your risk, this widget can generate very quickly very high number of vertices.\nCondisider the override of ImDrawIdx to ImU32 in your build system.");
				ImGui::PopStyleColor();

				static int initSampleCount = 8;
				ImGui::DragInt("Init Samples Count", &initSampleCount, 1.0f, 4, 16);
				static float minX = 0.0f;
				static float maxX = 8.0f;
				float fMaxX = maxX;
				float fMinX = minX;
				ImGui::DragFloat("Min X", &minX, 0.01f, 0.0f, fMaxX - 0.01f);
				ImGui::DragFloat("Max X", &maxX, 0.01f, fMinX + 0.01f, 16.0f);
				ImGui::Text("DearWidgets:Plot with Dynamic Resampling (Init Samples Count: 8)");
				ImGui::Dummy(ImVec2(1.0f, ImGui::GetTextLineHeightWithSpacing()));
				AnalyticalPlot("Analytical", [](float const x) { return sin(x * x * x) * sin(x); }, minX, maxX, initSampleCount);
				ImGui::Dummy(ImVec2(1.0f, ImGui::GetTextLineHeightWithSpacing()));
				ImGui::Dummy(ImVec2(1.0f, ImGui::GetTextLineHeightWithSpacing()));

				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Draft - Open Ideas mostly WIP"))
		{
			if (ImGui::TreeNode("Draw"))
			{
				if (ImGui::TreeNode("Chromaticity Draw"))
				{
					static int chromLinesampleCount = 16;
					ImGui::SliderInt("Chromatic Sample Count", &chromLinesampleCount, 3, 256);
					static int resX = 16;
					ImGui::SliderInt("Resolution X", &resX, 3, 256);
					static int resY = 16;
					ImGui::SliderInt("Resolution Y", &resY, 3, 256);
					static int waveMin = 400;
					static int waveMax = 700;
					ImGui::SliderInt("Wavelength Min", &waveMin, 300, waveMax);
					ImGui::SliderInt("Wavelength Max", &waveMax, waveMin, 800);
					char const* observer[] = { "1931 2 deg", "1964 10 deg" };
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
					ImDrawList* pDrawList = ImGui::GetWindowDrawList();
					float const width = ImGui::GetContentRegionAvailWidth();
					ImVec2 curPos = ImGui::GetCursorScreenPos();
					ImGui::InvisibleButton("##Zone", ImVec2(width, width), 0);
					//ImGui::DragInt("Triangle", &triangleDrawCur, 1.0f, -1, 1024);
					DrawChromaticPlotBilinear(
						pDrawList,
						curPos,
						width, width,
						chromLinesampleCount,
						curColorSpace,
						curObserver,
						curIllum,
						resX, resY,
						waveMin, waveMax,
						0.0f, 0.8f,
						0.0f, 0.9f);

					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Widgets"))
			{
				if (ImGui::TreeNode("Slider 2D Float"))
				{
					static ImVec2 slider2D;
					ImVec2 boundMin(-1.0f, -1.0f);
					ImVec2 boundMax(1.0f, 1.0f);
					Slider2DFloat("Slider 2D Float", &slider2D.x, &slider2D.y, boundMin.x, boundMax.x, boundMin.y, boundMax.y, 0.75f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Slider 2D Int"))
				{
					static int curX = 0;
					static int curY = 0;
					int minX = -5;
					int minY = -5;
					int maxX = 5;
					int maxY = 5;
					Slider2DInt("Slider 2D Int", &curX, &curY, &minX, &maxX, &minY, &maxY, 0.75f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Slider 3D"))
				{
					static ImVec4 cur3D;
					ImVec4 boundMin(-1.0f, -1.0f, -1.0f, 0.0f);
					ImVec4 boundMax(1.0f, 1.0f, 1.0f, 0.0f);
					SliderScalar3D("Slider 3D Float", &cur3D.x, &cur3D.y, &cur3D.z,
						boundMin.x, boundMax.x,
						boundMin.y, boundMax.y,
						boundMin.z, boundMax.z,
						0.75f);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Grid"))
				{
					ImVec2 boundMin(-1.0f, -1.0f);
					ImVec2 boundMax(1.0f, 1.0f);
					Grid2D_AoS_Float("Slider 3D Float", &grid_values[0], grid_rows, grid_columns, boundMin.x, boundMax.x, boundMin.y, boundMax.y);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("2D Move"))
				{
					static bool closeLoop = true;
					ImGui::Checkbox("Closed Loop", &closeLoop);
					MoveLine2D("Region", &linear_values[0], linear_values.size() / 2, -1.0f, 1.0f, -1.0f, 1.0f, closeLoop);
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Line Slider"))
				{
					float const width = ImGui::GetContentRegionAvailWidth();
					ImVec2 curPos = ImGui::GetCursorScreenPos();
					ImVec2 center = curPos + ImVec2(width, width) * 0.5f;
					ImGui::Dummy(ImVec2(width, width));
					static float fZero = 0.0f;
					static float fOne = 1.0f;
					static float fValue = 0.5f;
					for (int i = 0; i < 6; ++i)
					{
						float const fI = (float)i;
						float const cos0 = ImCos(fI * 2.0f * IM_PI / 6.0f);
						float const sin0 = ImSin(fI * 2.0f * IM_PI / 6.0f);

						ImVec2 dir = ImVec2(cos0, sin0);

						ImGui::PushID(i);
						ImWidgets::LineSlider("##LineSliderValue",
							center + dir * ImVec2(32.0f, 32.0f),
							center + dir * width * 0.5f, ImGuiDataType_Float, &fValue, &fZero, &fOne, ImWidgetsPointer_Up);
						ImGui::PopID();
					}
					ImGui::SliderFloat("##LineSliderSlodersdgf", &fValue, fZero, fOne);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		ImWidgets::BeginGroupPanel("Color Selector");
		{
			float const width = ImGui::GetContentRegionAvailWidth();
			float const height = 32.0f;
			static float offset = 0.0f;

			static float alphaHue = 1.0f;
			static float alphaHideHue = 0.75f;
			ImGui::DragFloat("Offset##ColorSelector", &offset, 0.0f, 0.0f, 1.0f);
			ImGui::DragFloat("Alpha Hue", &alphaHue, 0.0f, 0.0f, 1.0f);
			ImGui::DragFloat("Alpha Hue Hide", &alphaHideHue, 0.0f, 0.0f, 1.0f);
			static float hueCenter = 0.5f;
			static float hueWidth = 0.1f;
			static float featherLeft = 0.5f;
			static float featherRight = 0.5f;
			ImGui::DragFloat("featherLeft", &featherLeft, 0.0f, 0.0f, 0.5f);
			ImGui::DragFloat("featherRight", &featherRight, 0.0f, 0.0f, 0.5f);
			HueSelector("Hue Selector", ImVec2(width, height), &hueCenter, &hueWidth, &featherLeft, &featherRight, 64, alphaHue, alphaHideHue, offset);
		}
		ImWidgets::EndGroupPanel();

		ImGui::End();
	}
}
