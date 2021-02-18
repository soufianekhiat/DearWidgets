#include <demo.h>
#include <dear_widgets.h>

#include <vector>

static int grid_rows = 8;
static int grid_columns = 8;
static std::vector<float> grid_values;

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
		}
	}
};

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
		DragLengthScalar("DragLengthScalar", ImGuiDataType_Float, &length, &currentUnit, 1.0f, &fZero, nullptr, ImGuiSliderFlags_None);

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

		ImGui::Separator();
		ImGui::Text("External");
		ImWidgets::BeginGroupPanel( "GroupPanel" );
			ImGui::Text("First");
			ImGui::Text("Second");
		ImWidgets::EndGroupPanel();

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}
}
