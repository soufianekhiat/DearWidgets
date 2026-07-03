#include <demo.h>

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

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
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <IconFontCppHeaders/IconsFontAwesome6.h>
#include <IconFontCppHeaders/IconsFontAwesome6Brands.h>

// Demo-only on-demand font downloader. The repo ships only the LaTeX math
// font (workingdir/latex_fonts/); all other demo fonts are fetched by the
// user via the "Download all fonts" / per-category buttons in the Slug section.
#define FONT_DOWNLOADER_IMPLEMENTATION
#include "font_downloader.h"
#include "font_manifest.inl"

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

// ============================================================
// Screenshot system (Windows only)
// ============================================================
#if defined(IM_CURRENT_PLATFORM) && (IM_CURRENT_PLATFORM == IM_PLATFORM_WIN32)
#define DW_SCREENSHOT_SUPPORT 1
#else
#define DW_SCREENSHOT_SUPPORT 0
#endif

#if DW_SCREENSHOT_SUPPORT

#ifndef PW_RENDERFULLCONTENT
#define PW_RENDERFULLCONTENT 0x00000002
#endif

#ifdef DrawText
#undef DrawText
#endif

struct DW_ScreenshotSpec
{
	const char* imgui_window;  // ImGui window title (nullptr = full client area)
	const char* filename;      // output filename, relative to out_dir
};

static const DW_ScreenshotSpec g_screenshot_specs[] =
{
	{ nullptr,        "full.png"          },
	{ "Dear Widgets", "dear_widgets.png"  },
	{ "Shop 00",      "shop_00.png"       },
};

enum DW_SsPhase
{
	DW_SsPhase_Warmup = 0,  // render a few frames so GPU/layout stabilises
	DW_SsPhase_Overview = 1,  // capture full.png, showcase.png, shop_00.png
	DW_SsPhase_OpenAll = 2,  // force-open every CollapsingHeader / TreeNode
	DW_SsPhase_Stabilize = 3,  // let layout re-measure with everything expanded
	DW_SsPhase_RecordSections = 4,  // one frame pass: record section bounds via DW_SsRecord
	DW_SsPhase_SectionCapture = 5,  // iterate recorded sections: scroll -> resize -> capture
	DW_SsPhase_Showcase = 6,  // last: resize window to fit Showcase, capture showcase.png
	DW_SsPhase_Done = 7,
};

struct DW_ScreenshotState
{
	bool       active = false;
	char       out_dir[512] = {};
	char       section_filter[128] = {}; // if set, capture only sections whose name contains this (and skip overview shots)
	bool       done = false;
	DW_SsPhase phase = DW_SsPhase_Warmup;
	int        phase_frames = 0;    // frames elapsed in current phase
	bool       with_headers = true;   // include CollapsingHeader bar in capture
	float      demo_win_w = 1300.0f; // width of the "Dear Widgets" ImGui window (default 2x650)
	// section-capture state
	int        section_index = 0;    // index into g_ss_sections[]
	int        section_pass = 0;    // 0=scroll, 1=settle, 2=remeasure trigger, 3=capture
	int        base_client_w = 1380; // OS window client dimensions to restore after each section
	int        base_client_h = 960;
	float      correct_scroll = -1.0f; // running correct scroll (accumulated from actual heights)
};
static DW_ScreenshotState g_ss;

// Controls read by the demo rendering code (ApplyOpenAll, ShowDemo) each frame
static int   g_ss_open_all = 0;      // -1=close all, 0=off, 1=open all CollapsingHeaders/TreeNodes
static float g_ss_scroll_y = -1.0f;  // >=0: override scroll position of "Dear Widgets" window
static float g_ss_demo_win_h = -1.0f;  // >=0: override "Dear Widgets" window height
static float g_ss_demo_area_x = 10.0f;   // x position for Demo+Samples windows (push off-screen during Showcase)
static float g_ss_showcase_area_x = 5000.0f; // x position for Showcase window (off-screen except during Showcase phase)
static float g_ss_showcase_h = 1400.0f; // constraint height -- large enough to render all content without scroll
static float g_ss_showcase_capture_h = -1.0f;   // measured content height used for actual crop (set after render)

// Per-section bounds recorded during DW_SsPhase_RecordSections
struct DW_SsSection
{
	char  name[128];
	float start_y;       // GetCursorPos().y before the header (scroll-independent, unchanged)
	float end_y;         // GetCursorPos().y after the section (scroll-independent, unchanged)
	float indent_offset; // GetCursorPos().x - WindowPadding.x (unchanged)
	// Populated during the remeasure pass (section on-screen, avail.y is full):
	float dc_x0;         // GetCursorScreenPos().x before header
	float dc_y0;         // GetCursorScreenPos().y before header
	float dc_y1;         // GetCursorScreenPos().y after section
};
static DW_SsSection g_ss_sections[512];
static int          g_ss_nsections = 0;
static bool         g_ss_record_mode = false;
static int          g_ss_remeasure_idx = -1;  // when >=0, update that section's bounds in-place

// Called from ShowDemo / sub-functions while g_ss_record_mode == true,
// OR while g_ss_remeasure_idx == matching section index.
static void DW_SsRecord( const char* name, float y0, float y1 )
{
	if ( g_ss_remeasure_idx >= 0 )
	{
		// Re-measure mode: section is scrolled to the top so avail.y is full.
		// Record DC (screen-space) cursor positions for use as the exact crop rectangle.
		// start_y/end_y are NOT updated so gap computation in the state machine stays correct.
		DW_SsSection& s = g_ss_sections[g_ss_remeasure_idx];
		if ( strcmp( s.name, name ) == 0 && y1 > y0 + 2.0f )
		{
			ImGuiWindow* dw = ImGui::FindWindowByName( "Dear Widgets" );
			if ( dw )
			{
				// DC pos = GetCursorScreenPos().y = y + Pos.y - Scroll.y
				s.dc_y0 = y0 + dw->Pos.y - dw->Scroll.y;
				s.dc_y1 = y1 + dw->Pos.y - dw->Scroll.y;
				s.dc_x0 = dw->InnerRect.Min.x + (ImGui::GetCursorPos().x - ImGui::GetStyle().WindowPadding.x);
			}
			s.indent_offset = ImGui::GetCursorPos().x - ImGui::GetStyle().WindowPadding.x;
			g_ss_remeasure_idx = -1;  // consumed
		}
		return;
	}
	if ( !g_ss_record_mode || g_ss_nsections >= 512 ) return;
	if ( y1 <= y0 + 2.0f ) return;   // section was closed - skip
	DW_SsSection& s = g_ss_sections[g_ss_nsections++];
	ImStrncpy( s.name, name, sizeof( s.name ) );
	s.start_y = y0;
	s.end_y = y1;
	s.indent_offset = ImGui::GetCursorPos().x - ImGui::GetStyle().WindowPadding.x;
}

// Resize the Win32 OS window so the client area is at least (client_w x client_h).
static void DW_ResizeOsWindow( HWND hwnd, int client_w, int client_h )
{
	int screen_w = GetSystemMetrics( SM_CXSCREEN );
	int screen_h = GetSystemMetrics( SM_CYSCREEN );
	client_w = ImMin( client_w, screen_w - 40 );
	client_h = ImMin( client_h, screen_h - 10 );
	DWORD style = (DWORD)GetWindowLongA( hwnd, GWL_STYLE );
	DWORD exstyle = (DWORD)GetWindowLongA( hwnd, GWL_EXSTYLE );
	RECT  rc = { 0, 0, client_w, client_h };
	AdjustWindowRectEx( &rc, style, FALSE, exstyle );
	SetWindowPos( hwnd, nullptr, 0, 0,
				  rc.right - rc.left, rc.bottom - rc.top,
				  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
}

// Capture a rectangular region of the window's CLIENT AREA and write a PNG.
// clip_x/y/w/h are in client-area coordinates.  Pass clip_w==0/clip_h==0 for full client area.
// Uses PrintWindow with a full-window DC so the title bar is correctly excluded from the crop.
static bool DW_CaptureClientAreaPNG( HWND hwnd, const char* path,
									 int clip_x, int clip_y,
									 int clip_w, int clip_h )
{
	// Full window rect (includes title bar + borders)
	RECT wrc;
	GetWindowRect( hwnd, &wrc );
	int full_W = wrc.right - wrc.left;
	int full_H = wrc.bottom - wrc.top;
	if ( full_W <= 0 || full_H <= 0 ) return false;

	// Client area origin in screen coords -> non-client offsets
	POINT client_origin = { 0, 0 };
	ClientToScreen( hwnd, &client_origin );
	int nc_left = client_origin.x - wrc.left;
	int nc_top = client_origin.y - wrc.top;

	// Client area size for clamping
	RECT crc;
	GetClientRect( hwnd, &crc );
	int W = crc.right, H = crc.bottom;
	if ( W <= 0 || H <= 0 ) return false;

	// Resolve clip region in client-area coords
	if ( clip_w <= 0 ) clip_w = W;
	if ( clip_h <= 0 ) clip_h = H;
	clip_x = ImMax( clip_x, 0 );
	clip_y = ImMax( clip_y, 0 );
	clip_w = ImMin( clip_w, W - clip_x );
	clip_h = ImMin( clip_h, H - clip_y );
	if ( clip_w <= 0 || clip_h <= 0 ) return false;

	// Translate clip to full-window DC coordinates (add non-client offsets)
	int dc_x = nc_left + clip_x;
	int dc_y = nc_top + clip_y;
	// Clamp against full DC bounds
	dc_x = ImMax( dc_x, 0 );
	dc_y = ImMax( dc_y, 0 );
	clip_w = ImMin( clip_w, full_W - dc_x );
	clip_h = ImMin( clip_h, full_H - dc_y );
	if ( clip_w <= 0 || clip_h <= 0 ) return false;

	// Ask DWM to composite GPU content into a DC sized to the FULL window.
	// This ensures the title bar occupies DC rows 0..nc_top-1 and the client
	// area starts at nc_top -- we then crop starting at dc_y to exclude it.
	HDC     hdc = GetDC( hwnd );
	HDC     memdc = CreateCompatibleDC( hdc );
	HBITMAP hbm = CreateCompatibleBitmap( hdc, full_W, full_H );
	HGDIOBJ prev = SelectObject( memdc, hbm );
	BOOL    pwok = PrintWindow( hwnd, memdc, PW_RENDERFULLCONTENT );
	GdiFlush();

	if ( !pwok )
	{
		fprintf( stderr, "[screenshot] WARNING: PrintWindow returned FALSE (HWND=%p)\n", (void*)hwnd );
		fflush( stderr );
	}

	// Download pixels -- BGRX (32-bit, alpha byte is 0)
	ImVector<unsigned char> buf;
	buf.resize( full_W * full_H * 4 );
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth = full_W;
	bmi.bmiHeader.biHeight = -full_H;  // top-down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	int got = GetDIBits( memdc, hbm, 0, full_H, buf.Data, &bmi, DIB_RGB_COLORS );

	SelectObject( memdc, prev );
	DeleteObject( hbm );
	DeleteDC( memdc );
	ReleaseDC( hwnd, hdc );

	if ( !got )
	{
		fprintf( stderr, "[screenshot] ERROR: GetDIBits returned 0\n" );
		fflush( stderr );
		return false;
	}

	// Crop and convert BGRX -> RGBA
	ImVector<unsigned char> crop;
	crop.resize( clip_w * clip_h * 4 );
	for ( int row = 0; row < clip_h; ++row )
	{
		const unsigned char* src = &buf[((dc_y + row) * full_W + dc_x) * 4];
		unsigned char* dst = &crop[row * clip_w * 4];
		for ( int col = 0; col < clip_w; ++col, src += 4, dst += 4 )
		{
			dst[0] = src[2];  // R <- B
			dst[1] = src[1];  // G
			dst[2] = src[0];  // B <- R
			dst[3] = 255;
		}
	}

	return stbi_write_png( path, clip_w, clip_h, 4, crop.Data, clip_w * 4 ) != 0;
}

static void DW_RunScreenshotCapture()
{
	// Use window title -- more reliable than ImPlatform_App_GetHWND() in all configurations
	HWND hwnd = FindWindowA( NULL, "Dear Widgets Demo" );
	if ( !hwnd )
	{
		fprintf( stderr, "[screenshot] ERROR: window 'Dear Widgets Demo' not found\n" );
		fflush( stderr );
		return;
	}

	fprintf( stderr, "[screenshot] Capturing to: %s\n", g_ss.out_dir );
	fflush( stderr );

	// Ensure output directory exists
	CreateDirectoryA( g_ss.out_dir, nullptr );

	char path[1024];
	for ( int i = 0; i < (int)(sizeof( g_screenshot_specs ) / sizeof( g_screenshot_specs[0] )); ++i )
	{
		const DW_ScreenshotSpec& spec = g_screenshot_specs[i];
		int x = 0, y = 0, w = 0, h = 0;

		if ( spec.imgui_window )
		{
			ImGuiWindow* win = ImGui::FindWindowByName( spec.imgui_window );
			if ( !win )
			{
				fprintf( stderr, "[screenshot] SKIP  %s (window not found)\n", spec.imgui_window );
				fflush( stderr );
				continue;
			}
			if ( win->Hidden || win->Collapsed )
			{
				fprintf( stderr, "[screenshot] SKIP  %s (hidden/collapsed)\n", spec.imgui_window );
				fflush( stderr );
				continue;
			}
			x = (int)win->Pos.x;
			y = (int)win->Pos.y;
			w = (int)win->Size.x;
			h = (int)win->Size.y;
			fprintf( stderr, "[screenshot] Win '%s' pos=(%d,%d) size=(%dx%d)\n",
					 spec.imgui_window, x, y, w, h );
			fflush( stderr );
		}

		snprintf( path, sizeof( path ), "%s\\%s", g_ss.out_dir, spec.filename );
		bool ok = DW_CaptureClientAreaPNG( hwnd, path, x, y, w, h );
		fprintf( stderr, "[screenshot] %s  %s\n", ok ? "OK  " : "FAIL", path );
		fflush( stderr );
	}
}


#else  // !DW_SCREENSHOT_SUPPORT
// Stub so DW_SsRecord calls in ShowDemo compile in non-screenshot builds
static inline void DW_SsRecord( const char*, float, float )
{}
#endif // DW_SCREENSHOT_SUPPORT

// Read-only, full-width InputText showing a reference URL -- click in and
// Ctrl+A/Ctrl+C to copy, without needing a real hyperlink/browser-launch widget.
static void DW_ReferenceLink( char const* url )
{
	ImGui::TextDisabled( "Reference:" );
	ImGui::SameLine();
	ImGui::SetNextItemWidth( -FLT_MIN );
	ImGui::PushID( url );
	char buf[ 256 ];
	ImFormatString( buf, sizeof( buf ), "%s", url );
	ImGui::InputText( "##ref_link", buf, sizeof( buf ), ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll );
	ImGui::PopID();
}

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

	img_size->x = (float)width;
	img_size->y = (float)height;
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
	float const _10_9 = 1e9f / (T * T * T);
	float const _10_6 = 1e6f / (T * T);
	float const _10_3 = 1e3f / (T);
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
	p = ImVec2( (p.y > 0.0f) ? p.x : l * ImSign( -c.x ), (p.x > 0.0f) ? p.y : l );
	p = ImVec2( p.x, ImAbs( p.y - r ) ) - w;
	return ImWidgets::ImLength( ImMax( p, ImVec2( 0.0f, 0.0f ) ) ) + ImMin( 0.0f, ImMax( p.x, p.y ) );
}

ImU32 sdHorseshoeColor( ImVec2 p, float fTime )
{
	float t = IM_PI * (0.3f + 0.3f * ImCos( fTime * 0.5f ));
	ImVec2 tmp = ImVec2( 0.7f, 1.1f ) * fTime + ImVec2( 0.0f, 2.0f );
	ImVec2 w = ImVec2( 0.750f, 0.25f ) * (ImVec2( 0.5f, 0.5f ) + ImVec2( ImCos( tmp.x ), ImCos( tmp.y ) ) * 0.5f);

	// distance
	float d = sdHorseshoe( p - ImVec2( 0.0f, -0.1f ), ImVec2( ImCos( t ), ImSin( t ) ), 0.5f, w );

	// coloring
	ImVec4 col = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) - ImVec4( 0.1f, 0.4f, 0.7f, 1.0f ) * ImSign( d );
	col = col * (1.0f - exp( -2.0f * ImAbs( d ) ));
	col = col * (0.8f + 0.2f * ImCos( 120.0f * ImAbs( d ) ));
	col = ImLerp( col, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ), 1.0f - ImWidgets::ImSmoothStep( 0.0f, 0.02f, ImAbs( d ) ) );

	return IM_COL32( 255 * col.x, 255 * col.y, 255 * col.z, 255 );
}
#pragma endregion ShaderToyHelper

//////////////////////////////////////////////////////////////////////////
// Demo Helper Structures
//////////////////////////////////////////////////////////////////////////
namespace LayoutConstants{
	constexpr float HALF = 0.5f;
	constexpr float LEFT_PANEL_RATIO = 2.0f / 6.0f;
	constexpr float RIGHT_PANEL_RATIO = 4.0f / 6.0f;
	constexpr float THREE_QUARTERS = 3.0f / 4.0f;
}

struct DemoColor
{
	ImVec4 v;
	ImU32 u;

	DemoColor( float r, float g, float b, float a = 1.0f )
		: v( r, g, b, a ), u( ImGui::GetColorU32( v ) )
	{}

	bool Edit( const char* label )
	{
		if ( ImGui::ColorEdit4( label, &v.x ) )
		{
			u = ImGui::GetColorU32( v );
			return true;
		}
		return false;
	}
};

struct GradientParams
{
	ImVec2 uv_start;
	ImVec2 uv_end;
	DemoColor cola;
	DemoColor colb;

	GradientParams()
		: uv_start( 0.0f, 0.5f ), uv_end( 1.0f, 0.5f ),
		cola( 1.0f, 0.0f, 0.0f ), colb( 0.0f, 1.0f, 0.0f )
	{}

	void RenderControls( const char* suffix = "" )
	{
		char label_a[64], label_b[64], label_uv0[64], label_uv1[64];
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

struct ShapeDebugState
{
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
		vertex_col( 1.0f, 0.5f, 0.0f ), tri_idx( -1 ), side_count( default_sides )
	{}

	void RenderControls( const char* suffix = "", int min_sides = 3, int max_sides = 64 )
	{
		char label_sides[64], label_thick[64], label_radius[64];
		char label_edge[64], label_tri[64], label_vtx[64];
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
ImFont* g_firaCodeFont = nullptr;
ImFont* g_cinzelFont = nullptr;
ImFont* g_alfaSlabFont = nullptr;
ImFont* g_dottedFont = nullptr;
ImFont* g_franticallyFont = nullptr;
ImFont* g_loveLightFont = nullptr;
ImFont* g_magnoliaFont = nullptr;
ImFont* g_squareLilyFont = nullptr;
// Ligature showcase
ImFont* g_bollgoFont = nullptr;
ImFont* g_brightMarchFont = nullptr;
ImFont* g_camoodFont = nullptr;
ImFont* g_cheronaFont = nullptr;
ImFont* g_classicalFont = nullptr;
ImFont* g_endlessFont = nullptr;
ImFont* g_foglihtenFont = nullptr;
ImFont* g_gimboFont = nullptr;
ImFont* g_gingaFont = nullptr;
ImFont* g_metaforaSsFont = nullptr;
ImFont* g_molgethFont = nullptr;
ImFont* g_monblockFont = nullptr;
ImFont* g_reginaFont = nullptr;
ImFont* g_steelworksFont = nullptr;
// Additional Serif -- Google Fonts
ImFont* g_alegreyaFont = nullptr;
ImFont* g_cormorantFont = nullptr;
ImFont* g_cormorantUnicaseFont = nullptr;
ImFont* g_frauncesFont = nullptr;
ImFont* g_italianaFont = nullptr;
ImFont* g_yesevaOneFont = nullptr;
// Additional Script -- Google Fonts
ImFont* g_hurricaneFont = nullptr;
ImFont* g_imperialScriptFont = nullptr;
ImFont* g_ephesisFont = nullptr;
// Additional Display -- Google Fonts
ImFont* g_monotonFont = nullptr;
// Color fonts
ImFont* g_twemojiFont = nullptr;
ImFont* g_aquaphonicDownpourFont = nullptr;
ImFont* g_aquaphonicDrizzleFont = nullptr;
ImFont* g_bungeeSpiceFont = nullptr;
ImFont* g_cimeroProFont = nullptr;
ImFont* g_colorTubeFont = nullptr;
ImFont* g_fatternFont = nullptr;
ImFont* g_gilbertColorFont = nullptr;
ImFont* g_manbowClearFont = nullptr;
ImFont* g_manbowLinesFont = nullptr;
ImFont* g_manbowSpotsFont = nullptr;
ImFont* g_manbowToneFont = nullptr;
ImFont* g_multicoloreFont = nullptr;
ImFont* g_nablaFont = nullptr;
ImFont* g_primecolorCV1Font = nullptr;
ImFont* g_primecolorGFont = nullptr;
ImFont* g_primecolorMFont = nullptr;
ImFont* g_honkFont = nullptr;
ImFont* g_coralPixelsFont = nullptr;
ImFont* g_notoZnamennyFont = nullptr;
// Arabic
ImFont* g_amiriFont = nullptr;
ImFont* g_arefRuqaaBoldFont = nullptr;
ImFont* g_blakaInkFont = nullptr;
ImFont* g_reemKufiFunFont = nullptr;
ImFont* g_cairoPlayBoldFont = nullptr;
ImFont* g_cairoPlayXLightFont = nullptr;
ImFont* g_reemKufiInkFont = nullptr;

// Extra Velvetyne display fonts (overflow from the curated set).
ImFont* g_ouvrieresFont = nullptr;
ImFont* g_picnicFont = nullptr;

// Fontshare display batch (sharpie lives in script; trenchSlab lives in serif).
ImFont* g_comicoFont = nullptr;
ImFont* g_sharpieFont = nullptr;
ImFont* g_bespokeStencilFont = nullptr;
ImFont* g_akturaFont = nullptr;
ImFont* g_britneyFont = nullptr;
ImFont* g_styroFont = nullptr;
ImFont* g_trenchSlabFont = nullptr;
ImFont* g_boxingFont = nullptr;
ImFont* g_kolaFont = nullptr;
ImFont* g_zinaFont = nullptr;
ImFont* g_kihimFont = nullptr;
ImFont* g_striperFont = nullptr;
ImFont* g_kohinoorZeroneFont = nullptr;

// Non-Google color fonts (SVG / COLRv0 / COLRv1).
ImFont* g_notoColorEmojiSvgFont = nullptr;
ImFont* g_openMojiColr0Font = nullptr;
ImFont* g_openMojiColr1Font = nullptr;
ImFont* g_fluentEmojiFont = nullptr;
ImFont* g_amiriQuranColoredFont = nullptr;

// Non-Google Arabic fonts (aliftype / rastikerdar upstream).
ImFont* g_vazirmatnFont = nullptr;
ImFont* g_amiriQuranFont = nullptr;

// Ligature Showcase fonts -- rich ligature sets across programming, classical
// text, decorative script, and historical styles.
ImFont* g_jetbrainsMonoFont = nullptr;
ImFont* g_victorMonoFont = nullptr;
ImFont* g_monaspaceNeonFont = nullptr;
ImFont* g_ebGaramondFont = nullptr;
ImFont* g_lobsterFont = nullptr;
ImFont* g_abrilFatfaceFont = nullptr;
ImFont* g_meaCulpaFont = nullptr;
ImFont* g_junicodeFont = nullptr;
ImFont* g_tapestryFont = nullptr;
// Extra-ornate Ligature Showcase additions.
ImFont* g_birthstoneBounceFont = nullptr;
ImFont* g_monteCarloFont = nullptr;
ImFont* g_mrsSaintDelafieldFont = nullptr;
ImFont* g_sansitaSwashedFont = nullptr;
ImFont* g_bodoniModaFont = nullptr;
ImFont* g_unifrakturMaguntiaFont = nullptr;
ImFont* g_alluraFont = nullptr;
ImFont* g_grenzeGotischFont = nullptr;
ImFont* g_pirataOneFont = nullptr;
ImFont* g_ruthieFont = nullptr;
ImFont* g_leMurmureFont = nullptr;
ImFont* g_caudexFont = nullptr;
ImFont* g_unifrakturCookFont = nullptr;
ImFont* g_chomskyFont = nullptr;
ImFont* g_majorMonoFont = nullptr;

// Load a font into `slot` if slot is currently null AND the file exists on
// disk. Idempotent: calling each frame is a no-op once loaded. Used both at
// startup and when a new font arrives via the downloader UI (see
// LoadOrRefreshDemoFonts).
static inline void LoadFontIfMissing( ImFontAtlas* atlas, ImFont** slot,
									  const char* path, float size,
									  const ImFontConfig* cfg = nullptr,
									  const ImWchar* ranges = nullptr )
{
	if ( *slot != nullptr ) return;
	if ( !ImDwDownload::FileExists( path ) ) return;
	*slot = atlas->AddFontFromFileTTF( path, size, cfg, ranges );
}

// Curated (display-name, font-slot) pair used by the demo's font pickers.
// Only fonts in this list AND present in the live ImGui atlas surface in the
// pickers -- guarantees no dead entries pointing to unloaded files.
struct DemoFontChoice
{
	const char* name; ImFont** ptr;
};

// Test if `f` is currently registered in the active atlas. After dynamic atlas
// rebuilds, an old global slot may still hold a stale pointer; checking the
// atlas vector is the source of truth.
static inline bool DemoFontIsLive( ImFont* f )
{
	if ( !f ) return false;
	const ImVector<ImFont*>& fonts = ImGui::GetIO().Fonts->Fonts;
	for ( int i = 0; i < fonts.Size; ++i )
		if ( fonts[i] == f ) return true;
	return false;
}

// Render a font-picker ComboBox over a curated list, filtered to live atlas
// entries. Updates *io_idx; returns the resolved ImFont* (null if none live).
static inline ImFont* DemoFontPicker( const char* combo_label,
									  const DemoFontChoice* choices, int count,
									  int* io_idx )
{
	if ( !io_idx || !choices || count <= 0 ) return nullptr;
	if ( *io_idx < 0 || *io_idx >= count || !DemoFontIsLive( *choices[*io_idx].ptr ) )
	{
		*io_idx = -1;
		for ( int i = 0; i < count; ++i )
			if ( DemoFontIsLive( *choices[i].ptr ) )
			{
				*io_idx = i; break;
			}
	}
	if ( *io_idx < 0 ) return nullptr;
	if ( ImGui::BeginCombo( combo_label, choices[*io_idx].name ) )
	{
		for ( int i = 0; i < count; ++i )
		{
			if ( !DemoFontIsLive( *choices[i].ptr ) ) continue;
			if ( ImGui::Selectable( choices[i].name, i == *io_idx ) ) *io_idx = i;
		}
		ImGui::EndCombo();
	}
	return *choices[*io_idx].ptr;
}

// Walk every demo-font slot; load any whose file is present on disk but not
// yet in the ImGui atlas. ImGui 1.92's dynamic atlas rebuilds on demand, so
// fonts added here become renderable on the very next frame -- no restart.
static void LoadOrRefreshDemoFonts( ImGuiIO& io )
{
	ImFontConfig slugCfg;
	slugCfg.FontLoader = ImWidgets::GetSlugFontLoader();
	static const ImWchar arabicRanges[] = { 0x0020, 0x007E, 0x0600, 0x06FF, 0xFE70, 0xFEFF, 0 };
	const float sz = 24.0f;

	// Keepers / Code
	LoadFontIfMissing( io.Fonts, &g_firaCodeFont, "fonts/FiraCode[wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_monblockFont, "fonts/Sligoil-Micro.otf", sz, &slugCfg );

	// Serif
	LoadFontIfMissing( io.Fonts, &g_cinzelFont, "fonts/Cinzel[wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_alfaSlabFont, "fonts/AlfaSlabOne-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_classicalFont, "fonts/CinzelDecorative-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_foglihtenFont, "fonts/UnifrakturCook-Bold.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_steelworksFont, "fonts/Rye-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_trenchSlabFont, "fonts/TrenchSlab-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_alegreyaFont, "fonts/Alegreya[wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_cormorantFont, "fonts/Cormorant[wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_cormorantUnicaseFont, "fonts/CormorantUnicase-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_frauncesFont, "fonts/Fraunces[SOFT,WONK,opsz,wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_italianaFont, "fonts/Italiana-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_yesevaOneFont, "fonts/YesevaOne-Regular.ttf", sz, &slugCfg );

	// Script / Handwriting -- styles intentionally diverse (pencil, sharpie,
	// brush, retro, felt-tip, copperplate, ...) rather than many variants
	// of English-roundhand.
	LoadFontIfMissing( io.Fonts, &g_brightMarchFont, "fonts/Sacramento-Regular.ttf", sz, &slugCfg );  // monoline upright
	LoadFontIfMissing( io.Fonts, &g_camoodFont, "fonts/KaushanScript-Regular.ttf", sz, &slugCfg );  // bold brush
	LoadFontIfMissing( io.Fonts, &g_cheronaFont, "fonts/GreatVibes-Regular.ttf", sz, &slugCfg );  // classic copperplate
	LoadFontIfMissing( io.Fonts, &g_loveLightFont, "fonts/LoveLight-Regular.ttf", sz, &slugCfg );  // decorative
	LoadFontIfMissing( io.Fonts, &g_metaforaSsFont, "fonts/Sail-Regular.ttf", sz, &slugCfg );  // bold monoline display
	LoadFontIfMissing( io.Fonts, &g_reginaFont, "fonts/HomemadeApple-Regular.ttf", sz, &slugCfg );  // personal cursive
	LoadFontIfMissing( io.Fonts, &g_sharpieFont, "fonts/Sharpie-Regular.otf", sz, &slugCfg );  // marker handwriting
	LoadFontIfMissing( io.Fonts, &g_hurricaneFont, "fonts/Hurricane-Regular.ttf", sz, &slugCfg );  // ultra bold brush
	LoadFontIfMissing( io.Fonts, &g_imperialScriptFont, "fonts/ImperialScript-Regular.ttf", sz, &slugCfg );  // formal italic
	LoadFontIfMissing( io.Fonts, &g_ephesisFont, "fonts/Ephesis-Regular.ttf", sz, &slugCfg );  // casual handwriting

	// Display / Decorative -- curated Velvetyne picks (plus 3 overflow slots).
	LoadFontIfMissing( io.Fonts, &g_bollgoFont, "fonts/FlorDeRuina-Flor.otf", sz, &slugCfg );  // baroque organic
	LoadFontIfMissing( io.Fonts, &g_dottedFont, "fonts/Bianzhidai-NoBG-Base.otf", sz, &slugCfg );  // pixel/weave (concept match for Dotted)
	LoadFontIfMissing( io.Fonts, &g_franticallyFont, "fonts/Mess.otf", sz, &slugCfg );  // chaotic (concept match for Frantically)
	LoadFontIfMissing( io.Fonts, &g_gimboFont, "fonts/Pilowlava-Regular.otf", sz, &slugCfg );  // bulbous molten
	LoadFontIfMissing( io.Fonts, &g_gingaFont, "fonts/Interlope-Regular.otf", sz, &slugCfg );  // interlocking geometric
	LoadFontIfMissing( io.Fonts, &g_magnoliaFont, "fonts/Letters-Torn.otf", sz, &slugCfg );  // abstract letterforms
	LoadFontIfMissing( io.Fonts, &g_molgethFont, "fonts/Fungal-Grow400Thickness500.ttf", sz, &slugCfg );  // organic growing
	LoadFontIfMissing( io.Fonts, &g_squareLilyFont, "fonts/Lithops-Regular.otf", sz, &slugCfg );  // rock-like organic
	LoadFontIfMissing( io.Fonts, &g_ouvrieresFont, "fonts/Ouvrieres-Affamees.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_picnicFont, "fonts/PicNic-Regular.otf", sz, &slugCfg );

	// Display / Decorative -- Fontshare batch
	LoadFontIfMissing( io.Fonts, &g_comicoFont, "fonts/Comico-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_bespokeStencilFont, "fonts/BespokeStencil-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_akturaFont, "fonts/Aktura-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_britneyFont, "fonts/Britney-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_styroFont, "fonts/Styro-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_boxingFont, "fonts/Boxing-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_kolaFont, "fonts/Kola-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_zinaFont, "fonts/Zina-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_kihimFont, "fonts/Kihim-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_striperFont, "fonts/Striper-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_kohinoorZeroneFont, "fonts/KohinoorZerone-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_monotonFont, "fonts/Monoton-Regular.ttf", sz, &slugCfg );  // neon tubing display

	// CFF Monochrome
	LoadFontIfMissing( io.Fonts, &g_manbowClearFont, "fonts/Array-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_manbowLinesFont, "fonts/Tanker-Regular.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_manbowSpotsFont, "fonts/ManBow-Spots.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_manbowToneFont,  "fonts/ManBow-Lines.otf", sz, &slugCfg );

	// Color
	LoadFontIfMissing( io.Fonts, &g_twemojiFont, "fonts/Noto-COLRv1.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_coralPixelsFont, "fonts/CoralPixels-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_nablaFont, "fonts/Nabla[EDPT,EHLT].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_primecolorCV1Font, "fonts/BungeeSpice-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_bungeeSpiceFont, "fonts/BungeeSpice-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_honkFont, "fonts/Honk[MORF,SHLN].ttf", sz, &slugCfg );

	// Color -- non-Google sources (SVG / COLRv0 / COLRv1)
	LoadFontIfMissing( io.Fonts, &g_aquaphonicDownpourFont, "fonts/Aquaphonic-Downpour.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_aquaphonicDrizzleFont,  "fonts/Aquaphonic-Drizzle.otf",  sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_cimeroProFont,          "fonts/CimeroPro.otf",            sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_gilbertColorFont,       "fonts/GilbertColorBold.otf",     sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_multicoloreFont,        "fonts/Multicolore-Pro.otf",      sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_primecolorGFont,        "fonts/Primecolor-G.ttf",         sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_primecolorMFont,        "fonts/Primecolor-M.ttf",         sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_fatternFont,            "fonts/Fattern.otf",              sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_notoColorEmojiSvgFont, "fonts/NotoColorEmoji-SVG.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_openMojiColr0Font, "fonts/OpenMoji-color-glyf_colr_0.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_openMojiColr1Font, "fonts/OpenMoji-color-glyf_colr_1.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_fluentEmojiFont, "fonts/FluentEmojiColor.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_amiriQuranColoredFont, "fonts/AmiriQuranColored.ttf", sz, &slugCfg, arabicRanges );

	// Arabic
	LoadFontIfMissing( io.Fonts, &g_amiriFont, "fonts/Amiri-Regular.ttf", sz, &slugCfg, arabicRanges );
	LoadFontIfMissing( io.Fonts, &g_arefRuqaaBoldFont, "fonts/ArefRuqaaInk-Bold.ttf", sz, &slugCfg, arabicRanges );
	LoadFontIfMissing( io.Fonts, &g_blakaInkFont, "fonts/BlakaInk-Regular.ttf", sz, &slugCfg, arabicRanges );
	LoadFontIfMissing( io.Fonts, &g_reemKufiInkFont, "fonts/ReemKufiInk-Regular.ttf", sz, &slugCfg, arabicRanges );
	LoadFontIfMissing( io.Fonts, &g_reemKufiFunFont, "fonts/ReemKufiFun[wght].ttf", sz, &slugCfg, arabicRanges );
	LoadFontIfMissing( io.Fonts, &g_cairoPlayBoldFont, "fonts/CairoPlay[slnt,wght].ttf", sz, &slugCfg, arabicRanges );
	LoadFontIfMissing( io.Fonts, &g_cairoPlayXLightFont, "fonts/CairoPlay[slnt,wght].ttf", sz, &slugCfg, arabicRanges );
	LoadFontIfMissing( io.Fonts, &g_vazirmatnFont, "fonts/Vazirmatn-Regular.ttf", sz, &slugCfg, arabicRanges );
	LoadFontIfMissing( io.Fonts, &g_amiriQuranFont, "fonts/AmiriQuran.ttf", sz, &slugCfg, arabicRanges );

	// Ligature Showcase -- canonical filenames match what the downloader pulls.
	LoadFontIfMissing( io.Fonts, &g_jetbrainsMonoFont, "fonts/JetBrainsMono[wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_victorMonoFont, "fonts/VictorMono[wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_monaspaceNeonFont, "fonts/MonaspaceNeon-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_ebGaramondFont, "fonts/EBGaramond[wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_lobsterFont, "fonts/Lobster-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_abrilFatfaceFont, "fonts/AbrilFatface-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_meaCulpaFont, "fonts/MeaCulpa-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_junicodeFont, "fonts/Junicode-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_tapestryFont, "fonts/Tapestry-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_birthstoneBounceFont, "fonts/BirthstoneBounce-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_monteCarloFont, "fonts/MonteCarlo-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_mrsSaintDelafieldFont, "fonts/MrsSaintDelafield-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_sansitaSwashedFont, "fonts/SansitaSwashed[wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_bodoniModaFont, "fonts/BodoniModa[opsz,wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_unifrakturMaguntiaFont, "fonts/UnifrakturMaguntia-Book.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_alluraFont, "fonts/Allura-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_grenzeGotischFont, "fonts/GrenzeGotisch[wght].ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_pirataOneFont, "fonts/PirataOne-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_ruthieFont, "fonts/Ruthie-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_leMurmureFont, "fonts/le-murmure.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_caudexFont, "fonts/Caudex-Regular.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_unifrakturCookFont, "fonts/UnifrakturCook-Bold.ttf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_chomskyFont, "fonts/Chomsky.otf", sz, &slugCfg );
	LoadFontIfMissing( io.Fonts, &g_majorMonoFont, "fonts/MajorMonoDisplay-Regular.ttf", sz, &slugCfg );
}

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
}

namespace ImWidgets{
	void ShowShowcase();
}

// ---- Startup timing markers -------------------------------------------------
// Prints elapsed time per startup phase (delta since previous mark + total since
// the first mark) to stderr -- visible when launched from a console or with
// --screenshot -- and to the debugger via OutputDebugString. Useful for finding
// what dominates cold start (font atlas build, image loads, first-frame shader
// compile, ...). Set DW_STARTUP_TIMING to 0 to compile it out.
#ifndef DW_STARTUP_TIMING
#define DW_STARTUP_TIMING 1
#endif
#if DW_STARTUP_TIMING
static std::chrono::high_resolution_clock::time_point g_dwStartT0, g_dwStartPrev;
static bool g_dwStartInit = false;
static void DW_StartupMark( const char* label )
{
	auto now = std::chrono::high_resolution_clock::now();
	if ( !g_dwStartInit ) { g_dwStartT0 = now; g_dwStartPrev = now; g_dwStartInit = true; }
	double dms = std::chrono::duration<double, std::milli>( now - g_dwStartPrev ).count();
	double tot = std::chrono::duration<double, std::milli>( now - g_dwStartT0 ).count();
	g_dwStartPrev = now;
	char buf[ 256 ];
	snprintf( buf, sizeof( buf ), "[startup] +%8.2f ms  (total %9.2f ms)  %s\n", dms, tot, label );
	fputs( buf, stderr ); fflush( stderr );
#ifdef _WIN32
	OutputDebugStringA( buf );
#endif
}
#else
static inline void DW_StartupMark( const char* ) {}
#endif

int main( int argc, char** argv )
{
	DW_StartupMark( "main entry" );

	// Parse command-line arguments
#if DW_SCREENSHOT_SUPPORT
	for ( int i = 1; i < argc; ++i )
	{
		if ( strcmp( argv[i], "--screenshot" ) == 0 && i + 1 < argc )
		{
			g_ss.active = true;
			snprintf( g_ss.out_dir, sizeof( g_ss.out_dir ), "%s", argv[++i] );
		}
		else if ( strcmp( argv[i], "--no-headers" ) == 0 )
		{
			g_ss.with_headers = false;
		}
		else if ( strcmp( argv[i], "--width" ) == 0 && i + 1 < argc )
		{
			g_ss.demo_win_w = (float)atoi( argv[++i] );
		}
		else if ( strcmp( argv[i], "--section" ) == 0 && i + 1 < argc )
		{
			snprintf( g_ss.section_filter, sizeof( g_ss.section_filter ), "%s", argv[++i] );
		}
	}
	if ( g_ss.active )
	{
		// Allocate a console so that stderr diagnostic output is visible
		AllocConsole();
		freopen( "CONOUT$", "w", stderr );
		g_ss.base_client_w = (int)g_ss.demo_win_w + 80;
		fprintf( stderr, "[screenshot] mode active, output dir: %s  headers=%s  width=%.0f\n",
				 g_ss.out_dir, g_ss.with_headers ? "yes" : "no", g_ss.demo_win_w );
		fflush( stderr );
	}
#else
	(void)argc; (void)argv;
#endif

	// Using the new ImPlatform C API - following ImPlatform demo pattern
	bool bGood;

	// Create window -- use a compact fixed size in screenshot mode for consistent output
#if DW_SCREENSHOT_SUPPORT
	int win_w = g_ss.active ? g_ss.base_client_w : 1024;
	int win_h = g_ss.active ? 960 : 764 * 2;
	ImVec2 win_pos = g_ss.active ? ImVec2( 20.0f, 20.0f ) : ImVec2( 100.0f, 100.0f );
#else
	int win_w = 1024, win_h = 764 * 2;
	ImVec2 win_pos = ImVec2( 100.0f, 100.0f );
#endif
	bGood = ImPlatform_CreateWindow( "Dear Widgets Demo", win_pos, win_w, win_h );
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot create window.\n" );
		return 1;
	}
	DW_StartupMark( "window created" );
#if DW_SCREENSHOT_SUPPORT
	if ( g_ss.active )
	{
		// Read actual client height (width already computed from demo_win_w)
		HWND hwnd_init = FindWindowA( NULL, "Dear Widgets Demo" );
		if ( hwnd_init )
		{
			RECT crc; GetClientRect( hwnd_init, &crc );
			g_ss.base_client_h = crc.bottom;
		}
	}
#endif

	// Initialize Graphics API
	bGood = ImPlatform_InitGfxAPI();
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot initialize the Graphics API.\n" );
		return 1;
	}
	DW_StartupMark( "gfx API initialized" );

	// Show window
	bGood = ImPlatform_ShowWindow();
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot show the window.\n" );
		return 1;
	}
	DW_StartupMark( "window shown" );

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	bGood = ImGui::CreateContext() != nullptr;
	if ( !bGood )
	{
		fprintf( stderr, "ImGui: Cannot create context.\n" );
		return 1;
	}

	// Setup Dear ImGui IO
	ImGuiIO& io = ImGui::GetIO(); (void)io;
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
	DW_StartupMark( "imgui context + style" );

	// Setup DPI scaling (cross-platform)
	float dpi_scale = ImPlatform_GetDpiScale();

	// Load fonts (FontScaleDpi handles DPI scaling at render time)
	io.Fonts->AddFontFromFileTTF( "../extern/FiraCode/distr/ttf/FiraCode-Medium.ttf", 16.0f );

	// Demo fonts load from canonical names matching their download URLs
	// (see font_manifest.inl). LoadOrRefreshDemoFonts is idempotent -- any
	// missing files are re-checked each frame from ShowDrawTextDemo, so
	// fonts downloaded via the UI become renderable without restart.
	LoadOrRefreshDemoFonts( io );
	DW_StartupMark( "demo fonts loaded" );

	// (legacy block below is no-op'd -- replaced by LoadOrRefreshDemoFonts).
	ImFontConfig slugCfg;
	slugCfg.FontLoader = ImWidgets::GetSlugFontLoader();
	(void)slugCfg;
#if 0
	g_firaCodeFont = AddFontIfExists( io.Fonts, "fonts/FiraCode[wght].ttf", 24.0f, &slugCfg );

	// Keepers (already on Google Fonts)
	g_cinzelFont = AddFontIfExists( io.Fonts, "fonts/Cinzel[wght].ttf", 24.0f, &slugCfg );
	g_alfaSlabFont = AddFontIfExists( io.Fonts, "fonts/AlfaSlabOne-Regular.ttf", 24.0f, &slugCfg );
	g_loveLightFont = AddFontIfExists( io.Fonts, "fonts/LoveLight-Regular.ttf", 24.0f, &slugCfg );
	g_nablaFont = AddFontIfExists( io.Fonts, "fonts/Nabla[EDPT,EHLT].ttf", 24.0f, &slugCfg );
	g_bungeeSpiceFont = AddFontIfExists( io.Fonts, "fonts/BungeeSpice-Regular.ttf", 24.0f, &slugCfg );
	g_coralPixelsFont = AddFontIfExists( io.Fonts, "fonts/CoralPixels-Regular.ttf", 24.0f, &slugCfg );
	g_honkFont = AddFontIfExists( io.Fonts, "fonts/Honk[MORF,SHLN].ttf", 24.0f, &slugCfg );

	// --- Replacements (original -> replacement) ---
	g_monblockFont = AddFontIfExists( io.Fonts, "fonts/Sligoil-Micro.otf", 24.0f, &slugCfg );  // Monblock -> Sligoil (Velvetyne)
	g_classicalFont = AddFontIfExists( io.Fonts, "fonts/CinzelDecorative-Regular.ttf", 24.0f, &slugCfg );  // Classical Aesthetics
	g_foglihtenFont = AddFontIfExists( io.Fonts, "fonts/UnifrakturCook-Bold.ttf", 24.0f, &slugCfg );  // Foglihten -> UnifrakturCook
	g_prida61Font = AddFontIfExists( io.Fonts, "fonts/PlayfairDisplaySC-Regular.ttf", 24.0f, &slugCfg );  // Prida 61 -> Playfair Display SC
	g_steelworksFont = AddFontIfExists( io.Fonts, "fonts/Rye-Regular.ttf", 24.0f, &slugCfg );  // Steelworks -> Rye
	g_allessaFont = AddFontIfExists( io.Fonts, "fonts/Allura-Regular.ttf", 24.0f, &slugCfg );  // Allessa -> Allura
	g_brightMarchFont = AddFontIfExists( io.Fonts, "fonts/Sacramento-Regular.ttf", 24.0f, &slugCfg );  // Bright Marching -> Sacramento
	g_camoodFont = AddFontIfExists( io.Fonts, "fonts/KaushanScript-Regular.ttf", 24.0f, &slugCfg );  // Camood -> Kaushan Script
	g_cheronaFont = AddFontIfExists( io.Fonts, "fonts/GreatVibes-Regular.ttf", 24.0f, &slugCfg );  // Cherona -> Great Vibes
	g_daelingFont = AddFontIfExists( io.Fonts, "fonts/Parisienne-Regular.ttf", 24.0f, &slugCfg );  // Daeling -> Parisienne
	g_flowmeryFont = AddFontIfExists( io.Fonts, "fonts/PinyonScript-Regular.ttf", 24.0f, &slugCfg );  // Flowmery -> Pinyon Script
	g_galinsFont = AddFontIfExists( io.Fonts, "fonts/AlexBrush-Regular.ttf", 24.0f, &slugCfg );  // Galins -> Alex Brush
	g_gallanteFont = AddFontIfExists( io.Fonts, "fonts/Italianno-Regular.ttf", 24.0f, &slugCfg );  // Gallante -> Italianno
	g_kleymisskyFont = AddFontIfExists( io.Fonts, "fonts/Yellowtail-Regular.ttf", 24.0f, &slugCfg );  // Kleymissky -> Yellowtail
	g_metaforaAltFont = AddFontIfExists( io.Fonts, "fonts/Tangerine-Regular.ttf", 24.0f, &slugCfg );  // Metafora Alt -> Tangerine
	g_metaforaSsFont = AddFontIfExists( io.Fonts, "fonts/Sail-Regular.ttf", 24.0f, &slugCfg );  // Metafora SS -> Sail
	g_migullonFont = AddFontIfExists( io.Fonts, "fonts/Satisfy-Regular.ttf", 24.0f, &slugCfg );  // Migullon -> Satisfy
	g_milsskyFont = AddFontIfExists( io.Fonts, "fonts/DancingScript[wght].ttf", 24.0f, &slugCfg );  // Milssky -> Dancing Script
	g_reginaFont = AddFontIfExists( io.Fonts, "fonts/Niconne-Regular.ttf", 24.0f, &slugCfg );  // Regina -> Niconne
	g_retroHeartFont = AddFontIfExists( io.Fonts, "fonts/PetitFormalScript-Regular.ttf", 24.0f, &slugCfg );  // Retro Heart -> Petit Formal Script
	g_rosehotFont = AddFontIfExists( io.Fonts, "fonts/Melodrama-Regular.otf", 24.0f, &slugCfg );  // Rosehot -> Melodrama (Fontshare)
	g_sophieFont = AddFontIfExists( io.Fonts, "fonts/EagleLake-Regular.ttf", 24.0f, &slugCfg );  // Sophiemelanie -> Eagle Lake
	g_bollgoFont = AddFontIfExists( io.Fonts, "fonts/Basteleur-Moonlight.otf", 24.0f, &slugCfg );  // Bollgo -> Basteleur (Velvetyne)
	g_boucherFont = AddFontIfExists( io.Fonts, "fonts/AlmendraSC-Regular.ttf", 24.0f, &slugCfg );  // Boucher -> Almendra SC
	g_dottedFont = AddFontIfExists( io.Fonts, "fonts/Codystar-Regular.ttf", 24.0f, &slugCfg );  // Dotted -> Codystar
	g_franticallyFont = AddFontIfExists( io.Fonts, "fonts/RubikBeastly-Regular.ttf", 24.0f, &slugCfg );  // Frantically -> Rubik Beastly
	g_gimboFont = AddFontIfExists( io.Fonts, "fonts/Pilowlava-Regular.otf", 24.0f, &slugCfg );  // Gimbo -> Pilowlava (Velvetyne)
	g_gingaFont = AddFontIfExists( io.Fonts, "fonts/Ouroboros-Regular.otf", 24.0f, &slugCfg );  // Ginga -> Ouroboros (codeberg)
	g_magnoliaFont = AddFontIfExists( io.Fonts, "fonts/UncialAntiqua-Regular.ttf", 24.0f, &slugCfg );  // Magnolia -> Uncial Antiqua
	g_molgethFont = AddFontIfExists( io.Fonts, "fonts/Trickster-Reg.otf", 24.0f, &slugCfg );  // Molgeth -> Trickster (Velvetyne)
	g_squareLilyFont = AddFontIfExists( io.Fonts, "fonts/StalinistOne-Regular.ttf", 24.0f, &slugCfg );  // Square Lily -> Stalinist One
	g_manbowClearFont = AddFontIfExists( io.Fonts, "fonts/Array-Regular.otf", 24.0f, &slugCfg );  // Manbow Clear -> Array (Fontshare, CFF)
	g_manbowLinesFont = AddFontIfExists( io.Fonts, "fonts/Tanker-Regular.otf", 24.0f, &slugCfg );  // Manbow Lines -> Tanker (Fontshare, CFF)
	g_twemojiFont = AddFontIfExists( io.Fonts, "fonts/Noto-COLRv1.ttf", 24.0f, &slugCfg );  // Twemoji -> Noto Color Emoji (COLRv1)
	g_primecolorCV1Font = AddFontIfExists( io.Fonts, "fonts/BungeeSpice-Regular.ttf", 24.0f, &slugCfg );  // Primecolor CV1 -> Bungee Spice (same file, 2 slots)
	g_endlessFont = nullptr;  // originally commented-out; kept for source compatibility

	// --- Unmatched (no replacement found on Google Fonts / Fontshare /
	// Velvetyne / Open Foundry). These slots stay null; their UI labels
	// indicate no replacement is available. ---
	g_manbowSpotsFont = AddFontIfExists( io.Fonts, "fonts/ManBow-Spots.otf", 24.0f, &slugCfg );  // Manbow Spots (Typodermic, CC0)
	g_manbowToneFont  = AddFontIfExists( io.Fonts, "fonts/ManBow-Lines.otf", 24.0f, &slugCfg ); // Manbow Lines (Typodermic, CC0)
	g_aquaphonicDownpourFont = AddFontIfExists( io.Fonts, "fonts/Aquaphonic-Downpour.otf", 24.0f, &slugCfg );
	g_aquaphonicDrizzleFont  = AddFontIfExists( io.Fonts, "fonts/Aquaphonic-Drizzle.otf",  24.0f, &slugCfg );
	g_cimeroProFont    = AddFontIfExists( io.Fonts, "fonts/CimeroPro.otf",        24.0f, &slugCfg );
	g_colorTubeFont    = nullptr;  // no known free download URL
	g_gilbertColorFont = AddFontIfExists( io.Fonts, "fonts/GilbertColorBold.otf", 24.0f, &slugCfg );
	g_multicoloreFont  = AddFontIfExists( io.Fonts, "fonts/Multicolore-Pro.otf",  24.0f, &slugCfg );
	g_primecolorGFont  = AddFontIfExists( io.Fonts, "fonts/Primecolor-G.ttf",     24.0f, &slugCfg );
	g_primecolorMFont  = AddFontIfExists( io.Fonts, "fonts/Primecolor-M.ttf",     24.0f, &slugCfg );
	g_fatternFont      = AddFontIfExists( io.Fonts, "fonts/Fattern.otf",          24.0f, &slugCfg );

	// Arabic glyph range for Arabic fonts
	static const ImWchar arabicRanges[] = { 0x0020, 0x007E, 0x0600, 0x06FF, 0xFE70, 0xFEFF, 0 };
	g_amiriFont = AddFontIfExists( io.Fonts, "fonts/Amiri-Regular.ttf", 24.0f, &slugCfg, arabicRanges );
	g_arefRuqaaBoldFont = AddFontIfExists( io.Fonts, "fonts/ArefRuqaaInk-Bold.ttf", 24.0f, &slugCfg, arabicRanges );
	g_arefRuqaaRegFont = AddFontIfExists( io.Fonts, "fonts/ArefRuqaaInk-Regular.ttf", 24.0f, &slugCfg, arabicRanges );
	g_blakaInkFont = AddFontIfExists( io.Fonts, "fonts/BlakaInk-Regular.ttf", 24.0f, &slugCfg, arabicRanges );
	g_reemKufiInkFont = AddFontIfExists( io.Fonts, "fonts/ReemKufiInk-Regular.ttf", 24.0f, &slugCfg, arabicRanges );
	g_reemKufiFunFont = AddFontIfExists( io.Fonts, "fonts/ReemKufiFun[wght].ttf", 24.0f, &slugCfg, arabicRanges );
	// Cairo Play upstream is now a single variable font (slnt,wght axes); both
	// demo slots load the same file -- visually identical until we wire axis
	// instancing into the Slug font loader.
	g_cairoPlayBoldFont = AddFontIfExists( io.Fonts, "fonts/CairoPlay[slnt,wght].ttf", 24.0f, &slugCfg, arabicRanges );
	g_cairoPlayXLightFont = AddFontIfExists( io.Fonts, "fonts/CairoPlay[slnt,wght].ttf", 24.0f, &slugCfg, arabicRanges );
#endif

	// Sync downloader statuses against what's on disk (so "Downloaded" labels
	// render correctly for files that already exist).
	ImDwDownload::RefreshStatuses();
	DW_StartupMark( "download statuses refreshed" );

	// Load LaTeX math font (Latin Modern Math)
	ImWidgets::LoadLaTeXFont();
	DW_StartupMark( "LaTeX font loaded" );

	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes( dpi_scale );
	style.FontScaleDpi = dpi_scale;
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
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
#endif

	// Initialize ImPlatform backends
	bGood = ImPlatform_InitPlatform();
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot initialize platform.\n" );
		return 1;
	}
	DW_StartupMark( "platform backend init" );

	bGood = ImPlatform_InitGfx();
	if ( !bGood )
	{
		fprintf( stderr, "ImPlatform: Cannot initialize graphics.\n" );
		return 1;
	}
	DW_StartupMark( "gfx backend init (font atlas build/upload)" );

	// Create ImWidgets context
	ImWidgets::AddFeatures( ImWidgetsFeatures_Markers | ImWidgetsFeatures_RichFont | ImWidgetsFeatures_LaTeX );
	ImWidgetsContext* ctx = ImWidgets::CreateContext();
	DW_StartupMark( "ImWidgets context created" );

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
	DW_StartupMark( "demo images loaded" );


	ImVec4 clear_color = ImVec4( 0.461f, 0.461f, 0.461f, 1.0f );
	DW_StartupMark( "init complete -- entering main loop" );
	static bool s_firstFrameMarked = false;
	while ( ImPlatform_PlatformContinue() )
	{
		const bool dwFirst = !s_firstFrameMarked;

		ImPlatform_PlatformEvents();

		if ( !ImPlatform_GfxCheck() )
		{
			static int s_gfxCheckFails = 0;
			if ( dwFirst && s_gfxCheckFails++ == 0 )
				DW_StartupMark( "frame1: PlatformEvents done; GfxCheck FALSE -> spinning until renderable" );
			continue;
		}
		if ( dwFirst ) DW_StartupMark( "frame1: GfxCheck ok -> render begins" );

		// Capture previous frame's backbuffer for blur effects (before NewFrame clears state)
		ImWidgets::BlurBackgroundNewFrame();
		if ( dwFirst ) DW_StartupMark( "frame1: BlurBackgroundNewFrame" );

		// New frame
		ImPlatform_GfxAPINewFrame();
		ImPlatform_PlatformNewFrame();
		ImGui::NewFrame();
		if ( dwFirst ) DW_StartupMark( "frame1: NewFrame (gfx+platform+imgui)" );

		// Pre-warm tessellation cache on first frame so CollapsingHeaders open without stall
		static bool s_tessWarmed = false;
		if ( !s_tessWarmed )
		{
			if ( g_dottedFont ) ImWidgets::PrewarmTessellationCache( g_dottedFont );
			s_tessWarmed = true;
		}
		if ( dwFirst ) DW_StartupMark( "frame1: PrewarmTessellationCache" );

		// Render UI
		// In screenshot mode, force each window to a known position and size so captures
		// are deterministic regardless of imgui.ini saved state.
#if DW_SCREENSHOT_SUPPORT
		if ( g_ss.active )
		{
			ImGui::SetNextWindowPos( ImVec2( g_ss_demo_area_x, 10.0f ), ImGuiCond_Always );
			ImGui::SetNextWindowSize( ImVec2( 600.0f, 930.0f ), ImGuiCond_Always );
		}
#endif
		ImWidgets::ShowSamples();
		if ( dwFirst ) DW_StartupMark( "frame1: ShowSamples()" );

#if DW_SCREENSHOT_SUPPORT
		if ( g_ss.active )
		{
			float demo_h = (g_ss_demo_win_h > 0.0f) ? g_ss_demo_win_h : 930.0f;
			ImGui::SetNextWindowPos( ImVec2( g_ss_demo_area_x, 10.0f ), ImGuiCond_Always );
			ImGui::SetNextWindowSize( ImVec2( g_ss.demo_win_w, demo_h ), ImGuiCond_Always );
		}
#endif
		ImWidgets::ShowDemo();
		if ( dwFirst ) DW_StartupMark( "frame1: ShowDemo()" );

#if DW_SCREENSHOT_SUPPORT
		if ( g_ss.active )
		{
			float sh = (g_ss.phase == DW_SsPhase_Showcase) ? g_ss_showcase_h : 700.0f;
			ImGui::SetNextWindowPos( ImVec2( g_ss_showcase_area_x, 10.0f ), ImGuiCond_Always );
			ImGui::SetNextWindowSizeConstraints( ImVec2( 800.0f, sh ), ImVec2( 800.0f, sh ) );
			ImGui::SetNextWindowCollapsed( false, ImGuiCond_Always );
		}
#endif
		ImWidgets::ShowShowcase();
		if ( dwFirst ) DW_StartupMark( "frame1: ShowShowcase()" );

#if DW_SCREENSHOT_SUPPORT
		if ( !g_ss.active )
#endif
		{
			ImWidgets::ShowStyleEditor();
			ImGui::ShowMetricsWindow();
			ImGui::ShowDemoWindow();
		}
		if ( dwFirst ) DW_StartupMark( "frame1: StyleEditor/Metrics/DemoWindow" );

		ShowSampleOffscreen00();
		if ( dwFirst ) DW_StartupMark( "frame1: ShowSampleOffscreen00()" );

		// Background effect demo window
		{
			static int effectIdx = 0;
			static float blur_radius = 4.0f;
			static float glass_bevel = 0.3f;
			static float glass_ior = 1.5f;
			static float frost_radius = 6.0f;
			static float frost_noise = 0.5f;
			static float pixel_size = 8.0f;
			static float chroma_strength = 8.0f;
			static float chroma_samples = 8.0f;
			static float liquid_strength = 0.5f;
			static float liquid_bevel = 0.3f;
			static float haze_amplitude = 4.0f;
			static float haze_frequency = 6.0f;
			static float voronoi_cells = 12.0f;
			static float voronoi_edge = 2.0f;
			static float voronoi_ior = 1.5f;
			static float edge_intensity = 4.0f;
			static float halftone_spacing = 6.0f;
			static float halftone_sharp = 2.0f;
			static float mouse_radius = 150.0f;
			static float mouse_intensity = 3.0f;
			static float crt_scanlines = 0.4f;
			static float crt_barrel = 1.0f;
			static float dot_cellsize = 6.0f;
			static float dot_round = 0.8f;
			static float glitch_intensity = 0.5f;
			static float glitch_blocksize = 8.0f;
			static float stained_cells = 12.0f;
			static float stained_lead = 3.0f;
			static float rain_density = 0.6f;
			static float rain_trails = 1.0f;
			static float rain_speed = 1.0f;
			static float kal_segments = 6.0f;
			static float kal_rotation = 0.0f;
			static ImVec4 tint_color( 1.0f, 1.0f, 1.0f, 1.0f );
			static bool noTitleBar = false;

			ImGuiWindowFlags winFlags = noTitleBar ? ImGuiWindowFlags_NoTitleBar : 0;
			ImGui::SetNextWindowBgAlpha( 0.0f );
			ImGui::SetNextWindowSize( ImVec2( 340, 0 ), ImGuiCond_FirstUseEver );
			ImGui::Begin( "Background Effect", NULL, winFlags );

			ImWidgetsBgEffect eff = (ImWidgetsBgEffect)effectIdx;
			float p0 = 0.0f, p1 = 0.0f, p2 = 0.0f;
			switch ( eff )
			{
			default:
			case ImWidgetsBgEffect_Blur:                p0 = blur_radius; break;
			case ImWidgetsBgEffect_GlassRefraction:     p0 = glass_bevel; p1 = glass_ior; break;
			case ImWidgetsBgEffect_FrostedGlass:        p0 = frost_radius; p1 = frost_noise; break;
			case ImWidgetsBgEffect_Pixelate:            p0 = pixel_size; break;
			case ImWidgetsBgEffect_ChromaticAberration: p0 = chroma_strength; p1 = chroma_samples; break;
			case ImWidgetsBgEffect_LiquidGlass:         p0 = liquid_strength; p1 = liquid_bevel; break;
			case ImWidgetsBgEffect_HeatHaze:            p0 = haze_amplitude; p1 = haze_frequency; break;
			case ImWidgetsBgEffect_Voronoi:             p0 = voronoi_cells; p1 = voronoi_edge; p2 = voronoi_ior; break;
			case ImWidgetsBgEffect_EdgeGlow:            p0 = edge_intensity; break;
			case ImWidgetsBgEffect_Halftone:            p0 = halftone_spacing; p1 = halftone_sharp; break;
			case ImWidgetsBgEffect_MouseEdge:           p0 = mouse_radius; p1 = mouse_intensity; break;
			case ImWidgetsBgEffect_CRT:                 p0 = crt_scanlines; p1 = crt_barrel; break;
			case ImWidgetsBgEffect_DotMatrix:           p0 = dot_cellsize; p1 = dot_round; break;
			case ImWidgetsBgEffect_Glitch:              p0 = glitch_intensity; p1 = glitch_blocksize; break;
			case ImWidgetsBgEffect_StainedGlass:        p0 = stained_cells; p1 = stained_lead; break;
			case ImWidgetsBgEffect_Rain:                p0 = rain_density; p1 = rain_trails; p2 = (float)ImGui::GetTime() * rain_speed; break;
			case ImWidgetsBgEffect_Kaleidoscope:        p0 = kal_segments; p1 = kal_rotation; break;
			}
			ImU32 tint = ImGui::GetColorU32( tint_color );
			ImWidgets::SetCurrentWindowBlurBackground( eff, p0, p1, p2, tint );

			static const char* effectNames[] = {
				"Blur", "Glass Refraction", "Frosted Glass", "Pixelate",
				"Chromatic Aberration", "Liquid Glass", "Heat Haze",
				"Voronoi Shatter", "Edge Glow", "Halftone", "Mouse Edge",
				"CRT Scanlines", "Dot Matrix", "Glitch", "Stained Glass", "Rain", "Kaleidoscope"
			};
			ImGui::Combo( "Effect", &effectIdx, effectNames, ImWidgetsBgEffect_COUNT );
			ImGui::ColorEdit4( "Tint", &tint_color.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf );
			ImGui::Checkbox( "No Title Bar", &noTitleBar );
			ImGui::Separator();

			// Per-effect labels get a distinct ##suffix. Several sliders across
			// branches share the same visible label (Blur Radius, Bevel, IOR,
			// Intensity, Cells) and ImGui keys state by the full label string,
			// so without suffixes rapid effect switching would bleed drag /
			// popup / active-id state between branches.
			switch ( eff )
			{
			default:
			case ImWidgetsBgEffect_Blur:
				ImGui::SliderFloat( "Blur Radius##Blur", &blur_radius, 0.5f, 32.0f );
				break;
			case ImWidgetsBgEffect_GlassRefraction:
				ImGui::SliderFloat( "Bevel##Glass", &glass_bevel, 0.01f, 0.8f );
				ImGui::SliderFloat( "IOR##Glass", &glass_ior, 1.0f, 3.0f );
				break;
			case ImWidgetsBgEffect_FrostedGlass:
				ImGui::SliderFloat( "Blur Radius##Frost", &frost_radius, 1.0f, 20.0f );
				ImGui::SliderFloat( "Noise Scale##Frost", &frost_noise, 0.1f, 2.0f );
				break;
			case ImWidgetsBgEffect_Pixelate:
				ImGui::SliderFloat( "Block Size (px)##Pixelate", &pixel_size, 2.0f, 32.0f );
				break;
			case ImWidgetsBgEffect_ChromaticAberration:
				ImGui::SliderFloat( "Strength##Chroma", &chroma_strength, 1.0f, 30.0f );
				ImGui::SliderFloat( "Samples##Chroma", &chroma_samples, 4.0f, 16.0f );
				break;
			case ImWidgetsBgEffect_LiquidGlass:
				ImGui::SliderFloat( "Refraction##Liquid", &liquid_strength, 0.1f, 2.0f );
				ImGui::SliderFloat( "Bevel##Liquid", &liquid_bevel, 0.01f, 0.8f );
				break;
			case ImWidgetsBgEffect_HeatHaze:
				ImGui::SliderFloat( "Amplitude##Haze", &haze_amplitude, 0.5f, 16.0f );
				ImGui::SliderFloat( "Frequency##Haze", &haze_frequency, 1.0f, 20.0f );
				break;
			case ImWidgetsBgEffect_Voronoi:
				ImGui::SliderFloat( "Cells##Voronoi", &voronoi_cells, 2.0f, 40.0f );
				ImGui::SliderFloat( "Edge Width##Voronoi", &voronoi_edge, 0.5f, 8.0f );
				ImGui::SliderFloat( "IOR##Voronoi", &voronoi_ior, 1.0f, 3.0f );
				break;
			case ImWidgetsBgEffect_EdgeGlow:
				ImGui::SliderFloat( "Intensity##EdgeGlow", &edge_intensity, 0.5f, 10.0f );
				break;
			case ImWidgetsBgEffect_Halftone:
				ImGui::SliderFloat( "Dot Spacing##Halftone", &halftone_spacing, 3.0f, 20.0f );
				ImGui::SliderFloat( "Sharpness##Halftone", &halftone_sharp, 0.5f, 8.0f );
				break;
			case ImWidgetsBgEffect_MouseEdge:
				ImGui::SliderFloat( "Radius (px)##MouseEdge", &mouse_radius, 30.0f, 500.0f );
				ImGui::SliderFloat( "Intensity##MouseEdge", &mouse_intensity, 0.5f, 8.0f );
				break;
			case ImWidgetsBgEffect_CRT:
				ImGui::SliderFloat( "Scanline Darkness##CRT", &crt_scanlines, 0.0f, 1.0f );
				ImGui::SliderFloat( "Barrel Distortion##CRT", &crt_barrel, 0.0f, 5.0f );
				break;
			case ImWidgetsBgEffect_DotMatrix:
				ImGui::SliderFloat( "Cell Size (px)##DotMatrix", &dot_cellsize, 3.0f, 20.0f );
				ImGui::SliderFloat( "Roundness##DotMatrix", &dot_round, 0.0f, 1.0f );
				break;
			case ImWidgetsBgEffect_Glitch:
				ImGui::SliderFloat( "Intensity##Glitch", &glitch_intensity, 0.05f, 2.0f );
				ImGui::SliderFloat( "Block Size##Glitch", &glitch_blocksize, 2.0f, 32.0f );
				break;
			case ImWidgetsBgEffect_StainedGlass:
				ImGui::SliderFloat( "Cells##Stained", &stained_cells, 2.0f, 40.0f );
				ImGui::SliderFloat( "Lead Width##Stained", &stained_lead, 0.5f, 10.0f );
				break;
			case ImWidgetsBgEffect_Rain:
				ImGui::SliderFloat( "Rain Amount##Rain", &rain_density, 0.0f, 1.0f );
				ImGui::SliderFloat( "Fog Blur##Rain", &rain_trails, 0.0f, 5.0f );
				ImGui::SliderFloat( "Speed##Rain", &rain_speed, 0.1f, 4.0f );
				break;
			case ImWidgetsBgEffect_Kaleidoscope:
				ImGui::SliderFloat( "Segments##Kal", &kal_segments, 2.0f, 16.0f );
				ImGui::SliderAngle( "Rotation##Kal", &kal_rotation );
				break;
			}

			ImGui::End();
		}

		if ( dwFirst ) DW_StartupMark( "frame1: rest of UI (Background Effect window, etc.)" );

		// Rendering
		ImGui::Render();
		if ( dwFirst ) DW_StartupMark( "frame1: ImGui::Render()" );
		ImPlatform_GfxAPIClear( clear_color );
		ImPlatform_GfxAPIRender( clear_color );
		if ( dwFirst ) DW_StartupMark( "frame1: GfxAPIRender (GPU submit)" );

#ifdef IMGUI_HAS_VIEWPORT
		// Update and Render additional Platform Windows
		if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			ImPlatform_GfxViewportPre();
			ImPlatform_GfxViewportPost();
		}
#endif

		ImPlatform_GfxAPISwapBuffer();

		if ( !s_firstFrameMarked ) { DW_StartupMark( "frame1: SwapBuffer -> first frame presented" ); s_firstFrameMarked = true; }

		// Screenshot state machine
#if DW_SCREENSHOT_SUPPORT
		if ( g_ss.active && !g_ss.done )
		{
			g_ss.phase_frames++;

			switch ( g_ss.phase )
			{
			case DW_SsPhase_Warmup:
				// Wait for GPU + ImGui layout to fully settle
				if ( g_ss.phase_frames >= 8 )
				{
					g_ss.phase = DW_SsPhase_Overview;
					g_ss.phase_frames = 0;
				}
				break;

			case DW_SsPhase_Overview:
				// Capture the static overview shots (full window, Showcase, Shop 00)
				// Wait one extra frame so positions from imgui.ini are applied
				if ( g_ss.phase_frames >= 2 )
				{
					if ( g_ss.section_filter[0] == '\0' )
						DW_RunScreenshotCapture();  // full.png, showcase.png, shop_00.png (skipped when filtering one section)
					else
						CreateDirectoryA( g_ss.out_dir, nullptr );  // ensure out dir exists (normally created by DW_RunScreenshotCapture)
					g_ss.phase = DW_SsPhase_OpenAll;
					g_ss.phase_frames = 0;
				}
				break;

			case DW_SsPhase_OpenAll:
				// Signal ApplyOpenAll() to expand every CollapsingHeader / TreeNode.
				// This is consumed each frame, so we hold it for 3 frames to reach
				// nested headers that are only visible after their parent opens.
				g_ss_open_all = 1;
				if ( g_ss.phase_frames >= 3 )
				{
					g_ss_open_all = 0;
					g_ss.phase = DW_SsPhase_Stabilize;
					g_ss.phase_frames = 0;
				}
				break;

			case DW_SsPhase_Stabilize:
				// Let layout re-measure, then start section recording
				if ( g_ss.phase_frames >= 5 )
				{
					g_ss_scroll_y = 0.0f;
					g_ss.phase = DW_SsPhase_RecordSections;
					g_ss.phase_frames = 0;
				}
				break;

			case DW_SsPhase_RecordSections:
				// Enable recording; ShowDemo() will populate g_ss_sections[] this frame
				g_ss_record_mode = true;
				if ( g_ss.phase_frames >= 2 )
				{
					g_ss_record_mode = false;
					g_ss.section_index = 0;
					g_ss.section_pass = 0;
					g_ss.correct_scroll = -1.0f;  // bootstrap from first section in pass 0
					g_ss.phase = DW_SsPhase_SectionCapture;
					g_ss.phase_frames = 0;
					// Use a fixed tall window for ALL captures so GetContentRegionAvail().y
					// is consistent between remeasure and capture passes.
					{
						float max_h = (float)GetSystemMetrics( SM_CYSCREEN ) - 100.0f;
						g_ss_demo_win_h = max_h;
						HWND hwnd_ss = FindWindowA( NULL, "Dear Widgets Demo" );
						if ( hwnd_ss )
							DW_ResizeOsWindow( hwnd_ss, g_ss.base_client_w, (int)max_h + 40 );
					}
					fprintf( stderr, "[screenshot] Recorded %d sections, capture height=%.0f\n",
							 g_ss_nsections, g_ss_demo_win_h );
					fflush( stderr );
				}
				break;

			case DW_SsPhase_SectionCapture:
			{
				if ( g_ss.section_index >= g_ss_nsections )
				{
					// All sections captured -- restore window then capture showcase.png last
					HWND hwnd = FindWindowA( NULL, "Dear Widgets Demo" );
					if ( hwnd )
						DW_ResizeOsWindow( hwnd, g_ss.base_client_w, g_ss.base_client_h );
					g_ss_scroll_y = -1.0f;
					g_ss_demo_win_h = -1.0f;
					g_ss.correct_scroll = -1.0f;
					g_ss.phase = DW_SsPhase_Showcase;
					g_ss.phase_frames = 0;
					break;
				}

				const DW_SsSection& sec = g_ss_sections[g_ss.section_index];

				if ( g_ss.section_pass == 0 )
				{
					// Scroll so the section lands at InnerRect.Min.y.
					// For section 0 bootstrap from the recorded start_y (near top, correct).
					// For subsequent sections use the running correct_scroll accumulated from
					// actual remeasured heights, so accumulated errors don't compound.
					ImGuiWindow* dw_curr = ImGui::FindWindowByName( "Dear Widgets" );
					float top_chrome = dw_curr ? (dw_curr->InnerRect.Min.y - dw_curr->Pos.y) : 27.0f;

					if ( g_ss.correct_scroll < 0.0f )
						g_ss.correct_scroll = ImMax( 0.0f, sec.start_y - top_chrome );

					g_ss_scroll_y = g_ss.correct_scroll;

					g_ss.section_pass = 1;
					g_ss.phase_frames = 0;
				}
				else if ( g_ss.section_pass == 1 )
				{
					// Settle: wait for scroll to take effect (SetScrollY is deferred one frame)
					if ( g_ss.phase_frames >= 3 )
					{
						g_ss.section_pass = 2;
						g_ss.phase_frames = 0;
					}
				}
				else if ( g_ss.section_pass == 2 )
				{
					// ShowDemo() already ran this frame before the state machine.
					// Trigger remeasure: ShowDemo() will call DW_SsRecord next frame with
					// this section on-screen and avail.y = full window height.
					// DW_SsRecord will compute DC (screen-space) positions for exact crop.
					g_ss_remeasure_idx = g_ss.section_index;
					g_ss.section_pass = 3;
					g_ss.phase_frames = 0;
				}
				else if ( g_ss.section_pass == 3 )
				{
					// ShowDemo() ran first this frame: DW_SsRecord fired for this section
					// and populated sec.dc_y0 / sec.dc_y1 with screen-space positions.
					g_ss_remeasure_idx = -1;  // safety clear

					HWND         hwnd = FindWindowA( NULL, "Dear Widgets Demo" );
					ImGuiWindow* dw = ImGui::FindWindowByName( "Dear Widgets" );
					bool         want = ( g_ss.section_filter[0] == '\0' ) || ( strstr( sec.name, g_ss.section_filter ) != nullptr );
					if ( want && hwnd && dw && !dw->Hidden && !dw->Collapsed )
					{
						float hdr_skip = g_ss.with_headers ? 0.0f : ImGui::GetFrameHeightWithSpacing();

						// Use DC positions for the crop -- they reflect the actual on-screen
						// position including indent, and avail.y-driven content height.
						float cap_x_f = (sec.dc_x0 > 0.0f)
							? sec.dc_x0
							: (dw->InnerRect.Min.x + sec.indent_offset);
						int cap_x = (int)cap_x_f;
						int cap_y = (sec.dc_y0 > 0.0f)
							? (int)(sec.dc_y0 + hdr_skip)
							: (int)(dw->InnerRect.Min.y + hdr_skip);
						int cap_w = (int)(dw->InnerRect.Max.x - cap_x_f);
						int cap_h = (sec.dc_y0 > 0.0f && sec.dc_y1 > sec.dc_y0)
							? (int)ImMax( sec.dc_y1 - sec.dc_y0 - hdr_skip, 1.0f )
							: (int)ImMax( sec.end_y - sec.start_y - hdr_skip, 1.0f );

						char safe[128];
						ImStrncpy( safe, sec.name, sizeof( safe ) );
						for ( char* p = safe; *p; ++p )
							if ( *p == '/' || *p == '\\' || *p == ' ' || *p == ':' || *p == '#' )
								*p = '_';

						char path[1024];
						snprintf( path, sizeof( path ), "%s\\%04d_%s.png",
								  g_ss.out_dir, g_ss.section_index, safe );

						bool ok = DW_CaptureClientAreaPNG( hwnd, path, cap_x, cap_y, cap_w, cap_h );
						fprintf( stderr, "[screenshot] %s  %s  dc=(%.0f,%.0f)  h=%d\n",
								 ok ? "OK  " : "FAIL", path, sec.dc_y0, sec.dc_y1, cap_h );
						fflush( stderr );

						// When filtering to a single section, stop once captured.
						if ( g_ss.section_filter[0] != '\0' )
						{
							g_ss.done = true;
							g_ss.phase = DW_SsPhase_Done;
						}
					}

					// Advance running correct_scroll by the actual (DC-measured) section height
					// plus the original recorded gap to the next section.
					{
						float dc_h = (sec.dc_y0 > 0.0f && sec.dc_y1 > sec.dc_y0)
							? (sec.dc_y1 - sec.dc_y0)
							: (sec.end_y - sec.start_y);
						g_ss.correct_scroll += dc_h;
						int next_idx = g_ss.section_index + 1;
						if ( next_idx < g_ss_nsections )
						{
							// Gap = cursor advancement between end of this section and start of next.
							// Use original recorded values (start_y is scroll-independent, unchanged).
							float gap = g_ss_sections[next_idx].start_y - sec.end_y;
							g_ss.correct_scroll += ImMax( 0.0f, gap );
						}
					}

					g_ss.section_index++;
					g_ss.section_pass = 0;
					g_ss.phase_frames = 0;
				}
				break;
			}

			case DW_SsPhase_Showcase:
			{
				if ( g_ss.phase_frames == 1 )
				{
					// Resize OS window to 1450px client height -- well above content height (~975px).
					// The window may extend off-screen; PrintWindow(PW_RENDERFULLCONTENT) captures
					// the full D3D backbuffer including off-screen portions, so this is fine.
					HWND hwnd = FindWindowA( NULL, "Dear Widgets Demo" );
					if ( hwnd )
					{
						DWORD winStyle = (DWORD)GetWindowLongA( hwnd, GWL_STYLE );
						DWORD exstyle = (DWORD)GetWindowLongA( hwnd, GWL_EXSTYLE );
						RECT  rc = { 0, 0, 820, 1450 };
						AdjustWindowRectEx( &rc, winStyle, FALSE, exstyle );
						SetWindowPos( hwnd, nullptr, 0, 0,
									  rc.right - rc.left, rc.bottom - rc.top,
									  SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
					}
					g_ss_showcase_capture_h = -1.0f;
					// Push Demo and Samples windows off-screen; bring Showcase on-screen
					g_ss_demo_area_x = 5000.0f;
					g_ss_showcase_area_x = 10.0f;
				}
				// After content has rendered, measure the actual scrollable content height.
				if ( g_ss.phase_frames == 4 )
				{
					ImGuiWindow* win = ImGui::FindWindowByName( "Showcase" );
					if ( win && win->ContentSize.y > 10.0f )
					{
						float pad = ImGui::GetStyle().WindowPadding.y;
						g_ss_showcase_capture_h = win->ContentSize.y
							+ win->TitleBarHeight   // field since imgui 2024/05/28
							+ pad * 2.0f
							+ 4.0f; // small rounding margin
						fprintf( stderr, "[screenshot] Showcase content height measured: %.0fpx -> capture %.0fpx\n",
								 win->ContentSize.y, g_ss_showcase_capture_h );
						fflush( stderr );
					}
				}
				if ( g_ss.phase_frames >= 7 )
				{
					HWND hwnd = FindWindowA( NULL, "Dear Widgets Demo" );
					if ( hwnd )
					{
						ImGuiWindow* win = ImGui::FindWindowByName( "Showcase" );
						if ( win && !win->Hidden && !win->Collapsed )
						{
							char path[1024];
							snprintf( path, sizeof( path ), "%s\\showcase.png", g_ss.out_dir );
							int x = 10, y = 10, w = 800;
							int h = (g_ss_showcase_capture_h > 0.0f)
								? (int)g_ss_showcase_capture_h
								: (int)g_ss_showcase_h;
							bool ok = DW_CaptureClientAreaPNG( hwnd, path, x, y, w, h );
							fprintf( stderr, "[screenshot] %s  %s  (%dx%d)\n",
									 ok ? "OK  " : "FAIL", path, w, h );
							fflush( stderr );
						}
					}
					// Restore windows
					g_ss_demo_area_x = 10.0f;
					g_ss_showcase_area_x = 5000.0f;
					HWND hwnd2 = FindWindowA( NULL, "Dear Widgets Demo" );
					if ( hwnd2 )
						DW_ResizeOsWindow( hwnd2, g_ss.base_client_w, g_ss.base_client_h );
					g_ss.phase = DW_SsPhase_Done;
					g_ss.done = true;
				}
				break;
			}

			case DW_SsPhase_Done:
				g_ss.done = true;
				break;
			}
		}
		if ( g_ss.active && g_ss.done )
			break;
#endif
	}

	// Cleanup
	ImDwDownload::Shutdown();
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
	// Intentionally empty: disabled offscreen demo.
}

namespace ImWidgets{

	static void AspectRatio_6_2( ImGuiSizeCallbackData* data )
	{
		float aspect_ratio = *(float*)data->UserData;
		data->DesiredSize.y = (float)(int)(data->DesiredSize.x / aspect_ratio);
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
		ImGui::SetNextWindowSizeConstraints( ImVec2( 0, 0 ), ImVec2( FLT_MAX, FLT_MAX ), AspectRatio_6_2, (void*)&_6_2 );

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
	static void ApplyOpenAll()
	{
		int eff = (s_open_all != 0) ? s_open_all : g_ss_open_all;
		if ( eff != 0 ) ImGui::SetNextItemOpen( eff > 0, ImGuiCond_Always );
	}
	static float CanvasSize()
	{
		return ImMin( ImGui::GetContentRegionAvail().x, ImPlatform_LpToPx( 400.0f ) );
	}

	// Scroll culling: skip section content when entirely off-screen.
	// cached_h stores previous frame height (0 = first frame, always render to measure).
	// Returns true if the section should render its content.
	static bool BeginCullSection( float& cached_h, float& out_start_y )
	{
		out_start_y = ImGui::GetCursorScreenPos().y;
		if ( cached_h > 0.0f )
		{
			ImVec4 cr = ImGui::GetWindowDrawList()->_CmdHeader.ClipRect;
			if ( out_start_y + cached_h < cr.y || out_start_y > cr.w )
			{
				ImGui::Dummy( ImVec2( 0, cached_h ) );
				return false;
			}
		}
		return true;
	}
	static void EndCullSection( float& cached_h, float start_y )
	{
		float end_y = ImGui::GetCursorScreenPos().y;
		if ( end_y > start_y ) cached_h = end_y - start_y;
	}

	void ShowDrawShapeDemo()
	{
		ApplyOpenAll();
		if ( ImGui::CollapsingHeader( "Draw Shape" ) )
		{
			static float s_cull_h = 0; float s_cull_y;
			if ( BeginCullSection( s_cull_h, s_cull_y ) )
			{

				float const size = CanvasSize();
				ImDrawList* pDrawList = ImGui::GetWindowDrawList();

				static ShapeDebugState debug_state;
				static GradientParams gradient;
				static ImWidgetsShape shape;
#ifdef DEAR_WIDGETS_TESSELATION
				static int tess = 2;
				ImGui::SliderInt( "Tess##DrawShape", &tess, 0, 16 );
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
				EndCullSection( s_cull_h, s_cull_y );
			}
		}  // end CollapsingHeader "Draw Shape"
	}

	void ShowDrawTextDemo()
	{
		ApplyOpenAll();
		if ( !ImGui::CollapsingHeader( "GPU Text (Slug)" ) )
			return;
		static float s_cull_h = 0; float s_cull_y;
		if ( !BeginCullSection( s_cull_h, s_cull_y ) ) return;

		// Hot-reload any fonts that arrived via the downloader, then render
		// the Download UI. These run BEFORE the no-fonts-loaded early-exit
		// so the user can always click Download even on a fresh clone.
		ImDwDownload::Tick();
		LoadOrRefreshDemoFonts( ImGui::GetIO() );
		ImDwDownload::DrawDownloadAllButton();

		if ( !g_cinzelFont && !g_alfaSlabFont && !g_dottedFont &&
			 !g_franticallyFont && !g_loveLightFont && !g_magnoliaFont &&
			 !g_nablaFont && !g_squareLilyFont )
		{
			ImGui::TextDisabled( "No Slug fonts loaded yet -- click \"Download all fonts\" above." );
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
		ImGui::DragFloat( "Font Size##SlugDemo", &font_size, 0.5f, 8.0f, 300.0f, "%.0f lp" );
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

		enum FontTextType
		{
			kLatin = 0, kEmoji = 1, kArabic = 2
		};
		// `samplePrefix` is an optional per-font sample string prepended to the
		// default text buffer when rendering. Used by the Ligature Showcase
		// entries to seed each row with characters that actually trigger the
		// font's specific ligature set (fi fl ct st for classical serifs,
		// => -> for programming mono, ae oe for medieval, etc.). Unspecified /
		// nullptr -> render only the shared text_buf.
		struct FontEntry
		{
			ImFont** font; const char* label; FontTextType textType; const char* group; const char* samplePrefix;
		};
		static const char* kGrpCode = "Programming / Code";
		static const char* kGrpSerif = "Serif";
		static const char* kGrpScript = "Script / Handwriting";
		static const char* kGrpDisp = "Display / Decorative";
		static const char* kGrpCFF = "CFF Monochrome";
		static const char* kGrpColr0 = "Color: COLR v0";
		static const char* kGrpSVG = "Color: SVG";
		static const char* kGrpColr1 = "Color: COLR v1 / Gradient";
		static const char* kGrpArabic = "Arabic";
		static const char* kGrpLig = "Ligature Showcase";
		// Labels use the "orig -> replacement" form when the demo font was
		// swapped out for a Google Fonts / Fontshare / Velvetyne equivalent
		// (see workingdir/fonts/ and font_manifest.inl). Untouched entries
		// show just the Google Fonts name.
		static const FontEntry kFonts[] = {
			{ &g_firaCodeFont,    "Fira Code",                              kLatin,  kGrpCode },
			{ &g_monblockFont,    "Monblock -> Sligoil (Velvetyne)",         kLatin,  kGrpCode },
			{ &g_cinzelFont,      "Cinzel",                                  kLatin,  kGrpSerif },
			{ &g_alfaSlabFont,    "Alfa Slab One",                           kLatin,  kGrpSerif },
			{ &g_classicalFont,   "Classical Aesthetics -> Cinzel Decorative", kLatin, kGrpSerif },
			{ &g_foglihtenFont,   "Foglihten No07 -> UnifrakturCook",         kLatin,  kGrpSerif },
			{ &g_steelworksFont,  "Steelworks Vintage -> Rye",                kLatin,  kGrpSerif },
			{ &g_trenchSlabFont,  "Trench Slab (Fontshare)",                 kLatin,  kGrpSerif },
			{ &g_akturaFont,      "Aktura (Fontshare)",                       kLatin,  kGrpSerif },
			{ &g_britneyFont,     "Britney (Fontshare)",                      kLatin,  kGrpSerif },
			{ &g_kihimFont,            "Kihim (Fontshare)",         kLatin, kGrpSerif },
			{ &g_zinaFont,             "Zina (Fontshare)",          kLatin, kGrpSerif },
			{ &g_alegreyaFont,         "Alegreya",                  kLatin, kGrpSerif },
			{ &g_cormorantFont,        "Cormorant",                 kLatin, kGrpSerif },
			{ &g_cormorantUnicaseFont, "Cormorant Unicase",         kLatin, kGrpSerif },
			{ &g_frauncesFont,         "Fraunces",                  kLatin, kGrpSerif },
			{ &g_italianaFont,         "Italiana",                  kLatin, kGrpSerif },
			{ &g_yesevaOneFont,        "Yeseva One",                kLatin, kGrpSerif },
			{ &g_brightMarchFont, "Bright Marching -> Sacramento",            kLatin,  kGrpScript },
			{ &g_camoodFont,      "Camood -> Kaushan Script",                 kLatin,  kGrpScript },
			{ &g_cheronaFont,     "Cherona -> Great Vibes",                   kLatin,  kGrpScript },
			{ &g_loveLightFont,   "Love Light",                               kLatin,  kGrpScript },
			{ &g_metaforaSsFont,  "Metafora Stylistic -> Sail",               kLatin,  kGrpScript },
			{ &g_reginaFont,          "Regina -> Homemade Apple",   kLatin, kGrpScript },
			{ &g_sharpieFont,         "Sharpie (Fontshare)",        kLatin, kGrpScript },
			{ &g_hurricaneFont,       "Hurricane",                  kLatin, kGrpScript },
			{ &g_imperialScriptFont,  "Imperial Script",            kLatin, kGrpScript },
			{ &g_ephesisFont,         "Ephesis",                    kLatin, kGrpScript },
			{ &g_bollgoFont,      "Bollgo -> Flor de Ruina (Velvetyne)",      kLatin,  kGrpDisp },
			{ &g_dottedFont,      "Dotted -> Bianzhidai (Velvetyne)",         kLatin,  kGrpDisp },
			//{ &g_endlessFont,   "Endlessly Expanded",                       kLatin,  kGrpDisp },
			{ &g_franticallyFont, "Frantically -> Mess (Velvetyne)",          kLatin,  kGrpDisp },
			{ &g_gimboFont,       "Gimbo -> Pilowlava (Velvetyne)",           kLatin,  kGrpDisp },
			{ &g_gingaFont,       "Ginga -> Interlope (Velvetyne)",           kLatin,  kGrpDisp },
			{ &g_magnoliaFont,    "Magnolia -> Letters (Velvetyne)",          kLatin,  kGrpDisp },
			{ &g_molgethFont,     "Molgeth -> Fungal (Velvetyne)",            kLatin,  kGrpDisp },
			{ &g_squareLilyFont,  "Square Lily -> Lithops (Velvetyne)",       kLatin,  kGrpDisp },
			{ &g_ouvrieresFont,   "Ouvrieres (Velvetyne)",                    kLatin,  kGrpDisp },
			{ &g_picnicFont,      "Picnic (Velvetyne)",                       kLatin,  kGrpDisp },
			{ &g_comicoFont,         "Comico (Fontshare)",                   kLatin,  kGrpDisp },
			{ &g_bespokeStencilFont, "Bespoke Stencil (Fontshare)",          kLatin,  kGrpDisp },
			{ &g_styroFont,          "Styro (Fontshare)",                    kLatin,  kGrpDisp },
			{ &g_boxingFont,         "Boxing (Fontshare)",                   kLatin,  kGrpDisp },
			{ &g_kolaFont,           "Kola (Fontshare)",                     kLatin,  kGrpDisp },
			{ &g_striperFont,        "Striper (Fontshare)",                  kLatin,  kGrpDisp },
			{ &g_kohinoorZeroneFont, "Kohinoor Zerone (Fontshare)",          kLatin,  kGrpDisp },
			{ &g_monotonFont,        "Monoton",                              kLatin,  kGrpDisp },
			{ &g_manbowClearFont,        "Manbow Clear -> Array (Fontshare, CFF)", kLatin, kGrpCFF },
			{ &g_manbowLinesFont,        "Manbow Lines -> Tanker (Fontshare, CFF)", kLatin, kGrpCFF },
			{ &g_manbowSpotsFont,        "Manbow Spots",                 kLatin,  kGrpCFF },
			{ &g_manbowToneFont,         "Manbow Lines (DaFont)",         kLatin,  kGrpCFF },
			{ &g_twemojiFont,            "Twemoji -> Noto Color Emoji (COLRv1)", kEmoji, kGrpColr0 },
			{ &g_coralPixelsFont,        "Coral Pixels",                 kLatin,  kGrpColr0 },
			{ &g_openMojiColr0Font,      "OpenMoji Color (COLRv0)",      kEmoji,  kGrpColr0 },
			{ &g_aquaphonicDownpourFont, "Aquaphonic Downpour",           kLatin,  kGrpSVG },
			{ &g_aquaphonicDrizzleFont,  "Aquaphonic Drizzle",            kLatin,  kGrpSVG },
			{ &g_cimeroProFont,          "Cimero Pro",                    kLatin,  kGrpSVG },
			{ &g_colorTubeFont,          "Color Tube (unmatched)",        kLatin,  kGrpSVG },
			{ &g_gilbertColorFont,       "Gilbert Color Bold",            kLatin,  kGrpSVG },
			{ &g_multicoloreFont,        "Multicolore Pro",               kLatin,  kGrpSVG },
			{ &g_primecolorGFont,        "Primecolor G",                  kLatin,  kGrpSVG },
			{ &g_primecolorMFont,        "Primecolor M",                  kLatin,  kGrpSVG },
			{ &g_fatternFont,            "Fattern",                       kLatin,  kGrpSVG },
			{ &g_notoColorEmojiSvgFont,  "Noto Color Emoji (OT-SVG)",     kEmoji,  kGrpSVG },
			{ &g_nablaFont,              "Nabla",                         kLatin,  kGrpColr1 },
			{ &g_primecolorCV1Font,      "Primecolor CV1 -> Bungee Spice", kLatin, kGrpColr1 },
			{ &g_bungeeSpiceFont,        "Bungee Spice",                  kLatin,  kGrpColr1 },
			{ &g_honkFont,               "Honk",                          kLatin,  kGrpColr1 },
			{ &g_openMojiColr1Font,      "OpenMoji Color (COLRv1)",       kEmoji,  kGrpColr1 },
			{ &g_fluentEmojiFont,        "Microsoft Fluent Emoji (COLRv1)", kEmoji, kGrpColr1 },
			{ &g_amiriFont,              "Amiri",                 kArabic, kGrpArabic },
			{ &g_cairoPlayBoldFont,      "Cairo Play Bold",       kArabic, kGrpArabic },
			{ &g_cairoPlayXLightFont,    "Cairo Play ExtraLight", kArabic, kGrpArabic },
			{ &g_arefRuqaaBoldFont,      "Aref Ruqaa Ink Bold",  kArabic, kGrpArabic },
			{ &g_blakaInkFont,           "Blaka Ink",             kArabic, kGrpArabic },
			{ &g_reemKufiInkFont,        "Reem Kufi Ink",         kArabic, kGrpArabic },
			{ &g_reemKufiFunFont,        "Reem Kufi Fun",         kArabic, kGrpArabic },
			{ &g_vazirmatnFont,          "Vazirmatn (rastikerdar)", kArabic, kGrpArabic },
			{ &g_amiriQuranFont,         "Amiri Quran (aliftype upstream)", kArabic, kGrpArabic },
			{ &g_amiriQuranColoredFont,  "Amiri Quran Colored (COLR)",      kArabic, kGrpArabic },
			// Ligature Showcase -- programming arrows, classical ct/st/Th,
			// medieval ae/oe, decorative script flourishes. Sample text
			// "=><=Dear Widgets!" already triggers => and <= for mono fonts;
			// switch to something like "affection fluffy office Thirty" to
			// see fi/fl/ffi/ct/st/Th on Cormorant / EB Garamond / Fraunces.
			{ &g_jetbrainsMonoFont,   "JetBrains Mono (programming)",          kLatin, kGrpLig, "=> -> != === >= <= :: |> <- "              },
			{ &g_victorMonoFont,      "Victor Mono (italic cursive)",          kLatin, kGrpLig, "=> -> != === /* */ // "                     },
			{ &g_monaspaceNeonFont,   "Monaspace Neon (texture healing)",      kLatin, kGrpLig, "=> -> != === >= <= "                        },
			{ &g_ebGaramondFont,      "EB Garamond (classical oldstyle)",      kLatin, kGrpLig, "fi fl ffi ffl ct st -- "                     },
			{ &g_lobsterFont,         "Lobster (retro script)",                kLatin, kGrpLig, "The fi Th -- "                               },
			{ &g_abrilFatfaceFont,    "Abril Fatface (Didone display)",        kLatin, kGrpLig, "fi fl Th -- "                                },
			{ &g_meaCulpaFont,        "Mea Culpa (extreme calligraphy)",       kLatin, kGrpLig, "fi fl Th -- "                                },
			// UTF-8 for medieval glyphs: ae=C3 A6, oe=C5 93, ss=C5 BF C5 BF, st=C5 BF t, Eth=C3 90, thorn=C3 BE
			{ &g_junicodeFont,        "Junicode (medieval historical)",        kLatin, kGrpLig, "\xC3\xA6 \xC5\x93 \xC5\xBF\xC5\xBF \xC5\xBFt \xC3\x90 \xC3\xBE -- " },
			{ &g_tapestryFont,        "Tapestry (ornate script)",              kLatin, kGrpLig, "fi fl -- "                                   },
			{ &g_birthstoneBounceFont, "Birthstone Bounce (bouncy flourished)",   kLatin, kGrpLig, "The fi Th -- "                            },
			{ &g_monteCarloFont,       "MonteCarlo (Spencerian flourishes)",      kLatin, kGrpLig, "The fi Th Q -- "                          },
			{ &g_mrsSaintDelafieldFont,"Mrs Saint Delafield (Spencerian)",        kLatin, kGrpLig, "The fi Th -- "                            },
			{ &g_sansitaSwashedFont,   "Sansita Swashed (sans + swashes)",        kLatin, kGrpLig, "fi fl Th Qu -- "                          },
			{ &g_bodoniModaFont,       "Bodoni Moda (variable Didone)",           kLatin, kGrpLig, "fi fl ffi ffl Th -- "                     },
			{ &g_unifrakturMaguntiaFont, "Unifraktur Maguntia (blackletter ligs)", kLatin, kGrpLig, "\xC5\xBF" "ch \xC5\xBF\xC5\xBF" "i ch ck ll tz \xC3\x9F -- " },
			{ &g_alluraFont,           "Allura (script word ligs: The/tion/ing)", kLatin, kGrpLig, "The tion ion ing are ous -- "            },
			{ &g_grenzeGotischFont,    "Grenze Gotisch (modern blackletter)",     kLatin, kGrpLig, "\xC5\xBF" "ch ch ck ll tz \xC3\x9F -- "    },
			{ &g_pirataOneFont,        "Pirata One (tattoo gothic)",              kLatin, kGrpLig, "ch ck tz Th The -- "                      },
			{ &g_ruthieFont,           "Ruthie (extra-ornate script)",            kLatin, kGrpLig, "The and of tion ing -- "                  },
			{ &g_leMurmureFont,        "Le Murmure (tall art-nouveau)",           kLatin, kGrpLig, "fi fl ff Th The -- "                      },
			{ &g_caudexFont,           "Caudex (medievalist scholarly)",          kLatin, kGrpLig, "\xC3\xA6 \xC5\x93 \xC5\xBF\xC5\xBF ct st Th \xC3\x9F -- " },
			{ &g_unifrakturCookFont,   "UnifrakturCook (sharper blackletter)",    kLatin, kGrpLig, "\xC5\xBF" "ch ch ck ll tz \xC3\x9F -- "    },
			{ &g_chomskyFont,          "Chomsky (NYT masthead gothic)",           kLatin, kGrpLig, "ct st sp Th \xC5\xBFt -- "                },
			{ &g_majorMonoFont,        "Major Mono Display (caps mono)",          kLatin, kGrpLig, "TH NG OO LY TT -- "                       },
		};

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		float const canvas_w = CanvasSize();
		float const gap = ImGui::GetStyle().ItemSpacing.y;

		// Separate text buffer for emoji (Twemoji uses its own codepoints, not Latin text)
		static char emoji_buf[256] = "\xF0\x9F\x98\x80\xF0\x9F\x94\xA5\xF0\x9F\x8C\x88\xF0\x9F\x8E\xA8\xF0\x9F\x9A\x80\xF0\x9F\x92\xA1\xF0\x9F\x8C\x8D";
		// Emoji: smiley, fire, rainbow, art, rocket, bulb, earth (see escaped string above)
		ImGui::InputText( "Emoji##SlugEmoji", emoji_buf, sizeof( emoji_buf ) );
		// Arabic text buffer: (Dear Widgets in Arabic) (Dear Widgets)
		static char arabic_buf[256] = "\xd8\xa7\xd9\x84\xd8\xa3\xd8\xaf\xd9\x88\xd8\xa7\xd8\xaa \xd8\xa7\xd9\x84\xd8\xb9\xd8\xb2\xd9\x8a\xd8\xb2\xd8\xa9";
		ImGui::InputText( "Arabic##SlugArabic", arabic_buf, sizeof( arabic_buf ) );

		static int s_open_fonts = 0;
		if ( ImGui::Button( "Open Fonts" ) )  s_open_fonts = 1;
		ImGui::SameLine();
		if ( ImGui::Button( "Close Fonts" ) ) s_open_fonts = -1;

		ImGui::Separator();
		const char* currentGroup = NULL;
		bool groupOpen = false;
		float groupY0 = 0.0f;
		int nFonts = IM_ARRAYSIZE( kFonts );
		ImVec4 fontClipRect = pDrawList->_CmdHeader.ClipRect;
		float labelLineH = ImGui::GetTextLineHeightWithSpacing();
		static float s_cached_font_h[128]; // per-entry layout height cache

		// CalcTextSize cache: avoid re-shaping every frame when text/size unchanged
		static ImVec2 s_cached_sz[128];
		static float  s_cached_asc[128];
		static ImU32  s_text_cache_key = 0;
		ImU32 text_cache_key = ImHashData( &font_size, sizeof( font_size ) );
		text_cache_key = ImHashStr( text_buf, 0, text_cache_key );
		text_cache_key = ImHashStr( emoji_buf, 0, text_cache_key );
		text_cache_key = ImHashStr( arabic_buf, 0, text_cache_key );
		bool textCacheValid = (text_cache_key == s_text_cache_key);
		s_text_cache_key = text_cache_key;

		// When font_size or any text buffer changes, every per-entry cache
		// becomes stale. Clearing here is critical for the scroll-culled rows:
		// they rely on `s_cached_font_h[i]` as est_h, and without invalidation
		// off-screen rows at the new size use a size-mismatched height (from
		// the previous font_size), which visibly pushes visible neighbours
		// out of place. Clearing also lets the prewarm below re-populate.
		if ( !textCacheValid )
		{
			for ( int k = 0; k < IM_ARRAYSIZE( s_cached_font_h ); ++k )
			{
				s_cached_font_h[k] = 0.0f;
				s_cached_sz[k] = ImVec2( 0.0f, 0.0f );
				s_cached_asc[k] = 0.0f;
			}
		}

		// Pre-warm: compute one uncached CalcTextSize per frame so scrolling hits warm cache
		if ( textCacheValid )
		{
			for ( int pw = 0; pw < nFonts; pw++ )
			{
				if ( !*kFonts[pw].font || s_cached_sz[pw].y > 0 ) continue;
				ImFont* pwf = *kFonts[pw].font;
				const char* pws = (kFonts[pw].textType == kEmoji) ? emoji_buf : (kFonts[pw].textType == kArabic) ? arabic_buf : text_buf;
				char pwsComposed[512];
				if ( kFonts[pw].samplePrefix && kFonts[pw].textType == kLatin )
				{
					ImFormatString( pwsComposed, sizeof( pwsComposed ), "%s%s", kFonts[pw].samplePrefix, pws );
					pws = pwsComposed;
				}
				s_cached_sz[pw] = ImWidgets::CalcTextSize( pwf, font_size, pws, nullptr, &s_cached_asc[pw] );
				break; // one per frame
			}
		}

		for ( int i = 0; i <= nFonts; i++ )
		{
			const char* nextGroup = (i < nFonts) ? kFonts[i].group : NULL;
			if ( nextGroup != currentGroup )
			{
				if ( currentGroup != NULL )
					DW_SsRecord( currentGroup, groupY0, ImGui::GetCursorPos().y );
				currentGroup = nextGroup;
				if ( nextGroup != NULL )
				{
					groupY0 = ImGui::GetCursorPos().y;
					if ( s_open_fonts != 0 ) ImGui::SetNextItemOpen( s_open_fonts > 0, ImGuiCond_Always );
					else ApplyOpenAll();
					groupOpen = ImGui::CollapsingHeader( nextGroup );
					if ( groupOpen )
					{
						// Per-category bulk download button (see font_manifest.inl).
						ImDwDownload::DrawCategoryDownloadButton( nextGroup );
					}
				}
			}
			if ( i >= nFonts ) break;
			if ( !groupOpen ) continue;

			// Missing-font placeholder: offer a single-font download button
			// in place of the glyph preview so the category remains visible.
			if ( !*kFonts[i].font )
			{
				ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 0, 0, 0, 255 ) );
				ImGui::Text( "%s:", kFonts[i].label );
				ImGui::PopStyleColor();
				// Pair this FontEntry with its manifest slot: label must START
				// with display (not just contain it -- substring match caused
				// e.g. label "Classical Aesthetics -> Cinzel Decorative" to
				// resolve to the standalone "Cinzel" entry, producing
				// duplicate ImGui IDs).
				int dlIdx = -1;
				for ( int mi = 0; mi < ImDwDownload::kFontMetaCount; ++mi )
				{
					const ImDwDownload::Meta& m = ImDwDownload::kFontMeta[mi];
					if ( std::strcmp( m.category, kFonts[i].group ) != 0 ) continue;
					size_t dlen = std::strlen( m.display );
					if ( std::strncmp( kFonts[i].label, m.display, dlen ) == 0 )
					{
						dlIdx = mi;
						break;
					}
				}
				ImGui::SameLine();
				if ( dlIdx >= 0 ) ImDwDownload::DrawSingleFontDownloadButton( dlIdx );
				else ImGui::TextDisabled( "(not in manifest)" );
				continue;
			}

			// Per-font-line scroll culling: skip CalcTextSize + DrawText for off-screen entries
			float entryY = ImGui::GetCursorScreenPos().y;
			// First-frame cull estimate: match the visible-row formula below so
			// culled rows don't over/undershoot before the measured height is
			// cached. Must track the line_h floor used when rendering.
			float est_h = (s_cached_font_h[i] > 0) ? s_cached_font_h[i] : (labelLineH + font_size * 1.95f + gap);
			if ( entryY + est_h < fontClipRect.y || entryY > fontClipRect.w )
			{
				ImGui::Dummy( ImVec2( canvas_w, est_h ) );
				continue;
			}

			const FontEntry& e = kFonts[i];
			ImFont* f = *e.font;
			const char* drawStr = (e.textType == kEmoji) ? emoji_buf : (e.textType == kArabic) ? arabic_buf : text_buf;
			// Prepend per-font ligature showcase (e.g. "fi fl ct st -- ") so
			// the row demonstrates the ligatures the font actually supports.
			// Only used for Latin entries that explicitly set samplePrefix.
			char drawComposed[512];
			if ( e.samplePrefix && e.textType == kLatin )
			{
				ImFormatString( drawComposed, sizeof( drawComposed ), "%s%s", e.samplePrefix, drawStr );
				drawStr = drawComposed;
			}

			float   asc = 0.0f;
			ImVec2  sz;
			if ( textCacheValid && s_cached_sz[i].y > 0 )
			{
				sz = s_cached_sz[i];
				asc = s_cached_asc[i];
			}
			else
			{
				sz = ImWidgets::CalcTextSize( f, font_size, drawStr, nullptr, &asc );
				s_cached_sz[i] = sz;
				s_cached_asc[i] = asc;
			}
			// Some display/script fonts report a shaped-ink bbox smaller than the
			// font's natural line box -- the shaped measurement only covers the
			// glyphs in the current sample text, but many Script/Display fonts
			// have tall flourishes on specific capitals (W/D/Q/P swashes) and
			// generous designed leading that the tight ink bbox doesn't capture.
			//
			// At large font_size (100+ lp), the fixed `gap` (ImGui ItemSpacing,
			// ~8 px) becomes a negligible fraction of the row, so undercounted
			// sz.y leads to visible overlap between rows. Fix: floor at
			// font_size * 1.8 (a conservative Script/Display line-height) and
			// scale the trailing gap proportionally so spacing stays visually
			// consistent across sizes.
			float   line_h = ImMax( sz.y, font_size * 1.8f ) + gap + font_size * 0.15f;

			// Font name in solid black -- struck-through when flagged for deletion.
			// The little "[X]" / "[ ]" button at the end of the row toggles the
			// flag; all flagged labels are written to workingdir/_fonts_to_delete.txt
			// so Claude can pick them up for a batch removal pass.
			static bool s_flagged[128] = {};
			static bool s_flags_dirty = false;
			ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 0, 0, 0, 255 ) );
			if ( s_flagged[i] )
			{
				// Strikethrough rendering via two text draws + a line.
				ImVec2 tpos = ImGui::GetCursorScreenPos();
				ImVec2 tsize = ImGui::CalcTextSize( e.label );
				ImGui::Text( "%s:", e.label );
				pDrawList->AddLine(
					ImVec2( tpos.x, tpos.y + tsize.y * 0.55f ),
					ImVec2( tpos.x + tsize.x, tpos.y + tsize.y * 0.55f ),
					IM_COL32( 200, 0, 0, 255 ), 1.5f );
			}
			else
			{
				ImGui::Text( "%s:", e.label );
			}
			ImGui::PopStyleColor();
			// Flag toggle right after the font-name label (a fixed right-edge
			// alignment hid the button off-screen for narrow windows -- inline
			// next to the label is always visible).
			ImGui::SameLine();
			ImGui::PushID( i + 20000 );
			bool was_flagged = s_flagged[i];
			const char* btn_lbl = was_flagged ? "X" : "-";
			if ( was_flagged )
				ImGui::PushStyleColor( ImGuiCol_Button, IM_COL32( 180, 40, 40, 255 ) );
			if ( ImGui::SmallButton( btn_lbl ) )
			{
				s_flagged[i] = !s_flagged[i];
				s_flags_dirty = true;
			}
			if ( was_flagged )
				ImGui::PopStyleColor();
			if ( ImGui::IsItemHovered() )
				ImGui::SetTooltip( s_flagged[i] ? "Flagged for deletion (click to unflag)" : "Flag this font for deletion" );
			ImGui::PopID();
			// Flush flag list to disk on any change so Claude can read it.
			if ( s_flags_dirty )
			{
				FILE* fp = fopen( "_fonts_to_delete.txt", "w" );
				if ( fp )
				{
					fprintf( fp, "# Fonts flagged for deletion (toggle in the Slug demo). One label per line.\n" );
					for ( int k = 0; k < nFonts && k < 128; k++ )
					{
						if ( s_flagged[k] ) fprintf( fp, "%s\n", kFonts[k].label );
					}
					fclose( fp );
				}
				s_flags_dirty = false;
			}

			// Optional colored background spanning the full window width
			ImVec2 pos = ImGui::GetCursorScreenPos();
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
				int dbgFlags = (dbg_curves_on ? 1 : 0) | (dbg_ctrl_on ? 2 : 0)
					| (dbg_bbox_on ? 4 : 0) | (dbg_bands_on ? 8 : 0);
				ImWidgets::DrawTextDebugCurves( pDrawList, f, font_size, ImVec2( pos.x, pos.y + asc ), drawStr, nullptr, dbgFlags );
			}
			if ( debug_layers )
				ImWidgets::DrawTextDebugLayers( pDrawList, f, font_size, ImVec2( pos.x, pos.y + asc ), drawStr );

			ImGui::Dummy( ImVec2( canvas_w, line_h ) );
			s_cached_font_h[i] = ImGui::GetCursorScreenPos().y - entryY;
		}
		s_open_fonts = 0;

		// Typography Fills section (inside GPU Text)
		// ---- Debug Glyph Tessellation ----
		{ float _sy0 = ImGui::GetCursorPos().y;
		ApplyOpenAll();
		if ( ImGui::CollapsingHeader( "Debug Glyph Tessellation" ) )
		{
			static float s_cull_dbgtess_h = 0; float s_cull_dbgtess_y;
			if ( BeginCullSection( s_cull_dbgtess_h, s_cull_dbgtess_y ) )
			{
				static char dbgChar[8] = "O";
				static int dbgFontIdx = 0;
				static float dbgSize = 200.0f;
				static float dbgTol = 0.5f;

				static const DemoFontChoice kDbgFonts[] = {
					{ "Fira Code", &g_firaCodeFont },
					{ "Monblock", &g_monblockFont },
					{ "Cinzel", &g_cinzelFont },
					{ "Alfa Slab", &g_alfaSlabFont },
					{ "Classical Aesthetics", &g_classicalFont },
					{ "Foglighten", &g_foglihtenFont },
					{ "Steelworks", &g_steelworksFont },
					{ "Trench Slab", &g_trenchSlabFont },
					{ "Bright March", &g_brightMarchFont },
					{ "Love Light", &g_loveLightFont },
					{ "Metafora Stylistic", &g_metaforaSsFont },
					{ "Bollgo", &g_bollgoFont },
					{ "Dotted", &g_dottedFont },
					{ "Frantically", &g_franticallyFont },
					{ "Gimbo", &g_gimboFont },
					{ "Ginga", &g_gingaFont },
					{ "Molgeth", &g_molgethFont },
					{ "Square Lily", &g_squareLilyFont },
					{ "Nabla", &g_nablaFont },
					{ "Bungee Spice", &g_bungeeSpiceFont },
				};
				ImGui::InputText( "Character##DbgTess", dbgChar, sizeof( dbgChar ) );
				ImFont* dbgFont = DemoFontPicker( "Font##DbgTess", kDbgFonts, IM_ARRAYSIZE( kDbgFonts ), &dbgFontIdx );
				ImGui::SliderFloat( "Size##DbgTess", &dbgSize, 32.0f, 400.0f, "%.0f lp" );
				ImGui::SliderFloat( "Tess Tol##DbgTess", &dbgTol, 0.05f, 5.0f, "%.2f" );
				static float dbgSpacing = 30.0f;
				ImGui::SliderFloat( "Piece Spacing##DbgTess", &dbgSpacing, 0.0f, 100.0f, "%.0f lp" );
				if ( dbgFont && dbgChar[0] )
				{
					ImVec2 dbgPos = ImGui::GetCursorScreenPos();
					float dbgRowH = dbgSize * 1.3f;
					ImWidgets::DrawTesselateDebug( pDrawList, dbgFont, dbgSize, dbgChar, dbgPos, dbgTol, dbgSpacing, dbgRowH );
				}
				EndCullSection( s_cull_dbgtess_h, s_cull_dbgtess_y );
			}
		}
		DW_SsRecord( "Debug_Glyph_Tessellation", _sy0, ImGui::GetCursorPos().y ); }

		// Typography Fills: tesselated text with gradient/image fills
		{
			float _sy0 = ImGui::GetCursorPos().y;
			ApplyOpenAll();
			if ( g_monblockFont && ImGui::CollapsingHeader( "Typography Fills" ) )
			{
				static float s_cull_typofills_h = 0; float s_cull_typofills_y;
				if ( BeginCullSection( s_cull_typofills_h, s_cull_typofills_y ) )
				{
					static int tyFontIdx = 0;
					static const DemoFontChoice kTypoFonts[] = {
						{ "Fira Code", &g_firaCodeFont },
						{ "Monblock", &g_monblockFont },
						{ "Cinzel", &g_cinzelFont },
						{ "Alfa Slab", &g_alfaSlabFont },
						{ "Classical Aesthetics", &g_classicalFont },
						{ "Foglighten", &g_foglihtenFont },
						{ "Steelworks", &g_steelworksFont },
						{ "Trench Slab", &g_trenchSlabFont },
						{ "Bright March", &g_brightMarchFont },
						{ "Love Light", &g_loveLightFont },
						{ "Metafora Stylistic", &g_metaforaSsFont },
						{ "Bollgo", &g_bollgoFont },
						{ "Dotted", &g_dottedFont },
						{ "Frantically", &g_franticallyFont },
						{ "Gimbo", &g_gimboFont },
						{ "Ginga", &g_gingaFont },
						{ "Molgeth", &g_molgethFont },
						{ "Square Lily", &g_squareLilyFont },
						{ "Nabla", &g_nablaFont },
						{ "Bungee Spice", &g_bungeeSpiceFont },
					};
					ImFont* tyFont = DemoFontPicker( "Font##TypoFills", kTypoFonts, IM_ARRAYSIZE( kTypoFonts ), &tyFontIdx );
					if ( !tyFont ) tyFont = g_monblockFont;
					static float tySize = 64.0f;
					static bool perChar = true;
					static float tessTol = 0.25f;
					static int tyIterations = 2;
					ImGui::SliderFloat( "Typography Size##TypoFills", &tySize, 16.0f, 200.0f, "%.0f lp" );
					ImGui::SliderFloat( "Tessellation##TessTol", &tessTol, 0.01f, 2.0f, "%.2f" );
					ImGui::SameLine(); ImGui::TextDisabled( "(lower = smoother)" );
					ImGui::SliderInt( "Iterations##TypoFills", &tyIterations, 0, 6 );
					ImGui::Checkbox( "Per Character##TypoPerChar", &perChar );
					static int gradPath = 0; // 0=GPU, 1=CPU
					ImGui::RadioButton( "GPU Gradient##TypoPath", &gradPath, 0 ); ImGui::SameLine();
					ImGui::RadioButton( "CPU Tessellation##TypoPath", &gradPath, 1 );

					struct TypoEntry
					{
						const char* label; int type; ImU32 c0; ImU32 c1; pfSpace2sRGB s2r; pfsRGB2Space r2s; int interp;
					};
					static const TypoEntry kTypo[] = {
						{ "Linear Gradient",  0, IM_COL32( 255,50,50,255 ), IM_COL32( 50,50,255,255 ), NULL, NULL, ImWidgetsGradientInterp_sRGB },
						{ "Radial Gradient",  1, IM_COL32( 255,255,50,255 ), IM_COL32( 50,200,50,255 ), NULL, NULL, ImWidgetsGradientInterp_sRGB },
						{ "Diamond Gradient", 2, IM_COL32( 255,100,255,255 ), IM_COL32( 100,255,255,255 ), NULL, NULL, ImWidgetsGradientInterp_sRGB },
						{ "OkLab Linear",     0, IM_COL32( 255,0,0,255 ), IM_COL32( 0,0,255,255 ), &ImWidgets::ColorConvertOKLABtoRGB, &ImWidgets::ColorConvertRGBtoOKLAB, ImWidgetsGradientInterp_OkLab },
					};

					// Helper: render gradient text either whole or per-character
					auto DrawGradText = [ & ]( const TypoEntry& te, ImVec2 basePos ){
						// GPU path: no tessellation, gradient computed in pixel shader (all color spaces)
						if ( gradPath == 0 )
						{
							if ( te.type == 0 )
								ImWidgets::DrawLinearGradientTextGPU( pDrawList, tyFont, tySize, basePos, text_buf, ImVec2( 0, 0.5f ), ImVec2( 1, 0.5f ), te.c0, te.c1, nullptr, perChar, te.interp );
							else if ( te.type == 1 )
								ImWidgets::DrawRadialGradientTextGPU( pDrawList, tyFont, tySize, basePos, text_buf, ImVec2( 0.5f, 0.5f ), ImVec2( 1, 0.5f ), te.c0, te.c1, nullptr, perChar, te.interp );
							else
								ImWidgets::DrawDiamondGradientTextGPU( pDrawList, tyFont, tySize, basePos, text_buf, ImVec2( 0.5f, 0.5f ), ImVec2( 1, 0.5f ), te.c0, te.c1, nullptr, perChar, te.interp );
							return;
						}
						// CPU path: tessellation + per-vertex gradient
						if ( !perChar )
						{
							if ( te.type == 0 )
								ImWidgets::DrawLinearGradientText( pDrawList, tyFont, tySize, basePos, text_buf, ImVec2( 0, 0.5f ), ImVec2( 1, 0.5f ), te.c0, te.c1, te.s2r, te.r2s, nullptr, tessTol, tyIterations );
							else if ( te.type == 1 )
								ImWidgets::DrawRadialGradientText( pDrawList, tyFont, tySize, basePos, text_buf, ImVec2( 0.5f, 0.5f ), ImVec2( 1, 0.5f ), te.c0, te.c1, te.s2r, te.r2s, nullptr, tessTol, tyIterations );
							else
								ImWidgets::DrawDiamondGradientText( pDrawList, tyFont, tySize, basePos, text_buf, ImVec2( 0.5f, 0.5f ), ImVec2( 1, 0.5f ), te.c0, te.c1, te.s2r, te.r2s, nullptr, tessTol, tyIterations );
						}
						else
						{
							// Per-glyph: shape full text (preserves ligatures/calt), then apply gradient per glyph
							ImVector<ImWidgetsShape> glyphShapes;
							ImWidgets::TesselateTextPerGlyph( tyFont, tySize, text_buf, glyphShapes, nullptr, tessTol, tyIterations );
							pfSpace2sRGB s2r = te.s2r ? te.s2r : &ImWidgets::ColorConvertsRGBtosRGB;
							pfsRGB2Space r2s = te.r2s ? te.r2s : &ImWidgets::ColorConvertsRGBtosRGB;
							for ( int gs = 0; gs < glyphShapes.Size; gs++ )
							{
								ImWidgetsShape& shape = glyphShapes[gs];
								if ( shape.triangles.Size == 0 ) continue;
								// Offset to screen position
								for ( int vi = 0; vi < shape.vertices.Size; vi++ )
								{
									shape.vertices[vi].pos.x += basePos.x;
									shape.vertices[vi].pos.y += basePos.y;
								}
								shape.bb.Translate( basePos );
								// Apply gradient per glyph's own BBox
								if ( te.type == 0 )
									ImWidgets::ShapeLinearGradientGeneric( shape, ImVec2( 0, 0.5f ), ImVec2( 1, 0.5f ), te.c0, te.c1, s2r, r2s );
								else if ( te.type == 1 )
									ImWidgets::ShapeRadialGradientGeneric( shape, ImVec2( 0.5f, 0.5f ), ImVec2( 1, 0.5f ), te.c0, te.c1, s2r, r2s );
								else
									ImWidgets::ShapeDiamondGradientGeneric( shape, ImVec2( 0.5f, 0.5f ), ImVec2( 1, 0.5f ), te.c0, te.c1, s2r, r2s );
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

					// Image fill text -- cycle through all loaded images for per-character
					{
						// Collect all available images
						ImTextureID allImages[8]; int nImages = 0;
						if ( illlustration_img ) allImages[nImages++] = illlustration_img;
						if ( bike_img )          allImages[nImages++] = bike_img;
						if ( astro_img )         allImages[nImages++] = astro_img;
						if ( clock_img )         allImages[nImages++] = clock_img;
						if ( man_img )           allImages[nImages++] = man_img;

						if ( nImages > 0 )
						{
							ImGui::TextDisabled( "Image Fill" );
							ImVec2 p = ImGui::GetCursorScreenPos();
							float asc2 = 0;
							ImVec2 tsz = ImWidgets::CalcTextSize( tyFont, tySize, text_buf, nullptr, &asc2 );
							if ( gradPath == 0 && perChar )
							{
								// GPU per-char image fill: cycle through images per glyph
								ImWidgets::DrawImageTextGPU( pDrawList, tyFont, tySize, ImVec2( p.x, p.y + asc2 ), text_buf, allImages, nImages, IM_COL32_WHITE, ImVec2( 0, 0 ), ImVec2( 1, 1 ) );
							}
							else if ( gradPath == 0 )
							{
								// GPU whole-text image fill: single image across entire text
								ImWidgets::DrawImageTextGPU( pDrawList, tyFont, tySize, ImVec2( p.x, p.y + asc2 ), text_buf, allImages[0], IM_COL32_WHITE, ImVec2( 0, 0 ), ImVec2( 1, 1 ), nullptr, false );
							}
							else if ( !perChar )
							{
								ImWidgets::DrawImageText( pDrawList, tyFont, tySize, ImVec2( p.x, p.y + asc2 ), allImages[0], text_buf, nullptr, IM_COL32_WHITE, ImVec2( 0, 0 ), ImVec2( 1, 1 ), tessTol, tyIterations );
							}
							else
							{
								// Per-glyph image fill: shape full text, one different image per glyph
								ImVector<ImWidgetsShape> glyphShapes;
								ImWidgets::TesselateTextPerGlyph( tyFont, tySize, text_buf, glyphShapes, nullptr, tessTol, tyIterations );
								ImVec2 imgPos( p.x, p.y + asc2 );
								for ( int gs = 0; gs < glyphShapes.Size; gs++ )
								{
									ImWidgetsShape& shape = glyphShapes[gs];
									if ( shape.triangles.Size == 0 ) continue;
									for ( int vi = 0; vi < shape.vertices.Size; vi++ )
									{
										shape.vertices[vi].pos.x += imgPos.x;
										shape.vertices[vi].pos.y += imgPos.y;
									}
									shape.bb.Translate( imgPos );
									float bbW = ImMax( shape.bb.GetWidth(), 1.0f ), bbH = ImMax( shape.bb.GetHeight(), 1.0f );
									for ( int vi = 0; vi < shape.vertices.Size; vi++ )
									{
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
						ImGui::SliderFloat( "LaTeX Size##LatexFill", &latexFillSize, 16.0f, 80.0f, "%.0f lp" );

						struct LaTeXFillEntry
						{
							const char* eq; ImU32 c0; ImU32 c1;
						};
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

						for ( int ei = 0; ei < IM_ARRAYSIZE( kLatexFills ); ei++ )
						{
							const LaTeXFillEntry& lfe = kLatexFills[ei];
							ImVec2 sz = ImWidgets::CalcLaTeXSize( latexFillSize, lfe.eq );
							if ( sz.x < 1.0f || sz.y < 1.0f ) continue;
							ImVec2 p = ImGui::GetCursorScreenPos();

							ImWidgetsShape latexShape;
							ImWidgets::TesselateLaTeX( latexFillSize, lfe.eq, p, latexShape, tessTol, tyIterations );

							if ( latexShape.triangles.Size > 0 )
							{
								ImWidgets::ShapeLinearGradientGeneric( latexShape,
																	   ImVec2( 0, 0.5f ), ImVec2( 1, 0.5f ), lfe.c0, lfe.c1,
																	   &ImWidgets::ColorConvertsRGBtosRGB, &ImWidgets::ColorConvertsRGBtosRGB );
								ImWidgets::DrawShape( pDrawList, latexShape );
							}
							ImGui::Dummy( ImVec2( sz.x, sz.y + gap ) );
						}
					}
					EndCullSection( s_cull_typofills_h, s_cull_typofills_y );
				}
			}
			DW_SsRecord( "Typography_Fills", _sy0, ImGui::GetCursorPos().y );
		}
		EndCullSection( s_cull_h, s_cull_y );
	}

	void ShowTypographyAnimations()
	{
		// NOTE: ApplyOpenAll() sets the open state of the *next* widget, so it
		// must be called immediately before the CollapsingHeader below. The
		// previous placement here was consumed by the intervening font-picker
		// / ImGui controls and never reached the header.
		// Use the curated demo-font list so the user can pick any live atlas
		// font. If none of the curated slots are loaded (fresh clone, no
		// downloads yet), fall back to ImGui::GetFont() so the section still
		// renders the placeholder strings rather than being silent.
		static const DemoFontChoice kAnimFonts[] = {
			{ "Cinzel", &g_cinzelFont },
			{ "Alfa Slab", &g_alfaSlabFont },
			{ "Trench Slab", &g_trenchSlabFont },
			{ "Classical Aesthetics", &g_classicalFont },
			{ "Foglighten", &g_foglihtenFont },
			{ "Steelworks", &g_steelworksFont },
			{ "Monblock", &g_monblockFont },
			{ "Bright March", &g_brightMarchFont },
			{ "Love Light", &g_loveLightFont },
			{ "Metafora Stylistic", &g_metaforaSsFont },
			{ "Bollgo", &g_bollgoFont },
			{ "Dotted", &g_dottedFont },
			{ "Frantically", &g_franticallyFont },
			{ "Gimbo", &g_gimboFont },
			{ "Ginga", &g_gingaFont },
			{ "Molgeth", &g_molgethFont },
			{ "Square Lily", &g_squareLilyFont },
			{ "Nabla", &g_nablaFont },
			{ "Bungee Spice", &g_bungeeSpiceFont },
		};
		static int animFontIdx = 0;
		ImFont* animFont = DemoFontPicker( "Font##TypoAnim", kAnimFonts, IM_ARRAYSIZE( kAnimFonts ), &animFontIdx );
		if ( !animFont ) animFont = ImGui::GetFont();
		if ( !animFont )
		{
			ImGui::TextDisabled( "Typography Animations needs a display font; none loaded." );
			return;
		}
		static float animSize = 80.0f;
		static float prevAnimSize = 0;
		static ImFont* prevAnimFont = nullptr;

		// --- Cache: tessellate once, reuse every frame ---
		// Built every frame regardless of header state so opening the header is stall-free.
		struct AnimCache
		{
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
		bool needRebuild = (animSize != prevAnimSize) || (animFont != prevAnimFont);
		if ( needRebuild )
		{
			prevAnimSize = animSize;
			prevAnimFont = animFont;
			for ( int i = 0; i < 6; i++ ) cache[i].valid = false;
		}

		// Build one pending cache entry per frame to amortise startup cost
		float tessTol = 0.25f;
		int iterations = 2;
		for ( int ci = 0; ci < 6; ci++ )
		{
			if ( cache[ci].valid ) continue;
			cache[ci].textSize = ImWidgets::CalcTextSize( animFont, animSize, kTexts[ci], nullptr, &cache[ci].ascent );
			cache[ci].glyphs.resize( 0 );
			ImWidgets::TesselateTextPerGlyph( animFont, animSize, kTexts[ci], cache[ci].glyphs, nullptr, tessTol, iterations );
			// Also build whole-text shape for reveal/pulse
			if ( ci == 0 || ci == 4 )
			{
				cache[ci].whole.vertices.resize( 0 ); cache[ci].whole.triangles.resize( 0 );
				cache[ci].whole.bb = ImRect( FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX );
				ImWidgets::TesselateText( animFont, animSize, kTexts[ci], cache[ci].whole, nullptr, tessTol, iterations );
			}
			cache[ci].valid = true;
			break; // one entry per frame -- spreads cost across 6 frames at startup
		}

		ApplyOpenAll();
		if ( !ImGui::CollapsingHeader( "Typography Animations" ) )
			return;
		static float s_cull_h = 0; float s_cull_y;
		if ( !BeginCullSection( s_cull_h, s_cull_y ) ) return;

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		float t = (float)ImGui::GetTime();

		ImGui::SliderFloat( "Size##TypoAnim", &animSize, 32.0f, 200.0f, "%.0f lp" );
		float gap = 8.0f;

		// Diagnose: every animation depends on the Slug per-glyph tessellation
		// cache. When the chosen font has no Slug data registered (e.g., the
		// font was loaded without slugCfg, or ImGui's atlas dropped the font
		// data after rebuild), all caches build empty and the section appears
		// blank. Surface that explicitly so the user knows why.
		bool any_cache_built = false;
		for ( int ci = 0; ci < 6; ++ci )
			if ( cache[ci].valid && (cache[ci].glyphs.Size > 0 || cache[ci].whole.triangles.Size > 0) )
			{
				any_cache_built = true; break;
			}
		if ( !any_cache_built )
		{
			ImGui::TextColored( ImVec4( 1, 0.6f, 0.4f, 1 ),
								"Tessellation cache empty -- Slug font data missing for '%s'. Try a different font (e.g., Cinzel, Alfa Slab).",
								animFont == ImGui::GetFont() ? "<default>" : "selected" );
			// Still render plain text below as a fallback so the user sees the strings.
			for ( int i = 0; i < 6; ++i ) ImGui::Text( "%s", kTexts[i] );
			return;
		}

		// Helper: draw a cached glyph shape at a screen position with a color
		auto DrawGlyph = [ & ]( ImWidgetsShape& src, ImVec2 offset, ImU32 col ){
			if ( src.triangles.Size == 0 ) return;
			// Copy vertices, apply offset + color
			int baseVtx = pDrawList->VtxBuffer.Size;
			int baseIdx = pDrawList->IdxBuffer.Size;
			pDrawList->PrimReserve( src.triangles.Size * 3, src.vertices.Size );
			ImDrawVert* vtx = pDrawList->VtxBuffer.Data + baseVtx;
			ImDrawIdx* idx = pDrawList->IdxBuffer.Data + baseIdx;
			ImVec2 wuv = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
			for ( int vi = 0; vi < src.vertices.Size; vi++ )
			{
				vtx[vi].pos = ImVec2( src.vertices[vi].pos.x + offset.x, src.vertices[vi].pos.y + offset.y );
				vtx[vi].uv = wuv;
				vtx[vi].col = col;
			}
			for ( int ti = 0; ti < src.triangles.Size; ti++ )
			{
				idx[ti * 3 + 0] = (ImDrawIdx)(baseVtx + src.triangles[ti].a);
				idx[ti * 3 + 1] = (ImDrawIdx)(baseVtx + src.triangles[ti].b);
				idx[ti * 3 + 2] = (ImDrawIdx)(baseVtx + src.triangles[ti].c);
			}
			pDrawList->_VtxWritePtr += src.vertices.Size;
			pDrawList->_IdxWritePtr += src.triangles.Size * 3;
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
			memcpy( tmp.triangles.Data, c.whole.triangles.Data, c.whole.triangles.Size * sizeof( ImWidgetsTriIdx ) );
			tmp.bb = c.whole.bb;
			for ( int vi = 0; vi < c.whole.vertices.Size; vi++ )
			{
				tmp.vertices[vi].pos = ImVec2( c.whole.vertices[vi].pos.x + basePos.x, c.whole.vertices[vi].pos.y + basePos.y );
				tmp.vertices[vi].uv = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
			}
			tmp.bb.Translate( basePos );
			float bandW = 0.08f;
			ImWidgets::ShapeLinearGradientGeneric( tmp, ImVec2( sweep - bandW, 0.5f ), ImVec2( sweep, 0.5f ),
												   IM_COL32( 255, 200, 50, 0 ), IM_COL32( 255, 200, 50, 255 ),
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
			for ( int gs = 0; gs < c.glyphs.Size; gs++ )
			{
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
			for ( int gs = 0; gs < c.glyphs.Size; gs++ )
			{
				float phase = sinf( t * 3.0f + (float)gs * 0.8f );
				int alpha = (int)(ImSaturate( phase * 0.5f + 0.5f ) * 255.0f);
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
			for ( int gs = 0; gs < c.glyphs.Size; gs++ )
			{
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
			memcpy( tmp.triangles.Data, c.whole.triangles.Data, c.whole.triangles.Size * sizeof( ImWidgetsTriIdx ) );
			tmp.bb = c.whole.bb;
			for ( int vi = 0; vi < c.whole.vertices.Size; vi++ )
			{
				tmp.vertices[vi].pos = ImVec2( c.whole.vertices[vi].pos.x + basePos.x, c.whole.vertices[vi].pos.y + basePos.y );
				tmp.vertices[vi].uv = ImGui::GetDrawListSharedData()->TexUvWhitePixel;
			}
			tmp.bb.Translate( basePos );
			ImWidgets::ShapeRadialGradientGeneric( tmp, ImVec2( 0.5f, 0.5f ), ImVec2( pulse, 0.5f ),
												   IM_COL32( 255, 50, 255, 255 ), IM_COL32( 50, 50, 255, 60 ),
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
			for ( int gs = 0; gs < visibleCount; gs++ )
			{
				bool isCursor = (gs == visibleCount - 1);
				DrawGlyph( c.glyphs[gs], basePos,
						   isCursor ? IM_COL32( 255, 255, 255, 255 ) : IM_COL32( 200, 220, 200, 255 ) );
			}
			ImGui::Dummy( ImVec2( c.textSize.x, c.textSize.y + gap ) );
		}
		EndCullSection( s_cull_h, s_cull_y );
	}

	void ShowLaTeXDemo()
	{
		ApplyOpenAll();
		if ( !ImGui::CollapsingHeader( "LaTeX Math" ) )
			return;
		static float s_cull_h = 0; float s_cull_y;
		if ( !BeginCullSection( s_cull_h, s_cull_y ) ) return;

		// User-editable expression
		static char latex_buf[1024] = "L_o(x, \\omega_o) = L_e(x, \\omega_o) + \\int_{\\Omega} f_r(x, \\omega_i, \\omega_o) L_i(x, \\omega_i) \\langle \\omega_i \\cdot n \\rangle_+ d\\omega_i";
		static float latex_size = 24.0f;
		static ImVec4 latex_col_v( 1.0f, 1.0f, 1.0f, 1.0f );
		static ImU32  latex_col_u = IM_COL32( 255, 255, 255, 255 );
		static bool latex_show_bbox = false;

		ImGui::InputTextMultiline( "##LatexInput", latex_buf, sizeof( latex_buf ), ImVec2( -1, ImGui::GetTextLineHeight() * 3 ) );
		ImGui::DragFloat( "Size##LatexSize", &latex_size, 0.5f, 8.0f, 200.0f, "%.0f lp" );
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
		struct LaTeXEntry
		{
			const char* latex; const char* group;
		};
		static const char* kGrpClassic = "Classic Equations";
		static const char* kGrpFrac = "Fractions & Roots";
		static const char* kGrpMatrix = "Matrices & Vectors";
		static const char* kGrpDecor = "Decorations";
		static const char* kGrpEnv = "Environments";
		static const char* kGrpSymbols = "Symbols & Accents";
		static const char* kGrpCalc = "Calculus & Integrals";
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
		ImVec4 latexClipRect = pDrawList->_CmdHeader.ClipRect;
		static float s_cached_latex_h[64]; // per-example height cache
		const char* currentGroup = NULL;
		bool groupOpen = false;
		float groupY0 = 0.0f;
		int nExamples = IM_ARRAYSIZE( kExamples );
		for ( int i = 0; i <= nExamples; i++ )
		{
			const char* nextGroup = (i < nExamples) ? kExamples[i].group : NULL;
			if ( nextGroup != currentGroup )
			{
				if ( currentGroup != NULL )
					DW_SsRecord( currentGroup, groupY0, ImGui::GetCursorPos().y );
				currentGroup = nextGroup;
				if ( nextGroup != NULL )
				{
					groupY0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					groupOpen = ImGui::CollapsingHeader( nextGroup );
				}
			}
			if ( i >= nExamples ) break;
			if ( !groupOpen ) continue;

			// Per-entry scroll culling
			float entryY = ImGui::GetCursorScreenPos().y;
			float est_h = (s_cached_latex_h[i] > 0) ? s_cached_latex_h[i] : (latex_size * 3.0f + pad * 2 + gap);
			if ( entryY + est_h < latexClipRect.y || entryY > latexClipRect.w )
			{
				ImGui::Dummy( ImVec2( 0, est_h ) );
				continue;
			}

			const LaTeXEntry& e = kExamples[i];

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
			s_cached_latex_h[i] = ImGui::GetCursorScreenPos().y - entryY;
		}
		EndCullSection( s_cull_h, s_cull_y );
	}

	void ShowCustomShaderDemo()
	{
		float const size = CanvasSize();

		static float shape_size = 1.0f;
		static float line_width = 0.05f;
		static float angle = 180.0f * IM_PI / 180.0f;
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

		static int marker_idx = (int)ImWidgetsMarker_Pin;
		static const char* markers[] = {
			"Disc", "Square", "Triangle", "Diamond", "Heart",
			"Spade", "Club", "Chevron", "Clover", "Ring",
			"Tag", "Cross", "Asterisk", "Infinity", "Pin",
			"Arrow", "Ellipse", "EllipseApprox"
		};
		ImGui::Combo( "Marker", &marker_idx, markers, ImWidgetsMarker_COUNT );

		static int draw_type_idx = (int)ImWidgetsDrawType_Outline;
		static const char* draw_types[] = {
			"Filled", "Stroke", "Outline", "Signed Distance Field", "Cut Off"
		};
		ImGui::Combo( "Draw Type", &draw_type_idx, draw_types, ImWidgetsDrawType_COUNT );

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImWidgets::DrawMarker( pDrawList, pos, ImVec2( size, size ),
							   fg_color.u, bg_color.u, angle, shape_size, line_width, antialiasing,
							   (ImWidgetsMarker)marker_idx, (ImWidgetsDrawType)draw_type_idx );
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
		ImGui::SliderInt( "Tess##DrawSquircle", &tess, 0, 16 );
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

	// -------------------------------------------------------------------------
	// Text Showcase -- ImGui::Text / TextColored / TextWrapped equivalents
	// using Slug DrawText, supporting LTR and RTL layouts.
	// -------------------------------------------------------------------------

	// Returns the total rendered height of SlugTextWrapped (same algorithm, no draw).
	static float SlugCalcWrappedHeight( ImFont* font, float font_size,
										const char* text, float wrap_width )
	{
		float space_w = ImWidgets::CalcTextSize( font, font_size, " " ).x;
		float line_h = ImWidgets::CalcTextSize( font, font_size, "Ay" ).y;
		const char* p = text;
		float       line_w = 0.0f;
		float       total = 0.0f;
		while ( *p )
		{
			if ( *p == '\n' )
			{
				total += line_h; line_w = 0.0f; p++; continue;
			}
			const char* ws = p;
			while ( *p && *p != ' ' && *p != '\n' ) p++;
			float word_w = ImWidgets::CalcTextSize( font, font_size, ws, p ).x;
			float gap = (line_w > 0.0f) ? space_w : 0.0f;
			if ( line_w > 0.0f && line_w + gap + word_w > wrap_width )
			{
				total += line_h; line_w = word_w;
			}
			else
			{
				line_w += gap + word_w;
			}
			if ( *p == ' ' ) p++;
		}
		if ( line_w > 0.0f || total == 0.0f ) total += line_h;
		return total;
	}

	// Equivalent to ImGui::Text(): renders one unstyled line and advances cursor.
	static void SlugText( ImFont* font, float font_size, ImU32 col,
						  const char* text, const char* text_end = nullptr )
	{
		float  asc = 0.0f;
		ImVec2 sz = ImWidgets::CalcTextSize( font, font_size, text, text_end, &asc );
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImWidgets::DrawText( ImGui::GetWindowDrawList(), font, font_size,
							 ImVec2( pos.x, pos.y + asc ), col, text, text_end );
		ImGui::Dummy( ImVec2( sz.x, sz.y ) );
	}

	// Equivalent to ImGui::TextColored(): same as SlugText with an explicit RGBA color.
	static void SlugTextColored( ImFont* font, float font_size, ImVec4 col_v,
								 const char* text, const char* text_end = nullptr )
	{
		SlugText( font, font_size, ImGui::ColorConvertFloat4ToU32( col_v ), text, text_end );
	}

	// Equivalent to ImGui::TextWrapped(): word-wraps within wrap_width.
	// right_align = true renders each line flush-right (use for RTL / Arabic).
	static void SlugTextWrapped( ImFont* font, float font_size, ImU32 col,
								 const char* text, float wrap_width = 0.0f,
								 bool right_align = false )
	{
		if ( wrap_width <= 0.0f )
			wrap_width = ImGui::GetContentRegionAvail().x;

		// Measure line metrics once from a representative glyph
		float asc = 0.0f;
		ImVec2 ref = ImWidgets::CalcTextSize( font, font_size, "Ay", nullptr, &asc );
		float line_h = ref.y;

		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2         origin = ImGui::GetCursorScreenPos();
		float          pen_y = origin.y;
		float          total_h = 0.0f;

		// Clip rect for early line/word culling
		ImVec4 cr = dl->_CmdHeader.ClipRect;

		// Measure a single space width for inter-word gap
		float space_w = ImWidgets::CalcTextSize( font, font_size, " " ).x;

		// Walk token by token (words separated by spaces / explicit newlines)
		const char* p = text;
		const char* line_start = p;   // first char of current assembled line
		const char* line_end = p;   // one-past-last char of last word that fit
		float       line_w = 0.0f;
		bool        past_bottom = false; // true once pen_y is past clip rect bottom

		auto flush_line = [ & ]( const char* end, float w ){
			if ( line_start >= end ) return;
			// Skip all work for lines fully above/below visible area
			float lineTop = pen_y;
			float lineBot = pen_y + line_h;
			if ( lineBot < cr.y || lineTop > cr.w )
			{
				pen_y += line_h;
				total_h += line_h;
				if ( lineTop > cr.w ) past_bottom = true;
				return;
			}
			// Strip trailing space/newline so it doesn't skew right-alignment offset.
			const char* draw_end = end;
			while ( draw_end > line_start && (*(draw_end - 1) == ' ' || *(draw_end - 1) == '\n') )
				--draw_end;
			if ( right_align )
				w = ImWidgets::CalcShapedTextWidth( font, font_size, line_start, draw_end );
			float rx = right_align ? (origin.x + wrap_width - w) : origin.x;
			ImWidgets::DrawText( dl, font, font_size,
								 ImVec2( rx, pen_y + asc ), col, line_start, draw_end );
			pen_y += line_h;
			total_h += line_h;
			};

		while ( *p && !past_bottom )
		{
			if ( *p == '\n' )
			{
				flush_line( line_end, line_w );
				p++;
				line_start = line_end = p;
				line_w = 0.0f;
				continue;
			}

			// Scan to end of word
			const char* word_s = p;
			while ( *p && *p != ' ' && *p != '\n' ) ++p;
			const char* word_e = p;

			float word_w = ImWidgets::CalcTextSize( font, font_size, word_s, word_e ).x;
			float gap = (line_w > 0.0f) ? space_w : 0.0f;

			if ( line_w > 0.0f && line_w + gap + word_w > wrap_width )
			{
				// Overflow -> flush current line, start fresh with current word
				flush_line( line_end, line_w );
				line_start = word_s;
				line_end = word_e;
				line_w = word_w;
			}
			else
			{
				line_w += gap + word_w;
				line_end = word_e;
			}

			if ( *p == ' ' ) ++p;  // consume separator
		}

		// Flush last line
		if ( line_start < line_end && !past_bottom )
			flush_line( line_end, line_w );

		// If we early-exited, still account for remaining height
		if ( past_bottom )
		{
			// Count remaining newlines to estimate total height
			while ( *p )
			{
				if ( *p == '\n' ) total_h += line_h; ++p;
			}
			total_h += line_h; // last line
		}

		if ( total_h < line_h ) total_h = line_h;
		ImGui::Dummy( ImVec2( wrap_width, total_h ) );
	}

	void ShowTextShowcase()
	{
		ApplyOpenAll();
		if ( !ImGui::CollapsingHeader( "Text Showcase" ) )
			return;

#if !IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
		ImGui::TextDisabled( "Slug requires custom shader support." );
		return;
#else
		// -- Controls -----------------------------------------------------------
		static float  font_size = 18.0f;
		static ImVec4 col_v( 0.93f, 0.90f, 0.85f, 1.0f );
		static ImU32  col_u = ImGui::ColorConvertFloat4ToU32( col_v );
		static ImVec4 hi_col_v( 0.45f, 0.82f, 1.00f, 1.0f );
		static ImU32  hi_col_u = ImGui::ColorConvertFloat4ToU32( hi_col_v );
		static ImVec4 bg_col_v( 0.08f, 0.08f, 0.14f, 0.88f );

		ImGui::DragFloat( "Font Size##TxtShow", &font_size, 0.5f, 8.0f, 72.0f, "%.0f lp" );
		if ( ImGui::ColorEdit4( "Text Color##TxtShow", &col_v.x ) )
			col_u = ImGui::ColorConvertFloat4ToU32( col_v );
		if ( ImGui::ColorEdit4( "Highlight Color##TxtShow", &hi_col_v.x ) )
			hi_col_u = ImGui::ColorConvertFloat4ToU32( hi_col_v );
		ImGui::ColorEdit4( "Box Background##TxtShow", &bg_col_v.x );

		ImFont* latin_font = g_cinzelFont ? g_cinzelFont : g_firaCodeFont;
		ImFont* arabic_font = g_amiriFont;

		static const char* k_latin_lorem =
			"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
			"incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
			"exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure "
			"dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
			"Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
			"mollit anim id est laborum. Sed ut perspiciatis unde omnis iste natus error sit "
			"voluptatem accusantium doloremque laudantium, totam rem aperiam eaque ipsa quae ab "
			"illo inventore veritatis et quasi architecto beatae vitae dicta sunt explicabo.";

		// Arabic Lorem Ipsum (standard placeholder text used in Arabic typesetting)
		static const char* k_arabic_lorem =
			"\xd9\x84\xd9\x88\xd8\xb1\xd9\x8a\xd9\x85 \xd8\xa5\xd9\x8a\xd8\xa8\xd8\xb3\xd9\x88\xd9\x85 "
			"\xd9\x87\xd9\x88 \xd9\x86\xd9\x85\xd9\x88\xd8\xb0\xd8\xac \xd9\x8a\xd9\x8f\xd8\xb3\xd8\xaa\xd8\xae\xd8\xaf\xd9\x85 "
			"\xd9\x81\xd9\x8a \xd8\xb5\xd9\x86\xd8\xa7\xd8\xb9\xd8\xa9 \xd8\xa7\xd9\x84\xd8\xb7\xd8\xa8\xd8\xa7\xd8\xb9\xd8\xa9 "
			"\xd9\x88\xd8\xa7\xd9\x84\xd8\xaa\xd9\x86\xd8\xb6\xd9\x8a\xd8\xaf.\n"
			"\xd9\x83\xd8\xa7\xd9\x86 \xd9\x84\xd9\x88\xd8\xb1\xd9\x8a\xd9\x85 \xd8\xa5\xd9\x8a\xd8\xa8\xd8\xb3\xd9\x88\xd9\x85 "
			"\xd9\x87\xd9\x88 \xd8\xa7\xd9\x84\xd9\x86\xd9\x85\xd9\x88\xd8\xb0\xd8\xac \xd8\xa7\xd9\x84\xd9\x85\xd8\xb9\xd9\x8a\xd8\xa7\xd8\xb1\xd9\x8a "
			"\xd9\x85\xd9\x86\xd8\xb0 \xd8\xa7\xd9\x84\xd9\x82\xd8\xb1\xd9\x86 \xd8\xa7\xd9\x84\xd8\xae\xd8\xa7\xd9\x85\xd8\xb3 \xd8\xb9\xd8\xb4\xd8\xb1\xd8\x8c "
			"\xd8\xb9\xd9\x86\xd8\xaf\xd9\x85\xd8\xa7 \xd9\x82\xd8\xa7\xd9\x85\xd8\xaa \xd9\x85\xd8\xb7\xd8\xa8\xd8\xb9\xd8\xa9 \xd9\x85\xd8\xac\xd9\x87\xd9\x88\xd9\x84\xd8\xa9 "
			"\xd8\xa8\xd8\xb1\xd8\xb5 \xd9\x85\xd8\xac\xd9\x85\xd9\x88\xd8\xb9\xd8\xa9 \xd9\x85\xd9\x86 \xd8\xa7\xd9\x84\xd8\xa3\xd8\xad\xd8\xb1\xd9\x81 "
			"\xd8\xa8\xd8\xb4\xd9\x83\xd9\x84 \xd8\xb9\xd8\xb4\xd9\x88\xd8\xa7\xd8\xa6\xd9\x8a.\n"
			"\xd9\x84\xd9\x85 \xd9\x8a\xd8\xaa\xd9\x83\xd9\x86 \xd8\xae\xd9\x85\xd8\xb3\xd8\xa9 \xd9\x82\xd8\xb1\xd9\x88\xd9\x86 \xd9\x81\xd8\xad\xd8\xb3\xd8\xa8\xd8\x8c "
			"\xd8\xa8\xd9\x84 \xd8\xa7\xd9\x86\xd8\xaa\xd9\x82\xd9\x84 \xd8\xa5\xd9\x84\xd9\xb0 \xd8\xa7\xd9\x84\xd8\xaa\xd9\x86\xd8\xb6\xd9\x8a\xd8\xaf "
			"\xd8\xa7\xd9\x84\xd8\xa5\xd9\x84\xd9\x83\xd8\xaa\xd8\xb1\xd9\x88\xd9\x86\xd9\x8a\xd8\x8c "
			"\xd9\x88\xd9\x84\xd8\xa7 \xd9\x8a\xd8\xb2\xd8\xa7\xd9\x84 \xd8\xad\xd9\x8a\xd9\x91\xd8\xa7 \xd8\xad\xd8\xaa\xd9\xb0\xd9\x89 \xd8\xa7\xd9\x84\xd8\xb3\xd8\xa7\xd8\xb9\xd8\xa9.\n"
			"\xd8\xa7\xd9\x86\xd8\xaa\xd8\xb4\xd8\xb1 \xd9\x87\xd8\xb0\xd8\xa7 \xd8\xa7\xd9\x84\xd9\x86\xd9\x85\xd9\x88\xd8\xb0\xd8\xac \xd9\x81\xd9\x8a \xd8\xa7\xd9\x84\xd8\xb3\xd8\xaa\xd9\x8a\xd9\x86\xd8\xa7\xd8\xaa "
			"\xd9\x85\xd8\xb9 \xd8\xa5\xd8\xb5\xd8\xaf\xd8\xa7\xd8\xb1 \xd8\xb1\xd9\x82\xd8\xa7\xd8\xa6\xd9\x82 \xd9\x84\xd9\x8a\xd8\xaa\xd8\xb1\xd8\xa7\xd8\xb3\xd9\x8a\xd8\xaa "
			"\xd8\xa7\xd9\x84\xd8\xa8\xd9\x84\xd8\xa7\xd8\xb3\xd8\xaa\xd9\x8a\xd9\x83\xd9\x8a\xd8\xa9 \xd8\xaa\xd8\xad\xd9\x88\xd9\x8a \xd9\x85\xd9\x82\xd8\xa7\xd8\xb7\xd8\xb9 \xd9\x85\xd9\x86 \xd9\x87\xd8\xb0\xd8\xa7 \xd8\xa7\xd9\x84\xd9\x86\xd8\xb5.";

		float box_w = ImGui::GetContentRegionAvail().x;
		float box_pad = 10.0f;
		float gap = ImGui::GetStyle().ItemSpacing.y;
		ImDrawList* dl = ImGui::GetWindowDrawList();

		// -- Arabic block -------------------------------------------------------
		if ( arabic_font )
		{
			ImGui::SeparatorText( "Arabic  (right-aligned)" );

			static const char* k_ar_line1 = "\xd9\x86\xd8\xb5: \xd8\xa7\xd9\x84\xd8\xa3\xd8\xaf\xd9\x88\xd8\xa7\xd8\xaa \xd8\xa7\xd9\x84\xd8\xb9\xd8\xb2\xd9\x8a\xd8\xb2\xd8\xa9 \xe2\x80\x94 \xd8\xb9\xd8\xb1\xd8\xb6 \xd8\xa7\xd9\x84\xd9\x86\xd8\xb5\xd9\x88\xd8\xb5";
			static const char* k_ar_line2 = "\xd8\xa7\xd9\x84\xd8\xa3\xd8\xaf\xd9\x88\xd8\xa7\xd8\xaa \xd8\xa7\xd9\x84\xd8\xb9\xd8\xb2\xd9\x8a\xd8\xb2\xd8\xa9 \xe2\x80\x94 TextColored()";
			float h_ar1 = ImWidgets::CalcTextSize( arabic_font, font_size, k_ar_line1 ).y;
			float h_ar2 = ImWidgets::CalcTextSize( arabic_font, font_size, k_ar_line2 ).y;
			float wrap_w_ar = box_w - box_pad * 2.0f;
			float wrap_h = SlugCalcWrappedHeight( arabic_font, font_size, k_arabic_lorem, wrap_w_ar );
			float box_h = box_pad + h_ar1 + h_ar2 + wrap_h + box_pad + gap * 3.0f;

			ImVec2 box_pos = ImGui::GetCursorScreenPos();
			dl->AddRectFilled( box_pos, ImVec2( box_pos.x + box_w, box_pos.y + box_h ),
							   ImGui::ColorConvertFloat4ToU32( bg_col_v ), 6.0f );

			ImGui::Dummy( ImVec2( box_w, box_pad ) );     // top padding

			auto t0 = std::chrono::high_resolution_clock::now();

			// -- SlugText (RTL, right-aligned) --
			{
				float asc = 0.0f;
				const char* line = k_ar_line1;
				ImVec2 sz = ImWidgets::CalcTextSize( arabic_font, font_size, line, nullptr, &asc );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImWidgets::DrawText( dl, arabic_font, font_size,
									 ImVec2( pos.x + box_w - box_pad - sz.x, pos.y + asc ), hi_col_u, line );
				ImGui::Dummy( ImVec2( box_w, sz.y ) );
			}

			//-- SlugTextColored (RTL, right-aligned) --
			{
				float asc = 0.0f;
				const char* line = k_ar_line2;
				ImVec2 sz = ImWidgets::CalcTextSize( arabic_font, font_size, line, nullptr, &asc );
				ImVec2 pos = ImGui::GetCursorScreenPos();
				ImWidgets::DrawText( dl, arabic_font, font_size,
									 ImVec2( pos.x + box_w - box_pad - sz.x, pos.y + asc ),
									 ImGui::ColorConvertFloat4ToU32( col_v ), line );
				ImGui::Dummy( ImVec2( box_w, sz.y ) );
			}

			// -- SlugTextWrapped (RTL) -- full box width, right-aligned --
			ImGui::SetCursorPosX( ImGui::GetCursorPosX() + box_pad );
			SlugTextWrapped( arabic_font, font_size, col_u, k_arabic_lorem, wrap_w_ar, /*right_align=*/true );

			auto t1 = std::chrono::high_resolution_clock::now();
			double dt_ms = std::chrono::duration<double, std::milli>( t1 - t0 ).count();

			static float s_arabic_ring32[32] = {};
			static float s_arabic_ring128[128] = {};
			static int   s_arabic_head = 0;
			s_arabic_ring32[s_arabic_head % 32] = (float)dt_ms;
			s_arabic_ring128[s_arabic_head % 128] = (float)dt_ms;
			s_arabic_head++;
			float s_arabic_avg32 = 0.0f, s_arabic_avg128 = 0.0f;
			for ( int i = 0; i < 32; i++ ) s_arabic_avg32 += s_arabic_ring32[i];
			for ( int i = 0; i < 128; i++ ) s_arabic_avg128 += s_arabic_ring128[i];
			s_arabic_avg32 /= 32.0f;
			s_arabic_avg128 /= 128.0f;

			ImGui::Dummy( ImVec2( box_w, box_pad ) );     // bottom padding
			ImGui::TextDisabled( "DrawText time: %.3f ms  (avg32: %.3f ms  avg128: %.3f ms)", dt_ms, s_arabic_avg32, s_arabic_avg128 );
		}

		// -- Latin block --------------------------------------------------------
		if ( latin_font )
		{
			ImGui::SeparatorText( "Latin  (left-aligned)" );

			// Pre-compute box height so we can draw the background rect first,
			// then render text on top (draw-list order = render order).
			float wrap_w = box_w - box_pad * 2.0f;
			float h_line1 = ImWidgets::CalcTextSize( latin_font, font_size, "Text(): The quick brown fox jumps over the lazy dog." ).y;
			float h_line2 = ImWidgets::CalcTextSize( latin_font, font_size, "TextColored(): Dear Widgets -- GPU Text Showcase" ).y;
			float wrap_h = SlugCalcWrappedHeight( latin_font, font_size, k_latin_lorem, wrap_w );
			float box_h = box_pad + h_line1 + h_line2 + wrap_h + box_pad + gap * 3.0f;

			ImVec2 box_pos = ImGui::GetCursorScreenPos();
			dl->AddRectFilled( box_pos, ImVec2( box_pos.x + box_w, box_pos.y + box_h ),
							   ImGui::ColorConvertFloat4ToU32( bg_col_v ), 6.0f );

			ImGui::Dummy( ImVec2( box_w, box_pad ) );     // top padding
			ImGui::SetCursorPosX( ImGui::GetCursorPosX() + box_pad );

			auto t0 = std::chrono::high_resolution_clock::now();

			// -- SlugText --
			SlugText( latin_font, font_size, hi_col_u,
					  "Text(): The quick brown fox jumps over the lazy dog." );
			ImGui::SetCursorPosX( ImGui::GetCursorPosX() + box_pad );

			// -- SlugTextColored --
			SlugTextColored( latin_font, font_size, col_v,
							 "TextColored(): Dear Widgets \xe2\x80\x94 GPU Text Showcase" );
			ImGui::SetCursorPosX( ImGui::GetCursorPosX() + box_pad );

			// -- SlugTextWrapped --
			SlugTextWrapped( latin_font, font_size, col_u, k_latin_lorem, wrap_w, false );

			auto t1 = std::chrono::high_resolution_clock::now();
			double dt_ms = std::chrono::duration<double, std::milli>( t1 - t0 ).count();

			static float s_latin_ring32[32] = {};
			static float s_latin_ring128[128] = {};
			static int   s_latin_head = 0;
			s_latin_ring32[s_latin_head % 32] = (float)dt_ms;
			s_latin_ring128[s_latin_head % 128] = (float)dt_ms;
			s_latin_head++;
			float s_latin_avg32 = 0.0f, s_latin_avg128 = 0.0f;
			for ( int i = 0; i < 32; i++ ) s_latin_avg32 += s_latin_ring32[i];
			for ( int i = 0; i < 128; i++ ) s_latin_avg128 += s_latin_ring128[i];
			s_latin_avg32 /= 32.0f;
			s_latin_avg128 /= 128.0f;

			ImGui::Dummy( ImVec2( box_w, box_pad ) );     // bottom padding
			ImGui::TextDisabled( "DrawText time: %.3f ms  (avg32: %.3f ms  avg128: %.3f ms)", dt_ms, s_latin_avg32, s_latin_avg128 );
		}

		if ( !latin_font && !arabic_font )
			ImGui::TextDisabled( "No fonts available." );
#endif
	}

	// ---- Showcase: Rendering Equation BRDF Explorer ----
	void ShowShowcase()
	{
		ImGui::SetNextWindowSize( ImVec2( 860, 900 ), ImGuiCond_FirstUseEver );
		if ( !ImGui::Begin( "Showcase" ) )
		{
			ImGui::End(); return;
		}

		ShowTextShowcase();

		ApplyOpenAll();
		if ( !ImGui::CollapsingHeader( "BRDF Explorer" ) )
		{
			ImGui::End(); return;
		}

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

		// Static state (colors, draw options) -- declared here, UI shown after schema
		static ImVec4 cvLo = ImVec4( 1.00f, 0.86f, 0.31f, 1.0f );
		static ImVec4 cvLe = ImVec4( 1.00f, 0.63f, 0.20f, 1.0f );
		static ImVec4 cvFr = ImVec4( 0.31f, 0.86f, 0.47f, 1.0f );
		static ImVec4 cvLi = ImVec4( 0.39f, 0.71f, 1.00f, 1.0f );
		static ImVec4 cvCos = ImVec4( 1.00f, 0.39f, 0.39f, 1.0f );
		static ImVec4 cvN = ImVec4( 0.78f, 0.78f, 1.00f, 1.0f );
		static ImVec4 cvH = ImVec4( 0.71f, 0.51f, 1.00f, 1.0f );
		static ImVec4 cvSurf = ImVec4( 0.71f, 0.71f, 0.71f, 1.0f );
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
		ImU32 colLo = ImGui::ColorConvertFloat4ToU32( cvLo );
		ImU32 colLe = ImGui::ColorConvertFloat4ToU32( cvLe );
		ImU32 colFr = ImGui::ColorConvertFloat4ToU32( cvFr );
		ImU32 colLi = ImGui::ColorConvertFloat4ToU32( cvLi );
		ImU32 colCos = ImGui::ColorConvertFloat4ToU32( cvCos );
		ImU32 colN = ImGui::ColorConvertFloat4ToU32( cvN );
		ImU32 colOmegI = colLi;
		ImU32 colOmegO = colLo;
		ImU32 colH = ImGui::ColorConvertFloat4ToU32( cvH );
		ImU32 colSurf = ImGui::ColorConvertFloat4ToU32( cvSurf );

		// Build hex color strings for LaTeX \color{#RRGGBB}
		auto ToHex = []( ImVec4 c, char* buf ){
			snprintf( buf, 8, "#%02X%02X%02X", (int)(c.x * 255), (int)(c.y * 255), (int)(c.z * 255) );
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
				float sx = surfL + t * (surfR - surfL);
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
		if ( hLen > 0.001f )
		{
			hx2 /= hLen; hy2 /= hLen;
		}

		float arrowLen = hemiR * 0.85f;
		float arrowHead = 8.0f;

		float labelSz = 14.0f * schema_scale * label_scale; // LaTeX label size

		// Helper: draw arrow with triangle tip and LaTeX label
		auto DrawArrow = [ & ]( float dx, float dy, float len, ImU32 col, const char* latexLabel, bool incoming ){
			float ex = cx + dx * len;
			float ey = cy + dy * len;
			float perpX = -dy, perpY = dx;
			if ( incoming )
			{
				dl->AddLine( ImVec2( ex, ey ), ImVec2( cx, cy ), col, arrow_thick );
				dl->AddTriangleFilled(
					ImVec2( cx, cy ),
					ImVec2( cx + dx * arrowHead - perpX * arrowHead * 0.4f, cy + dy * arrowHead - perpY * arrowHead * 0.4f ),
					ImVec2( cx + dx * arrowHead + perpX * arrowHead * 0.4f, cy + dy * arrowHead + perpY * arrowHead * 0.4f ),
					col );
			}
			else
			{
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
		if ( show_cos_arc )
		{
			float arcR = arrowLen * 0.25f;
			int arcSegs = 16;
			for ( int i = 0; i < arcSegs; i++ )
			{
				float t0 = (float)i / (float)arcSegs;
				float t1 = (float)(i + 1) / (float)arcSegs;
				float a0 = -IM_PI * 0.5f - lightRad * t0;
				float a1 = -IM_PI * 0.5f - lightRad * t1;
				dl->AddLine(
					ImVec2( cx + cosf( a0 ) * arcR, cy + sinf( a0 ) * arcR ),
					ImVec2( cx + cosf( a1 ) * arcR, cy + sinf( a1 ) * arcR ),
					colCos, 1.5f );
			}
			float labelA = -IM_PI * 0.5f - lightRad * 0.5f;
			float lx2 = cx + cosf( labelA ) * (arcR + 16.0f);
			float ly2 = cy + sinf( labelA ) * (arcR + 16.0f);
			ImVec2 csz = ImWidgets::CalcLaTeXSize( labelSz, "\\cos\\theta_i" );
			ImWidgets::DrawLaTeX( dl, labelSz, ImVec2( lx2 - csz.x * 0.5f, ly2 - csz.y * 0.5f ), colCos, "\\cos\\theta_i" );
		}

		// ---- PBR BRDF lobe: f_r = diffuse/pi + D*G*F / (4*NdotL*NdotV) ----
		{
			float alpha = roughness * roughness;
			float alpha2 = alpha * alpha;
			// GGX Smith G1 helper
			auto SmithG1 = []( float NdotX, float a2 ) -> float{
				if ( NdotX <= 0.0f ) return 0.0f;
				float n2 = NdotX * NdotX;
				return 2.0f * NdotX / (NdotX + sqrtf( a2 + (1.0f - a2) * n2 ));
				};

			// Evaluate Cook-Torrance specular BRDF for a given outgoing angle theta_o
			// with fixed incoming light at lightRad from normal
			// Evaluate specular BRDF for outgoing angle theta_o from normal.
			// Light comes from LEFT at lightRad from normal.
			// Signed convention: negative = left, positive = right.
			// wi signed angle = -lightRad, wo signed angle = theta_o.
			auto EvalSpecular = [ & ]( float theta_o ) -> float{
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
				float denomNDF = cos2H * (alpha2 - 1.0f) + 1.0f;
				float D = alpha2 / (IM_PI * denomNDF * denomNDF);
				float G = SmithG1( NdotL, alpha2 ) * SmithG1( NdotV, alpha2 );
				float VdotH = cosf( theta_o - hAngle );
				float F = f0 + (1.0f - f0) * powf( ImMax( 1.0f - VdotH, 0.0f ), 5.0f );
				float denomBrdf = 4.0f * NdotL * NdotV;
				return (denomBrdf > 0.001f) ? D * G * F / denomBrdf : 0.0f;
				};

			// Diffuse term: albedo / pi (Lambertian)
			float diffuse = albedo / IM_PI;

			// Find max value for log normalization (sweep outgoing directions)
			float maxVal = diffuse;
			int lobeSegs = 100;
			for ( int i = 0; i <= lobeSegs; i++ )
			{
				// theta_o = outgoing angle from normal, sweep -pi/2 to +pi/2
				float theta_o = ((float)i / (float)lobeSegs - 0.5f) * IM_PI * 0.99f;
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
				float theta_o = ((float)i / (float)lobeSegs - 0.5f) * IM_PI * 0.99f;
				float cosT = cosf( theta_o );
				float spec = EvalSpecular( theta_o );

				// Log-space radii
				float lobeR = hemiR * 0.7f * brdf_scale;
				float diffCos = diffuse * ImMax( cosT, 0.0f );
				float rSpec = (logf( spec + 1.0f ) / logMax) * lobeR;
				float rDiff = (logf( diffCos + 1.0f ) / logMax) * lobeR;
				float rTotal = (logf( diffCos + spec + 1.0f ) / logMax) * lobeR;

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
			if ( show_reflection )
			{
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
				ImWidgets::DrawMarker( dl, ImVec2( specLabelX - astSz - 2.0f, specLabelY + (slsz.y - astSz) * 0.5f ), ImVec2( astSz, astSz ),
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
		auto DrawFormula = [ & ]( const char* latex, float sz2 ){
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImVec2 fsz = ImWidgets::CalcLaTeXSize( sz2, latex );
			fmDl->AddRectFilled( ImVec2( p.x - pad2, p.y - pad2 ), ImVec2( p.x + fsz.x + pad2, p.y + fsz.y + pad2 ), IM_COL32( 20, 22, 30, 255 ), 4.0f );
			ImWidgets::DrawLaTeX( fmDl, sz2, p, IM_COL32( 220, 220, 220, 255 ), latex );
			ImGui::Dummy( ImVec2( fsz.x + pad2 * 2, fsz.y + pad2 * 2 + 2.0f ) );
			};

		// Full BRDF formula with Asterisk marker on the left (links to schema lobe)
		{
			char buf[512]; snprintf( buf, sizeof( buf ),
									 "\\color{%s}{f_r} = "
									 "\\color{%s}{\\frac{\\text{albedo}}{\\pi}} + "
									 "\\frac{\\color{%s}{D_{GGX}} \\cdot \\color{%s}{G_{Smith}} \\cdot \\color{%s}{F_{Schlick}}}"
									 "{4 \\cdot \\color{%s}{|\\omega_i \\cdot n|} \\cdot \\color{%s}{|\\omega_o \\cdot n|}}",
									 hexFr, hexLe, hexFr, hexFr, hexFr, hexCos, hexCos );
			ImVec2 fPos = ImGui::GetCursorScreenPos();
			ImVec2 fSz2 = ImWidgets::CalcLaTeXSize( fmSize, buf );
			float astFmSz = fmSize * 0.6f;
			ImWidgets::DrawMarker( fmDl, ImVec2( fPos.x - pad2, fPos.y + (fSz2.y - astFmSz) * 0.5f ), ImVec2( astFmSz, astFmSz ),
								   colFr, IM_COL32( 0, 0, 0, 0 ), 0, 0.8f, 0.0f, 0.002f,
								   ImWidgetsMarker_Asterisk, ImWidgetsDrawType_Filled );
			// Indent the formula to make room for the asterisk
			ImGui::Indent( astFmSz + 4.0f );
			DrawFormula( buf, fmSize );
			ImGui::Unindent( astFmSz + 4.0f );
		}

		ImGui::Spacing();

		// ---- Two-column layout: formulas left, values right ----
		if ( ImGui::BeginTable( "##BRDFColumns", 2, ImGuiTableFlags_None ) )
		{
			ImGui::TableSetupColumn( "Formulas", ImGuiTableColumnFlags_WidthStretch, 0.55f );
			ImGui::TableSetupColumn( "Values", ImGuiTableColumnFlags_WidthStretch, 0.45f );

			ImGui::TableNextRow();

			// Left column: component formulas
			ImGui::TableSetColumnIndex( 0 );

			{
				char buf[256]; snprintf( buf, sizeof( buf ),
										 "\\color{%s}{D_{GGX}} = "
										 "\\frac{\\alpha^2}{\\pi (\\cos^2\\theta_h (\\alpha^2 - 1) + 1)^2}", hexFr );
				DrawFormula( buf, fmSize * 0.65f );
			}

			{
				char buf[256]; snprintf( buf, sizeof( buf ),
										 "\\color{%s}{G_{Smith}}(v) = "
										 "\\frac{2 (n \\cdot v)}{(n \\cdot v) + \\sqrt{\\alpha^2 + (1 - \\alpha^2)(n \\cdot v)^2}}", hexFr );
				DrawFormula( buf, fmSize * 0.65f );
			}

			{
				char buf[256]; snprintf( buf, sizeof( buf ),
										 "\\color{%s}{F_{Schlick}} = "
										 "F_0 + (1 - F_0)(1 - \\cos\\theta_h)^5", hexFr );
				DrawFormula( buf, fmSize * 0.65f );
			}

			// Right column: computed values
			ImGui::TableSetColumnIndex( 1 );

			{
				float NdotL2 = cosf( lightRad ), NdotV2 = cosf( viewRad );
				float cosH2 = cosf( (lightRad + viewRad) * 0.5f );
				float al = roughness * roughness;
				float al2 = al * al;
				float denomD = cosH2 * cosH2 * (al2 - 1.0f) + 1.0f;
				float Dval = al2 / (IM_PI * denomD * denomD);
				float G1L = 2.0f * NdotL2 / (NdotL2 + sqrtf( al2 + (1.0f - al2) * NdotL2 * NdotL2 ));
				float G1V = 2.0f * NdotV2 / (NdotV2 + sqrtf( al2 + (1.0f - al2) * NdotV2 * NdotV2 ));
				float Gval = G1L * G1V;
				float Fval = f0 + (1.0f - f0) * powf( 1.0f - cosH2, 5.0f );
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
						  NdotV2, (lightRad + viewRad) * 0.5f * 180.0f / IM_PI );
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
		ImWidgets::SetCurrentWindowBackgroundImage( background, background_size, false, IM_COL32( 255, 255, 255, 128 ) );

		// Apply scroll override from screenshot state machine
		if ( g_ss_scroll_y >= 0.0f )
			ImGui::SetScrollY( g_ss_scroll_y );

		// --- Open / Close All ----------------------------------------------
		if ( ImGui::Button( "Open All" ) )
		{
			s_open_all = 1;
		}
		ImGui::SameLine();
		if ( ImGui::Button( "Close All" ) )
		{
			s_open_all = -1; ImGui::SetScrollY( 0.0f );
		}

		// DPI / scaling control. FontScaleDpi drives ImPlatform_LpToPx; sizes authored
		// in "lp" (logical pixels) stay physically consistent across DPI settings.
		{
			float dpi = ImGui::GetStyle().FontScaleDpi;
			if ( ImGui::SliderFloat( "FontScaleDpi (lp -> px)", &dpi, 0.5f, 4.0f, "%.2fx" ) )
				ImGui::GetStyle().FontScaleDpi = dpi;
			ImGui::SameLine();
			ImGui::TextDisabled( "LpToPx(100)=%.1fpx   PxToLp(100)=%.1flp",
								 (double)ImPlatform_LpToPx( 100.0f ),
								 (double)ImPlatform_PxToLp( 100.0f ) );
		}
		ApplyOpenAll();
		if ( ImGui::CollapsingHeader( "Draw" ) )
		{
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Shapes##Draw" ) )
			{
				{
					float _sy0 = ImGui::GetCursorPos().y; ShowDrawShapeDemo(); DW_SsRecord( "Draw_Shape", _sy0, ImGui::GetCursorPos().y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Ephemerides##Draw" ) )
			{
				float _sy0 = ImGui::GetCursorPos().y;
				static int s_year = 2026, s_month = 5, s_day = 30, s_hour = 12, s_minute = 0;
				static float s_lon = 2.35f, s_lat = 48.85f; // Paris default

				ImGui::TextWrapped( "Schematic ephemerides: moon phase (synodic, ref new moon 2000-01-06 18:14 UT), "
				                    "and Sun/Earth system (heliocentric + observer-centric orthographic)." );

				ImGui::DragInt( "Year",   &s_year,   1.0f, 1900, 2100 );
				ImGui::DragInt( "Month",  &s_month,  0.1f, 1,    12 );
				ImGui::DragInt( "Day",    &s_day,    0.1f, 1,    31 );
				ImGui::DragInt( "Hour UT",   &s_hour,   0.1f, 0,    23 );
				ImGui::DragInt( "Minute UT", &s_minute, 0.5f, 0,    59 );
				ImGui::DragFloat( "Lon (deg)", &s_lon, 0.5f, -180.0f, 180.0f, "%.2f" );
				ImGui::DragFloat( "Lat (deg)", &s_lat, 0.25f, -90.0f,  90.0f, "%.2f" );

				ImGui::Spacing();
				ImGui::TextDisabled( "Moon" );
				{
					ImDrawList* dl = ImGui::GetWindowDrawList();
					ImVec2 p0 = ImGui::GetCursorScreenPos();
					float r = 90.0f;
					ImVec2 center( p0.x + r + 4.0f, p0.y + r + 4.0f );
					ImWidgets::DrawMoonEphemeris( dl, center, r, s_year, s_month, s_day, s_lon, s_lat );
					ImGui::Dummy( ImVec2( 2.0f * r + 8.0f, 2.0f * r + 8.0f ) );
				}

				ImGui::Spacing();
				ImGui::TextDisabled( "Sun / Earth system" );
				{
					ImDrawList* dl = ImGui::GetWindowDrawList();
					ImVec2 p0 = ImGui::GetCursorScreenPos();
					ImVec2 sz( ImMin( ImGui::GetContentRegionAvail().x, 560.0f ), 240.0f );
					dl->AddRectFilled( p0, ImVec2( p0.x + sz.x, p0.y + sz.y ), IM_COL32( 12, 14, 22, 255 ), 4.0f );
					ImWidgets::DrawEarthSunEphemeris( dl, p0, sz, s_year, s_month, s_day, s_hour, s_minute, s_lon, s_lat );
					dl->AddRect( p0, ImVec2( p0.x + sz.x, p0.y + sz.y ), IM_COL32( 90, 100, 120, 255 ), 4.0f );
					ImGui::Dummy( sz );
				}

				DW_SsRecord( "Draw_Ephemerides", _sy0, ImGui::GetCursorPos().y );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Star chart##Draw" ) )
			{
				float _sy0 = ImGui::GetCursorPos().y;
				static int s_year = 2026, s_month = 1, s_day = 15, s_hour = 21, s_minute = 0;
				static float s_lon = 2.35f, s_lat = 48.85f;
				static float s_mag = 5.5f;
				static float s_scale = 1.0f;
				static int s_culture = (int)ImWidgets::ImWidgetsSkyCulture_Western;
				static bool s_showSS = true;

				ImGui::TextWrapped( "Observer-centred sky chart: stars projected via alt/az, "
				                    "culture-specific constellation figure lines + native star names, "
				                    "solar system overlay (Moon with phase, 5 planets, Sun if above horizon). "
				                    "Arabic / CJK labels use ImWidgets::DrawText (Slug GPU font) for BiDi + complex shaping." );

				ImGui::DragInt( "Year",     &s_year,   1.0f, 1900, 2100 );
				ImGui::DragInt( "Month",    &s_month,  0.1f, 1,    12 );
				ImGui::DragInt( "Day",      &s_day,    0.1f, 1,    31 );
				ImGui::DragInt( "Hour UT",  &s_hour,   0.1f, 0,    23 );
				ImGui::DragInt( "Minute UT",&s_minute, 0.5f, 0,    59 );
				ImGui::DragFloat( "Lon (deg)", &s_lon, 0.5f, -180.0f, 180.0f, "%.2f" );
				ImGui::DragFloat( "Lat (deg)", &s_lat, 0.25f, -90.0f,  90.0f, "%.2f" );
				ImGui::SliderFloat( "Mag limit",  &s_mag,   0.0f, 7.0f, "%.1f" );
				ImGui::SliderFloat( "Star scale", &s_scale, 0.3f, 3.0f, "%.2f" );

				int nCul = ImWidgets::GetSkyCultureCount();
				char const* curCulName = ImWidgets::GetSkyCultureName( (ImWidgets::ImWidgetsSkyCulture)s_culture );
				if ( ImGui::BeginCombo( "Sky culture", curCulName ) )
				{
					for ( int i = 0; i < nCul; ++i )
					{
						bool sel = ( s_culture == i );
						char const* n = ImWidgets::GetSkyCultureName( (ImWidgets::ImWidgetsSkyCulture)i );
						if ( ImGui::Selectable( n, sel ) ) s_culture = i;
						if ( sel ) ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::Checkbox( "Show solar system", &s_showSS );

				ImGui::Spacing();
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 p0 = ImGui::GetCursorScreenPos();
				float R = 220.0f;
				ImVec2 center( p0.x + R + 4.0f, p0.y + R + 4.0f );
				// Pass the Arabic font when the Arabic culture is active so star labels
				// shape correctly (the default ImGui font has no Arabic glyphs / shaper).
				ImFont* chartLabelFont = nullptr;
				if ( (ImWidgets::ImWidgetsSkyCulture)s_culture == ImWidgets::ImWidgetsSkyCulture_Arabic )
					chartLabelFont = g_amiriFont;
				ImWidgets::DrawStarChart( dl, center, R, s_year, s_month, s_day, s_hour, s_minute,
				                          s_lon, s_lat, s_mag, s_scale,
				                          (ImWidgets::ImWidgetsSkyCulture)s_culture, s_showSS,
				                          IM_COL32( 8, 11, 22, 255 ),
				                          IM_COL32( 180, 195, 225, 230 ),
				                          chartLabelFont );
				ImGui::Dummy( ImVec2( 2.0f * R + 8.0f, 2.0f * R + 8.0f ) );

				DW_SsRecord( "Draw_StarChart", _sy0, ImGui::GetCursorPos().y );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Sun path##Draw" ) )
			{
				float _sy0 = ImGui::GetCursorPos().y;
				static float s_lat = 48.85f, s_lon = 2.35f;
				static int   s_tz = 1, s_year = 2026;
				static int   s_mode = (int)ImWidgets::ImWidgetsSunPathMode_Polar;

				ImGui::TextWrapped( "Sun-path diagram with monthly arcs (Jan blue -> Jun yellow -> Dec blue) "
				                    "and analemma figure-8 loops at fixed local clock times (06/09/12/15/18 h). "
				                    "Equation of time + longitude offset from time-zone meridian create the analemma's east-west spread." );

				ImGui::DragInt  ( "Year",        &s_year, 1.0f, 1900, 2100 );
				ImGui::DragFloat( "Lat (deg)",   &s_lat,  0.25f, -90.0f, 90.0f, "%.2f" );
				ImGui::DragFloat( "Lon (deg)",   &s_lon,  0.5f, -180.0f, 180.0f, "%.2f" );
				ImGui::DragInt  ( "TZ offset h", &s_tz,   0.1f, -12, 14 );
				const char* modes[] = { "Polar (sky dome)", "Cartesian (azimuth x altitude)" };
				ImGui::Combo( "Mode", &s_mode, modes, IM_ARRAYSIZE( modes ) );

				ImGui::Spacing();
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 p0 = ImGui::GetCursorScreenPos();
				if ( s_mode == (int)ImWidgets::ImWidgetsSunPathMode_Polar )
				{
					ImVec2 sz( 460.0f, 460.0f );
					dl->AddRectFilled( p0, ImVec2( p0.x + sz.x, p0.y + sz.y ),
					                   IM_COL32( 18, 22, 32, 255 ), 4.0f );
					ImWidgets::DrawSunPath( dl, p0, sz, s_lat, s_lon, s_tz, s_year,
					                        ImWidgets::ImWidgetsSunPathMode_Polar );
					ImGui::Dummy( sz );
				}
				else
				{
					ImVec2 sz( ImMin( ImGui::GetContentRegionAvail().x, 720.0f ), 320.0f );
					dl->AddRectFilled( p0, ImVec2( p0.x + sz.x, p0.y + sz.y ),
					                   IM_COL32( 18, 22, 32, 255 ), 4.0f );
					ImWidgets::DrawSunPath( dl, p0, sz, s_lat, s_lon, s_tz, s_year,
					                        ImWidgets::ImWidgetsSunPathMode_Cartesian );
					ImGui::Dummy( sz );
				}

				DW_SsRecord( "Draw_SunPath", _sy0, ImGui::GetCursorPos().y );
				ImGui::TreePop();
			}
#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Text##Draw" ) )
			{
				ShowDrawTextDemo();
				{
					float _sy0 = ImGui::GetCursorPos().y; ShowTypographyAnimations(); DW_SsRecord( "Typography_Animations", _sy0, ImGui::GetCursorPos().y );
				}
				ShowLaTeXDemo();
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Primitives##Draw" ) )
			{
				static float s_cull_cshader_h = 0; float s_cull_cshader_y;
				if ( BeginCullSection( s_cull_cshader_h, s_cull_cshader_y ) )
				{
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Custom Shader" ) )
						{
							ShowCustomShaderDemo();
						}
						DW_SsRecord( "Custom_Shader", _sy0, ImGui::GetCursorPos().y );
					}
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Thick line" ) )
					{
						ImGui::TextWrapped( "GPU stroke expansion based on \"Fast GPU stroke expansion\" (Nehab, HPG 2024). "
											"CPU: Euler spiral offset curves with ESPC flattening and cusp handling. "
											"GPU: winding-number pixel shader for zero-overdraw fill." );
						ImGui::TextDisabled( "Paper: https://arxiv.org/abs/2405.00127" );
						ImGui::TextDisabled( "Ref: https://github.com/linebender/gpu-stroke-expansion-paper" );
						ImGui::Spacing();
						float const size = CanvasSize();
						ImDrawList* pDrawList = ImGui::GetWindowDrawList();

						static float line_width = 64.0f;
						static float miter_limit = 4.0f;
						static float tolerance = 0.25f;
						static int cap_type = (int)ImWidgetsCap_Round;
						static int join_type = (int)ImWidgetsJoin_Round;
						static int path_type = 0; // 0=single cubic, 1=S-curve, 2=polyline
						static bool show_wireframe = false;
						static bool show_ctrl_poly = true;
						static bool closed = false;
						static bool dashed = false;
						static float dash_len = 20.0f;
						static float gap_len = 10.0f;
						static float dash_offset = 0.0f;

						static ImVec4 color_v( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 128.0f / 255.0f );
						static ImU32 color_col = ImGui::GetColorU32( color_v );
						if ( ImGui::ColorEdit4( "Color##ThickLine", &color_v.x ) )
							color_col = ImGui::GetColorU32( color_v );
						ImGui::DragFloat( "Thickness##TL", &line_width, 0.125f, 0.5f, 100.0f );
						ImGui::DragFloat( "Miter limit##TL", &miter_limit, 0.1f, 1.0f, 10.0f );
						ImGui::DragFloat( "Tolerance##TL", &tolerance, 0.01f, 0.05f, 2.0f );

						const char* cap_names[] = { "None", "Butt", "Square", "Round", "TriangleOut", "TriangleIn" };
						ImGui::Combo( "Cap##TL", &cap_type, cap_names, ImWidgetsCap_COUNT );
						const char* join_names[] = { "Round", "Miter", "Bevel" };
						ImGui::Combo( "Join##TL", &join_type, join_names, ImWidgetsJoin_COUNT );
						const char* path_names[] = { "Single Cubic", "S-Curve (2 cubics)", "Polyline (L-shape)" };
						ImGui::Combo( "Path##TL", &path_type, path_names, 3 );
						static int primitive_type = (int)ImWidgetsPrimitive_Line;
						static int correctness_type = (int)ImWidgetsCorrectness_Weak;
						const char* prim_names[] = { "Line", "Arc" };
						ImGui::Combo( "Primitive##TL", &primitive_type, prim_names, ImWidgetsPrimitive_COUNT );
						const char* corr_names[] = { "Weak", "Strong" };
						ImGui::Combo( "Correctness##TL", &correctness_type, corr_names, ImWidgetsCorrectness_COUNT );
						ImGui::Checkbox( "Control polygon##TL", &show_ctrl_poly );
						ImGui::SameLine();
						ImGui::Checkbox( "Wireframe##TL", &show_wireframe );
						ImGui::SameLine();
						ImGui::Checkbox( "Closed##TL", &closed );
						ImGui::SameLine();
						ImGui::Checkbox( "Dashed##TL", &dashed );
						if ( dashed )
						{
							ImGui::DragFloat( "Dash##TL", &dash_len, 0.5f, 1.0f, 200.0f );
							ImGui::DragFloat( "Gap##TL", &gap_len, 0.5f, 1.0f, 200.0f );
							ImGui::DragFloat( "Dash Offset##TL", &dash_offset, 0.5f, -200.0f, 200.0f );
						}

						ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
						ImVec2 canvas_size( size, size );
						ImGui::InvisibleButton( "##thick_line_canvas", canvas_size );
						bool canvas_hovered = ImGui::IsItemHovered();

						// Draggable control points (relative to canvas, normalized 0-1)
						static ImVec2 cp_cubic[4] = {
							ImVec2( 0.25f, 0.75f ), ImVec2( 1.0f, 0.0f ),
							ImVec2( 0.0f, 0.0f ), ImVec2( 0.75f, 0.75f )
							//ImVec2( 0.1f, 0.8f ), ImVec2( 0.3f, 0.1f ),
							//ImVec2( 0.7f, 0.1f ), ImVec2( 0.9f, 0.8f )
						};
						static ImVec2 cp_scurve[7] = {
							ImVec2( 0.05f, 0.5f ), ImVec2( 0.15f, 0.1f ), ImVec2( 0.35f, 0.1f ),
							ImVec2( 0.5f, 0.5f ),
							ImVec2( 0.65f, 0.9f ), ImVec2( 0.85f, 0.9f ), ImVec2( 0.95f, 0.5f )
						};
						static ImVec2 cp_poly[4] = {
							ImVec2( 0.15f, 0.15f ), ImVec2( 0.75f, 0.15f ),
							ImVec2( 0.75f, 0.85f ), ImVec2( 0.15f, 0.85f )
						};

						// Determine active control point set
						ImVec2* cp = NULL;
						int cp_count = 0;
						if ( path_type == 0 )
						{
							cp = cp_cubic; cp_count = 4;
						}
						else if ( path_type == 1 )
						{
							cp = cp_scurve; cp_count = 7;
						}
						else
						{
							cp = cp_poly; cp_count = 4;
						}

						// Drag logic
						static int dragging = -1;
						ImVec2 mouse = ImGui::GetMousePos();
						float grab_r = 8.0f;

						if ( ImGui::IsMouseReleased( 0 ) ) dragging = -1;
						if ( canvas_hovered && ImGui::IsMouseClicked( 0 ) )
						{
							for ( int i = 0; i < cp_count; ++i )
							{
								ImVec2 sp( canvas_pos.x + cp[i].x * size, canvas_pos.y + cp[i].y * size );
								float dx = mouse.x - sp.x, dy = mouse.y - sp.y;
								if ( dx * dx + dy * dy < grab_r * grab_r * 4.0f )
								{
									dragging = i;
									break;
								}
							}
						}
						if ( dragging >= 0 && dragging < cp_count )
						{
							cp[dragging].x = ImClamp( (mouse.x - canvas_pos.x) / size, 0.0f, 1.0f );
							cp[dragging].y = ImClamp( (mouse.y - canvas_pos.y) / size, 0.0f, 1.0f );
						}

						// Convert to screen-space points
						ImVec2 sp[7];
						for ( int i = 0; i < cp_count; ++i )
							sp[i] = ImVec2( canvas_pos.x + cp[i].x * size, canvas_pos.y + cp[i].y * size );

						// Draw background
						pDrawList->AddRectFilled( canvas_pos, ImVec2( canvas_pos.x + size, canvas_pos.y + size ),
												  IM_COL32( 30, 30, 30, 255 ) );

						// Set debug wireframe from checkbox
						ImWidgets::SetStrokeDebugWireframe( show_wireframe );

						// Draw the stroked path
						float dash_arr[2] = { dash_len, gap_len };
						if ( path_type == 2 ) // Polyline
						{
							if ( dashed )
								ImWidgets::DrawStrokedDashedPolyline( pDrawList, sp, cp_count, color_col, line_width,
																	  dash_arr, 2, dash_offset,
																	  (ImWidgetsCap)cap_type, (ImWidgetsJoin)join_type,
																	  miter_limit, closed );
							else
								ImWidgets::DrawStrokedPolyline( pDrawList, sp, cp_count, color_col, line_width,
																(ImWidgetsCap)cap_type, (ImWidgetsJoin)join_type,
																miter_limit, closed );
						}
						else if ( dashed )
						{
							int pc = (path_type == 0) ? 4 : 7;
							ImWidgets::DrawStrokedDashedBezierPath( pDrawList, sp, pc, color_col, line_width,
																	dash_arr, 2, dash_offset,
																	(ImWidgetsCap)cap_type, (ImWidgetsJoin)join_type,
																	miter_limit, tolerance, closed,
																	(ImWidgetsPrimitive)primitive_type,
																	(ImWidgetsCorrectness)correctness_type );
						}
						else
						{
							int pc = (path_type == 0) ? 4 : 7;
							ImWidgets::DrawStrokedBezierPath( pDrawList, sp, pc, color_col, line_width,
															  (ImWidgetsCap)cap_type, (ImWidgetsJoin)join_type,
															  miter_limit, tolerance, closed,
															  (ImWidgetsPrimitive)primitive_type,
															  (ImWidgetsCorrectness)correctness_type );
						}

						// Draw control polygon
						if ( show_ctrl_poly )
						{
							ImU32 ctrl_col = IM_COL32( 255, 255, 255, 80 );
							ImU32 handle_col = IM_COL32( 255, 200, 50, 200 );
							for ( int i = 0; i < cp_count - 1; ++i )
								pDrawList->AddLine( sp[i], sp[i + 1], ctrl_col, 1.0f );
							if ( closed && cp_count > 2 )
								pDrawList->AddLine( sp[cp_count - 1], sp[0], ctrl_col, 1.0f );
							for ( int i = 0; i < cp_count; ++i )
								pDrawList->AddCircleFilled( sp[i], grab_r, handle_col );
						}
					}
					{ float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Thick Line Benchmark" ) )
					{
						ImGui::TextWrapped( "Side-by-side comparison of three thick-line algorithms on a regular grid of V-shapes "
											"with increasing interior angle. Rendered left-to-right: "
											"(1) Stroke = ImWidgets DrawStrokedPolyline (ESPC Euler-spiral expansion + GPU winding shader; "
											"optional dashed variant via DrawStrokedDashedPolyline). "
											"(2) Dashed = ImWidgets DrawDashedPolylineAA (Rougier-style antialiased dashed polyline; "
											"selectable CPU vs GPU path). "
											"(3) Polyline = ImGui built-in AddPolyline (CPU triangulated fringe). "
											"Timing shown is the CPU submission cost over one frame." );
						ImGui::Spacing();

						static int   tlb_cols = 5;
						static int   tlb_rows = 5;
						static float tlb_line_width = 20.0f;
						static float tlb_min_deg = 5.0f;
						static float tlb_max_deg = 175.0f;
						static int   tlb_cap_type = (int)ImWidgetsCap_Round;
						static int   tlb_join_type = (int)ImWidgetsJoin_Round;
						static float tlb_miter_limit = 4.0f;
						static float tlb_dash_len = 8.0f;
						static float tlb_gap_len = 4.0f;
						static float tlb_dash_offset = 0.0f;
						static bool  tlb_show_grid = true;
						static bool  tlb_show_stroke = true;
						static bool  tlb_show_dashed = true;
						// Single shared toggle: when true both canvas 1 (Stroke) and
						// canvas 2 (PolylineAA) switch to their dashed variants.
						static bool  tlb_dashed = false;
						static bool  tlb_show_poly = true;
						static float tlb_canvas_scale = 2.0f;
						static ImVec4 tlb_color_v( 91.0f / 255.0f, 194.0f / 255.0f, 231.0f / 255.0f, 125.0f / 255.0f );
						ImU32 tlb_color_col = ImGui::GetColorU32( tlb_color_v );

						ImGui::SliderInt( "Grid Cols##TLB", &tlb_cols, 1, 32 );
						ImGui::SliderInt( "Grid Rows##TLB", &tlb_rows, 1, 32 );
						ImGui::SliderFloat( "Canvas Scale##TLB", &tlb_canvas_scale, 0.25f, 4.0f, "%.2fx" );
						ImGui::SameLine();
						if ( ImGui::SmallButton( "Reset##TLBScale" ) ) tlb_canvas_scale = 1.0f;
						ImGui::DragFloat( "Thickness##TLB", &tlb_line_width, 0.125f, 0.5f, 50.0f );
						ImGui::DragFloatRange2( "Angle Range (deg)##TLB", &tlb_min_deg, &tlb_max_deg,
												0.5f, 1.0f, 179.0f, "Min: %.1f", "Max: %.1f" );
						const char* tlb_cap_names[] = { "None", "Butt", "Square", "Round", "TriangleOut", "TriangleIn" };
						const char* tlb_join_names[] = { "Round", "Miter", "Bevel" };
						ImGui::Combo( "Cap##TLB", &tlb_cap_type, tlb_cap_names, ImWidgetsCap_COUNT );
						ImGui::Combo( "Join##TLB", &tlb_join_type, tlb_join_names, ImWidgetsJoin_COUNT );
						ImGui::DragFloat( "Miter Limit##TLB", &tlb_miter_limit, 0.1f, 1.0f, 10.0f );
						ImGui::DragFloat( "Dash Length##TLB", &tlb_dash_len, 0.1f, 0.5f, 200.0f );
						ImGui::DragFloat( "Gap Length##TLB", &tlb_gap_len, 0.1f, 0.0f, 200.0f );
						ImGui::DragFloat( "Dash Offset##TLB", &tlb_dash_offset, 0.5f, -200.0f, 200.0f );
						ImGui::Checkbox( "Show cell grid##TLB", &tlb_show_grid );
						ImGui::SameLine(); ImGui::Checkbox( "Dashed##TLB", &tlb_dashed );
						ImGui::SameLine(); ImGui::Checkbox( "Draw Stroke##TLB", &tlb_show_stroke );
						ImGui::SameLine(); ImGui::Checkbox( "Draw PolylineAA##TLB", &tlb_show_dashed );
						ImGui::SameLine(); ImGui::Checkbox( "Draw Polyline##TLB", &tlb_show_poly );
						// DrawDashedPolylineAA has two implementations -- toggle the global here.
						bool tlb_dashed_use_gpu = ImWidgets::GetDashedLinesUseGPU();
						if ( ImGui::Checkbox( "Dashed GPU Path##TLB", &tlb_dashed_use_gpu ) )
							ImWidgets::SetDashedLinesUseGPU( tlb_dashed_use_gpu );
						if ( ImGui::ColorEdit4( "Color##TLB", &tlb_color_v.x ) )
							tlb_color_col = ImGui::GetColorU32( tlb_color_v );

						ImDrawList* pDrawList = ImGui::GetWindowDrawList();

						// Three square canvases side by side
						float tlb_avail = ImGui::GetContentRegionAvail().x;
						float tlb_gap = 10.0f;
						float tlb_base_side = ImMin( (tlb_avail - 2.0f * tlb_gap) / 3.0f, 380.0f );
						float tlb_side = tlb_base_side * tlb_canvas_scale;
						if ( tlb_side < 32.0f ) tlb_side = 32.0f;

						ImVec2 tlb_origin_s = ImGui::GetCursorScreenPos();                                                   // Stroke
						ImVec2 tlb_origin_d = ImVec2( tlb_origin_s.x + (tlb_side + tlb_gap), tlb_origin_s.y );        // Dashed
						ImVec2 tlb_origin_p = ImVec2( tlb_origin_s.x + (tlb_side + tlb_gap) * 2.f, tlb_origin_s.y );        // Polyline
						ImGui::InvisibleButton( "##tlb_canvas", ImVec2( tlb_side * 3.0f + tlb_gap * 2.0f, tlb_side ) );

						// Background panels
						ImU32 bg_col = IM_COL32( 30, 30, 30, 255 );
						ImU32 border_col = IM_COL32( 80, 80, 80, 255 );
						ImU32 label_col = IM_COL32( 255, 255, 255, 220 );
						pDrawList->AddRectFilled( tlb_origin_s, ImVec2( tlb_origin_s.x + tlb_side, tlb_origin_s.y + tlb_side ), bg_col );
						pDrawList->AddRectFilled( tlb_origin_d, ImVec2( tlb_origin_d.x + tlb_side, tlb_origin_d.y + tlb_side ), bg_col );
						pDrawList->AddRectFilled( tlb_origin_p, ImVec2( tlb_origin_p.x + tlb_side, tlb_origin_p.y + tlb_side ), bg_col );
						pDrawList->AddRect( tlb_origin_s, ImVec2( tlb_origin_s.x + tlb_side, tlb_origin_s.y + tlb_side ), border_col );
						pDrawList->AddRect( tlb_origin_d, ImVec2( tlb_origin_d.x + tlb_side, tlb_origin_d.y + tlb_side ), border_col );
						pDrawList->AddRect( tlb_origin_p, ImVec2( tlb_origin_p.x + tlb_side, tlb_origin_p.y + tlb_side ), border_col );
						pDrawList->AddText( ImVec2( tlb_origin_s.x + 4.0f, tlb_origin_s.y + 2.0f ), label_col,
											tlb_dashed ? "1: DrawStrokedPolyline (dashed)" : "1: DrawStrokedPolyline" );
						const char* tlb_canvas2_label =
							tlb_dashed
							? (tlb_dashed_use_gpu ? "2: DrawDashedPolylineAA (GPU)" : "2: DrawDashedPolylineAA (CPU)")
							: (tlb_dashed_use_gpu ? "2: DrawPolylineAA (GPU)" : "2: DrawPolylineAA (CPU)");
						pDrawList->AddText( ImVec2( tlb_origin_d.x + 4.0f, tlb_origin_d.y + 2.0f ), label_col, tlb_canvas2_label );
						pDrawList->AddText( ImVec2( tlb_origin_p.x + 4.0f, tlb_origin_p.y + 2.0f ), label_col, "3: ImDrawList::AddPolyline" );

						int   tlb_total = tlb_cols * tlb_rows;
						float tlb_cell_w = tlb_side / (float)tlb_cols;
						float tlb_cell_h = tlb_side / (float)tlb_rows;
						float tlb_arm = ImMin( tlb_cell_w, tlb_cell_h ) * 0.40f;

						// Optional cell grid overlay
						if ( tlb_show_grid )
						{
							ImU32 grid_col = IM_COL32( 70, 70, 70, 200 );
							ImVec2 origins[3] = { tlb_origin_s, tlb_origin_d, tlb_origin_p };
							for ( int k = 0; k < 3; ++k )
							{
								for ( int c = 1; c < tlb_cols; ++c )
								{
									float x = origins[k].x + (float)c * tlb_cell_w;
									pDrawList->AddLine( ImVec2( x, origins[k].y ),
														ImVec2( x, origins[k].y + tlb_side ), grid_col, 1.0f );
								}
								for ( int r = 1; r < tlb_rows; ++r )
								{
									float y = origins[k].y + (float)r * tlb_cell_h;
									pDrawList->AddLine( ImVec2( origins[k].x, y ),
														ImVec2( origins[k].x + tlb_side, y ), grid_col, 1.0f );
								}
							}
						}

						// Lambda: fill a 3-point V-shape for cell index i, anchored at the given canvas origin.
						// The opening angle increases linearly with i across the whole grid.
						auto tlb_make_v = [ & ]( int i, ImVec2 origin, ImVec2 out_pts[3] ){
							int col_i = i % tlb_cols;
							int row_i = i / tlb_cols;
							ImVec2 center( origin.x + ((float)col_i + 0.5f) * tlb_cell_w,
										   origin.y + ((float)row_i + 0.5f) * tlb_cell_h );
							float t = (tlb_total > 1) ? (float)i / (float)(tlb_total - 1) : 0.5f;
							float angle_deg = ImLerp( tlb_min_deg, tlb_max_deg, t );
							float half = angle_deg * 0.5f * (IM_PI / 180.0f);
							float sh = sinf( half );
							float ch = cosf( half );
							// V opens upward: vertex sits low, both arms point up-left and up-right.
							ImVec2 vertex( center.x, center.y + tlb_arm * 0.5f );
							out_pts[0] = ImVec2( vertex.x - sh * tlb_arm, vertex.y - ch * tlb_arm );
							out_pts[1] = vertex;
							out_pts[2] = ImVec2( vertex.x + sh * tlb_arm, vertex.y - ch * tlb_arm );
							};

						// --- 1) Stroke: ImWidgets::DrawStrokedPolyline (solid or dashed) ----
						double tlb_ms_stroke = 0.0;
						if ( tlb_show_stroke )
						{
							float dash_arr_s[2] = { tlb_dash_len, tlb_gap_len };
							auto ts0 = std::chrono::high_resolution_clock::now();
							for ( int i = 0; i < tlb_total; ++i )
							{
								ImVec2 pts[3];
								tlb_make_v( i, tlb_origin_s, pts );
								if ( tlb_dashed )
								{
									ImWidgets::DrawStrokedDashedPolyline( pDrawList, pts, 3, tlb_color_col, tlb_line_width,
																		  dash_arr_s, 2, tlb_dash_offset,
																		  (ImWidgetsCap)tlb_cap_type,
																		  (ImWidgetsJoin)tlb_join_type,
																		  tlb_miter_limit, false );
								}
								else
								{
									ImWidgets::DrawStrokedPolyline( pDrawList, pts, 3, tlb_color_col, tlb_line_width,
																	(ImWidgetsCap)tlb_cap_type,
																	(ImWidgetsJoin)tlb_join_type,
																	tlb_miter_limit, false );
								}
							}
							auto ts1 = std::chrono::high_resolution_clock::now();
							tlb_ms_stroke = std::chrono::duration<double, std::milli>( ts1 - ts0 ).count();
						}

						// --- 2) Polyline AA: solid (DrawPolylineAA) or dashed (DrawDashedPolylineAA) ---
						// Both share the same Rougier 2013 SDF code paths and respect the
						// CPU/GPU global toggled above.
						double tlb_ms_dashed = 0.0;
						if ( tlb_show_dashed )
						{
							auto td0 = std::chrono::high_resolution_clock::now();
							for ( int i = 0; i < tlb_total; ++i )
							{
								ImVec2 pts[3];
								tlb_make_v( i, tlb_origin_d, pts );
								if ( tlb_dashed )
								{
									ImWidgets::DrawDashedPolylineAA( pDrawList, pts, 3, tlb_color_col, tlb_line_width,
																	 tlb_dash_len, tlb_gap_len, tlb_dash_offset,
																	 false,
																	 (ImWidgetsCap)tlb_cap_type,
																	 (ImWidgetsJoin)tlb_join_type,
																	 tlb_miter_limit );
								}
								else
								{
									ImWidgets::DrawPolylineAA( pDrawList, pts, 3, tlb_color_col, tlb_line_width,
															   false,
															   (ImWidgetsCap)tlb_cap_type,
															   (ImWidgetsJoin)tlb_join_type,
															   tlb_miter_limit );
								}
							}
							auto td1 = std::chrono::high_resolution_clock::now();
							tlb_ms_dashed = std::chrono::duration<double, std::milli>( td1 - td0 ).count();
						}

						// --- 3) Polyline: ImGui::AddPolyline ---------------------------------
						double tlb_ms_poly = 0.0;
						if ( tlb_show_poly )
						{
							auto tp0 = std::chrono::high_resolution_clock::now();
							for ( int i = 0; i < tlb_total; ++i )
							{
								ImVec2 pts[3];
								tlb_make_v( i, tlb_origin_p, pts );
								pDrawList->AddPolyline( pts, 3, tlb_color_col, ImDrawFlags_None, tlb_line_width );
							}
							auto tp1 = std::chrono::high_resolution_clock::now();
							tlb_ms_poly = std::chrono::duration<double, std::milli>( tp1 - tp0 ).count();
						}

						// --- Rolling 32-sample averages --------------------------------------
						static float tlb_ring_stroke[32] = {};
						static float tlb_ring_dashed[32] = {};
						static float tlb_ring_poly[32] = {};
						static int   tlb_ring_head = 0;
						int slot = tlb_ring_head % 32;
						tlb_ring_stroke[slot] = (float)tlb_ms_stroke;
						tlb_ring_dashed[slot] = (float)tlb_ms_dashed;
						tlb_ring_poly[slot] = (float)tlb_ms_poly;
						tlb_ring_head++;
						float tlb_avg_stroke = 0.0f, tlb_avg_dashed = 0.0f, tlb_avg_poly = 0.0f;
						for ( int i = 0; i < 32; ++i )
						{
							tlb_avg_stroke += tlb_ring_stroke[i];
							tlb_avg_dashed += tlb_ring_dashed[i];
							tlb_avg_poly += tlb_ring_poly[i];
						}
						tlb_avg_stroke /= 32.0f;
						tlb_avg_dashed /= 32.0f;
						tlb_avg_poly /= 32.0f;

						ImGui::Text( "V-shapes drawn: %d  (%d cols x %d rows)", tlb_total, tlb_cols, tlb_rows );
						ImGui::Text( "1 DrawStrokedPolyline %s : %7.3f ms   (avg32: %7.3f ms)",
									 tlb_dashed ? "(dashed)" : "(solid) ", tlb_ms_stroke, tlb_avg_stroke );
						ImGui::Text( "2 %-22s %s : %7.3f ms   (avg32: %7.3f ms)",
									 tlb_dashed ? "DrawDashedPolylineAA" : "DrawPolylineAA",
									 tlb_dashed_use_gpu ? "(GPU)" : "(CPU)", tlb_ms_dashed, tlb_avg_dashed );
						ImGui::Text( "3 AddPolyline               : %7.3f ms   (avg32: %7.3f ms)", tlb_ms_poly, tlb_avg_poly );
						if ( tlb_avg_stroke > 0.0f && tlb_avg_poly > 0.0f )
							ImGui::TextDisabled( "Stroke vs Polyline: %.2fx", tlb_avg_stroke / tlb_avg_poly );
						if ( tlb_avg_dashed > 0.0f && tlb_avg_stroke > 0.0f )
							ImGui::TextDisabled( "Dashed vs Stroke:   %.2fx (dashing overhead)", tlb_avg_dashed / tlb_avg_stroke );
					}
					DW_SsRecord( "Thick_Line_Benchmark", _sy0, ImGui::GetCursorPos().y ); }

					{ float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Dashed Polylines" ) )
					{
						ImGui::TextWrapped( "Arc-length accurate dashed polylines with proper caps and joins. "
											"Based on \"Shader-Based Antialiased, Dashed, Stroked Polylines\" (Rougier, JCGT 2013)." );
						ImGui::TextDisabled( "Paper: https://jcgt.org/published/0002/02/08/" );
						ImGui::Spacing();
						ImDrawList* dl = ImGui::GetWindowDrawList();
						float avail = ImMin( ImGui::GetContentRegionAvail().x, 400.0f );
						float side = ImMin( avail, ImGui::GetContentRegionAvail().y );
						if ( side < 64.0f ) side = avail;
						ImVec2 origin = ImGui::GetCursorScreenPos();
						ImGui::InvisibleButton( "##zone_dashed_poly", ImVec2( side, side ) );

						static float thickness = 16.0f;
						static float dash_len = 100.0f;
						static float gap_len = 32.0f;
						static float offset = 0.0f;
						static bool  animate = true;
						static bool  closed = false;
						static int   cap_idx = (int)ImWidgetsCap_Round;
						static int   join_idx = (int)ImWidgetsJoin_Round;
						static float miter_limit = 4.0f;
						static int   path_type = 1;
						const char* caps[] = { "None", "Butt", "Square", "Round", "TriangleOut", "TriangleIn" };
						const char* joins[] = { "Round", "Mitter", "Bevel" };
						const char* paths[] = { "ZigZag", "Sine", "Spiral", "RoundedRect", "Circle", "Infinity", "Rose (k=5)", "Heart", "Sawtooth", "Star", "Bezier S" };
						ImGui::SetCursorScreenPos( origin + ImVec2( 8, 6 ) );
						dl->AddRect( origin, origin + ImVec2( side, side ), IM_COL32( 64, 64, 64, 255 ) );

						ImVec2 pts_stack[256];
						ImVec2* pts = pts_stack;
						int pts_count = 0;
						float left = origin.x + 16.0f;
						float right = origin.x + side - 16.0f;
						float top = origin.y + 24.0f;
						float bottom = origin.y + side - 24.0f;
						float midx = (left + right) * 0.5f;
						if ( path_type == 0 )
						{
							pts_stack[0] = ImVec2( left, top ); pts_stack[1] = ImVec2( midx, bottom );
							pts_stack[2] = ImVec2( right, top ); pts_stack[3] = ImVec2( midx, top + (bottom - top) * 0.5f );
							pts_stack[4] = ImVec2( left, bottom ); pts_stack[5] = ImVec2( midx, top + (bottom - top) * 0.25f );
							pts_stack[6] = ImVec2( right, bottom ); pts_count = 7;
						}
						else if ( path_type == 1 )
						{
							int N = 64; for ( int i = 0; i < N; ++i )
							{
								float t = (float)i / (float)(N - 1); pts_stack[i] = ImVec2( ImLerp( left, right, t ), ImLerp( top + (bottom - top) * 0.2f, bottom - (bottom - top) * 0.2f, 0.5f + 0.4f * sinf( t * 4.0f * IM_PI ) ) );
							} pts_count = N;
						}
						else if ( path_type == 2 )
						{
							int N = 96; ImVec2 center( (left + right) * 0.5f, (top + bottom) * 0.5f ); float rx = (right - left) * 0.45f, ry = (bottom - top) * 0.45f; for ( int i = 0; i < N; ++i )
							{
								float t = (float)i / (float)(N - 1); float ang = t * 4.f * IM_PI, r = 0.1f + 0.9f * t; pts_stack[i] = ImVec2( center.x + cosf( ang ) * rx * r, center.y + sinf( ang ) * ry * r );
							} pts_count = N;
						}
						else if ( path_type == 3 )
						{
							float pad = 28.f; ImVec2 pmin( left + pad, top + pad ), pmax( right - pad, bottom - pad ); float rx = (pmax.x - pmin.x) * 0.18f, ry = (pmax.y - pmin.y) * 0.18f; int seg = 12, idx = 0;
							for ( int i = 0; i <= seg; ++i )
							{
								float a = IM_PI * 1.5f + (float)i / seg * IM_PI * 0.5f; pts_stack[idx++] = ImVec2( pmax.x - rx + cosf( a ) * rx, pmin.y + ry + sinf( a ) * ry );
							}
							for ( int i = 0; i <= seg; ++i )
							{
								float a = (float)i / seg * IM_PI * 0.5f; pts_stack[idx++] = ImVec2( pmax.x - rx + cosf( a ) * rx, pmax.y - ry + sinf( a ) * ry );
							}
							for ( int i = 0; i <= seg; ++i )
							{
								float a = IM_PI * 0.5f + (float)i / seg * IM_PI * 0.5f; pts_stack[idx++] = ImVec2( pmin.x + rx + cosf( a ) * rx, pmax.y - ry + sinf( a ) * ry );
							}
							for ( int i = 0; i <= seg; ++i )
							{
								float a = IM_PI + (float)i / seg * IM_PI * 0.5f; pts_stack[idx++] = ImVec2( pmin.x + rx + cosf( a ) * rx, pmin.y + ry + sinf( a ) * ry );
							}
							pts_count = idx;
						}
						else if ( path_type == 4 )
						{
							ImVec2 c( (left + right) * 0.5f, (top + bottom) * 0.5f ); float r = ImMin( right - left, bottom - top ) * 0.35f; int N = 128; for ( int i = 0; i < N; ++i )
							{
								float a = 2.f * IM_PI * (float)i / (float)N; pts_stack[i] = ImVec2( c.x + cosf( a ) * r, c.y + sinf( a ) * r );
							} pts_count = N;
						}
						else if ( path_type == 5 )
						{
							ImVec2 c( (left + right) * 0.5f, (top + bottom) * 0.5f ); float sx = (right - left) * 0.35f, sy = (bottom - top) * 0.25f; int N = 140; for ( int i = 0; i < N; ++i )
							{
								float t = 2.f * IM_PI * (float)i / (float)(N - 1); pts_stack[i] = ImVec2( c.x + cosf( t ) * sx, c.y + sinf( t ) * cosf( t ) * sy );
							} pts_count = N;
						}
						else if ( path_type == 6 )
						{
							ImVec2 c( (left + right) * 0.5f, (top + bottom) * 0.5f ); float a = ImMin( right - left, bottom - top ) * 0.35f; int N = 220, k = 5; for ( int i = 0; i < N; ++i )
							{
								float th = 2.f * IM_PI * (float)i / (float)(N - 1); float r = a * cosf( k * th ); pts_stack[i] = ImVec2( c.x + r * cosf( th ), c.y + r * sinf( th ) );
							} pts_count = N;
						}
						else if ( path_type == 7 )
						{
							ImVec2 c( (left + right) * 0.5f, (top + bottom) * 0.5f ); float s = ImMin( right - left, bottom - top ) * 0.035f; int N = 160, idx = 0; for ( int i = 0; i < N; ++i )
							{
								float t = 2.f * IM_PI * (float)i / (float)(N - 1); pts_stack[idx++] = ImVec2( c.x + 16.f * s * sinf( t ) * sinf( t ) * sinf( t ), c.y - (13.f * cosf( t ) - 5.f * cosf( 2 * t ) - 2.f * cosf( 3 * t ) - cosf( 4 * t )) * s );
							} pts_count = N;
						}
						else if ( path_type == 8 )
						{
							int teeth = 12, idx = 0; float h0 = top + (bottom - top) * 0.25f, h1 = bottom - (bottom - top) * 0.25f; for ( int i = 0; i <= teeth; ++i )
							{
								pts_stack[idx++] = ImVec2( ImLerp( left, right, (float)i / (float)teeth ), (i % 2) == 0 ? h0 : h1 );
							} pts_count = teeth + 1;
						}
						else if ( path_type == 9 )
						{
							ImVec2 c( (left + right) * 0.5f, (top + bottom) * 0.5f ); float R = ImMin( right - left, bottom - top ) * 0.42f, r = R * 0.45f; int idx = 0; for ( int i = 0; i < 10; ++i )
							{
								float ang = -IM_PI * 0.5f + (float)i * (IM_PI / 5.f); pts_stack[idx++] = ImVec2( c.x + cosf( ang ) * ((i % 2) == 0 ? R : r), c.y + sinf( ang ) * ((i % 2) == 0 ? R : r) );
							} pts_count = 10;
						}
						else if ( path_type == 10 )
						{
							ImVec2 p0( left, (top + bottom) * 0.5f ), p1( left + (right - left) * 0.25f, top ), p2( left + (right - left) * 0.25f, bottom ), p3( left + (right - left) * 0.5f, (top + bottom) * 0.5f );
							ImVec2 q0 = p3, q1( left + (right - left) * 0.75f, bottom ), q2( left + (right - left) * 0.75f, top ), q3( right, (top + bottom) * 0.5f );
							int N = 32, idx = 0; for ( int i = 0; i < N; ++i )
							{
								float t = (float)i / (float)(N - 1), u = 1.f - t; pts_stack[idx++] = ImVec2( u * u * u * p0.x + 3 * u * u * t * p1.x + 3 * u * t * t * p2.x + t * t * t * p3.x, u * u * u * p0.y + 3 * u * u * t * p1.y + 3 * u * t * t * p2.y + t * t * t * p3.y );
							}
							for ( int i = 0; i < N; ++i )
							{
								float t = (float)i / (float)(N - 1), u = 1.f - t; pts_stack[idx++] = ImVec2( u * u * u * q0.x + 3 * u * u * t * q1.x + 3 * u * t * t * q2.x + t * t * t * q3.x, u * u * u * q0.y + 3 * u * u * t * q1.y + 3 * u * t * t * q2.y + t * t * t * q3.y );
							}
							pts_count = 2 * N;
						}

						static ImVec4 col_v4 = ImVec4( 1.0f, 0.784f, 0.157f, 1.0f );
						ImU32 col = ImGui::ColorConvertFloat4ToU32( col_v4 );
						if ( animate ) offset += ImGui::GetIO().DeltaTime * 50.0f;
						ImWidgets::DrawDashedPolylineAA( dl, pts, pts_count, col, thickness, dash_len, gap_len, offset, closed, (ImWidgetsCap_)cap_idx, (ImWidgetsJoin)join_idx, miter_limit );

						ImGui::SetCursorScreenPos( origin + ImVec2( 0, side + 6 ) );
						ImGui::SliderFloat( "Thickness##dashed", &thickness, 1.0f, 24.0f );
						ImGui::SliderFloat( "Dash##dashed", &dash_len, 1.0f, 100.0f );
						ImGui::SliderFloat( "Gap##dashed", &gap_len, 0.0f, 100.0f );
						ImGui::SliderFloat( "Offset##dashed", &offset, -200.0f, 200.0f );
						ImGui::Checkbox( "Animate Offset##dashed", &animate );
						ImGui::Checkbox( "Closed##dashed", &closed );
						ImGui::Combo( "Cap##dashed", &cap_idx, caps, IM_ARRAYSIZE( caps ) );
						ImGui::Combo( "Join##dashed", &join_idx, joins, IM_ARRAYSIZE( joins ) );
						ImGui::SliderFloat( "Miter Limit##dashed", &miter_limit, 1.0f, 12.0f, "%.2f" );
						ImGui::Combo( "Path##dashed", &path_type, paths, IM_ARRAYSIZE( paths ) );
						ImGui::ColorEdit4( "Color##dashed", &col_v4.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf );
						bool use_gpu = ImWidgets::GetDashedLinesUseGPU();
						if ( ImGui::Checkbox( "GPU Path##dashed", &use_gpu ) )
							ImWidgets::SetDashedLinesUseGPU( use_gpu );
						bool debug_joins = ImWidgets::GetDashedLinesDebugJoins();
						if ( ImGui::Checkbox( "Debug Joins (CPU)##dashed", &debug_joins ) )
							ImWidgets::SetDashedLinesDebugJoins( debug_joins );
					}
					DW_SsRecord( "Dashed_Polylines", _sy0, ImGui::GetCursorPos().y ); }

					ShowDrawSquircleDemo();
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Wavy / Zigzag / Scallop / Dashed-Zigzag" ) )
						{
							static float amp = 8.0f, per = 24.0f, dash = 6.0f, gap = 4.0f, thk = 1.5f;
							ImGui::SliderFloat( "Amplitude##WZ", &amp, 1.0f, 30.0f );
							ImGui::SliderFloat( "Period##WZ",    &per, 4.0f, 80.0f );
							ImGui::SliderFloat( "Dash##WZ",      &dash, 1.0f, 24.0f );
							ImGui::SliderFloat( "Gap##WZ",       &gap,  1.0f, 24.0f );
							ImGui::SliderFloat( "Thickness##WZ", &thk,  0.5f, 4.0f );
							ImDrawList* dl = ImGui::GetWindowDrawList();
							ImVec2 p = ImGui::GetCursorScreenPos();
							float W = ImGui::GetContentRegionAvail().x;
							float row_h = ImPlatform_LpToPx( 160.0f );
							ImGui::Dummy( ImVec2( W, row_h ) );
							float yA = p.y + row_h * 0.18f;
							float yB = p.y + row_h * 0.40f;
							float yC = p.y + row_h * 0.62f;
							float yD = p.y + row_h * 0.84f;
							ImU32 c = IM_COL32( 230, 230, 230, 255 );
							ImWidgets::DrawWavyLine        ( dl, ImVec2( p.x + 30, yA ), ImVec2( p.x + W - 30, yA ), amp, per, c, thk );
							ImWidgets::DrawZigzagLine      ( dl, ImVec2( p.x + 30, yB ), ImVec2( p.x + W - 30, yB ), amp, per, c, thk );
							ImWidgets::DrawScallopLine     ( dl, ImVec2( p.x + 30, yC ), ImVec2( p.x + W - 30, yC ), amp, per, c, thk );
							ImWidgets::DrawDashedZigzagLine( dl, ImVec2( p.x + 30, yD ), ImVec2( p.x + W - 30, yD ), amp, per, dash, gap, c, thk );
							dl->AddText( ImVec2( p.x + 4, yA - amp - 8 ), IM_COL32( 180, 180, 180, 255 ), "wavy" );
							dl->AddText( ImVec2( p.x + 4, yB - amp - 8 ), IM_COL32( 180, 180, 180, 255 ), "zigzag" );
							dl->AddText( ImVec2( p.x + 4, yC - amp - 8 ), IM_COL32( 180, 180, 180, 255 ), "scallop" );
							dl->AddText( ImVec2( p.x + 4, yD - amp - 8 ), IM_COL32( 180, 180, 180, 255 ), "dashed zig" );
						}
						DW_SsRecord( "Wavy_Zigzag_Scallop", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Drop-Shadow / Inner-Glow" ) )
						{
							static float ds_radius = 16.0f, ds_round = 6.0f;
							static ImVec2 ds_off( 4.0f, 6.0f );
							static ImVec4 ds_col( 0.0f, 0.0f, 0.0f, 0.7f );
							static ImVec4 ig_col( 1.0f, 0.85f, 0.30f, 0.8f );
							ImGui::SliderFloat( "Radius##DS",  &ds_radius, 0.0f, 60.0f );
							ImGui::SliderFloat( "Corner##DS",  &ds_round,  0.0f, 30.0f );
							ImGui::SliderFloat2( "Offset##DS", &ds_off.x, -30.0f, 30.0f );
							ImGui::ColorEdit4( "Shadow##DS",   &ds_col.x );
							ImGui::ColorEdit4( "Glow##DS",     &ig_col.x );
							ImDrawList* dl = ImGui::GetWindowDrawList();
							ImVec2 p = ImGui::GetCursorScreenPos();
							float W = ImGui::GetContentRegionAvail().x;
							float row_h = ImPlatform_LpToPx( 180.0f );
							ImGui::Dummy( ImVec2( W, row_h ) );
							dl->AddRectFilled( p, ImVec2( p.x + W, p.y + row_h ),
								IM_COL32( 220, 220, 225, 255 ) );
							ImRect r1( p.x + 60, p.y + 30, p.x + 220, p.y + row_h - 30 );
							ImWidgets::DrawDropShadowRect( dl, r1, ds_radius, ds_off,
								ImGui::ColorConvertFloat4ToU32( ds_col ), ds_round );
							dl->AddRectFilled( r1.Min, r1.Max, IM_COL32( 80, 130, 200, 255 ), ds_round );
							ImRect r2( p.x + 280, p.y + 30, p.x + 440, p.y + row_h - 30 );
							dl->AddRectFilled( r2.Min, r2.Max, IM_COL32( 35, 35, 45, 255 ), ds_round );
							ImWidgets::DrawInnerGlowRect( dl, r2, ds_radius,
								ImGui::ColorConvertFloat4ToU32( ig_col ), ds_round );
						}
						DW_SsRecord( "Drop_Shadow_Inner_Glow", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Grid Overlay" ) )
						{
							static float g_major = 40.0f;
							static int   g_minor = 4;
							static int   g_flags = (int)ImWidgets::ImWidgetsGridFlags_Default;
							static bool  g_dots  = false;
							ImGui::SliderFloat( "Major step##Grid", &g_major, 8.0f, 200.0f );
							ImGui::SliderInt  ( "Minor subdivs##Grid", &g_minor, 0, 10 );
							ImGui::CheckboxFlags( "Major", &g_flags, ImWidgets::ImWidgetsGridFlags_Major  ); ImGui::SameLine();
							ImGui::CheckboxFlags( "Minor", &g_flags, ImWidgets::ImWidgetsGridFlags_Minor  ); ImGui::SameLine();
							ImGui::CheckboxFlags( "Origin",&g_flags, ImWidgets::ImWidgetsGridFlags_Origin ); ImGui::SameLine();
							if ( ImGui::Checkbox( "Dots", &g_dots ) )
							{
								if ( g_dots ) g_flags |= ImWidgets::ImWidgetsGridFlags_Dots;
								else          g_flags &= ~ImWidgets::ImWidgetsGridFlags_Dots;
							}
							ImDrawList* dl = ImGui::GetWindowDrawList();
							ImVec2 p = ImGui::GetCursorScreenPos();
							float W = ImGui::GetContentRegionAvail().x;
							float row_h = ImPlatform_LpToPx( 220.0f );
							ImGui::Dummy( ImVec2( W, row_h ) );
							dl->AddRectFilled( p, ImVec2( p.x + W, p.y + row_h ),
								IM_COL32( 30, 30, 38, 255 ) );
							ImWidgets::DrawGridOverlay( dl,
								ImRect( p, ImVec2( p.x + W, p.y + row_h ) ),
								ImVec2( p.x + W * 0.5f, p.y + row_h * 0.5f ),
								g_major, g_minor,
								IM_COL32( 130, 130, 130, 220 ),
								IM_COL32(  70,  70,  70, 160 ),
								IM_COL32( 230, 220, 120, 230 ),
								(ImWidgets::ImWidgetsGridFlags)g_flags );
						}
						DW_SsRecord( "Grid_Overlay", _sy0, ImGui::GetCursorPos().y );
					}
					EndCullSection( s_cull_cshader_h, s_cull_cshader_y );
				}
				ImGui::TreePop();
			}
#endif
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Gradients##Draw" ) )
			{
				static float s_cull_gradients_h = 0; float s_cull_gradients_y;
				if ( BeginCullSection( s_cull_gradients_h, s_cull_gradients_y ) )
				{
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
							ImGui::SliderInt( "Tess##LinearGrad", &tess, 0, 16 );
#endif
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							Slider2DFloat( "uv0##LinearGrad", &uv_start.x, &uv_start.y, 0.0f, 1.0f, -1.0f, 2.0f );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							Slider2DFloat( "uv1##LinearGrad", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
							ImGui::PopItemWidth();
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							if ( ImGui::ColorEdit4( "ColA##DrawShape##LinearGrad", &cola_v.x ) )
								cola = ImGui::GetColorU32( cola_v );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit4( "ColB##DrawShape##LinearGrad", &colb_v.x ) )
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
						DW_SsRecord( "Linear_Gradient", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
							ImGui::SliderInt( "Tess##RadialGrad", &tess, 0, 16 );
#endif
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							Slider2DFloat( "uv0##RadialGrad", &uv_start.x, &uv_start.y, 0.0f, 1.0f, -1.0f, 2.0f );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							Slider2DFloat( "uv1##RadialGrad", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
							ImGui::PopItemWidth();
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							if ( ImGui::ColorEdit4( "ColA##DrawShape##RadialGrad", &cola_v.x ) )
								cola = ImGui::GetColorU32( cola_v );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit4( "ColB##DrawShape##RadialGrad", &colb_v.x ) )
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
						DW_SsRecord( "Radial_Gradient", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
							ImGui::SliderInt( "Tess##DiamondGrad", &tess, 0, 16 );
#endif
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							Slider2DFloat( "uv0##DiamondGrad", &uv_start.x, &uv_start.y, 0.0f, 1.0f, -1.0f, 2.0f );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							Slider2DFloat( "uv1##DiamondGrad", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
							ImGui::PopItemWidth();
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							if ( ImGui::ColorEdit4( "ColA##DrawShape##DiamondGrad", &cola_v.x ) )
								cola = ImGui::GetColorU32( cola_v );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit4( "ColB##DrawShape##DiamondGrad", &colb_v.x ) )
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
						DW_SsRecord( "Diamond_Gradient", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Conic Gradient" ) )
						{
							static ImGradientData s_conic_grad;
							ImGui::Text( "Uses existing ImGradientData stops." );
							ImDrawList* dl = ImGui::GetWindowDrawList();
							ImVec2 p = ImGui::GetCursorScreenPos();
							ImVec2 cg_sz = ImPlatform_LpToPx( ImVec2( 260, 260 ) );
							ImGui::Dummy( cg_sz );
							ImWidgets::DrawConicGradient( dl, ImVec2( p.x + cg_sz.x * 0.5f, p.y + cg_sz.y * 0.5f ),
														  ImPlatform_LpToPx( 120.0f ), s_conic_grad, 0.0f, 192 );
						}
						DW_SsRecord( "Conic_Gradient", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
							static float angle_min = IM_PI / 6.0f;
							static float angle_max = 11.0f * IM_PI / 6.0f;
							static float radius = size * 0.5f;
							static int division = 12;
#ifdef DEAR_WIDGETS_TESSELATION
							static int tess = 0;
							ImGui::SliderInt( "Tess##ConicGrad", &tess, 0, 16 );
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
						DW_SsRecord( "Image_Shape", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
							ImGui::SliderInt( "Tess##ImageShapeGrad", &tess, 0, 16 );
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
							Slider2DFloat( "uv0##ImageShapeGrad", &uv_start.x, &uv_start.y, 0.0f, 1.0f, -1.0f, 2.0f );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							Slider2DFloat( "uv1##ImageShapeGrad", &uv_end.x, &uv_end.y, 0.0f, 1.0f, -1.0f, 2.0f );
							ImGui::PopItemWidth();
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							if ( ImGui::ColorEdit4( "ColA##DrawShape##ImageShapeGrad", &cola_v.x ) )
								cola = ImGui::GetColorU32( cola_v );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit4( "ColB##DrawShape##ImageShapeGrad", &colb_v.x ) )
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
						DW_SsRecord( "Image_Shape_Gradient", _sy0, ImGui::GetCursorPos().y );
					}
					EndCullSection( s_cull_gradients_h, s_cull_gradients_y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Pointers##Draw" ) )
			{
				static float s_cull_pointers_h = 0; float s_cull_pointers_y;
				if ( BeginCullSection( s_cull_pointers_h, s_cull_pointers_y ) )
				{
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
						DW_SsRecord( "Triangles_Pointers", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
						DW_SsRecord( "Signet_Pointer", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Arrows" ) )
						{
							const float S = ImPlatform_GetDpiScale();
							float const width = ImGui::GetContentRegionAvail().x;

							static float angle     = 0.4f;
							static float length    = 140.0f;
							static float thickness = 1.5f;
							static float head_size = 14.0f;
							static int   head_end   = ImWidgets::ImWidgetsArrowHead_Triangle;
							static int   head_start = ImWidgets::ImWidgetsArrowHead_None;
							ImGui::SliderAngle( "Angle##Arrow", &angle, -180.0f, 180.0f );
							ImGui::SliderFloat( "Length##Arrow",    &length,    20.0f, 320.0f );
							ImGui::SliderFloat( "Thickness##Arrow", &thickness, 0.5f, 6.0f );
							ImGui::SliderFloat( "Head Size##Arrow", &head_size, 2.0f, 32.0f );
							char const* heads[] = {
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_None ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Triangle ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Open ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Diamond ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Stealth ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Tick ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Dot ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Square ),
							};
							ImGui::Combo( "Head End##Arrow",   &head_end,   heads, IM_ARRAYSIZE( heads ) );
							ImGui::Combo( "Head Start##Arrow", &head_start, heads, IM_ARRAYSIZE( heads ) );

							ImVec2 curPos = ImGui::GetCursorScreenPos();
							ImDrawList* dl = ImGui::GetWindowDrawList();
							float rowH = 96.0f * S;
							ImGui::InvisibleButton( "##ArrowZone0", ImVec2( width, rowH ), 0 );
							ImGui::InvisibleButton( "##ArrowZone1", ImVec2( width, rowH ), 0 );

							// Row 1: each head style as the end head, identical shaft for comparison.
							{
								float y = curPos.y + rowH * 0.5f;
								float colW = width / 8.0f;
								for ( int i = 0; i < 8; ++i )
								{
									ImVec2 from( curPos.x + i * colW + 12.0f, y );
									ImVec2 to  ( curPos.x + ( i + 1 ) * colW - 12.0f, y );
									ImWidgets::DrawArrow( dl, from, to,
										IM_COL32( 200, 220, 240, 255 ), thickness,
										( ImWidgets::ImWidgetsArrowHead )i,
										ImWidgets::ImWidgetsArrowHead_None,
										head_size );
									ImVec2 tsz = ImGui::CalcTextSize( heads[ i ] );
									dl->AddText( ImVec2( ( from.x + to.x ) * 0.5f - tsz.x * 0.5f, y + 10.0f ),
										IM_COL32( 160, 180, 200, 255 ), heads[ i ] );
								}
							}

							// Row 2: axis arrows + configurable arrow + double-ended.
							{
								float yc = curPos.y + rowH + rowH * 0.5f;
								ImVec2 origin( curPos.x + 64.0f, yc );
								ImWidgets::DrawAxisArrows( dl, origin, 96.0f, -56.0f,
									IM_COL32( 230, 80, 80, 255 ), IM_COL32( 80, 200, 80, 255 ),
									thickness, head_size );

								ImVec2 from( curPos.x + 240.0f, yc );
								ImVec2 to(   from.x + length * ImCos( angle ),
								             from.y + length * ImSin( angle ) );
								ImWidgets::DrawArrow( dl, from, to,
									IM_COL32( 255, 220, 120, 255 ), thickness,
									( ImWidgets::ImWidgetsArrowHead )head_end,
									( ImWidgets::ImWidgetsArrowHead )head_start,
									head_size );
								dl->AddCircleFilled( from, 3.0f * S, IM_COL32( 255, 128, 0, 255 ) );
								dl->AddCircleFilled( to,   3.0f * S, IM_COL32( 255, 128, 0, 255 ) );

								ImVec2 c2( curPos.x + width - 200.0f, yc );
								ImWidgets::DrawArrow( dl, c2, c2 + ImVec2( 160.0f, 0.0f ),
									IM_COL32( 120, 200, 255, 255 ), thickness,
									ImWidgets::ImWidgetsArrowHead_Triangle,
									ImWidgets::ImWidgetsArrowHead_Triangle,
									head_size );
								dl->AddText( ImVec2( c2.x, c2.y - 22.0f ),
									IM_COL32( 160, 200, 230, 255 ), "Double-ended" );
							}
						}
						DW_SsRecord( "Arrows", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Dimension Line" ) )
						{
							const float S = ImPlatform_GetDpiScale();
							float const width = ImGui::GetContentRegionAvail().x;

							static float angle      = 0.35f;
							static float dist       = 220.0f;
							static float offset     = 30.0f;
							static float head_size  = 9.0f;
							static float ext_over   = 4.0f;
							static float ext_gap    = 2.0f;
							static int   orient     = ImWidgets::ImWidgetsDimensionTextOrient_FollowReading;
							static bool  ext_lines  = true;
							static bool  heads_in   = false;
							static bool  text_above = true;
							static bool  text_below = false;
							static bool  no_break   = false;
							static int   head_style = ImWidgets::ImWidgetsArrowHead_Triangle;
							ImGui::SliderAngle( "Angle##Dim", &angle, -180.0f, 180.0f );
							ImGui::SliderFloat( "Length##Dim",         &dist,      40.0f, 400.0f );
							ImGui::SliderFloat( "Offset##Dim",         &offset,   -80.0f, 80.0f );
							ImGui::SliderFloat( "Head Size##Dim",      &head_size, 2.0f, 24.0f );
							ImGui::SliderFloat( "Ext Overshoot##Dim",  &ext_over,  0.0f, 16.0f );
							ImGui::SliderFloat( "Ext Gap##Dim",        &ext_gap,   0.0f, 16.0f );
							char const* orients[] = {
								ImWidgets::GetDimensionTextOrientName( ImWidgets::ImWidgetsDimensionTextOrient_FollowLine ),
								ImWidgets::GetDimensionTextOrientName( ImWidgets::ImWidgetsDimensionTextOrient_AlwaysHorizontal ),
								ImWidgets::GetDimensionTextOrientName( ImWidgets::ImWidgetsDimensionTextOrient_FollowReading ),
								ImWidgets::GetDimensionTextOrientName( ImWidgets::ImWidgetsDimensionTextOrient_Perpendicular ),
							};
							ImGui::Combo( "Text Orient##Dim", &orient, orients, IM_ARRAYSIZE( orients ) );
							char const* dim_heads[] = {
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_None ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Triangle ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Open ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Diamond ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Stealth ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Tick ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Dot ),
								ImWidgets::GetArrowHeadName( ImWidgets::ImWidgetsArrowHead_Square ),
							};
							ImGui::Combo( "Head##Dim", &head_style, dim_heads, IM_ARRAYSIZE( dim_heads ) );
							ImGui::Checkbox( "Extension Lines##Dim", &ext_lines );  ImGui::SameLine();
							ImGui::Checkbox( "Heads Inside##Dim",    &heads_in );   ImGui::SameLine();
							ImGui::Checkbox( "Text Above##Dim",      &text_above ); ImGui::SameLine();
							ImGui::Checkbox( "Text Below##Dim",      &text_below ); ImGui::SameLine();
							ImGui::Checkbox( "No Break##Dim",        &no_break );

							int flags = ImWidgets::ImWidgetsDimensionFlags_None;
							if ( ext_lines  ) flags |= ImWidgets::ImWidgetsDimensionFlags_ExtensionLines;
							if ( heads_in   ) flags |= ImWidgets::ImWidgetsDimensionFlags_HeadsInside;
							if ( text_above ) flags |= ImWidgets::ImWidgetsDimensionFlags_TextAbove;
							if ( text_below ) flags |= ImWidgets::ImWidgetsDimensionFlags_TextBelow;
							if ( no_break   ) flags |= ImWidgets::ImWidgetsDimensionFlags_NoBreak;

							ImVec2 curPos = ImGui::GetCursorScreenPos();
							ImDrawList* dl = ImGui::GetWindowDrawList();
							float zoneH = 220.0f * S;
							ImGui::InvisibleButton( "##DimZone", ImVec2( width, zoneH ), 0 );

							ImVec2 mid( curPos.x + width * 0.5f, curPos.y + zoneH * 0.5f );
							ImVec2 from( mid.x - 0.5f * dist * ImCos( angle ),
							             mid.y - 0.5f * dist * ImSin( angle ) );
							ImVec2 to  ( mid.x + 0.5f * dist * ImCos( angle ),
							             mid.y + 0.5f * dist * ImSin( angle ) );

							// Show the measured segment in a faint guide color.
							dl->AddLine( from, to, IM_COL32( 120, 140, 170, 180 ), 1.0f );
							dl->AddCircleFilled( from, 3.0f, IM_COL32( 255, 128, 0, 255 ) );
							dl->AddCircleFilled( to,   3.0f, IM_COL32( 255, 128, 0, 255 ) );

							char label[ 32 ];
							ImFormatString( label, sizeof( label ), "%.1f px", dist );
							ImWidgets::DrawDimensionLine( dl, from, to, offset, label,
								IM_COL32( 220, 230, 240, 255 ),
								IM_COL32( 240, 240, 240, 255 ),
								1.0f,
								( ImWidgets::ImWidgetsArrowHead )head_style,
								head_size, ext_over, ext_gap,
								( ImWidgets::ImWidgetsDimensionTextOrient )orient,
								( ImWidgets::ImWidgetsDimensionFlags )flags );
						}
						DW_SsRecord( "Dimension_Line", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Bracket / Curly Brace" ) )
						{
							static int   br_style = (int)ImWidgets::ImWidgetsBracketStyle_Curly;
							static float br_depth = 16.0f;
							static float br_thick = 1.5f;
							char const* names[] = { "Square", "Curly", "Round" };
							ImGui::Combo( "Style##Bracket", &br_style, names, IM_ARRAYSIZE( names ) );
							ImGui::SliderFloat( "Depth##Bracket",     &br_depth, 4.0f, 60.0f );
							ImGui::SliderFloat( "Thickness##Bracket", &br_thick, 0.5f, 4.0f );
							ImDrawList* dl = ImGui::GetWindowDrawList();
							ImVec2 p = ImGui::GetCursorScreenPos();
							float W = ImGui::GetContentRegionAvail().x;
							float row_h = ImPlatform_LpToPx( 90.0f );
							ImGui::Dummy( ImVec2( W, row_h ) );
							ImWidgets::DrawBracket( dl, ImVec2( p.x + 60, p.y + 12 ),
								ImVec2( p.x + 60, p.y + row_h - 12 ),
								br_depth, (ImWidgets::ImWidgetsBracketStyle)br_style,
								IM_COL32( 230, 230, 230, 255 ), br_thick );
							ImWidgets::DrawBracket( dl, ImVec2( p.x + 220, p.y + 12 ),
								ImVec2( p.x + 220, p.y + row_h - 12 ),
								-br_depth, (ImWidgets::ImWidgetsBracketStyle)br_style,
								IM_COL32( 230, 230, 230, 255 ), br_thick );
							ImWidgets::DrawBracket( dl,
								ImVec2( p.x + 280, p.y + row_h * 0.5f ),
								ImVec2( p.x + 460, p.y + row_h * 0.5f ),
								-br_depth, (ImWidgets::ImWidgetsBracketStyle)br_style,
								IM_COL32( 160, 200, 230, 255 ), br_thick );
							dl->AddText( ImVec2( p.x + 80, p.y + row_h * 0.5f - 8 ),
								IM_COL32( 200, 200, 200, 255 ), "label" );
							dl->AddText( ImVec2( p.x + 240, p.y + row_h * 0.5f - 8 ),
								IM_COL32( 200, 200, 200, 255 ), "rhs" );
							dl->AddText( ImVec2( p.x + 340, p.y + row_h * 0.5f - 24 ),
								IM_COL32( 200, 200, 200, 255 ), "group of items" );
						}
						DW_SsRecord( "Bracket", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Crosshair / Reticle" ) )
						{
							static float cs_radius = 24.0f;
							static float cs_gap    = 6.0f;
							static float cs_thick  = 1.5f;
							ImGui::SliderFloat( "Radius##CS",    &cs_radius, 6.0f, 80.0f );
							ImGui::SliderFloat( "Gap##CS",       &cs_gap,    0.0f, cs_radius * 0.9f );
							ImGui::SliderFloat( "Thickness##CS", &cs_thick,  0.5f, 4.0f );
							ImDrawList* dl = ImGui::GetWindowDrawList();
							ImVec2 p = ImGui::GetCursorScreenPos();
							float row_h = ImPlatform_LpToPx( 110.0f );
							ImGui::Dummy( ImVec2( ImGui::GetContentRegionAvail().x, row_h ) );
							float step = ImPlatform_LpToPx( 110.0f );
							for ( int i = 0; i < ImWidgets::ImWidgetsCrosshairStyle_COUNT; ++i )
							{
								ImVec2 c( p.x + 55 + i * step, p.y + row_h * 0.5f );
								ImWidgets::DrawCrosshair( dl, c, cs_radius,
									(ImWidgets::ImWidgetsCrosshairStyle)i,
									IM_COL32( 230, 230, 230, 255 ), cs_thick, cs_gap );
								dl->AddText( ImVec2( c.x - 32, c.y + cs_radius + 6 ),
									IM_COL32( 200, 200, 200, 255 ),
									ImWidgets::GetCrosshairStyleName( (ImWidgets::ImWidgetsCrosshairStyle)i ) );
							}
						}
						DW_SsRecord( "Crosshair", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Pin / Map Marker" ) )
						{
							static float  pin_h    = 50.0f, pin_r = 16.0f;
							static float  pin_hole = 5.0f;
							static ImVec4 pin_fill( 0.95f, 0.30f, 0.30f, 1.0f );
							ImGui::SliderFloat( "Height##Pin", &pin_h, 20.0f, 100.0f );
							ImGui::SliderFloat( "Head r##Pin", &pin_r,  8.0f,  40.0f );
							ImGui::SliderFloat( "Hole r##Pin", &pin_hole, 0.0f, pin_r * 0.7f );
							ImGui::ColorEdit3 ( "Fill##Pin",   &pin_fill.x );
							ImDrawList* dl = ImGui::GetWindowDrawList();
							ImVec2 p = ImGui::GetCursorScreenPos();
							float W = ImGui::GetContentRegionAvail().x;
							float row_h = ImPlatform_LpToPx( 130.0f );
							ImGui::Dummy( ImVec2( W, row_h ) );
							dl->AddRectFilled( p, ImVec2( p.x + W, p.y + row_h ),
								IM_COL32( 40, 50, 65, 255 ) );
							ImU32 fill = ImGui::ColorConvertFloat4ToU32( pin_fill );
							ImU32 stroke = IM_COL32( 20, 20, 20, 220 );
							ImU32 hole = IM_COL32( 240, 240, 240, 255 );
							ImWidgets::DrawMapPin( dl, ImVec2( p.x + 100, p.y + row_h - 8 ),
								pin_h, pin_r, fill, stroke, 1.5f, hole, pin_hole );
							ImWidgets::DrawMapPin( dl, ImVec2( p.x + 220, p.y + row_h - 8 ),
								pin_h * 0.7f, pin_r * 0.7f,
								IM_COL32( 80, 150, 220, 255 ), stroke, 1.0f, 0u, 0.0f );
							ImWidgets::DrawMapPin( dl, ImVec2( p.x + 340, p.y + row_h - 8 ),
								pin_h * 1.2f, pin_r * 1.2f,
								IM_COL32( 250, 200, 80, 255 ), 0u, 0.0f,
								IM_COL32( 30, 30, 30, 255 ), pin_hole * 1.5f );
						}
						DW_SsRecord( "Pin_Map_Marker", _sy0, ImGui::GetCursorPos().y );
					}
					EndCullSection( s_cull_pointers_h, s_cull_pointers_y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Color##Draw" ) )
			{
				static float s_cull_color_h = 0; float s_cull_color_y;
				if ( BeginCullSection( s_cull_color_h, s_cull_color_y ) )
				{
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Color Bands" ) )
						{
							static float col[4] = { 1, 0, 0, 1 };
							ImGui::ColorEdit4( "Color##ColorBand", col );
							float const width = ImGui::GetContentRegionAvail().x;
							static float height = 32.0f;
							static float gamma = 1.0f;
							ImGui::DragFloat( "Height##ColorBand", &height, 1.0f, 1.0f, 128.0f );
							ImGui::DragFloat( "Gamma##ColorBand", &gamma, 0.01f, 0.1f, 10.0f );
							static int division = 32;
							ImGui::DragInt( "Division##ColorBand", &division, 1, 1, 128 );

							ImGui::Text( "HueBand" );
							DrawHueBand( ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2( width, height ), division, col, col[3], gamma );
							ImGui::InvisibleButton( "Hue##ColorBand", ImVec2( width, height ), 0 );

							ImGui::Text( "LuminanceBand" );
							DrawLumianceBand( ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2( width, height ), division, ImVec4( col[0], col[1], col[2], col[3] ), gamma );
							ImGui::InvisibleButton( "Luminance##ColorBand", ImVec2( width, height ), 0 );

							ImGui::Text( "SaturationBand" );
							DrawSaturationBand( ImGui::GetWindowDrawList(), ImGui::GetCursorScreenPos(), ImVec2( width, height ), division, ImVec4( col[0], col[1], col[2], col[3] ), gamma );
							ImGui::InvisibleButton( "Saturation##ColorBand", ImVec2( width, height ), 0 );

#ifdef __cpp_lambdas 
							ImGui::Separator();
							ImGui::Text( "Custom Color Band" );
							static int frequency = 6;
							ImGui::SliderInt( "Frequency##ColorBand", &frequency, 1, 32 );
							static float alpha = 1.0f;
							ImGui::SliderFloat( "alpha##ColorBand", &alpha, 0.0f, 1.0f );
							float data[] = { (float)frequency, alpha };
							DrawProceduralColor1DBilinearHorizontal(
								ImGui::GetWindowDrawList(),
								[]( float t, void* pUserData ) -> ImU32{
									float fFrequency = ((float*)pUserData)[0];
									float fAlpha = ((float*)pUserData)[1];
									float r = ImSign( ImSin( fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 0.0f / fFrequency ) ) * 0.5f + 0.5f;
									float g = ImSign( ImSin( fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 2.0f / fFrequency ) ) * 0.5f + 0.5f;
									float b = ImSign( ImSin( fFrequency * 2.0f * IM_PI * t + 2.0f * IM_PI * 4.0f / fFrequency ) ) * 0.5f + 0.5f;

									return IM_COL32( r * 255, g * 255, b * 255, fAlpha * 255 );
								},
								&data[0],
								0.0f, 1.0f, ImGui::GetCursorScreenPos(), ImVec2( width, height ), division );
							ImGui::InvisibleButton( "Custom##ColorBand", ImVec2( width, height ), 0 );
#else
							// TODO add function pointer C-like
							// ImColor1DCallback
							// ImU32 CustomColorBand( float x, void* );
#endif
						}
						DW_SsRecord( "Color_Bands", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Tessellated Shape Color (ImWidgetsShape)" ) )
						{
							ImGui::TextWrapped( "Shape-based color bands and discs. Each shape is a tessellated "
								"ImWidgetsShape (vertices + triangles + UVs) generated by GenShape*, then its "
								"vertex colors are written by ShapeFillProceduralColor*, then DrawShape pushes it to "
								"the drawlist. Same machinery as text/gradient fills - just different topologies." );
							float const tsWidth = ImGui::GetContentRegionAvail().x;
							static int tsDivisions = 32;
							static int tsSectors = 64;
							static int tsRings = 12;
							static float tsBandH = 32.0f;
							ImGui::SliderInt( "Divisions / Sectors##TS", &tsDivisions, 2, 128 );
							ImGui::SliderInt( "Disc sectors##TS",  &tsSectors,  6, 256 );
							ImGui::SliderInt( "Disc rings##TS",    &tsRings,    1,  64 );
							ImGui::SliderFloat( "Band height##TS", &tsBandH, 8.0f, 128.0f );

							ImDrawList* dl = ImGui::GetWindowDrawList();
							float dummy[1] = { 1.0f };

							ImGui::Text( "Horizontal hue band (GenShapeHorizontalBand)" );
							{
								ImVec2 p = ImGui::GetCursorScreenPos();
								ImRect bb( p, p + ImVec2( tsWidth, tsBandH ) );
								DrawShapeProceduralColorHorizontalBand( dl, bb, tsDivisions,
									[]( float t, void* )->ImU32 {
										float r, g, b; ImGui::ColorConvertHSVtoRGB( t, 1.0f, 1.0f, r, g, b );
										return IM_COL32( (int)(r*255), (int)(g*255), (int)(b*255), 255 );
									}, dummy );
								ImGui::InvisibleButton( "ShapeHueBand##TS", ImVec2( tsWidth, tsBandH ) );
							}

							ImGui::Text( "Vertical brightness band (GenShapeVerticalBand)" );
							{
								ImVec2 p = ImGui::GetCursorScreenPos();
								ImRect bb( p, p + ImVec2( tsBandH * 2.0f, tsBandH * 4.0f ) );
								DrawShapeProceduralColorVerticalBand( dl, bb, tsDivisions,
									[]( float t, void* )->ImU32 {
										int g = (int)((1.0f - t) * 255.0f);
										return IM_COL32( g, g, g, 255 );
									}, dummy );
								ImGui::InvisibleButton( "ShapeVBand##TS", bb.GetSize() );
							}

							ImGui::Text( "Rect grid 2D color (GenShapeRectGrid)" );
							{
								ImVec2 p = ImGui::GetCursorScreenPos();
								float side = ImMin( tsWidth, 256.0f );
								ImRect bb( p, p + ImVec2( side, side ) );
								DrawShapeProceduralColorRectGrid( dl, bb, tsDivisions, tsDivisions,
									[]( float u, float v, void* )->ImU32 {
										float r, g, b; ImGui::ColorConvertHSVtoRGB( u, 1.0f - v, 1.0f, r, g, b );
										return IM_COL32( (int)(r*255), (int)(g*255), (int)(b*255), 255 );
									}, dummy );
								ImGui::InvisibleButton( "ShapeGrid##TS", bb.GetSize() );
							}

							ImGui::Text( "Disc with rings (GenShapeDiscRings) - HSV wheel" );
							{
								ImVec2 p = ImGui::GetCursorScreenPos();
								float side = ImMin( tsWidth, 256.0f );
								ImVec2 c = p + ImVec2( side * 0.5f, side * 0.5f );
								float radius = side * 0.5f - 4.0f;
								DrawShapeProceduralColorDiscRings( dl, c, radius, tsSectors, tsRings,
									[]( float u, float v, void* )->ImU32 {
										float r, g, b; ImGui::ColorConvertHSVtoRGB( u, v, 1.0f, r, g, b );
										return IM_COL32( (int)(r*255), (int)(g*255), (int)(b*255), 255 );
									}, dummy );
								ImGui::InvisibleButton( "ShapeDisc##TS", ImVec2( side, side ) );
							}

							ImGui::Text( "Annulus (1-ring rim, GenShapeAnnulus) - hue wheel" );
							{
								ImVec2 p = ImGui::GetCursorScreenPos();
								float side = ImMin( tsWidth, 256.0f );
								ImVec2 c = p + ImVec2( side * 0.5f, side * 0.5f );
								float rOut = side * 0.5f - 4.0f;
								float rIn = rOut * 0.65f;
								DrawShapeProceduralColorAnnulus( dl, c, rIn, rOut, tsSectors,
									[]( float t, void* )->ImU32 {
										float r, g, b; ImGui::ColorConvertHSVtoRGB( t, 1.0f, 1.0f, r, g, b );
										return IM_COL32( (int)(r*255), (int)(g*255), (int)(b*255), 255 );
									}, dummy );
								ImGui::InvisibleButton( "ShapeAnnulus##TS", ImVec2( side, side ) );
							}

							ImGui::Text( "Annulus with rings (GenShapeAnnulusRings) - hue × value rim" );
							{
								ImVec2 p = ImGui::GetCursorScreenPos();
								float side = ImMin( tsWidth, 256.0f );
								ImVec2 c = p + ImVec2( side * 0.5f, side * 0.5f );
								float rOut = side * 0.5f - 4.0f;
								float rIn = rOut * 0.45f;
								DrawShapeProceduralColorAnnulusRings( dl, c, rIn, rOut, tsSectors, tsRings,
									[]( float u, float v, void* )->ImU32 {
										float r, g, b; ImGui::ColorConvertHSVtoRGB( u, 1.0f, v, r, g, b );
										return IM_COL32( (int)(r*255), (int)(g*255), (int)(b*255), 255 );
									}, dummy );
								ImGui::InvisibleButton( "ShapeAnnulusRings##TS", ImVec2( side, side ) );
							}
						}
						DW_SsRecord( "Tessellated_Shape_Color", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Procedural Color Primitives" ) )
						{
							static float gamma = 1.0f;
							static float offset = 0.0f;
							static float alpha = 1.0f;
							ImGui::SliderFloat( "Gamma##PCP", &gamma, 0.1f, 4.0f );
							ImGui::SliderFloat( "Offset##PCP", &offset, 0.0f, 1.0f );
							ImGui::SliderFloat( "Alpha##PCP", &alpha, 0.0f, 1.0f );
							float hueData[] = { alpha, offset, gamma };
							auto hueFunc = []( float tt, void* pUserData ) -> ImU32{
								float a = ((float*)pUserData)[0];
								float off = ((float*)pUserData)[1];
								float gm = ((float*)pUserData)[2];
								float t = ImFmod( 1.0f + ImPow( tt, gm ) - off, 1.0f );
								float r, g, b;
								ImGui::ColorConvertHSVtoRGB( t, 1.0f, 1.0f, r, g, b );
								return IM_COL32( (int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), (int)(a * 255.0f) );
								};

							ImDrawList* dl = ImGui::GetWindowDrawList();
							ImVec2 p = ImGui::GetCursorScreenPos();

							// Sized to roughly 2x original (previous /2.5 pass overshot at
							// high DPI). Still routed through Lp so it scales with font size.
							const float SX = ImPlatform_LpPxScale() * 0.8f;
							// Horizontal
							ImWidgets::DrawProceduralColor1DBilinearHorizontal( dl, hueFunc, &hueData[0], 0.0f, 1.0f, p, ImVec2( 260.0f * SX, 32.0f * SX ), 64 );
							// Vertical
							ImWidgets::DrawProceduralColor1DBilinearVertical( dl, hueFunc, &hueData[0], 0.0f, 1.0f, ImVec2( p.x + 280.0f * SX, p.y ), ImVec2( 32.0f * SX, 160.0f * SX ), 48 );
							// Arc (half ring)
							ImWidgets::DrawProceduralColorArcBilinear( dl, ImVec2( p.x + 80.0f * SX, p.y + 200.0f * SX ), 40.0f * SX, 80.0f * SX, IM_PI, IM_PI, hueFunc, &hueData[0], 64, true );
							// Spline (zig-zag)
							ImVec2 pts[6] = {
								ImVec2( p.x + 200.0f * SX, p.y + 200.0f * SX ),
								ImVec2( p.x + 250.0f * SX, p.y + 250.0f * SX ),
								ImVec2( p.x + 300.0f * SX, p.y + 200.0f * SX ),
								ImVec2( p.x + 350.0f * SX, p.y + 250.0f * SX ),
								ImVec2( p.x + 400.0f * SX, p.y + 200.0f * SX ),
								ImVec2( p.x + 450.0f * SX, p.y + 250.0f * SX )
							};
							ImWidgets::DrawProceduralColorSplineBilinear( dl, pts, 6, 14.0f * SX, hueFunc, &hueData[0], /*96*/256, false );

							ImGui::Dummy( ImVec2( 480.0f * SX, 320.0f * SX ) );
						}
						DW_SsRecord( "Procedural_Color_Primitives", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
												   float fCenter = ((float*)pUserData)[0];
												   float fColorDotBound = ((float*)pUserData)[1];
												   float r, g, b;
												   ImGui::ColorConvertHSVtoRGB( t, 1.0f, 1.0f, r, g, b );

												   ImVec2 const v0( ImCos( t * 2.0f * IM_PI ), ImSin( t * 2.0f * IM_PI ) );
												   ImVec2 const v1( ImCos( fCenter * 2.0f * IM_PI ), ImSin( fCenter * 2.0f * IM_PI ) );

												   float const dot = ImDot( v0, v1 );
												   //float const angle = ImAcos( dot ) / IM_PI;// / width;

												   return IM_COL32( r * 255, g * 255, b * 255, (dot > fColorDotBound ? 1.0f : 0.0f) * 255 );
											   }, &data[0], division, colorOffset, false );
							}
							{
								ImGui::Text( "Custom" );
								ImVec2 curPos = ImGui::GetCursorScreenPos();
								ImGui::InvisibleButton( "##ZoneColorRing2", ImVec2( width, width ) * 0.5f, 0 );

								float fFreqValue = (float)frequency;
								DrawColorRing( pDrawList, curPos, ImVec2( width, width ) * 0.5f, thickness,
											   []( float t, void* pUserData ){
												   float fFreq = *((float*)pUserData);
												   float v = ImSign( ImCos( fFreq * 2.0f * IM_PI * t ) ) * 0.5f + 0.5f;

												   return IM_COL32( v * 255, v * 255, v * 255, 255 );
											   }, &fFreqValue, division, colorOffset, true );
							}
						}
						DW_SsRecord( "Color_Ring", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
						DW_SsRecord( "OkLab_Color_Quad", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Color2D" ) )
						{
							float const width = CanvasSize();
							ImDrawList* pDrawList = ImGui::GetWindowDrawList();

							float const fTime = static_cast<float>(ImGui::GetTime());

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
								float timeCopy = *((float*)pUserData);
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
						DW_SsRecord( "Color2D", _sy0, ImGui::GetCursorPos().y );
					}
					EndCullSection( s_cull_color_h, s_cull_color_y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Masked Shapes##Draw" ) )
			{
				static float s_cull_masked_h = 0; float s_cull_masked_y;
				if ( BeginCullSection( s_cull_masked_h, s_cull_masked_y ) )
				{
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Image Convex Shape" ) )
						{
							float const size = CanvasSize();
							ImDrawList* pDrawList = ImGui::GetWindowDrawList();
							static ImVec2 uv_offset( 0.0f, 0.0f );
							static ImVec2 uv_scale( 1.0f, 1.0f );
							ImGui::DragFloat2( "Offset##DrawImageConvexShape", &uv_offset[0], 0.001f, -3.0f, 3.0f );
							ImGui::DragFloat2( "Scale##DrawImageConvexShape", &uv_scale[0], 0.001f, -3.0f, 3.0f );
							ImVec2 pos = ImGui::GetCursorScreenPos();
							ImVector<ImVec2> disk;
							disk.resize( 32 );
							for ( int k = 0; k < 32; ++k )
							{
								float angle = ((float)k) * 2.0f * IM_PI / 32.0f;
								float cos0 = ImCos( angle );
								float sin0 = ImSin( angle );
								disk[k].x = pos.x + 0.5f * size + cos0 * size * 0.5f;
								disk[k].y = pos.y + 0.5f * size + sin0 * size * 0.5f;
							}
							DrawImageConvexShape( pDrawList, background, &disk[0], 32, IM_COL32( 255, 255, 255, 255 ), uv_offset, uv_scale );
							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Image_Convex_Shape", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Image Concave Shape" ) )
						{
							float const size = CanvasSize();
							ImDrawList* pDrawList = ImGui::GetWindowDrawList();
							static ImVec2 uv_offset( 0.0f, 0.0f );
							static ImVec2 uv_scale( 1.0f, 1.0f );
							ImGui::DragFloat2( "Offset##DrawImageConcaveShape", &uv_offset[0], 0.001f, -3.0f, 3.0f );
							ImGui::DragFloat2( "Scale##DrawImageConcaveShape", &uv_scale[0], 0.001f, -3.0f, 3.0f );
							ImVec2 pos = ImGui::GetCursorScreenPos();
							int sz = 8;
							ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.3f, 0.0f }, { 0.3f, 0.7f }, { 0.7f, 0.7f }, { 0.7f, 0.0f },
												   { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
							for ( int k = 0; k < sz; ++k )
							{
								ImVec2& v = pos_norms[k];
								v.x *= size;
								v.y *= size;
								v += pos;
							}
							DrawImageConcaveShape( pDrawList, background, &pos_norms[0], sz, IM_COL32( 255, 255, 255, 255 ), uv_offset, uv_scale );
							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Image_Concave_Shape", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
							DrawShapeWithHole( pDrawList, &pos_norms[0], 10, IM_COL32( 255 * col.x, 255 * col.y, 255 * col.z, 255 * col.w ), &bb, gap, strokeWidth );

							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Shape_with_Hole", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Image Shape With Hole" ) )
						{
							float const size = CanvasSize();
							ImDrawList* pDrawList = ImGui::GetWindowDrawList();
							static ImVec2 uv_offset( 0.0f, 0.0f );
							static ImVec2 uv_scale( 1.0f, 1.0f );
							static int gap = 3;
							static int strokeWidth = 3;
							ImGui::DragFloat2( "Offset##DrawImageShapeWithHole", &uv_offset[0], 0.001f, -3.0f, 3.0f );
							ImGui::DragFloat2( "Scale##DrawImageShapeWithHole", &uv_scale[0], 0.001f, -3.0f, 3.0f );
							ImGui::SliderInt( "Gap##DrawImageShapeWithHole", &gap, 1, 16 );
							ImGui::SliderInt( "Stroke Width##DrawImageShapeWithHole", &strokeWidth, 1, 16 );

							ImVec2 pos = ImGui::GetCursorScreenPos();

							// Outer polygon: CW square in screen space (y-down)
							const int hole_segs = 32;
							ImVector<ImVec2> pts;
							pts.resize( 5 + (hole_segs + 1) );
							pts[0] = ImVec2( pos.x, pos.y );
							pts[1] = ImVec2( pos.x + size, pos.y );
							pts[2] = ImVec2( pos.x + size, pos.y + size );
							pts[3] = ImVec2( pos.x, pos.y + size );
							pts[4] = ImVec2( pos.x, pos.y );       // close outer
							// Hole: CCW circle (counter-clockwise in screen space)
							float cx = pos.x + size * 0.5f;
							float cy = pos.y + size * 0.5f;
							float r = size * 0.3f;
							for ( int k = 0; k <= hole_segs; k++ )
							{
								float angle = 2.0f * IM_PI * k / hole_segs;
								pts[5 + k] = ImVec2( cx + r * ImCos( angle ), cy - r * ImSin( angle ) );
							}

							DrawImageShapeWithHole( pDrawList, background, pts.Data, pts.Size, IM_COL32( 255, 255, 255, 255 ), uv_offset, uv_scale, gap, strokeWidth );
							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Image_Shape_With_Hole", _sy0, ImGui::GetCursorPos().y );
					}
					EndCullSection( s_cull_masked_h, s_cull_masked_y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Chromaticity##Draw" ) )
			{
				static float s_cull_chroma_h = 0; float s_cull_chroma_y;
				if ( BeginCullSection( s_cull_chroma_h, s_cull_chroma_y ) )
				{
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Chromaticity Plot" ) )
						{
							ImDrawList* pDrawList = ImGui::GetWindowDrawList();
							float const size = CanvasSize();

							ImWidgetsStyle const& cpStyle = ImWidgets::GetStyle();
							int chromLinesampleCount = cpStyle.ChromaticityPlot_LineSamples;
							int resX = cpStyle.ChromaticityPlot_Resolution;
							int resY = cpStyle.ChromaticityPlot_Resolution;
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
							static ImVec4 borderColor = (ImVec4)ImColor( IM_COL32( 0, 0, 0, 255 ) );
							ImGui::ColorEdit4( "Border Color##Chromaticity", &borderColor.x );
							float borderThickness = ImPlatform_LpToPx( cpStyle.ChromaticityPlot_BorderThickness );

							ImVec2 pos = ImGui::GetCursorScreenPos();
							DrawChromaticityPlot( pDrawList,
												  curIllum,
												  curObserver,
												  curColorSpace,
												  chromLinesampleCount,
												  pos, ImVec2( size, size ),
												  resX, resY,
												  maskColor,
												  (float)waveMin, (float)waveMax,
												  vMin.x, vMax.x,
												  vMin.y, vMax.y,
												  showColorSpaceTriangle,
												  showWhitePoint,
												  showBorder,
												  ImGui::GetColorU32( borderColor ),
												  borderThickness );

							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Chromaticity_Plot", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
								ImU32 col = KelvinTemperatureTosRGBColors( ImLerp( 3000.0f, 8000.0f, (float)i / ((float)(samplesCount - 1)) ) );
								colors[i] = col;
							}
							ImU32 tempCol = KelvinTemperatureTosRGBColors( temp );

							static ImVec4 lineColor = (ImVec4)ImColor( IM_COL32( 0, 0, 0, 255 ) );
							ImGui::ColorEdit4( "Line Color##ChromaticityLines", &lineColor.x );

							ImVec2 pos = ImGui::GetCursorScreenPos();
							ImWidgetsStyle const& clpStyle = ImWidgets::GetStyle();
							int plotRes       = clpStyle.ChromaticityPlot_Resolution;
							int plotSamples   = clpStyle.ChromaticityPlot_LineSamples;
							float plotBorder  = ImPlatform_LpToPx( clpStyle.ChromaticityPlot_BorderThickness );
							float ptRadius    = ImPlatform_LpToPx( clpStyle.ChromaticityPoint_Radius );
							int ptSegments    = clpStyle.ChromaticityPoint_Segments;
							float lineThickness = ImPlatform_LpToPx( clpStyle.ChromaticityLine_Thickness );
							DrawChromaticityPlot( pDrawList,
												  ImWidgetsIlluminant_D55,
												  ImWidgetsObserver_CIE1964_10deg,
												  ImWidgetsColorSpace_sRGB,
												  plotSamples,
												  pos, ImVec2( size, size ),
												  plotRes, plotRes,
												  IM_COL32( 255, 255, 255, 255 ),
												  360.0f, 830.0f,
												  vMin.x, vMax.x,
												  vMin.y, vMax.y,
												  true,
												  true,
												  true,
												  IM_COL32( 0, 0, 0, 255 ),
												  plotBorder );
							DrawChromaticityLines( pDrawList,
												   pos,
												   ImVec2( size, size ),
												   &colors[0],
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
													IM_COL32( 255, 0, 0, 255 ), ptRadius, ptSegments );

							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Chromaticity_Line_Point", _sy0, ImGui::GetCursorPos().y );
					}
					EndCullSection( s_cull_chroma_h, s_cull_chroma_y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Graduation##Draw" ) )
			{
				static float s_cull_grad_h = 0; float s_cull_grad_y;
				if ( BeginCullSection( s_cull_grad_h, s_cull_grad_y ) )
				{
					{
						float _sy0 = ImGui::GetCursorPos().y;
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

							ImGui::DragInt3( "Divisions", &divisions[0], 1.0f, 1, 10 );
							ImGui::DragFloat3( "Heights", &heights[0], 1.0f, 1.0f, 128.0f );
							ImGui::DragFloat3( "Thicknesses", &thicknesses[0], 1.0f, 1.0f, 16.0f );
							ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
							ImGui::SliderAngle( "a0", &angles[0] ); ImGui::PopItemWidth(); ImGui::SameLine();
							ImGui::SliderAngle( "a1", &angles[1] ); ImGui::PopItemWidth(); ImGui::SameLine();
							ImGui::SliderAngle( "a2", &angles[2] ); ImGui::PopItemWidth();
							ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
							if ( ImGui::ColorEdit3( "c0", &colors[0].x ) )
								col0 = ImGui::GetColorU32( colors[0] );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit3( "c1", &colors[1].x ) )
								col1 = ImGui::GetColorU32( colors[1] );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit3( "c2", &colors[2].x ) )
								col2 = ImGui::GetColorU32( colors[2] );
							ImGui::PopItemWidth();

							float height = ImMax( heights[0], ImMax( heights[1], heights[2] ) );
							ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
							DrawLinearLineGraduation( pDrawList, pos, pos + ImVec2( size, 0.0f ),
													  mainLineThickness, mainCol,
													  divisions[0], heights[0], thicknesses[0], angles[0], col0,
													  divisions[1], heights[1], thicknesses[1], angles[1], col1,
													  divisions[2], heights[2], thicknesses[2], angles[2], col2 );
							ImGui::Dummy( ImVec2( size, height ) );
							DrawLinearLineGraduation( pDrawList, pos, pos + ImVec2( size, size ),
													  mainLineThickness, mainCol,
													  divisions[0], heights[0], thicknesses[0], angles[0], col0,
													  divisions[1], heights[1], thicknesses[1], angles[1], col1,
													  divisions[2], heights[2], thicknesses[2], angles[2], col2 );
							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Linear_Line_Graduation", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
							//static float radius = size * 0.5f - 2.0f * ImMax( height0, ImMax( height1, height2 ) );
							float radius = size * 0.5f;
							static int num_segments = 0;
							static ImVec4 colors[] = { ImGui::ColorConvertU32ToFloat4( col0 ), ImGui::ColorConvertU32ToFloat4( col1 ), ImGui::ColorConvertU32ToFloat4( col2 ) };

							ImGui::DragFloat( "Main Thickness", &mainLineThickness, 1.0f, 1.0f, 16.0f );
							ImVec4 vMainCol = ImGui::ColorConvertU32ToFloat4( mainCol );
							if ( ImGui::ColorEdit3( "Main", &vMainCol.x ) )
								mainCol = ImGui::GetColorU32( vMainCol );

							ImGui::DragInt3( "Divisions", &divisions[0], 1.0f, 1, 10 );
							ImGui::DragFloat3( "Heights", &heights[0], 1.0f, 1.0f, 128.0f );
							ImGui::DragFloat3( "Thicknesses", &thicknesses[0], 1.0f, 1.0f, 16.0f );
							ImGui::DragFloat( "Radius", &radius, 1.0f, 1.0f, size );
							ImGui::DragInt( "Segment", &num_segments, 1.0f, 0, 64 );
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							ImGui::SliderAngle( "start angle", &angles_bound[0], -360.0f, angles_bound[1] * 180.0f / IM_PI ); ImGui::PopItemWidth(); ImGui::SameLine();
							ImGui::SliderAngle( "end angle", &angles_bound[1], angles_bound[0] * 180.0f / IM_PI, 360.0f ); ImGui::PopItemWidth();
							ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
							ImGui::SliderAngle( "a0", &angles[0] ); ImGui::PopItemWidth(); ImGui::SameLine();
							ImGui::SliderAngle( "a1", &angles[1] ); ImGui::PopItemWidth(); ImGui::SameLine();
							ImGui::SliderAngle( "a2", &angles[2] ); ImGui::PopItemWidth();
							ImGui::PushMultiItemsWidths( 3, ImGui::CalcItemWidth() );
							if ( ImGui::ColorEdit3( "c0", &colors[0].x ) )
								col0 = ImGui::GetColorU32( colors[0] );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit3( "c1", &colors[1].x ) )
								col1 = ImGui::GetColorU32( colors[1] );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit3( "c2", &colors[2].x ) )
								col2 = ImGui::GetColorU32( colors[2] );
							ImGui::PopItemWidth();

							float height = ImMax( heights[0], ImMax( heights[1], heights[2] ) );
							ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
							DrawLinearCircularGraduation( pDrawList, pos + ImVec2( size * 0.5f, size * 0.5f ), radius, angles_bound[0], angles_bound[1], num_segments,
														  mainLineThickness, mainCol,
														  divisions[0], heights[0], thicknesses[0], angles[0], col0,
														  divisions[1], heights[1], thicknesses[1], angles[1], col1,
														  divisions[2], heights[2], thicknesses[2], angles[2], col2 );
							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Linear_Circular_Graduation", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
							static float angles[] = { angle0, angle1 };
							static ImVec4 colors[] = { ImGui::ColorConvertU32ToFloat4( col0 ), ImGui::ColorConvertU32ToFloat4( col1 ) };

							ImGui::DragFloat( "Main Thickness", &mainLineThickness, 1.0f, 1.0f, 16.0f );
							ImVec4 vMainCol = ImGui::ColorConvertU32ToFloat4( mainCol );
							if ( ImGui::ColorEdit3( "Main", &vMainCol.x ) )
								mainCol = ImGui::GetColorU32( vMainCol );

							ImGui::DragInt2( "Divisions", &divisions[0], 1.0f, 1, 20 );
							ImGui::DragFloat2( "Heights", &heights[0], 1.0f, 1.0f, 128.0f );
							ImGui::DragFloat2( "Thicknesses", &thicknesses[0], 1.0f, 1.0f, 16.0f );
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							ImGui::SliderAngle( "a0", &angles[0] ); ImGui::PopItemWidth(); ImGui::SameLine();
							ImGui::SliderAngle( "a1", &angles[1] ); ImGui::PopItemWidth(); ImGui::SameLine();
							ImGui::SliderAngle( "a2", &angles[2] );
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							if ( ImGui::ColorEdit3( "c0", &colors[0].x ) )
								col0 = ImGui::GetColorU32( colors[0] );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit3( "c1", &colors[1].x ) )
								col1 = ImGui::GetColorU32( colors[1] );
							ImGui::PopItemWidth();

							float height = ImMax( heights[0], ImMax( heights[1], heights[2] ) );
							ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
							DrawLogLineGraduation( pDrawList, pos, pos + ImVec2( size, 0.0f ),
												   mainLineThickness, mainCol,
												   divisions[0], heights[0], thicknesses[0], angles[0], col0,
												   divisions[1], heights[1], thicknesses[1], angles[1], col1 );
							ImGui::Dummy( ImVec2( size, height ) );
							DrawLogLineGraduation( pDrawList, pos, pos + ImVec2( size, size ),
												   mainLineThickness, mainCol,
												   divisions[0], heights[0], thicknesses[0], angles[0], col0,
												   divisions[1], heights[1], thicknesses[1], angles[1], col1 );
							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Log_Line_Graduation", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Log Circular Graduation" ) )
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

							ImGui::DragInt2( "Divisions", &divisions[0], 1.0f, 1, 20 );
							ImGui::DragFloat2( "Heights", &heights[0], 1.0f, 1.0f, 128.0f );
							ImGui::DragFloat2( "Thicknesses", &thicknesses[0], 1.0f, 1.0f, 16.0f );
							ImGui::DragFloat( "Radius", &radius, 1.0f, 1.0f, size );
							ImGui::DragInt( "Segment", &num_segments, 1.0f, 0, 64 );
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							ImGui::SliderAngle( "start angle", &angles_bound[0], -360.0f, angles_bound[1] * 180.0f / IM_PI ); ImGui::PopItemWidth(); ImGui::SameLine();
							ImGui::SliderAngle( "end angle", &angles_bound[1], angles_bound[0] * 180.0f / IM_PI, 360.0f ); ImGui::PopItemWidth();
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							ImGui::SliderAngle( "a0", &angles[0] ); ImGui::PopItemWidth(); ImGui::SameLine();
							ImGui::SliderAngle( "a1", &angles[1] ); ImGui::PopItemWidth();
							ImGui::PushMultiItemsWidths( 2, ImGui::CalcItemWidth() );
							if ( ImGui::ColorEdit3( "c0", &colors[0].x ) )
								col0 = ImGui::GetColorU32( colors[0] );
							ImGui::PopItemWidth();
							ImGui::SameLine();
							if ( ImGui::ColorEdit3( "c1", &colors[1].x ) )
								col1 = ImGui::GetColorU32( colors[1] );
							ImGui::PopItemWidth();

							float height = ImMax( heights[0], heights[1] );
							ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2( 0.0f, height );
							DrawLogCircularGraduation( pDrawList, pos + ImVec2( size * 0.5f, size * 0.5f ), radius, angles_bound[0], angles_bound[1], num_segments,
													   mainLineThickness, mainCol,
													   divisions[0], heights[0], thicknesses[0], angles[0], col0,
													   divisions[1], heights[1], thicknesses[1], angles[1], col1 );
							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Log_Circular_Graduation", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Rulers (Top / Bottom / Left / Right)" ) )
						{
							static float ruler_min  = 0.0f;
							static float ruler_max  = 1000.0f;
							static float ruler_step = 100.0f;
							static int   ruler_subs = 5;
							static bool  show_tracker = true;
							ImGui::SliderFloat( "Min##Ruler",   &ruler_min, -500.0f, 1500.0f );
							ImGui::SliderFloat( "Max##Ruler",   &ruler_max, -500.0f, 2500.0f );
							ImGui::SliderFloat( "Major##Ruler", &ruler_step, 10.0f, 500.0f );
							ImGui::SliderInt  ( "Minor subdivs##Ruler", &ruler_subs, 0, 10 );
							ImGui::Checkbox   ( "Mouse tracker##Ruler", &show_tracker );
							ImDrawList* dl = ImGui::GetWindowDrawList();
							ImVec2 p = ImGui::GetCursorScreenPos();
							float W = ImGui::GetContentRegionAvail().x;
							float content_h = ImPlatform_LpToPx( 240.0f );
							float ruler_w   = ImPlatform_LpToPx( 24.0f );
							float total_w = W;
							float total_h = content_h + ruler_w;
							ImGui::Dummy( ImVec2( total_w, total_h ) );
							ImRect content( p.x + ruler_w, p.y + ruler_w, p.x + total_w, p.y + total_h );
							dl->AddRectFilled( content.Min, content.Max, IM_COL32( 35, 40, 50, 255 ) );
							ImWidgets::DrawGridOverlay( dl, content,
								ImVec2( content.Min.x, content.Min.y ),
								ruler_step, ruler_subs,
								IM_COL32( 110, 115, 130, 200 ),
								IM_COL32(  60,  65,  80, 140 ),
								IM_COL32( 200, 220, 255, 220 ) );
							ImVec2 mouse = ImGui::GetIO().MousePos;
							bool inside = mouse.x >= content.Min.x && mouse.x <= content.Max.x
								       && mouse.y >= content.Min.y && mouse.y <= content.Max.y;
							float mwx = ruler_min + ( ruler_max - ruler_min ) * ( ( mouse.x - content.Min.x ) / content.GetWidth() );
							float mwy = ruler_min + ( ruler_max - ruler_min ) * ( ( mouse.y - content.Min.y ) / content.GetHeight() );
							float const* pmx = ( show_tracker && inside ) ? &mwx : NULL;
							float const* pmy = ( show_tracker && inside ) ? &mwy : NULL;
							ImWidgets::DrawRuler( dl,
								ImRect( content.Min.x, p.y, content.Max.x, content.Min.y ),
								ImWidgets::ImWidgetsRulerOrient_Top,
								ruler_min, ruler_max, ruler_step, ruler_subs, "%.0f",
								IM_COL32( 220, 225, 230, 255 ), IM_COL32( 240, 240, 240, 255 ),
								12.0f, 6.0f, 1.0f, pmx );
							ImWidgets::DrawRuler( dl,
								ImRect( p.x, content.Min.y, content.Min.x, content.Max.y ),
								ImWidgets::ImWidgetsRulerOrient_Left,
								ruler_min, ruler_max, ruler_step, ruler_subs, "%.0f",
								IM_COL32( 220, 225, 230, 255 ), IM_COL32( 240, 240, 240, 255 ),
								12.0f, 6.0f, 1.0f, pmy );
						}
						DW_SsRecord( "Rulers", _sy0, ImGui::GetCursorPos().y );
					}
					EndCullSection( s_cull_grad_h, s_cull_grad_y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Thick Line##Draw" ) )
			{
				// Full parameter playground for ImWidgets::DrawThickLine.
				static int   mode_idx       = (int)ImWidgetsThickLineMode_StrokedBezierPath;
				static bool  dashed         = false;
				static ImVec4 line_color    = ImVec4( 0.95f, 0.45f, 0.25f, 1.0f );
				static float thickness_lp   = 4.0f;
				static int   cap_idx        = (int)ImWidgetsCap_Butt;
				static int   join_idx       = (int)ImWidgetsJoin_Round;
				static float miter_limit    = 4.0f;
				static float dash_len_lp    = 12.0f;
				static float gap_len_lp     = 6.0f;
				static float dash_offset_lp = 0.0f;
				static float tolerance      = 0.25f;

				// Sample-curve generator
				static int   curve_idx      = 0;
				static int   sample_count   = 32;
				static float wave_amp_lp    = 60.0f;
				static float wave_freq      = 2.0f;

				char const* mode_names[] = {
					"AddPolyline", "PolylineAA", "StrokedPolyline", "StrokedBezierPath", "ImGuiBezier",
				};
				char const* cap_names[]  = { "None", "Butt", "Square", "Round", "TriangleOut", "TriangleIn" };
				char const* join_names[] = { "Round", "Mitter", "Bevel" };
				char const* curve_names[] = { "Sinusoid", "Lissajous", "Spiral", "Polyline kink" };

				if ( ImGui::BeginTable( "##TL_Params", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings ) )
				{
					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Mode" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::Combo( "##TL_mode", &mode_idx, mode_names, IM_ARRAYSIZE( mode_names ) );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Dashed" );
					ImGui::TableNextColumn(); ImGui::Checkbox( "##TL_dashed", &dashed );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Color" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::ColorEdit4( "##TL_color", &line_color.x, ImGuiColorEditFlags_AlphaBar );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Thickness (lp)" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::SliderFloat( "##TL_thick", &thickness_lp, 0.5f, 30.0f, "%.2f" );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Cap" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::Combo( "##TL_cap", &cap_idx, cap_names, IM_ARRAYSIZE( cap_names ) );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Join" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::Combo( "##TL_join", &join_idx, join_names, IM_ARRAYSIZE( join_names ) );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Miter limit" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::SliderFloat( "##TL_miter", &miter_limit, 1.0f, 16.0f, "%.2f" );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Dash length (lp)" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::SliderFloat( "##TL_dlen", &dash_len_lp, 0.5f, 80.0f, "%.2f" );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Gap length (lp)" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					// Negative gap = overlapping dashes (matches DrawDashedPolylineAA semantics).
					ImGui::SliderFloat( "##TL_glen", &gap_len_lp, -40.0f, 80.0f, "%.2f" );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Dash offset (lp)" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::SliderFloat( "##TL_doff", &dash_offset_lp, -80.0f, 80.0f, "%.2f" );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Tolerance (px)" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::SliderFloat( "##TL_tol", &tolerance, 0.05f, 4.0f, "%.3f" );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Curve" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::Combo( "##TL_curve", &curve_idx, curve_names, IM_ARRAYSIZE( curve_names ) );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Samples" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::SliderInt( "##TL_n", &sample_count, 2, 256 );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Wave amplitude (lp)" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::SliderFloat( "##TL_amp", &wave_amp_lp, 0.0f, 200.0f, "%.1f" );

					ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted( "Wave frequency" );
					ImGui::TableNextColumn(); ImGui::SetNextItemWidth( -FLT_MIN );
					ImGui::SliderFloat( "##TL_freq", &wave_freq, 0.5f, 8.0f, "%.2f" );
					ImGui::EndTable();
				}

				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 p = ImGui::GetCursorScreenPos();
				float canvas_w = ImMin( ImGui::GetContentRegionAvail().x, ImPlatform_LpToPx( 800.0f ) );
				float canvas_h = ImPlatform_LpToPx( 280.0f );
				ImGui::Dummy( ImVec2( canvas_w, canvas_h ) );
				dl->AddRect( p, ImVec2( p.x + canvas_w, p.y + canvas_h ), IM_COL32( 255, 255, 255, 60 ) );

				// Generate the input polyline (in screen coords, px).
				ImVector<ImVec2> pts;
				pts.reserve( sample_count );
				float const cx = p.x + canvas_w * 0.5f;
				float const cy = p.y + canvas_h * 0.5f;
				float const amp = ImPlatform_LpToPx( wave_amp_lp );
				switch ( curve_idx )
				{
				case 0: // Sinusoid: x linear, y sin
					for ( int i = 0; i < sample_count; ++i )
					{
						float t = (float)i / (float)( sample_count - 1 );
						float x = p.x + t * canvas_w;
						float y = cy + amp * ImSin( t * wave_freq * 2.0f * IM_PI );
						pts.push_back( ImVec2( x, y ) );
					}
					break;
				case 1: // Lissajous
				{
					float rx = canvas_w * 0.42f;
					float ry = amp;
					for ( int i = 0; i < sample_count; ++i )
					{
						float t = (float)i / (float)( sample_count - 1 );
						float u = t * 2.0f * IM_PI;
						pts.push_back( ImVec2( cx + rx * ImSin( wave_freq * u ),
											   cy + ry * ImSin( ( wave_freq + 1.0f ) * u + 1.2f ) ) );
					}
					break;
				}
				case 2: // Spiral
				{
					float r_max = ImMin( canvas_w * 0.4f, amp * 2.0f );
					if ( r_max < 4.0f ) r_max = canvas_h * 0.4f;
					for ( int i = 0; i < sample_count; ++i )
					{
						float t = (float)i / (float)( sample_count - 1 );
						float u = t * wave_freq * 2.0f * IM_PI;
						float r = r_max * t;
						pts.push_back( ImVec2( cx + r * ImCos( u ), cy + r * ImSin( u ) ) );
					}
					break;
				}
				case 3: // Polyline with sharp kinks (zig-zag)
				{
					int const n = ImMax( sample_count, 4 );
					for ( int i = 0; i < n; ++i )
					{
						float t = (float)i / (float)( n - 1 );
						float x = p.x + t * canvas_w;
						float y = cy + amp * ( ( i & 1 ) ? 1.0f : -1.0f );
						pts.push_back( ImVec2( x, y ) );
					}
					break;
				}
				}

				ImWidgetsThickLineDesc desc;
				desc.color       = ImGui::GetColorU32( line_color );
				desc.mode        = (ImWidgetsThickLineMode)mode_idx;
				desc.cap         = (ImWidgetsCap)cap_idx;
				desc.join        = (ImWidgetsJoin)join_idx;
				desc.thickness   = ImPlatform_LpToPx( thickness_lp );
				desc.miter_limit = miter_limit;
				desc.dash_len    = ImPlatform_LpToPx( dash_len_lp );
				desc.gap_len     = ImPlatform_LpToPx( gap_len_lp );
				desc.dash_offset = ImPlatform_LpToPx( dash_offset_lp );
				desc.tolerance   = tolerance;
				desc.dashed      = dashed;

				dl->PushClipRect( p, ImVec2( p.x + canvas_w, p.y + canvas_h ), true );
				ImWidgets::DrawThickLine( dl, pts.Data, pts.Size, desc );
				dl->PopClipRect();

				// Tiny readout: which mode is actually doing the rendering this frame.
				ImGui::Text( "%s%s, %d samples", ImWidgets::GetThickLineModeName( desc.mode ),
							 dashed ? " + dashed" : "", pts.Size );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Patterns" ) )
			{
				static int patt = (int)ImWidgetsHatchPattern_Cross;
				static float spacing = 10.0f, angle_deg = 0.0f, thickness = 1.0f;
				const char* patt_names[] = { "Parallel", "Cross", "Diagonal", "DiagonalCross", "Dots", "ConcentricRings", "BenDay", "Screentone" };
				ImGui::Combo( "Pattern", &patt, patt_names, IM_ARRAYSIZE( patt_names ) );
				ImGui::SliderFloat( "Spacing", &spacing, 2.0f, 40.0f );
				ImGui::SliderFloat( "Angle (deg)", &angle_deg, -180.0f, 180.0f );
				ImGui::SliderFloat( "Thickness", &thickness, 0.5f, 4.0f );
				ImGui::Spacing();
				// Reserve canvas area AFTER the controls.
				ImVec2 origin = ImGui::GetCursorScreenPos();
				ImVec2 canvas = ImPlatform_LpToPx( ImVec2( 360, 240 ) );
				ImGui::Dummy( canvas );
				ImVec2 star[10];
				float cx = origin.x + canvas.x * 0.5f, cy = origin.y + canvas.y * 0.5f;
				for ( int i = 0; i < 10; ++i )
				{
					float r = ImPlatform_LpToPx( (i & 1) ? 40.0f : 90.0f );
					float a = -IM_PI * 0.5f + i * IM_PI / 5.0f;
					star[i] = ImVec2( cx + r * ImCos( a ), cy + r * ImSin( a ) );
				}
				ImDrawList* dl = ImGui::GetWindowDrawList();
				dl->AddPolyline( star, 10, IM_COL32( 255, 255, 255, 120 ), ImDrawFlags_Closed, 1.0f );
				ImWidgets::DrawHatchFill( dl, star, 10,
										  (ImWidgetsHatchPattern)patt, ImPlatform_LpToPx( spacing ),
										  angle_deg * IM_PI / 180.0f, ImPlatform_LpToPx( thickness ),
										  IM_COL32( 255, 200, 100, 220 ) );

				ImGui::Separator();
				ImGui::Text( "Stipple" );
				static float density = 0.15f, jitter = 0.6f, radius = 1.2f;
				ImGui::SliderFloat( "Density", &density, 0.05f, 1.0f );
				ImGui::SliderFloat( "Jitter", &jitter, 0.0f, 1.0f );
				ImGui::SliderFloat( "Radius", &radius, 0.5f, 4.0f );
				ImGui::Spacing();
				ImVec2 o2 = ImGui::GetCursorScreenPos();
				ImVec2 stipple_sz = ImPlatform_LpToPx( ImVec2( 260, 200 ) );
				ImGui::Dummy( stipple_sz );
				ImVec2 quad[4] = {
					ImVec2( o2.x + stipple_sz.x * 0.04f, o2.y + stipple_sz.y * 0.05f ),
					ImVec2( o2.x + stipple_sz.x * 0.88f, o2.y + stipple_sz.y * 0.10f ),
					ImVec2( o2.x + stipple_sz.x * 0.96f, o2.y + stipple_sz.y * 0.90f ),
					ImVec2( o2.x + stipple_sz.x * 0.08f, o2.y + stipple_sz.y * 0.85f ),
				};
				dl->AddPolyline( quad, 4, IM_COL32( 255, 255, 255, 120 ), ImDrawFlags_Closed, 1.0f );
				ImWidgets::DrawStippleFill( dl, quad, 4, density, jitter, ImPlatform_LpToPx( radius ),
											IM_COL32( 120, 220, 255, 220 ) );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Iso-Contours" ) )
			{
				struct Fn
				{
					static float f( float x, float y, void* ud )
					{
						float* sc = (float*)ud;
						x *= sc[0]; y *= sc[0];
						return ImSin( x * 0.05f ) + ImCos( y * 0.05f );
					}
				};
				static float scale = 0.5f;
				ImGui::SliderFloat( "Scale", &scale, 0.25f, 4.0f );
				static float isos[] = { -0.8f, -0.4f, 0.0f, 0.4f, 0.8f };
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 iso_sz = ImPlatform_LpToPx( ImVec2( 360, 240 ) );
				ImGui::Dummy( iso_sz );
				dl->AddRect( p, ImVec2( p.x + iso_sz.x, p.y + iso_sz.y ), IM_COL32( 255, 255, 255, 120 ) );
				float ud[1] = { scale };
				ImWidgets::DrawIsoContour( dl, p, iso_sz, 48, 32,
										   Fn::f, ud, isos, IM_ARRAYSIZE( isos ),
										   IM_COL32( 120, 255, 160, 255 ), ImPlatform_LpToPx( 1.5f ) );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Superellipse" ) )
			{
				static float rx = 80, ry = 60, nx = 4, ny = 4;
				static int sides = 64;
				static bool show_fill = true;
				static bool show_edges = true;
				static bool show_verts = false;
				static bool show_tris = false;
				static int focus_tri = -1;
				static int tess_iter = 0;
				ImGui::SliderFloat( "rx", &rx, 10, 120 );
				ImGui::SliderFloat( "ry", &ry, 10, 120 );
				ImGui::SliderFloat( "nx", &nx, 0.5f, 10.0f );
				ImGui::SliderFloat( "ny", &ny, 0.5f, 10.0f );
				ImGui::SliderInt( "sides", &sides, 16, 256 );
				ImGui::SliderInt( "Tessellation iterations", &tess_iter, 0, 4 );
				ImGui::Checkbox( "Fill", &show_fill ); ImGui::SameLine();
				ImGui::Checkbox( "Edges", &show_edges ); ImGui::SameLine();
				ImGui::Checkbox( "Vertices", &show_verts ); ImGui::SameLine();
				ImGui::Checkbox( "Highlight triangle", &show_tris );
				if ( show_tris )
				{
					int max_tri = 0; // computed after GenShape below
					ImGui::SliderInt( "Triangle index", &focus_tri, -1, 1024 );
					IM_UNUSED( max_tri );
				}
				ImWidgetsShape sh;
				ImVec2 se_sz = ImPlatform_LpToPx( ImVec2( 360, 300 ) );
				ImWidgets::GenShapeSuperellipse( sh, ImVec2( se_sz.x * 0.5f, se_sz.y * 0.5f ),
												 ImPlatform_LpToPx( rx ), ImPlatform_LpToPx( ry ), nx, ny, sides );
				for ( int k = 0; k < tess_iter; ++k ) ImWidgets::ShapeTesselationUniform( sh );
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 o = ImGui::GetCursorScreenPos();
				ImGui::Dummy( se_sz );
				// Translate shape into canvas.
				ImWidgetsShape shd = sh;
				for ( int i = 0; i < shd.vertices.Size; ++i )
				{
					shd.vertices[i].pos.x += o.x;
					shd.vertices[i].pos.y += o.y;
				}
				if ( show_fill )
					ImWidgets::DrawShape( dl, shd );
				// NOTE: DrawShapeDebug re-fills the triangles with the shape's
				// own per-vertex colors. That is intentional (it matches the
				// Draw Shape / ImageShape demos), so with SuperEllipse's opaque
				// white vertex colors the shape looks filled even with Fill off
				// when any debug overlay is on. Acceptable.
				if ( show_edges || show_verts || show_tris )
					ImWidgets::DrawShapeDebug( dl, shd,
											   ImPlatform_LpToPx( show_edges ? 1.5f : 0.0f ),
											   IM_COL32( 255, 180, 80, 220 ),
											   show_tris ? IM_COL32( 120, 220, 255, 160 ) : 0u,
											   ImPlatform_LpToPx( show_verts ? 3.0f : 0.0f ),
											   IM_COL32( 255, 255, 255, 255 ),
											   show_tris ? focus_tri : -1 );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Coons / Gregory Patch" ) )
			{
				static int patch_kind = 0; // 0 = Coons, 1 = Gregory
				ImGui::Combo( "Patch type", &patch_kind, "Coons (4 cubic boundaries)\0Gregory (20 control points)\0" );
				static ImCoonsPatch P;
				static bool init = false;
				if ( !init )
				{
					init = true;
					ImVec2 o( 40, 40 );
					P.bottom[0] = ImVec2( o.x, o.y + 200 );
					P.bottom[1] = ImVec2( o.x + 80, o.y + 220 );
					P.bottom[2] = ImVec2( o.x + 180, o.y + 180 );
					P.bottom[3] = ImVec2( o.x + 260, o.y + 210 );
					P.top[0] = ImVec2( o.x, o.y );
					P.top[1] = ImVec2( o.x + 100, o.y - 20 );
					P.top[2] = ImVec2( o.x + 200, o.y + 20 );
					P.top[3] = ImVec2( o.x + 260, o.y );
					P.left[0] = P.bottom[0];
					P.left[1] = ImVec2( o.x - 10, o.y + 130 );
					P.left[2] = ImVec2( o.x + 20, o.y + 70 );
					P.left[3] = P.top[0];
					P.right[0] = P.bottom[3];
					P.right[1] = ImVec2( o.x + 280, o.y + 140 );
					P.right[2] = ImVec2( o.x + 240, o.y + 90 );
					P.right[3] = P.top[3];
				}
				static int resU = 16, resV = 16;
				ImGui::SliderInt( "resU", &resU, 2, 48 );
				ImGui::SliderInt( "resV", &resV, 2, 48 );
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 origin2 = ImGui::GetCursorScreenPos();
				ImVec2 coons_sz = ImPlatform_LpToPx( ImVec2( 360, 260 ) );
				ImGui::Dummy( coons_sz );
				float dpi = ImPlatform_LpPxScale();
				ImCoonsPatch Pd = P;
				for ( int i = 0; i < 4; ++i )
				{
					Pd.bottom[i].x = origin2.x + P.bottom[i].x * dpi; Pd.bottom[i].y = origin2.y + P.bottom[i].y * dpi;
					Pd.top[i].x = origin2.x + P.top[i].x * dpi; Pd.top[i].y = origin2.y + P.top[i].y * dpi;
					Pd.left[i].x = origin2.x + P.left[i].x * dpi; Pd.left[i].y = origin2.y + P.left[i].y * dpi;
					Pd.right[i].x = origin2.x + P.right[i].x * dpi; Pd.right[i].y = origin2.y + P.right[i].y * dpi;
				}
				if ( patch_kind == 0 )
				{
					ImWidgets::DrawCoonsPatchGradient( dl, Pd,
													   IM_COL32( 255, 80, 80, 255 ), IM_COL32( 80, 255, 80, 255 ),
													   IM_COL32( 80, 80, 255, 255 ), IM_COL32( 255, 255, 80, 255 ),
													   resU, resV );
					ImWidgets::DrawCoonsPatchWireframe( dl, Pd, IM_COL32( 255, 255, 255, 100 ),
														ImPlatform_LpToPx( 1.0f ), resU, resV );
				}
				else
				{
					// Build a Gregory patch from the Coons boundary: corner, two edge CPs
					// per edge, and two twist CPs per corner (20 total). Seed twists from
					// the nearest interior boundary tangent for a smooth default.
					ImGregoryPatch G;
					// Corners
					G.cp[0] = Pd.bottom[0]; G.cp[1] = Pd.bottom[3];
					G.cp[2] = Pd.top[3];    G.cp[3] = Pd.top[0];
					// Bottom edge tangent CPs (2)
					G.cp[4] = Pd.bottom[1]; G.cp[5] = Pd.bottom[2];
					// Right edge (v dir, u=1)
					G.cp[6] = Pd.right[1];  G.cp[7] = Pd.right[2];
					// Top edge (u dir, v=1, reversed direction)
					G.cp[8] = Pd.top[2];    G.cp[9] = Pd.top[1];
					// Left edge (v dir, u=0)
					G.cp[10] = Pd.left[2];  G.cp[11] = Pd.left[1];
					// Twist CPs -- two per corner. Use the average of adjacent edge CPs.
					auto twist = [ & ]( int a, int b, int c ){
						return ImVec2( (G.cp[a].x + G.cp[b].x + G.cp[c].x) / 3.0f,
									   (G.cp[a].y + G.cp[b].y + G.cp[c].y) / 3.0f );
						};
					G.cp[12] = twist( 0, 4, 11 ); G.cp[13] = twist( 0, 11, 4 );
					G.cp[14] = twist( 1, 5, 6 );  G.cp[15] = twist( 1, 6, 5 );
					G.cp[16] = twist( 2, 7, 8 );  G.cp[17] = twist( 2, 8, 7 );
					G.cp[18] = twist( 3, 9, 10 ); G.cp[19] = twist( 3, 10, 9 );
					ImWidgets::DrawGregoryPatchGradient( dl, G,
														 IM_COL32( 255, 80, 80, 255 ), IM_COL32( 80, 255, 80, 255 ),
														 IM_COL32( 80, 80, 255, 255 ), IM_COL32( 255, 255, 80, 255 ),
														 resU, resV );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Offset Path" ) )
			{
				static ImVec2 pts[] = {
					ImVec2( 40, 40 ), ImVec2( 180, 30 ), ImVec2( 260, 120 ),
					ImVec2( 220, 220 ), ImVec2( 80, 200 )
				};
				static float offset = 12.0f;
				static int join = (int)ImWidgetsJoin_Round;
				static float miter_limit = 4.0f;
				ImGui::SliderFloat( "Offset", &offset, -40.0f, 40.0f );
				const char* join_names[] = { "Round", "Mitter", "Bevel" };
				ImGui::Combo( "Join", &join, join_names, IM_ARRAYSIZE( join_names ) );
				ImGui::SliderFloat( "Miter limit", &miter_limit, 1.0f, 20.0f );
				ImGui::Spacing();
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 o = ImGui::GetCursorScreenPos();
				ImVec2 off_sz = ImPlatform_LpToPx( ImVec2( 360, 280 ) );
				ImGui::Dummy( off_sz );
				float off_dpi = ImPlatform_LpPxScale();
				ImVec2 off[5];
				for ( int i = 0; i < 5; ++i ) off[i] = ImVec2( o.x + pts[i].x * off_dpi, o.y + pts[i].y * off_dpi );
				dl->AddPolyline( off, 5, IM_COL32( 255, 255, 255, 200 ), ImDrawFlags_Closed, ImPlatform_LpToPx( 1.5f ) );
				ImWidgets::DrawOffsetOutline( dl, off, 5, ImPlatform_LpToPx( offset ), (ImWidgetsJoin)join,
											  miter_limit, ImPlatform_LpToPx( 1.5f ), IM_COL32( 255, 180, 80, 255 ), true );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "IsoContour Tiered (topographic)" ) )
			{
				struct Fn
				{
					static float f( float x, float y, void* ud )
					{
						float* sc = (float*)ud;
						return 50.0f + 40.0f * ImSin( x * 0.02f * sc[0] ) + 30.0f * ImCos( y * 0.015f * sc[0] )
							+ 10.0f * ImSin( (x + y) * 0.05f * sc[0] );
					}
				};
				static float scale = 1.0f;
				static bool pre_log = false;
				static ImIsoContourTier tiers[3] = {
					ImIsoContourTier( 2.0f,  IM_COL32( 180, 200, 220, 80 ), 0.5f, false ),
					ImIsoContourTier( 10.0f, IM_COL32( 200, 220, 240, 160 ), 1.0f, false ),
					ImIsoContourTier( 40.0f, IM_COL32( 255, 240, 200, 255 ), 2.0f, false ),
				};
				ImGui::SliderFloat( "Scale", &scale, 0.25f, 4.0f );
				ImGui::Checkbox( "Pre-log transform field", &pre_log );
				static int iso_resX = 80, iso_resY = 48;
				ImGui::SliderInt( "Base resX", &iso_resX, 8, 256 );
				ImGui::SliderInt( "Base resY", &iso_resY, 8, 256 );
				static bool iso_half_pixel = true;
				static int iso_subdiv = 1;
				ImGui::Checkbox( "Half-pixel sampling", &iso_half_pixel );
				ImGui::SliderInt( "Sub-sample (bilinear)", &iso_subdiv, 1, 4 );
				ImGui::SliderFloat( "Minor spacing", &tiers[0].spacing, 0.5f, 10.0f );
				ImGui::SliderFloat( "Medium spacing", &tiers[1].spacing, 1.0f, 40.0f );
				ImGui::SliderFloat( "Major spacing", &tiers[2].spacing, 10.0f, 100.0f );
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 iso_sz2 = ImPlatform_LpToPx( ImVec2( 460, 280 ) );
				ImGui::Dummy( iso_sz2 );
				dl->AddRect( p, ImVec2( p.x + iso_sz2.x, p.y + iso_sz2.y ), IM_COL32( 255, 255, 255, 100 ) );
				float ud[1] = { scale };
				ImIsoContourTier tiers_scaled[3] = {
					tiers[0], tiers[1], tiers[2]
				};
				for ( int i = 0; i < 3; ++i ) tiers_scaled[i].thickness = ImPlatform_LpToPx( tiers[i].thickness );
				ImWidgets::DrawIsoContourTiered( dl, p, iso_sz2, iso_resX, iso_resY,
												 Fn::f, ud, tiers_scaled, 3, -FLT_MAX, FLT_MAX, pre_log,
												 iso_half_pixel, iso_subdiv );
				ImGui::TreePop();
			}
		}
		ApplyOpenAll();
		if ( ImGui::CollapsingHeader( "Interactions" ) )
		{
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Polygon Hit Testing##Interactions" ) )
			{
				static float s_cull_interact_h = 0; float s_cull_interact_y;
				if ( BeginCullSection( s_cull_interact_h, s_cull_interact_y ) )
				{
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
							ImPolyShapeData data = { &pos_norms[0], 3 };
							bool hovered = IsMouseHovering( pos, pos + ImVec2( size, size ), Im_IsPolyConvexContains, &data );
							pDrawList->AddConvexPolyFilled( &pos_norms[0], 3, IM_COL32( hovered ? 200 : 0, hovered ? 0 : 200, 0, 80 ) );
							ImGui::Dummy( ImVec2( size, size ) );
							pos = ImGui::GetCursorScreenPos();
							ImVector<ImVec2> disk;
							disk.resize( 32 );
							for ( int k = 0; k < 32; ++k )
							{
								float angle = ((float)k) * 2.0f * IM_PI / 32.0f;
								float cos0 = ImCos( angle );
								float sin0 = ImSin( angle );
								disk[k].x = pos.x + 0.5f * size + cos0 * size * 0.5f;
								disk[k].y = pos.y + 0.5f * size + sin0 * size * 0.5f;
							}
							data = { &disk[0], 32 };
							hovered = IsMouseHovering( pos, pos + ImVec2( size, size ), Im_IsPolyConvexContains, &data );
							pDrawList->AddConvexPolyFilled( &disk[0], 32, IM_COL32( hovered ? 200 : 0, hovered ? 0 : 200, 0, 80 ) );

							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Poly_Convex_Hovered", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
								ImVec2& v = pos_norms[k];
								v.x *= size;
								v.y *= size;
								v += pos;
							}
							ImPolyShapeData data = { &pos_norms[0], sz };
							bool hovered = IsMouseHovering( pos * 0.99f, pos + ImVec2( 1.01f * size, 1.01f * size ), Im_IsPolyConcaveContains, &data );
							pDrawList->AddConcavePolyFilled( &pos_norms[0], sz, IM_COL32( hovered ? 200 : 0, hovered ? 0 : 200, 0, 80 ) );
							ImGui::Dummy( ImVec2( size, size ) );
							pos = ImGui::GetCursorScreenPos();
							ImVector<ImVec2> ring;
							sz = 20; // 10-pointed star: alternating outer/inner vertices
							ring.resize( sz );
							float outer_r = size * 0.45f;
							float inner_r = size * 0.18f;
							for ( int k = 0; k < sz; ++k )
							{
								float angle = ( (float)k ) * 2.0f * IM_PI / (float)sz - IM_PI * 0.5f;
								float r = ( k % 2 == 0 ) ? outer_r : inner_r;
								ring[k].x = pos.x + size * 0.5f + r * ImCos( angle );
								ring[k].y = pos.y + size * 0.5f + r * ImSin( angle );
							}
							data = { &ring[0], sz };
							hovered = IsMouseHovering( pos * 0.99f, pos + ImVec2( 1.01f * size, 1.01f * size ), Im_IsPolyConcaveContains, &data );
							pDrawList->AddConcavePolyFilled( &ring[0], sz, IM_COL32( hovered ? 200 : 0, hovered ? 0 : 200, 0, 80 ) );
							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Poly_Concave_Hovered", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
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
								ImVec2& v = pos_norms[k];
								v.x *= size;
								v.y *= size;
								v += pos;
							}
							ImPolyHoleShapeData data = { &pos_norms[0], NULL, sz, 1, 1 };
							bool hovered = IsMouseHovering( pos * 0.99f, pos + ImVec2( 1.01f * size, 1.01f * size ), Im_IsPolyWithHoleContains, &data );
							DrawShapeWithHole( pDrawList, &pos_norms[0], sz, IM_COL32( hovered ? 200 : 0, hovered ? 0 : 200, 0, 80 ) );
							ImGui::Dummy( ImVec2( size, size ) );
							pos = ImGui::GetCursorScreenPos();
							ImVector<ImVec2> ring;
							sz = 64;
							ring.resize( sz );
							float r;
							for ( int k = 0; k < 32; ++k )
							{
								float angle = -((float)k) * 2.0f * IM_PI / 31.0f;
								float cos0 = ImCos( angle );
								float sin0 = ImSin( angle );
								r = size * (((float)(rand() % 1000) / 1000.0f) * 0.25f + 0.75f);
								ring[k].x = pos.x + size * 0.5f + r * 0.5f * cos0;
								ring[k].y = pos.y + size * 0.5f + r * 0.5f * sin0;
							}
							srand( 97 );
							for ( int k = 32; k < 64; ++k )
							{
								float angle = ((float)(k - 32)) * 2.0f * IM_PI / 31.0f;
								float cos0 = ImCos( angle );
								float sin0 = ImSin( angle );
								r = size * 0.75f * (((float)(rand() % 1000) / 1000.0f) * 0.5f + 0.5f);
								ring[k].x = pos.x + size * 0.5f + r * 0.5f * cos0;
								ring[k].y = pos.y + size * 0.5f + r * 0.5f * sin0;
							}
							data = { &ring[0], NULL, sz, 1, 1 };
							hovered = IsMouseHovering( pos, pos + ImVec2( size, size ), Im_IsPolyWithHoleContains, &data );
							DrawShapeWithHole( pDrawList, &ring[0], sz, IM_COL32( hovered ? 200 : 0, hovered ? 0 : 200, 0, 80 ) );
							ImGui::Dummy( ImVec2( size, size ) );
						}
						DW_SsRecord( "Poly_With_Hole_Hovered", _sy0, ImGui::GetCursorPos().y );
					}
					EndCullSection( s_cull_interact_h, s_cull_interact_y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Proportional Drag Group" ) )
			{
				ImGui::Text( "Proportional multi-drag group (drag any bar, neighbors follow):" );
				static float sliders[12] = { 0.3f, 0.35f, 0.4f, 0.45f, 0.5f, 0.5f, 0.5f, 0.45f, 0.4f, 0.35f, 0.3f, 0.25f };
				static float pm_radius = 3.0f;
				static int   pm_kernel = (int)ImWidgetsFalloff_Gaussian;
				ImGui::SliderFloat( "Falloff radius (slots)", &pm_radius, 0.5f, 8.0f );
				ImGui::Combo( "Kernel", &pm_kernel, "Linear\0Gaussian\0Smoothstep\0" );
				ImGui::BeginGroup();
				ImDrawList* pm_dl = ImGui::GetWindowDrawList();
				ImVec2 pm_o = ImGui::GetCursorScreenPos();
				const int N = IM_ARRAYSIZE( sliders );
				const float bar_w = ImPlatform_LpToPx( 28.0f );
				const float bar_h = ImPlatform_LpToPx( 120.0f );
				const float gap = ImPlatform_LpToPx( 6.0f );
				ImGui::InvisibleButton( "##pm_area", ImVec2( (bar_w + gap) * N, bar_h ) );
				ImWidgets::PushDragGroup( ImGui::GetID( "grp" ), pm_radius, (ImWidgetsFalloff)pm_kernel );
				for ( int i = 0; i < N; ++i )
				{
					float old = sliders[i];
					float g = ImWidgets::GetDragGroupDelta( i );
					if ( g != 0.0f ) sliders[i] = ImClamp( sliders[i] + g, 0.0f, 1.0f );
					float x0 = pm_o.x + i * (bar_w + gap);
					float y0 = pm_o.y;
					float y1 = pm_o.y + bar_h;
					ImVec2 bmin( x0, y0 ), bmax( x0 + bar_w, y1 );
					float fy = y1 - sliders[i] * bar_h;
					pm_dl->AddRect( bmin, bmax, IM_COL32( 180, 180, 180, 200 ) );
					pm_dl->AddRectFilled( ImVec2( bmin.x + 1, fy ), ImVec2( bmax.x - 1, bmax.y - 1 ),
										  IM_COL32( 120, 220, 255, 220 ) );
					ImVec2 mouse = ImGui::GetIO().MousePos;
					bool inside = mouse.x >= bmin.x && mouse.x <= bmax.x && mouse.y >= bmin.y && mouse.y <= bmax.y;
					if ( ImGui::IsItemActive() && inside && ImGui::IsMouseDown( 0 ) )
					{
						float nv = ImClamp( (bmax.y - mouse.y) / bar_h, 0.0f, 1.0f );
						float delta = nv - old;
						sliders[i] = nv;
						ImWidgets::SetDragGroupActive( i, delta );
					}
				}
				ImWidgets::PopDragGroup();
				ImGui::EndGroup();
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Vector Drawing Tool" ) )
			{
				ImGui::TextWrapped(
					"Left-click empty: add anchor (drag to pull out tangent). "
					"Left-click existing anchor: select (drag moves). "
					"Click first anchor of active path to close. "
					"Right-click: finish path open. Middle-drag / Shift+drag: pan. "
					"Wheel: zoom. Delete: remove selected anchor." );
				static ImVectorDrawingData vdt;
				// Default style bucket -- edits here seed new paths when you start drawing,
				// and edit the current path once one exists. This means the Style/Color/
				// Thickness controls are always visible instead of appearing only after
				// the first click.
				static ImVectorDrawingPath s_next_path_defaults;
				int pi = vdt.SelectedPath >= 0 ? vdt.SelectedPath
					: (vdt.ActivePath >= 0 ? vdt.ActivePath : (vdt.Paths.Size - 1));
				ImVectorDrawingPath& p = (pi >= 0 && pi < vdt.Paths.Size)
					? vdt.Paths[pi]
					: s_next_path_defaults;
				const char* style_names[] = { "Polyline", "PolylineAA", "StrokedBezier",
					"StrokedDashedBezier", "DashedPolyline" };
				int s = (int)p.Style;
				if ( ImGui::Combo( "Style", &s, style_names, IM_ARRAYSIZE( style_names ) ) )
					p.Style = (ImVectorDrawingStyle)s;
				ImGui::SliderFloat( "Thickness", &p.Thickness, 0.5f, 16.0f );
				ImVec4 col = ImGui::ColorConvertU32ToFloat4( p.Color );
				if ( ImGui::ColorEdit4( "Color", &col.x, ImGuiColorEditFlags_NoInputs ) )
					p.Color = ImGui::ColorConvertFloat4ToU32( col );
				if ( p.Style == ImVectorDrawingStyle_StrokedDashedBezier
					 || p.Style == ImVectorDrawingStyle_DashedPolyline )
				{
					ImGui::SliderFloat( "Dash", &p.DashLen, 1.0f, 40.0f );
					ImGui::SliderFloat( "Gap", &p.GapLen, 1.0f, 40.0f );
				}
				ImGui::Checkbox( "Closed", &p.Closed );
				// Seed new paths created by the user with the defaults currently shown.
				int prev_path_count = vdt.Paths.Size;
				if ( ImGui::Button( "Clear All" ) )
				{
					vdt.Paths.clear(); vdt.ActivePath = -1; vdt.SelectedPath = -1;
				}
				ImGui::SameLine();
				if ( ImGui::Button( "Reset View" ) )
				{
					vdt.PanOffset = ImVec2( 0, 0 ); vdt.Zoom = 1.0f;
				}
				ImWidgets::VectorDrawingTool( "vdt", vdt, ImVec2( 0, 380 ) );
				// If a new path was just created, copy the currently-shown defaults into it.
				if ( vdt.Paths.Size > prev_path_count && prev_path_count >= 0 )
				{
					ImVectorDrawingPath& np = vdt.Paths.back();
					np.Style = s_next_path_defaults.Style;
					np.Thickness = s_next_path_defaults.Thickness;
					np.Color = s_next_path_defaults.Color;
					np.DashLen = s_next_path_defaults.DashLen;
					np.GapLen = s_next_path_defaults.GapLen;
					np.Closed = s_next_path_defaults.Closed;
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Eyedropper##Interactions" ) )
			{
				static ImTextureID s_ed_tex = ImTextureID_Invalid;
				static ImVector<ImU32> s_ed_pixels;
				static const int s_ed_w = 256, s_ed_h = 128;
				if ( s_ed_tex == ImTextureID_Invalid )
				{
					s_ed_pixels.resize( s_ed_w * s_ed_h );
					for ( int y = 0; y < s_ed_h; ++y )
					{
						float v = 1.0f - (float)y / (float)( s_ed_h - 1 );  // top = bright
						for ( int x = 0; x < s_ed_w; ++x )
						{
							float h = (float)x / (float)( s_ed_w - 1 );
							float R, G, B;
							ImGui::ColorConvertHSVtoRGB( h, 1.0f, v, R, G, B );
							s_ed_pixels[ y * s_ed_w + x ] = IM_COL32( (int)( R * 255.0f ), (int)( G * 255.0f ), (int)( B * 255.0f ), 255 );
						}
					}
					ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( s_ed_w, s_ed_h );
					s_ed_tex = ImPlatform_CreateTexture( s_ed_pixels.Data, &td );
				}
				static ImU32 s_ed_picked = IM_COL32_WHITE;
				ImGui::TextWrapped( "Hover the swatch to see a magnified neighbor grid; click to lock the sampled color." );
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 sz( ImPlatform_LpToPx( 380.0f ), ImPlatform_LpToPx( 190.0f ) );
				if ( s_ed_tex != ImTextureID_Invalid )
					dl->AddImage( s_ed_tex, p, p + sz );
				else
					dl->AddRectFilledMultiColor( p, p + sz, IM_COL32( 255, 0, 0, 255 ), IM_COL32( 255, 255, 0, 255 ), IM_COL32( 0, 255, 255, 255 ), IM_COL32( 0, 0, 255, 255 ) );
				ImWidgets::ImWidgetsEyedropperBitmap bm = { s_ed_pixels.Data, s_ed_w, s_ed_h };
				ImWidgets::ImWidgetsEyedropperResult r = ImWidgets::Eyedropper( "##ed_demo", ImRect( p, p + sz ),
					ImWidgets::EyedropperSampleBitmap, &bm );
				if ( r.Picked ) s_ed_picked = r.Color;
				ImGui::SetCursorScreenPos( ImVec2( p.x, p.y + sz.y + 8.0f ) );
				ImGui::AlignTextToFramePadding();
				ImGui::Text( "Sampled:" );
				ImGui::SameLine();
				ImVec4 cv = ImGui::ColorConvertU32ToFloat4( r.Hovered ? r.Color : s_ed_picked );
				ImGui::ColorButton( "##ed_sample", cv, ImGuiColorEditFlags_NoTooltip, ImVec2( 32, 16 ) );
				ImGui::SameLine();
				ImGui::Text( r.Hovered ? "(hovering)" : "(locked)" );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Gradient Drop##Interactions" ) )
			{
				// Source #0: synthetic HSV swatch (always available).
				static ImTextureID s_gd_hsv_tex = ImTextureID_Invalid;
				static ImVector<ImU32> s_gd_hsv_pixels;
				static const int s_gd_hsv_w = 256, s_gd_hsv_h = 128;
				if ( s_gd_hsv_tex == ImTextureID_Invalid )
				{
					s_gd_hsv_pixels.resize( s_gd_hsv_w * s_gd_hsv_h );
					for ( int y = 0; y < s_gd_hsv_h; ++y )
					{
						float v = 1.0f - (float)y / (float)( s_gd_hsv_h - 1 );
						for ( int x = 0; x < s_gd_hsv_w; ++x )
						{
							float h = (float)x / (float)( s_gd_hsv_w - 1 );
							float R, G, B;
							ImGui::ColorConvertHSVtoRGB( h, 1.0f, v, R, G, B );
							s_gd_hsv_pixels[ y * s_gd_hsv_w + x ] = IM_COL32(
								(int)( R * 255.0f ), (int)( G * 255.0f ), (int)( B * 255.0f ), 255 );
						}
					}
					ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( s_gd_hsv_w, s_gd_hsv_h );
					s_gd_hsv_tex = ImPlatform_CreateTexture( s_gd_hsv_pixels.Data, &td );
				}
				// Source #1: a real loaded image. CPU buffer (for sampling) and
				// GPU texture (for display) are both held here.
				static ImTextureID s_gd_img_tex = ImTextureID_Invalid;
				static ImVector<ImU32> s_gd_img_pixels;
				static int s_gd_img_w = 0, s_gd_img_h = 0;
				if ( s_gd_img_tex == ImTextureID_Invalid )
				{
					int w = 0, h = 0;
					stbi_uc* data = stbi_load( "astro.png", &w, &h, NULL, 4 );
					if ( data )
					{
						s_gd_img_w = w; s_gd_img_h = h;
						s_gd_img_pixels.resize( w * h );
						memcpy( s_gd_img_pixels.Data, data, (size_t)w * (size_t)h * 4 );
						ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( w, h );
						s_gd_img_tex = ImPlatform_CreateTexture( s_gd_img_pixels.Data, &td );
						STBI_FREE( data );
					}
				}

				static ImWidgets::ImWidgetsGradientDropState s_gd_state;
				static ImGradientData s_gd_grad;
				static int   s_gd_source    = 0;   // 0 = HSV, 1 = astro.png
				static int   s_gd_last_src  = 0;
				static int   s_gd_max_stops = 8;
				static float s_gd_thresh    = 0.04f;
				static float s_gd_step      = 3.0f;
				static bool  s_gd_live      = true;

				ImGui::TextWrapped( "Click + drag across the swatch to collect colors. "
					"On release the stroke is Douglas-Peucker simplified in OkLab into "
					"the gradient below. A live preview follows the cursor while dragging. "
					"Try the image source to pull a palette out of a real photograph." );

				ImGui::Combo      ( "Source",               &s_gd_source, "HSV synthetic\0astro.png\0" );
				ImGui::SliderInt  ( "Max stops",            &s_gd_max_stops, 2, 16 );
				ImGui::SliderFloat( "OkLab dE threshold",   &s_gd_thresh,    0.005f, 0.2f, "%.3f" );
				ImGui::SliderFloat( "Min step (px)",        &s_gd_step,      1.0f, 20.0f, "%.1f" );
				ImGui::Checkbox   ( "Live preview tooltip", &s_gd_live );

				// Changing source: drop the in-flight stroke + last gradient
				// so they don't survive across unrelated swatches.
				if ( s_gd_source != s_gd_last_src )
				{
					s_gd_state.Path.clear();
					s_gd_state.Colors.clear();
					s_gd_grad = ImGradientData();
					s_gd_last_src = s_gd_source;
				}

				bool image_mode = ( s_gd_source == 1 && s_gd_img_tex != ImTextureID_Invalid );

				ImTextureID tex = image_mode ? s_gd_img_tex : s_gd_hsv_tex;
				ImU32 const* src_pixels = image_mode ? s_gd_img_pixels.Data : s_gd_hsv_pixels.Data;
				int src_w = image_mode ? s_gd_img_w : s_gd_hsv_w;
				int src_h = image_mode ? s_gd_img_h : s_gd_hsv_h;

				// Display size: 380 lp wide, height respects source aspect (capped).
				float disp_w = ImPlatform_LpToPx( 380.0f );
				float disp_h = ( src_w > 0 ) ? disp_w * (float)src_h / (float)src_w : disp_w * 0.5f;
				float max_h  = ImPlatform_LpToPx( 280.0f );
				if ( disp_h > max_h ) { disp_h = max_h; disp_w = disp_h * (float)src_w / ImMax( 1, src_h ); }
				ImVec2 sz( disp_w, disp_h );

				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 p = ImGui::GetCursorScreenPos();
				if ( tex != ImTextureID_Invalid )
					dl->AddImage( tex, p, p + sz );
				else
					dl->AddRectFilledMultiColor( p, p + sz,
						IM_COL32( 255, 0, 0, 255 ), IM_COL32( 255, 255, 0, 255 ),
						IM_COL32( 0, 255, 255, 255 ), IM_COL32( 0, 0, 255, 255 ) );

				ImWidgets::ImWidgetsEyedropperBitmap bm = { src_pixels, src_w, src_h };
				bool fresh = ImWidgets::GradientDrop( "##gd_demo",
					ImRect( p, p + sz ),
					ImWidgets::EyedropperSampleBitmap, (void*)&bm,
					s_gd_state, &s_gd_grad,
					s_gd_max_stops, s_gd_thresh, s_gd_step, 3.0f, s_gd_live );

				ImGui::SetCursorScreenPos( ImVec2( p.x, p.y + sz.y + 8.0f ) );
				ImGui::Text( "Resulting gradient (%d stops%s):",
					s_gd_grad.Stops.Size, fresh ? " - just finalized" : "" );
				ImVec2 bar_pos = ImGui::GetCursorScreenPos();
				ImVec2 bar_sz ( sz.x, 28.0f );
				ImWidgets::DrawGradientBar( dl, s_gd_grad, bar_pos, bar_sz, 256 );
				ImGui::Dummy( bar_sz );

				if ( ImGui::Button( "Reset gradient" ) )
				{
					s_gd_grad = ImGradientData();
					s_gd_state.Path.clear();
					s_gd_state.Colors.clear();
				}
				if ( s_gd_source == 1 && s_gd_img_tex == ImTextureID_Invalid )
					ImGui::TextDisabled( "(astro.png not found in workingdir/ -- image source unavailable)" );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Crop Rect##Interactions" ) )
			{
				static ImWidgets::ImWidgetsCropState s_crop = { ImRect(), -1, ImVec2( 0, 0 ), ImRect(), ImVec2( FLT_MAX, FLT_MAX ) };
				static int   s_crop_guides = (int)ImWidgets::ImWidgetsCropGuides_RuleOfThirds;
				static float s_crop_aspect = 0.0f;
				static int   s_crop_aspect_idx = 0;
				const char* aspect_names[] = { "Free", "1:1", "3:2", "4:3", "16:9", "21:9" };
				const float aspect_vals [] = { 0.0f, 1.0f, 1.5f, 4.0f / 3.0f, 16.0f / 9.0f, 21.0f / 9.0f };
				if ( ImGui::Combo( "Aspect", &s_crop_aspect_idx, aspect_names, IM_ARRAYSIZE( aspect_names ) ) )
					s_crop_aspect = aspect_vals[ s_crop_aspect_idx ];
				ImGui::CheckboxFlags( "Rule of thirds", &s_crop_guides, ImWidgets::ImWidgetsCropGuides_RuleOfThirds ); ImGui::SameLine();
				ImGui::CheckboxFlags( "Golden ratio",   &s_crop_guides, ImWidgets::ImWidgetsCropGuides_GoldenRatio  ); ImGui::SameLine();
				ImGui::CheckboxFlags( "Diagonals",      &s_crop_guides, ImWidgets::ImWidgetsCropGuides_Diagonals    ); ImGui::SameLine();
				ImGui::CheckboxFlags( "Center",         &s_crop_guides, ImWidgets::ImWidgetsCropGuides_Center       );
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 sz( ImPlatform_LpToPx( 420.0f ), ImPlatform_LpToPx( 240.0f ) );
				ImDrawList* dl = ImGui::GetWindowDrawList();
				dl->AddRectFilledMultiColor( p, p + sz, IM_COL32( 60, 80, 160, 255 ), IM_COL32( 220, 80, 80, 255 ),
					                          IM_COL32( 240, 200, 80, 255 ), IM_COL32( 60, 180, 120, 255 ) );
				ImWidgets::CropRect( "##crop_demo", s_crop, ImRect( p, p + sz ), s_crop_aspect,
					                  (ImWidgets::ImWidgetsCropGuides)s_crop_guides );
				ImGui::SetCursorScreenPos( ImVec2( p.x, p.y + sz.y + 6.0f ) );
				ImGui::Text( "Crop: (%.0f, %.0f) -> (%.0f, %.0f)  %.0f x %.0f",
					           s_crop.Rect.Min.x - p.x, s_crop.Rect.Min.y - p.y,
					           s_crop.Rect.Max.x - p.x, s_crop.Rect.Max.y - p.y,
					           s_crop.Rect.GetWidth(), s_crop.Rect.GetHeight() );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Snap Lines / Smart Guides##Interactions" ) )
			{
				static ImVec2 s_snap_moving_pos( 60.0f, 60.0f );
				static ImVec2 s_snap_moving_sz ( 110.0f, 70.0f );
				static int    s_snap_flags = (int)ImWidgets::ImWidgetsSnapFlags_All;
				static float  s_snap_radius = 6.0f;
				ImGui::SliderFloat( "Snap radius (px)", &s_snap_radius, 1.0f, 20.0f );
				ImGui::CheckboxFlags( "Left",    &s_snap_flags, ImWidgets::ImWidgetsSnapFlags_LeftEdge   ); ImGui::SameLine();
				ImGui::CheckboxFlags( "Right",   &s_snap_flags, ImWidgets::ImWidgetsSnapFlags_RightEdge  ); ImGui::SameLine();
				ImGui::CheckboxFlags( "Top",     &s_snap_flags, ImWidgets::ImWidgetsSnapFlags_TopEdge    ); ImGui::SameLine();
				ImGui::CheckboxFlags( "Bottom",  &s_snap_flags, ImWidgets::ImWidgetsSnapFlags_BottomEdge ); ImGui::SameLine();
				ImGui::CheckboxFlags( "CenterH", &s_snap_flags, ImWidgets::ImWidgetsSnapFlags_CenterH    ); ImGui::SameLine();
				ImGui::CheckboxFlags( "CenterV", &s_snap_flags, ImWidgets::ImWidgetsSnapFlags_CenterV    );
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 sz( ImPlatform_LpToPx( 480.0f ), ImPlatform_LpToPx( 280.0f ) );
				ImDrawList* dl = ImGui::GetWindowDrawList();
				dl->AddRectFilled( p, p + sz, IM_COL32( 40, 40, 50, 255 ) );
				ImRect targets[] =
				{
					ImRect( p.x +  40, p.y +  40, p.x + 140, p.y +  90 ),
					ImRect( p.x + 220, p.y +  60, p.x + 320, p.y + 130 ),
					ImRect( p.x + 120, p.y + 160, p.x + 250, p.y + 230 ),
					ImRect( p.x + 340, p.y + 170, p.x + 440, p.y + 240 ),
				};
				for ( int i = 0; i < IM_ARRAYSIZE( targets ); ++i )
				{
					dl->AddRectFilled( targets[ i ].Min, targets[ i ].Max, IM_COL32( 110, 130, 160, 200 ) );
					dl->AddRect      ( targets[ i ].Min, targets[ i ].Max, IM_COL32( 220, 230, 240, 255 ) );
				}
				ImGui::InvisibleButton( "##snap_area", sz );
				ImRect moving( p + s_snap_moving_pos, p + s_snap_moving_pos + s_snap_moving_sz );
				if ( ImGui::IsItemActive() )
				{
					ImVec2 d = ImGui::GetIO().MouseDelta;
					s_snap_moving_pos += d;
					moving = ImRect( p + s_snap_moving_pos, p + s_snap_moving_pos + s_snap_moving_sz );
					ImVec2 snap = ImWidgets::ComputeSnapAndDraw( dl, moving, targets, IM_ARRAYSIZE( targets ),
						                                          s_snap_radius, (ImWidgets::ImWidgetsSnapFlags)s_snap_flags );
					s_snap_moving_pos += snap;
					moving = ImRect( p + s_snap_moving_pos, p + s_snap_moving_pos + s_snap_moving_sz );
				}
				dl->AddRectFilled( moving.Min, moving.Max, IM_COL32( 220, 160, 60, 200 ) );
				dl->AddRect      ( moving.Min, moving.Max, IM_COL32( 255, 255, 255, 255 ), 0.0f, 0, 1.5f );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Onion-Skin Overlay##Interactions" ) )
			{
				static ImTextureID s_onion_tex[ 9 ] = { ImTextureID_Invalid, ImTextureID_Invalid, ImTextureID_Invalid, ImTextureID_Invalid, ImTextureID_Invalid, ImTextureID_Invalid, ImTextureID_Invalid, ImTextureID_Invalid, ImTextureID_Invalid };
				static const int s_onion_w = 128, s_onion_h = 96;
				if ( s_onion_tex[ 0 ] == ImTextureID_Invalid )
				{
					ImVector<ImU32> px;
					px.resize( s_onion_w * s_onion_h );
					for ( int fi = 0; fi < 9; ++fi )
					{
						for ( int i = 0; i < s_onion_w * s_onion_h; ++i ) px[ i ] = IM_COL32( 0, 0, 0, 0 );
						float t = (float)fi / 8.0f;
						float cx = 16.0f + t * ( s_onion_w - 32.0f );
						float cy = s_onion_h * 0.5f + ImSin( t * IM_PI * 1.5f ) * 18.0f;
						for ( int y = 0; y < s_onion_h; ++y )
						{
							for ( int x = 0; x < s_onion_w; ++x )
							{
								float dx = (float)x - cx, dy = (float)y - cy;
								float r2 = dx * dx + dy * dy;
								if ( r2 < 100.0f )
								{
									float a = ImClamp( 1.0f - ImSqrt( r2 ) / 10.0f, 0.0f, 1.0f );
									int A = (int)( a * 255.0f );
									px[ y * s_onion_w + x ] = IM_COL32( 240, 240, 240, A );
								}
							}
						}
						ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( s_onion_w, s_onion_h );
						s_onion_tex[ fi ] = ImPlatform_CreateTexture( px.Data, &td );
					}
				}
				static int s_onion_cur = 4;
				static int s_onion_back = 2, s_onion_forward = 2;
				static float s_onion_base_alpha = 0.45f;
				ImGui::SliderInt  ( "Current frame", &s_onion_cur, 0, 8 );
				ImGui::SliderInt  ( "Back ghosts",   &s_onion_back,    0, 4 );
				ImGui::SliderInt  ( "Forward ghosts",&s_onion_forward, 0, 4 );
				ImGui::SliderFloat( "Base alpha",    &s_onion_base_alpha, 0.0f, 1.0f );
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 sz( ImPlatform_LpToPx( 380.0f ), ImPlatform_LpToPx( 280.0f ) );
				ImDrawList* dl = ImGui::GetWindowDrawList();
				dl->AddRectFilled( p, p + sz, IM_COL32( 30, 30, 35, 255 ) );
				ImWidgets::DrawOnionSkinAuto( dl, ImRect( p, p + sz ), s_onion_tex, 9, s_onion_cur,
					                            s_onion_back, s_onion_forward,
					                            IM_COL32( 80, 160, 255, 255 ), IM_COL32( 255, 110, 80, 255 ),
					                            s_onion_base_alpha );
				dl->AddRect( p, p + sz, IM_COL32( 200, 200, 200, 200 ) );
				ImGui::Dummy( sz );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Lasso / Marquee Selection##Interactions" ) )
			{
				static ImWidgets::ImWidgetsSelectionState s_sel = { ImWidgets::ImWidgetsSelectionMode_Marquee, ImVector<ImVec2>(), false, false };
				static int s_sel_mode = 0;
				const char* mode_names[] = { "Marquee", "Lasso" };
				if ( ImGui::Combo( "Mode", &s_sel_mode, mode_names, IM_ARRAYSIZE( mode_names ) ) )
					s_sel.Mode = (ImWidgets::ImWidgetsSelectionMode)s_sel_mode;
				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 sz( ImPlatform_LpToPx( 480.0f ), ImPlatform_LpToPx( 280.0f ) );
				ImDrawList* dl = ImGui::GetWindowDrawList();
				dl->AddRectFilled( p, p + sz, IM_COL32( 25, 25, 30, 255 ) );
				const int N = 60;
				static ImVec2 s_sel_pts[ N ];
				static bool s_sel_init = false;
				if ( !s_sel_init )
				{
					srand( 1357 );
					for ( int i = 0; i < N; ++i )
						s_sel_pts[ i ] = ImVec2( ( (float)( rand() % 1000 ) / 1000.0f ) * ( sz.x - 20.0f ) + 10.0f,
							                       ( (float)( rand() % 1000 ) / 1000.0f ) * ( sz.y - 20.0f ) + 10.0f );
					s_sel_init = true;
				}
				ImVec2 abs_pts[ N ];
				for ( int i = 0; i < N; ++i ) abs_pts[ i ] = p + s_sel_pts[ i ];
				ImWidgets::BeginSelection( "##sel_demo", ImRect( p, p + sz ), s_sel );
				bool inside[ N ];
				ImWidgets::TestSelectionPoints( s_sel, abs_pts, N, inside );
				int hit = 0;
				for ( int i = 0; i < N; ++i )
				{
					ImU32 c = inside[ i ] ? IM_COL32( 255, 220, 80, 255 ) : IM_COL32( 120, 160, 220, 255 );
					dl->AddCircleFilled( abs_pts[ i ], 4.0f, c );
					if ( inside[ i ] ) ++hit;
				}
				ImGui::SetCursorScreenPos( ImVec2( p.x, p.y + sz.y + 4.0f ) );
				ImGui::Text( "%d / %d points inside selection", hit, N );
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Pan-Zoom Canvas + Minimap##Interactions" ) )
			{
				static ImWidgets::ImWidgetsCanvasState s_pz = { ImVec2( 200, 120 ), 1.0f, false, ImVec2( 0, 0 ), ImRect() };
				ImGui::TextWrapped( "Wheel: zoom around cursor. MMB or Shift+LMB: pan. The minimap shows the viewport (yellow) within the world content (gray); click it to recenter." );
				ImVec2 size( ImPlatform_LpToPx( 520.0f ), ImPlatform_LpToPx( 320.0f ) );
				if ( ImWidgets::BeginCanvas( "##pz_demo", size, s_pz ) )
				{
					ImDrawList* dl = ImGui::GetWindowDrawList();
					const ImRect world_content( -200, -150, 400, 250 );
					ImVec2 a = ImWidgets::CanvasWorldToScreen( s_pz, world_content.Min );
					ImVec2 b = ImWidgets::CanvasWorldToScreen( s_pz, world_content.Max );
					dl->AddRect( a, b, IM_COL32( 180, 180, 220, 220 ), 0.0f, 0, 2.0f );
					for ( int k = 0; k < 6; ++k )
					{
						float angle = (float)k * IM_PI / 3.0f;
						ImVec2 wpos( ImCos( angle ) * 120.0f + 100.0f, ImSin( angle ) * 80.0f + 50.0f );
						ImVec2 sp = ImWidgets::CanvasWorldToScreen( s_pz, wpos );
						dl->AddCircleFilled( sp, 14.0f * s_pz.Zoom, IM_COL32( 80 + k * 30, 200 - k * 20, 120, 220 ) );
						dl->AddCircle      ( sp, 14.0f * s_pz.Zoom, IM_COL32( 255, 255, 255, 255 ) );
					}
					ImVec2 og = ImWidgets::CanvasWorldToScreen( s_pz, ImVec2( 0, 0 ) );
					ImWidgets::DrawAxisArrows( dl, og, 60.0f * s_pz.Zoom, 60.0f * s_pz.Zoom );
					ImWidgets::DrawCanvasMinimap( s_pz, world_content );
					ImWidgets::EndCanvas();
				}
				ImGui::Text( "Pan: (%.0f, %.0f)  Zoom: %.2fx", s_pz.Pan.x, s_pz.Pan.y, s_pz.Zoom );
				ImGui::TreePop();
			}
		}
		ApplyOpenAll();
		if ( ImGui::CollapsingHeader( "Widgets" ) )
		{
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Buttons##Widgets" ) )
			{

				{
					float _sy0 = ImGui::GetCursorPos().y;
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
						value += (int)ImWidgets::ButtonExCircle( caption.c_str(), radius, 0 );
					}
					DW_SsRecord( "Button_Circle", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Button Capsule" ) )
					{
						float const size = CanvasSize();
						static int value = 0;
						static float length = size;
						static float thickness = size * 0.25f;
						ImGui::DragFloat( "length", &length, 1.0f, 0.0f, 2.0f * size );
						ImGui::DragFloat( "thickness", &thickness, 1.0f, 0.0f, 2.0f * size );
						ImGui::Text( "Value: %d", value );
						value += (int)ButtonExCapsuleH( "CapsuleH", length, thickness, 0 );
						value += (int)ButtonExCapsuleV( "CapsuleV", length, thickness, 0 );
					}
					DW_SsRecord( "Button_Capsule", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Button Convex" ) )
					{
						float const size = CanvasSize();
						ImVector<ImVec2> disk;
						disk.resize( 32 );
						for ( int k = 0; k < 32; ++k )
						{
							float angle = ((float)k) * 2.0f * IM_PI / 32.0f;
							float cos0 = ImCos( angle );
							float sin0 = ImSin( angle );
							disk[k].x = 0.5f * size + cos0 * size * 0.5f;
							disk[k].y = 0.5f * size + sin0 * size * 0.5f;
						}
						static int value = 0;
						ImGui::Text( "Value: %d", value );
						value += (int)ImWidgets::ButtonExConvex( "Convex", ImVec2( 0, 0 ), &disk[0], 32, 0 );
					}
					DW_SsRecord( "Button_Convex", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Button Concave" ) )
					{
						float const size = CanvasSize();
						int sz = 8;
						ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.3f, 0.0f }, { 0.3f, 0.7f }, { 0.7f, 0.7f }, { 0.7f, 0.0f },
											   { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
						for ( int k = 0; k < sz; ++k )
						{
							ImVec2& v = pos_norms[k];
							v.x *= size;
							v.y *= size;
						}
						static int value = 0;
						ImGui::Text( "Value: %d", value );
						value += (int)ImWidgets::ButtonExConcave( "Concave", ImVec2( 0, 0 ), &pos_norms[0], sz, ImVec2( 0.0f, size / 3.0f ), 0 );
					}
					DW_SsRecord( "Button_Concave", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Button With Hole" ) )
					{
						float const size = CanvasSize();
						int sz = 10;
						ImVec2 pos_norms[] = { { 0.0f, 0.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f },
											   { 0.3f, 0.3f }, { 0.7f, 0.3f }, { 0.7f, 0.7f }, { 0.3f, 0.7f }, { 0.3f, 0.3f } };
						for ( int k = 0; k < sz; ++k )
						{
							ImVec2& v = pos_norms[k];
							v.x *= size;
							v.y *= size;
						}
						static int value = 0;
						ImGui::Text( "Value: %d", value );
						value += (int)ImWidgets::ButtonExWithHole( "With Hole", ImVec2( 0, 0 ), &pos_norms[0], sz, ImVec2( 0.0f, size / 3.0f ), 0 );
					}
					DW_SsRecord( "Button_With_Hole", _sy0, ImGui::GetCursorPos().y );
				}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Sliders & Inputs##Widgets" ) )
			{

				{
					float _sy0 = ImGui::GetCursorPos().y;
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
					DW_SsRecord( "DragFloatPrecise", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "SliderN" ) )
					{
						static float value[3] = { 0.25f, 10.0f, 100.0f };
						static float min = 0.1f;
						static float max = 150.0f;
						ImGui::Text( "Hover per region of influence" );
						ImWidgets::SliderNScalar( "Values##SliderNRegions", ImGuiDataType_Float, &value, 3, &min, &max, 8.0f, true );
						ImGui::Text( "Global Hover" );
						ImWidgets::SliderNScalar( "Values##SliderNGlobal", ImGuiDataType_Float, &value, 3, &min, &max, 8.0f, false );
						ImGui::DragFloat( "Near Plane", &value[0], 1.0f, min, value[1] );
						ImGui::DragFloat( "Focal Planes", &value[1], 1.0f, value[0], value[2] );
						ImGui::DragFloat( "Far Planes", &value[2], 1.0f, value[1], max );
					}
					DW_SsRecord( "SliderN", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "SliderN Vertical" ) )
					{
						static float vvalue[3] = { 0.25f, 10.0f, 100.0f };
						static float vmin = 0.1f;
						static float vmax = 150.0f;
						ImGui::Text( "Hover per region of influence" );
						ImGui::BeginGroup();
						ImWidgets::SliderNVerticalScalar( "V1##SliderNVRegions", ImGuiDataType_Float, &vvalue, 3, &vmin, &vmax, 8.0f, true, ImVec2( 20.0f, 160.0f ) );
						ImGui::SameLine();
						ImWidgets::SliderNVerticalScalar( "V2##SliderNVGlobal", ImGuiDataType_Float, &vvalue, 3, &vmin, &vmax, 8.0f, false, ImVec2( 20.0f, 160.0f ) );
						ImGui::EndGroup();
						ImGui::DragFloat( "Near Plane##VN", &vvalue[0], 1.0f, vmin, vvalue[1] );
						ImGui::DragFloat( "Focal Planes##VN", &vvalue[1], 1.0f, vvalue[0], vvalue[2] );
						ImGui::DragFloat( "Far Planes##VN", &vvalue[2], 1.0f, vvalue[1], vmax );
					}
					DW_SsRecord( "SliderN Vertical", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Range Slider" ) )
					{
						static float fLo = 0.25f, fHi = 0.75f;
						ImWidgets::RangeSliderFloat( "Float [0..1]##RS", &fLo, &fHi, 0.0f, 1.0f );

						static int iLo = 10, iHi = 80;
						ImWidgets::RangeSliderInt( "Int [0..100]##RS", &iLo, &iHi, 0, 100 );

						static double dLo = 1.5, dHi = 8.25;
						static double dMin = 0.0, dMax = 10.0;
						ImWidgets::RangeSliderScalar( "Double [0..10]##RS", ImGuiDataType_Double, &dLo, &dHi,
							&dMin, &dMax, "%.2f" );

						static ImU16 uLo = 200, uHi = 1800;
						static ImU16 uMin = 0, uMax = 4095;
						ImWidgets::RangeSliderScalar( "U16 [0..4095]##RS", ImGuiDataType_U16, &uLo, &uHi,
							&uMin, &uMax, "%u" );

						ImGui::TextWrapped( "Two-handle min/max slider. Drag either handle; the other is clamped." );
					}
					DW_SsRecord( "Range_Slider", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Segmented Control" ) )
					{
						static char const* const blendModes[] = { "Normal", "Multiply", "Screen", "Overlay" };
						static int sel = 0;
						ImWidgets::SegmentedControlInt( "Blend Mode##SC", &sel, blendModes, IM_ARRAYSIZE( blendModes ) );

						static char const* const axisLabels[] = { "X", "Y", "Z" };
						static ImU8 axisSel = 1;
						ImWidgets::SegmentedControlScalar( "Axis##SC", ImGuiDataType_U8, &axisSel,
							axisLabels, IM_ARRAYSIZE( axisLabels ) );

						static char const* const presets[] = { "Low", "Medium", "High", "Ultra", "Custom" };
						static int preset = 2;
						ImWidgets::SegmentedControlInt( "Preset##SC", &preset, presets, IM_ARRAYSIZE( presets ),
							ImVec2( 360, 0 ) );

						ImGui::TextWrapped( "Click a segment to select. p_value stores the index in any integer ImGuiDataType." );
					}
					DW_SsRecord( "Segmented_Control", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "SliderRing" ) )
					{
						static float fval = 0.5f;
						ImWidgets::SliderRingFloat( "Float##SR", &fval, 0.0f, 1.0f );

						static int ival = 50;
						ImWidgets::SliderRingInt( "Int##SR", &ival, 0, 100 );

						ImGui::Separator();
						ImGui::Text( "Clockwise:" );
						static float fCW = 0.25f;
						ImWidgets::SliderRingFloat( "CW Half##SR_CW", &fCW, 0.0f, 1.0f, -IM_PI, 0.0f, 12.0f );

						static float fCW2 = 0.75f;
						ImWidgets::SliderRingFloat( "CW Full##SR_CW2", &fCW2, 0.0f, 1.0f, -IM_PI, IM_PI, 6.0f );

						ImGui::Separator();
						ImGui::Text( "Counter-clockwise:" );
						static float fCCW = 0.25f;
						ImWidgets::SliderRingFloat( "CCW Half##SR_CCW", &fCCW, 0.0f, 1.0f, 0.0f, -IM_PI, 12.0f );

						static float fCCW2 = 0.75f;
						ImWidgets::SliderRingFloat( "CCW Full##SR_CCW2", &fCCW2, 0.0f, 1.0f, IM_PI, -IM_PI, 6.0f );
					}
					DW_SsRecord( "SliderRing", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
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
						static const ImVec2 arcUp[4] = { ImVec2( 0.0f, 0.8f ), ImVec2( 0.25f, 0.0f ), ImVec2( 0.75f, 0.0f ), ImVec2( 1.0f, 0.8f ) };
						static float fval2 = 0.3f;
						ImWidgets::SliderSplineFloat( "Arc Up##SS3", &fval2, 0.0f, 1.0f, arcUp );

						// Arc down
						static const ImVec2 arcDown[4] = { ImVec2( 0.0f, 0.2f ), ImVec2( 0.25f, 1.0f ), ImVec2( 0.75f, 1.0f ), ImVec2( 1.0f, 0.2f ) };
						static float fval3 = 0.7f;
						ImWidgets::SliderSplineFloat( "Arc Down##SS4", &fval3, 0.0f, 1.0f, arcDown );

						// Straight line
						static const ImVec2 straight[4] = { ImVec2( 0.0f, 0.5f ), ImVec2( 0.33f, 0.5f ), ImVec2( 0.66f, 0.5f ), ImVec2( 1.0f, 0.5f ) };
						static float fval4 = 0.5f;
						ImWidgets::SliderSplineFloat( "Straight##SS5", &fval4, -10.0f, 10.0f, straight );

						// Wave
						static const ImVec2 wave[4] = { ImVec2( 0.0f, 0.5f ), ImVec2( 0.15f, 0.0f ), ImVec2( 0.85f, 1.0f ), ImVec2( 1.0f, 0.5f ) };
						static float fval5 = 0.5f;
						ImWidgets::SliderSplineFloat( "Wave##SS6", &fval5, 0.0f, 100.0f, wave, 4, 80.0f, 6.0f );

						ImGui::Separator();
						ImGui::Text( "Loops:" );

						// Closed loop (circle-like, 2 bezier segments = 7 points)
						static const ImVec2 closedLoop[7] = {
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
						static const ImVec2 infinity[7] = {
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

						// Right-to-left variant: mirror the X axis so the spline flows
						// from 1.0 (left edge of the widget) down to 0.0 (right edge).
						// Toggleable so the user can A/B against the default L->R sense.
						static bool rtl = true;
						ImGui::Checkbox( "Right-to-left##SS_RTL", &rtl );
						static const ImVec2 arcUp_LTR[4] = { ImVec2( 0.0f, 0.8f ), ImVec2( 0.25f, 0.0f ), ImVec2( 0.75f, 0.0f ), ImVec2( 1.0f, 0.8f ) };
						static const ImVec2 arcUp_RTL[4] = { ImVec2( 1.0f, 0.8f ), ImVec2( 0.75f, 0.0f ), ImVec2( 0.25f, 0.0f ), ImVec2( 0.0f, 0.8f ) };
						static float fval_rtl = 0.25f;
						ImWidgets::SliderSplineFloat( "RTL Arc##SS_RTL", &fval_rtl, 0.0f, 1.0f,
													  rtl ? arcUp_RTL : arcUp_LTR );
					}
					DW_SsRecord( "SliderSpline", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "SliderGradient" ) )
					{
						static ImGradientData gradRainbow;
						static ImGradientData gradOkLch;
						static bool gradInit = false;
						if ( !gradInit )
						{
							// sRGB rainbow
							gradRainbow.Stops.clear();
							gradRainbow.Stops.push_back( { 0.00f, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) } );
							gradRainbow.Stops.push_back( { 0.33f, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) } );
							gradRainbow.Stops.push_back( { 0.66f, ImVec4( 0.0f, 0.0f, 1.0f, 1.0f ) } );
							gradRainbow.Stops.push_back( { 1.00f, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) } );
							gradRainbow.Interpolation = ImWidgetsGradientInterp_sRGB;
							// OkLCH blue-to-orange
							gradOkLch.Stops.clear();
							gradOkLch.Stops.push_back( { 0.0f, ImVec4( 0.1f, 0.25f, 0.85f, 1.0f ) } );
							gradOkLch.Stops.push_back( { 1.0f, ImVec4( 1.0f, 0.55f, 0.05f, 1.0f ) } );
							gradOkLch.Interpolation = ImWidgetsGradientInterp_OkLCH;
							gradInit = true;
						}

						static float gradFloat = 0.5f;
						static int   gradInt = 50;
						ImWidgets::SliderGradientFloat( "Float sRGB##SG", &gradFloat, 0.0f, 1.0f, &gradRainbow );
						ImWidgets::SliderGradientInt( "Int OkLCH##SG", &gradInt, 0, 100, &gradOkLch );

						// Fill up to cursor: callback-swap trick -- gradient renders
						// for t in [0, value], fully transparent past it, so the
						// FrameBg under the track is what reads past the cursor.
						static bool sg_fill = true;
						ImGui::Checkbox( "Fill gradient up to cursor##SG_FILL", &sg_fill );
						static float gradFillF = 0.35f;
						ImWidgets::SliderGradientFloat( "Float Fill##SG", &gradFillF, 0.0f, 1.0f, &gradRainbow, ImVec2( 0, 0 ), sg_fill );
						static int gradFillI = 60;
						ImWidgets::SliderGradientInt( "Int Fill##SG", &gradFillI, 0, 100, &gradOkLch, ImVec2( 0, 0 ), sg_fill );

						// Right-to-left variant: use the right_to_left flag -- grab
						// travels from the right edge (value=v_min) to the left
						// edge (value=v_max), and when fill_up_to_cursor is on the
						// fill grows from the right. Gradient colors keep their
						// natural positions (blue at 0, orange at 1 of the bar).
						static float gradFillF_RTL = 0.35f;
						ImWidgets::SliderGradientFloat( "Float Fill RTL##SG", &gradFillF_RTL, 0.0f, 1.0f, &gradRainbow, ImVec2( 0, 0 ), sg_fill, /*right_to_left=*/true );
						static int gradFillI_RTL = 60;
						ImWidgets::SliderGradientInt( "Int Fill RTL##SG", &gradFillI_RTL, 0, 100, &gradOkLch, ImVec2( 0, 0 ), sg_fill, /*right_to_left=*/true );

						// Range variant (two handles, cutoff on [lower, upper]).
						// Both min and max are user-controlled -- outside the range the
						// FrameBg shows through, same transparency trick as fill_up_to_cursor.
						ImGui::Separator();
						ImGui::TextUnformatted( "Range (cutoff min + max):" );
						static float gradRangeLo = 0.2f, gradRangeHi = 0.75f;
						ImWidgets::SliderGradientRangeFloat( "Float Range##SG", &gradRangeLo, &gradRangeHi, 0.0f, 1.0f, &gradRainbow );
						static int gradRangeLoI = 20, gradRangeHiI = 70;
						ImWidgets::SliderGradientRangeInt( "Int Range##SG", &gradRangeLoI, &gradRangeHiI, 0, 100, &gradOkLch );
						// Range + RTL: lower handle sits on the right edge at value=v_min.
						static float gradRangeLo_RTL = 0.25f, gradRangeHi_RTL = 0.8f;
						ImWidgets::SliderGradientRangeFloat( "Float Range RTL##SG", &gradRangeLo_RTL, &gradRangeHi_RTL, 0.0f, 1.0f, &gradRainbow, ImVec2( 0, 0 ), /*right_to_left=*/true );

						// Live-editable gradient + matching SliderGradient. The editor
						// has alpha enabled so the user can drag alpha stops too -- the
						// bar feeds that gradient straight into SliderGradientFloat so
						// the slider reacts in real time to stop edits.
						ImGui::Separator();
						ImGui::TextUnformatted( "Editable (alpha enabled):" );
						static ImGradientData gradEditSG;
						static bool gradEditSGInit = false;
						if ( !gradEditSGInit )
						{
							gradEditSG.Stops.clear();
							gradEditSG.Stops.push_back( { 0.00f, ImVec4( 0.1f, 0.25f, 0.85f, 1.0f ) } );
							gradEditSG.Stops.push_back( { 0.50f, ImVec4( 1.0f, 1.0f,  1.0f,  0.2f ) } );
							gradEditSG.Stops.push_back( { 1.00f, ImVec4( 1.0f, 0.55f, 0.05f, 1.0f ) } );
							gradEditSG.Interpolation = ImWidgetsGradientInterp_OkLab;
							gradEditSGInit = true;
						}
						ImWidgets::GradientEditor( "##SG_GradEdit", &gradEditSG, /*alpha=*/true );
						static float gradEditVal = 0.5f;
						ImWidgets::SliderGradientFloat( "Driven by editor##SG", &gradEditVal, 0.0f, 1.0f, &gradEditSG );
						ImWidgets::SliderGradientFloat( "Driven (fill)##SG",    &gradEditVal, 0.0f, 1.0f, &gradEditSG, ImVec2( 0, 0 ), /*fill_up_to_cursor=*/true );
					}
					DW_SsRecord( "SliderGradient", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "SliderSplineGradient" ) )
					{
						static ImGradientData gradSSG;
						static bool gradSSGInit = false;
						if ( !gradSSGInit )
						{
							gradSSG.Stops.clear();
							gradSSG.Stops.push_back( { 0.00f, ImVec4( 0.1f, 0.25f, 0.85f, 1.0f ) } );
							gradSSG.Stops.push_back( { 0.50f, ImVec4( 1.0f, 1.0f,  1.0f,  1.0f ) } );
							gradSSG.Stops.push_back( { 1.00f, ImVec4( 1.0f, 0.55f, 0.05f, 1.0f ) } );
							gradSSG.Interpolation = ImWidgetsGradientInterp_OkLab;
							gradSSGInit = true;
						}

						// S-curve (default)
						static float val1 = 0.5f;
						ImWidgets::SliderSplineGradientFloat( "S-Curve##SSG1", &val1, 0.0f, 1.0f, &gradSSG );

						// Arc up
						static const ImVec2 arcUp[4] = { ImVec2( 0.0f, 0.8f ), ImVec2( 0.25f, 0.0f ), ImVec2( 0.75f, 0.0f ), ImVec2( 1.0f, 0.8f ) };
						static float val2 = 0.3f;
						ImWidgets::SliderSplineGradientFloat( "Arc Up##SSG2", &val2, 0.0f, 1.0f, &gradSSG, arcUp );

						// Wave (2 bezier segments)
						static const ImVec2 wave[7] = {
							ImVec2( 0.0f,  0.5f ),
							ImVec2( 0.15f, 0.0f ),
							ImVec2( 0.35f, 0.0f ),
							ImVec2( 0.5f,  0.5f ),
							ImVec2( 0.65f, 1.0f ),
							ImVec2( 0.85f, 1.0f ),
							ImVec2( 1.0f,  0.5f )
						};
						static float val3 = 0.5f;
						ImWidgets::SliderSplineGradientFloat( "Wave##SSG3", &val3, 0.0f, 100.0f, &gradSSG, wave, 7, 80.0f, 6.0f );

						// Int slider with straight bezier control points
						static const ImVec2 straight[4] = { ImVec2( 0.0f, 0.5f ), ImVec2( 0.33f, 0.5f ), ImVec2( 0.66f, 0.5f ), ImVec2( 1.0f, 0.5f ) };
						static int intVal = 50;
						ImWidgets::SliderSplineGradientInt( "Int Straight##SSG4", &intVal, 0, 100, &gradSSG, straight );

						// "Gradient follows cursor": rebuild the gradient each frame so
						// the visible color band only fills up to the current value, and
						// beyond the cursor it drops to a near-black desaturated tone.
						// Color-channel driven (not just alpha) so the split is obvious
						// against any background.
						// Fill-up-to-cursor is now a built-in feature of SliderSplineGradient --
						// pass the flag and the widget handles the gradient compression +
						// plain track rendering past the cursor internally.
						static bool grad_fill = true;
						ImGui::Checkbox( "Fill gradient up to cursor##SSG_FILL", &grad_fill );
						static float valFill = 0.35f;
						ImWidgets::SliderSplineGradientFloat(
							"Fill##SSG_FILL", &valFill, 0.0f, 1.0f, &gradSSG,
							nullptr, 4, 0.0f, 0.0f, "%.3f",
							grad_fill );

						// Right-to-left variant: mirror control points across the X axis
						// so the spline arc flows from right to left. Works transparently
						// with fill_up_to_cursor -- as the cursor grows, the filled arc
						// walks right-to-left across the widget.
						static const ImVec2 arcUp_RTL_SSG[ 4 ] = {
							ImVec2( 1.0f, 0.8f ), ImVec2( 0.75f, 0.0f ),
							ImVec2( 0.25f, 0.0f ), ImVec2( 0.0f, 0.8f )
						};
						static float valFillRTL = 0.35f;
						ImWidgets::SliderSplineGradientFloat(
							"Fill RTL##SSG_FILL", &valFillRTL, 0.0f, 1.0f, &gradSSG,
							arcUp_RTL_SSG, 4, 0.0f, 0.0f, "%.3f",
							grad_fill );

						// Range along the spline -- two handles, gradient painted only
						// on [lo, hi]. Same cut callback the "Fill" variant uses.
						ImGui::Separator();
						ImGui::TextUnformatted( "Range (cutoff min + max):" );
						static float ssgRangeLo = 0.2f, ssgRangeHi = 0.75f;
						ImWidgets::SliderSplineGradientRangeFloat( "Float Range##SSG", &ssgRangeLo, &ssgRangeHi, 0.0f, 1.0f, &gradSSG );
						// On the Arc-Up control points:
						static float ssgRangeLoArc = 0.15f, ssgRangeHiArc = 0.65f;
						ImWidgets::SliderSplineGradientRangeFloat( "Arc Range##SSG", &ssgRangeLoArc, &ssgRangeHiArc, 0.0f, 1.0f, &gradSSG, arcUp );
						static int ssgRangeLoI = 20, ssgRangeHiI = 70;
						ImWidgets::SliderSplineGradientRangeInt( "Int Range##SSG", &ssgRangeLoI, &ssgRangeHiI, 0, 100, &gradSSG );

						ImGui::Separator();
						ImGui::TextUnformatted( "Editable (alpha enabled):" );
						static ImGradientData gradEditSSG;
						static bool gradEditSSGInit = false;
						if ( !gradEditSSGInit )
						{
							gradEditSSG.Stops.clear();
							gradEditSSG.Stops.push_back( { 0.00f, ImVec4( 0.85f, 0.15f, 0.45f, 1.0f ) } );
							gradEditSSG.Stops.push_back( { 0.50f, ImVec4( 1.0f,  1.0f,  1.0f,  0.25f ) } );
							gradEditSSG.Stops.push_back( { 1.00f, ImVec4( 0.15f, 0.65f, 0.95f, 1.0f ) } );
							gradEditSSG.Interpolation = ImWidgetsGradientInterp_OkLab;
							gradEditSSGInit = true;
						}
						ImWidgets::GradientEditor( "##SSG_GradEdit", &gradEditSSG, /*alpha=*/true );
						static float gradEditValSSG = 0.5f;
						ImWidgets::SliderSplineGradientFloat(
							"Driven by editor##SSG", &gradEditValSSG, 0.0f, 1.0f, &gradEditSSG );
						ImWidgets::SliderSplineGradientFloat(
							"Driven (fill)##SSG", &gradEditValSSG, 0.0f, 1.0f, &gradEditSSG,
							nullptr, 4, 0.0f, 0.0f, "%.3f", /*fill_up_to_cursor=*/true );
					}
					DW_SsRecord( "SliderSplineGradient", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "SliderRing (Gradient)" ) )
					{
						static ImGradientData ringHueGrad;
						static ImGradientData ringTempGrad;
						static bool ringGradInit = false;
						if ( !ringGradInit )
						{
							ringHueGrad.Stops.clear();
							ringHueGrad.Stops.push_back( { 0.00f, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) } );
							ringHueGrad.Stops.push_back( { 0.33f, ImVec4( 0.0f, 1.0f, 0.0f, 1.0f ) } );
							ringHueGrad.Stops.push_back( { 0.66f, ImVec4( 0.0f, 0.0f, 1.0f, 1.0f ) } );
							ringHueGrad.Stops.push_back( { 1.00f, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) } );
							ringHueGrad.Interpolation = ImWidgetsGradientInterp_OkLCH;

							ringTempGrad.Stops.clear();
							ringTempGrad.Stops.push_back( { 0.0f, ImVec4( 0.3f, 0.55f, 1.0f, 1.0f ) } );
							ringTempGrad.Stops.push_back( { 0.5f, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) } );
							ringTempGrad.Stops.push_back( { 1.0f, ImVec4( 1.0f, 0.65f, 0.2f, 1.0f ) } );
							ringTempGrad.Interpolation = ImWidgetsGradientInterp_OkLab;
							ringGradInit = true;
						}

						static float hueVal = 0.3f;
						static float tempVal = 0.5f;
						static int   meter = 40;

						// Previous 3x bump was too large at high DPI; reduced by 2.5x.
						const float SRG_R = ImPlatform_LpToPx( 62.0f );
						const float SRG_TH = ImPlatform_LpToPx( 16.0f );
						// Render labels on one row, ring sliders on the next, using a 3-column table.
						if ( ImGui::BeginTable( "##SRG_Basic", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings ) )
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Hue" );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Temp" );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Meter" );
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImWidgets::SliderGradientRingFloat( "##SRG_Hue", &hueVal, 0.0f, 1.0f, &ringHueGrad, SRG_R, SRG_TH );
							ImGui::TableNextColumn();
							ImWidgets::SliderGradientRingFloat( "##SRG_Temp", &tempVal, 0.0f, 1.0f, &ringTempGrad, SRG_R, SRG_TH, IM_PI, IM_PI );
							ImGui::TableNextColumn();
							ImWidgets::SliderGradientRingInt( "##SRG_Meter", &meter, 0, 100, &ringTempGrad, SRG_R, SRG_TH, 0.75f * IM_PI, 1.5f * IM_PI );
							ImGui::EndTable();
						}

						// Fill up to cursor -- same callback-swap approach applied to
						// the arc primitive. Gradient paints only on [0, value];
						// past the cursor the arc's FrameBg fill shows through.
						static bool srg_fill = true;
						ImGui::Checkbox( "Fill gradient up to cursor##SRG_FILL", &srg_fill );
						static float hueFill  = 0.35f;
						static float tempFill = 0.6f;
						static int   meterFill = 30;
						if ( ImGui::BeginTable( "##SRG_Fill", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings ) )
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Hue Fill" );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Temp Fill" );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Meter Fill" );
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImWidgets::SliderGradientRingFloat( "##SRG_HueFill", &hueFill, 0.0f, 1.0f, &ringHueGrad, SRG_R, SRG_TH, srg_fill );
							ImGui::TableNextColumn();
							ImWidgets::SliderGradientRingFloat( "##SRG_TempFill", &tempFill, 0.0f, 1.0f, &ringTempGrad, SRG_R, SRG_TH, IM_PI, IM_PI, srg_fill );
							ImGui::TableNextColumn();
							ImWidgets::SliderGradientRingInt( "##SRG_MeterFill", &meterFill, 0, 100, &ringTempGrad, SRG_R, SRG_TH, 0.75f * IM_PI, 1.5f * IM_PI, srg_fill );
							ImGui::EndTable();
						}

						// Right-to-left variants: negate the sweep angle so the arc
						// progresses counter-clockwise. Works transparently with
						// fill_up_to_cursor -- the fill walks RTL around the ring.
						static float hueFill_RTL  = 0.35f;
						static float tempFill_RTL = 0.6f;
						static int   meterFill_RTL = 30;
						if ( ImGui::BeginTable( "##SRG_RTL", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings ) )
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Hue Fill RTL" );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Temp Fill RTL" );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Meter Fill RTL" );
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							// Full ring RTL: start at -PI/2 (top), sweep = -2PI (full circle CCW).
							ImWidgets::SliderGradientRingFloat( "##SRG_HueRTL", &hueFill_RTL, 0.0f, 1.0f, &ringHueGrad, SRG_R, SRG_TH, -0.5f * IM_PI, -2.0f * IM_PI, srg_fill );
							ImGui::TableNextColumn();
							// Half-ring RTL: start at 2PI, sweep = -PI.
							ImWidgets::SliderGradientRingFloat( "##SRG_TempRTL", &tempFill_RTL, 0.0f, 1.0f, &ringTempGrad, SRG_R, SRG_TH, 2.0f * IM_PI, -IM_PI, srg_fill );
							ImGui::TableNextColumn();
							// 3/4 meter RTL: start at 2.25PI, sweep = -1.5PI.
							ImWidgets::SliderGradientRingInt( "##SRG_MeterRTL", &meterFill_RTL, 0, 100, &ringTempGrad, SRG_R, SRG_TH, 2.25f * IM_PI, -1.5f * IM_PI, srg_fill );
							ImGui::EndTable();
						}

						// Ring range -- arcs only, not full-circle (range-across-wrap is
						// ambiguous). Two handles bound the cut gradient along the arc.
						ImGui::Separator();
						ImGui::TextUnformatted( "Range (arcs only, cutoff min + max):" );
						static float rangeHueLo = 0.1f,  rangeHueHi = 0.6f;
						static float rangeTempLo = 0.2f, rangeTempHi = 0.8f;
						static int   rangeMeterLo = 20, rangeMeterHi = 70;
						if ( ImGui::BeginTable( "##SRG_Range", 3, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings ) )
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Temp Range" );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Meter Range" );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Hue Range" );
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							// Half-ring temperature: start=PI, sweep=PI.
							ImWidgets::SliderGradientRingRangeFloat( "##SRG_TempRange", &rangeTempLo, &rangeTempHi, 0.0f, 1.0f, &ringTempGrad, SRG_R, SRG_TH, IM_PI, IM_PI );
							ImGui::TableNextColumn();
							// 3/4 meter: start=0.75PI, sweep=1.5PI.
							ImWidgets::SliderGradientRingRangeInt( "##SRG_MeterRange", &rangeMeterLo, &rangeMeterHi, 0, 100, &ringTempGrad, SRG_R, SRG_TH, 0.75f * IM_PI, 1.5f * IM_PI );
							ImGui::TableNextColumn();
							// 3/4 hue arc (not full): start=-0.75PI, sweep=1.5PI.
							ImWidgets::SliderGradientRingRangeFloat( "##SRG_HueRange", &rangeHueLo, &rangeHueHi, 0.0f, 1.0f, &ringHueGrad, SRG_R, SRG_TH, -0.75f * IM_PI, 1.5f * IM_PI );
							ImGui::EndTable();
						}

						ImGui::Separator();
						ImGui::TextUnformatted( "Editable (alpha enabled):" );
						static ImGradientData gradEditSRG;
						static bool gradEditSRGInit = false;
						if ( !gradEditSRGInit )
						{
							gradEditSRG.Stops.clear();
							gradEditSRG.Stops.push_back( { 0.00f, ImVec4( 0.95f, 0.25f, 0.35f, 1.0f ) } );
							gradEditSRG.Stops.push_back( { 0.50f, ImVec4( 1.0f,  1.0f,  1.0f,  0.2f ) } );
							gradEditSRG.Stops.push_back( { 1.00f, ImVec4( 0.25f, 0.65f, 0.95f, 1.0f ) } );
							gradEditSRG.Interpolation = ImWidgetsGradientInterp_OkLCH;
							gradEditSRGInit = true;
						}
						ImWidgets::GradientEditor( "##SRG_GradEdit", &gradEditSRG, /*alpha=*/true );
						static float gradEditValSRG = 0.5f;
						if ( ImGui::BeginTable( "##SRG_Driven", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings ) )
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Driven by editor" );
							ImGui::TableNextColumn(); ImGui::TextUnformatted( "Driven (fill)" );
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImWidgets::SliderGradientRingFloat( "##SRG_Driven", &gradEditValSRG, 0.0f, 1.0f, &gradEditSRG, SRG_R, SRG_TH );
							ImGui::TableNextColumn();
							ImWidgets::SliderGradientRingFloat( "##SRG_DrivenFill", &gradEditValSRG, 0.0f, 1.0f, &gradEditSRG, SRG_R, SRG_TH, /*fill_up_to_cursor=*/true );
							ImGui::EndTable();
						}
					}
					DW_SsRecord( "SliderRing_Gradient", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Slider2D Float" ) )
					{
						static ImVec2 slider2D;
						ImVec2 boundMin( -1.0f, -1.0f );
						ImVec2 boundMax( 1.0f, 1.0f );
						Slider2DFloat( "Slider 2D Float", &slider2D.x, &slider2D.y, boundMin.x, boundMax.x, boundMin.y, boundMax.y );
						ImGui::InputFloat2( "Value", &slider2D.x );
					}
					DW_SsRecord( "Slider2D_Float", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Slider2D Int" ) )
					{
						static int vv[2];
						Slider2DInt( "Slider 2D Int", &vv[0], &vv[1], -5, 5, -5, 5 );
						ImGui::InputInt2( "Value", &vv[0] );
					}
					DW_SsRecord( "Slider2D_Int", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Slider2D Range Float" ) )
					{
						static float rMinX = -0.5f, rMinY = -0.5f, rMaxX = 0.5f, rMaxY = 0.5f;
						Slider2DRangeFloat( "Range 2D Float", &rMinX, &rMinY, &rMaxX, &rMaxY, -1.0f, 1.0f, -1.0f, 1.0f );
						ImGui::InputFloat2( "Min", &rMinX );
						ImGui::InputFloat2( "Max", &rMaxX );
					}
					DW_SsRecord( "Slider2D_RangeFloat", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Slider2D Range Int" ) )
					{
						static int riMinX = -2, riMinY = -2, riMaxX = 2, riMaxY = 2;
						Slider2DRangeInt( "Range 2D Int", &riMinX, &riMinY, &riMaxX, &riMaxY, -5, 5, -5, 5 );
						ImGui::InputInt2( "Min", &riMinX );
						ImGui::InputInt2( "Max", &riMaxX );
					}
					DW_SsRecord( "Slider2D_RangeInt", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Slider2D Disc Float" ) )
					{
						static float jx = 0.0f, jy = 0.0f;
						Slider2DDiscFloat( "Disc Float", &jx, &jy, -1.0f, 1.0f );
						ImGui::InputFloat2( "Value", &jx );
						ImGui::Text( "Radius: %.3f", sqrtf( jx * jx + jy * jy ) );
					}
					DW_SsRecord( "Slider2D_DiscFloat", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Slider2D Disc Int" ) )
					{
						static int dix = 0, diy = 0;
						Slider2DDiscInt( "Disc Int", &dix, &diy, -10, 10 );
						ImGui::InputInt2( "Value", &dix );
					}
					DW_SsRecord( "Slider2D_DiscInt", _sy0, ImGui::GetCursorPos().y );
				}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Text##Widgets" ) )
			{
#if IMPLATFORM_GFX_SUPPORT_CUSTOM_SHADER
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Slug Text" ) )
					{
						static char   slug_buf[256] = "Dear Widgets";
						static float  slug_sz = 32.0f;
						static ImVec4 slug_col_v( 0.95f, 0.88f, 0.60f, 1.0f );
						static ImU32  slug_col_u = ImGui::ColorConvertFloat4ToU32( slug_col_v );
						static ImVec4 slug_hi_v( 0.40f, 0.80f, 1.00f, 1.0f );
						static ImU32  slug_hi_u = ImGui::ColorConvertFloat4ToU32( slug_hi_v );

						ImGui::InputText( "Text##SlugWidget", slug_buf, sizeof( slug_buf ) );
						ImGui::DragFloat( "Size##SlugWidget", &slug_sz, 0.5f, 8.0f, 120.0f, "%.0f lp" );
						if ( ImGui::ColorEdit4( "Color##SlugWidget", &slug_col_v.x ) )
							slug_col_u = ImGui::ColorConvertFloat4ToU32( slug_col_v );
						if ( ImGui::ColorEdit4( "Highlight##SlugWidget", &slug_hi_v.x ) )
							slug_hi_u = ImGui::ColorConvertFloat4ToU32( slug_hi_v );

						ImFont* slugFont = g_cinzelFont ? g_cinzelFont : g_firaCodeFont;
						if ( slugFont )
						{
							float wrap_w = ImGui::GetContentRegionAvail().x;

							ImGui::Separator();
							ImGui::TextDisabled( "SlugText()" );
							SlugText( slugFont, slug_sz, slug_col_u, slug_buf );

							ImGui::TextDisabled( "SlugTextColored()" );
							SlugTextColored( slugFont, slug_sz, slug_hi_v, slug_buf );

							ImGui::TextDisabled( "SlugTextWrapped()" );
							SlugTextWrapped( slugFont, slug_sz, slug_col_u, slug_buf, wrap_w );
						}
						else
						{
							ImGui::TextDisabled( "No font loaded." );
						}

						// -- Arabic --
						ImFont* fa = g_amiriFont;
						if ( fa )
						{
							static char   ar_buf[256] = "\xd8\xa7\xd9\x84\xd8\xa3\xd8\xaf\xd9\x88\xd8\xa7\xd8\xaa \xd8\xa7\xd9\x84\xd8\xb9\xd8\xb2\xd9\x8a\xd8\xb2\xd8\xa9";
							static float  ar_sz = 24.0f;
							float         wrap_ar = ImGui::GetContentRegionAvail().x;

							ImGui::Separator();
							ImGui::TextDisabled( "Arabic SlugText() -- right-aligned" );
							ImGui::DragFloat( "Arabic Size##SlugAr", &ar_sz, 0.5f, 8.0f, 72.0f, "%.0f lp" );
							ImGui::InputText( "Arabic Text##SlugAr", ar_buf, sizeof( ar_buf ) );

							{
								float asc = 0.0f;
								ImVec2 sz = ImWidgets::CalcTextSize( fa, ar_sz, ar_buf, nullptr, &asc );
								ImVec2 pos = ImGui::GetCursorScreenPos();
								ImWidgets::DrawText( ImGui::GetWindowDrawList(), fa, ar_sz,
													 ImVec2( pos.x + wrap_ar - sz.x, pos.y + asc ),
													 slug_col_u, ar_buf );
								ImGui::Dummy( ImVec2( wrap_ar, sz.y ) );
							}

							ImGui::TextDisabled( "SlugTextColored()" );
							{
								float asc = 0.0f;
								ImVec2 sz = ImWidgets::CalcTextSize( fa, ar_sz, ar_buf, nullptr, &asc );
								ImVec2 pos = ImGui::GetCursorScreenPos();
								ImWidgets::DrawText( ImGui::GetWindowDrawList(), fa, ar_sz,
													 ImVec2( pos.x + wrap_ar - sz.x, pos.y + asc ),
													 ImGui::ColorConvertFloat4ToU32( slug_hi_v ), ar_buf );
								ImGui::Dummy( ImVec2( wrap_ar, sz.y ) );
							}

							ImGui::TextDisabled( "SlugTextWrapped() right-align" );
							SlugTextWrapped( fa, ar_sz, slug_col_u, ar_buf, wrap_ar, /*right_align=*/true );
						}
					}
					DW_SsRecord( "Slug_Text", _sy0, ImGui::GetCursorPos().y );
				}
#else
				ImGui::TextDisabled( "Requires custom shader support." );
#endif
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Text on Path" ) )
					{
						static float top_amp = 30.0f, top_phase = 0.0f;
						static char  top_text[ 128 ] = "DearWidgets text on a sinusoidal path!";
						ImGui::SliderFloat( "Amplitude##TOP", &top_amp, 0.0f, 80.0f );
						ImGui::SliderFloat( "Phase##TOP",     &top_phase, 0.0f, IM_PI * 2.0f );
						ImGui::InputText  ( "Text##TOP", top_text, sizeof( top_text ) );
						ImDrawList* dl = ImGui::GetWindowDrawList();
						ImVec2 p = ImGui::GetCursorScreenPos();
						float W = ImGui::GetContentRegionAvail().x;
						float row_h = ImPlatform_LpToPx( 120.0f );
						ImGui::Dummy( ImVec2( W, row_h ) );
						const int N = 64;
						ImVec2 path[ N ];
						for ( int i = 0; i < N; ++i )
						{
							float u = (float)i / (float)( N - 1 );
							path[ i ].x = p.x + 16.0f + u * ( W - 32.0f );
							path[ i ].y = p.y + row_h * 0.55f + ImSin( u * IM_PI * 2.0f + top_phase ) * top_amp;
						}
						dl->AddPolyline( path, N, IM_COL32( 80, 80, 100, 220 ), ImDrawFlags_None, 1.0f );
						ImWidgets::DrawTextOnPath( dl, ImGui::GetFont(), 24.0f, top_text,
							path, N, IM_COL32( 230, 230, 230, 255 ) );
					}
					DW_SsRecord( "Text_On_Path", _sy0, ImGui::GetCursorPos().y );
				}
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Font Waterfall" ) )
					{
						static char  fw_text[ 128 ] = "The quick brown fox jumps over the lazy dog.";
						ImGui::InputText( "Text##FW", fw_text, sizeof( fw_text ) );
						float sizes[] = { 10, 12, 14, 16, 20, 24, 32, 48, 64 };
						ImDrawList* dl = ImGui::GetWindowDrawList();
						ImVec2 p = ImGui::GetCursorScreenPos();
						float final_y = ImWidgets::DrawFontWaterfall( dl, ImGui::GetFont(), p, fw_text,
							sizes, IM_ARRAYSIZE( sizes ),
							IM_COL32( 230, 230, 230, 255 ), 6.0f );
						ImGui::Dummy( ImVec2( ImGui::GetContentRegionAvail().x, final_y - p.y ) );
					}
					DW_SsRecord( "Font_Waterfall", _sy0, ImGui::GetCursorPos().y );
				}
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Variable Font Axis Sliders" ) )
					{
						static ImWidgets::ImWidgetsVarFontAxis vfa_axes[] = {
							{ "wght", "Weight",   100.0f,  900.0f, 400.0f, 400.0f },
							{ "wdth", "Width",     75.0f,  125.0f, 100.0f, 100.0f },
							{ "ital", "Italic",     0.0f,    1.0f,   0.0f,   0.0f },
							{ "opsz", "Optical sz", 8.0f,   48.0f,  16.0f,  16.0f },
						};
						ImWidgets::VarFontAxisSliders( "##VFA", vfa_axes, IM_ARRAYSIZE( vfa_axes ),
							ImGui::GetFont(), "DearWidgets Aa Bb 123" );
					}
					DW_SsRecord( "Variable_Font_Axis", _sy0, ImGui::GetCursorPos().y );
				}
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Kerning Pair Editor" ) )
					{
						static char  k_left  = 'A';
						static char  k_right = 'V';
						static float k_kern  = -4.0f;
						ImGui::PushItemWidth( 80 );
						ImGui::InputScalar( "Left",  ImGuiDataType_S8, &k_left );
						ImGui::SameLine();
						ImGui::InputScalar( "Right", ImGuiDataType_S8, &k_right );
						ImGui::PopItemWidth();
						ImWidgets::KerningPairEditor( "##KP", ImGui::GetFont(), 96.0f, k_left, k_right, &k_kern );
					}
					DW_SsRecord( "Kerning_Pair_Editor", _sy0, ImGui::GetCursorPos().y );
				}
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Text inside Shape" ) )
					{
						static char tis_text[ 512 ] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
							"Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. "
							"Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi.";
						static float tis_font_size = 14.0f;
						ImGui::InputTextMultiline( "##tis", tis_text, sizeof( tis_text ),
							ImVec2( ImGui::GetContentRegionAvail().x, 60.0f ) );
						ImGui::SliderFloat( "Font size (px)##TIS", &tis_font_size, 6.0f, 48.0f, "%.0f" );
						ImDrawList* dl = ImGui::GetWindowDrawList();
						ImVec2 p = ImGui::GetCursorScreenPos();
						float W = ImGui::GetContentRegionAvail().x;
						float row_h = ImPlatform_LpToPx( 220.0f );
						ImGui::Dummy( ImVec2( W, row_h ) );
						ImRect rect( p.x + 8.0f, p.y + 8.0f, p.x + W * 0.48f, p.y + row_h - 8.0f );
						dl->AddRectFilled( rect.Min, rect.Max, IM_COL32( 30, 50, 70, 255 ) );
						dl->AddRect      ( rect.Min, rect.Max, IM_COL32( 120, 160, 200, 255 ) );
						ImWidgets::DrawTextInsideRect( dl, ImGui::GetFont(), tis_font_size, rect, tis_text,
							IM_COL32( 220, 230, 240, 255 ) );
						// convex (hexagon)
						ImVec2 c( p.x + W * 0.75f, p.y + row_h * 0.5f );
						float r = ImMin( W * 0.22f, row_h * 0.45f );
						ImVec2 hex[ 6 ];
						for ( int k = 0; k < 6; ++k )
						{
							float a = ( (float)k / 6.0f ) * IM_PI * 2.0f + IM_PI * 0.5f;
							hex[ k ] = ImVec2( c.x + ImCos( a ) * r, c.y + ImSin( a ) * r );
						}
						dl->AddConvexPolyFilled( hex, 6, IM_COL32( 70, 30, 50, 255 ) );
						dl->AddPolyline( hex, 6, IM_COL32( 220, 120, 180, 255 ), ImDrawFlags_Closed, 1.0f );
						ImWidgets::DrawTextInsideConvex( dl, ImGui::GetFont(), tis_font_size, hex, 6, tis_text,
							IM_COL32( 240, 220, 240, 255 ) );
					}
					DW_SsRecord( "Text_Inside_Shape", _sy0, ImGui::GetCursorPos().y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Images##Widgets" ) )
			{

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Image Carousel" ) )
					{
						static int carouselIdx = 0;
						ImTextureID carouselImages[] = { astro_img, clock_img, man_img, illlustration_img, bike_img };
						ImVec2 carouselSizes[] = { astro_size, clock_size, man_size, illlustration_size, bike_size };
						ImWidgets::ImageCarousel( "##Carousel", carouselImages, carouselSizes, IM_ARRAYSIZE( carouselImages ), &carouselIdx );
						ImGui::Text( "Selected: %d", carouselIdx );
					}
					DW_SsRecord( "Image_Carousel", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Image Bento" ) )
					{
						static int   bentoIdx = 0;
						static int   bentoColumns = 3;
						static float bentoAspect = 1.0f;
						// Use static so reorder swaps persist between frames. Stable
						// `pItemIds` are required for clean drag-reorder -- without
						// them ImGui's per-cell ID is positional and a swap would
						// teleport input focus.
						static ImTextureID bentoImages[] = { astro_img, clock_img, man_img, illlustration_img, bike_img };
						static ImVec2      bentoSizes[] = { astro_size, clock_size, man_size, illlustration_size, bike_size };
						static const char* bentoIds[] = { "astro", "clock", "man", "illustration", "bike" };
						ImGui::SliderInt( "Columns##Bento", &bentoColumns, 1, 6 );
						ImGui::SliderFloat( "Aspect (W/H)##Bento", &bentoAspect, 0.25f, 4.0f, "%.2f" );

						int reorderFrom = -1, reorderTo = -1;
						ImWidgets::ImageBento( "##Bento", bentoImages, bentoSizes,
											   IM_ARRAYSIZE( bentoImages ), &bentoIdx,
											   bentoColumns, bentoAspect, /*spacing=*/4.0f,
											   bentoIds, &reorderFrom, &reorderTo );
						if ( reorderFrom >= 0 && reorderTo >= 0 &&
							 reorderFrom < IM_ARRAYSIZE( bentoImages ) &&
							 reorderTo < IM_ARRAYSIZE( bentoImages ) )
						{
							// ImageBento itself never mutates the arrays -- the caller
							// must perform the swap. Adjacent-only swaps make it a
							// straightforward std::swap on each parallel array.
							ImTextureID tImg = bentoImages[reorderFrom]; bentoImages[reorderFrom] = bentoImages[reorderTo]; bentoImages[reorderTo] = tImg;
							ImVec2      tSz = bentoSizes[reorderFrom]; bentoSizes[reorderFrom] = bentoSizes[reorderTo]; bentoSizes[reorderTo] = tSz;
							const char* tId = bentoIds[reorderFrom]; bentoIds[reorderFrom] = bentoIds[reorderTo]; bentoIds[reorderTo] = tId;
							// Keep the selection following the dragged item.
							if ( bentoIdx == reorderFrom ) bentoIdx = reorderTo;
							else if ( bentoIdx == reorderTo ) bentoIdx = reorderFrom;
						}
						ImGui::Text( "Selected: %d (drag a cell to reorder)", bentoIdx );
					}
					DW_SsRecord( "Image_Bento", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
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
						static stbi_uc* viewerCPU[5] = {};
						static bool        viewerCPULoaded = false;

						if ( !viewerCPULoaded )
						{
							viewerCPULoaded = true;
							for ( int i = 0; i < 5; i++ )
							{
								int w, h;
								viewerCPU[i] = stbi_load( viewerFiles[i], &w, &h, NULL, 4 );
							}
						}

						static int viewerIdx = 0;
						static ImImageViewerState viewerState;

						if ( ImGui::Combo( "Image##Viewer", &viewerIdx, viewerNames, 5 ) )
							viewerState = ImImageViewerState{};

						ImTextureID viewerTexes[] = { astro_img, clock_img, man_img, illlustration_img, bike_img };
						ImVec2      viewerSizes[] = { astro_size, clock_size, man_size, illlustration_size, bike_size };

						// Wire CPU buffer for current image (enables pixel value readback in inspector)
						viewerState.Pixels = viewerCPU[viewerIdx];
						viewerState.PixelSize = viewerSizes[viewerIdx];
						viewerState.PixelFormat = ImPlatform_PixelFormat_RGBA8;

						ImGui::Text( "Scroll: zoom  |  Left-drag: pan  |  Dbl-click: reset  |  Right-click: inspect" );
						ImGui::BeginChild( "##iv_75", ImVec2( ImGui::GetContentRegionAvail().x * 0.75f, 0 ), ImGuiChildFlags_AutoResizeY );
						ImWidgets::ImageViewer( "##Viewer", viewerTexes[viewerIdx], viewerSizes[viewerIdx], viewerState );
						ImGui::EndChild();

					}
					DW_SsRecord( "Image_Viewer", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Image Overlays" ) )
					{
						ImGui::TextWrapped(
							"Read-only overlays composed on top of ImageViewer. All coordinates are UV [0,1] "
							"relative to the image, so overlays follow the viewer's pan/zoom automatically. "
							"Toggle each overlay independently. Overlays are driven from an overlay_callback passed "
							"to ImageViewer, so they also render inside its built-in expand-to-window modal (try the "
							"expand button in the viewer's corner) -- not just the compact view." );

						// --- Shared viewer for all overlays ---
						static ImImageViewerState ovState;
						static const char* ovClassNames[] = { "person", "cat", "dog", "car", "book" };
						static const char* ovPartNames[]  = {
							"head", "l.shoulder", "r.shoulder", "l.elbow", "r.elbow",
							"l.hand", "r.hand", "l.hip", "r.hip", "l.knee",
							"r.knee", "l.foot", "r.foot"
						};

						// Synthetic detection boxes (UV coords)
						static ImWidgets::ImDetectionBox ovBoxes[] = {
							{ 0.10f, 0.15f, 0.30f, 0.45f, 0.94f, 0, 100 },
							{ 0.55f, 0.10f, 0.28f, 0.30f, 0.87f, 3, 101 },
							{ 0.50f, 0.55f, 0.25f, 0.30f, 0.72f, 1, 102 },
							{ 0.05f, 0.65f, 0.35f, 0.30f, 0.61f, 2, 103 },
						};
						static const int ovBoxCount = (int)IM_ARRAYSIZE( ovBoxes );

						// Synthetic keypoints (13-part skeleton)
						static ImWidgets::ImKeypoint ovKps[] = {
							{ 0.50f, 0.22f, 0.95f, 0 },  // head
							{ 0.42f, 0.32f, 0.90f, 1 },  // l.shoulder
							{ 0.58f, 0.32f, 0.90f, 2 },  // r.shoulder
							{ 0.38f, 0.44f, 0.80f, 3 },  // l.elbow
							{ 0.62f, 0.44f, 0.80f, 4 },  // r.elbow
							{ 0.35f, 0.56f, 0.70f, 5 },  // l.hand
							{ 0.65f, 0.56f, 0.70f, 6 },  // r.hand
							{ 0.44f, 0.55f, 0.85f, 7 },  // l.hip
							{ 0.56f, 0.55f, 0.85f, 8 },  // r.hip
							{ 0.42f, 0.70f, 0.75f, 9 },  // l.knee
							{ 0.58f, 0.70f, 0.75f, 10 }, // r.knee
							{ 0.40f, 0.85f, 0.25f, 11 }, // l.foot (low confidence)
							{ 0.60f, 0.85f, 0.65f, 12 }, // r.foot
						};
						static const int ovKpCount = (int)IM_ARRAYSIZE( ovKps );

						static ImWidgets::ImSkeletonEdge ovEdges[] = {
							{ 0, 1 }, { 0, 2 },
							{ 1, 3 }, { 3, 5 },
							{ 2, 4 }, { 4, 6 },
							{ 1, 7 }, { 2, 8 },
							{ 7, 8 },
							{ 7, 9 }, { 9, 11 },
							{ 8, 10 }, { 10, 12 },
						};
						static const int ovEdgeCount = (int)IM_ARRAYSIZE( ovEdges );

						// Synthetic text labels
						static ImWidgets::ImTextLabel ovLabels[] = {
							{ 0.25f, 0.60f, "top-left anchor",  IM_COL32( 255, 220, 100, 255 ), 1.0f, ImWidgets::ImTextLabelAnchor_TopLeft },
							{ 0.70f, 0.25f, "top-right",        IM_COL32( 100, 220, 255, 255 ), 1.0f, ImWidgets::ImTextLabelAnchor_TopRight },
							{ 0.50f, 0.05f, "center header",    IM_COL32( 255, 255, 255, 255 ), 1.2f, ImWidgets::ImTextLabelAnchor_TopCenter },
							{ 0.50f, 0.95f, "bottom center",    IM_COL32( 200, 255, 200, 255 ), 1.0f, ImWidgets::ImTextLabelAnchor_BotCenter },
						};
						static const int ovLabelCount = (int)IM_ARRAYSIZE( ovLabels );

						// Annotation editor: caller owns the mutable boxes array
						static ImVector<ImWidgets::ImDetectionBox> ovEditBoxes;
						static ImWidgets::ImAnnotationEditorState  ovEditor;
						static int ovNextUserId = 200;
						if ( ovEditBoxes.empty() )
						{
							ImWidgets::ImDetectionBox init = { 0.20f, 0.20f, 0.20f, 0.20f, -1.0f, 0, ovNextUserId++ };
							ovEditBoxes.push_back( init );
						}

						// --- Toggle bar ---
						static bool ovShowDet   = true;
						static bool ovShowKp    = false;
						static bool ovShowText  = false;
						static bool ovShowEdit  = false;
						ImGui::Checkbox( "DetectionOverlay",   &ovShowDet );  ImGui::SameLine();
						ImGui::Checkbox( "KeypointOverlay",    &ovShowKp );   ImGui::SameLine();
						ImGui::Checkbox( "TextLabelOverlay",   &ovShowText ); ImGui::SameLine();
						ImGui::Checkbox( "AnnotationEditor",   &ovShowEdit );

						static ImWidgets::ImOverlayStyle ovStyle;
						if ( ImGui::TreeNode( "Style" ) )
						{
							ImGui::SliderFloat( "Box thickness (lp)",   &ovStyle.box_thickness,    0.5f, 6.0f );
							ImGui::SliderFloat( "Label font scale",     &ovStyle.label_font_scale, 0.5f, 2.5f );
							ImGui::SliderFloat( "Label bg alpha",       &ovStyle.label_bg_alpha,   0.0f, 1.0f );
							ImGui::SliderFloat( "Point radius (lp)",    &ovStyle.point_radius,     1.0f, 12.0f );
							ImGui::SliderFloat( "Edge thickness (lp)",  &ovStyle.edge_thickness,   0.5f, 6.0f );
							ImGui::Checkbox   ( "Show score",           &ovStyle.show_score );
							ImGui::SameLine();
							ImGui::Checkbox   ( "Show class label",     &ovStyle.show_class_label );
							ImGui::TreePop();
						}

						// Reuse the same image bank as the viewer demo
						static const char* ovFiles[] = {
							"astro.png", "clock.png", "man.png",
							"pexels-robert-bogdan-156165-1152351.jpg", "camera-542784_1280.png"
						};
						static const char* ovNames[] = { "Astronaut", "Clock", "Man", "Illustration", "Bike" };
						static int ovImgIdx = 0;
						IM_UNUSED( ovFiles );
						ImGui::Combo( "Image##Overlay", &ovImgIdx, ovNames, 5 );
						ImTextureID ovTexes[] = { astro_img, clock_img, man_img, illlustration_img, bike_img };
						ImVec2      ovSizes[] = { astro_size, clock_size, man_size, illlustration_size, bike_size };

						// Overlays are drawn from this callback rather than called manually
						// after ImageViewer, so ImageViewer can also invoke it inside its
						// built-in expand-to-window modal (using that instance's own
						// transform/draw-list) -- not just the compact widget below.
						// Captureless, so it converts to the plain function pointer
						// ImageViewer expects; it only touches `state` (its own reference
						// parameter -- ImageViewer passes the same ovState through
						// unmodified) and static locals of this demo function.
						static int ovLastHoveredId = -1;
						auto ovDrawOverlays = []( ImImageViewerState& state, void* )
						{
							if ( ovShowDet )
							{
								ovLastHoveredId = ImWidgets::DetectionOverlay( "##ovDet", state,
														ovBoxes, ovBoxCount,
														ovClassNames, IM_ARRAYSIZE( ovClassNames ),
														&ovStyle );
							}

							if ( ovShowKp )
							{
								ImWidgets::KeypointOverlay( "##ovKp", state,
														ovKps, ovKpCount,
														ovEdges, ovEdgeCount,
														ovPartNames, IM_ARRAYSIZE( ovPartNames ),
														&ovStyle );
							}

							if ( ovShowText )
							{
								ImWidgets::TextLabelOverlay( "##ovText", state, ovLabels, ovLabelCount );
							}

							if ( ovShowEdit )
							{
								ImWidgets::ImAnnotationResult r = ImWidgets::AnnotationEditor(
										"##ovEdit", state, ovEditor,
										ovEditBoxes.Data, ovEditBoxes.Size,
										ovClassNames, IM_ARRAYSIZE( ovClassNames ),
										&ovStyle );

								// Apply the returned edit
								switch ( r.action )
								{
									case ImWidgets::ImAnnotationAction_Move:
									case ImWidgets::ImAnnotationAction_Resize:
									{
										for ( int i = 0; i < ovEditBoxes.Size; i++ )
											if ( ovEditBoxes[ i ].user_id == r.user_id )
											{
												ovEditBoxes[ i ].x = r.new_value.x;
												ovEditBoxes[ i ].y = r.new_value.y;
												ovEditBoxes[ i ].w = r.new_value.w;
												ovEditBoxes[ i ].h = r.new_value.h;
												break;
											}
										break;
									}
									case ImWidgets::ImAnnotationAction_Create:
									{
										ImWidgets::ImDetectionBox nb = r.new_value;
										nb.class_id = 0;
										nb.user_id  = ovNextUserId++;
										nb.score    = -1.0f;
										ovEditBoxes.push_back( nb );
										ovEditor.SelectedId = nb.user_id;
										break;
									}
									case ImWidgets::ImAnnotationAction_Delete:
									{
										for ( int i = 0; i < ovEditBoxes.Size; i++ )
											if ( ovEditBoxes[ i ].user_id == r.user_id )
											{
												ovEditBoxes.erase( ovEditBoxes.Data + i );
												break;
											}
										break;
									}
									case ImWidgets::ImAnnotationAction_LabelEdit:
									{
										for ( int i = 0; i < ovEditBoxes.Size; i++ )
											if ( ovEditBoxes[ i ].user_id == r.user_id )
											{
												int cid = -1;
												for ( int c = 0; c < IM_ARRAYSIZE( ovClassNames ); c++ )
													if ( strcmp( ovClassNames[ c ], r.new_label ) == 0 ) { cid = c; break; }
												if ( cid >= 0 ) ovEditBoxes[ i ].class_id = cid;
												break;
											}
										break;
									}
									default: break;
								}
							}
						};

						ImWidgets::ImageViewer( "##OverlayViewer", ovTexes[ovImgIdx], ovSizes[ovImgIdx], ovState,
												ImVec2( ImGui::GetContentRegionAvail().x * 0.75f, 480.0f ),
												ovDrawOverlays, nullptr );

						if ( ovShowDet )
						{
							if ( ovLastHoveredId >= 0 )
								ImGui::TextDisabled( "hovered detection user_id: %d", ovLastHoveredId );
							else
								ImGui::TextDisabled( "hovered detection user_id: (none)" );
						}

						if ( ovShowEdit )
						{
							ImGui::TextDisabled( "Editor: %d boxes | selected=%d | hovered=%d",
								ovEditBoxes.Size, ovEditor.SelectedId, ovEditor.HoveredId );
							ImGui::TextDisabled(
								"Click empty=create | Click box=select | Drag=move | Drag handle=resize | "
								"Del=delete | Esc=cancel | Dbl-click=edit label | Right-click=menu | "
								"Ctrl=axis-lock | Shift=square" );
						}
					}
					DW_SsRecord( "Image_Overlays", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Image Inspector" ) )
					{
						// Synthetic test buffers covering several sample-type / channel combinations.
						// All bytes are kept alive in static storage so the inspector loupe can read them.
						static bool inspectorInit = false;
						static const int kU8W = 256, kU8H = 256;
						static unsigned char inspectorU8Checker[kU8W * kU8H * 4];
						static const int kHdrW = 256, kHdrH = 256;
						static float    inspectorHDR[kHdrW * kHdrH * 4];
						static const int kRadW = 128, kRadH = 128;
						static unsigned short inspectorF16Radial[kRadW * kRadH];
						static const int kPatW = 192, kPatH = 192;
						static signed short inspectorI16Pattern[kPatW * kPatH * 3];

						if ( !inspectorInit )
						{
							inspectorInit = true;
							// U8 RGBA checkerboard with hue gradient
							for ( int y = 0; y < kU8H; y++ )
							{
								for ( int x = 0; x < kU8W; x++ )
								{
									int   off = (y * kU8W + x) * 4;
									int   cell = ((x / 16) + (y / 16)) & 1;
									float hue = (float)x / (float)(kU8W - 1);
									float r = cell ? (0.5f + 0.5f * cosf( 6.2831853f * (hue + 0.0f / 3.0f) )) : 0.1f;
									float g = cell ? (0.5f + 0.5f * cosf( 6.2831853f * (hue + 1.0f / 3.0f) )) : 0.1f;
									float b = cell ? (0.5f + 0.5f * cosf( 6.2831853f * (hue + 2.0f / 3.0f) )) : 0.1f;
									inspectorU8Checker[off + 0] = (unsigned char)(ImClamp( r, 0.0f, 1.0f ) * 255.0f);
									inspectorU8Checker[off + 1] = (unsigned char)(ImClamp( g, 0.0f, 1.0f ) * 255.0f);
									inspectorU8Checker[off + 2] = (unsigned char)(ImClamp( b, 0.0f, 1.0f ) * 255.0f);
									inspectorU8Checker[off + 3] = 255;
								}
							}
							// HDR F32 RGBA gradient: x = exposure stops -10..+10, y = hue
							for ( int y = 0; y < kHdrH; y++ )
							{
								for ( int x = 0; x < kHdrW; x++ )
								{
									float stops = ((float)x / (float)(kHdrW - 1)) * 20.0f - 10.0f;
									float scale = powf( 2.0f, stops );
									float hue = (float)y / (float)(kHdrH - 1);
									float r = scale * (0.5f + 0.5f * cosf( 6.2831853f * (hue + 0.0f / 3.0f) ));
									float g = scale * (0.5f + 0.5f * cosf( 6.2831853f * (hue + 1.0f / 3.0f) ));
									float b = scale * (0.5f + 0.5f * cosf( 6.2831853f * (hue + 2.0f / 3.0f) ));
									int   off = (y * kHdrW + x) * 4;
									inspectorHDR[off + 0] = r;
									inspectorHDR[off + 1] = g;
									inspectorHDR[off + 2] = b;
									inspectorHDR[off + 3] = 1.0f;
								}
							}
							// F16 single-channel radial gradient (IEEE half encoding)
							for ( int y = 0; y < kRadH; y++ )
							{
								for ( int x = 0; x < kRadW; x++ )
								{
									float dx = ((float)x - kRadW * 0.5f) / (kRadW * 0.5f);
									float dy = ((float)y - kRadH * 0.5f) / (kRadH * 0.5f);
									float r = sqrtf( dx * dx + dy * dy );
									float v = ImClamp( 1.0f - r, 0.0f, 1.0f );
									// Encode float -> half (simple, no denormal handling)
									unsigned int fb;
									memcpy( &fb, &v, 4 );
									unsigned int sign = (fb >> 31) & 0x1u;
									int          exp = (int)((fb >> 23) & 0xFFu) - 127;
									unsigned int mant = fb & 0x7FFFFFu;
									unsigned short h;
									if ( exp <= -15 )      h = (unsigned short)(sign << 15);
									else if ( exp >= 16 )  h = (unsigned short)((sign << 15) | (0x1Fu << 10));
									else                   h = (unsigned short)((sign << 15) | ((unsigned int)(exp + 15) << 10) | (mant >> 13));
									inspectorF16Radial[y * kRadW + x] = h;
								}
							}
							// I16 RGB test pattern: gradient + center spike
							for ( int y = 0; y < kPatH; y++ )
							{
								for ( int x = 0; x < kPatW; x++ )
								{
									int   off = (y * kPatW + x) * 3;
									float r = (float)x / (float)(kPatW - 1);
									float g = (float)y / (float)(kPatH - 1);
									float dx = ((float)x - kPatW * 0.5f) / (kPatW * 0.5f);
									float dy = ((float)y - kPatH * 0.5f) / (kPatH * 0.5f);
									float b = ImClamp( 1.0f - sqrtf( dx * dx + dy * dy ), 0.0f, 1.0f );
									inspectorI16Pattern[off + 0] = (signed short)(r * 32767.0f);
									inspectorI16Pattern[off + 1] = (signed short)(g * 32767.0f);
									inspectorI16Pattern[off + 2] = (signed short)(b * 32767.0f);
								}
							}
						}

						// Buffer descriptors (Halide-style strides)
						static const char* inspectorNames[] = {
							"U8 Checker (sRGB)", "F32 HDR Gradient", "F16 Radial", "I16 Pattern"
						};
						const int kInspectorCount = 4;
						static int inspectorIdx = 0;
						static ImImageInspectorState inspectorState;
						static ImU64 inspectorU8Version = 1;
						static ImU64 inspectorHdrVersion = 1;
						static ImU64 inspectorRadVersion = 1;
						static ImU64 inspectorPatVersion = 1;

						// Build current ImImageBuffer based on selection
						ImImageBuffer buf;
						memset( &buf, 0, sizeof( buf ) );
						if ( inspectorIdx == 0 )
						{
							buf.host = inspectorU8Checker;
							buf.byte_offset = 0;
							buf.width = kU8W;
							buf.height = kU8H;
							buf.channels = 4;
							buf.x_stride_bytes = 4;
							buf.y_stride_bytes = 4 * (ptrdiff_t)kU8W;
							buf.c_stride_bytes = 1;
							buf.type = ImSampleType_U8;
							buf.version = inspectorU8Version;
						}
						else if ( inspectorIdx == 1 )
						{
							buf.host = inspectorHDR;
							buf.byte_offset = 0;
							buf.width = kHdrW;
							buf.height = kHdrH;
							buf.channels = 4;
							buf.x_stride_bytes = 16;
							buf.y_stride_bytes = 16 * (ptrdiff_t)kHdrW;
							buf.c_stride_bytes = 4;
							buf.type = ImSampleType_F32;
							buf.version = inspectorHdrVersion;
						}
						else if ( inspectorIdx == 2 )
						{
							buf.host = inspectorF16Radial;
							buf.byte_offset = 0;
							buf.width = kRadW;
							buf.height = kRadH;
							buf.channels = 1;
							buf.x_stride_bytes = 2;
							buf.y_stride_bytes = 2 * (ptrdiff_t)kRadW;
							buf.c_stride_bytes = 2;
							buf.type = ImSampleType_F16;
							buf.version = inspectorRadVersion;
						}
						else
						{
							buf.host = inspectorI16Pattern;
							buf.byte_offset = 0;
							buf.width = kPatW;
							buf.height = kPatH;
							buf.channels = 3;
							buf.x_stride_bytes = 6;
							buf.y_stride_bytes = 6 * (ptrdiff_t)kPatW;
							buf.c_stride_bytes = 2;
							buf.type = ImSampleType_I16;
							buf.version = inspectorPatVersion;
						}

						if ( ImGui::Combo( "Buffer##Inspector", &inspectorIdx, inspectorNames, kInspectorCount ) )
						{
							// Reset view but keep the texture cache around -- version mismatch will trigger re-upload
							inspectorState.Zoom = 1.0f;
							inspectorState.Pan = ImVec2( 0, 0 );
						}

						// View-transform controls
						ImGui::PushItemWidth( 220 );
						const char* xferNames[] = {
							"Linear", "Gamma", "sRGB", "Rec.709", "Rec.1886", "Cineon",
							"S-Log2", "S-Log3", "LogC3", "LogC4",
							"Canon Log", "Canon Log 2", "Canon Log 3",
							"V-Log", "Log3G10", "BMFilm Gen5", "Apple Log",
							"F-Log", "D-Log", "PQ", "HLG"
						};
						IM_STATIC_ASSERT( IM_ARRAYSIZE( xferNames ) == ImImageInspector_Transfer_COUNT );
						ImGui::Combo( "Input Transfer", &inspectorState.InputTransfer, xferNames, IM_ARRAYSIZE( xferNames ) );

						const char* outXferNames[] = { "Linear", "Gamma", "sRGB", "PQ", "HLG" };
						IM_STATIC_ASSERT( IM_ARRAYSIZE( outXferNames ) == ImImageInspector_OutputTransfer_COUNT );
						ImGui::Combo( "Output Transfer", &inspectorState.OutputTransfer, outXferNames, IM_ARRAYSIZE( outXferNames ) );

						const char* gamutNames[] = {
							"Rec.709", "Rec.2020", "DCI-P3", "Display P3", "Adobe RGB",
							"ProPhoto", "ACES AP0", "ACES AP1"
						};
						IM_STATIC_ASSERT( IM_ARRAYSIZE( gamutNames ) == ImImageInspector_Gamut_COUNT );
						ImGui::Combo( "Input Gamut", &inspectorState.InputGamut, gamutNames, IM_ARRAYSIZE( gamutNames ) );
						ImGui::Combo( "Working Gamut", &inspectorState.WorkingGamut, gamutNames, IM_ARRAYSIZE( gamutNames ) );
						ImGui::Combo( "Output Gamut", &inspectorState.OutputGamut, gamutNames, IM_ARRAYSIZE( gamutNames ) );

						const char* tonemapNames[] = {
							"None", "Reinhard", "Reinhard Ext", "ACES Filmic", "AGX", "PBR Neutral", "Hable"
						};
						IM_STATIC_ASSERT( IM_ARRAYSIZE( tonemapNames ) == ImImageInspector_Tonemap_COUNT );
						ImGui::Combo( "Tonemap", &inspectorState.Tonemap, tonemapNames, IM_ARRAYSIZE( tonemapNames ) );

						const char* filterNames[] = {
							"Nearest", "Bilinear", "Bicubic Mitchell", "Bicubic Catmull-Rom", "Lanczos2", "Lanczos3"
						};
						IM_STATIC_ASSERT( IM_ARRAYSIZE( filterNames ) == ImImageInspector_Filter_COUNT );
						ImGui::Combo( "Filter", &inspectorState.Filter, filterNames, IM_ARRAYSIZE( filterNames ) );

						const char* fcNames[] = {
							"Off", "Viridis", "Magma", "Inferno", "Plasma", "Cividis", "Turbo", "Cinematographer", "Out of Gamut"
						};
						IM_STATIC_ASSERT( IM_ARRAYSIZE( fcNames ) == ImImageInspector_FalseColor_COUNT );
						ImGui::Combo( "False Color", &inspectorState.FalseColor, fcNames, IM_ARRAYSIZE( fcNames ) );

						ImGui::SliderFloat( "Exposure (stops)", &inspectorState.Exposure, -10.0f, 10.0f );
						ImGui::SliderFloat( "Black", &inspectorState.Black, -1.0f, 1.0f );
						ImGui::SliderFloat( "White", &inspectorState.White, 0.0f, 4.0f );
						ImGui::SliderFloat( "Gamma", &inspectorState.Gamma, 0.1f, 4.0f );
						ImGui::SliderFloat( "Temperature", &inspectorState.Temperature, -1.0f, 1.0f );
						ImGui::SliderFloat( "Tint", &inspectorState.Tint, -1.0f, 1.0f );
						ImGui::PopItemWidth();

						bool maskR = inspectorState.ChannelMask[0] > 0.5f;
						bool maskG = inspectorState.ChannelMask[1] > 0.5f;
						bool maskB = inspectorState.ChannelMask[2] > 0.5f;
						bool maskA = inspectorState.ChannelMask[3] > 0.5f;
						ImGui::Checkbox( "R", &maskR ); ImGui::SameLine();
						ImGui::Checkbox( "G", &maskG ); ImGui::SameLine();
						ImGui::Checkbox( "B", &maskB ); ImGui::SameLine();
						ImGui::Checkbox( "A", &maskA );
						inspectorState.ChannelMask[0] = maskR ? 1.0f : 0.0f;
						inspectorState.ChannelMask[1] = maskG ? 1.0f : 0.0f;
						inspectorState.ChannelMask[2] = maskB ? 1.0f : 0.0f;
						inspectorState.ChannelMask[3] = maskA ? 1.0f : 0.0f;

						ImGui::ColorEdit4( "NaN Color", &inspectorState.NaNColor.x, ImGuiColorEditFlags_NoInputs );

						if ( ImGui::Button( "Inject NaN at center" ) && inspectorIdx == 1 )
						{
							int   cx = kHdrW / 2;
							int   cy = kHdrH / 2;
							int   off = (cy * kHdrW + cx) * 4;
							unsigned int nan_bits = 0x7FC00000u;
							float nan_val;
							memcpy( &nan_val, &nan_bits, 4 );
							inspectorHDR[off + 0] = nan_val;
							inspectorHdrVersion += 1;
						}
						ImGui::SameLine();
						if ( ImGui::Button( "Regenerate HDR" ) )
						{
							for ( int y = 0; y < kHdrH; y++ )
								for ( int x = 0; x < kHdrW; x++ )
								{
									float stops = ((float)x / (float)(kHdrW - 1)) * 20.0f - 10.0f;
									float scale = powf( 2.0f, stops );
									float hue = (float)y / (float)(kHdrH - 1);
									float r = scale * (0.5f + 0.5f * cosf( 6.2831853f * (hue + 0.0f / 3.0f) ));
									float g = scale * (0.5f + 0.5f * cosf( 6.2831853f * (hue + 1.0f / 3.0f) ));
									float b = scale * (0.5f + 0.5f * cosf( 6.2831853f * (hue + 2.0f / 3.0f) ));
									int   off = (y * kHdrW + x) * 4;
									inspectorHDR[off + 0] = r;
									inspectorHDR[off + 1] = g;
									inspectorHDR[off + 2] = b;
									inspectorHDR[off + 3] = 1.0f;
								}
							inspectorHdrVersion += 1;
						}

						ImGui::Text( "Scroll: zoom  |  Left-drag: pan  |  Dbl-click: reset  |  Right-click: inspect" );
						ImGui::BeginChild( "##ii_75", ImVec2( ImGui::GetContentRegionAvail().x * 0.75f, 0 ), ImGuiChildFlags_AutoResizeY );
						ImWidgets::ImageInspector( "##Inspector", buf, inspectorState );
						ImGui::EndChild();
					}
					DW_SsRecord( "Image_Inspector", _sy0, ImGui::GetCursorPos().y );
				}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Drawing Tools##Widgets" ) )
			{

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Up Vector" ) )
					{
						// Four convention variants — each variant's "up" axis is shown vertically
						// in the hemisphere render, with the dome facing the viewer.
						// Handedness only flips the sign of one horizontal axis on read-back.
						struct Variant { const char* label; int upAxis; bool leftHanded; };
						static const Variant variants[] = {
							{ "Y-up RH (OpenGL / Maya)", 1, false },
							{ "Y-up LH (Unity / DX)",    1, true  },
							{ "Z-up RH (Blender / Max)", 2, false },
							{ "Z-up LH (Unreal)",        2, true  },
						};

						static float upDirs[ IM_ARRAYSIZE( variants ) ][ 3 ] = {
							{ 0.0f, 1.0f, 0.0f },
							{ 0.0f, 1.0f, 0.0f },
							{ 0.0f, 0.0f, 1.0f },
							{ 0.0f, 0.0f, 1.0f },
						};

						for ( int i = 0; i < IM_ARRAYSIZE( variants ); ++i )
						{
							ImGui::PushID( i );
							ImGui::TextUnformatted( variants[ i ].label );
							// For LH conventions flip the non-up horizontal axis when handing
							// the direction to the widget (which works in RH internally), and
							// undo that flip on write-back. The flipped axis is the one that
							// isn't the up axis and isn't X (so Z for Y-up, Y for Z-up).
							int flipIdx = ( variants[ i ].upAxis == 1 ) ? 2 : 1;
							float dirIn[ 3 ] = { upDirs[ i ][ 0 ], upDirs[ i ][ 1 ], upDirs[ i ][ 2 ] };
							if ( variants[ i ].leftHanded ) dirIn[ flipIdx ] = -dirIn[ flipIdx ];
							if ( ImWidgets::UpVector( "##UpVec", dirIn, variants[ i ].upAxis ) )
							{
								if ( variants[ i ].leftHanded ) dirIn[ flipIdx ] = -dirIn[ flipIdx ];
								upDirs[ i ][ 0 ] = dirIn[ 0 ];
								upDirs[ i ][ 1 ] = dirIn[ 1 ];
								upDirs[ i ][ 2 ] = dirIn[ 2 ];
							}
							ImGui::Text( "Direction: %.3f, %.3f, %.3f", upDirs[ i ][ 0 ], upDirs[ i ][ 1 ], upDirs[ i ][ 2 ] );
							ImGui::Separator();
							ImGui::PopID();
						}
					}
					DW_SsRecord( "Up_Vector", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Paint Canvas" ) )
					{
						// User-owned pixel buffers (different aspect ratios)
						static unsigned char maskPixels[64 * 64];            // 1:1
						static unsigned char grayPixels[60 * 70];            // 6:7
						static ImU32         colorPixels[160 * 90];          // 16:9
						static float         floatPixels[128 * 64 * 4];     // 2:1

						static ImPaintCanvasData canvases[4];
						static bool paintInit = false;
						if ( !paintInit )
						{
							memset( maskPixels, 0, sizeof( maskPixels ) );
							memset( grayPixels, 0, sizeof( grayPixels ) );
							memset( colorPixels, 0, sizeof( colorPixels ) );
							memset( floatPixels, 0, sizeof( floatPixels ) );

							canvases[0].Pixels = maskPixels;   canvases[0].Width = 64;  canvases[0].Height = 64; canvases[0].Format = ImPlatform_PixelFormat_R8;      canvases[0].Mode = ImPaintMode_BinaryMask;
							canvases[1].Pixels = grayPixels;   canvases[1].Width = 60;  canvases[1].Height = 70; canvases[1].Format = ImPlatform_PixelFormat_R8;      canvases[1].Mode = ImPaintMode_Grayscale;
							canvases[2].Pixels = colorPixels;  canvases[2].Width = 160; canvases[2].Height = 90; canvases[2].Format = ImPlatform_PixelFormat_RGBA8;   canvases[2].Mode = ImPaintMode_Color;
							canvases[3].Pixels = floatPixels;  canvases[3].Width = 128; canvases[3].Height = 64; canvases[3].Format = ImPlatform_PixelFormat_RGBA32F; canvases[3].Mode = ImPaintMode_Color;
							paintInit = true;
						}

						static int activePaint = 2;
						ImGui::Combo( "Format##paint", &activePaint, "Binary Mask (R8)\0Grayscale (R8)\0Color (RGBA8)\0Color (RGBA32F)\0" );
						ImPaintCanvasData& pc = canvases[activePaint];

						PaintCanvas( "##PaintMain", &pc, ImVec2( 480, 0 ) );

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
					DW_SsRecord( "Paint_Canvas", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Transform Gizmo" ) )
					{
						// --- Images ---
						static ImTransformData  gizmoTransforms[3];
						static ImVec2           gizmoSizes[3];
						static ImTextureID      gizmoTextures[3];
						static bool gizmoInit = false;
						if ( !gizmoInit )
						{
							gizmoTextures[0] = astro_img;  gizmoSizes[0] = astro_size;
							gizmoTextures[1] = clock_img;  gizmoSizes[1] = clock_size;
							gizmoTransforms[1].Translation = ImVec2( -120.0f, -60.0f );
							gizmoTransforms[1].Scale = ImVec2( 0.4f, 0.4f );
							gizmoTextures[2] = man_img;    gizmoSizes[2] = man_size;
							gizmoTransforms[2].Translation = ImVec2( 100.0f, 50.0f );
							gizmoTransforms[2].Scale = ImVec2( 0.5f, 0.5f );
							gizmoInit = true;
						}

						static int gizmoSel = 0;
						static bool gizmoNonUniform = false;

						ImGui::Checkbox( "Non-Uniform Scale", &gizmoNonUniform );

						ImTransformGizmoFlags gizmoFlags = ImTransformGizmoFlags_None;
						if ( gizmoNonUniform )
							gizmoFlags |= ImTransformGizmoFlags_NonUniformScale;

						auto DrawImage = []( const ImTransformGizmoDrawParams& p, void* ud ){
							ImTextureID* textures = (ImTextureID*)ud;
							p.DrawList->AddImageQuad( textures[p.Index],
													  p.Corners[0], p.Corners[1], p.Corners[2], p.Corners[3],
													  ImVec2( 0, 0 ), ImVec2( 1, 0 ), ImVec2( 1, 1 ), ImVec2( 0, 1 ) );
							};
						auto SwapImage = []( int a, int b, void* ud ){
							ImTextureID* textures = (ImTextureID*)ud;
							ImSwap( textures[a], textures[b] );
							};
						ImTransformGizmoCallbacks imgCB;
						imgCB.DrawFn = DrawImage;
						imgCB.DrawData = gizmoTextures;
						imgCB.SwapFn = SwapImage;
						imgCB.SwapData = gizmoTextures;
						ImWidgets::TransformGizmo( "##xform", gizmoTransforms, gizmoSizes, 3, &gizmoSel, &imgCB, gizmoFlags );

						if ( gizmoSel >= 0 && gizmoSel < 3 )
						{
							ImTransformData* tr = &gizmoTransforms[gizmoSel];
							ImGui::Text( "Selected: %d", gizmoSel );
							float halfW = ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.x;
							ImGui::SetNextItemWidth( halfW );
							ImGui::DragFloat2( "Position", &tr->Translation.x, 1.0f );
							ImGui::SameLine();
							float deg = tr->Rotation * (180.0f / IM_PI);
							ImGui::SetNextItemWidth( halfW );
							if ( ImGui::DragFloat( "Rotation", &deg, 0.5f ) )
								tr->Rotation = deg * (IM_PI / 180.0f);
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

						ImGui::Separator();
						ImGui::TextUnformatted( "Shapes" );

						// --- Shapes: drawn with ImDrawList, no textures needed ---
						// shapeTypes[] owns the shape identity per slot so it follows layer reordering.
						struct ShapeDrawData
						{
							int* types; ImU32* colors;
						};
						static int   shapeTypes[3] = { 0, 1, 2 };          // 0=circle, 1=quad, 2=triangle
						static ImU32 shapeColorsMut[3] = {
							IM_COL32( 90,  170, 255, 255 ),  // circle   - blue
							IM_COL32( 255, 165, 60,  255 ),  // quad     - orange
							IM_COL32( 100, 220, 100, 255 ),  // triangle - green
						};
						static ShapeDrawData shapeDrawData = { shapeTypes, shapeColorsMut };
						static ImTransformData shapeTransforms[3];
						static ImVec2          shapeSizes[3];
						static bool shapeInit = false;
						if ( !shapeInit )
						{
							const float SZ = 128.0f;
							shapeSizes[0] = shapeSizes[1] = shapeSizes[2] = ImVec2( SZ, SZ );
							shapeTransforms[1].Translation = ImVec2( -120.0f, -50.0f );
							shapeTransforms[2].Translation = ImVec2( 110.0f, 40.0f );
							shapeInit = true;
						}

						static int shapeSel = 0;

						auto DrawShape = []( const ImTransformGizmoDrawParams& p, void* ud ){
							const ShapeDrawData* d = (const ShapeDrawData*)ud;
							int  type = d->types[p.Index];
							ImU32 col = d->colors[p.Index];
							if ( type == 0 )
							{
								const int N = 32;
								ImVec2 pts[N];
								for ( int i = 0; i < N; ++i )
								{
									float a = (float)i / N * 2.0f * IM_PI;
									float lx = ImCos( a ) * p.HalfW;
									float ly = ImSin( a ) * p.HalfH;
									pts[i] = ImVec2( p.Center.x + lx * p.CosR - ly * p.SinR,
													 p.Center.y + lx * p.SinR + ly * p.CosR );
								}
								p.DrawList->AddConvexPolyFilled( pts, N, col );
							}
							else if ( type == 1 )
							{
								p.DrawList->AddQuadFilled( p.Corners[0], p.Corners[1], p.Corners[2], p.Corners[3], col );
							}
							else
							{
								float lpts[3][2] = { { 0, -p.HalfH }, { -p.HalfW, p.HalfH }, { p.HalfW, p.HalfH } };
								ImVec2 pts[3];
								for ( int i = 0; i < 3; ++i )
								{
									float lx = lpts[i][0], ly = lpts[i][1];
									pts[i] = ImVec2( p.Center.x + lx * p.CosR - ly * p.SinR,
													 p.Center.y + lx * p.SinR + ly * p.CosR );
								}
								p.DrawList->AddTriangleFilled( pts[0], pts[1], pts[2], col );
							}
							};
						auto SwapShape = []( int a, int b, void* ud ){
							ShapeDrawData* d = (ShapeDrawData*)ud;
							ImSwap( d->types[a], d->types[b] );
							ImSwap( d->colors[a], d->colors[b] );
							};
						ImTransformGizmoCallbacks shapeCB;
						shapeCB.DrawFn = DrawShape;
						shapeCB.DrawData = &shapeDrawData;
						shapeCB.SwapFn = SwapShape;
						shapeCB.SwapData = &shapeDrawData;
						ImWidgets::TransformGizmo( "##xformShapes", shapeTransforms, shapeSizes, 3, &shapeSel, &shapeCB, gizmoFlags );

						if ( shapeSel >= 0 && shapeSel < 3 )
						{
							static const char* shapeNames[] = { "Circle", "Quad", "Triangle" };
							ImTransformData* tr = &shapeTransforms[shapeSel];
							ImGui::Text( "Selected: %s", shapeNames[shapeTypes[shapeSel]] );
							float halfW = ImGui::GetContentRegionAvail().x * 0.5f - ImGui::GetStyle().ItemSpacing.x;
							ImGui::SetNextItemWidth( halfW );
							ImGui::DragFloat2( "Position##shapes", &tr->Translation.x, 1.0f );
							ImGui::SameLine();
							float deg = tr->Rotation * (180.0f / IM_PI);
							ImGui::SetNextItemWidth( halfW );
							if ( ImGui::DragFloat( "Rotation##shapes", &deg, 0.5f ) )
								tr->Rotation = deg * (IM_PI / 180.0f);
							ImGui::SetNextItemWidth( halfW );
							ImGui::DragFloat2( "Scale##shapes", &tr->Scale.x, 0.01f, 0.01f, 10.0f );
							ImGui::SameLine();
							if ( ImGui::Button( "Reset##shapes" ) )
								*tr = ImTransformData();
						}
						else
						{
							ImGui::TextDisabled( "Click a shape to select it" );
						}
					}
					DW_SsRecord( "Transform_Gizmo", _sy0, ImGui::GetCursorPos().y );
				}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Color Editing##Widgets" ) )
			{

				{
					float _sy0 = ImGui::GetCursorPos().y;
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
					DW_SsRecord( "Hue_Selector", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
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
						ImGui::SameLine();
						if ( ImGui::Checkbox( "Split Alpha##GradEditor", &gradient.SplitAlpha ) )
						{
							gradient.SelectedAlphaIdx = -1;
						}

						GradientEditor( "##GradientMain", &gradient, gradAlpha, ImVec2( 0, 32 ) );

						static char const* interpNames[] = { "sRGB", "Linear sRGB", "OkLab", "OkLCH", "HSV" };
						ImGui::Combo( "Interpolation##GradEditor", &gradient.Interpolation, interpNames, ImWidgetsGradientInterp_COUNT );

						// Edit selected color stop
						if ( gradient.SelectedIdx >= 0 && gradient.SelectedIdx < gradient.Stops.Size )
						{
							ImGradientStop& stop = gradient.Stops[gradient.SelectedIdx];
							ImGui::Text( "Color Stop %d  Position: %.3f", gradient.SelectedIdx, stop.Position );
							ImGuiColorEditFlags ceFlags = (gradAlpha && !gradient.SplitAlpha) ? (ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf) : ImGuiColorEditFlags_NoAlpha;
							ImGui::ColorEdit4( "Stop Color##GradEditor", &stop.Color.x, ceFlags );
						}
						else if ( gradient.SplitAlpha && gradient.SelectedAlphaIdx >= 0 && gradient.SelectedAlphaIdx < gradient.AlphaStops.Size )
						{
							ImGradientAlphaStop& astop = gradient.AlphaStops[gradient.SelectedAlphaIdx];
							ImGui::Text( "Alpha Stop %d  Position: %.3f", gradient.SelectedAlphaIdx, astop.Position );
							ImGui::SliderFloat( "Alpha##GradAlphaStop", &astop.Alpha, 0.0f, 1.0f );
						}
						else
						{
							ImGui::TextDisabled( "No stop selected" );
						}

						ImGui::TextWrapped( "Click bar to add stop. Drag to move. Double-click to edit. Right-click for context menu. Drag far away to remove." );

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
					DW_SsRecord( "Gradient_Editor", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
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
							ImCurveEditorKey& key = curve.Keys[curve.SelectedIdx];
							float cePosMin = (curve.SelectedIdx > 0) ? curve.Keys[curve.SelectedIdx - 1].Pos.x : curve.RangeMin.x;
							float cePosMax = (curve.SelectedIdx < curve.Keys.Size - 1) ? curve.Keys[curve.SelectedIdx + 1].Pos.x : curve.RangeMax.x;
							ImGui::DragFloat2( "Position##CurveKey", &key.Pos.x, 0.01f );
							key.Pos.x = ImClamp( key.Pos.x, cePosMin, cePosMax );

							// Segment type combo
							int currentSeg = key.Segment;
							if ( ImGui::BeginCombo( "Segment##CurveKey", ImWidgets::CurveEditorSegName( (ImCurveEditorSeg)currentSeg ) ) )
							{
								for ( int s = 0; s < ImCurveEditorSeg_COUNT; ++s )
								{
									bool isSelected = (currentSeg == s);
									if ( ImGui::Selectable( ImWidgets::CurveEditorSegName( (ImCurveEditorSeg)s ), isSelected ) )
										key.Segment = (ImCurveEditorSeg)s;
									if ( isSelected )
										ImGui::SetItemDefaultFocus();
								}
								ImGui::EndCombo();
							}

							// Tangent mode combo (only when bezier handles are relevant)
							bool hasBezier = (key.Segment == ImCurveEditorSeg_CubicBezier)
								|| (curve.SelectedIdx > 0 && curve.Keys[curve.SelectedIdx - 1].Segment == ImCurveEditorSeg_CubicBezier);
							if ( hasBezier )
							{
								int currentMode = key.TangentMode;
								if ( ImGui::BeginCombo( "Tangent Mode##CurveKey", ImWidgets::CurveEditorTangentModeName( (ImCurveEditorTangentMode)currentMode ) ) )
								{
									for ( int m = 0; m < ImCurveEditorTangentMode_COUNT; ++m )
									{
										bool isSelected = (currentMode == m);
										if ( ImGui::Selectable( ImWidgets::CurveEditorTangentModeName( (ImCurveEditorTangentMode)m ), isSelected ) )
										{
											key.TangentMode = (ImCurveEditorTangentMode)m;
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
					DW_SsRecord( "Curve_Editor", _sy0, ImGui::GetCursorPos().y );
				}

				{ float _sy0 = ImGui::GetCursorPos().y;
				ApplyOpenAll();
				if ( ImGui::CollapsingHeader( "Color Wheel" ) )
				{
					static ImVec4 wheelColor( 0.8f, 0.2f, 0.3f, 1.0f );
					static int wheelMode = ImColorWheelMode_HSV;

					ImGui::Combo( "Mode##Wheel", &wheelMode, "HSV\0OkLCH\0" );

					ColorWheel( "##WheelMain", &wheelColor, (ImColorWheelMode)wheelMode );

					ImGui::ColorEdit4( "Color##Wheel", &wheelColor.x, ImGuiColorEditFlags_Float );

					ImGui::Separator();

					// Second wheel: OkLCH with HDR slider
					static ImVec4 wheelColor2( 0.5f, 0.7f, 0.2f, 1.0f );
					ImGui::Text( "OkLCH Wheel (HDR max = 2.0)" );
					ColorWheel( "##WheelHDR", &wheelColor2, ImColorWheelMode_OkLCH, 2.0f );
					ImGui::ColorEdit4( "HDR Color##Wheel2", &wheelColor2.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR );
				}
				DW_SsRecord( "Color_Wheel", _sy0, ImGui::GetCursorPos().y ); }

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Color Picker" ) )
					{
						static ImVec4 pickerColor( 0.4f, 0.7f, 0.3f, 1.0f );
						static int pickerSpace = ImColorPickerSpace_sRGB;
						static int srgbFixedAxis = 2;

						ImGui::Combo( "Space##Picker", &pickerSpace, "sRGB\0HSV\0OkLab\0OkLCH\0CIE Lab\0XYZ\0" );
						if ( pickerSpace == ImColorPickerSpace_sRGB )
							ImGui::Combo( "Fixed Axis##Picker", &srgbFixedAxis, "R (GB plane)\0G (RB plane)\0B (RG plane)\0" );

						ColorPicker( "##PickerMain", &pickerColor, (ImColorPickerSpace)pickerSpace, srgbFixedAxis );

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
					DW_SsRecord( "Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
				float _sy0_phys = ImGui::GetCursorPos().y;
				ApplyOpenAll();
				if ( ImGui::CollapsingHeader( "Physically-Based Color Pickers" ) )
				{

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Skin Color Picker (Biophysical)" ) )
					{
						static ImVec4 skinColor( 0.8f, 0.6f, 0.5f, 1.0f );
						ImGui::TextWrapped( "Physically-based skin tone from chromophores: melanin fraction (plane X), "
							"eumelanin/pheomelanin blend (plane Y), and dermal hemoglobin (vertical slider). "
							"Switch Mode to drive these from high-level age / gender / skin-care / skin-type controls." );

						DW_ReferenceLink( "http://graphics.ucsd.edu/~henrik/papers/skin_bssrdf/skin_bssrdf.pdf" );
						ColorPickerSkin( "##SkinPicker", &skinColor );

						ImGui::ColorEdit4( "Color##Skin", &skinColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Skin_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Hair Color Picker (Biophysical)" ) )
					{
						static ImVec4 hairColor( 0.35f, 0.22f, 0.12f, 1.0f );
						ImGui::TextWrapped( "Physically-based hair color from melanin (Marschner/d'Eon/Chiang fiber pigments): "
							"eumelanin amount (plane X, blonde->black) x pheomelanin amount (plane Y, none->ginger/auburn), "
							"with a redness (pheomelanin gain) vertical slider. Color is the single-strand Beer-Lambert "
							"transmittance of the colored TRT lobe (integrated across the fiber cross-section), so pheomelanin "
							"reads as a vivid ginger. Switch Mode to drive these from high-level shade / warmth / graying controls." );

						DW_ReferenceLink( "https://media.disneyanimation.com/uploads/production/publication_asset/152/asset/eurographics2016Fur_Smaller.pdf" );
						ColorPickerHair( "##HairPicker", &hairColor );

						ImGui::ColorEdit4( "Color##Hair", &hairColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Hair_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Leaf Color Picker (PROSPECT-D)" ) )
					{
						static ImVec4 leafColor( 0.25f, 0.45f, 0.12f, 1.0f );
						ImGui::TextWrapped( "Physically-based leaf color from the PROSPECT-D leaf optical model (Feret et al. 2017): "
							"chlorophyll a+b (plane X, green), carotenoids (plane Y, yellow/orange), and anthocyanins "
							"(vertical slider, red/purple). Covers the full lifecycle green->yellow->orange->red->brown. "
							"Switch Mode to drive these from high-level season / health / autumn-redness controls." );

						DW_ReferenceLink( "https://hal.science/hal-01584365/file/mt2017-pub00054524.pdf" );
						ColorPickerLeaf( "##LeafPicker", &leafColor );

						ImGui::ColorEdit4( "Color##Leaf", &leafColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Leaf_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Blackbody / Color Temperature (Planck)" ) )
					{
						static ImVec4 bbColor( 1.0f, 0.95f, 0.9f, 1.0f );
						ImGui::TextWrapped( "Planck's law along the Planckian locus (CIE 15). Single gradient slider keyed with blackbody colors: "
							"warm/orange at low Kelvin, white ~6500K, cool/blue at high Kelvin." );
						DW_ReferenceLink( "http://www.cvrl.org/cmfs.htm" );
						ColorPickerBlackbody( "##BBPicker", &bbColor );
						ImGui::ColorEdit4( "Color##BB", &bbColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Blackbody_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Pigment Mixing (Kubelka-Munk)" ) )
					{
						static ImVec4 pigColor( 0.3f, 0.5f, 0.2f, 1.0f );
						ImGui::TextWrapped( "Subtractive paint mixing via Kubelka-Munk theory (Kubelka & Munk 1931; Haase & Meyer 1992). "
							"Pick two paints; plane X = A->B mix, Y = white tint; slider = black. Yellow + blue makes green, like real paint." );
						DW_ReferenceLink( "https://doi.org/10.1145/146443.146452" );
						ColorPickerPigment( "##PigPicker", &pigColor );
						ImGui::ColorEdit4( "Color##Pig", &pigColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Pigment_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Gemstone (Crystal-Field Absorption)" ) )
					{
						static ImVec4 gemColor( 0.7f, 0.05f, 0.1f, 1.0f );
						ImGui::TextWrapped( "Gemstone body color from Beer-Lambert absorption by trace transition-metal ions (Nassau 1983; "
							"Fritsch & Rossman 1987-88). Pick a gem; plane X = concentration, Y = path length; slider = clarity." );
						DW_ReferenceLink( "https://www.gia.edu/dam/migrated-assets/docs/doc1/An-Update-on-Color-in-Gems-Part-1-Introduction-and-Colors-Caused-by-Dispersed-Metal-Ions.pdf" );
						ColorPickerGem( "##GemPicker", &gemColor );
						ImGui::ColorEdit4( "Color##Gem", &gemColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Gemstone_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Water / Ocean (Bio-Optical)" ) )
					{
						static ImVec4 waterColor( 0.0f, 0.2f, 0.35f, 1.0f );
						ImGui::TextWrapped( "Water color from a bio-optical model R ~ bb/(a+bb) (Morel & Prieur 1977; Gordon 1988) with pure-water "
							"absorption (Pope & Fry 1997), chlorophyll and CDOM. Plane X = chlorophyll, Y = CDOM; slider = turbidity." );
						DW_ReferenceLink( "https://omlc.org/spectra/water/abs/index.html" );
						ColorPickerWater( "##WaterPicker", &waterColor );
						ImGui::ColorEdit4( "Color##Water", &waterColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Water_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Iris / Eye Color (Melanin + Tyndall)" ) )
					{
						static ImVec4 irisColor( 0.3f, 0.45f, 0.6f, 1.0f );
						ImGui::TextWrapped( "Eye color: anterior melanin absorbs while the stroma scatters blue (Tyndall/Rayleigh) over a pigmented "
							"epithelium - so blue eyes are structural, not a blue pigment. Plane X = anterior melanin (blue->brown), "
							"Y = stromal scattering; slider = posterior melanin." );
						DW_ReferenceLink( "https://pubmed.ncbi.nlm.nih.gov/19619260/" );
						ColorPickerIris( "##IrisPicker", &irisColor );
						ImGui::ColorEdit4( "Color##Iris", &irisColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Iris_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Flame / Emission Spectrum" ) )
					{
						static ImVec4 flameColor( 0.2f, 0.8f, 0.3f, 1.0f );
						ImGui::TextWrapped( "Additive emission: a blackbody flame continuum plus atomic emission lines for the chosen element "
							"(NIST lines; flame tests). Plane X = flame temperature, Y = element line strength; slider = sodium contamination." );
						DW_ReferenceLink( "https://www.nist.gov/pml/atomic-spectra-database" );
						ColorPickerFlame( "##FlamePicker", &flameColor );
						ImGui::ColorEdit4( "Color##Flame", &flameColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Flame_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Bruise / Hematoma Healing" ) )
					{
						static ImVec4 bruiseColor( 0.5f, 0.2f, 0.3f, 1.0f );
						ImGui::TextWrapped( "Bruise color as it heals: extravasated hemoglobin deoxygenates (red->purple), then heme breaks "
							"down to biliverdin (green) and bilirubin (yellow). Chromophore dynamics after Randeberg et al. 2006, "
							"Lasers Surg. Med. Plane X = days since injury, Y = severity; slider = skin melanin." );
						DW_ReferenceLink( "https://pubmed.ncbi.nlm.nih.gov/16538661/" );
						ColorPickerBruise( "##BruisePicker", &bruiseColor );
						ImGui::ColorEdit4( "Color##Bruise", &bruiseColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Bruise_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Emission Nebula" ) )
					{
						static ImVec4 nebulaColor( 0.6f, 0.2f, 0.3f, 1.0f );
						ImGui::TextWrapped( "Color of an ionized gas cloud from its emission lines (Osterbrock & Ferland): hydrogen Balmer (Halpha), "
							"high-ionization [O III] (green) + He, and low-ionization [N II]/[S II] (red). Plane X = ionization, Y = low-ionization; "
							"slider = hydrogen strength." );
						DW_ReferenceLink( "https://www.nist.gov/pml/atomic-spectra-database" );
						ColorPickerNebula( "##NebulaPicker", &nebulaColor );
						ImGui::ColorEdit4( "Color##Nebula", &nebulaColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Nebula_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Maillard / Caramelization Browning" ) )
					{
						static ImVec4 maillardColor( 0.6f, 0.45f, 0.25f, 1.0f );
						ImGui::TextWrapped( "Food browning: melanoidin/caramel pigments accumulate with Arrhenius time-temperature kinetics and absorb "
							"toward short wavelengths (after Maillard-kinetics & CIELab browning studies). Plane X = temperature, Y = time; "
							"slider = sugar(caramel)<->protein(Maillard)." );
						DW_ReferenceLink( "https://doi.org/10.1016/S0924-2244(01)00022-X" );
						ColorPickerMaillard( "##MaillardPicker", &maillardColor );
						ImGui::ColorEdit4( "Color##Maillard", &maillardColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Maillard_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Copper / Bronze Patina" ) )
					{
						static ImVec4 patinaColor( 0.7f, 0.45f, 0.3f, 1.0f );
						ImGui::TextWrapped( "Atmospheric weathering of copper: bright metal -> cuprite/tarnish (brown) -> basic sulfate/carbonate patina "
							"(green) over years, faster in marine/industrial air (Graedel et al., Corrosion Science 1987). Plane X = age (years), "
							"Y = environment; slider = humidity." );
						DW_ReferenceLink( "https://doi.org/10.1016/0010-938X(87)90047-3" );
						ColorPickerPatina( "##PatinaPicker", &patinaColor );
						ImGui::ColorEdit4( "Color##Patina", &patinaColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Patina_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Subsurface Translucency" ) )
					{
						static ImVec4 sssColor( 0.5f, 0.7f, 0.55f, 1.0f );
						ImGui::TextWrapped( "Diffuse color of a translucent multiply-scattering material via the dipole model (Jensen et al., SIGGRAPH 2001). "
							"Material sets the absorption hue; plane X = absorption, Y = scattering (opaque<->translucent); slider = IOR. "
							"Try jade, wax, marble, milk, skin, amber." );
						DW_ReferenceLink( "https://graphics.stanford.edu/papers/bssrdf/bssrdf.pdf" );
						ColorPickerSubsurface( "##SubsurfacePicker", &sssColor );
						ImGui::ColorEdit4( "Color##SSS", &sssColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Subsurface_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Gas-Discharge / Neon Tubes" ) )
					{
						static ImVec4 dischargeColor( 1.0f, 0.4f, 0.2f, 1.0f );
						ImGui::TextWrapped( "Neon-tube color from low-pressure gas emission lines (NIST ASD; Waymouth). Pick the gas; plane X = mercury "
							"additive (toward blue), Y = white phosphor coating (toward pastel/white); slider = phosphor white point. "
							"Neon=red-orange, Ar+Hg=blue, helium=peach." );
						DW_ReferenceLink( "https://www.nist.gov/pml/atomic-spectra-database" );
						ColorPickerDischarge( "##DischargePicker", &dischargeColor );
						ImGui::ColorEdit4( "Color##Discharge", &dischargeColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Discharge_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Glacier / Sea Ice" ) )
					{
						static ImVec4 iceColor( 0.4f, 0.6f, 0.75f, 1.0f );
						ImGui::TextWrapped( "Why ice is blue: pure ice absorbs red far more than blue (Warren & Brandt 2008), so with enough path it "
							"turns deep blue, while fine grains/bubbles scatter and look white (Bohren 1983). Kubelka-Munk. Plane X = grain/scatter "
							"(blue ice <-> snow), Y = path depth; slider = impurity/dirt." );
						DW_ReferenceLink( "https://atmos.uw.edu/ice_optical_constants/" );
						ColorPickerIce( "##IcePicker", &iceColor );
						ImGui::ColorEdit4( "Color##Ice", &iceColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Ice_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Earth Pigments / Ochre (Kubelka-Munk)" ) )
					{
						static ImVec4 ochreColor( 0.6f, 0.35f, 0.12f, 1.0f );
						ImGui::TextWrapped( "Subtractive mixing of natural iron-oxide earth pigments via Kubelka-Munk (masstones after Elias et al. 2006). "
							"Pick two pigments; plane X = A->B mix, Y = chalk-white tint; slider = charcoal. Ochres, siennas, umbers - the oldest palette." );
						DW_ReferenceLink( "https://doi.org/10.1016/j.mseb.2005.09.061" );
						ColorPickerOchre( "##OchrePicker", &ochreColor );
						ImGui::ColorEdit4( "Color##Ochre", &ochreColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Ochre_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Sky (Bruneton multi-scatter)" ) )
					{
						static ImVec4 skyColor( 0.4f, 0.6f, 0.9f, 1.0f );
						ImGui::TextWrapped( "Physically-based sky colour: Rayleigh + Mie + ozone (Chappuis bands) with "
							"single-scatter direct integral plus a multi-scatter LUT (Hillaire 2020), baked transmittance LUT. "
							"Plane: X = time of day (0 -> 24h), Y = view elevation (bottom = horizon, top = zenith). "
							"Vertical slider = observer altitude (ground -> top of atmosphere). "
							"Component sliders = day-of-year + observer latitude + view azimuth (0 = toward sun, pi = away). "
							"Default view-az = 0 (toward sun) — at twilight columns (~18-20h for mid-latitudes) look for "
							"the warm orange glow at the very bottom and a thin cyan/teal stripe just above it from "
							"ozone preferentially absorbing the green-yellow-red band (Chappuis) along the long horizon path." );
						DW_ReferenceLink( "https://ebruneton.github.io/precomputed_atmospheric_scattering/" );
						DW_ReferenceLink( "https://sebh.github.io/publications/egsr2020.pdf" );
						ColorPickerSky( "##SkyPicker", &skyColor );
						ImGui::ColorEdit4( "Color##Sky", &skyColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Sky_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Star (Planck + line blanketing + TiO)" ) )
					{
						static ImVec4 starColor( 1.0f, 0.97f, 0.85f, 1.0f );
						ImGui::TextWrapped( "Stellar photosphere colour. Planck blackbody at effective temperature, "
							"modulated by metallicity-driven UV/blue line blanketing and surface-gravity-dependent "
							"TiO molecular absorption between 600-720 nm. Plane: X = log Teff (2500 K -> 40000 K), "
							"Y = log surface gravity. Vertical slider = metallicity [Fe/H]. "
							"Reference: Mamajek 2022 dwarf colour-temperature sequence + Pecaut & Mamajek 2013." );
						DW_ReferenceLink( "https://www.pas.rochester.edu/~emamajek/EEM_dwarf_UBVIJHK_colors_Teff.txt" );
						DW_ReferenceLink( "https://arxiv.org/abs/1307.2657" );
						ColorPickerStar( "##StarPicker", &starColor );
						ImGui::ColorEdit4( "Color##Star", &starColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Star_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Haemoglobin (vascular skin colour, Prahl)" ) )
					{
						static ImVec4 hemColor( 0.85f, 0.45f, 0.45f, 1.0f );
						ImGui::TextWrapped( "Skin reflectance through a Beer-Lambert layer of oxy- and deoxy-haemoglobin "
							"under a melanin attenuation layer, integrated against D65 + CIE 1931 CMFs. "
							"Plane: X = SpO2 (50%% -> 100%%), Y = dermal blood-volume fraction (0.5%% -> 10%%). "
							"Vertical slider = melanin density (fair -> very dark). "
							"Reference: Prahl haemoglobin extinction tables (OMLC), Jacques 1996 melanin model." );
						DW_ReferenceLink( "https://omlc.org/spectra/hemoglobin/" );
						ColorPickerHemoglobin( "##HemPicker", &hemColor );
						ImGui::ColorEdit4( "Color##Hem", &hemColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Hemoglobin_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Cloud (Schneider Beer-Powder + dual HG)" ) )
					{
						static ImVec4 cloudColor( 0.9f, 0.92f, 0.95f, 1.0f );
						ImGui::TextWrapped( "Volumetric cloud lighting from Schneider & Vos 2015 (Horizon Zero Dawn). "
							"Beer-Powder term E(d) = 2*exp(-sigma*d)*(1-exp(-2*sigma*d)) handles absorption AND in-scattering "
							"at cloud edges. Dual Henyey-Greenstein phase function mixes a forward Mie lobe (g1=+0.8) "
							"with a small back-scatter lobe (g2=-0.3) - silver lining when looking toward the sun. "
							"Reference: SIGGRAPH 2015 'Real-Time Volumetric Cloudscapes'." );
						DW_ReferenceLink( "https://advances.realtimerendering.com/s2015/The%20Real-time%20Volumetric%20Cloudscapes%20of%20Horizon%20-%20Zero%20Dawn%20-%20ARTR.pdf" );
						ColorPickerCloud( "##CloudPicker", &cloudColor );
						ImGui::ColorEdit4( "Color##Cloud", &cloudColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Cloud_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::TreeNode( "Streetlight (Hg / Na vapour + tri-phosphor)" ) )
					{
						static ImVec4 slColor( 1.0f, 0.7f, 0.3f, 1.0f );
						ImGui::TextWrapped( "Gas-discharge streetlamp spectrum. Mercury vapour gives the green-tinged "
							"city lamp (lines at 405, 436, 546, 577 nm); low-pressure sodium gives the unmistakable "
							"orange (589 nm D-doublet); high-pressure sodium broadens into a warm cluster with a "
							"self-reversal notch at the line centre. Plane: X = Hg <-> Na mix, Y = pressure. "
							"Vertical slider = tri-phosphor coating fraction. "
							"Reference: RIT 'Spectral Distribution of Gas Discharge Sources'." );
						DW_ReferenceLink( "https://www.nist.gov/pml/atomic-spectra-database" );
						ColorPickerStreetlight( "##SLPicker", &slColor );
						ImGui::ColorEdit4( "Color##SL", &slColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
						ImGui::TreePop();
					}
					DW_SsRecord( "Streetlight_Color_Picker", _sy0, ImGui::GetCursorPos().y );
				}

				}
				DW_SsRecord( "Physically_Based_Color_Pickers", _sy0_phys, ImGui::GetCursorPos().y );
				}

				{
					float _sy0_art = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Artist Color Pickers" ) )
					{
						ApplyOpenAll();
						if ( ImGui::TreeNode( "Harmony Wheel" ) )
						{
							static ImVec4 hwColor( 0.6f, 0.3f, 0.8f, 1.0f );
							static ImVec4 hwPalette[5];
							static int    hwCount = 0;
							ImGui::TextWrapped( "Circular disc with multiple draggable harmony handles - inspired by "
								"Adobe's color wheel (color.adobe.com). Pick the colour-space cylinder (HSV / HSL / HSY / "
								"HSP / OkLCH), then a harmony scheme; the secondary handles follow the active one "
								"automatically. Click a swatch below to pick which handle is the active output. "
								"Drag the disc to reposition the active handle; the other handles rotate with it." );
							ColorPickerHarmonyWheel( "##HarmWheel", &hwColor, hwPalette, &hwCount );
							ImGui::ColorEdit4( "Active##HW", &hwColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
							ImGui::TreePop();
						}
						ApplyOpenAll();
						if ( ImGui::TreeNode( "Palette Harmony" ) )
						{
							static ImVec4 phColor( 0.5f, 0.7f, 0.9f, 1.0f );
							static ImVec4 phPalette[5];
							static int    phCount = 0;
							ImGui::TextWrapped( "Anchor + harmony scheme -> 4..5-swatch palette. Plane = saturation x value at "
								"the anchor hue, slider = anchor hue, combo = scheme. Click the swatches below to make one "
								"of the harmony colours the active output. Guarantees a palette that 'reads'." );
							ColorPickerPaletteHarmony( "##PalHarm", &phColor, phPalette, &phCount );
							ImGui::ColorEdit4( "Active##PH", &phColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
							ImGui::TreePop();
						}
						ApplyOpenAll();
						if ( ImGui::TreeNode( "Trichromatic Mixer (subtractive)" ) )
						{
							static ImVec4 trColor( 0.7f, 0.4f, 0.3f, 1.0f );
							ImGui::TextWrapped( "Barycentric mix of three artist primaries (defaults to Y/M/C). Mixing is done in "
								"absorbance space, so yellow + cyan -> green, not muddy grey-green. Click the triangle to set "
								"the weights; outside the triangle the click is clamped to the nearest valid mix. Slider = "
								"tinting toward white (top) or black (bottom). Edit the three ColorEdit3 fields below to swap "
								"in your own primaries." );
							ColorPickerTrichromaticMixer( "##TriMix", &trColor );
							ImGui::ColorEdit4( "Mix##TR", &trColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
							ImGui::TreePop();
						}
						ApplyOpenAll();
						if ( ImGui::TreeNode( "Weathered Metal (PBR albedo)" ) )
						{
							static ImVec4 wmColor( 0.6f, 0.55f, 0.5f, 1.0f );
							ImGui::TextWrapped( "Pick a base metal (steel / iron / copper / brass / aluminum / gold), then dial "
								"patina coverage, roughness wash and grime. Each metal has a known F0 from PBR tables "
								"(Naty Hoffman GDC) and a metal-specific oxide colour (copper -> verdigris teal, iron -> rust, "
								"brass -> green tarnish, steel -> rust brown). Plane: X = patina, Y = roughness. Slider = grime." );
							ColorPickerWeatheredMetal( "##WMet", &wmColor );
							ImGui::ColorEdit4( "Albedo##WM", &wmColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
							ImGui::TreePop();
						}
						ApplyOpenAll();
						if ( ImGui::TreeNode( "Fabric Dye (substrate + dye)" ) )
						{
							static ImVec4 fdColor( 0.4f, 0.5f, 0.6f, 1.0f );
							ImGui::TextWrapped( "Dye laid on a fabric substrate via Beer-Lambert. The same red dye looks completely "
								"different on raw linen (warm beige base) vs natural wool (taupe) vs bleached cotton (cream). "
								"Plane: X = dye hue, Y = dye saturation. Slider = dye intensity (washed -> saturated). Combo = "
								"substrate fabric." );
							ColorPickerFabricDye( "##Fab", &fdColor );
							ImGui::ColorEdit4( "Color##FB", &fdColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
							ImGui::TreePop();
						}
						ApplyOpenAll();
						if ( ImGui::TreeNode( "Mood Palette (curated)" ) )
						{
							static ImVec4 mdColor( 0.9f, 0.5f, 0.3f, 1.0f );
							static ImVec4 mdPalette[5];
							ImGui::TextWrapped( "Curated 5-swatch palettes by mood word (warm / cool / melancholy / fresh / vintage / "
								"pastel / neon / earth / sunset / ocean). Plane: X = blend position across the 5 stops, "
								"Y = lightness shift. Slider = saturation crush (desaturated -> raw)." );
							ColorPickerMoodPalette( "##Mood", &mdColor, mdPalette );
							ImGui::ColorEdit4( "Blended##MD", &mdColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
							ImGui::TreePop();
						}
						ApplyOpenAll();
						if ( ImGui::TreeNode( "Toon Ramp (shadow / mid / highlight)" ) )
						{
							static ImVec4 trnColor( 0.85f, 0.55f, 0.35f, 1.0f );
							static ImVec4 trnRamp[3];
							ImGui::TextWrapped( "Pick a midtone and get back a shadow / midtone / highlight 3-stop ramp ready to "
								"feed a toon shader. Plane: midtone hue x value. Slider: midtone saturation. Component sliders "
								"control chroma drop into the shadow, warm-cool hue shift between shadow and highlight, and "
								"terminator hardness (soft -> hard cel edge)." );
							ColorPickerToonRamp( "##Toon", &trnColor, trnRamp );
							ImGui::ColorEdit4( "Midtone##TN", &trnColor.x, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoPicker );
							ImGui::TreePop();
						}
					}
					DW_SsRecord( "Artist_Color_Pickers", _sy0_art, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Primaries Wheels (Lift/Gamma/Gain/Offset)" ) )
					{
						static ImVec4 primColors[4] = {
							ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Lift
							ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Gamma
							ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Gain
							ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Offset
						};
						static float primY[4] = { 0.0f, 0.0f, 1.0f, 25.0f };
						static int primMode = ImColorWheelMode_HSV;

						const char* primNames[] = { "Lift", "Gamma", "Gain", "Offset" };
						const float yMins[] = { -1.0f, -1.0f, -1.0f, -175.0f };
						const float yMaxs[] = { 1.0f,  1.0f,  1.0f,  225.0f };

						ImGui::Combo( "Mode##Prim", &primMode, "HSV\0OkLCH\0" );

						float outerSize = ImGui::GetContentRegionAvail().x / 4.0f - ImGui::GetStyle().ItemSpacing.x;
						if ( outerSize < 100.0f ) outerSize = 100.0f;
						if ( outerSize > 200.0f ) outerSize = 200.0f;

						if ( ImGui::BeginTable( "##PrimWheels", 4, ImGuiTableFlags_NoSavedSettings ) )
						{
							for ( int i = 0; i < 4; ++i )
								ImGui::TableSetupColumn( primNames[i], ImGuiTableColumnFlags_WidthFixed, outerSize );

							for ( int i = 0; i < 4; ++i )
							{
								ImGui::TableNextColumn();
								ImGui::TextUnformatted( primNames[i] );

								ImGui::PushID( i );

								HDRWheel( "##pw", &primColors[i], &primY[i], yMins[i], yMaxs[i], NULL, 0.0f, 0.0f, NULL, 0.0f, 0.0f, (ImColorWheelMode)primMode, ImVec2( outerSize, outerSize ) );

								// YRGB readouts
								float qw = outerSize * 0.25f - 1.0f;
								ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "Y##v", &primY[i], 0.01f, yMins[i], yMaxs[i], "%.2f" );
								ImGui::SameLine();
								ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "R##v", &primColors[i].x, 0.01f, 0.0f, 1.0f, "%.2f" );
								ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "G##v", &primColors[i].y, 0.01f, 0.0f, 1.0f, "%.2f" );
								ImGui::SameLine();
								ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "B##v", &primColors[i].z, 0.01f, 0.0f, 1.0f, "%.2f" );

								ImGui::PopID();
							}
							ImGui::EndTable();
						}

						ImGui::Separator();

						// Shared controls
						static float primTemp = 0.0f, primTint = 0.0f;
						static float primContrast = 0.0f, primPivot = 0.5f;
						static float primSaturation = 50.0f, primHue = 0.0f;

						// Narrower sliders (~180 lp) so label + widget pairs don't consume
						// the full row width. Drops from 0.5x avail -> 4 sliders per row.
						float ctrlW = ImPlatform_LpToPx( 180.0f );
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Temp", &primTemp, -100.0f, 100.0f, "%.1f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Tint", &primTint, -100.0f, 100.0f, "%.1f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Contrast", &primContrast, -100.0f, 100.0f, "%.1f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Pivot", &primPivot, 0.0f, 1.0f, "%.2f" );

						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Saturation", &primSaturation, 0.0f, 100.0f, "%.1f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Hue", &primHue, -180.0f, 180.0f, "%.1f" );
					}
					DW_SsRecord( "Primaries_Wheels", _sy0, ImGui::GetCursorPos().y );
				}

				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "HDR Wheels (Dark/Shadow/Light/Global)" ) )
					{
						static ImVec4 hdrColors[4] = {
							ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Dark
							ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Shadow
							ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Light
							ImVec4( 0.5f, 0.5f, 0.5f, 1.0f ), // Global
						};
						static float hdrY[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
						static float hdrExposure[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
						static float hdrSaturation[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

						const char* hdrNames[] = { "Dark", "Shadow", "Light", "Global" };

						float colW = ImGui::GetContentRegionAvail().x / 4.0f - ImGui::GetStyle().ItemSpacing.x;
						if ( colW < 100.0f ) colW = 100.0f;
						if ( colW > 200.0f ) colW = 200.0f;

						// HDRWheel adds arc overhead on top of the disc+ring size (colW), so columns are wider.
						// Widget computes its arcOverhead in PX (via LpToPx), so we must too — otherwise
						// at DPI > 1 the column is too narrow and the right-side arc spills into the next cell.
						ImWidgetsStyle& hdrDwStyle = ImWidgets::GetStyle();
						float arcOverhead = 2.0f * ImPlatform_LpToPx(hdrDwStyle.HDRWheel_ArcGrabRadius + hdrDwStyle.HDRWheel_ArcThickness + hdrDwStyle.HDRWheel_ArcGap);
						float hdrColW = colW + arcOverhead;

						if ( ImGui::BeginTable( "##HDRWheels", 4, ImGuiTableFlags_NoSavedSettings ) )
						{
							for ( int i = 0; i < 4; ++i )
								ImGui::TableSetupColumn( hdrNames[i], ImGuiTableColumnFlags_WidthFixed, hdrColW );

							for ( int i = 0; i < 4; ++i )
							{
								ImGui::TableNextColumn();
								ImGui::TextUnformatted( hdrNames[i] );

								ImGui::PushID( i );

								HDRWheel( "##hw", &hdrColors[i], &hdrY[i], -1.0f, 1.0f,
										  &hdrExposure[i], -4.0f, 4.0f,
										  &hdrSaturation[i], 0.0f, 2.0f,
										  ImColorWheelMode_OkLCH, ImVec2( colW, colW ) );

								// Readouts
								float qw = colW * 0.5f - 1.0f;
								ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "Exp##v", &hdrExposure[i], 0.01f, -4.0f, 4.0f, "%.2f" );
								ImGui::SameLine();
								ImGui::SetNextItemWidth( qw ); ImGui::DragFloat( "Sat##v", &hdrSaturation[i], 0.01f, 0.0f, 2.0f, "%.2f" );

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

						float ctrlW = ImPlatform_LpToPx( 180.0f );
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Temp##HDR", &hdrTemp, -100.0f, 100.0f, "%.1f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Tint##HDR", &hdrTint, -100.0f, 100.0f, "%.1f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Contrast##HDR", &hdrContrast, -100.0f, 100.0f, "%.1f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Pivot##HDR", &hdrPivot, 0.0f, 1.0f, "%.2f" );

						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Mid Detail##HDR", &hdrMidDetail, -100.0f, 100.0f, "%.1f" );
						ImGui::SameLine();
						ImGui::SetNextItemWidth( ctrlW ); ImGui::SliderFloat( "Black Offset##HDR", &hdrBlackOffset, -1.0f, 1.0f, "%.3f" );
					}
					DW_SsRecord( "HDR_Wheels", _sy0, ImGui::GetCursorPos().y );
				}
				ApplyOpenAll();
				if ( ImGui::CollapsingHeader( "Gradient Mesh Editor" ) )
				{
					float _sy0 = ImGui::GetCursorPos().y;
					static ImWidgets::ImWidgetsGradientMesh gm;
					if ( ImGui::Button( "Reset 3x3 RGB" ) )
						ImWidgets::GradientMeshInit( gm, 3, 3,
							IM_COL32( 220,  80,  80, 255 ), IM_COL32(  80, 220,  80, 255 ),
							IM_COL32(  80,  80, 220, 255 ), IM_COL32( 240, 240, 240, 255 ) );
					ImGui::SameLine();
					if ( ImGui::Button( "Reset 4x4 Cyan/Magenta" ) )
						ImWidgets::GradientMeshInit( gm, 4, 4,
							IM_COL32(  10, 200, 220, 255 ), IM_COL32( 240, 200,  10, 255 ),
							IM_COL32( 220,  10, 200, 255 ), IM_COL32(  10,  60, 110, 255 ) );
					ImGui::Checkbox( "Show mesh", &gm.ShowMesh );
					ImWidgets::GradientMeshEditor( "##gm_demo", gm,
						ImVec2( ImGui::GetContentRegionAvail().x, ImPlatform_LpToPx( 220.0f ) ) );
					DW_SsRecord( "Gradient_Mesh", _sy0, ImGui::GetCursorPos().y );
				}
				ApplyOpenAll();
				if ( ImGui::CollapsingHeader( "Toon Ramp Editor" ) )
				{
					float _sy0 = ImGui::GetCursorPos().y;
					static ImWidgets::ImWidgetsToonRamp tr;
					ImWidgets::ToonRampEditor( "##tr_demo", tr,
						ImVec2( ImGui::GetContentRegionAvail().x, 48.0f ) );
					ImGui::Text( "Sample at t = 0.0 / 0.25 / 0.5 / 0.75 / 1.0:" );
					ImDrawList* dl = ImGui::GetWindowDrawList();
					ImVec2 p = ImGui::GetCursorScreenPos();
					float bx = p.x;
					for ( int k = 0; k <= 4; ++k )
					{
						float t = (float)k / 4.0f;
						ImU32 c = ImWidgets::ToonRampSample( tr, t );
						dl->AddRectFilled( ImVec2( bx, p.y ), ImVec2( bx + 60, p.y + 24 ), c );
						bx += 70;
					}
					ImGui::Dummy( ImVec2( 0, 28 ) );
					DW_SsRecord( "Toon_Ramp", _sy0, ImGui::GetCursorPos().y );
				}

				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Color Analysis##Widgets" ) )
			{

				// Color Curve -- one CollapsingHeader per mode so the screenshot system
				// generates one image per curve type.
				{
					static bool             ccShowHistogram = true;
					static bool             ccAdvancedSegments = false;
					static float            ccSampleX = 0.5f;
					static ImHistogramData  ccHistData;
					static bool             ccHistInit = false;
					static ImColorCurveData ccData[ImColorCurveMode_COUNT];
					static bool             ccInit = false;

					if ( !ccHistInit )
					{
						int imgW, imgH, imgCh;
						stbi_uc* imgData = stbi_load( "pexels-fotoaibe-1571453.jpg", &imgW, &imgH, &imgCh, 0 );
						if ( imgData )
						{
							int ch = (imgCh >= 3) ? imgCh : 3;
							ccHistData.Accumulate( imgData, imgW, imgH, ch,
												   ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved, ImHistogramMode_Luma,
												   256, 1000000 );
							STBI_FREE( imgData );
						}
						ccHistInit = true;
					}
					if ( !ccInit )
					{
						// Hue vs Hue: shift reds toward orange
						ccData[ImColorCurveMode_HueVsHue].AddKey( 0.0f, 0.05f );
						ccData[ImColorCurveMode_HueVsHue].AddKey( 0.15f, 0.0f );
						// Hue vs Sat: boost greens
						ccData[ImColorCurveMode_HueVsSat].AddKey( 0.25f, 1.0f );
						ccData[ImColorCurveMode_HueVsSat].AddKey( 0.33f, 1.5f );
						ccData[ImColorCurveMode_HueVsSat].AddKey( 0.42f, 1.0f );
						ccInit = true;
					}
					// Helper: "Add default keys" -- places one key per primary at the
					// 6 canonical hue stops (R Y G C B M) at neutral. Red gets a
					// single key at 0.0 (the curve wraps; a duplicate at 1.0 is
					// redundant). Returns true if the curve was modified.
					auto AddDefaultHueKeys = []( ImColorCurveData& d, ImColorCurveMode m ) -> bool{
						float neutral = 0.0f;
						if ( m == ImColorCurveMode_HueVsSat ) neutral = 1.0f;
						else if ( m == ImColorCurveMode_HueVsHue ) neutral = 0.0f;
						else if ( m == ImColorCurveMode_HueVsLum ) neutral = 0.0f;
						d.Keys.clear();
						d.SelectedIdx = -1;
						const float stops[] = { 0.0f, 1.0f / 6.0f, 2.0f / 6.0f, 0.5f, 4.0f / 6.0f, 5.0f / 6.0f };
						for ( int i = 0; i < IM_ARRAYSIZE( stops ); ++i )
							d.AddKey( stops[i], neutral );
						return true;
						};
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Hue vs Hue" ) )
						{
							ImGui::Checkbox( "Histogram##CC_HH", &ccShowHistogram );
							ImGui::SameLine();
							ImGui::Checkbox( "Advanced Segments##CC_HH", &ccAdvancedSegments );
							ImColorCurveData& curData = ccData[ImColorCurveMode_HueVsHue];
							ImGui::SameLine();
							if ( ImGui::Button( "Add default keys##CC_HH" ) ) AddDefaultHueKeys( curData, ImColorCurveMode_HueVsHue );
							ImHistogramData const* histPtr = (ccShowHistogram && ccHistData.BinCount > 0) ? &ccHistData : NULL;
							ColorCurve( "##CC_HH", &curData, ImColorCurveMode_HueVsHue, histPtr, ccAdvancedSegments, ImVec2( 0, 150 ) );
							if ( curData.SelectedIdx >= 0 && curData.SelectedIdx < curData.Keys.Size )
							{
								ImColorCurveKey& key = curData.Keys[curData.SelectedIdx];
								float posMin = (curData.SelectedIdx > 0) ? curData.Keys[curData.SelectedIdx - 1].Position : 0.0f;
								float posMax = (curData.SelectedIdx < curData.Keys.Size - 1) ? curData.Keys[curData.SelectedIdx + 1].Position : 1.0f;
								ImGui::DragFloat( "Position##CC_HH_Key", &key.Position, 0.005f, posMin, posMax, "%.3f" );
								float rMin, rMax;
								ImWidgets::ColorCurveRange( ImColorCurveMode_HueVsHue, &rMin, &rMax );
								ImGui::DragFloat( "Value##CC_HH_Key", &key.Value, 0.01f, rMin, rMax, "%.3f" );
							}
							else
							{
								ImGui::TextDisabled( "No key selected" );
							}
							ImGui::SliderFloat( "Sample x##CC_HH", &ccSampleX, 0.0f, 1.0f );
							ImGui::Text( "y = %.4f", ImWidgets::ColorCurveSample( curData, ImColorCurveMode_HueVsHue, ccSampleX ) );
							ImGui::TextWrapped( "Click to add key. Drag to move. Drag far outside to delete. Right-click for options." );
						}
						DW_SsRecord( "Color_Curve_Hue_vs_Hue", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Hue vs Sat" ) )
						{
							ImGui::Checkbox( "Histogram##CC_HS", &ccShowHistogram );
							ImGui::SameLine();
							ImGui::Checkbox( "Advanced Segments##CC_HS", &ccAdvancedSegments );
							ImColorCurveData& curData = ccData[ImColorCurveMode_HueVsSat];
							ImGui::SameLine();
							if ( ImGui::Button( "Add default keys##CC_HS" ) ) AddDefaultHueKeys( curData, ImColorCurveMode_HueVsSat );
							ImHistogramData const* histPtr = (ccShowHistogram && ccHistData.BinCount > 0) ? &ccHistData : NULL;
							ColorCurve( "##CC_HS", &curData, ImColorCurveMode_HueVsSat, histPtr, ccAdvancedSegments, ImVec2( 0, 150 ) );
							if ( curData.SelectedIdx >= 0 && curData.SelectedIdx < curData.Keys.Size )
							{
								ImColorCurveKey& key = curData.Keys[curData.SelectedIdx];
								float posMin = (curData.SelectedIdx > 0) ? curData.Keys[curData.SelectedIdx - 1].Position : 0.0f;
								float posMax = (curData.SelectedIdx < curData.Keys.Size - 1) ? curData.Keys[curData.SelectedIdx + 1].Position : 1.0f;
								ImGui::DragFloat( "Position##CC_HS_Key", &key.Position, 0.005f, posMin, posMax, "%.3f" );
								float rMin, rMax;
								ImWidgets::ColorCurveRange( ImColorCurveMode_HueVsSat, &rMin, &rMax );
								ImGui::DragFloat( "Value##CC_HS_Key", &key.Value, 0.01f, rMin, rMax, "%.3f" );
							}
							else
							{
								ImGui::TextDisabled( "No key selected" );
							}
							ImGui::SliderFloat( "Sample x##CC_HS", &ccSampleX, 0.0f, 1.0f );
							ImGui::Text( "y = %.4f", ImWidgets::ColorCurveSample( curData, ImColorCurveMode_HueVsSat, ccSampleX ) );
							ImGui::TextWrapped( "Click to add key. Drag to move. Drag far outside to delete. Right-click for options." );
						}
						DW_SsRecord( "Color_Curve_Hue_vs_Sat", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Hue vs Lum" ) )
						{
							ImGui::Checkbox( "Histogram##CC_HL", &ccShowHistogram );
							ImGui::SameLine();
							ImGui::Checkbox( "Advanced Segments##CC_HL", &ccAdvancedSegments );
							ImColorCurveData& curData = ccData[ImColorCurveMode_HueVsLum];
							ImGui::SameLine();
							if ( ImGui::Button( "Add default keys##CC_HL" ) ) AddDefaultHueKeys( curData, ImColorCurveMode_HueVsLum );
							ImHistogramData const* histPtr = (ccShowHistogram && ccHistData.BinCount > 0) ? &ccHistData : NULL;
							ColorCurve( "##CC_HL", &curData, ImColorCurveMode_HueVsLum, histPtr, ccAdvancedSegments, ImVec2( 0, 150 ) );
							if ( curData.SelectedIdx >= 0 && curData.SelectedIdx < curData.Keys.Size )
							{
								ImColorCurveKey& key = curData.Keys[curData.SelectedIdx];
								float posMin = (curData.SelectedIdx > 0) ? curData.Keys[curData.SelectedIdx - 1].Position : 0.0f;
								float posMax = (curData.SelectedIdx < curData.Keys.Size - 1) ? curData.Keys[curData.SelectedIdx + 1].Position : 1.0f;
								ImGui::DragFloat( "Position##CC_HL_Key", &key.Position, 0.005f, posMin, posMax, "%.3f" );
								float rMin, rMax;
								ImWidgets::ColorCurveRange( ImColorCurveMode_HueVsLum, &rMin, &rMax );
								ImGui::DragFloat( "Value##CC_HL_Key", &key.Value, 0.01f, rMin, rMax, "%.3f" );
							}
							else
							{
								ImGui::TextDisabled( "No key selected" );
							}
							ImGui::SliderFloat( "Sample x##CC_HL", &ccSampleX, 0.0f, 1.0f );
							ImGui::Text( "y = %.4f", ImWidgets::ColorCurveSample( curData, ImColorCurveMode_HueVsLum, ccSampleX ) );
							ImGui::TextWrapped( "Click to add key. Drag to move. Drag far outside to delete. Right-click for options." );
						}
						DW_SsRecord( "Color_Curve_Hue_vs_Lum", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Lum vs Sat" ) )
						{
							ImGui::Checkbox( "Histogram##CC_LS", &ccShowHistogram );
							ImGui::SameLine();
							ImGui::Checkbox( "Advanced Segments##CC_LS", &ccAdvancedSegments );
							ImHistogramData const* histPtr = (ccShowHistogram && ccHistData.BinCount > 0) ? &ccHistData : NULL;
							ImColorCurveData& curData = ccData[ImColorCurveMode_LumVsSat];
							ColorCurve( "##CC_LS", &curData, ImColorCurveMode_LumVsSat, histPtr, ccAdvancedSegments, ImVec2( 0, 150 ) );
							if ( curData.SelectedIdx >= 0 && curData.SelectedIdx < curData.Keys.Size )
							{
								ImColorCurveKey& key = curData.Keys[curData.SelectedIdx];
								float posMin = (curData.SelectedIdx > 0) ? curData.Keys[curData.SelectedIdx - 1].Position : 0.0f;
								float posMax = (curData.SelectedIdx < curData.Keys.Size - 1) ? curData.Keys[curData.SelectedIdx + 1].Position : 1.0f;
								ImGui::DragFloat( "Position##CC_LS_Key", &key.Position, 0.005f, posMin, posMax, "%.3f" );
								float rMin, rMax;
								ImWidgets::ColorCurveRange( ImColorCurveMode_LumVsSat, &rMin, &rMax );
								ImGui::DragFloat( "Value##CC_LS_Key", &key.Value, 0.01f, rMin, rMax, "%.3f" );
							}
							else
							{
								ImGui::TextDisabled( "No key selected" );
							}
							ImGui::SliderFloat( "Sample x##CC_LS", &ccSampleX, 0.0f, 1.0f );
							ImGui::Text( "y = %.4f", ImWidgets::ColorCurveSample( curData, ImColorCurveMode_LumVsSat, ccSampleX ) );
							ImGui::TextWrapped( "Click to add key. Drag to move. Drag far outside to delete. Right-click for options." );
						}
						DW_SsRecord( "Color_Curve_Lum_vs_Sat", _sy0, ImGui::GetCursorPos().y );
					}
					{
						float _sy0 = ImGui::GetCursorPos().y;
						ApplyOpenAll();
						if ( ImGui::CollapsingHeader( "Sat vs Sat" ) )
						{
							ImGui::Checkbox( "Histogram##CC_SS", &ccShowHistogram );
							ImGui::SameLine();
							ImGui::Checkbox( "Advanced Segments##CC_SS", &ccAdvancedSegments );
							ImHistogramData const* histPtr = (ccShowHistogram && ccHistData.BinCount > 0) ? &ccHistData : NULL;
							ImColorCurveData& curData = ccData[ImColorCurveMode_SatVsSat];
							ColorCurve( "##CC_SS", &curData, ImColorCurveMode_SatVsSat, histPtr, ccAdvancedSegments, ImVec2( 0, 150 ) );
							if ( curData.SelectedIdx >= 0 && curData.SelectedIdx < curData.Keys.Size )
							{
								ImColorCurveKey& key = curData.Keys[curData.SelectedIdx];
								float posMin = (curData.SelectedIdx > 0) ? curData.Keys[curData.SelectedIdx - 1].Position : 0.0f;
								float posMax = (curData.SelectedIdx < curData.Keys.Size - 1) ? curData.Keys[curData.SelectedIdx + 1].Position : 1.0f;
								ImGui::DragFloat( "Position##CC_SS_Key", &key.Position, 0.005f, posMin, posMax, "%.3f" );
								float rMin, rMax;
								ImWidgets::ColorCurveRange( ImColorCurveMode_SatVsSat, &rMin, &rMax );
								ImGui::DragFloat( "Value##CC_SS_Key", &key.Value, 0.01f, rMin, rMax, "%.3f" );
							}
							else
							{
								ImGui::TextDisabled( "No key selected" );
							}
							ImGui::SliderFloat( "Sample x##CC_SS", &ccSampleX, 0.0f, 1.0f );
							ImGui::Text( "y = %.4f", ImWidgets::ColorCurveSample( curData, ImColorCurveMode_SatVsSat, ccSampleX ) );
							ImGui::TextWrapped( "Click to add key. Drag to move. Drag far outside to delete. Right-click for options." );
						}
						DW_SsRecord( "Color_Curve_Sat_vs_Sat", _sy0, ImGui::GetCursorPos().y );
					}
				}  // end Color Curve shared block


				{ float _sy0 = ImGui::GetCursorPos().y;
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
								static ImU8 const bars[7][3] = {
									{ 191, 191, 191 }, { 191, 191,  17 }, {  17, 191, 191 }, {  17, 191,  17 },
									{ 191,  17, 191 }, { 191,  17,  17 }, {  17,  17, 191 }
								};
								for ( int y = 0; y < kTestH; ++y )
								{
									for ( int x = 0; x < kTestW; ++x )
									{
										int barIdx = x * 7 / kTestW;
										int off = (y * kTestW + x) * 3;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 28) - 8;
										testImage[off + 0] = (ImU8)ImClamp( bars[barIdx][0] + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( bars[barIdx][1] + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( bars[barIdx][2] + noise, 0, 255 );
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
										float t = (float)x / (float)(kTestW - 1);
										int off = (y * kTestW + x) * 3;
										int third = y * 3 / kTestH;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 29) - 4;
										testImage[off + 0] = (ImU8)ImClamp( (third == 0 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( (third == 1 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( (third == 2 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
									}
								}
							}
							else
							{
								// Random noise
								for ( int i = 0; i < kTestW * kTestH * 3; ++i )
								{
									rng = rng * 1664525u + 1013904223u;
									testImage[i] = (ImU8)(rng >> 24);
								}
							}

							paradeData.Accumulate( testImage.Data, kTestW, kTestH, 3,
												   ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved, (ImParadeMode)paradeMode,
												   128, 128, 500000 );

							// Create thumbnail texture (RGB -> RGBA)
							{
								static ImVector<ImU8> rgba;
								rgba.resize( kTestW * kTestH * 4 );
								for ( int i = 0; i < kTestW * kTestH; ++i )
								{
									rgba[i * 4 + 0] = testImage[i * 3 + 0];
									rgba[i * 4 + 1] = testImage[i * 3 + 1];
									rgba[i * 4 + 2] = testImage[i * 3 + 2];
									rgba[i * 4 + 3] = 255;
								}
								ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kTestW, kTestH );
								paradeThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
								paradeThumbnailSize = ImVec2( (float)kTestW, (float)kTestH );
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
							paradeImgData = stbi_load( filenames[fileIdx], &paradeImgW, &paradeImgH, &paradeImgCh, 0 );
							if ( paradeImgData )
							{
								int ch = (paradeImgCh >= 3) ? paradeImgCh : 3;
								paradeData.Accumulate( paradeImgData, paradeImgW, paradeImgH, ch,
													   ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved, (ImParadeMode)paradeMode,
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
						else if ( paradeSource == 3 )
						{
							thumbTex = illlustration_img; thumbSize = illlustration_size;
						}
						else if ( paradeSource == 4 )
						{
							thumbTex = background;        thumbSize = background_size;
						}
						else if ( paradeSource == 5 )
						{
							thumbTex = man_img;           thumbSize = man_size;
						}
						else if ( paradeSource == 6 )
						{
							thumbTex = astro_img;         thumbSize = astro_size;
						}
						if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
						{
							float thumbH = ImPlatform_LpToPx( 240.0f );
							float thumbW = thumbH * thumbSize.x / thumbSize.y;
							ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
						}
					}

					ImWidgets::ParadeScope( "##ParadeMain", paradeData, paradeOverlay, (ImParadeScale)paradeScale, ImVec2( 0, 300 ) );
					if ( paradeSource <= 2 )
						ImGui::Text( "Source: 1920x1080 (generated)  Peak: %u", paradeData.PeakCount );
					else if ( paradeImgData )
						ImGui::Text( "Source: %dx%d (%d ch)  Peak: %u", paradeImgW, paradeImgH, paradeImgCh, paradeData.PeakCount );
					else
						ImGui::TextDisabled( "Failed to load image" );
				}
				DW_SsRecord( "Parade_Scope", _sy0, ImGui::GetCursorPos().y ); }

				{ float _sy0 = ImGui::GetCursorPos().y;
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
								static ImU8 const bars[7][3] = {
									{ 191, 191, 191 }, { 191, 191,  17 }, {  17, 191, 191 }, {  17, 191,  17 },
									{ 191,  17, 191 }, { 191,  17,  17 }, {  17,  17, 191 }
								};
								for ( int y = 0; y < kTestH; ++y )
								{
									for ( int x = 0; x < kTestW; ++x )
									{
										int barIdx = x * 7 / kTestW;
										int off = (y * kTestW + x) * 3;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 28) - 8;
										testImage[off + 0] = (ImU8)ImClamp( bars[barIdx][0] + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( bars[barIdx][1] + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( bars[barIdx][2] + noise, 0, 255 );
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
										float t = (float)x / (float)(kTestW - 1);
										int off = (y * kTestW + x) * 3;
										int third = y * 3 / kTestH;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 29) - 4;
										testImage[off + 0] = (ImU8)ImClamp( (third == 0 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( (third == 1 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( (third == 2 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
									}
								}
							}
							else
							{
								// Random noise
								for ( int i = 0; i < kTestW * kTestH * 3; ++i )
								{
									rng = rng * 1664525u + 1013904223u;
									testImage[i] = (ImU8)(rng >> 24);
								}
							}

							vectorData.Accumulate( testImage.Data, kTestW, kTestH, 3,
												   ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved,
												   256, 500000 );

							// Create thumbnail texture (RGB -> RGBA)
							{
								static ImVector<ImU8> rgba;
								rgba.resize( kTestW * kTestH * 4 );
								for ( int i = 0; i < kTestW * kTestH; ++i )
								{
									rgba[i * 4 + 0] = testImage[i * 3 + 0];
									rgba[i * 4 + 1] = testImage[i * 3 + 1];
									rgba[i * 4 + 2] = testImage[i * 3 + 2];
									rgba[i * 4 + 3] = 255;
								}
								ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kTestW, kTestH );
								vectorThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
								vectorThumbnailSize = ImVec2( (float)kTestW, (float)kTestH );
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
							vectorImgData = stbi_load( filenames[fileIdx], &vectorImgW, &vectorImgH, &vectorImgCh, 0 );
							if ( vectorImgData )
							{
								int ch = (vectorImgCh >= 3) ? vectorImgCh : 3;
								vectorData.Accumulate( vectorImgData, vectorImgW, vectorImgH, ch,
													   ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved,
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
						else if ( vectorSource == 3 )
						{
							thumbTex = illlustration_img; thumbSize = illlustration_size;
						}
						else if ( vectorSource == 4 )
						{
							thumbTex = background;        thumbSize = background_size;
						}
						else if ( vectorSource == 5 )
						{
							thumbTex = man_img;           thumbSize = man_size;
						}
						else if ( vectorSource == 6 )
						{
							thumbTex = astro_img;         thumbSize = astro_size;
						}
						if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
						{
							float thumbH = ImPlatform_LpToPx( 240.0f );
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
				DW_SsRecord( "Vector_Scope", _sy0, ImGui::GetCursorPos().y ); }

				{ float _sy0 = ImGui::GetCursorPos().y;
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
								static ImU8 const bars[7][3] = {
									{ 191, 191, 191 }, { 191, 191,  17 }, {  17, 191, 191 }, {  17, 191,  17 },
									{ 191,  17, 191 }, { 191,  17,  17 }, {  17,  17, 191 }
								};
								for ( int y = 0; y < kTestH; ++y )
								{
									for ( int x = 0; x < kTestW; ++x )
									{
										int barIdx = x * 7 / kTestW;
										int off = (y * kTestW + x) * 3;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 28) - 8;
										testImage[off + 0] = (ImU8)ImClamp( bars[barIdx][0] + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( bars[barIdx][1] + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( bars[barIdx][2] + noise, 0, 255 );
									}
								}
							}
							else if ( histSource == 1 )
							{
								for ( int y = 0; y < kTestH; ++y )
								{
									for ( int x = 0; x < kTestW; ++x )
									{
										float t = (float)x / (float)(kTestW - 1);
										int off = (y * kTestW + x) * 3;
										int third = y * 3 / kTestH;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 29) - 4;
										testImage[off + 0] = (ImU8)ImClamp( (third == 0 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( (third == 1 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( (third == 2 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
									}
								}
							}
							else
							{
								for ( int i = 0; i < kTestW * kTestH * 3; ++i )
								{
									rng = rng * 1664525u + 1013904223u;
									testImage[i] = (ImU8)(rng >> 24);
								}
							}

							histData.Accumulate( testImage.Data, kTestW, kTestH, 3,
												 ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved, (ImHistogramMode)histMode,
												 256, 500000 );

							// Create thumbnail texture (RGB -> RGBA)
							{
								static ImVector<ImU8> rgba;
								rgba.resize( kTestW * kTestH * 4 );
								for ( int i = 0; i < kTestW * kTestH; ++i )
								{
									rgba[i * 4 + 0] = testImage[i * 3 + 0];
									rgba[i * 4 + 1] = testImage[i * 3 + 1];
									rgba[i * 4 + 2] = testImage[i * 3 + 2];
									rgba[i * 4 + 3] = 255;
								}
								ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kTestW, kTestH );
								histThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
								histThumbnailSize = ImVec2( (float)kTestW, (float)kTestH );
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
							histImgData = stbi_load( filenames[fileIdx], &histImgW, &histImgH, &histImgCh, 0 );
							if ( histImgData )
							{
								int ch = (histImgCh >= 3) ? histImgCh : 3;
								histData.Accumulate( histImgData, histImgW, histImgH, ch,
													 ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved, (ImHistogramMode)histMode,
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
						else if ( histSource == 3 )
						{
							thumbTex = illlustration_img; thumbSize = illlustration_size;
						}
						else if ( histSource == 4 )
						{
							thumbTex = background;        thumbSize = background_size;
						}
						else if ( histSource == 5 )
						{
							thumbTex = man_img;           thumbSize = man_size;
						}
						else if ( histSource == 6 )
						{
							thumbTex = astro_img;         thumbSize = astro_size;
						}
						if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
						{
							float thumbH = ImPlatform_LpToPx( 240.0f );
							float thumbW = thumbH * thumbSize.x / thumbSize.y;
							ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
						}
					}

					ImWidgets::Histogram( "##HistMain", histData, (ImHistogramLayout)histLayout, (ImParadeScale)histXScale, (ImParadeScale)histYScale, ImVec2( 0, 300 ) );
					if ( histSource <= 2 )
						ImGui::Text( "Source: 1920x1080 (generated)  Peak: %u", histData.PeakCount );
					else if ( histImgData )
						ImGui::Text( "Source: %dx%d (%d ch)  Peak: %u", histImgW, histImgH, histImgCh, histData.PeakCount );
					else
						ImGui::TextDisabled( "Failed to load image" );
				}
				DW_SsRecord( "Histogram", _sy0, ImGui::GetCursorPos().y ); }

				{ float _sy0 = ImGui::GetCursorPos().y;
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
								static ImU8 const bars[7][3] = {
									{ 191, 191, 191 }, { 191, 191,  17 }, {  17, 191, 191 }, {  17, 191,  17 },
									{ 191,  17, 191 }, { 191,  17,  17 }, {  17,  17, 191 }
								};
								for ( int y = 0; y < kTestH; ++y )
								{
									for ( int x = 0; x < kTestW; ++x )
									{
										int barIdx = x * 7 / kTestW;
										int off = (y * kTestW + x) * 3;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 28) - 8;
										testImage[off + 0] = (ImU8)ImClamp( bars[barIdx][0] + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( bars[barIdx][1] + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( bars[barIdx][2] + noise, 0, 255 );
									}
								}
							}
							else if ( cieSource == 1 )
							{
								for ( int y = 0; y < kTestH; ++y )
								{
									for ( int x = 0; x < kTestW; ++x )
									{
										float t = (float)x / (float)(kTestW - 1);
										int off = (y * kTestW + x) * 3;
										int third = y * 3 / kTestH;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 29) - 4;
										testImage[off + 0] = (ImU8)ImClamp( (third == 0 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( (third == 1 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( (third == 2 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
									}
								}
							}
							else
							{
								for ( int i = 0; i < kTestW * kTestH * 3; ++i )
								{
									rng = rng * 1664525u + 1013904223u;
									testImage[i] = (ImU8)(rng >> 24);
								}
							}

							cieData.Accumulate( testImage.Data, kTestW, kTestH, 3,
												ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved,
												50000 );

							// Create thumbnail texture (RGB -> RGBA)
							{
								static ImVector<ImU8> rgba;
								rgba.resize( kTestW * kTestH * 4 );
								for ( int i = 0; i < kTestW * kTestH; ++i )
								{
									rgba[i * 4 + 0] = testImage[i * 3 + 0];
									rgba[i * 4 + 1] = testImage[i * 3 + 1];
									rgba[i * 4 + 2] = testImage[i * 3 + 2];
									rgba[i * 4 + 3] = 255;
								}
								ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kTestW, kTestH );
								cieThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
								cieThumbnailSize = ImVec2( (float)kTestW, (float)kTestH );
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
							cieImgData = stbi_load( filenames[fileIdx], &cieImgW, &cieImgH, &cieImgCh, 0 );
							if ( cieImgData )
							{
								int ch = (cieImgCh >= 3) ? cieImgCh : 3;
								cieData.Accumulate( cieImgData, cieImgW, cieImgH, ch,
													ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved,
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
						else if ( cieSource == 3 )
						{
							thumbTex = illlustration_img; thumbSize = illlustration_size;
						}
						else if ( cieSource == 4 )
						{
							thumbTex = background;        thumbSize = background_size;
						}
						else if ( cieSource == 5 )
						{
							thumbTex = man_img;           thumbSize = man_size;
						}
						else if ( cieSource == 6 )
						{
							thumbTex = astro_img;         thumbSize = astro_size;
						}
						if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
						{
							float thumbH = ImPlatform_LpToPx( 240.0f );
							float thumbW = thumbH * thumbSize.x / thumbSize.y;
							ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
						}
					}

					ImWidgets::PushStyleVar( StyleVar_CIEChromaticity_SignalAlpha, cieSignalAlpha );
					ImWidgets::PushStyleVar( StyleVar_CIEChromaticity_SignalRadius, cieSignalRadius );
					ImWidgets::CIEChromaticity( "##CIEMain", cieData, (ImCIEChromaticityGamut)cieGamut, cieShowBackground, (ImCIEChromaticitySignalColor)cieSignalColor, ImVec2( 0, 0 ) );
					ImWidgets::PopStyleVar( 2 );
					if ( cieSource <= 2 )
						ImGui::Text( "Source: 1920x1080 (generated)  Samples: %d", cieData.SampleCount );
					else if ( cieImgData )
						ImGui::Text( "Source: %dx%d (%d ch)  Samples: %d", cieImgW, cieImgH, cieImgCh, cieData.SampleCount );
					else
						ImGui::TextDisabled( "Failed to load image" );
				}
				DW_SsRecord( "CIE_Chromaticity", _sy0, ImGui::GetCursorPos().y ); }

				{ float _sy0 = ImGui::GetCursorPos().y;
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
						int channelCount = ImWidgets::ToneCurveChannelCount( (ImHistogramMode)tcMode );
						for ( int ch = 0; ch < channelCount; ++ch )
						{
							if ( ch > 0 ) ImGui::SameLine();
							bool isActive = (tcData.ActiveChannel == ch);
							if ( isActive )
								ImGui::PushStyleColor( ImGuiCol_Button, ImGui::GetStyleColorVec4( ImGuiCol_ButtonActive ) );
							char btnLabel[16];
							ImFormatString( btnLabel, sizeof( btnLabel ), "%s##TC_ch", ImWidgets::ToneCurveChannelName( (ImHistogramMode)tcMode, ch ) );
							if ( ImGui::Button( btnLabel ) )
								tcData.ActiveChannel = ch;
							if ( isActive )
								ImGui::PopStyleColor();
						}
					}

					if ( sourceChanged || modeChanged )
						tcNeedsUpdate = true;

					if ( modeChanged )
						tcData.Reset( ImWidgets::ToneCurveChannelCount( (ImHistogramMode)tcMode ) );

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
								static ImU8 const bars[7][3] = {
									{ 191, 191, 191 }, { 191, 191,  17 }, {  17, 191, 191 }, {  17, 191,  17 },
									{ 191,  17, 191 }, { 191,  17,  17 }, {  17,  17, 191 }
								};
								for ( int y = 0; y < kTestH; ++y )
								{
									for ( int x = 0; x < kTestW; ++x )
									{
										int barIdx = x * 7 / kTestW;
										int off = (y * kTestW + x) * 3;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 28) - 8;
										testImage[off + 0] = (ImU8)ImClamp( bars[barIdx][0] + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( bars[barIdx][1] + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( bars[barIdx][2] + noise, 0, 255 );
									}
								}
							}
							else if ( tcSource == 1 )
							{
								for ( int y = 0; y < kTestH; ++y )
								{
									for ( int x = 0; x < kTestW; ++x )
									{
										float t = (float)x / (float)(kTestW - 1);
										int off = (y * kTestW + x) * 3;
										int third = y * 3 / kTestH;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 29) - 4;
										testImage[off + 0] = (ImU8)ImClamp( (third == 0 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 1] = (ImU8)ImClamp( (third == 1 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImage[off + 2] = (ImU8)ImClamp( (third == 2 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
									}
								}
							}
							else
							{
								for ( int i = 0; i < kTestW * kTestH * 3; ++i )
								{
									rng = rng * 1664525u + 1013904223u;
									testImage[i] = (ImU8)(rng >> 24);
								}
							}

							tcHistData.Accumulate( testImage.Data, kTestW, kTestH, 3,
												   ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved, (ImHistogramMode)tcMode,
												   256, 500000 );

							// Create thumbnail texture (RGB -> RGBA)
							{
								static ImVector<ImU8> rgba;
								rgba.resize( kTestW * kTestH * 4 );
								for ( int i = 0; i < kTestW * kTestH; ++i )
								{
									rgba[i * 4 + 0] = testImage[i * 3 + 0];
									rgba[i * 4 + 1] = testImage[i * 3 + 1];
									rgba[i * 4 + 2] = testImage[i * 3 + 2];
									rgba[i * 4 + 3] = 255;
								}
								ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kTestW, kTestH );
								tcThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
								tcThumbnailSize = ImVec2( (float)kTestW, (float)kTestH );
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
							tcImgData = stbi_load( filenames[fileIdx], &tcImgW, &tcImgH, &tcImgCh, 0 );
							if ( tcImgData )
							{
								int ch = (tcImgCh >= 3) ? tcImgCh : 3;
								tcHistData.Accumulate( tcImgData, tcImgW, tcImgH, ch,
													   ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved, (ImHistogramMode)tcMode,
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
						else if ( tcSource == 3 )
						{
							thumbTex = illlustration_img; thumbSize = illlustration_size;
						}
						else if ( tcSource == 4 )
						{
							thumbTex = background;        thumbSize = background_size;
						}
						else if ( tcSource == 5 )
						{
							thumbTex = man_img;           thumbSize = man_size;
						}
						else if ( tcSource == 6 )
						{
							thumbTex = astro_img;         thumbSize = astro_size;
						}
						if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
						{
							float thumbH = ImPlatform_LpToPx( 240.0f );
							float thumbW = thumbH * thumbSize.x / thumbSize.y;
							ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
						}
					}

					ImGui::Checkbox( "Advanced Segments##TC", &tcAdvancedSegments );
					ImHistogramData const* histPtr = (tcShowHistogram && tcHistData.BinCount > 0) ? &tcHistData : NULL;
					ImWidgets::ToneCurve( "##ToneCurveMain", &tcData, (ImHistogramMode)tcMode, histPtr, tcAdvancedSegments );
					if ( tcSource <= 2 )
						ImGui::Text( "Source: 1920x1080 (generated)" );
					else if ( tcImgData )
						ImGui::Text( "Source: %dx%d (%d ch)", tcImgW, tcImgH, tcImgCh );
					else
						ImGui::TextDisabled( "Failed to load image" );
					ImGui::TextWrapped( "Click to add key. Drag to move. Drag far outside to delete. Right-click for options." );
				}
				DW_SsRecord( "Tone_Curve", _sy0, ImGui::GetCursorPos().y ); }

				{ float _sy0 = ImGui::GetCursorPos().y;
				ApplyOpenAll();
				if ( ImGui::CollapsingHeader( "Color Warper" ) )
				{
					// CanvasSize() returns pixels; ColorWarper expects logical-pixel sizing
					// and re-applies LpToPx internally. Pass 0 so the widget auto-sizes to
					// 75% of the available content width, like other big square widgets.
					float const size = 0.0f;
					static ImColorWarperData warperData;
					static bool warperInited = false;
					if ( !warperInited )
					{
						warperData.Init( 12, 6 ); warperInited = true;
					}

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
						ImGui::SliderFloat( thirdAxisLabels[ImClamp( warperSpace, 0, 6 )], &warperThirdAxis, 0.0f, 1.0f );
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
						warperData.Init( hueDivs[warperGrid], satDivs[warperGrid] );
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
						if ( warperImgData )
						{
							STBI_FREE( warperImgData ); warperImgData = NULL;
						}
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
								static ImU8 const bars[7][3] = {
									{ 191, 191, 191 }, { 191, 191, 17 }, { 17, 191, 191 }, { 17, 191, 17 },
									{ 191, 17, 191 }, { 191, 17, 17 }, { 17, 17, 191 }
								};
								for ( int y = 0; y < kH; ++y )
									for ( int x = 0; x < kW; ++x )
									{
										int off = (y * kW + x) * 3;
										int bar = x * 7 / kW;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 28) - 8;
										testImg[off + 0] = (ImU8)ImClamp( bars[bar][0] + noise, 0, 255 );
										testImg[off + 1] = (ImU8)ImClamp( bars[bar][1] + noise, 0, 255 );
										testImg[off + 2] = (ImU8)ImClamp( bars[bar][2] + noise, 0, 255 );
									}
							}
							else if ( warperSource == 1 )
							{
								for ( int y = 0; y < kH; ++y )
									for ( int x = 0; x < kW; ++x )
									{
										float t = (float)x / (float)(kW - 1);
										int off = (y * kW + x) * 3;
										int third = y * 3 / kH;
										rng = rng * 1664525u + 1013904223u;
										int noise = (int)(rng >> 29) - 4;
										testImg[off + 0] = (ImU8)ImClamp( (third == 0 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImg[off + 1] = (ImU8)ImClamp( (third == 1 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
										testImg[off + 2] = (ImU8)ImClamp( (third == 2 ? (int)(t * 255.0f) : 0) + noise, 0, 255 );
									}
							}
							else
							{
								for ( int i = 0; i < kW * kH * 3; ++i )
								{
									rng = rng * 1664525u + 1013904223u;
									testImg[i] = (ImU8)(rng >> 24);
								}
							}
							warperSignal.Accumulate( testImg.Data, kW, kH, 3,
													 ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved );
							// Create thumbnail for synthetic image
							{
								ImVector<ImU8> rgba;
								rgba.resize( kW * kH * 4 );
								for ( int i = 0; i < kW * kH; ++i )
								{
									rgba[i * 4 + 0] = testImg[i * 3 + 0];
									rgba[i * 4 + 1] = testImg[i * 3 + 1];
									rgba[i * 4 + 2] = testImg[i * 3 + 2];
									rgba[i * 4 + 3] = 255;
								}
								ImPlatform_TextureDesc td = ImPlatform_TextureDesc_Default( kW, kH );
								warperThumbnail = ImPlatform_CreateTexture( rgba.Data, &td );
								warperThumbnailSize = ImVec2( (float)kW, (float)kH );
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
							warperImgData = stbi_load( filenames[warperSource - 3], &warperImgW, &warperImgH, &warperImgCh, 0 );
							if ( warperImgData )
							{
								int ch = (warperImgCh >= 3) ? warperImgCh : 3;
								warperSignal.Accumulate( warperImgData, warperImgW, warperImgH, ch,
														 ImPixelBitDepth_UInt8, ImPixelLayout_Interleaved );
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
						else if ( warperSource == 3 )
						{
							thumbTex = illlustration_img; thumbSize = illlustration_size;
						}
						else if ( warperSource == 4 )
						{
							thumbTex = background;        thumbSize = background_size;
						}
						else if ( warperSource == 5 )
						{
							thumbTex = man_img;           thumbSize = man_size;
						}
						else if ( warperSource == 6 )
						{
							thumbTex = astro_img;         thumbSize = astro_size;
						}
						if ( thumbTex != ImTextureID_Invalid && thumbSize.x > 0.0f )
						{
							float thumbH = ImPlatform_LpToPx( 240.0f );
							float thumbW = thumbH * thumbSize.x / thumbSize.y;
							ImGui::Image( thumbTex, ImVec2( thumbW, thumbH ) );
						}
					}

					ImColorWarperOverlay const* sigPtr = (warperSignal.SampleCount > 0) ? &warperSignal : NULL;
					ImWidgets::PushStyleVar( StyleVar_CIEChromaticity_SignalAlpha, warperSignalAlpha );
					ImWidgets::PushStyleVar( StyleVar_CIEChromaticity_SignalRadius, warperSignalRadius );
					ColorWarper( "##WarperMain", &warperData,
								 (ImColorWarperMode)warperMode, (ImColorWarperSpace)warperSpace, warperThirdAxis, sigPtr,
								 (ImColorWarperSignalColor)warperSignalColor, warperAxisAngle, ImVec2(size, size) );
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
				DW_SsRecord( "Color_Warper", _sy0, ImGui::GetCursorPos().y ); }

				ImGui::TreePop();
			}
			{
				float _sy0 = ImGui::GetCursorPos().y;
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
							ImUnitDef_Simple( "Fahrenheit", "F", 9.0f / 5.0f, 32.0f ),
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
				DW_SsRecord( "Misc", _sy0, ImGui::GetCursorPos().y ); }  // end Misc block

			ApplyOpenAll();
			if ( ImGui::TreeNode( "Status##Widgets" ) )
			{
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Diff View" ) )
					{
						static ImWidgets::ImWidgetsDiffEntry s_diff[] = {
							{ ImWidgets::ImWidgetsDiffLine_Hunk,    -1, -1, "@@ -10,7 +10,8 @@" },
							{ ImWidgets::ImWidgetsDiffLine_Context, 10, 10, "int main(int argc, char** argv)" },
							{ ImWidgets::ImWidgetsDiffLine_Context, 11, 11, "{" },
							{ ImWidgets::ImWidgetsDiffLine_Removed, 12, -1, "    printf(\"hello\\n\");" },
							{ ImWidgets::ImWidgetsDiffLine_Added,   -1, 12, "    printf(\"hello %s\\n\", argv[0]);" },
							{ ImWidgets::ImWidgetsDiffLine_Added,   -1, 13, "    printf(\"argc = %d\\n\", argc);" },
							{ ImWidgets::ImWidgetsDiffLine_Context, 13, 14, "    return 0;" },
							{ ImWidgets::ImWidgetsDiffLine_Context, 14, 15, "}" },
						};
						ImWidgets::DiffView( "##diff", s_diff, IM_ARRAYSIZE( s_diff ), ImVec2( 0.0f, 200.0f ) );
					}
					DW_SsRecord( "Diff_View", _sy0, ImGui::GetCursorPos().y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "Math##Widgets" ) )
			{
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Matrix Editor" ) )
					{
						static float M[ 16 ] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1 };
						ImWidgets::MatrixEditor( "4x4 (float)", M, 4, 4, 0.05f, "%.3f" );
						static double D[ 6 ] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6 };
						ImWidgets::MatrixEditorDouble( "2x3 (double)", D, 2, 3, 0.01f, "%.4f" );
					}
					DW_SsRecord( "Matrix_Editor", _sy0, ImGui::GetCursorPos().y );
				}
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Vector Field" ) )
					{
						struct VF { static ImVec2 swirl( ImVec2 uv, void* )
						{
							float x = uv.x - 0.5f, y = uv.y - 0.5f;
							return ImVec2( -y, x );
						} };
						ImDrawList* dl = ImGui::GetWindowDrawList();
						ImVec2 p = ImGui::GetCursorScreenPos();
						ImVec2 sz( ImGui::GetContentRegionAvail().x, ImPlatform_LpToPx( 260.0f ) );
						dl->AddRectFilled( p, p + sz, IM_COL32( 25, 30, 40, 255 ) );
						ImWidgets::DrawVectorField( dl, ImRect( p, p + sz ),
							VF::swirl, NULL, 24, 16, 16.0f, IM_COL32_WHITE, 1.0f, true );
						ImGui::Dummy( sz );
					}
					DW_SsRecord( "Vector_Field", _sy0, ImGui::GetCursorPos().y );
				}
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Stream Lines" ) )
					{
						struct VF { static ImVec2 saddle( ImVec2 uv, void* )
						{
							float x = uv.x - 0.5f, y = uv.y - 0.5f;
							return ImVec2( y, x );
						} };
						ImDrawList* dl = ImGui::GetWindowDrawList();
						ImVec2 p = ImGui::GetCursorScreenPos();
						ImVec2 sz( ImGui::GetContentRegionAvail().x, ImPlatform_LpToPx( 240.0f ) );
						dl->AddRectFilled( p, p + sz, IM_COL32( 25, 30, 40, 255 ) );
						ImVec2 seeds[ 20 ];
						for ( int k = 0; k < 20; ++k )
							seeds[ k ] = ImVec2( 0.05f + ( k % 5 ) * 0.225f, 0.05f + ( k / 5 ) * 0.30f );
						ImWidgets::DrawStreamLines( dl, ImRect( p, p + sz ),
							VF::saddle, NULL, seeds, 20, 100, 3.0f,
							IM_COL32( 130, 220, 255, 220 ), 1.5f );
						ImGui::Dummy( sz );
					}
					DW_SsRecord( "Stream_Lines", _sy0, ImGui::GetCursorPos().y );
				}
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Polynomial Roots" ) )
					{
						ImDrawList* dl = ImGui::GetWindowDrawList();
						ImVec2 p = ImGui::GetCursorScreenPos();
						float side = ImMin( ImGui::GetContentRegionAvail().x, ImPlatform_LpToPx( 260.0f ) );
						ImVec2 sz( side, side );
						dl->AddRectFilled( p, p + sz, IM_COL32( 25, 30, 40, 255 ) );
						ImVec2 roots[ 4 ] = { ImVec2( 1, 0 ), ImVec2( -1, 0 ), ImVec2( 0, 1 ), ImVec2( 0, -1 ) };
						ImWidgets::DrawPolynomialRoots( dl, ImRect( p, p + sz ), roots, 4, 1.5f );
						ImGui::Dummy( sz );
					}
					DW_SsRecord( "Polynomial_Roots", _sy0, ImGui::GetCursorPos().y );
				}
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Curve Sketch" ) )
					{
						struct CS { static float f( float x, void* )
						{
							return ImSin( x ) + 0.3f * ImSin( 4.0f * x );
						} };
						ImDrawList* dl = ImGui::GetWindowDrawList();
						ImVec2 p = ImGui::GetCursorScreenPos();
						ImVec2 sz( ImGui::GetContentRegionAvail().x, ImPlatform_LpToPx( 220.0f ) );
						dl->AddRectFilled( p, p + sz, IM_COL32( 25, 30, 40, 255 ) );
						ImWidgets::DrawCurveSketch( dl, ImRect( p, p + sz ),
							CS::f, NULL, -IM_PI * 2.0f, IM_PI * 2.0f, -1.4f, 1.4f, 256 );
						ImGui::Dummy( sz );
					}
					DW_SsRecord( "Curve_Sketch", _sy0, ImGui::GetCursorPos().y );
				}
				ImGui::TreePop();
			}
			ApplyOpenAll();
			if ( ImGui::TreeNode( "3D Inputs##Widgets" ) )
			{
				{
					float _sy0 = ImGui::GetCursorPos().y;
					ApplyOpenAll();
					if ( ImGui::CollapsingHeader( "Vector Input 3D (azimuth + elevation)" ) )
					{
						static float v[ 3 ] = { 0.5f, 0.3f, 1.0f };
						ImWidgets::VectorInput3D( "Direction", v, 200.0f );
					}
					DW_SsRecord( "Vector_Input_3D", _sy0, ImGui::GetCursorPos().y );
				}
				ImGui::TreePop();
			}

			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Font Inspector" ) )
			{
				ImGui::Text( "Font inspector:" );
				static int fi_mode = 0;
				const char* modes[] = { "Grid", "Metrics", "Curves", "Kerning" };
				ImGui::Combo( "Mode", &fi_mode, modes, IM_ARRAYSIZE( modes ) );
				// Curated picker -- same font set as the other demos; includes the default.
				struct FiFont
				{
					const char* name; ImFont* font;
				};
				FiFont fi_list[] = {
					{ "(default)",       ImGui::GetFont() },
					{ "Fira Code",       g_firaCodeFont },
					{ "Monblock",        g_monblockFont },
					{ "Cinzel",          g_cinzelFont },
					{ "Alfa Slab",       g_alfaSlabFont },
					{ "Classical",       g_classicalFont },
					{ "Foglighten",      g_foglihtenFont },
					{ "Steelworks",      g_steelworksFont },
					{ "Trench Slab",     g_trenchSlabFont },
					{ "Bright March",    g_brightMarchFont },
					{ "Dotted",          g_dottedFont },
					{ "Gimbo",           g_gimboFont },
				};
				int fi_n = IM_ARRAYSIZE( fi_list );
				static int fi_sel = 0;
				if ( fi_sel < 0 || fi_sel >= fi_n || !fi_list[fi_sel].font )
				{
					fi_sel = 0;
					for ( int i = 0; i < fi_n; ++i ) if ( fi_list[i].font )
					{
						fi_sel = i; break;
					}
				}
				if ( ImGui::BeginCombo( "Font##fi", fi_list[fi_sel].name ) )
				{
					for ( int i = 0; i < fi_n; ++i )
					{
						if ( !fi_list[i].font ) continue;
						if ( ImGui::Selectable( fi_list[i].name, i == fi_sel ) ) fi_sel = i;
					}
					ImGui::EndCombo();
				}
				ImWidgets::FontInspector( "font_ins", fi_list[fi_sel].font,
										  (ImFontInspectorMode)fi_mode, ImPlatform_LpToPx( 48.0f ), ImVec2( 0, 320 ) );
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Notched Dial" ) )
			{
				ImGui::Text( "Notched Dial:" );
				static float nv = 0.0f;
				static float stops[] = { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f };
				static const char* slabels[] = { "Off", "Low", "Mid", "High", "Max" };
				ImWidgets::NotchedDial( "dial", &nv, stops, 5, slabels, ImVec2( 220, 240 ) );
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Angle Dial" ) )
			{
				ImGui::TextWrapped( "Continuous angle dial. Value is in degrees "
					"(0 = East, CCW positive). Drag to set, scroll to nudge "
					"(+/-1, +/-10 with Shift), Ctrl snaps to 15, double-click to "
					"type an exact value." );
				ImGui::Spacing();

				// Full-turn dial: a heading angle stored in degrees [-180, 180].
				static float heading_deg = 30.0f;
				ImGui::BeginGroup();
				ImGui::TextUnformatted( "Full turn (degrees, [-180, 180])" );
				ImWidgets::AngleDial( "##angle_full", &heading_deg, -180.0f, 180.0f );
				ImGui::Text( "heading = %.1f deg", heading_deg );
				ImGui::EndGroup();

				ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemSpacing.x * 3.0f );

				// Radian-stored value: convert to/from degrees around the dial,
				// exactly how the inspector's type_widgets::edit_angle wraps it.
				static float rotation_rad = 0.0f;
				ImGui::BeginGroup();
				ImGui::TextUnformatted( "Stored in radians [-pi, pi]" );
				float rot_deg = rotation_rad * 57.29577951308232f;
				if ( ImWidgets::AngleDial( "##angle_rad", &rot_deg, -180.0f, 180.0f ) )
					rotation_rad = rot_deg * 0.017453292519943295f;
				ImGui::Text( "rotation = %.3f rad", rotation_rad );
				ImGui::EndGroup();

				ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemSpacing.x * 3.0f );

				// Clamped arc: a sub-360 range draws boundary ticks and clamps.
				static float cone_deg = 20.0f;
				ImGui::BeginGroup();
				ImGui::TextUnformatted( "Clamped arc [0, 90]" );
				ImWidgets::AngleDial( "##angle_arc", &cone_deg, 0.0f, 90.0f );
				ImGui::Text( "cone = %.1f deg", cone_deg );
				ImGui::EndGroup();
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Equation Editor" ) )
			{
				ImGui::Text( "Equation input ($..$ inline, $$..$$ block, multiline):" );
				static int eq_align = 3; // 0=Top, 1=Center, 2=Bottom, 3=Baseline
				const char* eq_align_names[] = { "Inline Top", "Inline Center", "Inline Bottom", "Inline Baseline" };
				ImGui::Combo( "Inline alignment", &eq_align, eq_align_names, IM_ARRAYSIZE( eq_align_names ) );
				int eq_flags = 0;
				if ( eq_align == 0 ) eq_flags = ImWidgetsEquationFlags_AlignInlineTop;
				else if ( eq_align == 1 ) eq_flags = ImWidgetsEquationFlags_AlignInlineCenter;
				else if ( eq_align == 2 ) eq_flags = ImWidgetsEquationFlags_AlignInlineBottom;
				else eq_flags = ImWidgetsEquationFlags_AlignInlineBaseline;
				static char eq_buf[1024] =
					"Mass-energy: $E = mc^2$.\n"
					"\n"
					"Euler:\n"
					"$$e^{i\\pi} + 1 = 0$$\n"
					"\n"
					"Gaussian integral:\n"
					"$$\\int_{-\\infty}^{\\infty} e^{-x^2} dx = \\sqrt{\\pi}$$\n";
				ImWidgets::EquationInput( "eq", eq_buf, sizeof( eq_buf ),
										  ImVec2( 0, 360 ), (ImWidgetsEquationFlags)eq_flags );
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "3D LUT Viewer" ) )
			{
				static ImColorLUT3D lut;
				if ( lut.Size == 0 ) ImWidgets::InitIdentityLUT3D( lut, 9 );
				ImGui::Text( "Size: %d  (identity)", lut.Size );
				if ( ImGui::Button( "Reset identity" ) ) ImWidgets::InitIdentityLUT3D( lut, lut.Size > 0 ? lut.Size : 9 );
				ImWidgets::ColorLUT3DViewer( "lut3d", &lut, ImVec2( 0, 0 ) );
				ImGui::TextDisabled( "Click a cell to edit." );
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "LookDev A/B" ) )
			{
				ImGui::TextWrapped(
					"A/B compare with rotatable divider. Pick two images to compare; "
					"drag the yellow midpoint to slide the divider, blue handle to rotate, "
					"and A/B button in the top-right corner to swap sides." );
				static const char* img_names[] = {
					"Astronaut", "Clock", "Man", "Illustration", "Bike", "Interior"
				};
				ImTextureID tex_arr[6] = { astro_img, clock_img, man_img, illlustration_img, bike_img, background };
				static int idx_a = 0, idx_b = 2;
				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 160.0f ) );
				ImGui::Combo( "Left (A)", &idx_a, img_names, IM_ARRAYSIZE( img_names ) );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 160.0f ) );
				ImGui::Combo( "Right (B)", &idx_b, img_names, IM_ARRAYSIZE( img_names ) );
				static ImLookDevState state;
				ImTextureID texA = tex_arr[idx_a];
				ImTextureID texB = tex_arr[idx_b];
				// A/B swap lives outside the widget now (the top-right button in
				// the canvas is the maximize button, mirroring Histogram et al).
				ImGui::Checkbox( "Swap A/B##ldv_swap", &state.Swap );
				static bool s_ldv_maximized = false;
				if ( ImGui::Button( "Maximize (demo-driven)" ) ) s_ldv_maximized = true;
				ImGui::SameLine();
				ImGui::TextDisabled( "Widget also has its own maximize button." );
				if ( texA != ImTextureID_Invalid && texB != ImTextureID_Invalid )
				{
					ImWidgets::LookDevCompare( "ldv", texA, texB,
											   ImVec2( 0, 0 ), ImVec2( 1, 1 ),
											   ImVec2( 0, 0 ), ImVec2( 1, 1 ),
											   &state, ImVec2( 0, 360 ) );
				}
				else
				{
					ImGui::TextDisabled( "Images not loaded yet." );
				}
				if ( s_ldv_maximized && texA != ImTextureID_Invalid && texB != ImTextureID_Invalid )
				{
					ImGui::SetNextWindowSize( ImPlatform_LpToPx( ImVec2( 1200, 720 ) ), ImGuiCond_Appearing );
					if ( ImGui::Begin( "Dear Widgets - LookDev (Maximized)",
									   &s_ldv_maximized, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings ) )
					{
						ImWidgets::LookDevCompare( "ldv_max", texA, texB,
												   ImVec2( 0, 0 ), ImVec2( 1, 1 ),
												   ImVec2( 0, 0 ), ImVec2( 1, 1 ),
												   &state, ImGui::GetContentRegionAvail() );
					}
					ImGui::End();
				}
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Color Difference (shader)" ) )
			{
				ImGui::TextWrapped( "False-color dE map between two images. "
									"Pick the formula (dE76 / dE-OK / dE94 / dE2000) and the color ramp." );
				static const char* img_names_de[] = {
					"Astronaut", "Clock", "Man", "Illustration", "Bike", "Interior"
				};
				ImTextureID tex_arr_de[6] = { astro_img, clock_img, man_img, illlustration_img, bike_img, background };
				// Default to two different images so the false-color map has
				// visible signal on first open (was Astronaut-vs-Astronaut -> zero dE).
				static int de_a = 0, de_b = 2;
				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 160.0f ) );
				ImGui::Combo( "Image A##de", &de_a, img_names_de, IM_ARRAYSIZE( img_names_de ) );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 160.0f ) );
				ImGui::Combo( "Image B##de", &de_b, img_names_de, IM_ARRAYSIZE( img_names_de ) );
				static ImWidgets::ImDeltaECompareState de_state;
				ImTextureID ta = tex_arr_de[de_a];
				ImTextureID tb = tex_arr_de[de_b];
				if ( ta != ImTextureID_Invalid && tb != ImTextureID_Invalid )
				{
					ImWidgets::ColorDifferenceImageViewer( "de_img", ta, tb, &de_state,
														   ImVec2( 0, 360 ) );
				}
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "LookDev Inspector (shader)" ) )
			{
				ImGui::TextWrapped( "Shader-based A/B compare with per-side exposure, black, white, gamma. "
									"Requires the lookdev_inspector shader to load successfully." );
				static const char* img_names_ldi[] = {
					"Astronaut", "Clock", "Man", "Illustration", "Bike", "Interior"
				};
				ImTextureID tex_arr_ldi[6] = { astro_img, clock_img, man_img, illlustration_img, bike_img, background };
				static int ldi_a = 0, ldi_b = 2;
				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 160.0f ) );
				ImGui::Combo( "Left (A)##ldi", &ldi_a, img_names_ldi, IM_ARRAYSIZE( img_names_ldi ) );
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 160.0f ) );
				ImGui::Combo( "Right (B)##ldi", &ldi_b, img_names_ldi, IM_ARRAYSIZE( img_names_ldi ) );
				static ImLookDevInspectorState ldi_state;
				ImGui::Checkbox( "Swap A/B##ldi_swap", &ldi_state.Divider.Swap );
				ImTextureID ta = tex_arr_ldi[ldi_a];
				ImTextureID tb = tex_arr_ldi[ldi_b];
				if ( ta != ImTextureID_Invalid && tb != ImTextureID_Invalid )
				{
					ImWidgets::LookDevInspector( "ldi", ta, tb, &ldi_state,
												 ImVec2( 0, 360 ) );
				}
			}
			ApplyOpenAll();
			if ( ImGui::CollapsingHeader( "Volume Slice Viewer" ) )
			{
				static ImWidgets::ImVolumeSliceState vsv;
				static ImWidgets::ImVolumeViewerState vvs;
				static ImVector<float> voxels;
				static int vol_field = 0;
				static float vol_seed = 0.0f;
				const char* field_names[] = { "Sum-of-sines", "Sphere", "Nested Spheres", "Torus", "Gyroid", "Spherical Noise" };
				auto rebuild_field = [ & ](){
					const int W = 64, H = 64, D = 64;
					voxels.resize( W * H * D );
					ImWidgets::VolumeGenerateField( voxels.Data, W, H, D,
													(ImWidgets::ImVolumeField)vol_field, vol_seed );
					vsv.Voxels = voxels.Data;
					vsv.Width = W; vsv.Height = H; vsv.Depth = D;
					// Invalidate the VolumeViewer's 3D texture cache so it
					// re-uploads on the next frame with the new voxels.
					vvs.Tex3DLastVoxels = NULL;
					};
				if ( voxels.Size == 0 ) rebuild_field();

				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 200.0f ) );
				if ( ImGui::Combo( "Field##Vol", &vol_field, field_names, IM_ARRAYSIZE( field_names ) ) ) rebuild_field();
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 180.0f ) );
				if ( ImGui::SliderFloat( "Seed##Vol", &vol_seed, 0.0f, 8.0f, "%.2f" ) ) rebuild_field();

				ImGui::BeginChild( "##vsv_50", ImVec2( ImGui::GetContentRegionAvail().x * 0.5f, 0 ), ImGuiChildFlags_AutoResizeY );
				ImWidgets::VolumeSliceViewer( "vsv", &vsv, ImVec2( 0, 0 ) );
				ImGui::EndChild();

				ImGui::Separator();
				ImGui::TextUnformatted( "VolumeViewer (raymarch / MIP / iso-surface + 3-slice layout):" );
				// Dedicated field/seed picker for the VolumeViewer -- shares the
				// same voxel buffer as the Slice viewer above, so selecting a
				// field here updates both and triggers a 3D-texture re-upload.
				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 200.0f ) );
				if ( ImGui::Combo( "Field##VolVV", &vol_field, field_names, IM_ARRAYSIZE( field_names ) ) ) rebuild_field();
				ImGui::SameLine();
				ImGui::SetNextItemWidth( ImPlatform_LpToPx( 180.0f ) );
				if ( ImGui::SliderFloat( "Seed##VolVV", &vol_seed, 0.0f, 8.0f, "%.2f" ) ) rebuild_field();

				vvs.Voxels = voxels.Data;
				vvs.Width = vsv.Width; vvs.Height = vsv.Height; vvs.Depth = vsv.Depth;
				// 75% width -- sized similarly to the Volume Slice Viewer above.
				ImGui::BeginChild( "##vv_75", ImVec2( ImGui::GetContentRegionAvail().x * 0.75f, 0 ),
					ImGuiChildFlags_AutoResizeY );
				ImWidgets::VolumeViewer( "vv", &vvs );
				ImGui::EndChild();
			}
		}

		s_open_all = 0;

		ImGui::End();
		ImGui::PopStyleVar();
	}
}

#include <imgui_demo.cpp>
