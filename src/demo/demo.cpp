#include <demo.h>
#include <dear_widgets.h>
#include <imgui_internal.h>

#if defined(__DEAR_GFX_OGL3__)
#define IM_GLFW3_AUTO_LINK
#define IM_CURRENT_TARGET IM_TARGET_GLFW_OPENGL3
#elif defined(__DEAR_GFX_DX9__)
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX9
#elif defined(__DEAR_GFX_DX10__)
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX10
#elif defined(__DEAR_GFX_DX11__)
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX11
#elif defined(__DEAR_GFX_DX12__)
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX12
#endif
#define IM_PLATFORM_IMPLEMENTATION
#include <ImPlatform.h>

#include <vector>
#include <random>

static int grid_rows = 8;
static int grid_columns = 8;
static ImVector<float> grid_values;

static ImVector<float> linear_values;
static ImVector<float> maskShape_values;

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
			float const angle = -2.0f * IM_PI * ((float)i)/((float)(ptsCount - 1));

			float x = radius * ImCos(angle);
			float y = radius * ImSin(angle);

			maskShape_values.push_back(x);
			maskShape_values.push_back(y);
		}
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
		xc = -0.2661239 * _10_9 - 0.2343589 * _10_6 + 0.8776956 * _10_3 + 0.179910;
	else //if (x = 25000.0f)
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
	col = col * (0.8f + 0.2f * ImCos(120.0f * ImAbs(d)));
	col = ImLerp(col, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f - ImWidgets::ImSmoothStep(0.0f, 0.02f, ImAbs(d)));

	return IM_COL32( 255 * col.x, 255 * col.y, 255 * col.z, 255);
}
#pragma endregion ShaderToyHelper

int main()
{
	if ( !ImPlatform::ImSimpleStart( "Dear Widgets Demo", ImVec2( 0.0f, 0.0f ), 1024 * 2, 764 * 2) )
		return 1;

	// Setup Dear ImGui context
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
	////io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		// Enable Multi-Viewport / Platform Windows
	////io.ConfigViewportsNoAutoMerge = true;
	////io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	io.Fonts->AddFontFromFileTTF( "../extern/FiraCode/distr/ttf/FiraCode-Medium.ttf", 16.0f );

	io.FontGlobalScale = 3.0f;
	ImGui::GetStyle().ScaleAllSizes( 3.0f );

	if ( !ImPlatform::ImSimpleInitialize( false ) )
	{
		return 0;
	}

	//ImGui::GetStyle().ScaleAllSizes();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	//ImGuiStyle& style = ImGui::GetStyle();
	//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//{
	//	style.WindowRounding = 0.0f;
	//	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	//}

	ImVec4 clear_color = ImVec4( 0.461f, 0.461f, 0.461f, 1.0f );
	while ( ImPlatform::ImPlatformContinue() )
	{
		bool quit = ImPlatform::ImPlatformEvents();
		if ( quit )
			break;

		if ( !ImPlatform::ImGfxCheck() )
		{
			continue;
		}

		ImPlatform::ImSimpleBegin();

		ImWidgets::ShowDemo();

		ImPlatform::ImSimpleEnd( clear_color, false );
	}

	ImPlatform::ImSimpleFinish();

	return 0;
}

namespace ImWidgets {
	void	ShowDemo()
	{
		static StaticInit s_StaticInit;
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Dear Widgets");

		if (ImGui::TreeNode("Draw"))
		{
			if ( ImGui::TreeNode( "Triangles Pointers" ) )
			{
				float const width = ImGui::GetContentRegionAvail().x;

				static float angle = 0.0f;
				static float size = 16.0f;
				static float thickness = 1.0f;
				ImGui::SliderAngle( "Angle##Triangle", &angle );
				ImGui::SliderFloat( "Size##Triangle", &size, 1.0f, 64.0f );
				ImGui::SliderFloat( "Thickness##Triangle", &thickness, 1.0f, 5.0f );

				ImVec2 curPos = ImGui::GetCursorScreenPos();
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				ImGui::InvisibleButton( "##Zone", ImVec2( width, 96.0f ), 0 );
				ImGui::InvisibleButton( "##Zone", ImVec2( width, 96.0f ), 0 );
				float fPointerLine = 64.0f;
				pDrawList->AddLine( ImVec2( curPos.x + 0.5f * 32.0f, curPos.y + fPointerLine ), ImVec2( curPos.x + 3.5f * 32.0f, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				pDrawList->AddLine( ImVec2( curPos.x + 5.0f * 32.0f, curPos.y ), ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + 72.0f ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				pDrawList->AddLine( ImVec2( curPos.x + 7.0f * 32.0f, curPos.y ), ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + 72.0f ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 1.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 3.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawTrianglePointer( pDrawList, ImVec2( curPos.x + 1.0f * 32.0f, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTrianglePointer( pDrawList, ImVec2( curPos.x + 3.0f * 32.0f, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTrianglePointer( pDrawList, ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTrianglePointer( pDrawList, ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );

				fPointerLine *= 3.0f;
				pDrawList->AddLine( ImVec2( curPos.x + 0.5f * 32.0f, curPos.y + fPointerLine ), ImVec2( curPos.x + 3.5f * 32.0f, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				pDrawList->AddLine( ImVec2( curPos.x + 5.0f * 32.0f, curPos.y ), ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				pDrawList->AddLine( ImVec2( curPos.x + 7.0f * 32.0f, curPos.y ), ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 1.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 3.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawTrianglePointerFilled( pDrawList, ImVec2( curPos.x + 1.0f * 32.0f, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTrianglePointerFilled( pDrawList, ImVec2( curPos.x + 3.0f * 32.0f, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTrianglePointerFilled( pDrawList, ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTrianglePointerFilled( pDrawList, ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImGui::TreePop();
			}
			if ( ImGui::TreeNode( "Color Bands" ) )
			{
				static float col[ 4 ] = { 1, 0, 0, 1 };
				ImGui::ColorEdit4( "Color##ColorBand", col );
				float const width = ImGui::GetContentRegionAvail().x;
				static float height = 32.0f;
				static float gamma = 1.0f;
				ImGui::DragFloat( "Height##ColorBand", &height, 1.0f, 1.0f, 128.0f );
				ImGui::DragFloat( "Gamma##ColorBand", &gamma, 0.01f, 0.1f, 10.0f );
				static int division = 32;
				ImGui::DragInt( "Division##ColorBand", &division, 1, 1, 128 );

				ImGui::Text( "HueBand" );
				DrawHueBand( ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2( width, height ), division, col, col[ 3 ], gamma );
				ImGui::InvisibleButton( "Hue##ColorBand", ImVec2( width, height ), 0 );

				ImGui::Text( "LuminanceBand" );
				DrawLumianceBand( ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2( width, height ), division, ImVec4( col[ 0 ], col[ 1 ], col[ 2 ], col[ 3 ] ), gamma );
				ImGui::InvisibleButton( "Luminance##ColorBand", ImVec2( width, height ), 0 );

				ImGui::Text( "SaturationBand" );
				DrawSaturationBand( ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2( width, height ), division, ImVec4( col[ 0 ], col[ 1 ], col[ 2 ], col[ 3 ] ), gamma );
				ImGui::InvisibleButton( "Saturation##ColorBand", ImVec2( width, height ), 0 );

				ImGui::Separator();
				ImGui::Text( "Custom Color Band" );
				static int frequency = 6;
				ImGui::SliderInt( "Frequency##ColorBand", &frequency, 1, 32 );
				static float alpha = 1.0f;
				ImGui::SliderFloat( "alpha##ColorBand", &alpha, 0.0f, 1.0f );
				float data[] = { ( float )frequency, alpha };
				DrawProceduralColor1DBilinear(
					ImGui::GetWindowDrawList(),
					[]( float t, void* pUserData ) -> ImU32
					{
						float fFrequency = ( ( float* )pUserData )[ 0 ];
						float fAlpha = ( ( float* )pUserData )[ 1 ];
						float r = ImSign( ImSin( fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 0.0f / fFrequency ) ) * 0.5f + 0.5f;
						float g = ImSign( ImSin( fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 2.0f / fFrequency ) ) * 0.5f + 0.5f;
						float b = ImSign( ImSin( fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 4.0f / fFrequency ) ) * 0.5f + 0.5f;
											
						return IM_COL32( r * 255, g * 255, b * 255, fAlpha * 255 );
					},
					&data[ 0 ],
					0.0f, 1.0f, ImGui::GetCursorScreenPos(), ImVec2( width, height ), division );
				ImGui::InvisibleButton( "##Zone", ImVec2( width, height ), 0 );
				ImGui::TreePop();
			}
			if ( ImGui::TreeNode( "Color Ring" ) )
			{
				ImGui::TreePop();
			}
			if ( ImGui::TreeNode( "Color2D" ) )
			{
				float const width = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();

				float const fTime = static_cast< float >( ImGui::GetTime() );

				static int resX = 124;
				static int resY = 124;
				static bool isBilinear = true;
				static bool pause = false;
				ImGui::SliderInt( "ResX", &resX, 4, 512 );
				ImGui::SliderInt( "ResY", &resY, 4, 512 );
				ImGui::Checkbox( "Is Bilinear", &isBilinear );
				if ( ImGui::Button( "Pause" ) )
				{
					pause = !pause;
				}
				static float usedTime = 0.0f;
				if ( !pause )
				{
					usedTime = fTime;
				}
				float timeCopy = usedTime;
				ImColor2DCallback func = []( float x, float y, void* pUserData ) -> ImU32
					{
						float timeCopy = *( ( float* )pUserData );
						return sdHorseshoeColor( ImVec2( x, y ), timeCopy );
					};
				if ( isBilinear )
				{
					DrawProceduralColor2DBilinear( pDrawList,
													func, &timeCopy, -1.0f, 1.0f, -1.0f, 1.0f, ImGui::GetCursorScreenPos(), ImVec2( width, width ), resX, resY );
				}
				else
				{
					DrawProceduralColor2DNearest( pDrawList,
													 func, &timeCopy, -1.0f, 1.0f, -1.0f, 1.0f, ImGui::GetCursorScreenPos(), ImVec2( width, width ), resX, resY );
				}
				ImGui::Dummy( ImVec2( width, width ) );

				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Widgets"))
		{
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Alpha - Draft - Open Ideas mostly WIP"))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 128, 0, 255));
			ImGui::TextWrapped("/!\\ Use carefully and at your risk!");
			ImGui::TextWrapped("/!\\ API will change, that at 'first draft' stage.");
			ImGui::TextWrapped("/!\\ PR are welcome to contribute.");
			ImGui::PopStyleColor();

			if (ImGui::TreeNode("Draw"))
			{
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Widgets"))
			{
				if (ImGui::TreeNode("DragLengthScalar"))
				{
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		ImGui::End();

		bool show_app_metrics = true;
		ImGui::ShowMetricsWindow(&show_app_metrics);
	}
}
