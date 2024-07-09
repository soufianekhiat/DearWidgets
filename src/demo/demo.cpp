#include <demo.h>
#include <dear_widgets.h>
#include <imgui_internal.h>

#if defined(__DEAR_GFX_OGL3__)
#define IM_GLFW3_AUTO_LINK
#define IM_CURRENT_TARGET IM_TARGET_WIN32_OPENGL3
#elif defined(__DEAR_GFX_DX9__)
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX9
#elif defined(__DEAR_GFX_DX10__)
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX10
#elif defined(__DEAR_GFX_DX11__)
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX11
#elif defined(__DEAR_GFX_DX12__)
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX12
#endif
//#define IM_PLATFORM_ENABLE_CUSTOM_SHADER
#define IM_PLATFORM_IMPLEMENTATION
#include <ImPlatform.h>

#include <vector>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <IconFontCppHeaders/IconsFontAwesome6.h>
#include <IconFontCppHeaders/IconsFontAwesome6Brands.h>

//static int grid_rows = 8;
//static int grid_columns = 8;
//static ImVector<float> grid_values;
//
//static ImVector<float> linear_values;
//static ImVector<float> maskShape_values;
//
//std::random_device rd;
//std::mt19937_64 gen( rd() );
//std::uniform_real_distribution<float> dis( -1.0f, 1.0f );
//
//class StaticInit
//{
//public:
//	StaticInit()
//	{
//		for ( int j = 0; j < grid_rows; ++j )
//		{
//			for ( int i = 0; i < grid_columns; ++i )
//			{
//				float x = ( ( float )i ) / ( ( float )( grid_columns - 1 ) );
//				float y = ( ( float )j ) / ( ( float )( grid_rows - 1 ) );
//
//				grid_values.push_back( x );
//				grid_values.push_back( y );
//			}
//
//			linear_values.push_back( dis( gen ) * 0.5f );
//			linear_values.push_back( dis( gen ) );
//		}
//
//		constexpr int ptsCount = 16;
//		float const radius = 0.5f;
//		for ( int i = 0; i < ptsCount; ++i )
//		{
//			float const angle = -2.0f * IM_PI * ( ( float )i ) / ( ( float )( ptsCount - 1 ) );
//
//			float x = radius * ImCos( angle );
//			float y = radius * ImSin( angle );
//
//			maskShape_values.push_back( x );
//			maskShape_values.push_back( y );
//		}
//	}
//};

ImVec2 TemperatureTo_xy( float TT )
{
	float T = TT;
	float xc, yc;
	//float const invT = 1.0f / T;
	//float const invT2 = 1.0f / ( T * T );
	//float const invT3 = ( 1.0f / ( T * T ) ) / T;
	float const _10_9 = 1e9f / ( T * T * T );
	float const _10_6 = 1e6f / ( T * T );
	float const _10_3 = 1e3f / ( T );
	if (/*T >= 1667.0f &&*/ T <= 4000.0f )
		xc = -0.2661239f * _10_9 - 0.2343589f * _10_6 + 0.8776956f * _10_3 + 0.179910f;
	else //if (x = 25000.0f)
		xc = -3.0258469f * _10_9 + 2.1070379f * _10_6 + 0.2226347f * _10_3 + 0.240390f;

	float const xc2 = xc * xc;
	float const xc3 = xc2 * xc;

	if (/*T >= 1667.0f &&*/ T <= 2222.0f )
		yc = -1.1063814f * xc3 - 1.34811020f * xc2 + 2.18555832f * xc - 0.20219683f;
	else if ( T < 4000.0f )
		yc = -0.9549476f * xc3 - 1.37418593f * xc2 + 2.09137015f * xc - 0.16748867f;
	else //if (T <= 25000.0f)
		yc = +3.0817580f * xc3 - 5.87338670f * xc2 + 3.75112997f * xc - 0.37001483f;

	return ImVec2( xc, yc );
}

static inline ImVec4 operator*( const ImVec4& lhs, const float rhs )
{
	return ImVec4( lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs );
}

#pragma region ShaderToyHelper
// Ref: https://www.shadertoy.com/view/WlSGW1
float sdHorseshoe( ImVec2 p, ImVec2 c, float r, ImVec2 w )
{
	p.x = ImAbs( p.x );
	float l = ImWidgets::ImLength( p );
	p = ImVec2( -c.x * p.x + p.y * c.y, c.y * p.x + p.y * c.x );
	p = ImVec2( ( p.y > 0.0f ) ? p.x : l * ImSign( -c.x ), ( p.x > 0.0f ) ? p.y : l );
	p = ImVec2( p.x, ImAbs( p.y - r ) ) - w;
	return ImWidgets::ImLength( ImMax( p, ImVec2( 0.0f, 0.0f ) ) ) + ImMin( 0.0f, ImMax( p.x, p.y ) );
}

ImU32 sdHorseshoeColor( ImVec2 p, float fTime )
{
	float t = IM_PI * ( 0.3f + 0.3f * ImCos( fTime * 0.5f ) );
	ImVec2 tmp = ImVec2( 0.7f, 1.1f ) * fTime + ImVec2( 0.0f, 2.0f );
	ImVec2 w = ImVec2( 0.750f, 0.25f ) * ( ImVec2( 0.5f, 0.5f ) + ImVec2( ImCos( tmp.x ), ImCos( tmp.y ) ) * 0.5f );

	// distance
	float d = sdHorseshoe( p - ImVec2( 0.0f, -0.1f ), ImVec2( ImCos( t ), ImSin( t ) ), 0.5f, w );

	// coloring
	ImVec4 col = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) - ImVec4( 0.1f, 0.4f, 0.7f, 1.0f ) * ImSign( d );
	col = col * ( 1.0f - exp( -2.0f * ImAbs( d ) ) );
	col = col * ( 0.8f + 0.2f * ImCos( 120.0f * ImAbs( d ) ) );
	col = ImLerp( col, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ), 1.0f - ImWidgets::ImSmoothStep( 0.0f, 0.02f, ImAbs( d ) ) );

	return IM_COL32( 255 * col.x, 255 * col.y, 255 * col.z, 255 );
}
#pragma endregion ShaderToyHelper

void ShowSampleOffscreen00();

ImTextureID background;
ImVec2 background_size;
ImTextureID illlustration_img;
ImVec2 illlustration_size;
int main()
{
	if ( !ImPlatform::ImSimpleStart( "Dear Widgets Demo", ImVec2( 0.0f, 0.0f ), 1024 * 2, 764 * 2 ) )
		return 1;

	// Setup Dear ImGui context
	ImGuiIO& io = ImGui::GetIO(); ( void )io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
	////io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// Enable Gamepad Controls
#ifdef IMGUI_HAS_DOCK
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable Docking
#endif
#ifdef IMGUI_HAS_VIEWPORT
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;		// Enable Multi-Viewport / Platform Windows
#endif
	////io.ConfigViewportsNoAutoMerge = true;
	////io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	io.Fonts->AddFontFromFileTTF( "../extern/FiraCode/distr/ttf/FiraCode-Medium.ttf", 16.0f );

	//io.FontGlobalScale = 3.0f;
	//ImGui::GetStyle().ScaleAllSizes( 3.0f );

	if ( !ImPlatform::ImSimpleInitialize( false ) )
	{
		return 0;
	}

	//ImGui::GetStyle().ScaleAllSizes();

#ifdef IMGUI_HAS_VIEWPORT
	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//{
	//	style.WindowRounding = 0.0f;
	//	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	//}
#endif

	int width;
	int height;
	// Image from: https://www.pexels.com/fr-fr/photo/framboises-mures-dans-une-tasse-de-the-blanche-en-photographie-a-decalage-d-inclinaison-1152351/
	stbi_uc* data = stbi_load( "pexels-robert-bogdan-156165-1152351.jpg", &width, &height, NULL, 4 );
	illlustration_img = ImPlatform::ImCreateTexture2D( ( char* )data, width, height,
												{
													ImPlatform::IM_RGBA,
													ImPlatform::IM_TYPE_UINT8,
													ImPlatform::IM_FILTERING_LINEAR,
													ImPlatform::IM_BOUNDARY_CLAMP,
													ImPlatform::IM_BOUNDARY_CLAMP
												} );
	illlustration_size = ImVec2( ( float )width, ( float )height );
	STBI_FREE( data );
	// Image from: https://www.pexels.com/fr-fr/photo/deux-chaises-avec-table-en-verre-sur-le-salon-pres-de-la-fenetre-1571453/
	data = stbi_load( "pexels-fotoaibe-1571453.jpg", &width, &height, NULL, 4 );
	background = ImPlatform::ImCreateTexture2D( ( char* )data, width, height,
												{
													ImPlatform::IM_RGBA,
													ImPlatform::IM_TYPE_UINT8,
													ImPlatform::IM_FILTERING_LINEAR,
													ImPlatform::IM_BOUNDARY_CLAMP,
													ImPlatform::IM_BOUNDARY_CLAMP
												} );
	background_size = ImVec2( ( float )width, ( float )height );
	STBI_FREE( data );

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

		ShowSampleOffscreen00();

		ImPlatform::ImSimpleEnd( clear_color, false );
	}

	ImPlatform::ImSimpleFinish();
	ImPlatform::ImReleaseTexture2D( background );

	return 0;
}

void ShowSampleOffscreen00()
{
	ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 8 );

	ImGui::Begin( "Off Screen 00" );

	ImDrawList* draw = ImGui::GetBackgroundDrawList();

	ImVec2 cur = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();

	draw->AddImageRounded( illlustration_img,
						   cur + ImVec2( 0.0f - 50.0f, 0.0f ),
						   cur + ImVec2( 0.0f + 50.0f, 50.0f ),
						   ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255), 8);

	ImGui::End();

	ImGui::PopStyleVar( 1 );
}

namespace ImWidgets{
	void	ShowDemo()
	{
		//static StaticInit s_StaticInit;
		static float f = 0.0f;
		static int counter = 0;

		ImGui::SetNextWindowBgAlpha( 0.75f );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 16 );
		ImGui::Begin( "Dear Widgets", NULL, ImGuiWindowFlags_NoTitleBar );
		ImWidgets::SetCurrentWindowBackgroundImage( background, background_size, false );

		if ( ImGui::CollapsingHeader( "Draw" ) )
		{
			ImGui::Indent();
			if ( ImGui::CollapsingHeader( "Draw Shape" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static float edge_thickness = 2.0f;
				static float vertex_radius = 16.0f;
				static ImVec4 edge_col_v( 1.0f, 0.0f, 0.0f, 1.0f );
				static ImVec4 triangle_col_v( 0.0f, 1.0f, 0.0f, 1.0f );
				static ImVec4 vertex_col_v( 0.0f, 0.0f, 1.0f, 1.0f );
				static ImU32 edge_col = ImGui::GetColorU32( edge_col_v );
				static ImU32 triangle_col = ImGui::GetColorU32( triangle_col_v );
				static ImU32 vertex_col = ImGui::GetColorU32( vertex_col_v );
				static ImVec2 uv_start( 0.0f, 0.5f );
				static ImVec2 uv_end( 1.0f, 0.5f );
				static ImVec4 cola_v( 1.0f, 0.0f, 0.0f, 1.0f );
				static ImVec4 colb_v( 0.0f, 1.0f, 0.0f, 1.0f );
				static ImU32 cola = ImGui::GetColorU32( cola_v );
				static ImU32 colb = ImGui::GetColorU32( colb_v );
				static int tri_idx = -1;
				static int side_count = 3;
#ifdef DEAR_WIDGETS_TESSELATION
				static int tess = 1;
				ImGui::SliderInt( "Tess", &tess, 0, 16 );
#endif
				ImGui::SliderInt( "Sides", &side_count, 3, 64 );
				ImGui::SliderFloat( "Thickness", &edge_thickness, 0.0f, 16.0f );
				ImGui::SliderFloat( "Radius", &vertex_radius, 0.0f, 64.0f );
				if ( ImGui::ColorEdit4( "Edge##DrawShape", &edge_col_v.x ) )
					edge_col = ImGui::GetColorU32( edge_col_v );
				if ( ImGui::ColorEdit4( "Triangle##DrawShape", &triangle_col_v.x ) )
					triangle_col = ImGui::GetColorU32( triangle_col_v );
				if ( ImGui::ColorEdit4( "Vertices##DrawShape", &vertex_col_v.x ) )
					vertex_col = ImGui::GetColorU32( vertex_col_v );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				Slider2DFloat( "uv0", &uv_start.x, &uv_start.y, 0.0f, 1.0f, 0.0f, 1.0f );
				ImGui::SameLine();
				Slider2DFloat( "uv1", &uv_end.x, &uv_end.y, 0.0f, 1.0f, 0.0f, 1.0f );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &cola_v.x ) )
					cola = ImGui::GetColorU32( cola_v );
				ImGui::SameLine();
				if ( ImGui::ColorEdit4( "ColB##DrawShape", &colb_v.x ) )
					colb = ImGui::GetColorU32( colb_v );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				static ImShape shape;
				GenShapeCircle( shape, pos + ImVec2( size * 0.5f, size * 0.5f ), size * 0.5f, side_count );
				ShapeSetDefaultUV( shape );
#ifdef DEAR_WIDGETS_TESSELATION
				for ( int k = 0; k < tess; ++k )
					ShapeTesselationUniform( shape );
#endif
				ShapeSRGBLinearGradient( shape,
										 uv_start, uv_end,
										 cola, colb );
				DrawShapeDebug( pDrawList, shape, edge_thickness, edge_col, triangle_col, vertex_radius, vertex_col, tri_idx );
				ImGui::Dummy( ImVec2( size, size ) );
				ImGui::SliderInt( "tri_idx", &tri_idx, -1, shape.triangles.size() - 1 );
				ImGui::Text( "Tri: %d", shape.triangles.size() );
				ImGui::Text( "Vtx: %d", shape.vertices.size() );
			}
			if ( ImGui::CollapsingHeader( "Linear Gradient" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static ImVec2 uv_start( 0.0f, 0.0f );
				static ImVec2 uv_end( 1.0f, 0.0f );
				static ImVec4 cola_v( 0.0f, 0.0f, 1.0f, 1.0f );
				static ImVec4 colb_v( 1.0f, 0.0f, 0.0f, 1.0f );
				static ImU32 cola = ImGui::GetColorU32( cola_v );
				static ImU32 colb = ImGui::GetColorU32( colb_v );
#ifdef DEAR_WIDGETS_TESSELATION
				static int tess = 5;
				ImGui::SliderInt( "Tess", &tess, 0, 16 );
#endif
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				Slider2DFloat( "uv0", &uv_start.x, &uv_start.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::SameLine();
				Slider2DFloat( "uv1", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &cola_v.x ) )
					cola = ImGui::GetColorU32( cola_v );
				ImGui::SameLine();
				if ( ImGui::ColorEdit4( "ColB##DrawShape", &colb_v.x ) )
					colb = ImGui::GetColorU32( colb_v );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				static ImShape shape;
				float height = size * 0.25f;
				GenShapeRect( shape, ImRect( pos, pos + ImVec2( size, height ) ) );
				ShapeSetDefaultUV( shape );
#ifdef DEAR_WIDGETS_TESSELATION
				for ( int k = 0; k < tess; ++k )
					ShapeTesselationUniform( shape );
#endif
				ShapeSRGBLinearGradient( shape,
									 uv_start, uv_end,
									 cola, colb );
				DrawShape( pDrawList, shape );
				pDrawList->AddText( shape.bb.Min, IM_COL32( 255, 255, 255, 255 ), "sRGB" );

				ShapeTranslate( shape, ImVec2( 0.0f, height ) );
				ShapeHSVLinearGradient( shape,
										uv_start, uv_end,
										cola, colb );
				DrawShape( pDrawList, shape );
				pDrawList->AddText( shape.bb.Min, IM_COL32( 255, 255, 255, 255 ), "HSV" );

				ShapeTranslate( shape, ImVec2( 0.0f, height ) );
				ShapeLinearSRGBLinearGradient( shape,
											   uv_start, uv_end,
											   cola, colb );
				DrawShape( pDrawList, shape );
				pDrawList->AddText( shape.bb.Min, IM_COL32( 255, 255, 255, 255 ), "Linear sRGB" );

				ShapeTranslate( shape, ImVec2( 0.0f, height ) );
				ShapeOkLabLinearGradient( shape,
										  uv_start, uv_end,
										  cola, colb );
				DrawShape( pDrawList, shape );
				pDrawList->AddText( shape.bb.Min, IM_COL32( 255, 255, 255, 255 ), "OkLab" );

				ShapeTranslate( shape, ImVec2( 0.0f, height ) );
				ShapeOkLchLinearGradient( shape,
										  uv_start, uv_end,
										  cola, colb );
				DrawShape( pDrawList, shape );
				pDrawList->AddText( shape.bb.Min, IM_COL32( 255, 255, 255, 255 ), "OkLch" );

				ImGui::Dummy( ImVec2( size, height ) );
				ImGui::Dummy( ImVec2( size, height ) );
				ImGui::Dummy( ImVec2( size, height ) );
				ImGui::Dummy( ImVec2( size, height ) );
				ImGui::Dummy( ImVec2( size, height ) );
				ImGui::Text( "Tri: %d", shape.triangles.size() );
				ImGui::Text( "Vtx: %d", shape.vertices.size() );
			}
			if ( ImGui::CollapsingHeader( "Radial Gradient" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static ImVec2 uv_start( 0.5f, 0.5f );
				static ImVec2 uv_end( 0.95f, 0.5f );
				static ImVec4 cola_v( 1.0f, 0.0f, 0.0f, 1.0f );
				static ImVec4 colb_v( 0.0f, 1.0f, 1.0f, 0.0f );
				static ImU32 cola = ImGui::GetColorU32( cola_v );
				static ImU32 colb = ImGui::GetColorU32( colb_v );
#ifdef DEAR_WIDGETS_TESSELATION
				static int tess = 5;
				ImGui::SliderInt( "Tess", &tess, 0, 16 );
#endif
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				Slider2DFloat( "uv0", &uv_start.x, &uv_start.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::SameLine();
				Slider2DFloat( "uv1", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &cola_v.x ) )
					cola = ImGui::GetColorU32( cola_v );
				ImGui::SameLine();
				if ( ImGui::ColorEdit4( "ColB##DrawShape", &colb_v.x ) )
					colb = ImGui::GetColorU32( colb_v );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				static ImShape shape;
				GenShapeCircle( shape, pos + ImVec2( 0.5f * size, 0.5f * size ), size * 0.5f, 16 );
				ShapeSetDefaultUV( shape );
#ifdef DEAR_WIDGETS_TESSELATION
				for ( int k = 0; k < tess; ++k )
					ShapeTesselationUniform( shape );
#endif
				ShapeOkLabRadialGradient( shape,
										  uv_start, uv_end,
										  cola, colb );
				DrawShape( pDrawList, shape );
				ImGui::Dummy( ImVec2( size, size ) );
				ImGui::Text( "Tri: %d", shape.triangles.size() );
				ImGui::Text( "Vtx: %d", shape.vertices.size() );
			}
			if ( ImGui::CollapsingHeader( "Diamond Gradient" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static ImVec2 uv_start( 0.5f, 0.5f );
				static ImVec2 uv_end( 1.0f, 0.5f );
				static ImVec4 cola_v( 1.0f, 0.0f, 0.0f, 1.0f );
				static ImVec4 colb_v( 0.0f, 1.0f, 0.0f, 0.25f );
				static ImU32 cola = ImGui::GetColorU32( cola_v );
				static ImU32 colb = ImGui::GetColorU32( colb_v );
#ifdef DEAR_WIDGETS_TESSELATION
				static int tess = 5;
				ImGui::SliderInt( "Tess", &tess, 0, 16 );
#endif
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				Slider2DFloat( "uv0", &uv_start.x, &uv_start.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::SameLine();
				Slider2DFloat( "uv1", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &cola_v.x ) )
					cola = ImGui::GetColorU32( cola_v );
				ImGui::SameLine();
				if ( ImGui::ColorEdit4( "ColB##DrawShape", &colb_v.x ) )
					colb = ImGui::GetColorU32( colb_v );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				static ImShape shape;
				GenShapeRect( shape, ImRect( pos, pos + ImVec2( size, size ) ) );
				ShapeSetDefaultUV( shape );
#ifdef DEAR_WIDGETS_TESSELATION
				for ( int k = 0; k < tess; ++k )
					ShapeTesselationUniform( shape );
#endif
				ShapeOkLabDiamondGradient( shape,
										   uv_start, uv_end,
										   cola, colb );
				DrawShape( pDrawList, shape );
				ImGui::Dummy( ImVec2( size, size ) );
				ImGui::Text( "Tri: %d", shape.triangles.size() );
				ImGui::Text( "Vtx: %d", shape.vertices.size() );
			}
			if ( ImGui::CollapsingHeader( "Image Shape" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static int tri_idx = -1;
				static float edge_thickness = 2.0f;
				static float vertex_radius = 4.0f;
				static ImVec4 edge_col_v( 1.0f, 0.0f, 0.0f, 1.0f );
				static ImVec4 triangle_col_v( 0.0f, 1.0f, 0.0f, 1.0f );
				static ImVec4 vertex_col_v( 0.0f, 0.0f, 1.0f, 1.0f );
				static ImU32 edge_col = ImGui::GetColorU32( edge_col_v );
				static ImU32 triangle_col = ImGui::GetColorU32( triangle_col_v );
				static ImU32 vertex_col = ImGui::GetColorU32( vertex_col_v );
				static float angle_min =  IM_PI / 6.0f;
				static float angle_max =  11.0f * IM_PI / 6.0f;
				static float radius = size * 0.5f;
				static int division = 12;
#ifdef DEAR_WIDGETS_TESSELATION
				static int tess = 0;
				ImGui::SliderInt( "Tess", &tess, 0, 16 );
#endif
				ImGui::SliderFloat( "Thickness", &edge_thickness, 0.0f, 16.0f );
				ImGui::SliderFloat( "Vrtx Radius", &vertex_radius, 0.0f, 64.0f );
				if ( ImGui::ColorEdit4( "Edge##DrawShape", &edge_col_v.x ) )
					edge_col = ImGui::GetColorU32( edge_col_v );
				if ( ImGui::ColorEdit4( "Triangle##DrawShape", &triangle_col_v.x ) )
					triangle_col = ImGui::GetColorU32( triangle_col_v );
				if ( ImGui::ColorEdit4( "Vertices##DrawShape", &vertex_col_v.x ) )
					vertex_col = ImGui::GetColorU32( vertex_col_v );
				ImGui::SliderInt( "Division", &division, 3, 64 );
				ImGui::SliderFloat( "Radius", &radius, 0.0f, size * 0.5f );
				ImGui::SliderAngle( "AngleMin", &angle_min, -360.0f, angle_max * 180.0f / IM_PI );
				ImGui::SliderAngle( "AngleMax", &angle_max, angle_min * 180.0f / IM_PI, 360.0f );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				static ImShape shape;
				GenShapeCircleArc( shape, pos + ImVec2( 0.5f * size, 0.5f * size ), radius, angle_min, angle_max, division );
				ShapeSetDefaultBoundUVWhiteCol( shape );
#ifdef DEAR_WIDGETS_TESSELATION
				for ( int k = 0; k < tess; ++k )
					ShapeTesselationUniform( shape );
#endif
				DrawImageShapeDebug( pDrawList, background, shape, edge_thickness, edge_col, triangle_col, vertex_radius, vertex_col, tri_idx );
				ImGui::Dummy( ImVec2( size, size ) );
				ImGui::SliderInt( "tri_idx", &tri_idx, -1, shape.triangles.size() - 1 );
				ImGui::Text( "Tri: %d", shape.triangles.size() );
				ImGui::Text( "Vtx: %d", shape.vertices.size() );
			}
			if ( ImGui::CollapsingHeader( "Image Shape Gradient" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static int tri_idx = -1;
				static float edge_thickness = 2.0f;
				static float vertex_radius = 16.0f;
				static ImVec4 edge_col_v( 1.0f, 0.0f, 0.0f, 1.0f );
				static ImVec4 triangle_col_v( 0.0f, 1.0f, 0.0f, 1.0f );
				static ImVec4 vertex_col_v( 0.0f, 0.0f, 1.0f, 1.0f );
				static ImU32 edge_col = ImGui::GetColorU32( edge_col_v );
				static ImU32 triangle_col = ImGui::GetColorU32( triangle_col_v );
				static ImU32 vertex_col = ImGui::GetColorU32( vertex_col_v );
				static ImVec2 uv_start( 0.0f, 0.0f );
				static ImVec2 uv_end( 0.0f, 1.0f );
				static ImVec4 cola_v( 1.0f, 1.0f, 1.0f, 1.0f );
				static ImVec4 colb_v( 0.0f, 0.0f, 0.0f, 0.0f );
				static ImU32 cola = ImGui::GetColorU32( cola_v );
				static ImU32 colb = ImGui::GetColorU32( colb_v );
#ifdef DEAR_WIDGETS_TESSELATION
				static int tess = 5;
				ImGui::SliderInt( "Tess", &tess, 0, 16 );
#endif
				ImGui::SliderFloat( "Thickness", &edge_thickness, 0.0f, 16.0f );
				ImGui::SliderFloat( "Vrtx Radius", &vertex_radius, 0.0f, 64.0f );
				if ( ImGui::ColorEdit4( "Edge##DrawShape", &edge_col_v.x ) )
					edge_col = ImGui::GetColorU32( edge_col_v );
				if ( ImGui::ColorEdit4( "Triangle##DrawShape", &triangle_col_v.x ) )
					triangle_col = ImGui::GetColorU32( triangle_col_v );
				if ( ImGui::ColorEdit4( "Vertices##DrawShape", &vertex_col_v.x ) )
					vertex_col = ImGui::GetColorU32( vertex_col_v );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				Slider2DFloat( "uv0", &uv_start.x, &uv_start.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::SameLine();
				Slider2DFloat( "uv1", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &cola_v.x ) )
					cola = ImGui::GetColorU32( cola_v );
				ImGui::SameLine();
				if ( ImGui::ColorEdit4( "ColB##DrawShape", &colb_v.x ) )
					colb = ImGui::GetColorU32( colb_v );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				static ImShape shape;
				GenShapeRect( shape, ImRect( pos, pos + ImVec2( size, size ) ) );
				ShapeSetDefaultBoundUV( shape );
#ifdef DEAR_WIDGETS_TESSELATION
				for ( int k = 0; k < tess; ++k )
					ShapeTesselationUniform( shape );
#endif
				ShapeSRGBLinearGradient( shape,
										 uv_start, uv_end,
										 cola, colb );
				DrawImageShape( pDrawList, illlustration_img, shape );
				ImGui::Dummy( ImVec2( size, size ) );
				ImGui::Text( "Tri: %d", shape.triangles.size() );
				ImGui::Text( "Vtx: %d", shape.vertices.size() );
			}
			if ( ImGui::CollapsingHeader( "Triangles Pointers" ) )
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
				ImWidgets::DrawTriangleCursor( pDrawList, ImVec2( curPos.x + 1.0f * 32.0f, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursor( pDrawList, ImVec2( curPos.x + 3.0f * 32.0f, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursor( pDrawList, ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursor( pDrawList, ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );

				fPointerLine *= 3.0f;
				pDrawList->AddLine( ImVec2( curPos.x + 0.5f * 32.0f, curPos.y + fPointerLine ), ImVec2( curPos.x + 3.5f * 32.0f, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				pDrawList->AddLine( ImVec2( curPos.x + 5.0f * 32.0f, curPos.y ), ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				pDrawList->AddLine( ImVec2( curPos.x + 7.0f * 32.0f, curPos.y ), ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 1.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 3.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawTriangleCursorFilled( pDrawList, ImVec2( curPos.x + 1.0f * 32.0f, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursorFilled( pDrawList, ImVec2( curPos.x + 3.0f * 32.0f, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursorFilled( pDrawList, ImVec2( curPos.x + 5.0f * 32.0f, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursorFilled( pDrawList, ImVec2( curPos.x + 7.0f * 32.0f, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
			}
			if ( ImGui::CollapsingHeader( "Signet Pointer" ) )
			{
				float const widthZone = ImGui::GetContentRegionAvail().x;

				static float angle = 0.0f;
				static float width = 16.0f;
				static float height = 21.0f;
				static float height_ratio = 1.0f / 3.0f;
				static float align01 = 0.5f;
				static float thickness = 5.0f;
				ImGui::SliderAngle( "Angle##Triangle", &angle );
				ImGui::SliderFloat( "Width##Triangle", &width, 1.0f, 64.0f );
				ImGui::SliderFloat( "Height##Triangle", &height, 1.0f, 128.0f );
				ImGui::SliderFloat( "Array Ratio##Triangle", &height_ratio, 0.0f, 1.0f );
				ImGui::SliderFloat( "Align##Triangle", &align01, 0.0f, 1.0f );
				ImGui::SliderFloat( "Thickness##Triangle", &thickness, 1.0f, 16.0f );

				ImVec2 curPos = ImGui::GetCursorScreenPos();
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				ImGui::InvisibleButton( "##Zone", ImVec2( widthZone, height * 1.1f ), 0 );
				ImGui::InvisibleButton( "##Zone", ImVec2( widthZone, height * 1.1f ), 0 );
				float fPointerLine = 32.0f;
				float dx = 16.0f;
				pDrawList->AddLine( ImVec2( curPos.x + 0.5f * dx, curPos.y + fPointerLine ), ImVec2( curPos.x + 11.5f * dx, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f );
				ImVec4 vBlue( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 1.0f );
				ImU32 uBlue = ImGui::GetColorU32( vBlue );
				ImWidgets::DrawSignetCursor( pDrawList, ImVec2( curPos.x + 1.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, align01, angle, thickness, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 1.0f * dx, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetFilledCursor( pDrawList, ImVec2( curPos.x + 3.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, align01, angle, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 3.0f * dx, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetCursor( pDrawList, ImVec2( curPos.x + 5.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, 0.0f, angle, thickness, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 5.0f * dx, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetFilledCursor( pDrawList, ImVec2( curPos.x + 7.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, 0.0f, angle, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 7.0f * dx, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetCursor( pDrawList, ImVec2( curPos.x + 9.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, 1.0f, angle, thickness, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 9.0f * dx, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetFilledCursor( pDrawList, ImVec2( curPos.x + 11.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, 1.0f, angle, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 11.0f * dx, curPos.y + fPointerLine ), 4.0f, IM_COL32( 255, 128, 0, 255 ), 16 );
			}
			if ( ImGui::CollapsingHeader( "Color Bands" ) )
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

#ifdef __cpp_lambdas 
				ImGui::Separator();
				ImGui::Text( "Custom Color Band" );
				static int frequency = 6;
				ImGui::SliderInt( "Frequency##ColorBand", &frequency, 1, 32 );
				static float alpha = 1.0f;
				ImGui::SliderFloat( "alpha##ColorBand", &alpha, 0.0f, 1.0f );
				float data[] = { ( float )frequency, alpha };
				DrawProceduralColor1DBilinear(
					ImGui::GetWindowDrawList(),
					[]( float t, void* pUserData ) -> ImU32{
						float fFrequency = ( ( float* )pUserData )[ 0 ];
						float fAlpha = ( ( float* )pUserData )[ 1 ];
						float r = ImSign( ImSin( fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 0.0f / fFrequency ) ) * 0.5f + 0.5f;
						float g = ImSign( ImSin( fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 2.0f / fFrequency ) ) * 0.5f + 0.5f;
						float b = ImSign( ImSin( fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 4.0f / fFrequency ) ) * 0.5f + 0.5f;

						return IM_COL32( r * 255, g * 255, b * 255, fAlpha * 255 );
					},
					&data[ 0 ],
					0.0f, 1.0f, ImGui::GetCursorScreenPos(), ImVec2( width, height ), division );
				ImGui::InvisibleButton( "Custom##ColorBand", ImVec2( width, height ), 0 );
#else
				// TODO add function pointer C-like
				// ImColor1DCallback
				// ImU32 CustomColorBand( float x, void* );
#endif
			}
			if ( ImGui::CollapsingHeader( "Color Ring" ) )
			{
				float const width = ImGui::GetContentRegionAvail().x;

				static int division = 16;
				ImGui::SliderInt( "Division", &division, 3, 128 );
				static float colorOffset = 16;
				ImGui::SliderFloat( "Color Offset", &colorOffset, 0.0f, 2.0f );
				static float thickness = 0.5f;
				ImGui::SliderFloat( "Thickness", &thickness, 1.0f / width, 1.0f );

				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				{
					//float const width = ImGui::GetContentRegionAvail().x;
					ImVec2 curPos = ImGui::GetCursorScreenPos();
					ImGui::InvisibleButton( "##Zone", ImVec2( width, width ), 0 );

					DrawColorRing( pDrawList, curPos, ImVec2( width, width ), thickness,
								   []( float t, void* ){
									   float r, g, b;
									   ImGui::ColorConvertHSVtoRGB( t, 1.0f, 1.0f, r, g, b );

									   return IM_COL32( r * 255, g * 255, b * 255, 255 );
								   }, NULL, division, colorOffset, true );
				}
				static float center = 0.5f;
				ImGui::DragFloat( "Center", &center, 0.01f, 0.0f, 1.0f );
				static float colorDotBound = 0.5f;
				ImGui::SliderFloat( "Alpha Pow", &colorDotBound, -1.0f, 1.0f );
				static int frequency = 6;
				ImGui::SliderInt( "Frequency", &frequency, 1, 32 );
				{
					ImGui::Text( "Nearest" );
					//float const width = ImGui::GetContentRegionAvail().x;
					ImVec2 curPos = ImGui::GetCursorScreenPos();
					ImGui::InvisibleButton( "##Zone", ImVec2( width, width ) * 0.5f, 0 );

					float data[] = { center, colorDotBound };
					DrawColorRing( pDrawList, curPos, ImVec2( width, width * 0.5f ), thickness,
								   []( float t, void* pUserData ){
									   float fCenter = ( ( float* )pUserData )[ 0 ];
									   float fColorDotBound = ( ( float* )pUserData )[ 1 ];
									   float r, g, b;
									   ImGui::ColorConvertHSVtoRGB( t, 1.0f, 1.0f, r, g, b );

									   ImVec2 const v0( ImCos( t * 2.0f * IM_PI ), ImSin( t * 2.0f * IM_PI ) );
									   ImVec2 const v1( ImCos( fCenter * 2.0f * IM_PI ), ImSin( fCenter * 2.0f * IM_PI ) );

									   float const dot = ImDot( v0, v1 );
									   //float const angle = ImAcos( dot ) / IM_PI;// / width;

									   return IM_COL32( r * 255, g * 255, b * 255, ( dot > fColorDotBound ? 1.0f : 0.0f ) * 255 );
								   }, &data[ 0 ], division, colorOffset, false );
				}
				{
					ImGui::Text( "Custom" );
					ImVec2 curPos = ImGui::GetCursorScreenPos();
					ImGui::InvisibleButton( "##Zone", ImVec2( width, width ) * 0.5f, 0 );

					float fFreqValue = (float)frequency;
					DrawColorRing( pDrawList, curPos, ImVec2( width, width ) * 0.5f, thickness,
								   []( float t, void* pUserData ){
									   float fFreq = *( ( float* )pUserData );
									   float v = ImSign( ImCos( fFreq * 2.0f * IM_PI * t ) ) * 0.5f + 0.5f;

									   return IM_COL32( v * 255, v * 255, v * 255, 255 );
								   }, &fFreqValue, division, colorOffset, true );
				}
			}
			if ( ImGui::CollapsingHeader( "OkLab/OkLch Color Quad" ) )
			{
				float const width = ImGui::GetContentRegionAvail().x;

				static int resX = 16;
				static int resY = 16;
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				ImGui::SliderInt( "resX", &resX, 4, 64 ); ImGui::SameLine();
				ImGui::SliderInt( "resY", &resY, 4, 64 );
				static float L = 1.0f;
				ImGui::SliderFloat( "L", &L, 0.0f, 1.0f );

				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				ImVec2 curPos = ImGui::GetCursorScreenPos();
				float const size = ImGui::GetContentRegionAvail().x;
				DrawOkLabQuad( pDrawList, curPos, ImVec2( size, size ), L, resX, resY );
				ImGui::Dummy( ImVec2( size, size ) );
				curPos = ImGui::GetCursorScreenPos();
				DrawOkLchQuad( pDrawList, curPos, ImVec2( size, size ), L, resX, resY );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Color2D" ) )
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
				ImColor2DCallback func = []( float x, float y, void* pUserData ) -> ImU32{
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
			}
			if ( ImGui::CollapsingHeader( "Image Convex Shape" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static ImVec2 uv_offset( 0.0f, 0.0f );
				static ImVec2 uv_scale( 1.0f, 1.0f );
				ImGui::DragFloat2( "Offset##DrawImageConvexShape", &uv_offset[ 0 ], 0.001f, -3.0f, 3.0f );
				ImGui::DragFloat2( "Scale##DrawImageConvexShape", &uv_scale[ 0 ], 0.001f, -3.0f, 3.0f );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImVector<ImVec2> disk;
				disk.resize( 32 );
				for ( int k = 0; k < 32; ++k )
				{
					float angle = ( ( float )k ) * 2.0f * IM_PI / 32.0f;
					float cos0 = ImCos( angle );
					float sin0 = ImSin( angle );
					disk[ k ].x = pos.x + 0.5f * size + cos0 * size * 0.5f;
					disk[ k ].y = pos.y + 0.5f * size + sin0 * size * 0.5f;
				}
				DrawImageConvexShape( pDrawList, background, &disk[ 0 ], 32, IM_COL32( 255, 255, 255, 255 ), uv_offset, uv_scale );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Image Concave Shape" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static ImVec2 uv_offset( 0.0f, 0.0f );
				static ImVec2 uv_scale( 1.0f, 1.0f );
				ImGui::DragFloat2( "Offset##DrawImageConcaveShape", &uv_offset[ 0 ], 0.001f, -3.0f, 3.0f );
				ImGui::DragFloat2( "Scale##DrawImageConcaveShape", &uv_scale[ 0 ], 0.001f, -3.0f, 3.0f );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				int sz = 8;
				ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.3f, 0.0f }, { 0.3f, 0.7f }, { 0.7f, 0.7f }, { 0.7f, 0.0f },
									   { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
				for ( int k = 0; k < sz; ++k )
				{
					ImVec2& v = pos_norms[ k ];
					v.x *= size;
					v.y *= size;
					v += pos;
				}
				DrawImageConcaveShape( pDrawList, background, &pos_norms[ 0 ], sz, IM_COL32( 255, 255, 255, 255 ), uv_offset, uv_scale );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Shape with Hole" ) )
			{
				static ImVec4 col = { 1, 0, 0, 1 };
				static int gap = 1;
				static int strokeWidth = 1;
				ImGui::ColorEdit4( "Color##Hole", &col.x );
				ImGui::SliderInt( "Gap##Hole", &gap, 1, 16 );
				ImGui::SliderInt( "Stroke Width##Hole", &strokeWidth, 1, 16 );
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();

				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f },
									   { 0.3f, 0.3f }, { 0.7f, 0.3f }, { 0.7f, 0.7f }, { 0.3f, 0.7f }, { 0.3f, 0.3f } };
				for ( ImVec2& v : pos_norms )
				{
					v.x *= size;
					v.y *= size;
					v += pos;
				}

				ImRect bb( pos, pos + ImVec2( size, size ) );
				DrawShapeWithHole( pDrawList, &pos_norms[ 0 ], 10, IM_COL32( 255 * col.x, 255 * col.y, 255 * col.z, 255 * col.w ), &bb, gap, strokeWidth );

				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Chromaticity Plot" ) )
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				float const size = ImGui::GetContentRegionAvail().x;

				static int chromLinesampleCount = 128;
				ImGui::SliderInt( "Chromatic Sample Count##Chromaticity", &chromLinesampleCount, 3, 256 );
				static int resX = 64;
				ImGui::SliderInt( "Resolution X##Chromaticity", &resX, 3, 256 );
				static int resY = 64;
				ImGui::SliderInt( "Resolution Y##Chromaticity", &resY, 3, 256 );
				static int waveMin = 400;
				static int waveMax = 700;
				ImGui::SliderInt( "Wavelength Min##Chromaticity", &waveMin, 300, waveMax );
				ImGui::SliderInt( "Wavelength Max##Chromaticity", &waveMax, waveMin, 800 );
				char const* observer[] = { "1931 2 deg", "1964 10 deg" };
				char const* illum[] = { "D50", "D65" };
				char const* colorSpace[] = { "AdobeRGB", "AppleRGB", "Best", "Beta", "Bruce", "CIERGB",
					"ColorMatch", "Don_RGB_4", "ECI","Ekta_Space_PS5", "NTSC",
					"PAL_SECAM", "ProPhoto", "SMPTE_C", "sRGB", "WideGamutRGB", "Rec2020" };
				static int curObserver = 0;
				static int curIllum = 1;
				static int curColorSpace = 0;
				ImGui::Combo( "Observer##Chromaticity", &curObserver, observer, IM_ARRAYSIZE( observer ) );
				ImGui::Combo( "Illuminance##Chromaticity", &curIllum, illum, IM_ARRAYSIZE( illum ) );
				ImGui::Combo( "ColorSpace##Chromaticity", &curColorSpace, colorSpace, IM_ARRAYSIZE( colorSpace ) );
				static ImVec4 vMaskColor( 1.0f, 1.0f, 1.0f, 0.5f );
				ImGui::ColorEdit4( "Mask Color##Chromaticity", &vMaskColor.x );
				static bool showColorSpaceTriangle = true;
				ImGui::Checkbox( "Color Space Triangle##Chromaticity", &showColorSpaceTriangle );
				static bool showWhitePoint = true;
				ImGui::Checkbox( "White Point##Chromaticity", &showWhitePoint );
				ImU32 maskColor = ImGui::ColorConvertFloat4ToU32( vMaskColor );

				static ImVec2 vMin( -0.2f, -0.1f );
				static ImVec2 vMax( 1.0f, 1.0f );

				if ( ImGui::DragFloat2( "min##Chromaticity", &vMin.x, 0.001f, -1.0f, 2.0f ) )
				{
					vMin.x = ImMin( vMin.x, vMax.x - 1e-6f );
					vMin.y = ImMin( vMin.y, vMax.y - 1e-6f );
				}
				if ( ImGui::DragFloat2( "max##Chromaticity", &vMax.x, 0.001f, -1.0f, 2.0f ) )
				{
					vMax.x = ImMax( vMax.x, vMin.x + 1e-6f );
					vMax.y = ImMax( vMax.y, vMin.y + 1e-6f );
				}

				static bool showBorder = true;
				ImGui::Checkbox( "Show Border##Chromaticity", &showBorder );
				static ImVec4 borderColor = ( ImVec4 )ImColor( IM_COL32( 0, 0, 0, 255 ) );
				ImGui::ColorEdit4( "Border Color##Chromaticity", &borderColor.x );
				static float borderThickness = 3.0f;
				ImGui::SliderFloat( "Border Thickness##Chromaticity", &borderThickness, 0.5f, 10.0f );

				ImVec2 pos = ImGui::GetCursorScreenPos();
				DrawChromaticityPlot( pDrawList,
									  curIllum,
									  curObserver,
									  curColorSpace,
									  chromLinesampleCount,
									  pos, ImVec2( size, size ),
									  resX, resY,
									  maskColor,
									  ( float )waveMin, ( float )waveMax,
									  vMin.x, vMax.x,
									  vMin.y, vMax.y,
									  showColorSpaceTriangle,
									  showWhitePoint,
									  showBorder,
									  ImGui::GetColorU32( borderColor ),
									  borderThickness );

				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Chromaticity Line/Point" ) )
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				float const size = ImGui::GetContentRegionAvail().x;

				static float temp = 6504.0f;
				ImGui::SliderFloat( "Temperature K##ChromaticityLines", &temp, 1000.0f, 12000.0f );
				static int samplesCount = 64;
				ImGui::SliderInt( "Sample Count##ChromaticityLines", &samplesCount, 2, 256 );

				static ImVec2 vMin( 0.261f, 0.285f );
				static ImVec2 vMax( 0.446f, 0.395f );

				if ( ImGui::DragFloat2( "min##ChromaticityLines", &vMin.x, 0.001f, -1.0f, 2.0f ) )
				{
					vMin.x = ImMin( vMin.x, vMax.x - 1e-6f );
					vMin.y = ImMin( vMin.y, vMax.y - 1e-6f );
				}
				if ( ImGui::DragFloat2( "max##ChromaticityLines", &vMax.x, 0.001f, -1.0f, 2.0f ) )
				{
					vMax.x = ImMax( vMax.x, vMin.x + 1e-6f );
					vMax.y = ImMax( vMax.y, vMin.y + 1e-6f );
				}

				ImVector<ImU32> colors;
				colors.resize( samplesCount );
				for ( int i = 0; i < samplesCount; ++i )
				{
					ImU32 col = KelvinTemperatureTosRGBColors( ImLerp( 3000.0f, 8000.0f, ( float )i / ( ( float )( samplesCount - 1 ) ) ) );
					colors[ i ] = col;
				}
				ImU32 tempCol = KelvinTemperatureTosRGBColors( temp );

				static ImVec4 lineColor = ( ImVec4 )ImColor( IM_COL32( 0, 0, 0, 255 ) );
				ImGui::ColorEdit4( "Line Color##ChromaticityLines", &lineColor.x );
				static float lineThickness = 5.0f;
				ImGui::SliderFloat( "Border Thickness##ChromaticityLines", &lineThickness, 0.5f, 10.0f );

				ImVec2 pos = ImGui::GetCursorScreenPos();
				DrawChromaticityPlot( pDrawList,
									  ImWidgetsWhitePointChromaticPlot_D55,
									  ImWidgetsObserverChromaticPlot_1964_10deg,
									  ImWidgetsColorSpace_sRGB,
									  128,
									  pos, ImVec2( size, size ),
									  64, 64,
									  IM_COL32( 255, 255, 255, 255 ),
									  //IM_COL32( 21, 21, 21, 255 ),
									  360.0f, 830.0f,
									  vMin.x, vMax.x,
									  vMin.y, vMax.y,
									  true,
									  true,
									  true,
									  IM_COL32( 0, 0, 0, 255 ),
									  2.0f );
				DrawChromaticityLines( pDrawList,
									   pos,
									   ImVec2( size, size ),
									   &colors[ 0 ],
									   samplesCount,
									   vMin.x, vMax.x,
									   vMin.y, vMax.y,
									   ImGui::GetColorU32( lineColor ),
									   ImDrawFlags_None,
									   lineThickness );
				DrawChromaticityPoints( pDrawList,
										pos,
										ImVec2( size, size ),
										&tempCol,
										1,
										vMin.x, vMax.x,
										vMin.y, vMax.y,
										IM_COL32( 255, 0, 0, 255 ), 16.0f, 4 );

				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Linear Line Graduation" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static float mainLineThickness = 1.0f;
				static ImU32 mainCol = IM_COL32( 255, 255, 255, 255 );
				static int division0 = 3;  static float height0 = 32.0f; static float thickness0 = 5.0f; static float angle0 = 0; static ImU32 col0 = IM_COL32( 255, 0, 0, 255 );
				static int division1 = 5;  static float height1 = 16.0f; static float thickness1 = 2.0f; static float angle1 = 0; static ImU32 col1 = IM_COL32( 0, 255, 0, 255 );
				static int division2 = 10; static float height2 = 8.0f;  static float thickness2 = 1.0f; static float angle2 = 0; static ImU32 col2 = IM_COL32( 255, 255, 0, 255 );
				static int divisions[] = { division0, division1, division2 };
				static float heights[] = { height0, height1, height2 };
				static float thicknesses[] = { thickness0, thickness1, thickness2 };
				static float angles[] = { angle0, angle1, angle2 };
				static ImVec4 colors[] = { ImGui::ColorConvertU32ToFloat4( col0 ), ImGui::ColorConvertU32ToFloat4( col1 ), ImGui::ColorConvertU32ToFloat4( col2 ) };

				ImGui::DragFloat( "Main Thickness", &mainLineThickness, 1.0f, 1.0f, 16.0f );
				ImVec4 vMainCol = ImGui::ColorConvertU32ToFloat4( mainCol );
				if ( ImGui::ColorEdit3( "Main", &vMainCol.x ) )
					mainCol = ImGui::GetColorU32( vMainCol );

				ImGui::DragInt3( "Divisions", &divisions[ 0 ], 1.0f, 1, 10 );
				ImGui::DragFloat3( "Heights", &heights[ 0 ], 1.0f, 1.0f, 128.0f );
				ImGui::DragFloat3( "Thicknesses", &thicknesses[ 0 ], 1.0f, 1.0f, 16.0f );
				ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
				ImGui::SliderAngle( "a0", &angles[ 0 ] ); ImGui::SameLine();
				ImGui::SliderAngle( "a1", &angles[ 1 ] ); ImGui::SameLine();
				ImGui::SliderAngle( "a2", &angles[ 2 ] );
				ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit3( "c0", &colors[ 0 ].x ) )
					col0 = ImGui::GetColorU32( colors[ 0 ] );
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c1", &colors[ 1 ].x ) )
					col1 = ImGui::GetColorU32( colors[ 1 ] );
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c2", &colors[ 2 ].x ) )
					col2 = ImGui::GetColorU32( colors[ 2 ] );

				float height = ImMax( heights[ 0 ], ImMax( heights[ 1 ], heights[ 2 ] ) );
				ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
				DrawLinearLineGraduation( pDrawList, pos, pos + ImVec2( size, 0.0f ),
										  mainLineThickness, mainCol,
										  divisions[ 0 ], heights[ 0 ], thicknesses[ 0 ], angles[ 0 ], col0,
										  divisions[ 1 ], heights[ 1 ], thicknesses[ 1 ], angles[ 1 ], col1,
										  divisions[ 2 ], heights[ 2 ], thicknesses[ 2 ], angles[ 2 ], col2 );
				ImGui::Dummy( ImVec2( size, height ) );
				DrawLinearLineGraduation( pDrawList, pos, pos + ImVec2( size, size ),
										  mainLineThickness, mainCol,
										  divisions[ 0 ], heights[ 0 ], thicknesses[ 0 ], angles[ 0 ], col0,
										  divisions[ 1 ], heights[ 1 ], thicknesses[ 1 ], angles[ 1 ], col1,
										  divisions[ 2 ], heights[ 2 ], thicknesses[ 2 ], angles[ 2 ], col2 );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Linear Circular Graduation" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static float mainLineThickness = 1.0f;
				static ImU32 mainCol = IM_COL32( 255, 255, 255, 255 );
				static int division0 = 3;  static float height0 = 32.0f; static float thickness0 = 5.0f; static float angle0 = 0; static ImU32 col0 = IM_COL32( 255, 0, 0, 255 );
				static int division1 = 5;  static float height1 = 16.0f; static float thickness1 = 2.0f; static float angle1 = 0; static ImU32 col1 = IM_COL32( 0, 255, 0, 255 );
				static int division2 = 10; static float height2 = 8.0f;  static float thickness2 = 1.0f; static float angle2 = 0; static ImU32 col2 = IM_COL32( 255, 255, 0, 255 );
				static int divisions[] = { division0, division1, division2 };
				static float heights[] = { height0, height1, height2 };
				static float thicknesses[] = { thickness0, thickness1, thickness2 };
				static float angles[] = { angle0, angle1, angle2 };
				static float start_angle = -IM_PI / 3.0f;
				static float end_angle = 4.0f * IM_PI / 3.0f;
				static float angles_bound[] = { start_angle, end_angle };
				static float radius = size * 0.5f - 2.0f * ImMax( height0, ImMax( height1, height2 ) );
				static int num_segments = 0;
				static ImVec4 colors[] = { ImGui::ColorConvertU32ToFloat4( col0 ), ImGui::ColorConvertU32ToFloat4( col1 ), ImGui::ColorConvertU32ToFloat4( col2 ) };

				ImGui::DragFloat( "Main Thickness", &mainLineThickness, 1.0f, 1.0f, 16.0f );
				ImVec4 vMainCol = ImGui::ColorConvertU32ToFloat4( mainCol );
				if ( ImGui::ColorEdit3( "Main", &vMainCol.x ) )
					mainCol = ImGui::GetColorU32( vMainCol );

				ImGui::DragInt3( "Divisions", &divisions[ 0 ], 1.0f, 1, 10 );
				ImGui::DragFloat3( "Heights", &heights[ 0 ], 1.0f, 1.0f, 128.0f );
				ImGui::DragFloat3( "Thicknesses", &thicknesses[ 0 ], 1.0f, 1.0f, 16.0f );
				ImGui::DragFloat( "Radius", &radius, 1.0f, 1.0f, size );
				ImGui::DragInt( "Segment", &num_segments, 1.0f, 0, 64 );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				ImGui::SliderAngle( "start angle", &angles_bound[ 0 ], -360.0f, angles_bound[ 1 ] * 180.0f / IM_PI ); ImGui::SameLine();
				ImGui::SliderAngle( "end angle", &angles_bound[ 1 ], angles_bound[ 0 ] * 180.0f / IM_PI, 360.0f );
				ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
				ImGui::SliderAngle( "a0", &angles[ 0 ] ); ImGui::SameLine();
				ImGui::SliderAngle( "a1", &angles[ 1 ] ); ImGui::SameLine();
				ImGui::SliderAngle( "a2", &angles[ 2 ] );
				ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit3( "c0", &colors[ 0 ].x ) )
					col0 = ImGui::GetColorU32( colors[ 0 ] );
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c1", &colors[ 1 ].x ) )
					col1 = ImGui::GetColorU32( colors[ 1 ] );
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c2", &colors[ 2 ].x ) )
					col2 = ImGui::GetColorU32( colors[ 2 ] );

				float height = ImMax( heights[ 0 ], ImMax( heights[ 1 ], heights[ 2 ] ) );
				ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
				DrawLinearCircularGraduation( pDrawList, pos + ImVec2( size * 0.5f, size * 0.5f ), radius, angles_bound[ 0 ], angles_bound[ 1 ], num_segments,
											  mainLineThickness, mainCol,
											  divisions[ 0 ], heights[ 0 ], thicknesses[ 0 ], angles[ 0 ], col0,
											  divisions[ 1 ], heights[ 1 ], thicknesses[ 1 ], angles[ 1 ], col1,
											  divisions[ 2 ], heights[ 2 ], thicknesses[ 2 ], angles[ 2 ], col2 );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Log Line Graduation" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static float mainLineThickness = 1.0f;
				static ImU32 mainCol = IM_COL32( 255, 255, 255, 255 );
				static int division0 = 3;  static float height0 = 32.0f; static float thickness0 = 5.0f; static float angle0 = 0; static ImU32 col0 = IM_COL32( 255, 0, 0, 255 );
				static int division1 = 10;  static float height1 = 16.0f; static float thickness1 = 2.0f; static float angle1 = 0; static ImU32 col1 = IM_COL32( 0, 255, 0, 255 );
				static int divisions[] = { division0, division1 };
				static float heights[] = { height0, height1 };
				static float thicknesses[] = { thickness0, thickness1 };
				static float angles[] = { angle0, angle1  };
				static ImVec4 colors[] = { ImGui::ColorConvertU32ToFloat4( col0 ), ImGui::ColorConvertU32ToFloat4( col1 ) };

				ImGui::DragFloat( "Main Thickness", &mainLineThickness, 1.0f, 1.0f, 16.0f );
				ImVec4 vMainCol = ImGui::ColorConvertU32ToFloat4( mainCol );
				if ( ImGui::ColorEdit3( "Main", &vMainCol.x ) )
					mainCol = ImGui::GetColorU32( vMainCol );

				ImGui::DragInt2( "Divisions", &divisions[ 0 ], 1.0f, 1, 20 );
				ImGui::DragFloat2( "Heights", &heights[ 0 ], 1.0f, 1.0f, 128.0f );
				ImGui::DragFloat2( "Thicknesses", &thicknesses[ 0 ], 1.0f, 1.0f, 16.0f );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				ImGui::SliderAngle( "a0", &angles[ 0 ] ); ImGui::SameLine();
				ImGui::SliderAngle( "a1", &angles[ 1 ] ); ImGui::SameLine();
				ImGui::SliderAngle( "a2", &angles[ 2 ] );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit3( "c0", &colors[ 0 ].x ) )
					col0 = ImGui::GetColorU32( colors[ 0 ] );
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c1", &colors[ 1 ].x ) )
					col1 = ImGui::GetColorU32( colors[ 1 ] );

				float height = ImMax( heights[ 0 ], ImMax( heights[ 1 ], heights[ 2 ] ) );
				ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
				DrawLogLineGraduation( pDrawList, pos, pos + ImVec2( size, 0.0f ),
									   mainLineThickness, mainCol,
									   divisions[ 0 ], heights[ 0 ], thicknesses[ 0 ], angles[ 0 ], col0,
									   divisions[ 1 ], heights[ 1 ], thicknesses[ 1 ], angles[ 1 ], col1 );
				ImGui::Dummy( ImVec2( size, height ) );
				DrawLogLineGraduation( pDrawList, pos, pos + ImVec2( size, size ),
									   mainLineThickness, mainCol,
									   divisions[ 0 ], heights[ 0 ], thicknesses[ 0 ], angles[ 0 ], col0,
									   divisions[ 1 ], heights[ 1 ], thicknesses[ 1 ], angles[ 1 ], col1 );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Log Circular Graduation" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static float mainLineThickness = 1.0f;
				static ImU32 mainCol = IM_COL32( 255, 255, 255, 255 );
				static int division0 =  3;  static float height0 = 32.0f; static float thickness0 = 5.0f; static float angle0 = 0; static ImU32 col0 = IM_COL32( 255, 0, 0, 255 );
				static int division1 = 10;  static float height1 = 16.0f; static float thickness1 = 2.0f; static float angle1 = 0; static ImU32 col1 = IM_COL32( 0, 255, 0, 255 );
				static int divisions[] = { division0, division1 };
				static float heights[] = { height0, height1 };
				static float thicknesses[] = { thickness0, thickness1 };
				static float angles[] = { angle0, angle1 };
				static float start_angle = -IM_PI / 3.0f;
				static float end_angle = 4.0f * IM_PI / 3.0f;
				static float angles_bound[] = { start_angle, end_angle };
				static float radius = size * 0.5f - 2.0f * ImMax( height0, height1 );
				static int num_segments = 0;
				static ImVec4 colors[] = { ImGui::ColorConvertU32ToFloat4( col0 ), ImGui::ColorConvertU32ToFloat4( col1 ) };

				ImGui::DragFloat( "Main Thickness", &mainLineThickness, 1.0f, 1.0f, 16.0f );
				ImVec4 vMainCol = ImGui::ColorConvertU32ToFloat4( mainCol );
				if ( ImGui::ColorEdit3( "Main", &vMainCol.x ) )
					mainCol = ImGui::GetColorU32( vMainCol );

				ImGui::DragInt2( "Divisions", &divisions[ 0 ], 1.0f, 1, 20 );
				ImGui::DragFloat2( "Heights", &heights[ 0 ], 1.0f, 1.0f, 128.0f );
				ImGui::DragFloat2( "Thicknesses", &thicknesses[ 0 ], 1.0f, 1.0f, 16.0f );
				ImGui::DragFloat( "Radius", &radius, 1.0f, 1.0f, size );
				ImGui::DragInt( "Segment", &num_segments, 1.0f, 0, 64 );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				ImGui::SliderAngle( "start angle", &angles_bound[ 0 ], -360.0f, angles_bound[ 1 ] * 180.0f / IM_PI ); ImGui::SameLine();
				ImGui::SliderAngle( "end angle", &angles_bound[ 1 ], angles_bound[ 0 ] * 180.0f / IM_PI, 360.0f );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				ImGui::SliderAngle( "a0", &angles[ 0 ] ); ImGui::SameLine();
				ImGui::SliderAngle( "a1", &angles[ 1 ] );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit3( "c0", &colors[ 0 ].x ) )
					col0 = ImGui::GetColorU32( colors[ 0 ] );
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c1", &colors[ 1 ].x ) )
					col1 = ImGui::GetColorU32( colors[ 1 ] );

				float height = ImMax( heights[ 0 ], heights[ 1 ] );
				ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
				DrawLogCircularGraduation( pDrawList, pos + ImVec2( size * 0.5f, size * 0.5f ), radius, angles_bound[ 0 ], angles_bound[ 1 ], num_segments,
										   mainLineThickness, mainCol,
										   divisions[ 0 ], heights[ 0 ], thicknesses[ 0 ], angles[ 0 ], col0,
										   divisions[ 1 ], heights[ 1 ], thicknesses[ 1 ], angles[ 1 ], col1 );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			ImGui::Unindent();
		}
		if ( ImGui::CollapsingHeader( "Interactions" ) )
		{
			ImGui::Indent();
			if ( ImGui::CollapsingHeader( "Poly Convex Hovered" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } };
				for ( ImVec2& v : pos_norms )
				{
					v.x *= size;
					v.y *= size;
					v += pos;
				}
				bool hovered = IsMouseHoveringPolyConvex( pos, pos + ImVec2( size, size ), pos_norms, 3 );
				pDrawList->AddConvexPolyFilled( &pos_norms[ 0 ], 3, IM_COL32( hovered ? 255 : 0, hovered ? 0 : 255, 0, 255 ) );
				ImGui::Dummy( ImVec2( size, size ) );
				pos = ImGui::GetCursorScreenPos();
				ImVector<ImVec2> disk;
				disk.resize( 32 );
				for ( int k = 0; k < 32; ++k )
				{
					float angle = ( ( float )k ) * 2.0f * IM_PI / 32.0f;
					float cos0 = ImCos( angle );
					float sin0 = ImSin( angle );
					disk[ k ].x = pos.x + 0.5f * size + cos0 * size * 0.5f;
					disk[ k ].y = pos.y + 0.5f * size + sin0 * size * 0.5f;
				}
				hovered = IsMouseHoveringPolyConvex( pos, pos + ImVec2( size, size ), &disk[ 0 ], 32 );
				pDrawList->AddConvexPolyFilled( &disk[ 0 ], 32, IM_COL32( hovered ? 255 : 0, hovered ? 0 : 255, 0, 255 ) );

				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Poly Concave Hovered" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				ImVec2 pos = ImGui::GetCursorScreenPos();
				int sz = 8;
				ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.3f, 0.0f }, { 0.3f, 0.7f }, { 0.7f, 0.7f }, { 0.7f, 0.0f },
									   { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
				for ( int k = 0; k < sz; ++k )
				{
					ImVec2& v = pos_norms[ k ];
					v.x *= size;
					v.y *= size;
					v += pos;
				}
				bool hovered = IsMouseHoveringPolyConcave( pos * 0.99f, pos + ImVec2( 1.01f * size, 1.01f * size ), pos_norms, sz );
				pDrawList->AddConcavePolyFilled( &pos_norms[ 0 ], sz, IM_COL32( hovered ? 255 : 0, hovered ? 0 : 255, 0, 255 ) );
				ImGui::Dummy( ImVec2( size, size ) );
				pos = ImGui::GetCursorScreenPos();
				ImVector<ImVec2> ring;
				sz = 64;
				ring.resize( sz );
				srand( 97 );
				for ( int k = 0; k < sz; ++k )
				{
					float angle = -( ( float )k ) * 2.0f * IM_PI / 32.0f;
					float cos0 = ImCos( angle );
					float sin0 = ImSin( angle );
					float r = ( float )( rand() % ( ( int )ImRound( size ) ) );
					ring[ k ].x = pos.x + size * 0.5f + r * 0.5f * cos0;
					ring[ k ].y = pos.y + size * 0.5f + r * 0.5f * sin0;
				}
				hovered = IsMouseHoveringPolyConcave( pos * 0.99f, pos + ImVec2( 1.01f * size, 1.01f * size ), &ring[ 0 ], sz );
				pDrawList->AddConcavePolyFilled( &ring[ 0 ], sz, IM_COL32( hovered ? 255 : 0, hovered ? 0 : 255, 0, 255 ) );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			if ( ImGui::CollapsingHeader( "Poly With Hole Hovered" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				ImVec2 pos = ImGui::GetCursorScreenPos();
				int sz = 10;
				ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f },
									   { 0.3f, 0.3f }, { 0.7f, 0.3f }, { 0.7f, 0.7f }, { 0.3f, 0.7f }, { 0.3f, 0.3f } };
				for ( int k = 0; k < sz; ++k )
				{
					ImVec2& v = pos_norms[ k ];
					v.x *= size;
					v.y *= size;
					v += pos;
				}
				bool hovered = IsMouseHoveringPolyWithHole( pos * 0.99f, pos + ImVec2( 1.01f * size, 1.01f * size ), pos_norms, sz );
				DrawShapeWithHole( pDrawList, &pos_norms[ 0 ], sz, IM_COL32( hovered ? 255 : 0, hovered ? 0 : 255, 0, 255 ) );
				ImGui::Dummy( ImVec2( size, size ) );
				pos = ImGui::GetCursorScreenPos();
				ImVector<ImVec2> ring;
				sz = 64;
				ring.resize( sz );
				float r;
				for ( int k = 0; k < 32; ++k )
				{
					float angle = -( ( float )k ) * 2.0f * IM_PI / 31.0f;
					float cos0 = ImCos( angle );
					float sin0 = ImSin( angle );
					r = size * ( ( ( float )( rand() % 1000 ) / 1000.0f ) * 0.25f + 0.75f );
					ring[ k ].x = pos.x + size * 0.5f + r * 0.5f * cos0;
					ring[ k ].y = pos.y + size * 0.5f + r * 0.5f * sin0;
				}
				srand( 97 );
				for ( int k = 32; k < 64; ++k )
				{
					float angle = ( ( float )( k - 32 ) ) * 2.0f * IM_PI / 31.0f;
					float cos0 = ImCos( angle );
					float sin0 = ImSin( angle );
					r = size * 0.75f * ( ( ( float )( rand() % 1000 ) / 1000.0f ) * 0.5f + 0.5f );
					ring[ k ].x = pos.x + size * 0.5f + r * 0.5f * cos0;
					ring[ k ].y = pos.y + size * 0.5f + r * 0.5f * sin0;
				}
				hovered = IsMouseHoveringPolyWithHole( pos, pos + ImVec2( size, size ), &ring[ 0 ], sz );
				DrawShapeWithHole( pDrawList, &ring[ 0 ], sz, IM_COL32( hovered ? 255 : 0, hovered ? 0 : 255, 0, 255 ) );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			ImGui::Unindent();
		}
		if ( ImGui::CollapsingHeader( "Widgets" ) )
		{
			ImGui::Indent();
			if ( ImGui::CollapsingHeader( "Button Convex" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImVector<ImVec2> disk;
				disk.resize( 32 );
				for ( int k = 0; k < 32; ++k )
				{
					float angle = ( ( float )k ) * 2.0f * IM_PI / 32.0f;
					float cos0 = ImCos( angle );
					float sin0 = ImSin( angle );
					disk[ k ].x = 0.5f * size + cos0 * size * 0.5f;
					disk[ k ].y = 0.5f * size + sin0 * size * 0.5f;
				}
				static int value = 1;
				ImGui::Text( "Value: %d", value );
				if ( ImWidgets::ButtonExConvex( "Convex", ImVec2( 0, 0 ), &disk[ 0 ], 32, 0 ) )
					++value;
			}
			if ( ImGui::CollapsingHeader( "Button Concave" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				int sz = 8;
				ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.3f, 0.0f }, { 0.3f, 0.7f }, { 0.7f, 0.7f }, { 0.7f, 0.0f },
									   { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
				for ( int k = 0; k < sz; ++k )
				{
					ImVec2& v = pos_norms[ k ];
					v.x *= size;
					v.y *= size;
				}
				static int value = 1;
				ImGui::Text( "Value: %d", value );
				if ( ImWidgets::ButtonExConcave( "Concave", ImVec2( 0, 0 ), &pos_norms[ 0 ], sz, ImVec2( 0.0f, size / 3.0f ), 0 ) )
					++value;
			}
			if ( ImGui::CollapsingHeader( "Button With Hole" ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				int sz = 10;
				ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f },
									   { 0.3f, 0.3f }, { 0.7f, 0.3f }, { 0.7f, 0.7f }, { 0.3f, 0.7f }, { 0.3f, 0.3f } };
				for ( int k = 0; k < sz; ++k )
				{
					ImVec2& v = pos_norms[ k ];
					v.x *= size;
					v.y *= size;
				}
				static int value = 1;
				ImGui::Text( "Value: %d", value );
				if ( ImWidgets::ButtonExWithHole( "With Hole", ImVec2( 0, 0 ), &pos_norms[ 0 ], sz, ImVec2( 0.0f, size / 3.0f ), 0 ) )
					++value;
			}
#if 0
			if ( ImGui::CollapsingHeader( "DragFloatPrecise" ) )
			{
				static float value = 100.0f;
				ImWidgets::DragFloatPrecise( "Value##DragFloatPrecise", &value, -FLT_MAX, FLT_MAX, ImGuiSliderFlags_AlwaysClamp );

			}
#endif
			if ( ImGui::CollapsingHeader( "SliderN" ) )
			{
				static float value[ 3 ] = { 0.25f, 10.0f, 100.0f };
				static float min = 0.1f;
				static float max = 150.0f;
				ImGui::SetWindowFontScale( 0.75f );
				ImGui::Text( "Hover per region of influence" );
				ImGui::SetWindowFontScale( 1.0f );
				ImWidgets::SliderNScalar( "Values##SliderNRegions", ImGuiDataType_Float, &value, 3, &min, &max, 8.0f, true );
				ImGui::SetWindowFontScale( 0.75f );
				ImGui::Text( "Global Hover" );
				ImGui::SetWindowFontScale( 1.0f );
				ImWidgets::SliderNScalar( "Values##SliderNGlobal", ImGuiDataType_Float, &value, 3, &min, &max, 8.0f, false );
				ImGui::DragFloat( "Near Plane", &value[ 0 ], 1.0f, min, value[ 1 ] );
				ImGui::DragFloat( "Focal Planes", &value[ 1 ], 1.0f, value[ 0 ], value[ 2 ] );
				ImGui::DragFloat( "Far Planes", &value[ 2 ], 1.0f, value[ 1 ], max );
			}
#if 0 // TODO
			if ( ImGui::CollapsingHeader( "SliderRing" ) )
			{
				static float min = 0.0f;
				static float max = 1.0f;
				static float value = 0.5f;
				ImWidgets::SliderRingScalar( "Values##SliderRingScalar", ImGuiDataType_Float, &value, &min, &max, 0.0f, IM_PI, 64.0f, "%.3f", 0, NULL );
			}
#endif
			if ( ImGui::CollapsingHeader( "Hue Selector" ) )
			{
				static float offset = 1.0f;

				static int division = 32;
				ImGui::DragInt( "Division##HueSelector", &division, 1.0f, 2, 256 );
				static float alphaHue = 1.0f;
				static float alphaHideHue = 0.125f;
				ImGui::DragFloat( "Offset##HueSelector", &offset, 0.0f, -1.0f, 1.0f );
				ImGui::DragFloat( "Alpha Hue##HueSelector", &alphaHue, 0.0f, 0.0f, 1.0f );
				ImGui::DragFloat( "Alpha Hue Hide##HueSelector", &alphaHideHue, 0.0f, 0.0f, 1.0f );
				static float hueCenter = 0.5f;
				static float hueWidth = 0.1f;
				static float featherLeft = 0.125f;
				static float featherRight = 0.125f;
				ImGui::DragFloat( "Hue Width##HueSelector", &hueWidth, 0.0f, 0.0f, 0.5f );
				ImGui::DragFloat( "Feather Left##HueSelector", &featherLeft, 0.0f, 0.0f, 0.5f );
				ImGui::DragFloat( "Feather Right##HueSelector", &featherRight, 0.0f, 0.0f, 0.5f );
				static float hueHeight = 32.0f;
				static float cursorHeight = 8.0f;
				ImGui::DragFloat( "Hue Height##HueSelector", &hueHeight, 1.0f, 1.0f, 256.0f );
				ImGui::DragFloat( "Cursor Height##HueSelector", &cursorHeight, 1.0f, 1.0f, 64.0f );

				ImWidgets::GetStyle().PushVar( StyleVar_HueSelector_Thickness_ZeroWidth, 10.0f );
				HueSelector( "Hue##HueSelector", hueHeight, cursorHeight, &hueCenter, &hueWidth, &featherLeft, &featherRight, division, alphaHue, alphaHideHue, offset );
				ImWidgets::GetStyle().PopVar();
				HueSelector( "Hue##HueSelector", hueHeight, cursorHeight, &hueCenter, &hueWidth, &featherLeft, &featherRight, division, alphaHue, alphaHideHue, offset );
			}
			if ( ImGui::CollapsingHeader( "Slider2D Float" ) )
			{
				static ImVec2 slider2D;
				ImVec2 boundMin( -1.0f, -1.0f );
				ImVec2 boundMax( 1.0f, 1.0f );
				Slider2DFloat( "Slider 2D Float", &slider2D.x, &slider2D.y, boundMin.x, boundMax.x, boundMin.y, boundMax.y );
				ImGui::InputFloat2( "Value", &slider2D.x );
			}
			if ( ImGui::CollapsingHeader( "Slider2D Int" ) )
			{
				static int vv[ 2 ];
				Slider2DInt( "Slider 2D Int", &vv[ 0 ], &vv[ 1 ], -5, 5, -5, 5 );
				ImGui::InputInt2( "Value", &vv[ 0 ] );
			}
			ImGui::Unindent();
		}

		ImGui::End();
		ImGui::PopStyleVar();

		bool show = true;
		ImGui::ShowMetricsWindow( &show );
		ImGui::ShowDemoWindow( &show );
	}
}

#include <imgui/imgui_demo.cpp>
