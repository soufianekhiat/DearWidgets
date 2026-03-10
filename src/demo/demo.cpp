#include <demo.h>

#define IMGUI_DEFINE_MATH_OPERATORS

// Map project-specific graphics API defines to ImPlatform defines
#if defined(__DEAR_GFX_DX9__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_DIRECTX9
#elif defined(__DEAR_GFX_DX10__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_DIRECTX10
#elif defined(__DEAR_GFX_DX11__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_DIRECTX11
#elif defined(__DEAR_GFX_DX12__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_DIRECTX12
#elif defined(__DEAR_GFX_OGL3__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_OPENGL3
#elif defined(__DEAR_GFX_VULKAN__)
	#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
	#define IM_CURRENT_GFX IM_GFX_VULKAN
#else
	#error "No graphics API defined. Expected __DEAR_GFX_* define"
#endif

// Define Implementation and include ImPlatform
// Note: implatform_impl.cpp in API library provides this for API-only builds
// For full builds with demo, this provides the app functions the demo needs
#define IMPLATFORM_IMPLEMENTATION
#include <ImPlatform.h>

#include <dear_widgets.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>

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

ImTextureID TextureFromFile( char const* filename, ImVec2* img_size )
{
	int width;
	int height;
	ImTextureID img;

	stbi_uc* data = stbi_load( filename, &width, &height, NULL, 4 );
	if ( !data )
		return ImTextureID_Invalid;

	// Use new ImPlatform C API for texture creation
	ImPlatform_TextureDesc tex_desc = ImPlatform_TextureDesc_Default( width, height );
	img = ImPlatform_CreateTexture( data, &tex_desc );

	img_size->x = ( float )width;
	img_size->y = ( float )height;
	STBI_FREE( data );

	return img;
}

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

// ImVec4 operator* is now provided by IMGUI_DEFINE_MATH_OPERATORS
// static inline ImVec4 operator*( const ImVec4& lhs, const float rhs )
// {
// 	return ImVec4( lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs );
// }

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

//////////////////////////////////////////////////////////////////////////
// Demo Helper Structures
//////////////////////////////////////////////////////////////////////////
namespace LayoutConstants {
	constexpr float HALF = 0.5f;
	constexpr float LEFT_PANEL_RATIO = 2.0f / 6.0f;
	constexpr float RIGHT_PANEL_RATIO = 4.0f / 6.0f;
	constexpr float THREE_QUARTERS = 3.0f / 4.0f;
}

struct DemoColor {
	ImVec4 v;
	ImU32 u;

	DemoColor( float r, float g, float b, float a = 1.0f )
		: v( r, g, b, a ), u( ImGui::GetColorU32( v ) ) {}

	bool Edit( const char* label ) {
		if ( ImGui::ColorEdit4( label, &v.x ) ) {
			u = ImGui::GetColorU32( v );
			return true;
		}
		return false;
	}
};

struct GradientParams {
	ImVec2 uv_start;
	ImVec2 uv_end;
	DemoColor cola;
	DemoColor colb;

	GradientParams()
		: uv_start( 0.0f, 0.5f ), uv_end( 1.0f, 0.5f ),
		  cola( 1.0f, 0.0f, 0.0f ), colb( 0.0f, 1.0f, 0.0f ) {}

	void RenderControls( const char* suffix = "" ) {
		char label_a[ 64 ], label_b[ 64 ], label_uv0[ 64 ], label_uv1[ 64 ];
		snprintf( label_a, sizeof( label_a ), "ColA##%s", suffix );
		snprintf( label_b, sizeof( label_b ), "ColB##%s", suffix );
		snprintf( label_uv0, sizeof( label_uv0 ), "uv0##%s", suffix );
		snprintf( label_uv1, sizeof( label_uv1 ), "uv1##%s", suffix );

		ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
		ImWidgets::Slider2DFloat( label_uv0, &uv_start.x, &uv_start.y, 0.0f, 1.0f, 0.0f, 1.0f );
		ImGui::SameLine();
		ImWidgets::Slider2DFloat( label_uv1, &uv_end.x, &uv_end.y, 0.0f, 1.0f, 0.0f, 1.0f );
		ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
		cola.Edit( label_a );
		ImGui::SameLine();
		colb.Edit( label_b );
	}
};

struct ShapeDebugState {
	float edge_thickness;
	float vertex_radius;
	DemoColor edge_col;
	DemoColor triangle_col;
	DemoColor vertex_col;
	int tri_idx;
	int side_count;

	ShapeDebugState( int default_sides = 3 )
		: edge_thickness( 3.0f ), vertex_radius( 5.0f ),
		  edge_col( 0.0f, 0.0f, 1.0f ), triangle_col( 0.0f, 1.0f, 0.0f ),
		  vertex_col( 1.0f, 0.5f, 0.0f ), tri_idx( -1 ), side_count( default_sides ) {}

	void RenderControls( const char* suffix = "", int min_sides = 3, int max_sides = 64 ) {
		char label_sides[ 64 ], label_thick[ 64 ], label_radius[ 64 ];
		char label_edge[ 64 ], label_tri[ 64 ], label_vtx[ 64 ];
		snprintf( label_sides, sizeof( label_sides ), "Sides##%s", suffix );
		snprintf( label_thick, sizeof( label_thick ), "Thickness##%s", suffix );
		snprintf( label_radius, sizeof( label_radius ), "Radius##%s", suffix );
		snprintf( label_edge, sizeof( label_edge ), "Edge##%s", suffix );
		snprintf( label_tri, sizeof( label_tri ), "Triangle##%s", suffix );
		snprintf( label_vtx, sizeof( label_vtx ), "Vertices##%s", suffix );

		ImGui::SliderInt( label_sides, &side_count, min_sides, max_sides );
		ImGui::SliderFloat( label_thick, &edge_thickness, 0.0f, 16.0f );
		ImGui::SliderFloat( label_radius, &vertex_radius, 0.0f, 64.0f );
		edge_col.Edit( label_edge );
		triangle_col.Edit( label_tri );
		vertex_col.Edit( label_vtx );
	}
};

void ShowSampleOffscreen00();

ImTextureID background;
ImVec2 background_size;
ImTextureID illlustration_img;
ImVec2 illlustration_size;
ImTextureID bike_img;
ImVec2 bike_size;
ImTextureID astro_img;
ImVec2 astro_size;
ImTextureID clock_img;
ImVec2 clock_size;
ImTextureID man_img;
ImVec2 man_size;

static void OnDpiChanged( float new_scale, void* /*user_data*/ )
{
	ImGuiStyle& style = ImGui::GetStyle();
	style = ImGuiStyle();
	style.ScaleAllSizes( new_scale );
	style.FontScaleDpi = new_scale;
	ImWidgets::GetStyle() = ImWidgetsStyle();
	ImWidgets::GetStyle().ScaleAllSizes( new_scale );
}

int main()
{
	// Using the new ImPlatform C API - following ImPlatform demo pattern
	bool bGood;

	// Create window
	bGood = ImPlatform_CreateWindow( "Dear Widgets Demo", ImVec2( 100.0f, 100.0f ), 1024, 764 * 2 );
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot create window.\n" );
		return 1;
	}

	// Initialize Graphics API
	bGood = ImPlatform_InitGfxAPI();
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot initialize the Graphics API.\n" );
		return 1;
	}

	// Show window
	bGood = ImPlatform_ShowWindow();
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot show the window.\n" );
		return 1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	bGood = ImGui::CreateContext() != nullptr;
	if ( !bGood )
	{
		fprintf( stderr, "ImGui: Cannot create context.\n" );
		return 1;
	}

	// Setup Dear ImGui IO
	ImGuiIO& io = ImGui::GetIO(); ( void )io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;		// Enable Gamepad Controls
#ifdef IMGUI_HAS_DOCK
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;			// Enable Docking
#endif
#ifdef IMGUI_HAS_VIEWPORT
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;			// Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
#endif

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup DPI scaling (cross-platform)
	float dpi_scale = ImPlatform_GetDpiScale();

	// Load fonts (FontScaleDpi handles DPI scaling at render time)
	io.Fonts->AddFontFromFileTTF( "../extern/FiraCode/distr/ttf/FiraCode-Medium.ttf", 16.0f );

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes( dpi_scale );
	style.FontScaleDpi = dpi_scale;
	ImWidgets::GetStyle().ScaleAllSizes( dpi_scale );
#ifdef IMGUI_HAS_DOCK
	io.ConfigDpiScaleFonts = true;
#endif
#ifdef IMGUI_HAS_VIEWPORT
	io.ConfigDpiScaleViewports = true;
#endif

	// Register DPI change callback for runtime monitor changes
	ImPlatform_SetDpiChangeCallback( OnDpiChanged, nullptr );

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones
#ifdef IMGUI_HAS_VIEWPORT
	if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
	{
		style.WindowRounding = 0.0f;
		style.Colors[ ImGuiCol_WindowBg ].w = 1.0f;
	}
#endif

	// Initialize ImPlatform backends
	bGood = ImPlatform_InitPlatform();
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot initialize platform.\n" );
		return 1;
	}

	bGood = ImPlatform_InitGfx();
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot initialize graphics.\n" );
		return 1;
	}
	
	// Create ImWidgets context
	ImWidgets::AddFeatures( ImWidgetsFeatures_Markers );
	ImWidgetsContext* ctx = ImWidgets::CreateContext();

	// Load test images
	// Image from: https://www.pexels.com/fr-fr/photo/framboises-mures-dans-une-tasse-de-the-blanche-en-photographie-a-decalage-d-inclinaison-1152351/
	illlustration_img = TextureFromFile( "pexels-robert-bogdan-156165-1152351.jpg", &illlustration_size );
	// Image from: https://www.pexels.com/fr-fr/photo/deux-chaises-avec-table-en-verre-sur-le-salon-pres-de-la-fenetre-1571453/
	background = TextureFromFile( "pexels-fotoaibe-1571453.jpg", &background_size );
	bike_img = TextureFromFile( "camera-542784_1280.png", &bike_size );
	astro_img = TextureFromFile( "astro.png", &astro_size );
	clock_img = TextureFromFile( "clock.png", &clock_size );
	man_img = TextureFromFile( "man.png", &man_size );

	ImWidgets::OwnTexture( illlustration_img );
	ImWidgets::OwnTexture( background );
	ImWidgets::OwnTexture( bike_img );
	ImWidgets::OwnTexture( astro_img );
	ImWidgets::OwnTexture( clock_img );
	ImWidgets::OwnTexture( man_img );

	ImVec4 clear_color = ImVec4( 0.461f, 0.461f, 0.461f, 1.0f );
	while ( ImPlatform_PlatformContinue() )
	{
		ImPlatform_PlatformEvents();

		if ( !ImPlatform_GfxCheck() )
		{
			continue;
		}

		// New frame
		ImPlatform_GfxAPINewFrame();
		ImPlatform_PlatformNewFrame();
		ImGui::NewFrame();

		// Render UI
		ImWidgets::ShowSamples();
		ImWidgets::ShowDemo();
		ImGui::ShowMetricsWindow();
		ImGui::ShowDemoWindow();

		ShowSampleOffscreen00();

		// Rendering
		ImGui::Render();
		ImPlatform_GfxAPIClear( clear_color );
		ImPlatform_GfxAPIRender( clear_color );

#ifdef IMGUI_HAS_VIEWPORT
		// Update and Render additional Platform Windows
		if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			ImPlatform_GfxViewportPre();
			ImPlatform_GfxViewportPost();
		}
#endif

		ImPlatform_GfxAPISwapBuffer();
	}

	// Cleanup
	ImWidgets::DestroyContext( ctx );

	ImPlatform_ShutdownGfxAPI();
	ImPlatform_ShutdownWindow();
	ImPlatform_ShutdownPostGfxAPI();

	ImGui::DestroyContext();

	ImPlatform_DestroyWindow();

	return 0;
}

void ShowSampleOffscreen00()
{
	return;

	ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 8 );

	ImGui::Begin( "Off Screen 00" );

	ImDrawList* draw = ImGui::GetWindowDrawList();

	ImVec2 cur = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetContentRegionAvail();

	//draw->AddImageRounded( illlustration_img,
	//					   cur + ImVec2( 0.0f - 50.0f, 0.0f ),
	//					   cur + ImVec2( 0.0f + 50.0f, 50.0f ),
	//					   ImVec2(0, 0), ImVec2(1, 1), IM_COL32(255, 255, 255, 255), 8);

#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
	// DrawMarker now fully functional with new ImPlatform shader API
	ImWidgets::DrawMarker( draw, cur, size, IM_COL32_WHITE, IM_COL32_BLACK_TRANS, 0.0f, 1.0f, 10.0f, 0.5f, ImWidgetsMarker_Disc, ImWidgetsDrawType_Outline );
#endif
	ImGui::Dummy( size );

	ImGui::End();

	ImGui::PopStyleVar( 1 );
}

namespace ImWidgets {

	static void AspectRatio_6_2( ImGuiSizeCallbackData* data )
	{
		float aspect_ratio = *( float* )data->UserData;
		data->DesiredSize.y = ( float )( int )( data->DesiredSize.x / aspect_ratio );
	}

	//////////////////////////////////////////////////////////////////////////
	// ShowSamples Layout Helper Functions
	//////////////////////////////////////////////////////////////////////////
	void RenderLeftLeftPanel()
	{
		ImVec2 size = ImGui::GetContentRegionAvail();

		// Top section with clock image button
		ImGui::BeginChild( "Left_Left_Top", ImVec2( 0.0f, size.y * LayoutConstants::HALF ) );
		{
			size = ImGui::GetContentRegionAvail();
			ImWidgets::ImageButtonExCapsuleH( "X", clock_img, size.y, size.y,
				ImGuiButtonFlags_None, IM_COL32_WHITE,
				ImVec2( 0.16f, 0.16f ), ImVec2( 0.84f, 0.84f ) );
		}
		ImGui::EndChild();

		// Bottom section with circle button
		ImGui::BeginChild( "Left_Left_Bottom" );
		{
			size = ImGui::GetContentRegionAvail();
			ImWidgets::ButtonExCircle( "Y", size.y * LayoutConstants::HALF, 0 );
		}
		ImGui::EndChild();
	}

	void RenderLeftPanel()
	{
		ImVec2 size = ImGui::GetContentRegionAvail();

		// Left half of left panel
		ImGui::BeginChild( "Left_Left", ImVec2( size.x * LayoutConstants::HALF, 0.0f ) );
		RenderLeftLeftPanel();
		ImGui::EndChild();

		ImGui::SameLine();

		// Right half of left panel with vertical capsule
		ImGui::BeginChild( "Left_Right" );
		{
			size = ImGui::GetContentRegionAvail();
			ImWidgets::ImageButtonExCapsuleV( "Y", man_img, size.y, size.x, 0 );
		}
		ImGui::EndChild();
	}

	void RenderRightTopPanel()
	{
		ImVec2 size = ImGui::GetContentRegionAvail();

		// Left side with astronaut image
		ImGui::BeginChild( "Right_Top_Left", ImVec2( size.x * LayoutConstants::THREE_QUARTERS, 0.0f ) );
		{
			size = ImGui::GetContentRegionAvail();
			ImWidgets::ImageButtonExCapsuleH( "X", astro_img, size.x, size.y,
				ImGuiButtonFlags_None, IM_COL32_WHITE,
				ImVec2( 0.16f, 0.16f ), ImVec2( 0.84f, 0.84f ) );
		}
		ImGui::EndChild();

		ImGui::SameLine();

		// Right side with circle button
		ImGui::BeginChild( "Right_Top_Right" );
		{
			size = ImGui::GetContentRegionAvail();
			ImWidgets::ButtonExCircle( "Y", size.y * LayoutConstants::HALF, 0 );
		}
		ImGui::EndChild();
	}

	void RenderRightBottomPanel()
	{
		ImVec2 size = ImGui::GetContentRegionAvail();
		ImWidgets::ButtonExCircle( "A", size.y * LayoutConstants::HALF, 0 );
		ImGui::SameLine();
		ImWidgets::ButtonExCircle( "B", size.y * LayoutConstants::HALF, 0 );
		ImGui::SameLine();
		size = ImGui::GetContentRegionAvail();
		ImWidgets::ButtonExCapsuleH( "C", size.x, size.y, 0 );
	}

	void RenderRightPanel()
	{
		ImVec2 size = ImGui::GetContentRegionAvail();

		// Top half
		ImGui::BeginChild( "Right_Top", ImVec2( 0.0f, size.y * LayoutConstants::HALF ) );
		RenderRightTopPanel();
		ImGui::EndChild();

		// Bottom half
		ImGui::BeginChild( "Right_Bottom" );
		RenderRightBottomPanel();
		ImGui::EndChild();
	}

	void ShowSamples()
	{
		static const float _6_2 = 6.0f / 2.0f;
		ImGui::SetNextWindowSizeConstraints( ImVec2( 0, 0 ), ImVec2( FLT_MAX, FLT_MAX ), AspectRatio_6_2, ( void* )&_6_2 );

		if ( ImGui::Begin( "Shop 00", 0, ImGuiWindowFlags_NoTitleBar ) )
		{
			ImVec2 size = ImGui::GetContentRegionAvail();

			// Left panel (2/6 of width)
			ImGui::BeginChild( "Left", ImVec2( size.x * LayoutConstants::LEFT_PANEL_RATIO, 0.0f ) );
			RenderLeftPanel();
			ImGui::EndChild();

			ImGui::SameLine();

			// Right panel (4/6 of width)
			ImGui::BeginChild( "Right" );
			RenderRightPanel();
			ImGui::EndChild();
		}
		ImGui::End();
	}

	//////////////////////////////////////////////////////////////////////////
	// ShowDemo Section Functions
	//////////////////////////////////////////////////////////////////////////
	void ShowDrawShapeDemo()
	{
		if ( !ImGui::CollapsingHeader( "Draw Shape" ) )
			return;

		float const size = ImGui::GetContentRegionAvail().x;
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();

		static ShapeDebugState debug_state;
		static GradientParams gradient;
		static ImWidgetsShape shape;
#ifdef DEAR_WIDGETS_TESSELATION
		static int tess = 2;
		ImGui::SliderInt( "Tess", &tess, 0, 16 );
#endif

		// Render debug controls
		debug_state.RenderControls( "DrawShape" );

		// Render gradient controls
		gradient.RenderControls( "DrawShape" );

		// Generate and render shape
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImWidgets::GenShapeCircle( shape, pos + ImVec2( size * LayoutConstants::HALF, size * LayoutConstants::HALF ),
			size * LayoutConstants::HALF, debug_state.side_count );
		ImWidgets::ShapeSetDefaultUV( shape );
#ifdef DEAR_WIDGETS_TESSELATION
		for ( int k = 0; k < tess; ++k )
			ImWidgets::ShapeTesselationUniform( shape );
#endif
		ImWidgets::ShapeSRGBLinearGradient( shape, gradient.uv_start, gradient.uv_end,
			gradient.cola.u, gradient.colb.u );
		ImWidgets::DrawShapeDebug( pDrawList, shape, debug_state.edge_thickness,
			debug_state.edge_col.u, debug_state.triangle_col.u,
			debug_state.vertex_radius, debug_state.vertex_col.u, debug_state.tri_idx );

		ImGui::Dummy( ImVec2( size, size ) );
		ImGui::SliderInt( "tri_idx", &debug_state.tri_idx, -1, shape.triangles.size() - 1 );
		ImGui::Text( "Tri: %d", shape.triangles.size() );
		ImGui::Text( "Vtx: %d", shape.vertices.size() );
	}

	void ShowCustomShaderDemo()
	{
		if ( !ImGui::CollapsingHeader( "Custom Shader" ) )
			return;

		float const size = ImGui::GetContentRegionAvail().x;

		static float shape_size = 1.0f;
		static float line_width = 0.05f;
		static float angle = 0.0f;
		static float antialiasing = 0.001f;
		static DemoColor fg_color( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f );
		static DemoColor bg_color( 1.0f, 128.0f / 255.0f, 64.0f / 255.0f );

		ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
		fg_color.Edit( "ColA##CustomShader" );
		ImGui::SameLine();
		bg_color.Edit( "ColB##CustomShader" );
		ImGui::DragFloat( "shape_size", &shape_size, 0.25f, 0.0f, 1.0f );
		ImGui::DragFloat( "line_width", &line_width, 0.0125f, 0.0f, 0.1f );
		ImGui::DragFloat( "antialiasing", &antialiasing, 0.0125f, 0.0f, 16.0f );
		ImGui::SliderAngle( "angle", &angle );

		static int marker_idx = ( int )ImWidgetsMarker_Pin;
		static const char* markers[] = {
			"Disc", "Square", "Triangle", "Diamond", "Heart",
			"Spade", "Club", "Chevron", "Clover", "Ring",
			"Tag", "Cross", "Asterisk", "Infinity", "Pin",
			"Arrow", "Ellipse", "EllipseApprox"
		};
		ImGui::Combo( "Marker", &marker_idx, markers, ImWidgetsMarker_COUNT );

		static int draw_type_idx = ( int )ImWidgetsDrawType_Outline;
		static const char* draw_types[] = {
			"Filled", "Stroke", "Outline", "Signed Distance Field", "Cut Off"
		};
		ImGui::Combo( "Draw Type", &draw_type_idx, draw_types, ImWidgetsDrawType_COUNT );

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImWidgets::DrawMarker( pDrawList, pos, ImVec2( size, size ),
			fg_color.u, bg_color.u, angle, shape_size, line_width, antialiasing,
			( ImWidgetsMarker )marker_idx, ( ImWidgetsDrawType )draw_type_idx );
		ImGui::Dummy( ImVec2( size, size ) );
	}

	void ShowDrawSquircleDemo()
	{
		if ( !ImGui::CollapsingHeader( "Draw Squircle" ) )
			return;

		float const size = ImGui::GetContentRegionAvail().x;
		ImDrawList* pDrawList = ImGui::GetWindowDrawList();

		static ShapeDebugState debug_state( 32 );
		static GradientParams gradient;
		static ImWidgetsShape shape;
		static float squircle_n = 4.0f;
#ifdef DEAR_WIDGETS_TESSELATION
		static int tess = 1;
		ImGui::SliderInt( "Tess", &tess, 0, 16 );
#endif

		// Render debug controls
		debug_state.RenderControls( "DrawSquircle", 8, 128 );
		ImGui::SliderFloat( "Squircle n", &squircle_n, 2.0f, 10.0f );

		// Render gradient controls
		gradient.RenderControls( "DrawSquircle" );

		// Generate and render shape
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImWidgets::GenShapeSquircle( shape, pos + ImVec2( size * LayoutConstants::HALF, size * LayoutConstants::HALF ),
			size * 0.4f, debug_state.side_count, squircle_n );
		ImWidgets::ShapeSetDefaultUV( shape );
#ifdef DEAR_WIDGETS_TESSELATION
		for ( int k = 0; k < tess; ++k )
			ImWidgets::ShapeTesselationUniform( shape );
#endif
		ImWidgets::ShapeSRGBLinearGradient( shape, gradient.uv_start, gradient.uv_end,
			gradient.cola.u, gradient.colb.u );
		ImWidgets::DrawShapeDebug( pDrawList, shape, debug_state.edge_thickness,
			debug_state.edge_col.u, debug_state.triangle_col.u,
			debug_state.vertex_radius, debug_state.vertex_col.u, debug_state.tri_idx );

		ImGui::Dummy( ImVec2( size, size ) );
		ImGui::SliderInt( "tri_idx", &debug_state.tri_idx, -1, shape.triangles.size() - 1 );
		ImGui::Text( "Tri: %d", shape.triangles.size() );
		ImGui::Text( "Vtx: %d", shape.vertices.size() );
	}

	void	ShowDemo()
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::SetNextWindowBgAlpha( 0.75f );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 16 );
		ImGui::Begin( "Dear Widgets", NULL, ImGuiWindowFlags_NoTitleBar );
		ImWidgets::SetCurrentWindowBackgroundImage( background, background_size, false, IM_COL32(255, 255, 255, 128) );

		if ( ImGui::CollapsingHeader( "Draw" ) )
		{
			ImGui::Indent();
			ShowDrawShapeDemo();
#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
			ShowCustomShaderDemo();
			if ( ImGui::CollapsingHeader( "Thick line", ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				float const size = ImGui::GetContentRegionAvail().x;
				ImGui::Dummy( ImVec2( size, 0.25f * size ) );
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();

				static float line_width = 5.0f;
				static float mitter_limit = 0.0f;
				static float antialiasing = 1.0f / size;

				ImWidgetsStyle& widgetStyle = ImWidgets::GetStyle();
				ImVec4 vBlue = widgetStyle.Colors[ StyleColor_Slider2D_CursorX ];
				ImVec4 vOrange = widgetStyle.Colors[ StyleColor_Slider2D_CursorY ];
				ImU32 uBlue = ImGui::GetColorU32( vBlue );
				ImU32 uOrange = ImGui::GetColorU32( vOrange );
				static ImVec4 color_v( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 1.0f );
				static ImU32 color_col = ImGui::GetColorU32( color_v );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &color_v.x ) )
					color_col = ImGui::GetColorU32( color_v );
				ImGui::DragFloat( "line_width", &line_width, 0.0125f, 0.0f, 16.0f );
				ImGui::DragFloat( "antialiasing", &antialiasing, 0.0125f, 0.0f, 16.0f );
				ImGui::SliderAngle( "mitter_limit", &mitter_limit );

				ImVec2 pos = ImGui::GetCursorScreenPos();

				ImVec2 pts[] = {
					pos + ImVec2( size * 0.25f, size * 0.25f ),
					pos + ImVec2( size * 0.72f, size * 0.25f ),
					pos + ImVec2( size * 0.72f, size * 0.75f )
				};

				//pDrawList->AddLine( pts[ 0 ], pts[ 1 ], color_col, line_width );

				ImGui::Dummy( ImVec2( size, size ) );
			}
#endif
			ShowDrawSquircleDemo();
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
				static ImWidgetsShape shape;
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
				static ImWidgetsShape shape;
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
				static ImWidgetsShape shape;
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
				static ImWidgetsShape shape;
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
				static ImVec2 uv_start( 0.0f, 0.5f );
				static ImVec2 uv_end( 0.0f, 0.75f );
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
				static ImWidgetsShape shape;
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
				const float S = ImPlatform_GetDpiScale();
				float const width = ImGui::GetContentRegionAvail().x;

				static float angle = 3.1415926535987932f;
				static float size = 64.0f;
				static float thickness = 5.0f;
				ImGui::SliderAngle( "Angle##Triangle", &angle );
				ImGui::SliderFloat( "Size##Triangle", &size, 1.0f, 64.0f );
				ImGui::SliderFloat( "Thickness##Triangle", &thickness, 1.0f, 5.0f );

				ImVec2 curPos = ImGui::GetCursorScreenPos();
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				float dx = 32.0f * S;
				ImGui::InvisibleButton( "##Zone0", ImVec2( width, 96.0f * S ), 0 );
				ImGui::InvisibleButton( "##Zone1", ImVec2( width, 96.0f * S ), 0 );
				float fPointerLine = 64.0f * S;
				pDrawList->AddLine( ImVec2( curPos.x + 0.5f * dx, curPos.y + fPointerLine ), ImVec2( curPos.x + 3.5f * dx, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f * S );
				pDrawList->AddLine( ImVec2( curPos.x + 5.0f * dx, curPos.y ), ImVec2( curPos.x + 5.0f * dx, curPos.y + 72.0f * S ), IM_COL32( 0, 255, 0, 255 ), 2.0f * S );
				pDrawList->AddLine( ImVec2( curPos.x + 7.0f * dx, curPos.y ), ImVec2( curPos.x + 7.0f * dx, curPos.y + 72.0f * S ), IM_COL32( 0, 255, 0, 255 ), 2.0f * S );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 1.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 3.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 5.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 7.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawTriangleCursor( pDrawList, ImVec2( curPos.x + 1.0f * dx, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursor( pDrawList, ImVec2( curPos.x + 3.0f * dx, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursor( pDrawList, ImVec2( curPos.x + 5.0f * dx, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursor( pDrawList, ImVec2( curPos.x + 7.0f * dx, curPos.y + fPointerLine ), angle, size, thickness, IM_COL32( 255, 0, 0, 255 ) );

				fPointerLine *= 3.0f;
				pDrawList->AddLine( ImVec2( curPos.x + 0.5f * dx, curPos.y + fPointerLine ), ImVec2( curPos.x + 3.5f * dx, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f * S );
				pDrawList->AddLine( ImVec2( curPos.x + 5.0f * dx, curPos.y ), ImVec2( curPos.x + 5.0f * dx, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f * S );
				pDrawList->AddLine( ImVec2( curPos.x + 7.0f * dx, curPos.y ), ImVec2( curPos.x + 7.0f * dx, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f * S );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 1.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 3.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 5.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 7.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawTriangleCursorFilled( pDrawList, ImVec2( curPos.x + 1.0f * dx, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursorFilled( pDrawList, ImVec2( curPos.x + 3.0f * dx, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursorFilled( pDrawList, ImVec2( curPos.x + 5.0f * dx, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
				ImWidgets::DrawTriangleCursorFilled( pDrawList, ImVec2( curPos.x + 7.0f * dx, curPos.y + fPointerLine ), angle, size, IM_COL32( 255, 0, 0, 255 ) );
			}
			if ( ImGui::CollapsingHeader( "Signet Pointer" ) )
			{
				const float S = ImPlatform_GetDpiScale();
				float const widthZone = ImGui::GetContentRegionAvail().x;

				static float angle = 0.0f;
				static float width = 32.0f;
				static float height = 64.0f;
				static float height_ratio = 0.25f;
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
				ImGui::InvisibleButton( "##Zone00", ImVec2( widthZone, height * 1.1f ), 0 );
				ImGui::InvisibleButton( "##Zone01", ImVec2( widthZone, height * 1.1f ), 0 );
				float fPointerLine = 32.0f * S;
				float dx = 16.0f * S;
				pDrawList->AddLine( ImVec2( curPos.x + 0.5f * dx, curPos.y + fPointerLine ), ImVec2( curPos.x + 11.5f * dx, curPos.y + fPointerLine ), IM_COL32( 0, 255, 0, 255 ), 2.0f * S );
				ImVec4 vBlue( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 1.0f );
				ImU32 uBlue = ImGui::GetColorU32( vBlue );
				ImWidgets::DrawSignetCursor( pDrawList, ImVec2( curPos.x + 1.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, align01, angle, thickness, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 1.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetFilledCursor( pDrawList, ImVec2( curPos.x + 3.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, align01, angle, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 3.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetCursor( pDrawList, ImVec2( curPos.x + 5.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, 0.0f, angle, thickness, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 5.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetFilledCursor( pDrawList, ImVec2( curPos.x + 7.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, 0.0f, angle, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 7.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetCursor( pDrawList, ImVec2( curPos.x + 9.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, 1.0f, angle, thickness, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 9.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
				ImWidgets::DrawSignetFilledCursor( pDrawList, ImVec2( curPos.x + 11.0f * dx, curPos.y + fPointerLine ), width, height, height_ratio, 1.0f, angle, uBlue );
				pDrawList->AddCircleFilled( ImVec2( curPos.x + 11.0f * dx, curPos.y + fPointerLine ), 4.0f * S, IM_COL32( 255, 128, 0, 255 ), 16 );
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
					ImGui::InvisibleButton( "##ZoneColorRing0", ImVec2( width, width ), 0 );

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
					ImGui::InvisibleButton( "##ZoneColorRing1", ImVec2( width, width ) * 0.5f, 0 );

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
					ImGui::InvisibleButton( "##ZoneColorRing2", ImVec2( width, width ) * 0.5f, 0 );

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
				ImWidgetsColor2DCallback func = []( float x, float y, void* pUserData ) -> ImU32{
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
				ImPolyShapeData data = { &pos_norms[ 0 ], 3 };
				bool hovered = IsMouseHovering( pos, pos + ImVec2( size, size ), Im_IsPolyConvexContains, &data );
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
				data = { &disk[ 0 ], 32 };
				hovered = IsMouseHovering( pos, pos + ImVec2( size, size ), Im_IsPolyConvexContains, &data );
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
				ImPolyShapeData data = { &pos_norms[ 0 ], sz };
				bool hovered = IsMouseHovering( pos * 0.99f, pos + ImVec2( 1.01f * size, 1.01f * size ), Im_IsPolyConcaveContains, &data );
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
				data = { &ring[ 0 ], sz };
				hovered = IsMouseHovering( pos * 0.99f, pos + ImVec2( 1.01f * size, 1.01f * size ), Im_IsPolyConcaveContains, &data );
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
				ImPolyHoleShapeData data = { &pos_norms[ 0 ], NULL, sz, 1, 1 };
				bool hovered = IsMouseHovering( pos * 0.99f, pos + ImVec2( 1.01f * size, 1.01f * size ), Im_IsPolyWithHoleContains, &data );
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
				data = { &ring[ 0 ], NULL, sz, 1, 1 };
				hovered = IsMouseHovering( pos, pos + ImVec2( size, size ), Im_IsPolyWithHoleContains, &data );
				DrawShapeWithHole( pDrawList, &ring[ 0 ], sz, IM_COL32( hovered ? 255 : 0, hovered ? 0 : 255, 0, 255 ) );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			ImGui::Unindent();
		}
		if ( ImGui::CollapsingHeader( "Widgets", ImGuiTreeNodeFlags_DefaultOpen ) )
		{
			ImGui::Indent();
			if ( ImGui::CollapsingHeader( "Button Circle" ) )
			{
				float const half_size = 0.5f * ImGui::GetContentRegionAvail().x;
				static int value = 0;
				static float radius = half_size;
				static std::string caption = "Circle";
				ImGui::InputText( "value", &caption );
				ImGui::DragFloat( "radius", &radius, 1.0f, 0.0f, 2.0f * half_size );
				ImGui::Text( "Value: %d", value );
				value += ( int )ImWidgets::ButtonExCircle( caption.c_str(), radius, 0 );
			}
			if ( ImGui::CollapsingHeader( "Button Capsule" ) )
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				float const size = ImGui::GetContentRegionAvail().x;
				static int value = 0;
				static float length = size;
				static float thickness = size * 0.25f;
				ImGui::DragFloat( "length", &length, 1.0f, 0.0f, 2.0f * size );
				ImGui::DragFloat( "thickness", &thickness, 1.0f, 0.0f, 2.0f * size );
				ImGui::Text( "Value: %d", value );
				value += ( int )ButtonExCapsuleH( "CapsuleH", length, thickness, 0 );
				value += ( int )ButtonExCapsuleV( "CapsuleV", length, thickness, 0 );
			}
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
				static int value = 0;
				ImGui::Text( "Value: %d", value );
				value += ( int )ImWidgets::ButtonExConvex( "Convex", ImVec2( 0, 0 ), &disk[ 0 ], 32, 0 );
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
				static int value = 0;
				ImGui::Text( "Value: %d", value );
				value += ( int )ImWidgets::ButtonExConcave( "Concave", ImVec2( 0, 0 ), &pos_norms[ 0 ], sz, ImVec2( 0.0f, size / 3.0f ), 0 );
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
				static int value = 0;
				ImGui::Text( "Value: %d", value );
				value += ( int )ImWidgets::ButtonExWithHole( "With Hole", ImVec2( 0, 0 ), &pos_norms[ 0 ], sz, ImVec2( 0.0f, size / 3.0f ), 0 );
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
				ImGui::Text( "Hover per region of influence" );
				ImWidgets::SliderNScalar( "Values##SliderNRegions", ImGuiDataType_Float, &value, 3, &min, &max, 8.0f, true );
				ImGui::Text( "Global Hover" );
				ImWidgets::SliderNScalar( "Values##SliderNGlobal", ImGuiDataType_Float, &value, 3, &min, &max, 8.0f, false );
				ImGui::DragFloat( "Near Plane", &value[ 0 ], 1.0f, min, value[ 1 ] );
				ImGui::DragFloat( "Focal Planes", &value[ 1 ], 1.0f, value[ 0 ], value[ 2 ] );
				ImGui::DragFloat( "Far Planes", &value[ 2 ], 1.0f, value[ 1 ], max );
			}
#if 1
			if ( ImGui::CollapsingHeader( "Dashed Polylines", ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				ImDrawList* dl = ImGui::GetWindowDrawList();
				float avail = ImGui::GetContentRegionAvail().x;
				float height = 220.0f;
				ImVec2 origin = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton("##zone_dashed_poly", ImVec2(avail, height));

				static float thickness = 6.0f;
				static float dash_len = 24.0f;
				static float gap_len  = 12.0f;
				static float offset   = 0.0f;
				static bool  animate   = false;
				static bool  closed    = false;
				static int   cap_idx   = (int)ImWidgetsCap_Butt;
				static int   join_idx  = (int)ImWidgetsJoin_Mitter;
				static float miter_limit = 4.0f;
				static int   path_type = 1; // 0=ZigZag, 1=Sine, 2=Spiral, 3=RoundedRect, 4=Circle, 5=Infinity, 6=Rose, 7=Heart, 8=Sawtooth, 9=ArcChain, 10=Star, 11=BezierS
				const char* caps[] = { "None", "Butt", "Square", "Round", "TriangleOut", "TriangleIn" };
				const char* joins[] = { "Round", "Mitter", "Bevel" };
				const char* paths[] = { "ZigZag", "Sine", "Spiral", "RoundedRect", "Circle", "Infinity", "Rose (k=5)", "Heart", "Sawtooth", "Arc Chain", "Star", "Bezier S" };
				ImGui::SetCursorScreenPos(origin + ImVec2(8, 6));
				dl->AddRect(origin, origin + ImVec2(avail, height), IM_COL32(64,64,64,255));

				// Build path
				ImVec2 pts_stack[256];
				ImVec2* pts = pts_stack;
				int pts_count = 0;
				float left = origin.x + 16.0f;
				float right = origin.x + avail - 16.0f;
				float top = origin.y + 24.0f;
				float bottom = origin.y + height - 24.0f;
				float midx = (left + right) * 0.5f;
				if (path_type == 0)
				{
					pts_stack[0] = ImVec2(left, top);
					pts_stack[1] = ImVec2(midx, bottom);
					pts_stack[2] = ImVec2(right, top);
					pts_stack[3] = ImVec2(midx, top + (bottom-top)*0.5f);
					pts_stack[4] = ImVec2(left, bottom);
					pts_stack[5] = ImVec2(midx, top + (bottom-top)*0.25f);
					pts_stack[6] = ImVec2(right, bottom);
					pts_count = 7;
				}
				else if (path_type == 1)
				{
					// Sine path across the rect
					int N = 64;
					for (int i = 0; i < N; ++i)
					{
						float t = (float)i / (float)(N - 1);
						float x = ImLerp(left, right, t);
						float y = ImLerp(top + (bottom-top)*0.2f, bottom - (bottom-top)*0.2f, 0.5f + 0.4f * sinf(t * 4.0f * IM_PI));
						pts_stack[i] = ImVec2(x, y);
					}
					pts_count = N;
				}
				else if (path_type == 2)
				{
					// Spiral path centered in the rect
					int N = 96;
					ImVec2 center = ImVec2((left + right) * 0.5f, (top + bottom) * 0.5f);
					float rx = (right - left) * 0.45f;
					float ry = (bottom - top) * 0.45f;
					for (int i = 0; i < N; ++i)
					{
						float t = (float)i / (float)(N - 1);
						float ang = t * 4.0f * IM_PI;
						float r = 0.1f + 0.9f * t; // from center outward
						float x = center.x + cosf(ang) * rx * r;
						float y = center.y + sinf(ang) * ry * r;
						pts_stack[i] = ImVec2(x, y);
					}
					pts_count = N;
				}
				else if (path_type == 3)
				{
					// Rounded rectangle inside the zone
					float pad = 28.0f;
					ImVec2 pmin(left + pad, top + pad);
					ImVec2 pmax(right - pad, bottom - pad);
					float rx = (pmax.x - pmin.x) * 0.18f;
					float ry = (pmax.y - pmin.y) * 0.18f;
					int seg = 12;
					int idx = 0;
					// Top-right corner arc
					for (int i = 0; i <= seg; ++i) { float a = IM_PI * 1.5f + (float)i/seg * IM_PI*0.5f; pts_stack[idx++] = ImVec2(pmax.x - rx + cosf(a)*rx, pmin.y + ry + sinf(a)*ry); }
					// Bottom-right
					for (int i = 0; i <= seg; ++i) { float a = 0.0f + (float)i/seg * IM_PI*0.5f;  pts_stack[idx++] = ImVec2(pmax.x - rx + cosf(a)*rx, pmax.y - ry + sinf(a)*ry); }
					// Bottom-left
					for (int i = 0; i <= seg; ++i) { float a = IM_PI*0.5f + (float)i/seg * IM_PI*0.5f; pts_stack[idx++] = ImVec2(pmin.x + rx + cosf(a)*rx, pmax.y - ry + sinf(a)*ry); }
					// Top-left
					for (int i = 0; i <= seg; ++i) { float a = IM_PI + (float)i/seg * IM_PI*0.5f;   pts_stack[idx++] = ImVec2(pmin.x + rx + cosf(a)*rx, pmin.y + ry + sinf(a)*ry); }
					pts_count = idx;
				}
				else if (path_type == 4)
				{
					// Circle
					ImVec2 c((left+right)*0.5f, (top+bottom)*0.5f);
					float r = ImMin((right-left), (bottom-top)) * 0.35f;
					int N = 128;
					for (int i = 0; i < N; ++i)
					{
						float a = (2.0f*IM_PI) * (float)i / (float)N;
						pts_stack[i] = ImVec2(c.x + cosf(a)*r, c.y + sinf(a)*r);
					}
					pts_count = N;
				}
				else if (path_type == 5)
				{
					// Infinity (lemniscate of Gerono)
					ImVec2 c((left+right)*0.5f, (top+bottom)*0.5f);
					float sx = (right-left)*0.35f, sy = (bottom-top)*0.25f;
					int N = 140;
					for (int i = 0; i < N; ++i)
					{
						float t = (2.0f*IM_PI) * (float)i / (float)(N-1);
						float x = cosf(t);
						float y = sinf(t) * cosf(t);
						pts_stack[i] = ImVec2(c.x + x*sx, c.y + y*sy);
					}
					pts_count = N;
				}
				else if (path_type == 6)
				{
					// Rose curve r = a*cos(kθ) with k=5
					ImVec2 c((left+right)*0.5f, (top+bottom)*0.5f);
					float a = ImMin((right-left), (bottom-top))*0.35f;
					int N = 220; int k = 5;
					for (int i = 0; i < N; ++i)
					{
						float th = (2.0f*IM_PI) * (float)i / (float)(N-1);
						float r = a * cosf(k*th);
						pts_stack[i] = ImVec2(c.x + r*cosf(th), c.y + r*sinf(th));
					}
					pts_count = N;
				}
				else if (path_type == 7)
				{
					// Heart curve (scaled)
					ImVec2 c((left+right)*0.5f, (top+bottom)*0.5f);
					float s = ImMin((right-left), (bottom-top))*0.035f;
					int N = 160; int idx = 0;
					for (int i = 0; i < N; ++i)
					{
						float t = (2.0f*IM_PI) * (float)i / (float)(N-1);
						float x = 16.0f*s*sinf(t)*sinf(t)*sinf(t);
						float y = - (13.0f*cosf(t) - 5.0f*cosf(2*t) - 2.0f*cosf(3*t) - cosf(4*t)) * s;
						pts_stack[idx++] = ImVec2(c.x + x, c.y + y);
					}
					pts_count = N;
				}
				else if (path_type == 8)
				{
					// Sawtooth across the rect
					int teeth = 12; int idx = 0;
					float w = (right-left);
					float h0 = top + (bottom-top)*0.25f;
					float h1 = bottom - (bottom-top)*0.25f;
					for (int i = 0; i <= teeth; ++i)
					{
						float x = ImLerp(left, right, (float)i/(float)teeth);
						float y = (i%2)==0 ? h0 : h1;
						pts_stack[idx++] = ImVec2(x, y);
					}
					pts_count = (teeth+1);
				}
				else if (path_type == 9)
				{
					// Arc chain: alternating up/down semicircles
					int arcs = 6; int seg = 18; int idx = 0;
					float span = (right-left) / arcs;
					float cy = (top+bottom)*0.5f;
					float r = (bottom-top)*0.22f;
					for (int a = 0; a < arcs; ++a)
					{
						float cx = left + span*(a+0.5f);
						bool up = (a%2)==0;
						// Up arc should go from 0..pi (bulge up), down arc pi..2pi
						float start = up ? 0.0f : IM_PI;
						float end   = up ? IM_PI : 2.0f*IM_PI;
						for (int i = 0; i <= seg; ++i)
						{
							float t = (float)i/(float)seg;
							float ang = ImLerp(start, end, t);
							// Skip duplicate vertex between consecutive arcs
							if (i==0 && a>0) continue;
							pts_stack[idx++] = ImVec2(cx + cosf(ang)*r, cy + sinf(ang)*r);
						}
					}
					pts_count = idx;
				}
				else if (path_type == 10)
				{
					// 5-point star
					ImVec2 c((left+right)*0.5f, (top+bottom)*0.5f);
					float R = ImMin((right-left), (bottom-top))*0.42f;
					float r = R*0.45f; int idx = 0;
					for (int i = 0; i < 10; ++i)
					{
						float ang = -IM_PI*0.5f + (float)i * (IM_PI/5.0f);
						float rad = (i%2)==0 ? R : r;
						pts_stack[idx++] = ImVec2(c.x + cosf(ang)*rad, c.y + sinf(ang)*rad);
					}
					pts_count = 10;
				}
				else if (path_type == 11)
				{
					// Bezier S (two cubic segments)
					ImVec2 p0(left, (top+bottom)*0.5f);
					ImVec2 p1(left + (right-left)*0.25f, top);
					ImVec2 p2(left + (right-left)*0.25f, bottom);
					ImVec2 p3(left + (right-left)*0.5f, (top+bottom)*0.5f);
					ImVec2 q0 = p3;
					ImVec2 q1(left + (right-left)*0.75f, bottom);
					ImVec2 q2(left + (right-left)*0.75f, top);
					ImVec2 q3(right, (top+bottom)*0.5f);
					int N = 32; int idx = 0;
					for (int i = 0; i < N; ++i)
					{
						float t = (float)i/(float)(N-1);
						float u = 1.0f - t;
						ImVec2 a = ImVec2(u*u*u*p0.x + 3*u*u*t*p1.x + 3*u*t*t*p2.x + t*t*t*p3.x,
						                    u*u*u*p0.y + 3*u*u*t*p1.y + 3*u*t*t*p2.y + t*t*t*p3.y);
						pts_stack[idx++] = a;
					}
					for (int i = 0; i < N; ++i)
					{
						float t = (float)i/(float)(N-1);
						float u = 1.0f - t;
						ImVec2 b = ImVec2(u*u*u*q0.x + 3*u*u*t*q1.x + 3*u*t*t*q2.x + t*t*t*q3.x,
						                    u*u*u*q0.y + 3*u*u*t*q1.y + 3*u*t*t*q2.y + t*t*t*q3.y);
						pts_stack[idx++] = b;
					}
					pts_count = 2*N;
				}

				ImU32 col = IM_COL32(255, 200, 40, 255);
				if (animate)
					offset += ImGui::GetIO().DeltaTime * 50.0f;
				ImWidgets::DrawDashedPolylineAA(dl, pts, pts_count, col, thickness, dash_len, gap_len, offset, closed, (ImWidgetsCap_)cap_idx, (ImWidgetsJoin)join_idx, miter_limit);

				ImGui::SetCursorScreenPos(origin + ImVec2(0, height + 6));
				ImGui::SliderFloat("Thickness##dashed", &thickness, 1.0f, 24.0f);
				ImGui::SliderFloat("Dash##dashed", &dash_len, 1.0f, 100.0f);
				ImGui::SliderFloat("Gap##dashed", &gap_len, 0.0f, 100.0f);
				ImGui::SliderFloat("Offset##dashed", &offset, -200.0f, 200.0f);
				ImGui::Checkbox("Animate Offset##dashed", &animate);
				ImGui::Checkbox("Closed##dashed", &closed);
				ImGui::Combo("Cap##dashed", &cap_idx, caps, IM_ARRAYSIZE(caps));
				ImGui::Combo("Join##dashed", &join_idx, joins, IM_ARRAYSIZE(joins));
				ImGui::SliderFloat("Miter Limit##dashed", &miter_limit, 1.0f, 12.0f, "%.2f");
				ImGui::Combo("Path##dashed", &path_type, paths, IM_ARRAYSIZE(paths));
				bool use_gpu = ImWidgets::GetDashedLinesUseGPU();
				if (ImGui::Checkbox("GPU Path##dashed", &use_gpu))
					ImWidgets::SetDashedLinesUseGPU(use_gpu);
				bool debug_joins = ImWidgets::GetDashedLinesDebugJoins();
				if (ImGui::Checkbox("Debug Joins (CPU)##dashed", &debug_joins))
					ImWidgets::SetDashedLinesDebugJoins(debug_joins);
			}
#endif
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
				HueSelector( "Hue 0##HueSelector", hueHeight, cursorHeight, &hueCenter, &hueWidth, &featherLeft, &featherRight, division, alphaHue, alphaHideHue, offset );
				ImWidgets::GetStyle().PopVar();
				HueSelector( "Hue 1##HueSelector", hueHeight, cursorHeight, &hueCenter, &hueWidth, &featherLeft, &featherRight, division, alphaHue, alphaHideHue, offset );
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
			if ( ImGui::CollapsingHeader( "Gradient Editor", ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				static ImGradientData gradient;
				static bool gradInitialized = false;
				if ( !gradInitialized )
				{
					gradient.Stops.clear();
					gradient.AddStop( 0.0f, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
					gradient.AddStop( 0.5f, ImVec4( 0.0f, 1.0f, 0.0f, 0.5f ) );
					gradient.AddStop( 1.0f, ImVec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
					gradInitialized = true;
				}

				GradientEditor( "##GradientMain", &gradient, ImVec2( 0, 32 ) );

				static char const* interpNames[] = { "sRGB", "Linear sRGB", "OkLab", "OkLCH", "HSV" };
				ImGui::Combo( "Interpolation##GradEditor", &gradient.Interpolation, interpNames, ImWidgetsGradientInterp_COUNT );

				// Edit selected stop color
				if ( gradient.SelectedIdx >= 0 && gradient.SelectedIdx < gradient.Stops.Size )
				{
					ImGradientStop& stop = gradient.Stops[ gradient.SelectedIdx ];
					ImGui::Text( "Stop %d  Position: %.3f", gradient.SelectedIdx, stop.Position );
					ImGui::ColorEdit4( "Stop Color##GradEditor", &stop.Color.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf );
				}
				else
				{
					ImGui::TextDisabled( "No stop selected" );
				}

				ImGui::TextWrapped( "Click bar to add stop. Drag to move. Double-click to edit color. Right-click to delete. Drag far below to remove." );

				// Show sampled output
				static float sampleT = 0.5f;
				ImGui::SliderFloat( "Sample t##GradEditor", &sampleT, 0.0f, 1.0f );
				ImVec4 sampled = ImWidgets::GradientSample( gradient, sampleT );
				ImGui::ColorButton( "Sampled##GradEditor", sampled, ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2( 40, 40 ) );
				ImGui::SameLine();
				ImGui::Text( "RGBA: %.3f, %.3f, %.3f, %.3f", sampled.x, sampled.y, sampled.z, sampled.w );

				ImGui::Text( "Stops: %d, Selected: %d", gradient.Stops.Size, gradient.SelectedIdx );

				// Second gradient editor with different defaults
				static ImGradientData gradient2;
				static bool grad2Initialized = false;
				if ( !grad2Initialized )
				{
					gradient2.Stops.clear();
					gradient2.Interpolation = ImWidgetsGradientInterp_OkLab;
					gradient2.AddStop( 0.0f, ImVec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
					gradient2.AddStop( 1.0f, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
					grad2Initialized = true;
				}
				GradientEditor( "Black to White (OkLab)##Grad2", &gradient2 );
			}
			if ( ImGui::CollapsingHeader( "Curve Editor", ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				static ImCurveEditorData curve;
				static bool curveInitialized = false;
				if ( !curveInitialized )
				{
					curve.Keys.clear();
					curve.AddKey( ImVec2( 0.0f, 0.0f ), ImCurveEditorSeg_InOutCubic );
					curve.AddKey( ImVec2( 0.5f, 1.0f ), ImCurveEditorSeg_InOutCubic );
					curve.AddKey( ImVec2( 1.0f, 0.0f ) );
					curve.RangeMin = ImVec2( -0.1f, -0.2f );
					curve.RangeMax = ImVec2( 1.1f, 1.2f );
					curveInitialized = true;
				}

				CurveEditor( "##CurveMain", &curve, ImVec2( 0, 200 ) );

				// Edit selected key
				if ( curve.SelectedIdx >= 0 && curve.SelectedIdx < curve.Keys.Size )
				{
					ImCurveEditorKey& key = curve.Keys[ curve.SelectedIdx ];
					float cePosMin = ( curve.SelectedIdx > 0 ) ? curve.Keys[ curve.SelectedIdx - 1 ].Pos.x : curve.RangeMin.x;
					float cePosMax = ( curve.SelectedIdx < curve.Keys.Size - 1 ) ? curve.Keys[ curve.SelectedIdx + 1 ].Pos.x : curve.RangeMax.x;
					ImGui::DragFloat2( "Position##CurveKey", &key.Pos.x, 0.01f );
					key.Pos.x = ImClamp( key.Pos.x, cePosMin, cePosMax );

					// Segment type combo
					int currentSeg = key.Segment;
					if ( ImGui::BeginCombo( "Segment##CurveKey", ImWidgets::CurveEditorSegName( ( ImCurveEditorSeg )currentSeg ) ) )
					{
						for ( int s = 0; s < ImCurveEditorSeg_COUNT; ++s )
						{
							bool isSelected = ( currentSeg == s );
							if ( ImGui::Selectable( ImWidgets::CurveEditorSegName( ( ImCurveEditorSeg )s ), isSelected ) )
								key.Segment = ( ImCurveEditorSeg )s;
							if ( isSelected )
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					// Tangent mode combo (only when bezier handles are relevant)
					bool hasBezier = ( key.Segment == ImCurveEditorSeg_CubicBezier )
						|| ( curve.SelectedIdx > 0 && curve.Keys[ curve.SelectedIdx - 1 ].Segment == ImCurveEditorSeg_CubicBezier );
					if ( hasBezier )
					{
						int currentMode = key.TangentMode;
						if ( ImGui::BeginCombo( "Tangent Mode##CurveKey", ImWidgets::CurveEditorTangentModeName( ( ImCurveEditorTangentMode )currentMode ) ) )
						{
							for ( int m = 0; m < ImCurveEditorTangentMode_COUNT; ++m )
							{
								bool isSelected = ( currentMode == m );
								if ( ImGui::Selectable( ImWidgets::CurveEditorTangentModeName( ( ImCurveEditorTangentMode )m ), isSelected ) )
								{
									key.TangentMode = ( ImCurveEditorTangentMode )m;
									if ( m == ImCurveEditorTangentMode_Mirrored )
									{
										key.TangentRight = ImVec2( -key.TangentLeft.x, -key.TangentLeft.y );
										if ( key.TangentRight.x < 0.0f )
											key.TangentRight.x = 0.0f;
									}
								}
								if ( isSelected )
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}

						ImGui::DragFloat2( "In Handle##CurveKey", &key.TangentLeft.x, 0.005f );
						key.TangentLeft.x = ImMin( key.TangentLeft.x, 0.0f );
						ImGui::DragFloat2( "Out Handle##CurveKey", &key.TangentRight.x, 0.005f );
						key.TangentRight.x = ImMax( key.TangentRight.x, 0.0f );
					}
				}
				else
				{
					ImGui::TextDisabled( "No key selected" );
				}

				ImGui::TextWrapped( "Click to add key. Drag to move. Right-click key to change segment type or delete." );

				// Sample
				static float curveSampleX = 0.5f;
				ImGui::SliderFloat( "Sample x##Curve", &curveSampleX, 0.0f, 1.0f );
				float sampledY = ImWidgets::CurveEditorSample( curve, curveSampleX );
				ImGui::Text( "y = %.4f", sampledY );

				// Second curve with step demo
				static ImCurveEditorData curve2;
				static bool curve2Initialized = false;
				if ( !curve2Initialized )
				{
					curve2.Keys.clear();
					curve2.AddKey( ImVec2( 0.0f, 0.0f ), ImCurveEditorSeg_StepCenter );
					curve2.AddKey( ImVec2( 0.25f, 0.5f ), ImCurveEditorSeg_StepStart );
					curve2.AddKey( ImVec2( 0.5f, 1.0f ), ImCurveEditorSeg_StepEnd );
					curve2.AddKey( ImVec2( 0.75f, 0.25f ), ImCurveEditorSeg_Linear );
					curve2.AddKey( ImVec2( 1.0f, 0.75f ) );
					curve2.RangeMin = ImVec2( -0.05f, -0.1f );
					curve2.RangeMax = ImVec2( 1.05f, 1.1f );
					curve2Initialized = true;
				}
				CurveEditor( "Steps & Linear##Curve2", &curve2, ImVec2( 0, 150 ) );
			}

			if ( ImGui::CollapsingHeader( "Color Curve", ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				static int ccMode = ImColorCurveMode_HueVsHue;
				ImGui::Combo( "Mode##CC", &ccMode, "Hue vs Hue\0Hue vs Sat\0Hue vs Lum\0Lum vs Sat\0Sat vs Sat\0" );

				static ImColorCurveData ccData[ ImColorCurveMode_COUNT ];
				static bool ccInit = false;
				if ( !ccInit )
				{
					// Hue vs Hue: shift reds toward orange
					ccData[ ImColorCurveMode_HueVsHue ].AddKey( 0.0f, 0.05f );
					ccData[ ImColorCurveMode_HueVsHue ].AddKey( 0.15f, 0.0f );

					// Hue vs Sat: boost greens
					ccData[ ImColorCurveMode_HueVsSat ].AddKey( 0.25f, 1.0f );
					ccData[ ImColorCurveMode_HueVsSat ].AddKey( 0.33f, 1.5f );
					ccData[ ImColorCurveMode_HueVsSat ].AddKey( 0.42f, 1.0f );

					ccInit = true;
				}

				ImColorCurveData& curData = ccData[ ccMode ];
				ColorCurve( "##CCMain", &curData, ( ImColorCurveMode )ccMode, ImVec2( 0, 150 ) );

				if ( curData.SelectedIdx >= 0 && curData.SelectedIdx < curData.Keys.Size )
				{
					ImColorCurveKey& key = curData.Keys[ curData.SelectedIdx ];
					float posMin = ( curData.SelectedIdx > 0 ) ? curData.Keys[ curData.SelectedIdx - 1 ].Position : 0.0f;
					float posMax = ( curData.SelectedIdx < curData.Keys.Size - 1 ) ? curData.Keys[ curData.SelectedIdx + 1 ].Position : 1.0f;
					ImGui::DragFloat( "Position##CCKey", &key.Position, 0.005f, posMin, posMax, "%.3f" );
					float rMin, rMax;
					ImWidgets::ColorCurveRange( ( ImColorCurveMode )ccMode, &rMin, &rMax );
					ImGui::DragFloat( "Value##CCKey", &key.Value, 0.01f, rMin, rMax, "%.3f" );
				}
				else
				{
					ImGui::TextDisabled( "No key selected" );
				}

				// Sample readout
				static float ccSampleX = 0.5f;
				ImGui::SliderFloat( "Sample x##CC", &ccSampleX, 0.0f, 1.0f );
				float ccVal = ImWidgets::ColorCurveSample( curData, ( ImColorCurveMode )ccMode, ccSampleX );
				ImGui::Text( "y = %.4f", ccVal );

				ImGui::TextWrapped( "Click to add key. Drag to move. Drag far outside to delete. Right-click for options." );
			}

			if ( ImGui::CollapsingHeader( "Parade Scope", ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				static ImParadeScopeData paradeData;
				static int paradeMode = ImParadeMode_RGB;
				static int paradeSource = 0;
				static bool paradeNeedsUpdate = true;
				static stbi_uc* paradeImgData = NULL;
				static int paradeImgW = 0;
				static int paradeImgH = 0;
				static int paradeImgCh = 0;
				static ImTextureID paradeThumbnail = ImTextureID_Invalid;
				static ImVec2 paradeThumbnailSize( 0, 0 );

				static bool paradeOverlay = false;

				bool sourceChanged = ImGui::Combo( "Source##Parade", &paradeSource,
					"Color Bars\0Gradient Ramp\0Random Noise\0"
					"Berries (photo)\0Interior (photo)\0Man (photo)\0Astronaut (photo)\0" );
				bool modeChanged = ImGui::Combo( "Mode##Parade", &paradeMode, "Luminance\0RGB\0YRGB\0YCbCr\0" );
				static int paradeScale = ImParadeScale_Linear;
				ImGui::Checkbox( "Overlay##Parade", &paradeOverlay );
				ImGui::SameLine();
				ImGui::Combo( "Scale##Parade", &paradeScale, "Linear\0Log (Shadows)\0Inv Log (Highlights)\0" );
				if ( sourceChanged || modeChanged )
					paradeNeedsUpdate = true;

				if ( paradeNeedsUpdate )
				{
					// Free previous file-loaded image
					if ( paradeImgData )
					{
						STBI_FREE( paradeImgData );
						paradeImgData = NULL;
					}
					// Free previous synthetic thumbnail
					if ( paradeThumbnail != ImTextureID_Invalid )
					{
						ImPlatform_DestroyTexture( paradeThumbnail );
						paradeThumbnail = ImTextureID_Invalid;
					}

					static int const kTestW = 1920;
					static int const kTestH = 1080;
					static ImVector<ImU8> testImage;

					if ( paradeSource <= 2 )
					{
						// Synthetic test patterns
						testImage.resize( kTestW * kTestH * 3 );
						ImU32 rng = 0xDEADBEEFu;

						if ( paradeSource == 0 )
						{
							// SMPTE color bars with slight noise
							static ImU8 const bars[ 7 ][ 3 ] = {
								{ 191, 191, 191 }, { 191, 191,  17 }, {  17, 191, 191 }, {  17, 191,  17 },
								{ 191,  17, 191 }, { 191,  17,  17 }, {  17,  17, 191 }
							};
							for ( int y = 0; y < kTestH; ++y )
							{
								for ( int x = 0; x < kTestW; ++x )
								{
									int barIdx = x * 7 / kTestW;
									int off = ( y * kTestW + x ) * 3;
									rng = rng * 1664525u + 1013904223u;
									int noise = ( int )( rng >> 28 ) - 8;
									testImage[ off + 0 ] = ( ImU8 )ImClamp( bars[ barIdx ][ 0 ] + noise, 0, 255 );
									testImage[ off + 1 ] = ( ImU8 )ImClamp( bars[ barIdx ][ 1 ] + noise, 0, 255 );
									testImage[ off + 2 ] = ( ImU8 )ImClamp( bars[ barIdx ][ 2 ] + noise, 0, 255 );
								}
							}
						}
						else if ( paradeSource == 1 )
						{
							// Horizontal RGB gradient ramps (R top third, G middle, B bottom)
							for ( int y = 0; y < kTestH; ++y )
							{
								for ( int x = 0; x < kTestW; ++x )
								{
									float t = ( float )x / ( float )( kTestW - 1 );
									int off = ( y * kTestW + x ) * 3;
									int third = y * 3 / kTestH;
									rng = rng * 1664525u + 1013904223u;
									int noise = ( int )( rng >> 29 ) - 4;
									testImage[ off + 0 ] = ( ImU8 )ImClamp( ( third == 0 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
									testImage[ off + 1 ] = ( ImU8 )ImClamp( ( third == 1 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
									testImage[ off + 2 ] = ( ImU8 )ImClamp( ( third == 2 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
								}
							}
						}
						else
						{
							// Random noise
							for ( int i = 0; i < kTestW * kTestH * 3; ++i )
							{
								rng = rng * 1664525u + 1013904223u;
								testImage[ i ] = ( ImU8 )( rng >> 24 );
							}
						}

						paradeData.Accumulate( testImage.Data, kTestW, kTestH, 3,
							ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved, ( ImParadeMode )paradeMode,
							128, 128, 500000 );

						// Create thumbnail texture (RGB -> RGBA)
						{
							static ImVector<ImU8> rgba;
							rgba.resize( kTestW * kTestH * 4 );
							for ( int i = 0; i < kTestW * kTestH; ++i )
							{
								rgba[ i * 4 + 0 ] = testImage[ i * 3 + 0 ];
								rgba[ i * 4 + 1 ] = testImage[ i * 3 + 1 ];
								rgba[ i * 4 + 2 ] = testImage[ i * 3 + 2 ];
								rgba[ i * 4 + 3 ] = 255;
							}
							ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kTestW, kTestH );
							paradeThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
							paradeThumbnailSize = ImVec2( ( float )kTestW, ( float )kTestH );
						}
					}
					else
					{
						// Load real image from file
						char const* filenames[] = {
							"pexels-robert-bogdan-156165-1152351.jpg",
							"pexels-fotoaibe-1571453.jpg",
							"man.png",
							"astro.png"
						};
						int fileIdx = paradeSource - 3;
						paradeImgData = stbi_load( filenames[ fileIdx ], &paradeImgW, &paradeImgH, &paradeImgCh, 0 );
						if ( paradeImgData )
						{
							int ch = ( paradeImgCh >= 3 ) ? paradeImgCh : 3;
							paradeData.Accumulate( paradeImgData, paradeImgW, paradeImgH, ch,
								ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved, ( ImParadeMode )paradeMode,
								128, 128, 1000000 );
						}
					}

					paradeNeedsUpdate = false;
				}

				// Thumbnail
				{
					ImTextureID thumbTex = ImTextureID_Invalid;
					ImVec2 thumbSize( 0, 0 );
					if ( paradeSource <= 2 && paradeThumbnail != ImTextureID_Invalid )
					{
						thumbTex = paradeThumbnail;
						thumbSize = paradeThumbnailSize;
					}
					else if ( paradeSource == 3 ) { thumbTex = illlustration_img; thumbSize = illlustration_size; }
					else if ( paradeSource == 4 ) { thumbTex = background;        thumbSize = background_size; }
					else if ( paradeSource == 5 ) { thumbTex = man_img;           thumbSize = man_size; }
					else if ( paradeSource == 6 ) { thumbTex = astro_img;         thumbSize = astro_size; }
					if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
					{
						float thumbH = 120.0f;
						float thumbW = thumbH * thumbSize.x / thumbSize.y;
						ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
					}
				}

				ImWidgets::ParadeScope( "##ParadeMain", paradeData, paradeOverlay, ( ImParadeScale )paradeScale, ImVec2( 0, 300 ) );
				if ( paradeSource <= 2 )
					ImGui::Text( "Source: 1920x1080 (generated)  Peak: %u", paradeData.PeakCount );
				else if ( paradeImgData )
					ImGui::Text( "Source: %dx%d (%d ch)  Peak: %u", paradeImgW, paradeImgH, paradeImgCh, paradeData.PeakCount );
				else
					ImGui::TextDisabled( "Failed to load image" );
			}

			if ( ImGui::CollapsingHeader( "Vector Scope", ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				static ImVectorScopeData vectorData;
				static int vectorSource = 0;
				static bool vectorNeedsUpdate = true;
				static stbi_uc* vectorImgData = NULL;
				static int vectorImgW = 0;
				static int vectorImgH = 0;
				static int vectorImgCh = 0;
				static bool vectorShowSkinTone = true;
				static ImTextureID vectorThumbnail = ImTextureID_Invalid;
				static ImVec2 vectorThumbnailSize( 0, 0 );

				bool sourceChanged = ImGui::Combo( "Source##Vector", &vectorSource,
					"Color Bars\0Gradient Ramp\0Random Noise\0"
					"Berries (photo)\0Interior (photo)\0Man (photo)\0Astronaut (photo)\0" );
				ImGui::Checkbox( "Skin Tone Line##Vector", &vectorShowSkinTone );
				if ( sourceChanged )
					vectorNeedsUpdate = true;

				if ( vectorNeedsUpdate )
				{
					if ( vectorImgData )
					{
						STBI_FREE( vectorImgData );
						vectorImgData = NULL;
					}
					if ( vectorThumbnail != ImTextureID_Invalid )
					{
						ImPlatform_DestroyTexture( vectorThumbnail );
						vectorThumbnail = ImTextureID_Invalid;
					}

					static int const kTestW = 1920;
					static int const kTestH = 1080;
					static ImVector<ImU8> testImage;

					if ( vectorSource <= 2 )
					{
						testImage.resize( kTestW * kTestH * 3 );
						ImU32 rng = 0xDEADBEEFu;

						if ( vectorSource == 0 )
						{
							// SMPTE color bars with slight noise
							static ImU8 const bars[ 7 ][ 3 ] = {
								{ 191, 191, 191 }, { 191, 191,  17 }, {  17, 191, 191 }, {  17, 191,  17 },
								{ 191,  17, 191 }, { 191,  17,  17 }, {  17,  17, 191 }
							};
							for ( int y = 0; y < kTestH; ++y )
							{
								for ( int x = 0; x < kTestW; ++x )
								{
									int barIdx = x * 7 / kTestW;
									int off = ( y * kTestW + x ) * 3;
									rng = rng * 1664525u + 1013904223u;
									int noise = ( int )( rng >> 28 ) - 8;
									testImage[ off + 0 ] = ( ImU8 )ImClamp( bars[ barIdx ][ 0 ] + noise, 0, 255 );
									testImage[ off + 1 ] = ( ImU8 )ImClamp( bars[ barIdx ][ 1 ] + noise, 0, 255 );
									testImage[ off + 2 ] = ( ImU8 )ImClamp( bars[ barIdx ][ 2 ] + noise, 0, 255 );
								}
							}
						}
						else if ( vectorSource == 1 )
						{
							// Horizontal RGB gradient ramps
							for ( int y = 0; y < kTestH; ++y )
							{
								for ( int x = 0; x < kTestW; ++x )
								{
									float t = ( float )x / ( float )( kTestW - 1 );
									int off = ( y * kTestW + x ) * 3;
									int third = y * 3 / kTestH;
									rng = rng * 1664525u + 1013904223u;
									int noise = ( int )( rng >> 29 ) - 4;
									testImage[ off + 0 ] = ( ImU8 )ImClamp( ( third == 0 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
									testImage[ off + 1 ] = ( ImU8 )ImClamp( ( third == 1 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
									testImage[ off + 2 ] = ( ImU8 )ImClamp( ( third == 2 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
								}
							}
						}
						else
						{
							// Random noise
							for ( int i = 0; i < kTestW * kTestH * 3; ++i )
							{
								rng = rng * 1664525u + 1013904223u;
								testImage[ i ] = ( ImU8 )( rng >> 24 );
							}
						}

						vectorData.Accumulate( testImage.Data, kTestW, kTestH, 3,
							ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved,
							256, 500000 );

						// Create thumbnail texture (RGB -> RGBA)
						{
							static ImVector<ImU8> rgba;
							rgba.resize( kTestW * kTestH * 4 );
							for ( int i = 0; i < kTestW * kTestH; ++i )
							{
								rgba[ i * 4 + 0 ] = testImage[ i * 3 + 0 ];
								rgba[ i * 4 + 1 ] = testImage[ i * 3 + 1 ];
								rgba[ i * 4 + 2 ] = testImage[ i * 3 + 2 ];
								rgba[ i * 4 + 3 ] = 255;
							}
							ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kTestW, kTestH );
							vectorThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
							vectorThumbnailSize = ImVec2( ( float )kTestW, ( float )kTestH );
						}
					}
					else
					{
						char const* filenames[] = {
							"pexels-robert-bogdan-156165-1152351.jpg",
							"pexels-fotoaibe-1571453.jpg",
							"man.png",
							"astro.png"
						};
						int fileIdx = vectorSource - 3;
						vectorImgData = stbi_load( filenames[ fileIdx ], &vectorImgW, &vectorImgH, &vectorImgCh, 0 );
						if ( vectorImgData )
						{
							int ch = ( vectorImgCh >= 3 ) ? vectorImgCh : 3;
							vectorData.Accumulate( vectorImgData, vectorImgW, vectorImgH, ch,
								ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved,
								256, 1000000 );
						}
					}

					vectorNeedsUpdate = false;
				}

				// Thumbnail
				{
					ImTextureID thumbTex = ImTextureID_Invalid;
					ImVec2 thumbSize( 0, 0 );
					if ( vectorSource <= 2 && vectorThumbnail != ImTextureID_Invalid )
					{
						thumbTex = vectorThumbnail;
						thumbSize = vectorThumbnailSize;
					}
					else if ( vectorSource == 3 ) { thumbTex = illlustration_img; thumbSize = illlustration_size; }
					else if ( vectorSource == 4 ) { thumbTex = background;        thumbSize = background_size; }
					else if ( vectorSource == 5 ) { thumbTex = man_img;           thumbSize = man_size; }
					else if ( vectorSource == 6 ) { thumbTex = astro_img;         thumbSize = astro_size; }
					if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
					{
						float thumbH = 240.0f;
						float thumbW = thumbH * thumbSize.x / thumbSize.y;
						ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
					}
				}

				ImWidgets::VectorScope( "##VectorMain", vectorData, vectorShowSkinTone, ImVec2( 600, 600 ) );
				if ( vectorSource <= 2 )
					ImGui::Text( "Source: 1920x1080 (generated)  Peak: %u", vectorData.PeakCount );
				else if ( vectorImgData )
					ImGui::Text( "Source: %dx%d (%d ch)  Peak: %u", vectorImgW, vectorImgH, vectorImgCh, vectorData.PeakCount );
				else
					ImGui::TextDisabled( "Failed to load image" );
			}

			if ( ImGui::CollapsingHeader( "Histogram", ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				static ImHistogramData histData;
				static int histMode = ImHistogramMode_RGB;
				static int histSource = 0;
				static bool histNeedsUpdate = true;
				static stbi_uc* histImgData = NULL;
				static int histImgW = 0;
				static int histImgH = 0;
				static int histImgCh = 0;
				static ImTextureID histThumbnail = ImTextureID_Invalid;
				static ImVec2 histThumbnailSize( 0, 0 );

				bool sourceChanged = ImGui::Combo( "Source##Hist", &histSource,
					"Color Bars\0Gradient Ramp\0Random Noise\0"
					"Berries (photo)\0Interior (photo)\0Man (photo)\0Astronaut (photo)\0" );
				bool modeChanged = ImGui::Combo( "Mode##Hist", &histMode, "Luminance\0RGB\0YRGB\0YCbCr\0HSV\0OkLCH\0" );
				static int histLayout = ImHistogramLayout_Overlapped;
				static int histXScale = ImParadeScale_Linear;
				static int histYScale = ImParadeScale_Linear;
				ImGui::Combo( "Layout##Hist", &histLayout, "Overlapped\0Stacked\0" );
				ImGui::Combo( "X Scale##Hist", &histXScale, "Linear\0Log (Shadows)\0Inv Log (Highlights)\0" );
				ImGui::Combo( "Y Scale##Hist", &histYScale, "Linear\0Log\0Inv Log\0" );
				if ( sourceChanged || modeChanged )
					histNeedsUpdate = true;

				if ( histNeedsUpdate )
				{
					if ( histImgData )
					{
						STBI_FREE( histImgData );
						histImgData = NULL;
					}
					if ( histThumbnail != ImTextureID_Invalid )
					{
						ImPlatform_DestroyTexture( histThumbnail );
						histThumbnail = ImTextureID_Invalid;
					}

					static int const kTestW = 1920;
					static int const kTestH = 1080;
					static ImVector<ImU8> testImage;

					if ( histSource <= 2 )
					{
						testImage.resize( kTestW * kTestH * 3 );
						ImU32 rng = 0xDEADBEEFu;

						if ( histSource == 0 )
						{
							static ImU8 const bars[ 7 ][ 3 ] = {
								{ 191, 191, 191 }, { 191, 191,  17 }, {  17, 191, 191 }, {  17, 191,  17 },
								{ 191,  17, 191 }, { 191,  17,  17 }, {  17,  17, 191 }
							};
							for ( int y = 0; y < kTestH; ++y )
							{
								for ( int x = 0; x < kTestW; ++x )
								{
									int barIdx = x * 7 / kTestW;
									int off = ( y * kTestW + x ) * 3;
									rng = rng * 1664525u + 1013904223u;
									int noise = ( int )( rng >> 28 ) - 8;
									testImage[ off + 0 ] = ( ImU8 )ImClamp( bars[ barIdx ][ 0 ] + noise, 0, 255 );
									testImage[ off + 1 ] = ( ImU8 )ImClamp( bars[ barIdx ][ 1 ] + noise, 0, 255 );
									testImage[ off + 2 ] = ( ImU8 )ImClamp( bars[ barIdx ][ 2 ] + noise, 0, 255 );
								}
							}
						}
						else if ( histSource == 1 )
						{
							for ( int y = 0; y < kTestH; ++y )
							{
								for ( int x = 0; x < kTestW; ++x )
								{
									float t = ( float )x / ( float )( kTestW - 1 );
									int off = ( y * kTestW + x ) * 3;
									int third = y * 3 / kTestH;
									rng = rng * 1664525u + 1013904223u;
									int noise = ( int )( rng >> 29 ) - 4;
									testImage[ off + 0 ] = ( ImU8 )ImClamp( ( third == 0 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
									testImage[ off + 1 ] = ( ImU8 )ImClamp( ( third == 1 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
									testImage[ off + 2 ] = ( ImU8 )ImClamp( ( third == 2 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
								}
							}
						}
						else
						{
							for ( int i = 0; i < kTestW * kTestH * 3; ++i )
							{
								rng = rng * 1664525u + 1013904223u;
								testImage[ i ] = ( ImU8 )( rng >> 24 );
							}
						}

						histData.Accumulate( testImage.Data, kTestW, kTestH, 3,
							ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved, ( ImHistogramMode )histMode,
							256, 500000 );

						// Create thumbnail texture (RGB -> RGBA)
						{
							static ImVector<ImU8> rgba;
							rgba.resize( kTestW * kTestH * 4 );
							for ( int i = 0; i < kTestW * kTestH; ++i )
							{
								rgba[ i * 4 + 0 ] = testImage[ i * 3 + 0 ];
								rgba[ i * 4 + 1 ] = testImage[ i * 3 + 1 ];
								rgba[ i * 4 + 2 ] = testImage[ i * 3 + 2 ];
								rgba[ i * 4 + 3 ] = 255;
							}
							ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kTestW, kTestH );
							histThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
							histThumbnailSize = ImVec2( ( float )kTestW, ( float )kTestH );
						}
					}
					else
					{
						char const* filenames[] = {
							"pexels-robert-bogdan-156165-1152351.jpg",
							"pexels-fotoaibe-1571453.jpg",
							"man.png",
							"astro.png"
						};
						int fileIdx = histSource - 3;
						histImgData = stbi_load( filenames[ fileIdx ], &histImgW, &histImgH, &histImgCh, 0 );
						if ( histImgData )
						{
							int ch = ( histImgCh >= 3 ) ? histImgCh : 3;
							histData.Accumulate( histImgData, histImgW, histImgH, ch,
								ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved, ( ImHistogramMode )histMode,
								256, 1000000 );
						}
					}

					histNeedsUpdate = false;
				}

				// Thumbnail
				{
					ImTextureID thumbTex = ImTextureID_Invalid;
					ImVec2 thumbSize( 0, 0 );
					if ( histSource <= 2 && histThumbnail != ImTextureID_Invalid )
					{
						thumbTex = histThumbnail;
						thumbSize = histThumbnailSize;
					}
					else if ( histSource == 3 ) { thumbTex = illlustration_img; thumbSize = illlustration_size; }
					else if ( histSource == 4 ) { thumbTex = background;        thumbSize = background_size; }
					else if ( histSource == 5 ) { thumbTex = man_img;           thumbSize = man_size; }
					else if ( histSource == 6 ) { thumbTex = astro_img;         thumbSize = astro_size; }
					if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
					{
						float thumbH = 120.0f;
						float thumbW = thumbH * thumbSize.x / thumbSize.y;
						ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
					}
				}

				ImWidgets::Histogram( "##HistMain", histData, ( ImHistogramLayout )histLayout, ( ImParadeScale )histXScale, ( ImParadeScale )histYScale, ImVec2( 0, 300 ) );
				if ( histSource <= 2 )
					ImGui::Text( "Source: 1920x1080 (generated)  Peak: %u", histData.PeakCount );
				else if ( histImgData )
					ImGui::Text( "Source: %dx%d (%d ch)  Peak: %u", histImgW, histImgH, histImgCh, histData.PeakCount );
				else
					ImGui::TextDisabled( "Failed to load image" );
			}

			if ( ImGui::CollapsingHeader( "Color Wheel", ImGuiTreeNodeFlags_DefaultOpen ) )
			{
				static ImVec4 wheelColor( 0.8f, 0.2f, 0.3f, 1.0f );
				static int wheelMode = ImColorWheelMode_HSV;

				ImGui::Combo( "Mode##Wheel", &wheelMode, "HSV\0OkLCH\0" );

				ColorWheel( "##WheelMain", &wheelColor, ( ImColorWheelMode )wheelMode );

				ImGui::ColorEdit4( "Color##Wheel", &wheelColor.x, ImGuiColorEditFlags_Float );

				ImGui::Separator();

				// Second wheel: OkLCH with HDR slider
				static ImVec4 wheelColor2( 0.5f, 0.7f, 0.2f, 1.0f );
				ImGui::Text( "OkLCH Wheel (HDR max = 2.0)" );
				ColorWheel( "##WheelHDR", &wheelColor2, ImColorWheelMode_OkLCH, 2.0f );
				ImGui::ColorEdit4( "HDR Color##Wheel2", &wheelColor2.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR );
			}

			ImGui::Unindent();
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}
}

#include <imgui_demo.cpp>
