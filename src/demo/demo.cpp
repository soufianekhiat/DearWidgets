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
		ImGui::PopItemWidth();
		ImGui::SameLine();
		ImWidgets::Slider2DFloat( label_uv1, &uv_end.x, &uv_end.y, 0.0f, 1.0f, 0.0f, 1.0f );
		ImGui::PopItemWidth();
		ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
		cola.Edit( label_a );
		ImGui::PopItemWidth();
		ImGui::SameLine();
		colb.Edit( label_b );
		ImGui::PopItemWidth();
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

// Monochrome
ImFont* g_firaCodeFont       = nullptr;
ImFont* g_cinzelFont         = nullptr;
ImFont* g_alfaSlabFont       = nullptr;
ImFont* g_dottedFont         = nullptr;
ImFont* g_flowmeryFont       = nullptr;
ImFont* g_franticallyFont    = nullptr;
ImFont* g_loveLightFont      = nullptr;
ImFont* g_magnoliaFont       = nullptr;
ImFont* g_rosehotFont        = nullptr;
ImFont* g_squareLilyFont     = nullptr;
// Ligature showcase
ImFont* g_allessaFont        = nullptr;
ImFont* g_bollgoFont         = nullptr;
ImFont* g_boucherFont        = nullptr;
ImFont* g_brightMarchFont    = nullptr;
ImFont* g_camoodFont         = nullptr;
ImFont* g_cheronaFont        = nullptr;
ImFont* g_classicalFont      = nullptr;
ImFont* g_daelingFont        = nullptr;
ImFont* g_endlessFont        = nullptr;
ImFont* g_foglihtenFont      = nullptr;
ImFont* g_galinsFont         = nullptr;
ImFont* g_gallanteFont       = nullptr;
ImFont* g_gimboFont          = nullptr;
ImFont* g_gingaFont          = nullptr;
ImFont* g_kleymisskyFont     = nullptr;
ImFont* g_metaforaAltFont    = nullptr;
ImFont* g_metaforaSsFont     = nullptr;
ImFont* g_migullonFont       = nullptr;
ImFont* g_milsskyFont        = nullptr;
ImFont* g_molgethFont        = nullptr;
ImFont* g_monblockFont       = nullptr;
ImFont* g_prida61Font        = nullptr;
ImFont* g_reginaFont         = nullptr;
ImFont* g_retroHeartFont     = nullptr;
ImFont* g_sophieFont         = nullptr;
ImFont* g_steelworksFont     = nullptr;
// Color fonts
ImFont* g_twemojiFont        = nullptr;
ImFont* g_aquaphonicDownpourFont = nullptr;
ImFont* g_aquaphonicDrizzleFont  = nullptr;
ImFont* g_bungeeSpiceFont        = nullptr;
ImFont* g_cimeroProFont          = nullptr;
ImFont* g_colorTubeFont          = nullptr;
ImFont* g_fatternFont            = nullptr;
ImFont* g_gilbertColorFont       = nullptr;
ImFont* g_manbowClearFont        = nullptr;
ImFont* g_manbowLinesFont        = nullptr;
ImFont* g_manbowSpotsFont        = nullptr;
ImFont* g_manbowToneFont         = nullptr;
ImFont* g_multicoloreFont        = nullptr;
ImFont* g_nablaFont              = nullptr;
ImFont* g_primecolorCV1Font      = nullptr;
ImFont* g_primecolorGFont        = nullptr;
ImFont* g_primecolorMFont        = nullptr;
ImFont* g_honkFont               = nullptr;
ImFont* g_coralPixelsFont        = nullptr;
ImFont* g_notoZnamennyFont       = nullptr;
// Arabic
ImFont* g_arefRuqaaBoldFont     = nullptr;
ImFont* g_arefRuqaaRegFont      = nullptr;
ImFont* g_blakaInkFont          = nullptr;
ImFont* g_reemKufiFunFont       = nullptr;
ImFont* g_cairoPlayBoldFont     = nullptr;
ImFont* g_cairoPlayXLightFont   = nullptr;
ImFont* g_reemKufiInkFont       = nullptr;

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

namespace ImWidgets { void ShowShowcase(); }

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

	// Use Slug font loader for all demo fonts (enables colored atlas for COLR/SVG fonts)
	ImFontConfig slugCfg;
	slugCfg.FontLoader = ImWidgets::GetSlugFontLoader();

	g_firaCodeFont    = io.Fonts->AddFontFromFileTTF( "fonts/FiraCode-Regular.ttf",                           24.0f, &slugCfg );

	// Rasterized at 24 px just so ImGui holds the TTF data; Slug renders at any size.
	g_cinzelFont      = io.Fonts->AddFontFromFileTTF( "fonts/Cinzel.ttf", 24.0f, &slugCfg );
	g_alfaSlabFont    = io.Fonts->AddFontFromFileTTF( "fonts/AlfaSlabOne-Regular.ttf", 24.0f, &slugCfg );
	g_dottedFont      = io.Fonts->AddFontFromFileTTF( "fonts/Dotted.ttf", 24.0f, &slugCfg );
	g_flowmeryFont    = io.Fonts->AddFontFromFileTTF( "fonts/Flowmery-Regular.ttf", 24.0f, &slugCfg );
	g_franticallyFont = io.Fonts->AddFontFromFileTTF( "fonts/Frantically-Regular (1).ttf", 24.0f, &slugCfg );
	g_loveLightFont   = io.Fonts->AddFontFromFileTTF( "fonts/LoveLight-Regular.ttf", 24.0f, &slugCfg );
	g_magnoliaFont    = io.Fonts->AddFontFromFileTTF( "fonts/Magnolia Floral Line Monogram.ttf", 24.0f, &slugCfg );
	g_rosehotFont     = io.Fonts->AddFontFromFileTTF( "fonts/Rosehot.ttf", 24.0f, &slugCfg );
	g_nablaFont       = io.Fonts->AddFontFromFileTTF( "fonts/Nabla-Regular-VariableFont_EDPT,EHLT.ttf", 24.0f, &slugCfg );
	g_squareLilyFont  = io.Fonts->AddFontFromFileTTF( "fonts/Square Lily Monogram.ttf", 24.0f, &slugCfg );
	// Ligature showcase fonts
	g_allessaFont     = io.Fonts->AddFontFromFileTTF( "fonts/AllessaPersonalUse-4pRl.ttf", 24.0f, &slugCfg );
	g_bollgoFont      = io.Fonts->AddFontFromFileTTF( "fonts/Bollgo-zr2pX.ttf", 24.0f, &slugCfg );
	g_boucherFont     = io.Fonts->AddFontFromFileTTF( "fonts/BoucherDemoRegular-lxRoD.ttf", 24.0f, &slugCfg );
	g_brightMarchFont = io.Fonts->AddFontFromFileTTF( "fonts/BrightMarchingRegular-9MA72.otf", 24.0f, &slugCfg );
	g_camoodFont      = io.Fonts->AddFontFromFileTTF( "fonts/Camood-aYoaR.otf", 24.0f, &slugCfg );
	g_cheronaFont     = io.Fonts->AddFontFromFileTTF( "fonts/Cherona-LVG73.otf", 24.0f, &slugCfg );
	g_classicalFont   = io.Fonts->AddFontFromFileTTF( "fonts/ClassicalAestheticsDemoRegular-0vqjX.ttf", 24.0f, &slugCfg );
	g_daelingFont     = io.Fonts->AddFontFromFileTTF( "fonts/Daeling-Jp65x.ttf", 24.0f, &slugCfg );
	g_endlessFont     = io.Fonts->AddFontFromFileTTF( "fonts/EndlesslyExpandedDemoRegular-rvXlp.ttf", 24.0f, &slugCfg );
	g_foglihtenFont   = io.Fonts->AddFontFromFileTTF( "fonts/Foglihtenno07calt-WpzEA.otf", 24.0f, &slugCfg );
	g_galinsFont      = io.Fonts->AddFontFromFileTTF( "fonts/GalinsRegular-Wp5eY.otf", 24.0f, &slugCfg );
	g_gallanteFont    = io.Fonts->AddFontFromFileTTF( "fonts/Gallante-AR1ap.otf", 24.0f, &slugCfg );
	g_gimboFont       = io.Fonts->AddFontFromFileTTF( "fonts/Gimbo-ovZdA.ttf", 24.0f, &slugCfg );
	g_gingaFont       = io.Fonts->AddFontFromFileTTF( "fonts/Ginga-r09p.ttf", 24.0f, &slugCfg );
	g_kleymisskyFont  = io.Fonts->AddFontFromFileTTF( "fonts/Kleymissky-0xBG.otf", 24.0f, &slugCfg );
	g_metaforaAltFont = io.Fonts->AddFontFromFileTTF( "fonts/MetaforaAlternateAndSwashRegular-KVRnW.ttf", 24.0f, &slugCfg );
	g_metaforaSsFont  = io.Fonts->AddFontFromFileTTF( "fonts/MetaforaSs05Ss09Regular-vn5LZ.ttf", 24.0f, &slugCfg );
	g_migullonFont    = io.Fonts->AddFontFromFileTTF( "fonts/Migullon-V4e6l.otf", 24.0f, &slugCfg );
	g_milsskyFont     = io.Fonts->AddFontFromFileTTF( "fonts/MilsskyRegular-aYJOE.otf", 24.0f, &slugCfg );
	g_molgethFont     = io.Fonts->AddFontFromFileTTF( "fonts/Molgeth-xRMpm.ttf", 24.0f, &slugCfg );
	g_monblockFont    = io.Fonts->AddFontFromFileTTF( "fonts/Monblock-wo9Rn.otf", 24.0f, &slugCfg );
	g_prida61Font     = io.Fonts->AddFontFromFileTTF( "fonts/Prida61-Groa.otf", 24.0f, &slugCfg );
	g_reginaFont      = io.Fonts->AddFontFromFileTTF( "fonts/Regina-K7Yvl.ttf", 24.0f, &slugCfg );
	g_retroHeartFont  = io.Fonts->AddFontFromFileTTF( "fonts/RetroHeartYou-1jvD4.otf", 24.0f, &slugCfg );
	g_sophieFont      = io.Fonts->AddFontFromFileTTF( "fonts/SophiamelanieRegular-E4gee.otf", 24.0f, &slugCfg );
	g_steelworksFont  = io.Fonts->AddFontFromFileTTF( "fonts/SteelworksVintageDemo-rR98.ttf", 24.0f, &slugCfg );
	// Color fonts
	g_twemojiFont        = io.Fonts->AddFontFromFileTTF( "fonts/Twemoji.Mozilla.ttf", 24.0f, &slugCfg );
	g_aquaphonicDownpourFont = io.Fonts->AddFontFromFileTTF( "fonts/Aquaphonic-Downpour.otf", 24.0f, &slugCfg );
	g_aquaphonicDrizzleFont  = io.Fonts->AddFontFromFileTTF( "fonts/Aquaphonic-Drizzle.otf", 24.0f, &slugCfg );
	g_bungeeSpiceFont        = io.Fonts->AddFontFromFileTTF( "fonts/BungeeSpice-Regular.ttf", 24.0f, &slugCfg );
	g_cimeroProFont          = io.Fonts->AddFontFromFileTTF( "fonts/CimeroPro.otf", 24.0f, &slugCfg );
	g_colorTubeFont          = io.Fonts->AddFontFromFileTTF( "fonts/ColorTube.otf", 24.0f, &slugCfg );
	g_fatternFont            = io.Fonts->AddFontFromFileTTF( "fonts/Fattern-GO6zm.otf", 24.0f, &slugCfg );
	g_gilbertColorFont       = io.Fonts->AddFontFromFileTTF( "fonts/Gilbert-Color Bold Preview5.otf", 24.0f, &slugCfg );
	g_manbowClearFont        = io.Fonts->AddFontFromFileTTF( "fonts/Manbow Clear.otf", 24.0f, &slugCfg );
	g_manbowLinesFont        = io.Fonts->AddFontFromFileTTF( "fonts/Manbow Lines.otf", 24.0f, &slugCfg );
	g_manbowSpotsFont        = io.Fonts->AddFontFromFileTTF( "fonts/Manbow Spots.otf", 24.0f, &slugCfg );
	g_manbowToneFont         = io.Fonts->AddFontFromFileTTF( "fonts/Manbow Tone.otf", 24.0f, &slugCfg );
	g_multicoloreFont        = io.Fonts->AddFontFromFileTTF( "fonts/Multicolore Pro.otf", 24.0f, &slugCfg );
	g_primecolorCV1Font      = io.Fonts->AddFontFromFileTTF( "fonts/Primecolor-CV1.ttf", 24.0f, &slugCfg );
	g_primecolorGFont        = io.Fonts->AddFontFromFileTTF( "fonts/Primecolor-G.ttf", 24.0f, &slugCfg );
	g_primecolorMFont        = io.Fonts->AddFontFromFileTTF( "fonts/Primecolor-M.ttf", 24.0f, &slugCfg );
	// Arabic glyph range for Arabic fonts
	static const ImWchar arabicRanges[] = { 0x0020, 0x007E, 0x0600, 0x06FF, 0xFE70, 0xFEFF, 0 };
	g_arefRuqaaBoldFont      = io.Fonts->AddFontFromFileTTF( "fonts/ArefRuqaaInk-Bold.ttf",                        24.0f, &slugCfg, arabicRanges );
	g_arefRuqaaRegFont       = io.Fonts->AddFontFromFileTTF( "fonts/ArefRuqaaInk-Regular.ttf",                     24.0f, &slugCfg, arabicRanges );
	g_blakaInkFont           = io.Fonts->AddFontFromFileTTF( "fonts/BlakaInk-Regular.ttf",                         24.0f, &slugCfg, arabicRanges );
	g_reemKufiInkFont        = io.Fonts->AddFontFromFileTTF( "fonts/ReemKufiInk-Regular.ttf",                      24.0f, &slugCfg, arabicRanges );
	g_reemKufiFunFont        = io.Fonts->AddFontFromFileTTF( "fonts/ReemKufiFun-Regular.ttf",                      24.0f, &slugCfg, arabicRanges );
	g_cairoPlayBoldFont      = io.Fonts->AddFontFromFileTTF( "fonts/CairoPlay-Bold.ttf",                           24.0f, &slugCfg, arabicRanges );
	g_cairoPlayXLightFont    = io.Fonts->AddFontFromFileTTF( "fonts/CairoPlay-ExtraLight.ttf",                     24.0f, &slugCfg, arabicRanges );
	g_coralPixelsFont        = io.Fonts->AddFontFromFileTTF( "fonts/CoralPixels-Regular.ttf", 24.0f, &slugCfg );
	g_honkFont               = io.Fonts->AddFontFromFileTTF( "fonts/Honk-Regular-VariableFont_MORF,SHLN.ttf", 24.0f, &slugCfg );

	// Load LaTeX math font (Latin Modern Math)
	ImWidgets::LoadLaTeXFont();

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
	ImWidgets::AddFeatures( ImWidgetsFeatures_Markers | ImWidgetsFeatures_RichFont | ImWidgetsFeatures_LaTeX );
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
		ImWidgets::ShowShowcase();
		ImWidgets::ShowStyleEditor();
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
	static int  s_open_all = 0;
	static void ApplyOpenAll() { if ( s_open_all != 0 ) ImGui::SetNextItemOpen( s_open_all > 0, ImGuiCond_Always ); }
	static float CanvasSize() { return ImMin( ImGui::GetContentRegionAvail().x, 400.0f ); }

	void ShowDrawShapeDemo()
	{
		ApplyOpenAll();
		if ( !ImGui::CollapsingHeader( "Draw Shape" ) )
			return;

		float const size = CanvasSize();
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

	void ShowDrawTextDemo()
	{
		ApplyOpenAll();
		if ( !ImGui::CollapsingHeader( "GPU Text (Slug)" ) )
			return;

		if ( !g_cinzelFont && !g_alfaSlabFont && !g_dottedFont && !g_flowmeryFont &&
		     !g_franticallyFont && !g_loveLightFont && !g_magnoliaFont &&
		     !g_nablaFont && !g_rosehotFont && !g_squareLilyFont )
		{
			ImGui::TextDisabled( "No Slug fonts loaded." );
			return;
		}

		static char   text_buf[256] = "=><=Dear Widgets!";
		static float  font_size = 48.0f;
		static ImVec4 color_v( 0.92f, 0.82f, 0.60f, 1.0f );  // warm gold
		static ImU32  color_u = ImGui::ColorConvertFloat4ToU32( color_v );
		static bool   use_bg = false;
		static ImVec4 bg_color_v( 0.12f, 0.12f, 0.18f, 1.0f );  // dark navy
		static ImU32  bg_color_u = ImGui::ColorConvertFloat4ToU32( bg_color_v );

		ImGui::InputText( "Text##SlugDemo", text_buf, sizeof( text_buf ) );
		ImGui::DragFloat( "Font Size##SlugDemo", &font_size, 0.5f, 8.0f, 300.0f, "%.0f px" );
		if ( ImGui::ColorEdit4( "Color##SlugDemo", &color_v.x ) )
			color_u = ImGui::ColorConvertFloat4ToU32( color_v );
		ImGui::Checkbox( "Background##SlugDemo", &use_bg );
		if ( use_bg )
		{
			ImGui::SameLine();
			if ( ImGui::ColorEdit4( "##BgColor", &bg_color_v.x, ImGuiColorEditFlags_NoLabel ) )
				bg_color_u = ImGui::ColorConvertFloat4ToU32( bg_color_v );
		}

		static bool  use_atlas = false;
		ImGui::Checkbox( "Atlas Mode (bitmap, ImFontLoader)##SlugAtlas", &use_atlas );
		ImGui::SameLine(); ImGui::TextDisabled( use_atlas ? "(CPU rasterized)" : "(GPU Slug)" );
		if ( use_atlas )
			ImGui::TextDisabled( "  Atlas: COLR v0 = color, COLR v1 = flat (no gradient), SVG = monochrome" );
		static bool  debug_curves = false;
		static bool  dbg_curves_on = true, dbg_ctrl_on = true, dbg_bbox_on = true, dbg_bands_on = false;
		static bool  debug_layers = false;
		ImGui::Checkbox( "Debug Curves##SlugDebug", &debug_curves );
		if ( debug_curves )
		{
			ImGui::SameLine(); ImGui::Checkbox( "Curves", &dbg_curves_on );
			ImGui::SameLine(); ImGui::Checkbox( "Ctrl Pts", &dbg_ctrl_on );
			ImGui::SameLine(); ImGui::Checkbox( "BBox", &dbg_bbox_on );
			ImGui::SameLine(); ImGui::Checkbox( "Bands", &dbg_bands_on );
		}
		ImGui::Checkbox( "Show Layer Quads (flat color, no shader)##SlugLayerDbg", &debug_layers );
		ImGui::Checkbox( "Debug Shader (R=xcov G=ycov B=cov)##SlugShaderDbg", &ImWidgets::g_SlugDebugShader );

		enum FontTextType { kLatin = 0, kEmoji = 1, kArabic = 2 };
		struct FontEntry { ImFont** font; const char* label; FontTextType textType; const char* group; };
		static const char* kGrpCode   = "Programming / Code";
		static const char* kGrpSerif  = "Serif";
		static const char* kGrpScript = "Script / Handwriting";
		static const char* kGrpDisp   = "Display / Decorative";
		static const char* kGrpCFF    = "CFF Monochrome";
		static const char* kGrpColr0  = "Color: COLR v0";
		static const char* kGrpSVG    = "Color: SVG";
		static const char* kGrpColr1  = "Color: COLR v1 / Gradient";
		static const char* kGrpArabic = "Arabic";
		static const FontEntry kFonts[] = {
			{ &g_firaCodeFont,    "Fira Code",               kLatin,  kGrpCode },
			{ &g_monblockFont,    "Monblock",                kLatin,  kGrpCode },
			{ &g_cinzelFont,      "Cinzel",                  kLatin,  kGrpSerif },
			{ &g_alfaSlabFont,    "Alfa Slab One",           kLatin,  kGrpSerif },
			{ &g_classicalFont,   "Classical Aesthetics",    kLatin,  kGrpSerif },
			{ &g_foglihtenFont,   "Foglihten No07",          kLatin,  kGrpSerif },
			{ &g_prida61Font,     "Prida 61",                kLatin,  kGrpSerif },
			{ &g_steelworksFont,  "Steelworks Vintage",      kLatin,  kGrpSerif },
			{ &g_allessaFont,     "Allessa",                 kLatin,  kGrpScript },
			{ &g_brightMarchFont, "Bright Marching",         kLatin,  kGrpScript },
			{ &g_camoodFont,      "Camood",                  kLatin,  kGrpScript },
			{ &g_cheronaFont,     "Cherona",                 kLatin,  kGrpScript },
			{ &g_daelingFont,     "Daeling",                 kLatin,  kGrpScript },
			{ &g_flowmeryFont,    "Flowmery",                kLatin,  kGrpScript },
			{ &g_galinsFont,      "Galins",                  kLatin,  kGrpScript },
			{ &g_gallanteFont,    "Gallante",                kLatin,  kGrpScript },
			{ &g_kleymisskyFont,  "Kleymissky",              kLatin,  kGrpScript },
			{ &g_loveLightFont,   "Love Light",              kLatin,  kGrpScript },
			{ &g_metaforaAltFont, "Metafora Alternate",      kLatin,  kGrpScript },
			{ &g_metaforaSsFont,  "Metafora Stylistic",      kLatin,  kGrpScript },
			{ &g_migullonFont,    "Migullon",                kLatin,  kGrpScript },
			{ &g_milsskyFont,     "Milssky",                 kLatin,  kGrpScript },
			{ &g_reginaFont,      "Regina",                  kLatin,  kGrpScript },
			{ &g_retroHeartFont,  "Retro Heart You",         kLatin,  kGrpScript },
			{ &g_rosehotFont,     "Rosehot",                 kLatin,  kGrpScript },
			{ &g_sophieFont,      "Sophiemelanie",           kLatin,  kGrpScript },
			{ &g_bollgoFont,      "Bollgo",                  kLatin,  kGrpDisp },
			{ &g_boucherFont,     "Boucher",                 kLatin,  kGrpDisp },
			{ &g_dottedFont,      "Dotted",                  kLatin,  kGrpDisp },
			//{ &g_endlessFont,     "Endlessly Expanded",      kLatin,  kGrpDisp },
			{ &g_franticallyFont, "Frantically",             kLatin,  kGrpDisp },
			{ &g_gimboFont,       "Gimbo",                   kLatin,  kGrpDisp },
			{ &g_gingaFont,       "Ginga",                   kLatin,  kGrpDisp },
			{ &g_magnoliaFont,    "Magnolia Monogram",       kLatin,  kGrpDisp },
			{ &g_molgethFont,     "Molgeth",                 kLatin,  kGrpDisp },
			{ &g_squareLilyFont,  "Square Lily Monogram",    kLatin,  kGrpDisp },
			{ &g_manbowClearFont,        "Manbow Clear",     kLatin,  kGrpCFF },
			{ &g_manbowLinesFont,        "Manbow Lines",     kLatin,  kGrpCFF },
			{ &g_manbowSpotsFont,        "Manbow Spots",     kLatin,  kGrpCFF },
			{ &g_manbowToneFont,         "Manbow Tone",      kLatin,  kGrpCFF },
			{ &g_twemojiFont,            "Twemoji",          kEmoji,  kGrpColr0 },
			{ &g_coralPixelsFont,        "Coral Pixels",     kLatin,  kGrpColr0 },
			{ &g_aquaphonicDownpourFont, "Aquaphonic Downpour", kLatin, kGrpSVG },
			{ &g_aquaphonicDrizzleFont,  "Aquaphonic Drizzle",  kLatin, kGrpSVG },
			{ &g_cimeroProFont,          "Cimero Pro",       kLatin,  kGrpSVG },
			{ &g_colorTubeFont,          "Color Tube",       kLatin,  kGrpSVG },
			{ &g_gilbertColorFont,       "Gilbert Color Bold",kLatin, kGrpSVG },
			{ &g_multicoloreFont,        "Multicolore Pro",  kLatin,  kGrpSVG },
			{ &g_primecolorGFont,        "Primecolor G",     kLatin,  kGrpSVG },
			{ &g_primecolorMFont,        "Primecolor M",     kLatin,  kGrpSVG },
			{ &g_fatternFont,            "Fattern",           kLatin,  kGrpSVG },
			{ &g_nablaFont,              "Nabla",             kLatin,  kGrpColr1 },
			{ &g_primecolorCV1Font,      "Primecolor CV1",   kLatin,  kGrpColr1 },
			{ &g_bungeeSpiceFont,        "Bungee Spice",     kLatin,  kGrpColr1 },
			{ &g_honkFont,               "Honk",              kLatin,  kGrpColr1 },
			{ &g_cairoPlayBoldFont,      "Cairo Play Bold",       kArabic, kGrpArabic },
			{ &g_cairoPlayXLightFont,    "Cairo Play ExtraLight", kArabic, kGrpArabic },
			{ &g_arefRuqaaBoldFont,      "Aref Ruqaa Ink Bold",  kArabic, kGrpArabic },
			{ &g_arefRuqaaRegFont,       "Aref Ruqaa Ink Regular",kArabic, kGrpArabic },
			{ &g_blakaInkFont,           "Blaka Ink",             kArabic, kGrpArabic },
			{ &g_reemKufiInkFont,        "Reem Kufi Ink",         kArabic, kGrpArabic },
			{ &g_reemKufiFunFont,        "Reem Kufi Fun",         kArabic, kGrpArabic },
		};

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		float const canvas_w  = CanvasSize();
		float const gap       = ImGui::GetStyle().ItemSpacing.y;

		// Separate text buffer for emoji (Twemoji uses its own codepoints, not Latin text)
		static char emoji_buf[256] = "\xF0\x9F\x98\x80\xF0\x9F\x94\xA5\xF0\x9F\x8C\x88\xF0\x9F\x8E\xA8\xF0\x9F\x9A\x80\xF0\x9F\x92\xA1\xF0\x9F\x8C\x8D";
		// UTF-8 encoding of: 
		ImGui::InputText( "Emoji##SlugEmoji", emoji_buf, sizeof( emoji_buf ) );
		// Arabic text buffer: الأدوات العزيزة (Dear Widgets)
		static char arabic_buf[256] = "\xd8\xa7\xd9\x84\xd8\xa3\xd8\xaf\xd9\x88\xd8\xa7\xd8\xaa \xd8\xa7\xd9\x84\xd8\xb9\xd8\xb2\xd9\x8a\xd8\xb2\xd8\xa9";
		ImGui::InputText( "Arabic##SlugArabic", arabic_buf, sizeof( arabic_buf ) );

		ImGui::Separator();
		const char* currentGroup = NULL;
		bool groupOpen = false;
		for ( const FontEntry& e : kFonts )
		{
			if ( !*e.font ) continue;

			// Group header
			if ( e.group != currentGroup )
			{
				currentGroup = e.group;
				groupOpen = ImGui::CollapsingHeader( currentGroup );
			}
			if ( !groupOpen ) continue;

			ImFont* f           = *e.font;
			const char* drawStr = (e.textType == kEmoji) ? emoji_buf : (e.textType == kArabic) ? arabic_buf : text_buf;

			float   asc   = 0.0f;
			ImVec2  sz    = ImWidgets::CalcTextSize( f, font_size, drawStr, nullptr, &asc );
			float   line_h = sz.y + gap;

			// Font name in solid black
			ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 0, 0, 0, 255 ) );
			ImGui::Text( "%s:", e.label );
			ImGui::PopStyleColor();

			// Optional colored background spanning the full window width
			ImVec2 pos     = ImGui::GetCursorScreenPos();
			if ( use_bg )
			{
				float win_x0 = ImGui::GetWindowPos().x;
				float win_x1 = win_x0 + ImGui::GetWindowWidth();
				pDrawList->AddRectFilled( ImVec2( win_x0, pos.y ), ImVec2( win_x1, pos.y + sz.y ), bg_color_u );
			}

			// DrawText baseline shifted down by ascent so glyph top aligns with cursor
			if ( use_atlas )
				pDrawList->AddText( f, font_size, ImVec2( pos.x, pos.y ), color_u, drawStr );
			else
				ImWidgets::DrawText( pDrawList, f, font_size, ImVec2( pos.x, pos.y + asc ), color_u, drawStr );
			if ( debug_curves )
			{
				int dbgFlags = ( dbg_curves_on ? 1 : 0 ) | ( dbg_ctrl_on ? 2 : 0 )
				             | ( dbg_bbox_on ? 4 : 0 ) | ( dbg_bands_on ? 8 : 0 );
				ImWidgets::DrawTextDebugCurves( pDrawList, f, font_size, ImVec2( pos.x, pos.y + asc ), drawStr, nullptr, dbgFlags );
			}
			if ( debug_layers )
				ImWidgets::DrawTextDebugLayers( pDrawList, f, font_size, ImVec2( pos.x, pos.y + asc ), drawStr );

			ImGui::Dummy( ImVec2( canvas_w, line_h ) );
		}

		// Typography Fills section (inside GPU Text)
		// ---- Debug Glyph Tessellation ----
		ApplyOpenAll();
		if ( ImGui::CollapsingHeader( "Debug Glyph Tessellation" ) )
		{
			static char dbgChar[8] = "O";
			static int dbgFontIdx = 0;
			static float dbgSize = 200.0f;
			static float dbgTol = 0.5f;

			struct FontChoice { const char* name; ImFont** ptr; };
			static const FontChoice kDbgFonts[] = {
				{ "Monblock", &g_monblockFont },
				{ "Cinzel", &g_cinzelFont },
				{ "Fira Code", &g_firaCodeFont },
				{ "Alfa Slab", &g_alfaSlabFont },
				{ "Frantically", &g_franticallyFont },
				{ "Bollgo", &g_bollgoFont },
				{ "Gimbo", &g_gimboFont },
				{ "Bright Matching", &g_brightMarchFont },
				{ "Gallante", &g_gallanteFont },
				{ "Love Light", &g_loveLightFont },
				{ "Metafora Alternate", &g_metaforaAltFont },
				{ "Metafora Stylistic", &g_metaforaSsFont },
				{ "Migulon", &g_migullonFont },
				{ "Sophiemelanie", &g_sophieFont },
				{ "Classical Aesthetics", &g_classicalFont },
				{ "Prida 61", &g_prida61Font },
				{ "Foglighten No07", &g_foglihtenFont },
				{ "Steelworks Vintage", &g_steelworksFont },
				{ "Square Lily Monogram", &g_squareLilyFont },
				{ "Molgeth", &g_molgethFont },
				{ "Ginga", &g_gingaFont },
				{ "Dotted", &g_dottedFont },
				{ "Boucher", &g_boucherFont },
				{ "Manbow Clear", &g_manbowClearFont },
				{ "Manbow Lines", &g_manbowLinesFont },
				{ "Manbow Spots", &g_manbowSpotsFont },
				{ "Manbow Tone", &g_manbowToneFont },
				{ "Multicolor Pro", &g_multicoloreFont },
				{ "Fattern", &g_fatternFont },
				{ "Aphaphonic Drizzle", &g_aquaphonicDrizzleFont },
				{ "Nabla", &g_nablaFont },
				{ "Bungee Spice", &g_bungeeSpiceFont },
			};
			int numDbgFonts = IM_ARRAYSIZE( kDbgFonts );

			ImGui::InputText( "Character##DbgTess", dbgChar, sizeof( dbgChar ) );
			if ( ImGui::BeginCombo( "Font##DbgTess", kDbgFonts[dbgFontIdx].name ) ) {
				for ( int fi = 0; fi < numDbgFonts; fi++ ) {
					if ( !*kDbgFonts[fi].ptr ) continue;
					if ( ImGui::Selectable( kDbgFonts[fi].name, fi == dbgFontIdx ) ) dbgFontIdx = fi;
				}
				ImGui::EndCombo();
			}
			ImGui::SliderFloat( "Size##DbgTess", &dbgSize, 32.0f, 400.0f, "%.0f px" );
			ImGui::SliderFloat( "Tess Tol##DbgTess", &dbgTol, 0.05f, 5.0f, "%.2f" );
			static float dbgSpacing = 30.0f;
			ImGui::SliderFloat( "Piece Spacing##DbgTess", &dbgSpacing, 0.0f, 100.0f, "%.0f px" );

			ImFont* dbgFont = *kDbgFonts[dbgFontIdx].ptr;
			if ( dbgFont && dbgChar[0] )
			{
				ImVec2 dbgPos = ImGui::GetCursorScreenPos();
				float dbgRowH = dbgSize * 1.3f;
				ImWidgets::DrawTesselateDebug( pDrawList, dbgFont, dbgSize, dbgChar, dbgPos, dbgTol, dbgSpacing, dbgRowH );
			}
		}

		// Typography Fills: tesselated text with gradient/image fills
		ApplyOpenAll();
		if ( g_monblockFont && ImGui::CollapsingHeader( "Typography Fills" ) )
		{
			static int tyFontIdx = 0;
			struct FontChoice { const char* name; ImFont** ptr; };
			static const FontChoice kTypoFonts[] = {
				{ "Monblock", &g_monblockFont },
				{ "Cinzel", &g_cinzelFont },
				{ "Fira Code", &g_firaCodeFont },
				{ "Alfa Slab", &g_alfaSlabFont },
				{ "Frantically", &g_franticallyFont },
				{ "Bollgo", &g_bollgoFont },
				{ "Gimbo", &g_gimboFont },
				{ "Bright Matching", &g_brightMarchFont },
				{ "Gallante", &g_gallanteFont },
				{ "Love Light", &g_loveLightFont },
				{ "Metafora Alternate", &g_metaforaAltFont },
				{ "Metafora Stylistic", &g_metaforaSsFont },
				{ "Migulon", &g_migullonFont },
				{ "Sophiemelanie", &g_sophieFont },
				{ "Classical Aesthetics", &g_classicalFont },
				{ "Prida 61", &g_prida61Font },
				{ "Foglighten No07", &g_foglihtenFont },
				{ "Steelworks Vintage", &g_steelworksFont },
				{ "Square Lily Monogram", &g_squareLilyFont },
				{ "Molgeth", &g_molgethFont },
				{ "Ginga", &g_gingaFont },
				{ "Dotted", &g_dottedFont },
				{ "Boucher", &g_boucherFont },
				{ "Manbow Clear", &g_manbowClearFont },
				{ "Manbow Lines", &g_manbowLinesFont },
				{ "Manbow Spots", &g_manbowSpotsFont },
				{ "Manbow Tone", &g_manbowToneFont },
				{ "Multicolor Pro", &g_multicoloreFont },
				{ "Fattern", &g_fatternFont },
				{ "Aphaphonic Drizzle", &g_aquaphonicDrizzleFont },
				{ "Nabla", &g_nablaFont },
				{ "Bungee Spice", &g_bungeeSpiceFont },
			};
			int numTypoFonts = IM_ARRAYSIZE( kTypoFonts );
			if ( ImGui::BeginCombo( "Font##TypoFills", kTypoFonts[tyFontIdx].name ) ) {
				for ( int fi = 0; fi < numTypoFonts; fi++ ) {
					if ( !*kTypoFonts[fi].ptr ) continue;
					if ( ImGui::Selectable( kTypoFonts[fi].name, fi == tyFontIdx ) ) tyFontIdx = fi;
				}
				ImGui::EndCombo();
			}
			ImFont* tyFont = *kTypoFonts[tyFontIdx].ptr;
			if ( !tyFont ) tyFont = g_monblockFont;
			static float tySize = 64.0f;
			static bool perChar = true;
			static float tessTol = 0.25f;
			static int tyIterations = 2;
			ImGui::SliderFloat( "Typography Size##TypoFills", &tySize, 16.0f, 200.0f, "%.0f px" );
			ImGui::SliderFloat( "Tessellation##TessTol", &tessTol, 0.01f, 2.0f, "%.2f" );
			ImGui::SameLine(); ImGui::TextDisabled( "(lower = smoother)" );
			ImGui::SliderInt( "Iterations##TypoFills", &tyIterations, 0, 6 );
			ImGui::Checkbox( "Per Character##TypoPerChar", &perChar );

			struct TypoEntry { const char* label; int type; ImU32 c0; ImU32 c1; pfSpace2sRGB s2r; pfsRGB2Space r2s; };
			static const TypoEntry kTypo[] = {
				{ "Linear Gradient",  0, IM_COL32(255,50,50,255), IM_COL32(50,50,255,255), NULL, NULL },
				{ "Radial Gradient",  1, IM_COL32(255,255,50,255), IM_COL32(50,200,50,255), NULL, NULL },
				{ "Diamond Gradient", 2, IM_COL32(255,100,255,255), IM_COL32(100,255,255,255), NULL, NULL },
				{ "OkLab Linear",     0, IM_COL32(255,0,0,255), IM_COL32(0,0,255,255), &ImWidgets::ColorConvertOKLABtoRGB, &ImWidgets::ColorConvertRGBtoOKLAB },
			};

			// Helper: render gradient text either whole or per-character
			auto DrawGradText = [&]( const TypoEntry& te, ImVec2 basePos ) {
				if ( !perChar ) {
					if ( te.type == 0 )
						ImWidgets::DrawLinearGradientText( pDrawList, tyFont, tySize, basePos, text_buf, ImVec2(0,0.5f), ImVec2(1,0.5f), te.c0, te.c1, te.s2r, te.r2s, nullptr, tessTol, tyIterations );
					else if ( te.type == 1 )
						ImWidgets::DrawRadialGradientText( pDrawList, tyFont, tySize, basePos, text_buf, ImVec2(0.5f,0.5f), ImVec2(1,0.5f), te.c0, te.c1, te.s2r, te.r2s, nullptr, tessTol, tyIterations );
					else
						ImWidgets::DrawDiamondGradientText( pDrawList, tyFont, tySize, basePos, text_buf, ImVec2(0.5f,0.5f), ImVec2(1,0.5f), te.c0, te.c1, te.s2r, te.r2s, nullptr, tessTol, tyIterations );
				} else {
					// Per-glyph: shape full text (preserves ligatures/calt), then apply gradient per glyph
					ImVector<ImWidgetsShape> glyphShapes;
					ImWidgets::TesselateTextPerGlyph( tyFont, tySize, text_buf, glyphShapes, nullptr, tessTol, tyIterations );
					pfSpace2sRGB s2r = te.s2r ? te.s2r : &ImWidgets::ColorConvertsRGBtosRGB;
					pfsRGB2Space r2s = te.r2s ? te.r2s : &ImWidgets::ColorConvertsRGBtosRGB;
					for ( int gs = 0; gs < glyphShapes.Size; gs++ ) {
						ImWidgetsShape& shape = glyphShapes[gs];
						if ( shape.triangles.Size == 0 ) continue;
						// Offset to screen position
						for ( int vi = 0; vi < shape.vertices.Size; vi++ ) {
							shape.vertices[vi].pos.x += basePos.x;
							shape.vertices[vi].pos.y += basePos.y;
						}
						shape.bb.Translate( basePos );
						// Apply gradient per glyph's own BBox
						if ( te.type == 0 )
							ImWidgets::ShapeLinearGradientGeneric( shape, ImVec2(0,0.5f), ImVec2(1,0.5f), te.c0, te.c1, s2r, r2s );
						else if ( te.type == 1 )
							ImWidgets::ShapeRadialGradientGeneric( shape, ImVec2(0.5f,0.5f), ImVec2(1,0.5f), te.c0, te.c1, s2r, r2s );
						else
							ImWidgets::ShapeDiamondGradientGeneric( shape, ImVec2(0.5f,0.5f), ImVec2(1,0.5f), te.c0, te.c1, s2r, r2s );
						ImWidgets::DrawShape( pDrawList, shape );
					}
				}
			};

			for ( const TypoEntry& te : kTypo )
			{
				ImGui::TextDisabled( "%s", te.label );
				ImVec2 p = ImGui::GetCursorScreenPos();
				float asc2 = 0;
				ImVec2 tsz = ImWidgets::CalcTextSize( tyFont, tySize, text_buf, nullptr, &asc2 );
				DrawGradText( te, ImVec2( p.x, p.y + asc2 ) );
				ImGui::Dummy( ImVec2( tsz.x, tsz.y + gap ) );
			}

			// Image fill text — cycle through all loaded images for per-character
			{
				// Collect all available images
				ImTextureID allImages[8]; int nImages = 0;
				if ( illlustration_img ) allImages[nImages++] = illlustration_img;
				if ( bike_img )          allImages[nImages++] = bike_img;
				if ( astro_img )         allImages[nImages++] = astro_img;
				if ( clock_img )         allImages[nImages++] = clock_img;
				if ( man_img )           allImages[nImages++] = man_img;

				if ( nImages > 0 ) {
					ImGui::TextDisabled( "Image Fill" );
					ImVec2 p = ImGui::GetCursorScreenPos();
					float asc2 = 0;
					ImVec2 tsz = ImWidgets::CalcTextSize( tyFont, tySize, text_buf, nullptr, &asc2 );
					if ( !perChar ) {
						ImWidgets::DrawImageText( pDrawList, tyFont, tySize, ImVec2( p.x, p.y + asc2 ), allImages[0], text_buf, nullptr, IM_COL32_WHITE, ImVec2(0,0), ImVec2(1,1), tessTol, tyIterations );
					} else {
						// Per-glyph image fill: shape full text, one different image per glyph
						ImVector<ImWidgetsShape> glyphShapes;
						ImWidgets::TesselateTextPerGlyph( tyFont, tySize, text_buf, glyphShapes, nullptr, tessTol, tyIterations );
						ImVec2 imgPos( p.x, p.y + asc2 );
						for ( int gs = 0; gs < glyphShapes.Size; gs++ ) {
							ImWidgetsShape& shape = glyphShapes[gs];
							if ( shape.triangles.Size == 0 ) continue;
							for ( int vi = 0; vi < shape.vertices.Size; vi++ ) {
								shape.vertices[vi].pos.x += imgPos.x;
								shape.vertices[vi].pos.y += imgPos.y;
							}
							shape.bb.Translate( imgPos );
							float bbW = ImMax( shape.bb.GetWidth(), 1.0f ), bbH = ImMax( shape.bb.GetHeight(), 1.0f );
							for ( int vi = 0; vi < shape.vertices.Size; vi++ ) {
								ImWidgetsVertex& v = shape.vertices[vi];
								v.uv = ImVec2( (v.pos.x - shape.bb.Min.x) / bbW, (v.pos.y - shape.bb.Min.y) / bbH );
								v.col = IM_COL32_WHITE;
							}
							ImTextureID tex = allImages[gs % nImages]; // cycle through images
							ImWidgets::DrawShapeEx( pDrawList, tex, shape );
						}
					}
					ImGui::Dummy( ImVec2( tsz.x, tsz.y + gap ) );
				}
			}

			// LaTeX equation fills
			{
				ImGui::Separator();
				static float latexFillSize = 40.0f;
				ImGui::SliderFloat( "LaTeX Size##LatexFill", &latexFillSize, 16.0f, 80.0f, "%.0f px" );

				struct LaTeXFillEntry { const char* eq; ImU32 c0; ImU32 c1; };
				static const LaTeXFillEntry kLatexFills[] = {
					{ "E = mc^2",
					  IM_COL32( 255, 80,  50,  255 ), IM_COL32( 80,  80,  255, 255 ) },
					{ "\\frac{-b \\pm \\sqrt{b^2 - 4ac}}{2a}",
					  IM_COL32( 255, 200, 0,   255 ), IM_COL32( 0,   200, 255, 255 ) },
					{ "\\sum_{n=0}^{\\infty} \\frac{x^n}{n!} = e^x",
					  IM_COL32( 200, 100, 255, 255 ), IM_COL32( 255, 200, 50,  255 ) },
					{ "\\int_{-\\infty}^{\\infty} e^{-x^2} dx = \\sqrt{\\pi}",
					  IM_COL32( 50,  220, 150, 255 ), IM_COL32( 255, 100, 200, 255 ) },
					{ "\\frac{1}{\\sigma\\sqrt{2\\pi}} e^{-\\frac{(x-\\mu)^2}{2\\sigma^2}}",
					  IM_COL32( 255, 160, 30,  255 ), IM_COL32( 30,  180, 255, 255 ) },
					{ "i\\hbar\\frac{\\partial}{\\partial t}\\Psi = \\hat{H}\\Psi",
					  IM_COL32( 180, 255, 120, 255 ), IM_COL32( 255,  80, 180, 255 ) },
				};

				for ( int ei = 0; ei < IM_ARRAYSIZE( kLatexFills ); ei++ ) {
					const LaTeXFillEntry& lfe = kLatexFills[ei];
					ImVec2 sz = ImWidgets::CalcLaTeXSize( latexFillSize, lfe.eq );
					if ( sz.x < 1.0f || sz.y < 1.0f ) continue;
					ImVec2 p = ImGui::GetCursorScreenPos();

					ImWidgetsShape latexShape;
					ImWidgets::TesselateLaTeX( latexFillSize, lfe.eq, p, latexShape, tessTol, tyIterations );

					if ( latexShape.triangles.Size > 0 ) {
						ImWidgets::ShapeLinearGradientGeneric( latexShape,
							ImVec2( 0, 0.5f ), ImVec2( 1, 0.5f ), lfe.c0, lfe.c1,
							&ImWidgets::ColorConvertsRGBtosRGB, &ImWidgets::ColorConvertsRGBtosRGB );
						ImWidgets::DrawShape( pDrawList, latexShape );
					}
					ImGui::Dummy( ImVec2( sz.x, sz.y + gap ) );
				}
			}
		}
	}

	void ShowTypographyAnimations()
	{
		ApplyOpenAll();
		if ( !g_dottedFont || !ImGui::CollapsingHeader( "Typography Animations" ) )
			return;

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		ImFont* animFont = g_dottedFont;
		static float animSize = 80.0f;
		static float prevAnimSize = 0;
		float t = (float)ImGui::GetTime();

		ImGui::SliderFloat( "Size##TypoAnim", &animSize, 32.0f, 200.0f, "%.0f px" );
		float gap = 8.0f;

		// --- Cache: tessellate once, reuse every frame ---
		struct AnimCache {
			ImVector<ImWidgetsShape> glyphs; // per-glyph shapes in LOCAL space (not offset)
			ImWidgetsShape whole;             // whole-text shape in LOCAL space
			ImVec2 textSize;
			float ascent;
			bool valid;
		};
		static AnimCache cache[6];
		static const char* kTexts[6] = {
			"Hello World", "Rainbow Wave!", "Blinking Text",
			"Bouncing!", "Pulse", "Typewriter Effect..."
		};

		// Invalidate cache on size change
		bool needRebuild = (animSize != prevAnimSize);
		if ( needRebuild ) {
			prevAnimSize = animSize;
			for ( int i = 0; i < 6; i++ ) cache[i].valid = false;
		}

		// Build cache entries that need it
		float tessTol = 0.25f;
		int iterations = 2;
		for ( int ci = 0; ci < 6; ci++ ) {
			if ( cache[ci].valid ) continue;
			cache[ci].textSize = ImWidgets::CalcTextSize( animFont, animSize, kTexts[ci], nullptr, &cache[ci].ascent );
			cache[ci].glyphs.resize(0);
			ImWidgets::TesselateTextPerGlyph( animFont, animSize, kTexts[ci], cache[ci].glyphs, nullptr, tessTol, iterations );
			// Also build whole-text shape for reveal/pulse
			if ( ci == 0 || ci == 4 ) {
				cache[ci].whole.vertices.resize(0); cache[ci].whole.triangles.resize(0);
				cache[ci].whole.bb = ImRect(FLT_MAX,FLT_MAX,-FLT_MAX,-FLT_MAX);
				ImWidgets::TesselateText( animFont, animSize, kTexts[ci], cache[ci].whole, nullptr, tessTol, iterations );
			}
			cache[ci].valid = true;
		}

		// Helper: draw a cached glyph shape at a screen position with a color
		auto DrawGlyph = [&]( ImWidgetsShape& src, ImVec2 offset, ImU32 col ) {
			if ( src.triangles.Size == 0 ) return;
			// Copy vertices, apply offset + color
			int baseVtx = pDrawList->VtxBuffer.Size;
			int baseIdx = pDrawList->IdxBuffer.Size;
			pDrawList->PrimReserve( src.triangles.Size * 3, src.vertices.Size );
			ImDrawVert* vtx = pDrawList->VtxBuffer.Data + baseVtx;
			ImDrawIdx* idx = pDrawList->IdxBuffer.Data + baseIdx;
			ImVec2 wuv = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
			for ( int vi = 0; vi < src.vertices.Size; vi++ ) {
				vtx[vi].pos = ImVec2( src.vertices[vi].pos.x + offset.x, src.vertices[vi].pos.y + offset.y );
				vtx[vi].uv = wuv;
				vtx[vi].col = col;
			}
			for ( int ti = 0; ti < src.triangles.Size; ti++ ) {
				idx[ti*3+0] = (ImDrawIdx)( baseVtx + src.triangles[ti].a );
				idx[ti*3+1] = (ImDrawIdx)( baseVtx + src.triangles[ti].b );
				idx[ti*3+2] = (ImDrawIdx)( baseVtx + src.triangles[ti].c );
			}
			pDrawList->_VtxWritePtr   += src.vertices.Size;
			pDrawList->_IdxWritePtr   += src.triangles.Size * 3;
			pDrawList->_VtxCurrentIdx += (ImDrawIdx)src.vertices.Size;
		};

		// --- 1. Reveal: linear gradient sweeps left to right ---
		{
			ImGui::TextDisabled( "Reveal (sweep)" );
			ImVec2 p = ImGui::GetCursorScreenPos();
			AnimCache& c = cache[0];
			float sweep = fmodf( t * 0.4f, 1.0f );
			ImVec2 basePos( p.x, p.y + c.ascent );
			ImWidgetsShape tmp;
			tmp.vertices.resize( c.whole.vertices.Size );
			tmp.triangles.resize( c.whole.triangles.Size );
			memcpy( tmp.triangles.Data, c.whole.triangles.Data, c.whole.triangles.Size * sizeof(ImWidgetsTriIdx) );
			tmp.bb = c.whole.bb;
			for ( int vi = 0; vi < c.whole.vertices.Size; vi++ ) {
				tmp.vertices[vi].pos = ImVec2( c.whole.vertices[vi].pos.x + basePos.x, c.whole.vertices[vi].pos.y + basePos.y );
				tmp.vertices[vi].uv = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
			}
			tmp.bb.Translate( basePos );
			float bandW = 0.08f;
			ImWidgets::ShapeLinearGradientGeneric( tmp, ImVec2(sweep-bandW,0.5f), ImVec2(sweep,0.5f),
				IM_COL32(255,200,50,0), IM_COL32(255,200,50,255),
				&ImWidgets::ColorConvertsRGBtosRGB, &ImWidgets::ColorConvertsRGBtosRGB );
			ImWidgets::DrawShape( pDrawList, tmp );
			ImGui::Dummy( ImVec2( c.textSize.x, c.textSize.y + gap ) );
		}

		// --- 2. Rainbow wave ---
		{
			ImGui::TextDisabled( "Rainbow Wave" );
			ImVec2 p = ImGui::GetCursorScreenPos();
			AnimCache& c = cache[1];
			ImVec2 basePos( p.x, p.y + c.ascent );
			for ( int gs = 0; gs < c.glyphs.Size; gs++ ) {
				float hue = fmodf( (float)gs * 0.12f + t * 0.5f, 1.0f );
				ImVec4 hsv( hue, 0.9f, 1.0f, 1.0f );
				ImVec4 rgb; ImGui::ColorConvertHSVtoRGB( hsv.x, hsv.y, hsv.z, rgb.x, rgb.y, rgb.z ); rgb.w = 1.0f;
				DrawGlyph( c.glyphs[gs], basePos, ImGui::GetColorU32( rgb ) );
			}
			ImGui::Dummy( ImVec2( c.textSize.x, c.textSize.y + gap ) );
		}

		// --- 3. Blink ---
		{
			ImGui::TextDisabled( "Blink" );
			ImVec2 p = ImGui::GetCursorScreenPos();
			AnimCache& c = cache[2];
			ImVec2 basePos( p.x, p.y + c.ascent );
			for ( int gs = 0; gs < c.glyphs.Size; gs++ ) {
				float phase = sinf( t * 3.0f + (float)gs * 0.8f );
				int alpha = (int)( ImSaturate( phase * 0.5f + 0.5f ) * 255.0f );
				DrawGlyph( c.glyphs[gs], basePos, IM_COL32( 100, 200, 255, alpha ) );
			}
			ImGui::Dummy( ImVec2( c.textSize.x, c.textSize.y + gap ) );
		}

		// --- 4. Bounce ---
		{
			ImGui::TextDisabled( "Bounce" );
			ImVec2 p = ImGui::GetCursorScreenPos();
			AnimCache& c = cache[3];
			ImVec2 basePos( p.x, p.y + c.ascent );
			float bounceH = animSize * 0.15f;
			for ( int gs = 0; gs < c.glyphs.Size; gs++ ) {
				float bounce = fabsf( sinf( t * 4.0f + (float)gs * 0.6f ) ) * bounceH;
				DrawGlyph( c.glyphs[gs], ImVec2( basePos.x, basePos.y - bounce ), IM_COL32( 255, 140, 60, 255 ) );
			}
			ImGui::Dummy( ImVec2( c.textSize.x, c.textSize.y + bounceH + gap ) );
		}

		// --- 5. Radial pulse ---
		{
			ImGui::TextDisabled( "Radial Pulse" );
			ImVec2 p = ImGui::GetCursorScreenPos();
			AnimCache& c = cache[4];
			ImVec2 basePos( p.x, p.y + c.ascent );
			float pulse = sinf( t * 2.0f ) * 0.3f + 0.7f;
			ImWidgetsShape tmp;
			tmp.vertices.resize( c.whole.vertices.Size );
			tmp.triangles.resize( c.whole.triangles.Size );
			memcpy( tmp.triangles.Data, c.whole.triangles.Data, c.whole.triangles.Size * sizeof(ImWidgetsTriIdx) );
			tmp.bb = c.whole.bb;
			for ( int vi = 0; vi < c.whole.vertices.Size; vi++ ) {
				tmp.vertices[vi].pos = ImVec2( c.whole.vertices[vi].pos.x + basePos.x, c.whole.vertices[vi].pos.y + basePos.y );
				tmp.vertices[vi].uv = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
			}
			tmp.bb.Translate( basePos );
			ImWidgets::ShapeRadialGradientGeneric( tmp, ImVec2(0.5f,0.5f), ImVec2(pulse,0.5f),
				IM_COL32(255,50,255,255), IM_COL32(50,50,255,60),
				&ImWidgets::ColorConvertsRGBtosRGB, &ImWidgets::ColorConvertsRGBtosRGB );
			ImWidgets::DrawShape( pDrawList, tmp );
			ImGui::Dummy( ImVec2( c.textSize.x, c.textSize.y + gap ) );
		}

		// --- 6. Typewriter ---
		{
			ImGui::TextDisabled( "Typewriter" );
			ImVec2 p = ImGui::GetCursorScreenPos();
			AnimCache& c = cache[5];
			ImVec2 basePos( p.x, p.y + c.ascent );
			int totalGlyphs = c.glyphs.Size;
			int visibleCount = (int)fmodf( t * 6.0f, (float)(totalGlyphs + 4) );
			if ( visibleCount > totalGlyphs ) visibleCount = totalGlyphs;
			for ( int gs = 0; gs < visibleCount; gs++ ) {
				bool isCursor = (gs == visibleCount - 1);
				DrawGlyph( c.glyphs[gs], basePos,
					isCursor ? IM_COL32(255,255,255,255) : IM_COL32(200,220,200,255) );
			}
			ImGui::Dummy( ImVec2( c.textSize.x, c.textSize.y + gap ) );
		}
	}

	void ShowLaTeXDemo()
	{
		ApplyOpenAll();
		if ( !ImGui::CollapsingHeader( "LaTeX Math" ) )
			return;

		// User-editable expression
		static char latex_buf[1024] = "L_o(x, \\omega_o) = L_e(x, \\omega_o) + \\int_{\\Omega} f_r(x, \\omega_i, \\omega_o) L_i(x, \\omega_i) \\langle \\omega_i \\cdot n \\rangle_+ d\\omega_i";
		static float latex_size = 24.0f;
		static ImVec4 latex_col_v( 1.0f, 1.0f, 1.0f, 1.0f );
		static ImU32  latex_col_u = IM_COL32( 255, 255, 255, 255 );
		static bool latex_show_bbox = false;

		ImGui::InputTextMultiline( "##LatexInput", latex_buf, sizeof( latex_buf ), ImVec2( -1, ImGui::GetTextLineHeight() * 3 ) );
		ImGui::DragFloat( "Size##LatexSize", &latex_size, 0.5f, 8.0f, 200.0f, "%.0f px" );
		if ( ImGui::ColorEdit4( "Color##LatexColor", &latex_col_v.x ) )
			latex_col_u = ImGui::ColorConvertFloat4ToU32( latex_col_v );
		ImGui::Checkbox( "Show BBox##LatexBBox", &latex_show_bbox );

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();

		// Render the user expression
		{
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImVec2 sz = ImWidgets::CalcLaTeXSize( latex_size, latex_buf );
			float pad = 8.0f;
			pDrawList->AddRectFilled( ImVec2( pos.x - pad, pos.y - pad ), ImVec2( pos.x + sz.x + pad, pos.y + sz.y + pad ), IM_COL32( 30, 30, 40, 255 ), 4.0f );
			ImWidgets::DrawLaTeX( pDrawList, latex_size, pos, latex_col_u, latex_buf );
			if ( latex_show_bbox )
				ImWidgets::DrawLaTeXDebug( pDrawList, latex_size, pos, latex_buf );
			ImGui::Dummy( ImVec2( sz.x + pad * 2, sz.y + pad * 2 ) );
		}

		ImGui::Separator();

		// Static examples organized by category
		struct LaTeXEntry { const char* latex; const char* group; };
		static const char* kGrpClassic  = "Classic Equations";
		static const char* kGrpFrac     = "Fractions & Roots";
		static const char* kGrpMatrix   = "Matrices & Vectors";
		static const char* kGrpDecor    = "Decorations";
		static const char* kGrpEnv      = "Environments";
		static const char* kGrpSymbols  = "Symbols & Accents";
		static const char* kGrpCalc     = "Calculus & Integrals";
		static const LaTeXEntry kExamples[] = {
			// Classic
			{ "E = mc^2", kGrpClassic },
			{ "L_o(x, \\omega_o) = L_e(x, \\omega_o) + \\int_{\\Omega} f_r(x, \\omega_i, \\omega_o) L_i(x, \\omega_i) \\langle \\omega_i \\cdot n \\rangle_+ d\\omega_i", kGrpClassic },
			{ "\\nabla \\times E = -\\frac{\\partial B}{\\partial t}", kGrpClassic },
			// Fractions & Roots
			{ "\\frac{-b \\pm \\sqrt{b^2 - 4ac}}{2a}", kGrpFrac },
			{ "\\sqrt[3]{x} + \\sqrt[n]{a^2+b^2}", kGrpFrac },
			{ "\\binom{n}{k} = \\frac{n!}{k!(n-k)!}", kGrpFrac },
			{ "f(x) = \\frac{1}{\\sigma\\sqrt{2\\pi}} e^{-\\frac{(x-\\mu)^2}{2\\sigma^2}}", kGrpFrac },
			// Matrices
			{ "\\begin{pmatrix} a & b \\\\ c & d \\end{pmatrix}", kGrpMatrix },
			{ "\\begin{bmatrix} 1 & 0 & 0 \\\\ 0 & 1 & 0 \\\\ 0 & 0 & 1 \\end{bmatrix}", kGrpMatrix },
			{ "\\begin{vmatrix} a & b \\\\ c & d \\end{vmatrix} = ad - bc", kGrpMatrix },
			{ "\\begin{pmatrix} \\cos\\theta & -\\sin\\theta \\\\ \\sin\\theta & \\cos\\theta \\end{pmatrix}", kGrpMatrix },
			{ "\\begin{pmatrix} x' \\\\ y' \\end{pmatrix} = \\begin{pmatrix} a & b \\\\ c & d \\end{pmatrix} \\begin{pmatrix} x \\\\ y \\end{pmatrix}", kGrpMatrix },
			{ "\\begin{Bmatrix} a & b \\\\ c & d \\end{Bmatrix}", kGrpMatrix },
			// Decorations
			{ "\\overbrace{a+b+c}^{n} + \\underbrace{x+y}_{2}", kGrpDecor },
			{ "\\overset{def}{=}", kGrpDecor },
			{ "\\boxed{E = mc^2}", kGrpDecor },
			{ "\\cancel{x} + \\bcancel{y} = z", kGrpDecor },
			{ "\\color{red}{\\alpha} + \\color{blue}{\\beta} = \\color{green}{\\gamma}", kGrpDecor },
			// Environments
			{ "|x| = \\begin{cases} x & x \\geq 0 \\\\ -x & x < 0 \\end{cases}", kGrpEnv },
			{ "\\begin{aligned} \\text{Dear} &= b + c \\\\ \\text{Widgets} &= e + f \\end{aligned}", kGrpEnv },
			{ "\\text{if } x > 0 \\text{ then } f(x) = x^2", kGrpEnv },
			// Symbols & Accents
			{ "\\vec{F} = m\\vec{a}", kGrpSymbols },
			{ "\\hat{x} + \\bar{y} + \\dot{z} + \\ddot{w} + \\tilde{n}", kGrpSymbols },
			{ "x \\in \\mathbb{R}, n \\in \\mathbb{N}, z \\in \\mathbb{C}", kGrpSymbols },
			{ "\\forall x \\in \\mathbb{R}, \\exists y : x + y = 0", kGrpSymbols },
			// Calculus & Integrals
			{ "\\sum_{i=0}^{n} x_i", kGrpCalc },
			{ "\\int_0^{\\infty} e^{-x} dx", kGrpCalc },
			{ "\\iint_S \\vec{F} \\cdot d\\vec{S} = \\iiint_V \\nabla \\cdot \\vec{F} \\, dV", kGrpCalc },
		};

		float const pad = 6.0f;
		float const gap = ImGui::GetStyle().ItemSpacing.y;
		const char* currentGroup = NULL;
		bool groupOpen = false;
		for ( const LaTeXEntry& e : kExamples )
		{
			// Group header (collapsed by default)
			if ( e.group != currentGroup )
			{
				currentGroup = e.group;
				ApplyOpenAll();
				groupOpen = ImGui::CollapsingHeader( currentGroup );
			}
			if ( !groupOpen ) continue;

			// LaTeX source label
			ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 160, 160, 160, 255 ) );
			ImGui::TextWrapped( "%s", e.latex );
			ImGui::PopStyleColor();

			// Render the expression
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImVec2 sz = ImWidgets::CalcLaTeXSize( latex_size, e.latex );
			pDrawList->AddRectFilled( ImVec2( pos.x - pad, pos.y ), ImVec2( pos.x + sz.x + pad, pos.y + sz.y + pad ), IM_COL32( 30, 30, 40, 255 ), 4.0f );
			ImWidgets::DrawLaTeX( pDrawList, latex_size, ImVec2( pos.x, pos.y + pad * 0.5f ), latex_col_u, e.latex );
			if ( latex_show_bbox )
				ImWidgets::DrawLaTeXDebug( pDrawList, latex_size, ImVec2( pos.x, pos.y + pad * 0.5f ), e.latex );
			ImGui::Dummy( ImVec2( sz.x + pad * 2, sz.y + pad + gap ) );
		}
	}

	void ShowCustomShaderDemo()
	{
		float const size = CanvasSize();

		static float shape_size = 1.0f;
		static float line_width = 0.05f;
		static float angle = 0.0f;
		static float antialiasing = 0.001f;
		static DemoColor fg_color( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f );
		static DemoColor bg_color( 1.0f, 128.0f / 255.0f, 64.0f / 255.0f );

		ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
		fg_color.Edit( "ColA##CustomShader" );
		ImGui::PopItemWidth();
		ImGui::SameLine();
		bg_color.Edit( "ColB##CustomShader" );
		ImGui::PopItemWidth();
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
		ApplyOpenAll();
		if ( !ImGui::CollapsingHeader( "Draw Squircle" ) )
			return;

		float const size = CanvasSize();
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

	// ---- Showcase: Rendering Equation BRDF Explorer ----
	void ShowShowcase()
	{
		ImGui::SetNextWindowSize( ImVec2( 800, 700 ), ImGuiCond_FirstUseEver );
		if ( !ImGui::Begin( "Showcase" ) ) { ImGui::End(); return; }

		ImGui::TextDisabled( "Rendering Equation — BRDF Explorer" );
		ImGui::Separator();

		// Material parameters
		static float roughness = 0.4f;
		static float f0 = 0.04f;
		static float albedo = 0.8f;
		static float light_angle = 45.0f;
		static float view_angle = 30.0f;

		ImGui::SliderFloat( "Roughness", &roughness, 0.01f, 1.0f );
		ImGui::SliderFloat( "F0 (Fresnel)", &f0, 0.0f, 1.0f );
		ImGui::SliderFloat( "Albedo", &albedo, 0.0f, 1.0f );
		ImGui::SliderFloat( "Light Angle", &light_angle, 0.0f, 89.0f, "%.0f deg" );
		ImGui::SliderFloat( "View Angle", &view_angle, 0.0f, 89.0f, "%.0f deg" );

		// Static state (colors, draw options) — declared here, UI shown after schema
		static ImVec4 cvLo    = ImVec4( 1.00f, 0.86f, 0.31f, 1.0f );
		static ImVec4 cvLe    = ImVec4( 1.00f, 0.63f, 0.20f, 1.0f );
		static ImVec4 cvFr    = ImVec4( 0.31f, 0.86f, 0.47f, 1.0f );
		static ImVec4 cvLi    = ImVec4( 0.39f, 0.71f, 1.00f, 1.0f );
		static ImVec4 cvCos   = ImVec4( 1.00f, 0.39f, 0.39f, 1.0f );
		static ImVec4 cvN     = ImVec4( 0.78f, 0.78f, 1.00f, 1.0f );
		static ImVec4 cvH     = ImVec4( 0.71f, 0.51f, 1.00f, 1.0f );
		static ImVec4 cvSurf  = ImVec4( 0.71f, 0.71f, 0.71f, 1.0f );
		static float line_thick = 2.0f;
		static float arrow_thick = 2.0f;
		static float lobe_thick = 2.0f;
		static float hemi_thick = 1.5f;
		static float hemi_dash = 8.0f;
		static float hemi_gap = 5.0f;
		static float brdf_scale = 1.0f;
		static float schema_scale = 1.0f;
		static float label_scale = 1.0f;
		static bool show_diffuse = true;
		static bool show_specular = true;
		static bool show_total = true;
		static bool show_half_vec = true;
		static bool show_reflection = true;
		static bool show_cos_arc = true;

		// Convert ImVec4 colors to ImU32
		ImU32 colLo    = ImGui::ColorConvertFloat4ToU32( cvLo );
		ImU32 colLe    = ImGui::ColorConvertFloat4ToU32( cvLe );
		ImU32 colFr    = ImGui::ColorConvertFloat4ToU32( cvFr );
		ImU32 colLi    = ImGui::ColorConvertFloat4ToU32( cvLi );
		ImU32 colCos   = ImGui::ColorConvertFloat4ToU32( cvCos );
		ImU32 colN     = ImGui::ColorConvertFloat4ToU32( cvN );
		ImU32 colOmegI = colLi;
		ImU32 colOmegO = colLo;
		ImU32 colH     = ImGui::ColorConvertFloat4ToU32( cvH );
		ImU32 colSurf  = ImGui::ColorConvertFloat4ToU32( cvSurf );

		// Build hex color strings for LaTeX \color{#RRGGBB}
		auto ToHex = []( ImVec4 c, char* buf ) {
			snprintf( buf, 8, "#%02X%02X%02X", (int)(c.x*255), (int)(c.y*255), (int)(c.z*255) );
		};
		char hexLo[8], hexLe[8], hexFr[8], hexLi[8], hexCos[8];
		ToHex( cvLo, hexLo ); ToHex( cvLe, hexLe ); ToHex( cvFr, hexFr );
		ToHex( cvLi, hexLi ); ToHex( cvCos, hexCos );

		float eqSize = 64.0f;

		// ---- Side-view BRDF diagram ----
		float diagramW = ImGui::GetContentRegionAvail().x;
		float diagramH = 450.0f * schema_scale;
		ImVec2 canvasPos = ImGui::GetCursorScreenPos();
		ImGui::Dummy( ImVec2( diagramW, diagramH ) );
		ImDrawList* dl = ImGui::GetWindowDrawList();

		// Coordinate system: center of surface
		float cx = canvasPos.x + diagramW * 0.5f;
		float cy = canvasPos.y + diagramH * 0.55f; // surface line (higher to leave room for equation below)
		float hemiR = diagramH * 0.42f; // hemisphere radius

		// Background + clip rect to keep everything inside the canvas
		dl->AddRectFilled( canvasPos, ImVec2( canvasPos.x + diagramW, canvasPos.y + diagramH ), IM_COL32( 20, 22, 30, 255 ), 4.0f );
		dl->PushClipRect( canvasPos, ImVec2( canvasPos.x + diagramW, canvasPos.y + diagramH ), true );

		// Surface line
		float surfL = cx - hemiR * 1.3f;
		float surfR = cx + hemiR * 1.3f;
		dl->AddLine( ImVec2( surfL, cy ), ImVec2( surfR, cy ), colSurf, 2.0f );

		// Surface with roughness visualization
		// Smooth surface = straight line, rough = wavy/noisy
		{
			int surfSegs = 128;
			ImVec2 surfPts[129];
			for ( int i = 0; i <= surfSegs; i++ )
			{
				float t = (float)i / (float)surfSegs;
				float sx = surfL + t * ( surfR - surfL );
				// Roughness-dependent waviness using hash-like displacement
				float disp = 0.0f;
				if ( roughness > 0.05f )
				{
					float freq1 = 17.3f, freq2 = 43.7f, freq3 = 97.1f;
					disp = sinf( sx * freq1 * 0.05f ) * 0.5f + sinf( sx * freq2 * 0.05f ) * 0.3f + sinf( sx * freq3 * 0.05f ) * 0.2f;
					disp *= roughness * roughness * 4.0f;
				}
				surfPts[i] = ImVec2( sx, cy + disp );
			}
			dl->AddPolyline( surfPts, surfSegs + 1, colSurf, 0, line_thick );
		}

		// Surface hatching (below surface)
		for ( float hx = surfL; hx < surfR; hx += 8.0f )
			dl->AddLine( ImVec2( hx, cy ), ImVec2( hx - 6.0f, cy + 6.0f ), IM_COL32( 100, 100, 100, 120 ), 1.0f );

		// Hemisphere outline (dashed, thicker)
		{
			int hemiSegs = 64;
			ImVec2 hemiPts[65];
			for ( int i = 0; i <= hemiSegs; i++ )
			{
				float a = IM_PI * (float)i / (float)hemiSegs;
				hemiPts[i] = ImVec2( cx - cosf( a ) * hemiR, cy - sinf( a ) * hemiR );
			}
			ImWidgets::DrawDashedPolylineAA( dl, hemiPts, hemiSegs + 1,
				IM_COL32( 80, 80, 100, 180 ), hemi_thick, hemi_dash, hemi_gap, 0.0f );
		}

		// Convert angles to radians
		float lightRad = light_angle * IM_PI / 180.0f;
		float viewRad = view_angle * IM_PI / 180.0f;

		// Direction vectors (in screen space: up = -Y)
		float liDirX = -sinf( lightRad ), liDirY = -cosf( lightRad ); // incoming from left
		float loDirX = sinf( viewRad ), loDirY = -cosf( viewRad ); // outgoing to right

		// Half vector H = normalize(wi + wo)
		float hx2 = liDirX + loDirX, hy2 = liDirY + loDirY;
		float hLen = sqrtf( hx2 * hx2 + hy2 * hy2 );
		if ( hLen > 0.001f ) { hx2 /= hLen; hy2 /= hLen; }

		float arrowLen = hemiR * 0.85f;
		float arrowHead = 8.0f;

		float labelSz = 14.0f * schema_scale * label_scale; // LaTeX label size

		// Helper: draw arrow with triangle tip and LaTeX label
		auto DrawArrow = [&]( float dx, float dy, float len, ImU32 col, const char* latexLabel, bool incoming ) {
			float ex = cx + dx * len;
			float ey = cy + dy * len;
			float perpX = -dy, perpY = dx;
			if ( incoming ) {
				dl->AddLine( ImVec2( ex, ey ), ImVec2( cx, cy ), col, arrow_thick );
				dl->AddTriangleFilled(
					ImVec2( cx, cy ),
					ImVec2( cx + dx * arrowHead - perpX * arrowHead * 0.4f, cy + dy * arrowHead - perpY * arrowHead * 0.4f ),
					ImVec2( cx + dx * arrowHead + perpX * arrowHead * 0.4f, cy + dy * arrowHead + perpY * arrowHead * 0.4f ),
					col );
			} else {
				dl->AddLine( ImVec2( cx, cy ), ImVec2( ex, ey ), col, arrow_thick );
				dl->AddTriangleFilled(
					ImVec2( ex, ey ),
					ImVec2( ex - dx * arrowHead - perpX * arrowHead * 0.4f, ey - dy * arrowHead - perpY * arrowHead * 0.4f ),
					ImVec2( ex - dx * arrowHead + perpX * arrowHead * 0.4f, ey - dy * arrowHead + perpY * arrowHead * 0.4f ),
					col );
			}
			float labelOff = 14.0f * label_scale;
			ImVec2 lsz = ImWidgets::CalcLaTeXSize( labelSz, latexLabel );
			ImWidgets::DrawLaTeX( dl, labelSz, ImVec2( ex + dx * labelOff - lsz.x * 0.5f, ey + dy * labelOff - lsz.y * 0.5f ), col, latexLabel );
		};

		// Surface point marker (cross at origin)
		float mkSz = 6.0f * schema_scale;
		dl->AddLine( ImVec2( cx - mkSz, cy - mkSz ), ImVec2( cx + mkSz, cy + mkSz ), colSurf, 2.0f );
		dl->AddLine( ImVec2( cx - mkSz, cy + mkSz ), ImVec2( cx + mkSz, cy - mkSz ), colSurf, 2.0f );

		// Normal vector (up)
		DrawArrow( 0, -1, arrowLen * 0.6f, colN, "\\vec{n}", false );

		// Incoming light direction (omega_i)
		DrawArrow( liDirX, liDirY, arrowLen, colOmegI, "\\omega_i", true );

		// Outgoing view direction (omega_o / L_o)
		DrawArrow( loDirX, loDirY, arrowLen, colOmegO, "\\omega_o", false );

		// Half vector
		if ( show_half_vec )
			DrawArrow( hx2, hy2, arrowLen * 0.5f, colH, "H", false );

		// Cosine theta_i indicator (arc from normal to omega_i)
		if ( show_cos_arc ) {
			float arcR = arrowLen * 0.25f;
			int arcSegs = 16;
			for ( int i = 0; i < arcSegs; i++ )
			{
				float t0 = (float)i / (float)arcSegs;
				float t1 = (float)( i + 1 ) / (float)arcSegs;
				float a0 = -IM_PI * 0.5f - lightRad * t0;
				float a1 = -IM_PI * 0.5f - lightRad * t1;
				dl->AddLine(
					ImVec2( cx + cosf( a0 ) * arcR, cy + sinf( a0 ) * arcR ),
					ImVec2( cx + cosf( a1 ) * arcR, cy + sinf( a1 ) * arcR ),
					colCos, 1.5f );
			}
			float labelA = -IM_PI * 0.5f - lightRad * 0.5f;
			float lx2 = cx + cosf( labelA ) * ( arcR + 16.0f );
			float ly2 = cy + sinf( labelA ) * ( arcR + 16.0f );
			ImVec2 csz = ImWidgets::CalcLaTeXSize( labelSz, "\\cos\\theta_i" );
			ImWidgets::DrawLaTeX( dl, labelSz, ImVec2( lx2 - csz.x * 0.5f, ly2 - csz.y * 0.5f ), colCos, "\\cos\\theta_i" );
		}

		// ---- PBR BRDF lobe: f_r = diffuse/pi + D*G*F / (4*NdotL*NdotV) ----
		{
			float alpha = roughness * roughness;
			float alpha2 = alpha * alpha;
			// GGX Smith G1 helper
			auto SmithG1 = []( float NdotX, float a2 ) -> float {
				if ( NdotX <= 0.0f ) return 0.0f;
				float n2 = NdotX * NdotX;
				return 2.0f * NdotX / ( NdotX + sqrtf( a2 + ( 1.0f - a2 ) * n2 ) );
			};

			// Evaluate Cook-Torrance specular BRDF for a given outgoing angle theta_o
			// with fixed incoming light at lightRad from normal
			// Evaluate specular BRDF for outgoing angle theta_o from normal.
			// Light comes from LEFT at lightRad from normal.
			// Signed convention: negative = left, positive = right.
			// wi signed angle = -lightRad, wo signed angle = theta_o.
			auto EvalSpecular = [&]( float theta_o ) -> float {
				float NdotL = cosf( lightRad );
				float NdotV = cosf( theta_o );
				if ( NdotL <= 0.0f || NdotV <= 0.0f ) return 0.0f;
				// H = normalize(wi + wo); wi tangent = -sin(lightRad), wo tangent = sin(theta_o)
				float hTan = -sinf( lightRad ) + sinf( theta_o );
				float hNrm = cosf( lightRad ) + cosf( theta_o );
				float hAngle = atan2f( hTan, hNrm );
				float cosH = cosf( hAngle );
				if ( cosH <= 0.0f ) return 0.0f;
				float cos2H = cosH * cosH;
				float denomNDF = cos2H * ( alpha2 - 1.0f ) + 1.0f;
				float D = alpha2 / ( IM_PI * denomNDF * denomNDF );
				float G = SmithG1( NdotL, alpha2 ) * SmithG1( NdotV, alpha2 );
				float VdotH = cosf( theta_o - hAngle );
				float F = f0 + ( 1.0f - f0 ) * powf( ImMax( 1.0f - VdotH, 0.0f ), 5.0f );
				float denomBrdf = 4.0f * NdotL * NdotV;
				return ( denomBrdf > 0.001f ) ? D * G * F / denomBrdf : 0.0f;
			};

			// Diffuse term: albedo / pi (Lambertian)
			float diffuse = albedo / IM_PI;

			// Find max value for log normalization (sweep outgoing directions)
			float maxVal = diffuse;
			int lobeSegs = 100;
			for ( int i = 0; i <= lobeSegs; i++ )
			{
				// theta_o = outgoing angle from normal, sweep -pi/2 to +pi/2
				float theta_o = ( (float)i / (float)lobeSegs - 0.5f ) * IM_PI * 0.99f;
				float cosT = cosf( theta_o );
				float spec = EvalSpecular( theta_o );
				float total = diffuse * ImMax( cosT, 0.0f ) + spec;
				if ( total > maxVal ) maxVal = total;
			}
			float logMax = logf( maxVal + 1.0f );
			if ( logMax < 0.01f ) logMax = 0.01f;

			ImVec2 prevSpec( 0, 0 ), prevDiff( 0, 0 ), prevTotal( 0, 0 );

			for ( int i = 0; i <= lobeSegs; i++ )
			{
				float theta_o = ( (float)i / (float)lobeSegs - 0.5f ) * IM_PI * 0.99f;
				float cosT = cosf( theta_o );
				float spec = EvalSpecular( theta_o );

				// Log-space radii
				float lobeR = hemiR * 0.7f * brdf_scale;
				float diffCos = diffuse * ImMax( cosT, 0.0f );
				float rSpec  = ( logf( spec + 1.0f ) / logMax ) * lobeR;
				float rDiff  = ( logf( diffCos + 1.0f ) / logMax ) * lobeR;
				float rTotal = ( logf( diffCos + spec + 1.0f ) / logMax ) * lobeR;

				float pAngle = -IM_PI * 0.5f + theta_o;
				float ca = cosf( pAngle ), sa = sinf( pAngle );

				ImVec2 ptSpec( cx + ca * rSpec, cy + sa * rSpec );
				ImVec2 ptDiff( cx + ca * rDiff, cy + sa * rDiff );
				ImVec2 ptTotal( cx + ca * rTotal, cy + sa * rTotal );

				if ( i > 0 )
				{
					if ( show_diffuse )  dl->AddLine( prevDiff, ptDiff, colLe, lobe_thick );
					if ( show_specular ) dl->AddLine( prevSpec, ptSpec, colFr, lobe_thick );
					if ( show_total )    dl->AddLine( prevTotal, ptTotal, IM_COL32( 255, 255, 255, 200 ), lobe_thick * 0.7f );
				}
				prevSpec = ptSpec;
				prevDiff = ptDiff;
				prevTotal = ptTotal;
			}

			// Reflection direction indicator (dashed line)
			if ( show_reflection ) {
				float reflX = sinf( lightRad ), reflY = -cosf( lightRad );
				ImVec2 reflPts[2] = { ImVec2( cx, cy ), ImVec2( cx + reflX * hemiR * 0.7f, cy + reflY * hemiR * 0.7f ) };
				ImWidgets::DrawDashedPolylineAA( dl, reflPts, 2, IM_COL32( 255, 255, 255, 60 ), 1.0f, 4.0f, 3.0f, 0.0f );
			}

			// Labels using LaTeX + Asterisk marker to link schema to formula
			{
				const char* specLabel = "D \\cdot G \\cdot F";
				ImVec2 slsz = ImWidgets::CalcLaTeXSize( labelSz, specLabel );
				float specLabelX = cx - slsz.x * 0.5f;
				float specLabelY = cy - hemiR * 0.82f - slsz.y;
				// Asterisk marker to the left of the label
				float astSz = labelSz * 1.2f;
				ImWidgets::DrawMarker( dl, ImVec2( specLabelX - astSz - 2.0f, specLabelY + ( slsz.y - astSz ) * 0.5f ), ImVec2( astSz, astSz ),
					colFr, IM_COL32( 0, 0, 0, 0 ), 0, 0.8f, 0.0f, 0.002f,
					ImWidgetsMarker_Asterisk, ImWidgetsDrawType_Filled );
				ImWidgets::DrawLaTeX( dl, labelSz, ImVec2( specLabelX, specLabelY ), colFr, specLabel );
			}
			{
				const char* diffLabel = "\\frac{\\text{albedo}}{\\pi}";
				ImWidgets::DrawLaTeX( dl, labelSz, ImVec2( cx + hemiR * 0.35f, cy - hemiR * 0.4f ), colLe, diffLabel );
			}
		}

		// Rendering equation below the surface line, inside the canvas
		{
			char coloredEq[512];
			snprintf( coloredEq, sizeof( coloredEq ),
				"\\color{%s}{L_o}(x, \\color{%s}{\\omega_o}) = "
				"\\color{%s}{L_e} + \\int_{\\Omega} "
				"\\color{%s}{f_r} \\cdot "
				"\\color{%s}{L_i} \\cdot "
				"\\color{%s}{|\\omega_i \\cdot n|}"
				" \\, d\\omega_i",
				hexLo, hexLo, hexLe, hexFr, hexLi, hexCos );
			float eqRenderSz = eqSize * schema_scale * 0.5f;
			ImVec2 eqSz = ImWidgets::CalcLaTeXSize( eqRenderSz, coloredEq );
			float eqX = cx - eqSz.x * 0.5f; // center horizontally
			float eqY = cy + 12.0f * schema_scale; // just below surface + hatching
			ImWidgets::DrawLaTeX( dl, eqRenderSz, ImVec2( eqX, eqY ), IM_COL32( 200, 200, 200, 255 ), coloredEq );
		}

		dl->PopClipRect();

		// ---- Formulas using LaTeX rendering ----
		ImGui::Spacing();
		ImDrawList* fmDl = ImGui::GetWindowDrawList();
		float fmSize = eqSize * 0.75f; // formulas at 3/4 of main equation size
		float pad2 = 6.0f;

		// Helper: render a LaTeX formula with dark background
		auto DrawFormula = [&]( const char* latex, float sz2 ) {
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImVec2 fsz = ImWidgets::CalcLaTeXSize( sz2, latex );
			fmDl->AddRectFilled( ImVec2( p.x - pad2, p.y - pad2 ), ImVec2( p.x + fsz.x + pad2, p.y + fsz.y + pad2 ), IM_COL32( 20, 22, 30, 255 ), 4.0f );
			ImWidgets::DrawLaTeX( fmDl, sz2, p, IM_COL32( 220, 220, 220, 255 ), latex );
			ImGui::Dummy( ImVec2( fsz.x + pad2 * 2, fsz.y + pad2 * 2 + 2.0f ) );
		};

		// Full BRDF formula with Asterisk marker on the left (links to schema lobe)
		{ char buf[512]; snprintf( buf, sizeof(buf),
			"\\color{%s}{f_r} = "
			"\\color{%s}{\\frac{\\text{albedo}}{\\pi}} + "
			"\\frac{\\color{%s}{D_{GGX}} \\cdot \\color{%s}{G_{Smith}} \\cdot \\color{%s}{F_{Schlick}}}"
			"{4 \\cdot \\color{%s}{|\\omega_i \\cdot n|} \\cdot \\color{%s}{|\\omega_o \\cdot n|}}",
			hexFr, hexLe, hexFr, hexFr, hexFr, hexCos, hexCos );
		ImVec2 fPos = ImGui::GetCursorScreenPos();
		ImVec2 fSz2 = ImWidgets::CalcLaTeXSize( fmSize, buf );
		float astFmSz = fmSize * 0.6f;
		ImWidgets::DrawMarker( fmDl, ImVec2( fPos.x - pad2, fPos.y + ( fSz2.y - astFmSz ) * 0.5f ), ImVec2( astFmSz, astFmSz ),
			colFr, IM_COL32( 0, 0, 0, 0 ), 0, 0.8f, 0.0f, 0.002f,
			ImWidgetsMarker_Asterisk, ImWidgetsDrawType_Filled );
		// Indent the formula to make room for the asterisk
		ImGui::Indent( astFmSz + 4.0f );
		DrawFormula( buf, fmSize );
		ImGui::Unindent( astFmSz + 4.0f ); }

		ImGui::Spacing();

		// ---- Two-column layout: formulas left, values right ----
		if ( ImGui::BeginTable( "##BRDFColumns", 2, ImGuiTableFlags_None ) )
		{
			ImGui::TableSetupColumn( "Formulas", ImGuiTableColumnFlags_WidthStretch, 0.55f );
			ImGui::TableSetupColumn( "Values", ImGuiTableColumnFlags_WidthStretch, 0.45f );

			ImGui::TableNextRow();

			// Left column: component formulas
			ImGui::TableSetColumnIndex( 0 );

			{ char buf[256]; snprintf( buf, sizeof(buf),
				"\\color{%s}{D_{GGX}} = "
				"\\frac{\\alpha^2}{\\pi (\\cos^2\\theta_h (\\alpha^2 - 1) + 1)^2}", hexFr );
			DrawFormula( buf, fmSize * 0.65f ); }

			{ char buf[256]; snprintf( buf, sizeof(buf),
				"\\color{%s}{G_{Smith}}(v) = "
				"\\frac{2 (n \\cdot v)}{(n \\cdot v) + \\sqrt{\\alpha^2 + (1 - \\alpha^2)(n \\cdot v)^2}}", hexFr );
			DrawFormula( buf, fmSize * 0.65f ); }

			{ char buf[256]; snprintf( buf, sizeof(buf),
				"\\color{%s}{F_{Schlick}} = "
				"F_0 + (1 - F_0)(1 - \\cos\\theta_h)^5", hexFr );
			DrawFormula( buf, fmSize * 0.65f ); }

			// Right column: computed values
			ImGui::TableSetColumnIndex( 1 );

			{
				float NdotL2 = cosf( lightRad ), NdotV2 = cosf( viewRad );
				float cosH2 = cosf( ( lightRad + viewRad ) * 0.5f );
				float al = roughness * roughness;
				float al2 = al * al;
				float denomD = cosH2 * cosH2 * ( al2 - 1.0f ) + 1.0f;
				float Dval = al2 / ( IM_PI * denomD * denomD );
				float G1L = 2.0f * NdotL2 / ( NdotL2 + sqrtf( al2 + ( 1.0f - al2 ) * NdotL2 * NdotL2 ) );
				float G1V = 2.0f * NdotV2 / ( NdotV2 + sqrtf( al2 + ( 1.0f - al2 ) * NdotV2 * NdotV2 ) );
				float Gval = G1L * G1V;
				float Fval = f0 + ( 1.0f - f0 ) * powf( 1.0f - cosH2, 5.0f );
				float denomBrdf = 4.0f * ImMax( NdotL2, 0.001f ) * ImMax( NdotV2, 0.001f );
				float specVal = Dval * Gval * Fval / denomBrdf;
				float diffVal = albedo / IM_PI;
				float totalVal = diffVal + specVal;

				char dynBuf[512];
				snprintf( dynBuf, sizeof( dynBuf ),
					"\\alpha = %.3f \\quad "
					"N \\cdot L = %.3f",
					al, NdotL2 );
				DrawFormula( dynBuf, fmSize * 0.55f );

				snprintf( dynBuf, sizeof( dynBuf ),
					"N \\cdot V = %.3f \\quad "
					"\\theta_h = %.1f",
					NdotV2, ( lightRad + viewRad ) * 0.5f * 180.0f / IM_PI );
				DrawFormula( dynBuf, fmSize * 0.55f );

				snprintf( dynBuf, sizeof( dynBuf ),
					"\\color{%s}{D} = %.2f \\quad "
					"\\color{%s}{G} = %.3f \\quad "
					"\\color{%s}{F} = %.3f",
					hexFr, Dval, hexFr, Gval, hexFr, Fval );
				DrawFormula( dynBuf, fmSize * 0.55f );

				snprintf( dynBuf, sizeof( dynBuf ),
					"\\color{%s}{\\text{Diff}} = %.3f \\quad "
					"\\color{%s}{\\text{Spec}} = %.3f",
					hexLe, diffVal, hexFr, specVal );
				DrawFormula( dynBuf, fmSize * 0.55f );

				snprintf( dynBuf, sizeof( dynBuf ),
					"\\text{Total} = %.3f", totalVal );
				DrawFormula( dynBuf, fmSize * 0.55f );
			}

			ImGui::EndTable();
		}

		ImGui::Spacing();
		// ---- Display Options (after schema and formulas) ----
		ImGui::Spacing();
		if ( ImGui::CollapsingHeader( "Display Options" ) )
		{
			ImGui::SliderFloat( "Schema Scale", &schema_scale, 0.5f, 2.0f, "%.1fx" );
			ImGui::SliderFloat( "Label Scale", &label_scale, 0.5f, 3.0f, "%.1fx" );
			ImGui::SliderFloat( "BRDF Scale", &brdf_scale, 0.1f, 5.0f, "%.1fx" );

			ImGui::Spacing();
			ImGui::TextDisabled( "Line Style" );
			ImGui::SliderFloat( "Arrow Thickness", &arrow_thick, 0.5f, 5.0f );
			ImGui::SliderFloat( "Lobe Thickness", &lobe_thick, 0.5f, 5.0f );
			ImGui::SliderFloat( "Surface Thickness", &line_thick, 0.5f, 5.0f );
			ImGui::SliderFloat( "Hemisphere Thickness", &hemi_thick, 0.5f, 4.0f );
			ImGui::SliderFloat( "Hemisphere Dash", &hemi_dash, 2.0f, 20.0f );
			ImGui::SliderFloat( "Hemisphere Gap", &hemi_gap, 1.0f, 15.0f );

			ImGui::Spacing();
			ImGui::TextDisabled( "Visibility" );
			ImGui::Checkbox( "Diffuse Lobe", &show_diffuse );
			ImGui::SameLine();
			ImGui::Checkbox( "Specular Lobe", &show_specular );
			ImGui::SameLine();
			ImGui::Checkbox( "Total BRDF", &show_total );
			ImGui::Checkbox( "Half Vector", &show_half_vec );
			ImGui::SameLine();
			ImGui::Checkbox( "Reflection Dir", &show_reflection );
			ImGui::SameLine();
			ImGui::Checkbox( "Cos Arc", &show_cos_arc );

			ImGui::Spacing();
			ImGui::TextDisabled( "Colors" );
			ImGui::ColorEdit4( "L_o / wo##colLo", &cvLo.x, ImGuiColorEditFlags_NoInputs );
			ImGui::SameLine();
			ImGui::ColorEdit4( "L_i / wi##colLi", &cvLi.x, ImGuiColorEditFlags_NoInputs );
			ImGui::SameLine();
			ImGui::ColorEdit4( "f_r / Spec##colFr", &cvFr.x, ImGuiColorEditFlags_NoInputs );
			ImGui::SameLine();
			ImGui::ColorEdit4( "Diff##colLe", &cvLe.x, ImGuiColorEditFlags_NoInputs );
			ImGui::ColorEdit4( "Normal##colN", &cvN.x, ImGuiColorEditFlags_NoInputs );
			ImGui::SameLine();
			ImGui::ColorEdit4( "Half##colH", &cvH.x, ImGuiColorEditFlags_NoInputs );
			ImGui::SameLine();
			ImGui::ColorEdit4( "Cosine##colCos", &cvCos.x, ImGuiColorEditFlags_NoInputs );
			ImGui::SameLine();
			ImGui::ColorEdit4( "Surface##colSurf", &cvSurf.x, ImGuiColorEditFlags_NoInputs );
		}

		ImGui::End();
	}

	void	ShowDemo()
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::SetNextWindowBgAlpha( 0.75f );
		ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 16 );
		ImGui::Begin( "Dear Widgets", NULL, ImGuiWindowFlags_NoTitleBar );
		ImWidgets::SetCurrentWindowBackgroundImage( background, background_size, false, IM_COL32(255, 255, 255, 128) );


		// ─── Open / Close All ──────────────────────────────────────────────
		if ( ImGui::Button( "Open All" ) )  { s_open_all =  1; }
		ImGui::SameLine();
		if ( ImGui::Button( "Close All" ) ) { s_open_all = -1; ImGui::SetScrollY( 0.0f ); }

		ApplyOpenAll();
		if ( ImGui::CollapsingHeader( "Draw" ) )
		{
			ShowDrawShapeDemo();
#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
			ShowDrawTextDemo();
			ShowTypographyAnimations();
			ShowLaTeXDemo();
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Custom Shader" ) )
			{
				ImGui::Indent();
				ShowCustomShaderDemo();
				ApplyOpenAll();
				if ( ImGui::TreeNode( "Primitives##Draw" ) )
				{
				ApplyOpenAll();
				if ( ImGui::CollapsingHeader( "Thick line" ) )
				{
					float const size = CanvasSize();
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
				ShowDrawSquircleDemo();
				ImGui::TreePop();
				}
				ImGui::Unindent();
			}
#endif
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Gradients##Draw" ) )
			{
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Linear Gradient" ) )
			{
				float const size = CanvasSize();
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
				ImGui::PopItemWidth();
				ImGui::SameLine();
				Slider2DFloat( "uv1", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::PopItemWidth();
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &cola_v.x ) )
					cola = ImGui::GetColorU32( cola_v );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit4( "ColB##DrawShape", &colb_v.x ) )
					colb = ImGui::GetColorU32( colb_v );
				ImGui::PopItemWidth();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Radial Gradient" ) )
			{
				float const size = CanvasSize();
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
				ImGui::PopItemWidth();
				ImGui::SameLine();
				Slider2DFloat( "uv1", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::PopItemWidth();
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &cola_v.x ) )
					cola = ImGui::GetColorU32( cola_v );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit4( "ColB##DrawShape", &colb_v.x ) )
					colb = ImGui::GetColorU32( colb_v );
				ImGui::PopItemWidth();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Diamond Gradient" ) )
			{
				float const size = CanvasSize();
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
				ImGui::PopItemWidth();
				ImGui::SameLine();
				Slider2DFloat( "uv1", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::PopItemWidth();
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &cola_v.x ) )
					cola = ImGui::GetColorU32( cola_v );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit4( "ColB##DrawShape", &colb_v.x ) )
					colb = ImGui::GetColorU32( colb_v );
				ImGui::PopItemWidth();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Image Shape" ) )
			{
				float const size = CanvasSize();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Image Shape Gradient" ) )
			{
				float const size = CanvasSize();
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
				ImGui::PopItemWidth();
				ImGui::SameLine();
				Slider2DFloat( "uv1", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
				ImGui::PopItemWidth();
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit4( "ColA##DrawShape", &cola_v.x ) )
					cola = ImGui::GetColorU32( cola_v );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit4( "ColB##DrawShape", &colb_v.x ) )
					colb = ImGui::GetColorU32( colb_v );
				ImGui::PopItemWidth();
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
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Pointers##Draw" ) )
			{
			ApplyOpenAll();
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
			ApplyOpenAll();
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
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Color##Draw" ) )
			{
			ApplyOpenAll();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Color Ring" ) )
			{
				float const width = CanvasSize();

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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "OkLab/OkLch Color Quad" ) )
			{
				static int resX = 16;
				static int resY = 16;
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				ImGui::SliderInt( "resX", &resX, 4, 64 ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderInt( "resY", &resY, 4, 64 ); ImGui::PopItemWidth();
				static float L = 1.0f;
				ImGui::SliderFloat( "L", &L, 0.0f, 1.0f );

				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				ImVec2 curPos = ImGui::GetCursorScreenPos();
				float const size = CanvasSize();
				DrawOkLabQuad( pDrawList, curPos, ImVec2( size, size ), L, resX, resY );
				ImGui::Dummy( ImVec2( size, size ) );
				curPos = ImGui::GetCursorScreenPos();
				DrawOkLchQuad( pDrawList, curPos, ImVec2( size, size ), L, resX, resY );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Color2D" ) )
			{
				float const width = CanvasSize();
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
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Masked Shapes##Draw" ) )
			{
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Image Convex Shape" ) )
			{
				float const size = CanvasSize();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Image Concave Shape" ) )
			{
				float const size = CanvasSize();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Shape with Hole" ) )
			{
				static ImVec4 col = { 1, 0, 0, 1 };
				static int gap = 1;
				static int strokeWidth = 1;
				ImGui::ColorEdit4( "Color##Hole", &col.x );
				ImGui::SliderInt( "Gap##Hole", &gap, 1, 16 );
				ImGui::SliderInt( "Stroke Width##Hole", &strokeWidth, 1, 16 );
				float const size = CanvasSize();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Image Shape With Hole" ) )
			{
				float const size = CanvasSize();
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				static ImVec2 uv_offset( 0.0f, 0.0f );
				static ImVec2 uv_scale( 1.0f, 1.0f );
				static int gap = 3;
				static int strokeWidth = 3;
				ImGui::DragFloat2( "Offset##DrawImageShapeWithHole",      &uv_offset[ 0 ], 0.001f, -3.0f, 3.0f );
				ImGui::DragFloat2( "Scale##DrawImageShapeWithHole",       &uv_scale[ 0 ],  0.001f, -3.0f, 3.0f );
				ImGui::SliderInt( "Gap##DrawImageShapeWithHole",          &gap,         1, 16 );
				ImGui::SliderInt( "Stroke Width##DrawImageShapeWithHole", &strokeWidth, 1, 16 );

				ImVec2 pos = ImGui::GetCursorScreenPos();

				// Outer polygon: CW square in screen space (y-down)
				const int hole_segs = 32;
				ImVector<ImVec2> pts;
				pts.resize( 5 + ( hole_segs + 1 ) );
				pts[ 0 ] = ImVec2( pos.x,        pos.y );
				pts[ 1 ] = ImVec2( pos.x + size,  pos.y );
				pts[ 2 ] = ImVec2( pos.x + size,  pos.y + size );
				pts[ 3 ] = ImVec2( pos.x,         pos.y + size );
				pts[ 4 ] = ImVec2( pos.x,         pos.y );       // close outer
				// Hole: CCW circle (counter-clockwise in screen space)
				float cx = pos.x + size * 0.5f;
				float cy = pos.y + size * 0.5f;
				float r  = size * 0.3f;
				for ( int k = 0; k <= hole_segs; k++ )
				{
					float angle = 2.0f * IM_PI * k / hole_segs;
					pts[ 5 + k ] = ImVec2( cx + r * ImCos( angle ), cy - r * ImSin( angle ) );
				}

				DrawImageShapeWithHole( pDrawList, background, pts.Data, pts.Size, IM_COL32( 255, 255, 255, 255 ), uv_offset, uv_scale, gap, strokeWidth );
				ImGui::Dummy( ImVec2( size, size ) );
			}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Chromaticity##Draw" ) )
			{
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Chromaticity Plot" ) )
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				float const size = CanvasSize();

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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Chromaticity Line/Point" ) )
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				float const size = CanvasSize();

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
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Graduation##Draw" ) )
			{
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Linear Line Graduation" ) )
			{
				float const size = CanvasSize();
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
				ImGui::SliderAngle( "a0", &angles[ 0 ] ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderAngle( "a1", &angles[ 1 ] ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderAngle( "a2", &angles[ 2 ] ); ImGui::PopItemWidth();
				ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit3( "c0", &colors[ 0 ].x ) )
					col0 = ImGui::GetColorU32( colors[ 0 ] );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c1", &colors[ 1 ].x ) )
					col1 = ImGui::GetColorU32( colors[ 1 ] );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c2", &colors[ 2 ].x ) )
					col2 = ImGui::GetColorU32( colors[ 2 ] );
				ImGui::PopItemWidth();

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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Linear Circular Graduation" ) )
			{
				float const size = CanvasSize();
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
				ImGui::SliderAngle( "start angle", &angles_bound[ 0 ], -360.0f, angles_bound[ 1 ] * 180.0f / IM_PI ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderAngle( "end angle", &angles_bound[ 1 ], angles_bound[ 0 ] * 180.0f / IM_PI, 360.0f ); ImGui::PopItemWidth();
				ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
				ImGui::SliderAngle( "a0", &angles[ 0 ] ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderAngle( "a1", &angles[ 1 ] ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderAngle( "a2", &angles[ 2 ] ); ImGui::PopItemWidth();
				ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit3( "c0", &colors[ 0 ].x ) )
					col0 = ImGui::GetColorU32( colors[ 0 ] );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c1", &colors[ 1 ].x ) )
					col1 = ImGui::GetColorU32( colors[ 1 ] );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c2", &colors[ 2 ].x ) )
					col2 = ImGui::GetColorU32( colors[ 2 ] );
				ImGui::PopItemWidth();

				float height = ImMax( heights[ 0 ], ImMax( heights[ 1 ], heights[ 2 ] ) );
				ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
				DrawLinearCircularGraduation( pDrawList, pos + ImVec2( size * 0.5f, size * 0.5f ), radius, angles_bound[ 0 ], angles_bound[ 1 ], num_segments,
											  mainLineThickness, mainCol,
											  divisions[ 0 ], heights[ 0 ], thicknesses[ 0 ], angles[ 0 ], col0,
											  divisions[ 1 ], heights[ 1 ], thicknesses[ 1 ], angles[ 1 ], col1,
											  divisions[ 2 ], heights[ 2 ], thicknesses[ 2 ], angles[ 2 ], col2 );
				ImGui::Dummy( ImVec2( size, size ) );
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Log Line Graduation" ) )
			{
				float const size = CanvasSize();
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
				ImGui::SliderAngle( "a0", &angles[ 0 ] ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderAngle( "a1", &angles[ 1 ] ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderAngle( "a2", &angles[ 2 ] );
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit3( "c0", &colors[ 0 ].x ) )
					col0 = ImGui::GetColorU32( colors[ 0 ] );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c1", &colors[ 1 ].x ) )
					col1 = ImGui::GetColorU32( colors[ 1 ] );
				ImGui::PopItemWidth();

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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Log Circular Graduation" ) )
			{
				float const size = CanvasSize();
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
				ImGui::SliderAngle( "start angle", &angles_bound[ 0 ], -360.0f, angles_bound[ 1 ] * 180.0f / IM_PI ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderAngle( "end angle", &angles_bound[ 1 ], angles_bound[ 0 ] * 180.0f / IM_PI, 360.0f ); ImGui::PopItemWidth();
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				ImGui::SliderAngle( "a0", &angles[ 0 ] ); ImGui::PopItemWidth(); ImGui::SameLine();
				ImGui::SliderAngle( "a1", &angles[ 1 ] ); ImGui::PopItemWidth();
				ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
				if ( ImGui::ColorEdit3( "c0", &colors[ 0 ].x ) )
					col0 = ImGui::GetColorU32( colors[ 0 ] );
				ImGui::PopItemWidth();
				ImGui::SameLine();
				if ( ImGui::ColorEdit3( "c1", &colors[ 1 ].x ) )
					col1 = ImGui::GetColorU32( colors[ 1 ] );
				ImGui::PopItemWidth();

				float height = ImMax( heights[ 0 ], heights[ 1 ] );
				ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
				DrawLogCircularGraduation( pDrawList, pos + ImVec2( size * 0.5f, size * 0.5f ), radius, angles_bound[ 0 ], angles_bound[ 1 ], num_segments,
										   mainLineThickness, mainCol,
										   divisions[ 0 ], heights[ 0 ], thicknesses[ 0 ], angles[ 0 ], col0,
										   divisions[ 1 ], heights[ 1 ], thicknesses[ 1 ], angles[ 1 ], col1 );
				ImGui::Dummy( ImVec2( size, size ) );
			}
				ImGui::TreePop();
			}
		}
		ApplyOpenAll();
		if ( ImGui::CollapsingHeader( "Interactions" ) )
		{
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Polygon Hit Testing##Interactions" ) )
			{
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Poly Convex Hovered" ) )
			{
				float const size = CanvasSize();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Poly Concave Hovered" ) )
			{
				float const size = CanvasSize();
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
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Poly With Hole Hovered" ) )
			{
				float const size = CanvasSize();
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
				ImGui::TreePop();
			}
		}
		ApplyOpenAll();
		if ( ImGui::CollapsingHeader( "Widgets" ) )
		{
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Buttons##Widgets" ) )
			{

			ApplyOpenAll();
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Button Capsule" ) )
			{
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();
				float const size = CanvasSize();
				static int value = 0;
				static float length = size;
				static float thickness = size * 0.25f;
				ImGui::DragFloat( "length", &length, 1.0f, 0.0f, 2.0f * size );
				ImGui::DragFloat( "thickness", &thickness, 1.0f, 0.0f, 2.0f * size );
				ImGui::Text( "Value: %d", value );
				value += ( int )ButtonExCapsuleH( "CapsuleH", length, thickness, 0 );
				value += ( int )ButtonExCapsuleV( "CapsuleV", length, thickness, 0 );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Button Convex" ) )
			{
				float const size = CanvasSize();
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Button Concave" ) )
			{
				float const size = CanvasSize();
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Button With Hole" ) )
			{
				float const size = CanvasSize();
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

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Sliders & Inputs##Widgets" ) )
			{

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "DragFloatPrecise" ) )
			{
				static float value1 = 1.0f;
				static float value2 = 100.0f;
				static float value3 = 0.5f;
				ImWidgets::DragFloatPrecise( "Unbounded", &value1 );
				ImWidgets::DragFloatPrecise( "Clamped [0..1000]", &value2, 0.0f, 1000.0f );
				ImWidgets::DragFloatPrecise( "Fixed format", &value3, 0.0f, 0.0f, "%.6f" );
				ImGui::TextWrapped( "Click and drag left/right to edit. Move up/down to change precision rung." );
			}

			ApplyOpenAll();
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "SliderRing" ) )
			{
				static float fval = 0.5f;
				ImWidgets::SliderRingFloat( "Float##SR", &fval, 0.0f, 1.0f );

				static int ival = 50;
				ImWidgets::SliderRingInt( "Int##SR", &ival, 0, 100 );

				ImGui::Separator();
				ImGui::Text( "Custom angles & thickness:" );
				static float fval2 = 0.25f;
				ImWidgets::SliderRingFloat( "Half##SR2", &fval2, 0.0f, 1.0f, -IM_PI, 0.0f, 12.0f );

				static float fval3 = 0.75f;
				ImWidgets::SliderRingFloat( "Full##SR3", &fval3, 0.0f, 1.0f, -IM_PI, IM_PI, 6.0f );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "SliderSpline" ) )
			{
				static float fval = 0.5f;
				ImWidgets::SliderSplineFloat( "S-Curve##SS1", &fval, 0.0f, 1.0f );

				static int ival = 50;
				ImWidgets::SliderSplineInt( "Int##SS2", &ival, 0, 100 );

				ImGui::Separator();
				ImGui::Text( "Custom curves:" );

				// Arc up
				static const ImVec2 arcUp[ 4 ] = { ImVec2( 0.0f, 0.8f ), ImVec2( 0.25f, 0.0f ), ImVec2( 0.75f, 0.0f ), ImVec2( 1.0f, 0.8f ) };
				static float fval2 = 0.3f;
				ImWidgets::SliderSplineFloat( "Arc Up##SS3", &fval2, 0.0f, 1.0f, arcUp );

				// Arc down
				static const ImVec2 arcDown[ 4 ] = { ImVec2( 0.0f, 0.2f ), ImVec2( 0.25f, 1.0f ), ImVec2( 0.75f, 1.0f ), ImVec2( 1.0f, 0.2f ) };
				static float fval3 = 0.7f;
				ImWidgets::SliderSplineFloat( "Arc Down##SS4", &fval3, 0.0f, 1.0f, arcDown );

				// Straight line
				static const ImVec2 straight[ 4 ] = { ImVec2( 0.0f, 0.5f ), ImVec2( 0.33f, 0.5f ), ImVec2( 0.66f, 0.5f ), ImVec2( 1.0f, 0.5f ) };
				static float fval4 = 0.5f;
				ImWidgets::SliderSplineFloat( "Straight##SS5", &fval4, -10.0f, 10.0f, straight );

				// Wave
				static const ImVec2 wave[ 4 ] = { ImVec2( 0.0f, 0.5f ), ImVec2( 0.15f, 0.0f ), ImVec2( 0.85f, 1.0f ), ImVec2( 1.0f, 0.5f ) };
				static float fval5 = 0.5f;
				ImWidgets::SliderSplineFloat( "Wave##SS6", &fval5, 0.0f, 100.0f, wave, 4, 80.0f, 6.0f );

				ImGui::Separator();
				ImGui::Text( "Loops:" );

				// Closed loop (circle-like, 2 bezier segments = 7 points)
				static const ImVec2 closedLoop[ 7 ] = {
					ImVec2( 0.5f, 0.0f ),   // top center
					ImVec2( 1.1f, 0.0f ),   // cp: pull right
					ImVec2( 1.1f, 1.0f ),   // cp: pull right-bottom
					ImVec2( 0.5f, 1.0f ),   // bottom center
					ImVec2( -0.1f, 1.0f ),  // cp: pull left-bottom
					ImVec2( -0.1f, 0.0f ),  // cp: pull left
					ImVec2( 0.5f, 0.0f ),   // back to top
				};
				static float fval6 = 0.25f;
				ImWidgets::SliderSplineFloat( "Closed Loop##SS7", &fval6, 0.0f, 1.0f, closedLoop, 7, 200.0f );

				// Infinity sign (2 bezier segments = 7 points): right lobe then left lobe
				static const ImVec2 infinity[ 7 ] = {
					ImVec2( 0.5f, 0.5f ),    // center crossing
					ImVec2( 0.85f, -0.15f ), // cp: pull upper-right
					ImVec2( 1.15f, 1.15f ),  // cp: pull lower-right
					ImVec2( 0.5f, 0.5f ),    // back to center
					ImVec2( -0.15f, -0.15f ),// cp: pull upper-left
					ImVec2( 0.15f, 1.15f ),  // cp: pull lower-left
					ImVec2( 0.5f, 0.5f ),    // back to center
				};
				static float fval7 = 0.5f;
				ImWidgets::SliderSplineFloat( "Infinity##SS8", &fval7, 0.0f, 1.0f, infinity, 7, 200.0f );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Slider2D Float" ) )
			{
				static ImVec2 slider2D;
				ImVec2 boundMin( -1.0f, -1.0f );
				ImVec2 boundMax( 1.0f, 1.0f );
				Slider2DFloat( "Slider 2D Float", &slider2D.x, &slider2D.y, boundMin.x, boundMax.x, boundMin.y, boundMax.y );
				ImGui::InputFloat2( "Value", &slider2D.x );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Slider2D Int" ) )
			{
				static int vv[ 2 ];
				Slider2DInt( "Slider 2D Int", &vv[ 0 ], &vv[ 1 ], -5, 5, -5, 5 );
				ImGui::InputInt2( "Value", &vv[ 0 ] );
			}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Images##Widgets" ) )
			{

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Image Carousel" ) )
			{
				static int carouselIdx = 0;
				ImTextureID carouselImages[] = { astro_img, clock_img, man_img, illlustration_img, bike_img };
				ImVec2 carouselSizes[] = { astro_size, clock_size, man_size, illlustration_size, bike_size };
				ImWidgets::ImageCarousel( "##Carousel", carouselImages, carouselSizes, IM_ARRAYSIZE( carouselImages ), &carouselIdx );
				ImGui::Text( "Selected: %d", carouselIdx );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Image Bento" ) )
			{
				static int bentoIdx = 0;
				static int bentoColumns = 3;
				static float bentoAspect = 1.0f;
				ImTextureID bentoImages[] = { astro_img, clock_img, man_img, illlustration_img, bike_img };
				ImVec2 bentoSizes[] = { astro_size, clock_size, man_size, illlustration_size, bike_size };
				ImGui::SliderInt( "Columns##Bento", &bentoColumns, 1, 6 );
				ImGui::SliderFloat( "Aspect (W/H)##Bento", &bentoAspect, 0.25f, 4.0f, "%.2f" );
				ImWidgets::ImageBento( "##Bento", bentoImages, bentoSizes, IM_ARRAYSIZE( bentoImages ), &bentoIdx, bentoColumns, bentoAspect );
				ImGui::Text( "Selected: %d", bentoIdx );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Image Viewer" ) )
			{
				// Reuse GPU textures already loaded at startup.
				// CPU copies are loaded separately (stbi keeps them alive) so the inspector
				// can read pixel values. GPU memory is not duplicated.
				static const char* viewerFiles[] = {
					"astro.png", "clock.png", "man.png",
					"pexels-robert-bogdan-156165-1152351.jpg", "camera-542784_1280.png"
				};
				static const char* viewerNames[] = { "Astronaut", "Clock", "Man", "Illustration", "Bike" };
				static stbi_uc*    viewerCPU[ 5 ] = {};
				static bool        viewerCPULoaded = false;

				if ( !viewerCPULoaded )
				{
					viewerCPULoaded = true;
					for ( int i = 0; i < 5; i++ )
					{
						int w, h;
						viewerCPU[ i ] = stbi_load( viewerFiles[ i ], &w, &h, NULL, 4 );
					}
				}

				static int viewerIdx = 0;
				static ImImageViewerState viewerState;

				if ( ImGui::Combo( "Image##Viewer", &viewerIdx, viewerNames, 5 ) )
					viewerState = ImImageViewerState{};

				ImTextureID viewerTexes[] = { astro_img, clock_img, man_img, illlustration_img, bike_img };
				ImVec2      viewerSizes[] = { astro_size, clock_size, man_size, illlustration_size, bike_size };

				// Wire CPU buffer for current image (enables pixel value readback in inspector)
				viewerState.Pixels      = viewerCPU[ viewerIdx ];
				viewerState.PixelSize   = viewerSizes[ viewerIdx ];
				viewerState.PixelFormat = ImPlatform_PixelFormat_RGBA8;

				ImGui::Text( "Scroll: zoom  |  Left-drag: pan  |  Dbl-click: reset  |  Right-click: inspect" );
				ImWidgets::ImageViewer( "##Viewer", viewerTexes[ viewerIdx ], viewerSizes[ viewerIdx ], viewerState );

			}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Drawing Tools##Widgets" ) )
			{

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Up Vector" ) )
			{
				static float upDir[ 3 ] = { 0.0f, 1.0f, 0.0f };
				ImWidgets::UpVector( "##UpVec", upDir );
				ImGui::Text( "Direction: %.3f, %.3f, %.3f", upDir[ 0 ], upDir[ 1 ], upDir[ 2 ] );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Dashed Polylines" ) )
			{
				ImDrawList* dl = ImGui::GetWindowDrawList();
				float avail = ImMin( ImGui::GetContentRegionAvail().x, 400.0f );
				float side = ImMin( avail, ImGui::GetContentRegionAvail().y );
				if ( side < 64.0f ) side = avail; // fallback if vertical space is tiny
				ImVec2 origin = ImGui::GetCursorScreenPos();
				ImGui::InvisibleButton("##zone_dashed_poly", ImVec2(side, side));

				static float thickness = 6.0f;
				static float dash_len = 24.0f;
				static float gap_len  = 12.0f;
				static float offset   = 0.0f;
				static bool  animate   = false;
				static bool  closed    = false;
				static int   cap_idx   = (int)ImWidgetsCap_Butt;
				static int   join_idx  = (int)ImWidgetsJoin_Mitter;
				static float miter_limit = 4.0f;
				static int   path_type = 1; // 0=ZigZag, 1=Sine, 2=Spiral, 3=RoundedRect, 4=Circle, 5=Infinity, 6=Rose, 7=Heart, 8=Sawtooth, 9=Star, 10=BezierS
				const char* caps[] = { "None", "Butt", "Square", "Round", "TriangleOut", "TriangleIn" };
				const char* joins[] = { "Round", "Mitter", "Bevel" };
				const char* paths[] = { "ZigZag", "Sine", "Spiral", "RoundedRect", "Circle", "Infinity", "Rose (k=5)", "Heart", "Sawtooth", "Star", "Bezier S" };
				ImGui::SetCursorScreenPos(origin + ImVec2(8, 6));
				dl->AddRect(origin, origin + ImVec2(side, side), IM_COL32(64,64,64,255));

				// Build path
				ImVec2 pts_stack[256];
				ImVec2* pts = pts_stack;
				int pts_count = 0;
				float left = origin.x + 16.0f;
				float right = origin.x + side - 16.0f;
				float top = origin.y + 24.0f;
				float bottom = origin.y + side - 24.0f;
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
				else if (path_type == 10)
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

				static ImVec4 col_v4 = ImVec4(1.0f, 0.784f, 0.157f, 1.0f);
				ImU32 col = ImGui::ColorConvertFloat4ToU32(col_v4);
				if (animate)
					offset += ImGui::GetIO().DeltaTime * 50.0f;
				ImWidgets::DrawDashedPolylineAA(dl, pts, pts_count, col, thickness, dash_len, gap_len, offset, closed, (ImWidgetsCap_)cap_idx, (ImWidgetsJoin)join_idx, miter_limit);

				ImGui::SetCursorScreenPos(origin + ImVec2(0, side + 6));
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
				ImGui::ColorEdit4("Color##dashed", &col_v4.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
				bool use_gpu = ImWidgets::GetDashedLinesUseGPU();
				if (ImGui::Checkbox("GPU Path##dashed", &use_gpu))
					ImWidgets::SetDashedLinesUseGPU(use_gpu);
				bool debug_joins = ImWidgets::GetDashedLinesDebugJoins();
				if (ImGui::Checkbox("Debug Joins (CPU)##dashed", &debug_joins))
					ImWidgets::SetDashedLinesDebugJoins(debug_joins);
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Paint Canvas" ) )
			{
				// User-owned pixel buffers (different aspect ratios)
				static unsigned char maskPixels[ 64 * 64 ];            // 1:1
				static unsigned char grayPixels[ 60 * 70 ];            // 6:7
				static ImU32         colorPixels[ 160 * 90 ];          // 16:9
				static float         floatPixels[ 128 * 64 * 4 ];     // 2:1

				static ImPaintCanvasData canvases[ 4 ];
				static bool paintInit = false;
				if ( !paintInit )
				{
					memset( maskPixels, 0, sizeof( maskPixels ) );
					memset( grayPixels, 0, sizeof( grayPixels ) );
					memset( colorPixels, 0, sizeof( colorPixels ) );
					memset( floatPixels, 0, sizeof( floatPixels ) );

					canvases[ 0 ].Pixels = maskPixels;   canvases[ 0 ].Width = 64;  canvases[ 0 ].Height = 64; canvases[ 0 ].Format = ImPlatform_PixelFormat_R8;      canvases[ 0 ].Mode = ImPaintMode_BinaryMask;
					canvases[ 1 ].Pixels = grayPixels;   canvases[ 1 ].Width = 60;  canvases[ 1 ].Height = 70; canvases[ 1 ].Format = ImPlatform_PixelFormat_R8;      canvases[ 1 ].Mode = ImPaintMode_Grayscale;
					canvases[ 2 ].Pixels = colorPixels;  canvases[ 2 ].Width = 160; canvases[ 2 ].Height = 90; canvases[ 2 ].Format = ImPlatform_PixelFormat_RGBA8;   canvases[ 2 ].Mode = ImPaintMode_Color;
					canvases[ 3 ].Pixels = floatPixels;  canvases[ 3 ].Width = 128; canvases[ 3 ].Height = 64; canvases[ 3 ].Format = ImPlatform_PixelFormat_RGBA32F; canvases[ 3 ].Mode = ImPaintMode_Color;
					paintInit = true;
				}

				static int activePaint = 2;
				ImGui::Combo( "Format##paint", &activePaint, "Binary Mask (R8)\0Grayscale (R8)\0Color (RGBA8)\0Color (RGBA32F)\0" );
				ImPaintCanvasData& pc = canvases[ activePaint ];

				PaintCanvas( "##PaintMain", &pc, ImVec2( 192, 0 ) );

				// Brush controls
				if ( ImGui::RadioButton( "Brush##pc", pc.Tool == ImPaintTool_Brush ) ) pc.Tool = ImPaintTool_Brush;
				ImGui::SameLine();
				if ( ImGui::RadioButton( "Eraser##pc", pc.Tool == ImPaintTool_Eraser ) ) pc.Tool = ImPaintTool_Eraser;
				if ( pc.Mode != ImPaintMode_BinaryMask )
				{
					ImGui::SameLine();
					if ( ImGui::RadioButton( "Hard##pc", pc.Brush == ImPaintBrush_Hard ) ) pc.Brush = ImPaintBrush_Hard;
					ImGui::SameLine();
					if ( ImGui::RadioButton( "Soft##pc", pc.Brush == ImPaintBrush_Soft ) ) pc.Brush = ImPaintBrush_Soft;
				}

				ImGui::SliderFloat( "Size##pc", &pc.BrushSize, 1.0f, 64.0f, "%.0f" );
				if ( pc.Mode != ImPaintMode_BinaryMask && pc.Brush == ImPaintBrush_Soft )
					ImGui::SliderFloat( "Hardness##pc", &pc.BrushHardness, 0.0f, 1.0f, "%.2f" );
				ImGui::SliderFloat( "Opacity##pc", &pc.BrushOpacity, 0.0f, 1.0f, "%.2f" );

				if ( pc.Mode == ImPaintMode_Color )
					ImGui::ColorEdit4( "Color##pc", &pc.BrushColor.x );
				else if ( pc.Mode == ImPaintMode_Grayscale )
				{
					ImGui::SliderFloat( "Intensity##pc", &pc.BrushColor.x, 0.0f, 1.0f, "%.2f" );
					pc.BrushColor.y = pc.BrushColor.z = pc.BrushColor.x;
				}
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Transform Gizmo" ) )
			{
				static ImTransformImage gizmoImages[ 3 ];
				static bool gizmoInit = false;
				if ( !gizmoInit )
				{
					gizmoImages[ 0 ] = ImTransformImage( astro_img, astro_size );
					gizmoImages[ 1 ] = ImTransformImage( clock_img, clock_size );
					gizmoImages[ 1 ].Transform.Translation = ImVec2( -120.0f, -60.0f );
					gizmoImages[ 1 ].Transform.Scale = ImVec2( 0.4f, 0.4f );
					gizmoImages[ 2 ] = ImTransformImage( man_img, man_size );
					gizmoImages[ 2 ].Transform.Translation = ImVec2( 100.0f, 50.0f );
					gizmoImages[ 2 ].Transform.Scale = ImVec2( 0.5f, 0.5f );
					gizmoInit = true;
				}

				static int gizmoSel = 0;
				static bool gizmoNonUniform = false;

				ImGui::Checkbox( "Non-Uniform Scale", &gizmoNonUniform );

				ImTransformGizmoFlags gizmoFlags = ImTransformGizmoFlags_None;
				if ( gizmoNonUniform )
					gizmoFlags |= ImTransformGizmoFlags_NonUniformScale;

				ImWidgets::ImageTransformGizmo( "##xform", gizmoImages, 3, &gizmoSel, gizmoFlags );

				if ( gizmoSel >= 0 && gizmoSel < 3 )
				{
					ImTransformData* tr = &gizmoImages[ gizmoSel ].Transform;
					ImGui::Text( "Selected: %d", gizmoSel );
					float halfW = ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.x;
					ImGui::SetNextItemWidth( halfW );
					ImGui::DragFloat2( "Position", &tr->Translation.x, 1.0f );
					ImGui::SameLine();
					float deg = tr->Rotation * ( 180.0f / IM_PI );
					ImGui::SetNextItemWidth( halfW );
					if ( ImGui::DragFloat( "Rotation", &deg, 0.5f ) )
						tr->Rotation = deg * ( IM_PI / 180.0f );
					ImGui::SetNextItemWidth( halfW );
					ImGui::DragFloat2( "Scale", &tr->Scale.x, 0.01f, 0.01f, 10.0f );
					ImGui::SameLine();
					if ( ImGui::Button( "Reset" ) )
						*tr = ImTransformData();
				}
				else
				{
					ImGui::TextDisabled( "Click an image to select it" );
				}
			}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Color Editing##Widgets" ) )
			{

			ApplyOpenAll();
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Gradient Editor" ) )
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

				static bool gradAlpha = true;
				ImGui::Checkbox( "Alpha##GradEditor", &gradAlpha );

				GradientEditor( "##GradientMain", &gradient, gradAlpha, ImVec2( 0, 32 ) );

				static char const* interpNames[] = { "sRGB", "Linear sRGB", "OkLab", "OkLCH", "HSV" };
				ImGui::Combo( "Interpolation##GradEditor", &gradient.Interpolation, interpNames, ImWidgetsGradientInterp_COUNT );

				// Edit selected stop color
				if ( gradient.SelectedIdx >= 0 && gradient.SelectedIdx < gradient.Stops.Size )
				{
					ImGradientStop& stop = gradient.Stops[ gradient.SelectedIdx ];
					ImGui::Text( "Stop %d  Position: %.3f", gradient.SelectedIdx, stop.Position );
					ImGuiColorEditFlags ceFlags = gradAlpha ? ( ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf ) : ImGuiColorEditFlags_NoAlpha;
					ImGui::ColorEdit4( "Stop Color##GradEditor", &stop.Color.x, ceFlags );
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
				GradientEditor( "Black to White (OkLab)##Grad2", &gradient2, false );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Curve Editor" ) )
			{
				static ImCurveEditorData curve;
				static bool curveInitialized = false;
				if ( !curveInitialized )
				{
					curve.Keys.clear();
					curve.AddKey( ImVec2( 0.0f, 0.0f ), ImCurveEditorSeg_CubicBezier );
					curve.AddKey( ImVec2( 0.5f, 1.0f ), ImCurveEditorSeg_CubicBezier );
					curve.AddKey( ImVec2( 1.0f, 0.0f ), ImCurveEditorSeg_CubicBezier );
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Color Wheel" ) )
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Color Picker" ) )
			{
				static ImVec4 pickerColor( 0.4f, 0.7f, 0.3f, 1.0f );
				static int pickerSpace = ImColorPickerSpace_sRGB;
				static int srgbFixedAxis = 2;

				ImGui::Combo( "Space##Picker", &pickerSpace, "sRGB\0HSV\0OkLab\0OkLCH\0CIE Lab\0XYZ\0" );
				if ( pickerSpace == ImColorPickerSpace_sRGB )
					ImGui::Combo( "Fixed Axis##Picker", &srgbFixedAxis, "R (GB plane)\0G (RB plane)\0B (RG plane)\0" );

				ColorPicker( "##PickerMain", &pickerColor, ( ImColorPickerSpace )pickerSpace, srgbFixedAxis );

				ImGui::ColorEdit4( "Color##Picker", &pickerColor.x, ImGuiColorEditFlags_Float );

				ImGui::Separator();
				ImGui::Text( "Side-by-side: OkLab vs CIE Lab" );
				if ( ImGui::BeginTable( "##PickerCompare", 2, ImGuiTableFlags_NoSavedSettings ) )
				{
					float colW = ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.x;
					ImGui::TableSetupColumn( "OkLab", ImGuiTableColumnFlags_WidthFixed, colW );
					ImGui::TableSetupColumn( "CIE Lab", ImGuiTableColumnFlags_WidthFixed, colW );
					ImGui::TableHeadersRow();

					static ImVec4 cmpColor( 0.6f, 0.3f, 0.8f, 1.0f );

					ImGui::TableNextColumn();
					ImGui::SetNextItemWidth( colW );
					ColorPickerOkLab( "##CmpOkLab", &cmpColor );

					ImGui::TableNextColumn();
					ImGui::SetNextItemWidth( colW );
					ColorPickerCIELab( "##CmpCIELab", &cmpColor );

					ImGui::EndTable();
				}
				ImGui::ColorEdit4( "Shared Color##PickerCmp", &pickerColor.x, ImGuiColorEditFlags_Float );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Primaries Wheels (Lift/Gamma/Gain/Offset)" ) )
			{
				static ImVec4 primColors[ 4 ] = {
					ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Lift
					ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Gamma
					ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Gain
					ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Offset
				};
				static float primY[ 4 ] = { 0.0f, 0.0f, 1.0f, 25.0f };
				static int primMode = ImColorWheelMode_HSV;

				const char* primNames[] = { "Lift", "Gamma", "Gain", "Offset" };
				const float yMins[] = { -1.0f, -1.0f, -1.0f, -175.0f };
				const float yMaxs[] = {  1.0f,  1.0f,  1.0f,  225.0f };

				ImGui::Combo( "Mode##Prim", &primMode, "HSV\0OkLCH\0" );

				float ringThick = 12.0f;

				float outerSize = ImGui::GetContentRegionAvail().x / 4.0f - ImGui::GetStyle().ItemSpacing.x;
				if ( outerSize < 100.0f ) outerSize = 100.0f;
				if ( outerSize > 200.0f ) outerSize = 200.0f;

				if ( ImGui::BeginTable( "##PrimWheels", 4, ImGuiTableFlags_NoSavedSettings ) )
				{
					for ( int i = 0; i < 4; ++i )
						ImGui::TableSetupColumn( primNames[ i ], ImGuiTableColumnFlags_WidthFixed, outerSize );

					for ( int i = 0; i < 4; ++i )
					{
						ImGui::TableNextColumn();
						ImGui::TextUnformatted( primNames[ i ] );

						ImGui::PushID( i );

						PrimariesWheel( "##pw", &primColors[ i ], &primY[ i ], yMins[ i ], yMaxs[ i ], ( ImColorWheelMode )primMode, ringThick, ImVec2( outerSize, outerSize ) );

						// YRGB readouts
						float qw = outerSize * 0.25f - 1.0f;
						ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "Y##v", &primY[ i ], 0.01f, yMins[ i ], yMaxs[ i ], "%.2f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "R##v", &primColors[ i ].x, 0.01f, 0.0f, 1.0f, "%.2f" );
						ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "G##v", &primColors[ i ].y, 0.01f, 0.0f, 1.0f, "%.2f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "B##v", &primColors[ i ].z, 0.01f, 0.0f, 1.0f, "%.2f" );

						ImGui::PopID();
					}
					ImGui::EndTable();
				}

				ImGui::Separator();

				// Shared controls
				static float primTemp = 0.0f, primTint = 0.0f;
				static float primContrast = 0.0f, primPivot = 0.5f;
				static float primSaturation = 50.0f, primHue = 0.0f;

				float ctrlW = ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.x;
				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Temp", &primTemp, -100.0f, 100.0f, "%.1f" );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Tint", &primTint, -100.0f, 100.0f, "%.1f" );

				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Contrast", &primContrast, -100.0f, 100.0f, "%.1f" );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Pivot", &primPivot, 0.0f, 1.0f, "%.2f" );

				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Saturation", &primSaturation, 0.0f, 100.0f, "%.1f" );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Hue", &primHue, -180.0f, 180.0f, "%.1f" );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "HDR Wheels (Dark/Shadow/Light/Global)" ) )
			{
				static ImVec4 hdrColors[ 4 ] = {
					ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Dark
					ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Shadow
					ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Light
					ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Global
				};
				static float hdrY[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
				static float hdrExposure[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
				static float hdrSaturation[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };

				const char* hdrNames[] = { "Dark", "Shadow", "Light", "Global" };

				float colW = ImGui::GetContentRegionAvail().x / 4.0f - ImGui::GetStyle().ItemSpacing.x;
				if ( colW < 100.0f ) colW = 100.0f;
				if ( colW > 200.0f ) colW = 200.0f;

				float ringThick = 12.0f;

				if ( ImGui::BeginTable( "##HDRWheels", 4, ImGuiTableFlags_NoSavedSettings ) )
				{
					for ( int i = 0; i < 4; ++i )
						ImGui::TableSetupColumn( hdrNames[ i ], ImGuiTableColumnFlags_WidthFixed, colW );

					for ( int i = 0; i < 4; ++i )
					{
						ImGui::TableNextColumn();
						ImGui::TextUnformatted( hdrNames[ i ] );

						ImGui::PushID( i );

						HDRWheel( "##hw", &hdrColors[ i ], &hdrY[ i ], -1.0f, 1.0f,
							&hdrExposure[ i ], -4.0f, 4.0f,
							&hdrSaturation[ i ], 0.0f, 2.0f,
							ImColorWheelMode_OkLCH, ringThick, ImVec2( colW, colW ) );

						// Readouts
						float qw = colW * 0.5f - 1.0f;
						ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "Exp##v", &hdrExposure[ i ], 0.01f, -4.0f, 4.0f, "%.2f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "Sat##v", &hdrSaturation[ i ], 0.01f, 0.0f, 2.0f, "%.2f" );

						ImGui::PopID();
					}
					ImGui::EndTable();
				}

				ImGui::Separator();

				// Global controls
				static float hdrTemp = 0.0f, hdrTint = 0.0f;
				static float hdrContrast = 0.0f, hdrPivot = 0.5f;
				static float hdrMidDetail = 0.0f;
				static float hdrBlackOffset = 0.0f;

				float ctrlW = ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.x;
				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Temp##HDR", &hdrTemp, -100.0f, 100.0f, "%.1f" );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Tint##HDR", &hdrTint, -100.0f, 100.0f, "%.1f" );

				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Contrast##HDR", &hdrContrast, -100.0f, 100.0f, "%.1f" );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Pivot##HDR", &hdrPivot, 0.0f, 1.0f, "%.2f" );

				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Mid Detail##HDR", &hdrMidDetail, -100.0f, 100.0f, "%.1f" );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ctrlW );
				ImGui::SliderFloat( "Black Offset##HDR", &hdrBlackOffset, -1.0f, 1.0f, "%.3f" );
			}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Color Analysis##Widgets" ) )
			{

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Color Curve" ) )
			{
				static int ccMode = ImColorCurveMode_HueVsHue;
				static bool ccShowHistogram = true;
				static bool ccAdvancedSegments = false;
				static ImHistogramData ccHistData;
				static bool ccHistInit = false;

				ImGui::Combo( "Mode##CC", &ccMode, "Hue vs Hue\0Hue vs Sat\0Hue vs Lum\0Lum vs Sat\0Sat vs Sat\0" );
				ImGui::Checkbox( "Luminance Histogram##CC", &ccShowHistogram );

				if ( !ccHistInit )
				{
					int imgW, imgH, imgCh;
					stbi_uc* imgData = stbi_load( "pexels-fotoaibe-1571453.jpg", &imgW, &imgH, &imgCh, 0 );
					if ( imgData )
					{
						int ch = ( imgCh >= 3 ) ? imgCh : 3;
						ccHistData.Accumulate( imgData, imgW, imgH, ch,
							ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved, ImHistogramMode_Luma,
							256, 1000000 );
						STBI_FREE( imgData );
					}
					ccHistInit = true;
				}

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

				ImGui::Checkbox( "Advanced Segments##CC", &ccAdvancedSegments );
				ImColorCurveData& curData = ccData[ ccMode ];
				ImHistogramData const* histPtr = ( ccShowHistogram && ccHistData.BinCount > 0 ) ? &ccHistData : NULL;
				ColorCurve( "##CCMain", &curData, ( ImColorCurveMode )ccMode, histPtr, ccAdvancedSegments, ImVec2( 0, 150 ) );

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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Parade Scope" ) )
			{
				static ImParadeScopeData paradeData;
				static int paradeMode = ImParadeMode_RGB;
				static int paradeSource = 4;
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Vector Scope" ) )
			{
				static ImVectorScopeData vectorData;
				static int vectorSource = 4;
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Histogram" ) )
			{
				static ImHistogramData histData;
				static int histMode = ImHistogramMode_RGB;
				static int histSource = 4;
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

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "CIE Chromaticity" ) )
			{
				static ImCIEChromaticityData cieData;
				static int cieSource = 4;
				static bool cieNeedsUpdate = true;
				static stbi_uc* cieImgData = NULL;
				static int cieImgW = 0;
				static int cieImgH = 0;
				static int cieImgCh = 0;
				static int cieGamut = ImCIEChromaticityGamut_sRGB_Rec709;
				static bool cieShowBackground = false;
				static int cieSignalColor = ImCIEChromaticitySignalColor_PixelColor;
				static float cieSignalAlpha = 0.6f;
				static float cieSignalRadius = 1.5f;
				static ImTextureID cieThumbnail = ImTextureID_Invalid;
				static ImVec2 cieThumbnailSize( 0, 0 );

				bool sourceChanged = ImGui::Combo( "Source##CIE", &cieSource,
					"Color Bars\0Gradient Ramp\0Random Noise\0"
					"Berries (photo)\0Interior (photo)\0Man (photo)\0Astronaut (photo)\0" );
				ImGui::Combo( "Gamut##CIE", &cieGamut,
					"sRGB / Rec.709\0Rec.2020\0DCI-P3\0ACEScg\0Adobe RGB\0ProPhoto\0" );
				ImGui::Checkbox( "Show Background##CIE", &cieShowBackground );
				ImGui::Combo( "Signal Color##CIE", &cieSignalColor, "Flat\0Pixel Color\0" );
				ImGui::SliderFloat( "Signal Alpha##CIE", &cieSignalAlpha, 0.0f, 1.0f, "%.2f" );
				ImGui::SliderFloat( "Signal Radius##CIE", &cieSignalRadius, 0.5f, 6.0f, "%.1f" );
				if ( sourceChanged )
					cieNeedsUpdate = true;

				if ( cieNeedsUpdate )
				{
					if ( cieImgData )
					{
						STBI_FREE( cieImgData );
						cieImgData = NULL;
					}
					if ( cieThumbnail != ImTextureID_Invalid )
					{
						ImPlatform_DestroyTexture( cieThumbnail );
						cieThumbnail = ImTextureID_Invalid;
					}

					static int const kTestW = 1920;
					static int const kTestH = 1080;
					static ImVector<ImU8> testImage;

					if ( cieSource <= 2 )
					{
						testImage.resize( kTestW * kTestH * 3 );
						ImU32 rng = 0xDEADBEEFu;

						if ( cieSource == 0 )
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
						else if ( cieSource == 1 )
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

						cieData.Accumulate( testImage.Data, kTestW, kTestH, 3,
							ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved,
							50000 );

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
							cieThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
							cieThumbnailSize = ImVec2( ( float )kTestW, ( float )kTestH );
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
						int fileIdx = cieSource - 3;
						cieImgData = stbi_load( filenames[ fileIdx ], &cieImgW, &cieImgH, &cieImgCh, 0 );
						if ( cieImgData )
						{
							int ch = ( cieImgCh >= 3 ) ? cieImgCh : 3;
							cieData.Accumulate( cieImgData, cieImgW, cieImgH, ch,
								ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved,
								50000 );
						}
					}

					cieNeedsUpdate = false;
				}

				// Thumbnail
				{
					ImTextureID thumbTex = ImTextureID_Invalid;
					ImVec2 thumbSize( 0, 0 );
					if ( cieSource <= 2 && cieThumbnail != ImTextureID_Invalid )
					{
						thumbTex = cieThumbnail;
						thumbSize = cieThumbnailSize;
					}
					else if ( cieSource == 3 ) { thumbTex = illlustration_img; thumbSize = illlustration_size; }
					else if ( cieSource == 4 ) { thumbTex = background;        thumbSize = background_size; }
					else if ( cieSource == 5 ) { thumbTex = man_img;           thumbSize = man_size; }
					else if ( cieSource == 6 ) { thumbTex = astro_img;         thumbSize = astro_size; }
					if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
					{
						float thumbH = 120.0f;
						float thumbW = thumbH * thumbSize.x / thumbSize.y;
						ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
					}
				}

				ImWidgets::PushStyleVar( StyleVar_CIEChromaticity_SignalAlpha, cieSignalAlpha );
				ImWidgets::PushStyleVar( StyleVar_CIEChromaticity_SignalRadius, cieSignalRadius );
				ImWidgets::CIEChromaticity( "##CIEMain", cieData, ( ImCIEChromaticityGamut )cieGamut, cieShowBackground, ( ImCIEChromaticitySignalColor )cieSignalColor, ImVec2( 0, 0 ) );
				ImWidgets::PopStyleVar( 2 );
				if ( cieSource <= 2 )
					ImGui::Text( "Source: 1920x1080 (generated)  Samples: %d", cieData.SampleCount );
				else if ( cieImgData )
					ImGui::Text( "Source: %dx%d (%d ch)  Samples: %d", cieImgW, cieImgH, cieImgCh, cieData.SampleCount );
				else
					ImGui::TextDisabled( "Failed to load image" );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Tone Curve" ) )
			{
				static ImToneCurveData tcData;
				static ImHistogramData tcHistData;
				static int tcMode = ImHistogramMode_YRGB;
				static int tcSource = 4;
				static bool tcNeedsUpdate = true;
				static bool tcShowHistogram = true;
				static bool tcAdvancedSegments = false;
				static stbi_uc* tcImgData = NULL;
				static int tcImgW = 0;
				static int tcImgH = 0;
				static int tcImgCh = 0;
				static ImTextureID tcThumbnail = ImTextureID_Invalid;
				static ImVec2 tcThumbnailSize( 0, 0 );

				bool sourceChanged = ImGui::Combo( "Source##TC", &tcSource,
					"Color Bars\0Gradient Ramp\0Random Noise\0"
					"Berries (photo)\0Interior (photo)\0Man (photo)\0Astronaut (photo)\0" );
				bool modeChanged = ImGui::Combo( "Mode##TC", &tcMode, "Luminance\0RGB\0YRGB\0YCbCr\0HSV\0OkLCH\0" );
				ImGui::Checkbox( "Show Histogram##TC", &tcShowHistogram );

				// Channel selector buttons
				{
					int channelCount = ImWidgets::ToneCurveChannelCount( ( ImHistogramMode )tcMode );
					for ( int ch = 0; ch < channelCount; ++ch )
					{
						if ( ch > 0 ) ImGui::SameLine();
						bool isActive = ( tcData.ActiveChannel == ch );
						if ( isActive )
							ImGui::PushStyleColor( ImGuiCol_Button, ImGui::GetStyleColorVec4( ImGuiCol_ButtonActive ) );
						char btnLabel[ 16 ];
						ImFormatString( btnLabel, sizeof( btnLabel ), "%s##TC_ch", ImWidgets::ToneCurveChannelName( ( ImHistogramMode )tcMode, ch ) );
						if ( ImGui::Button( btnLabel ) )
							tcData.ActiveChannel = ch;
						if ( isActive )
							ImGui::PopStyleColor();
					}
				}

				if ( sourceChanged || modeChanged )
					tcNeedsUpdate = true;

				if ( modeChanged )
					tcData.Reset( ImWidgets::ToneCurveChannelCount( ( ImHistogramMode )tcMode ) );

				if ( tcNeedsUpdate )
				{
					if ( tcImgData )
					{
						STBI_FREE( tcImgData );
						tcImgData = NULL;
					}
					if ( tcThumbnail != ImTextureID_Invalid )
					{
						ImPlatform_DestroyTexture( tcThumbnail );
						tcThumbnail = ImTextureID_Invalid;
					}

					static int const kTestW = 1920;
					static int const kTestH = 1080;
					static ImVector<ImU8> testImage;

					if ( tcSource <= 2 )
					{
						testImage.resize( kTestW * kTestH * 3 );
						ImU32 rng = 0xDEADBEEFu;

						if ( tcSource == 0 )
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
						else if ( tcSource == 1 )
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

						tcHistData.Accumulate( testImage.Data, kTestW, kTestH, 3,
							ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved, ( ImHistogramMode )tcMode,
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
							tcThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
							tcThumbnailSize = ImVec2( ( float )kTestW, ( float )kTestH );
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
						int fileIdx = tcSource - 3;
						tcImgData = stbi_load( filenames[ fileIdx ], &tcImgW, &tcImgH, &tcImgCh, 0 );
						if ( tcImgData )
						{
							int ch = ( tcImgCh >= 3 ) ? tcImgCh : 3;
							tcHistData.Accumulate( tcImgData, tcImgW, tcImgH, ch,
								ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved, ( ImHistogramMode )tcMode,
								256, 1000000 );
						}
					}

					tcNeedsUpdate = false;
				}

				// Thumbnail
				{
					ImTextureID thumbTex = ImTextureID_Invalid;
					ImVec2 thumbSize( 0, 0 );
					if ( tcSource <= 2 && tcThumbnail != ImTextureID_Invalid )
					{
						thumbTex = tcThumbnail;
						thumbSize = tcThumbnailSize;
					}
					else if ( tcSource == 3 ) { thumbTex = illlustration_img; thumbSize = illlustration_size; }
					else if ( tcSource == 4 ) { thumbTex = background;        thumbSize = background_size; }
					else if ( tcSource == 5 ) { thumbTex = man_img;           thumbSize = man_size; }
					else if ( tcSource == 6 ) { thumbTex = astro_img;         thumbSize = astro_size; }
					if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
					{
						float thumbH = 120.0f;
						float thumbW = thumbH * thumbSize.x / thumbSize.y;
						ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
					}
				}

				ImGui::Checkbox( "Advanced Segments##TC", &tcAdvancedSegments );
				ImHistogramData const* histPtr = ( tcShowHistogram && tcHistData.BinCount > 0 ) ? &tcHistData : NULL;
				ImWidgets::ToneCurve( "##ToneCurveMain", &tcData, ( ImHistogramMode )tcMode, histPtr, tcAdvancedSegments );
				if ( tcSource <= 2 )
					ImGui::Text( "Source: 1920x1080 (generated)" );
				else if ( tcImgData )
					ImGui::Text( "Source: %dx%d (%d ch)", tcImgW, tcImgH, tcImgCh );
				else
					ImGui::TextDisabled( "Failed to load image" );
				ImGui::TextWrapped( "Click to add key. Drag to move. Drag far outside to delete. Right-click for options." );
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Color Warper" ) )
			{
				static ImColorWarperData warperData;
				static bool warperInited = false;
				if ( !warperInited ) { warperData.Init( 12, 6 ); warperInited = true; }

				static int warperMode = ImColorWarperMode_Circular;
				static int warperSpace = ImColorWarperSpace_HSV;
				static float warperThirdAxis = 1.0f;
				static float warperAxisAngle = 0.0f; // radians
				static int warperGrid = 1; // 0=6, 1=12, 2=24

				// Signal overlay
				static ImColorWarperOverlay warperSignal;
				static int warperSource = 4;
				static bool warperNeedsUpdate = true;
				static stbi_uc* warperImgData = NULL;
				static int warperImgW = 0, warperImgH = 0, warperImgCh = 0;
				static int warperSignalColor = ImColorWarperSignalColor_PixelColor;
				static float warperSignalAlpha = 0.6f;
				static float warperSignalRadius = 1.5f;
				static ImTextureID warperThumbnail = ImTextureID_Invalid;
				static ImVec2 warperThumbnailSize( 0, 0 );

				ImGui::Combo( "Shape##Warper", &warperMode, "Circular\0Square\0Chroma/Luma\0" );
				ImGui::Combo( "Color Space##Warper", &warperSpace, "HSV\0HSL\0HSY\0HSP\0HSP Log\0OkLab\0OkLCH\0" );
				{
					char const* thirdAxisLabels[] = { "Value##Warper", "Lightness##Warper", "Luma##Warper",
						"Brightness##Warper", "Brightness (Log)##Warper", "Lightness##Warper", "Lightness##Warper" };
					ImGui::SliderFloat( thirdAxisLabels[ ImClamp( warperSpace, 0, 6 ) ], &warperThirdAxis, 0.0f, 1.0f );
				}
				if ( warperMode == ImColorWarperMode_ChromaLuma )
				{
					float angleDeg = warperAxisAngle * 180.0f / IM_PI;
					if ( ImGui::SliderFloat( "Axis Angle##Warper", &angleDeg, -180.0f, 180.0f, "%.1f deg" ) )
						warperAxisAngle = angleDeg * IM_PI / 180.0f;
				}

				if ( ImGui::Combo( "Grid##Warper", &warperGrid, "6x6\0" "12x6\0" "24x12\0" ) )
				{
					int hueDivs[] = { 6, 12, 24 };
					int satDivs[] = { 6, 6, 12 };
					warperData.Init( hueDivs[ warperGrid ], satDivs[ warperGrid ] );
				}

				bool sourceChanged = ImGui::Combo( "Source##Warper", &warperSource,
					"Color Bars\0Gradient Ramp\0Random Noise\0"
					"Berries (photo)\0Interior (photo)\0Man (photo)\0Astronaut (photo)\0" );
				ImGui::Combo( "Signal Color##Warper", &warperSignalColor, "Flat\0Pixel Color\0" );
				ImGui::SliderFloat( "Signal Alpha##Warper", &warperSignalAlpha, 0.0f, 1.0f, "%.2f" );
				ImGui::SliderFloat( "Signal Radius##Warper", &warperSignalRadius, 0.5f, 6.0f, "%.1f" );
				if ( sourceChanged )
					warperNeedsUpdate = true;

				if ( warperNeedsUpdate )
				{
					if ( warperImgData ) { STBI_FREE( warperImgData ); warperImgData = NULL; }
					if ( warperThumbnail != ImTextureID_Invalid )
					{
						ImPlatform_DestroyTexture( warperThumbnail );
						warperThumbnail = ImTextureID_Invalid;
					}

					static int const kW = 1920, kH = 1080;
					static ImVector<ImU8> testImg;

					if ( warperSource <= 2 )
					{
						testImg.resize( kW * kH * 3 );
						ImU32 rng = 0xCAFEBABEu;
						if ( warperSource == 0 )
						{
							static ImU8 const bars[ 7 ][ 3 ] = {
								{ 191, 191, 191 }, { 191, 191, 17 }, { 17, 191, 191 }, { 17, 191, 17 },
								{ 191, 17, 191 }, { 191, 17, 17 }, { 17, 17, 191 }
							};
							for ( int y = 0; y < kH; ++y )
								for ( int x = 0; x < kW; ++x )
								{
									int off = ( y * kW + x ) * 3;
									int bar = x * 7 / kW;
									rng = rng * 1664525u + 1013904223u;
									int noise = ( int )( rng >> 28 ) - 8;
									testImg[ off + 0 ] = ( ImU8 )ImClamp( bars[ bar ][ 0 ] + noise, 0, 255 );
									testImg[ off + 1 ] = ( ImU8 )ImClamp( bars[ bar ][ 1 ] + noise, 0, 255 );
									testImg[ off + 2 ] = ( ImU8 )ImClamp( bars[ bar ][ 2 ] + noise, 0, 255 );
								}
						}
						else if ( warperSource == 1 )
						{
							for ( int y = 0; y < kH; ++y )
								for ( int x = 0; x < kW; ++x )
								{
									float t = ( float )x / ( float )( kW - 1 );
									int off = ( y * kW + x ) * 3;
									int third = y * 3 / kH;
									rng = rng * 1664525u + 1013904223u;
									int noise = ( int )( rng >> 29 ) - 4;
									testImg[ off + 0 ] = ( ImU8 )ImClamp( ( third == 0 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
									testImg[ off + 1 ] = ( ImU8 )ImClamp( ( third == 1 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
									testImg[ off + 2 ] = ( ImU8 )ImClamp( ( third == 2 ? ( int )( t * 255.0f ) : 0 ) + noise, 0, 255 );
								}
						}
						else
						{
							for ( int i = 0; i < kW * kH * 3; ++i )
							{
								rng = rng * 1664525u + 1013904223u;
								testImg[ i ] = ( ImU8 )( rng >> 24 );
							}
						}
						warperSignal.Accumulate( testImg.Data, kW, kH, 3,
							ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved );
						// Create thumbnail for synthetic image
						{
							ImVector<ImU8> rgba;
							rgba.resize( kW * kH * 4 );
							for ( int i = 0; i < kW * kH; ++i )
							{
								rgba[ i * 4 + 0 ] = testImg[ i * 3 + 0 ];
								rgba[ i * 4 + 1 ] = testImg[ i * 3 + 1 ];
								rgba[ i * 4 + 2 ] = testImg[ i * 3 + 2 ];
								rgba[ i * 4 + 3 ] = 255;
							}
							ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kW, kH );
							warperThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
							warperThumbnailSize = ImVec2( ( float )kW, ( float )kH );
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
						warperImgData = stbi_load( filenames[ warperSource - 3 ], &warperImgW, &warperImgH, &warperImgCh, 0 );
						if ( warperImgData )
						{
							int ch = ( warperImgCh >= 3 ) ? warperImgCh : 3;
							warperSignal.Accumulate( warperImgData, warperImgW, warperImgH, ch,
								ImParadeBitDepth_UInt8, ImParadeLayout_Interleaved );
						}
					}
					warperNeedsUpdate = false;
				}

				// Thumbnail
				{
					ImTextureID thumbTex = ImTextureID_Invalid;
					ImVec2 thumbSize( 0, 0 );
					if ( warperSource <= 2 && warperThumbnail != ImTextureID_Invalid )
					{
						thumbTex = warperThumbnail;
						thumbSize = warperThumbnailSize;
					}
					else if ( warperSource == 3 ) { thumbTex = illlustration_img; thumbSize = illlustration_size; }
					else if ( warperSource == 4 ) { thumbTex = background;        thumbSize = background_size; }
					else if ( warperSource == 5 ) { thumbTex = man_img;           thumbSize = man_size; }
					else if ( warperSource == 6 ) { thumbTex = astro_img;         thumbSize = astro_size; }
					if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
					{
						float thumbH = 120.0f;
						float thumbW = thumbH * thumbSize.x / thumbSize.y;
						ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
					}
				}

				ImColorWarperOverlay const* sigPtr = ( warperSignal.SampleCount > 0 ) ? &warperSignal : NULL;
				ImWidgets::PushStyleVar( StyleVar_CIEChromaticity_SignalAlpha, warperSignalAlpha );
				ImWidgets::PushStyleVar( StyleVar_CIEChromaticity_SignalRadius, warperSignalRadius );
				ColorWarper( "##WarperMain", &warperData,
					( ImColorWarperMode )warperMode, ( ImColorWarperSpace )warperSpace, warperThirdAxis, sigPtr,
					( ImColorWarperSignalColor )warperSignalColor, warperAxisAngle );
				ImWidgets::PopStyleVar( 2 );

				if ( warperSource <= 2 )
					ImGui::Text( "Source: 1920x1080 (generated)  Samples: %d", warperSignal.SampleCount );
				else if ( warperImgData )
					ImGui::Text( "Source: %dx%d (%d ch)  Samples: %d", warperImgW, warperImgH, warperImgCh, warperSignal.SampleCount );
				else
					ImGui::TextDisabled( "Failed to load image" );

				if ( ImGui::Button( "Reset##Warper" ) )
					warperData.Reset();
				ImGui::SameLine();
				ImGui::Text( "Points: %d (%dx%d)", warperData.PointCount(), warperData.HueDivisions, warperData.SatDivisions );
			}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Misc##Widgets" ) )
			{

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Unit Field" ) )
			{
				// Length example
				static float wallLength = 2.5f; // meters
				static int lengthUnit = 0;
				static ImUnitDef lengthUnits[] = {
					ImUnitDef_Simple( "meter",      "m",  1.0f ),
					ImUnitDef_Simple( "centimeter", "cm", 100.0f ),
					ImUnitDef_Simple( "millimeter", "mm", 1000.0f ),
					ImUnitDef_Simple( "foot",       "ft", 3.28084f ),
					ImUnitDef_Simple( "inch",       "in", 39.3701f ),
					ImUnitDef_Simple( "yard",       "yd", 1.09361f ),
				};
				ImWidgets::UnitField( "Wall Length", &wallLength, lengthUnits, IM_ARRAYSIZE( lengthUnits ), &lengthUnit, 0.01f, 0.0f, 100.0f );

				// Temperature example (mul+add)
				static float temperature = 20.0f; // Celsius
				static int tempUnit = 0;
				static ImUnitDef tempUnits[] = {
					ImUnitDef_Simple( "Celsius",    "C", 1.0f,      0.0f ),
					ImUnitDef_Simple( "Fahrenheit", "F", 9.0f/5.0f, 32.0f ),
					ImUnitDef_Simple( "Kelvin",     "K", 1.0f,      273.15f ),
				};
				ImWidgets::UnitField( "Temperature", &temperature, tempUnits, IM_ARRAYSIZE( tempUnits ), &tempUnit, 0.1f );

				// Weight example
				static float weight = 1.0f; // kg
				static int weightUnit = 0;
				static ImUnitDef weightUnits[] = {
					ImUnitDef_Simple( "kilogram", "kg", 1.0f ),
					ImUnitDef_Simple( "gram",     "g",  1000.0f ),
					ImUnitDef_Simple( "pound",    "lb", 2.20462f ),
					ImUnitDef_Simple( "ounce",    "oz", 35.274f ),
				};
				ImWidgets::UnitField( "Weight", &weight, weightUnits, IM_ARRAYSIZE( weightUnits ), &weightUnit, 0.01f, 0.0f, 1000.0f );
			}
				ImGui::TreePop();
			}
		}

		s_open_all = 0;

		ImGui::End();
		ImGui::PopStyleVar();
	}
}

#include <imgui_demo.cpp>
