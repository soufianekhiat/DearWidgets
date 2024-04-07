#include <ImPlatform.h>

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_win32.cpp>
#define DIRECTINPUT_VERSION 0x0800
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
#include <imgui/backends/imgui_impl_glfw.cpp>
#include <GLFW/glfw3.h>

#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
#include <imgui/backends/imgui_impl_dx9.cpp>
#include <d3d9.h>

#pragma comment( lib, "d3d9.lib" )

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
#include <imgui/backends/imgui_impl_dx10.cpp>

#pragma comment( lib, "d3d10.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxgi.lib" )

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#include <imgui/backends/imgui_impl_dx11.cpp>

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxgi.lib" )

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#include <imgui/backends/imgui_impl_dx12.cpp>

#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxgi.lib" )

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_VULKAN)
#include <imgui/backends/imgui_impl_vulkan.cpp>
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_METAL)
#include <imgui/backends/imgui_impl_metal.cpp>
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#include <imgui/backends/imgui_impl_opengl2.cpp>

#pragma comment( lib, "opengl32.lib" )
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
#include <imgui/backends/imgui_impl_opengl3.cpp>

#pragma comment( lib, "opengl32.lib" )

#define IM_OPENGL_GLAD // Use Glad by default

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_WGPU)
#include <imgui/backends/imgui_impl_wgpu.cpp>

#endif

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
#if defined(IM_OPENGL_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IM_OPENGL_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IM_OPENGL_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IM_OPENGL_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IM_OPENGL_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#error OpenGL Not defined properly {IM_OPENGL_GL3W, IM_OPENGL_GLEW, IM_OPENGL_GLAD, IM_OPENGL_GLBINDING2, IM_OPENGL_GLBINDING3} or custom
#endif
#endif

PlatformDataImpl PlatformData;

namespace ImWidgets
{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
	void	ImCreateRenderTarget()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ID3D10Texture2D* pBackBuffer;
		PlatformData.pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		PlatformData.pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &PlatformData.pMainRenderTargetView);
		pBackBuffer->Release();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
		ID3D11Texture2D* pBackBuffer;
		PlatformData.pSwapChain->GetBuffer( 0, IID_PPV_ARGS( &pBackBuffer ) );
		PlatformData.pD3DDevice->CreateRenderTargetView( pBackBuffer, NULL, &PlatformData.pMainRenderTargetView );
		pBackBuffer->Release();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#endif
	}
#endif

	void	ImCleanupRenderTarget()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		if (PlatformData.pMainRenderTargetView)
		{
			PlatformData.pMainRenderTargetView->Release();
			PlatformData.pMainRenderTargetView = nullptr;
		}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
		ID3D11Texture2D* pBackBuffer;
		PlatformData.pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		PlatformData.pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &PlatformData.pMainRenderTargetView);
		pBackBuffer->Release();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#endif
	}

	bool ImCreateDeviceD3D( HWND hWnd )
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)

		if ( ( PlatformData.pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) == nullptr )
			return false;

		// Create the D3DDevice
		ZeroMemory( &PlatformData.oD3Dpp, sizeof( PlatformData.oD3Dpp ) );
		PlatformData.oD3Dpp.Windowed				= TRUE;
		PlatformData.oD3Dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
		PlatformData.oD3Dpp.BackBufferFormat		= D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
		PlatformData.oD3Dpp.EnableAutoDepthStencil	= TRUE;
		PlatformData.oD3Dpp.AutoDepthStencilFormat	= D3DFMT_D16;
		//PlatformData.oD3Dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;	// Present with vsync
		PlatformData.oD3Dpp.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;		// Present without vsync, maximum unthrottled framerate
		if ( PlatformData.pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &PlatformData.oD3Dpp, &PlatformData.pD3DDevice ) < 0 )
			return false;

		return true;

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)

		// Setup swap chain
		DXGI_SWAP_CHAIN_DESC oSwapDesc;
		ZeroMemory( &oSwapDesc, sizeof( oSwapDesc ) );

		oSwapDesc.BufferCount							= 2;
		oSwapDesc.BufferDesc.Width						= 0;
		oSwapDesc.BufferDesc.Height						= 0;
		oSwapDesc.BufferDesc.Format						= DXGI_FORMAT_R8G8B8A8_UNORM;
		oSwapDesc.BufferDesc.RefreshRate.Numerator		= 60;
		oSwapDesc.BufferDesc.RefreshRate.Denominator	= 1;
		oSwapDesc.Flags									= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		oSwapDesc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		oSwapDesc.OutputWindow							= hWnd;
		oSwapDesc.SampleDesc.Count						= 1;
		oSwapDesc.SampleDesc.Quality					= 0;
		oSwapDesc.Windowed								= TRUE;
		oSwapDesc.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;

		UINT uCreateDeviceFlags = 0;
		//createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
		if ( D3D10CreateDeviceAndSwapChain( nullptr, D3D10_DRIVER_TYPE_HARDWARE, nullptr, uCreateDeviceFlags, D3D10_SDK_VERSION, &oSwapDesc, &PlatformData.pSwapChain, &PlatformData.pD3DDevice ) != S_OK )
			return false;

		ImCreateRenderTarget();

		return true;

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)

		// Setup swap chain
		DXGI_SWAP_CHAIN_DESC oSwapDesc;
		ZeroMemory( &oSwapDesc, sizeof( oSwapDesc ) );
		oSwapDesc.BufferCount							= 2;
		oSwapDesc.BufferDesc.Width						= 0;
		oSwapDesc.BufferDesc.Height						= 0;
		oSwapDesc.BufferDesc.Format						= DXGI_FORMAT_R8G8B8A8_UNORM;
		oSwapDesc.BufferDesc.RefreshRate.Numerator		= 60;
		oSwapDesc.BufferDesc.RefreshRate.Denominator	= 1;
		oSwapDesc.Flags									= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		oSwapDesc.BufferUsage							= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		oSwapDesc.OutputWindow							= hWnd;
		oSwapDesc.SampleDesc.Count						= 1;
		oSwapDesc.SampleDesc.Quality					= 0;
		oSwapDesc.Windowed								= TRUE;
		oSwapDesc.SwapEffect							= DXGI_SWAP_EFFECT_DISCARD;

		UINT uCreateDeviceFlags = 0;
		//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		D3D_FEATURE_LEVEL featureLevel;
		const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
		if ( D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, uCreateDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &oSwapDesc, &PlatformData.pSwapChain, &PlatformData.pD3DDevice, &featureLevel, &PlatformData.pD3DDeviceContext ) != S_OK )
			return false;

		ImCreateRenderTarget();

		return true;
#endif
	}

	void ImCleanupDeviceD3D()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
		if ( PlatformData.pD3DDevice )
		{
			PlatformData.pD3DDevice->Release();
			PlatformData.pD3DDevice = nullptr;
		}
		if ( PlatformData.pD3D )
		{
			PlatformData.pD3D->Release();
			PlatformData.pD3D = nullptr;
		}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ImCleanupRenderTarget();
		if ( PlatformData.pSwapChain )
		{
			PlatformData.pSwapChain->Release();
			PlatformData.pSwapChain = nullptr;
		}
		if ( g_pd3dDevice )
		{
			PlatformData.pD3DDevice->Release();
			PlatformData.pD3DDevice = nullptr;
		}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
		ImCleanupRenderTarget();
		if ( PlatformData.pSwapChain )
		{
			PlatformData.pSwapChain->Release();
			PlatformData.pSwapChain = nullptr;
		}
		if ( g_pd3dDeviceContext )
		{
			PlatformData.pD3DDeviceContext->Release();
			PlatformData.pD3DDeviceContext = nullptr;
		}
		if ( g_pd3dDevice )
		{
			PlatformData.pD3DDevice->Release();
			PlatformData.pD3DDevice = nullptr;
		}
#endif
	}

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
	void ImResetDevice()
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		HRESULT hr = PlatformData.pD3DDevice->Reset( &PlatformData.oD3Dpp );
		if ( hr == D3DERR_INVALIDCALL )
			IM_ASSERT( 0 );

		ImGui_ImplDX9_CreateDeviceObjects();
	}
#endif
#endif

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
	// Win32 message handler
	LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
	{
		if ( ImGui_ImplWin32_WndProcHandler( hWnd, msg, wParam, lParam ) )
			return true;

		switch ( msg )
		{
		case WM_SIZE:
			if ( wParam != SIZE_MINIMIZED )
			{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
				// TODO
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
				// TODO
				//glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
				if ( PlatformData.pD3DDevice != nullptr )
				{
					PlatformData.oD3Dpp.BackBufferWidth		= LOWORD(lParam);
					PlatformData.oD3Dpp.BackBufferHeight	= HIWORD(lParam);
					ImResetDevice();
				}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
				if ( PlatformData.pD3DDevice != nullptr )
				{
					ImCleanupRenderTarget();
					PlatformData.pSwapChain->ResizeBuffers( 0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0 );
					ImCreateRenderTarget();
				}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
				if ( PlatformData.pD3DDevice != nullptr )
				{
					ImCleanupRenderTarget();
					PlatformData.pSwapChain->ResizeBuffers( 0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0 );
					ImCreateRenderTarget();
				}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif
			}
			return 0;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		//case WM_DPICHANGED:
		//	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		//	{
		//		//const int dpi = HIWORD(wParam);
		//		//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
		//		const RECT* suggested_rect = (RECT*)lParam;
		//		::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		//	}
		//	break;
		}
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}
#endif

	bool ImInitWindow( char const* pWindowsName, ImU32 const uWidth, ImU32 const uHeight )
	{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		TCHAR szName[1024];
		_tcscpy(szName, pWindowsName);

		ImGui_ImplWin32_EnableDpiAwareness();

		PlatformData.oWinStruct = { sizeof( WNDCLASSEX ), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, szName, nullptr };
		::RegisterClassEx( &PlatformData.oWinStruct );

		PlatformData.pHandle = ::CreateWindow( PlatformData.oWinStruct.lpszClassName, szName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, uWidth, uHeight, nullptr, nullptr, PlatformData.oWinStruct.hInstance, nullptr );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)

		if ( !glfwInit() )
			return false;

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
		// GL 3.0 + GLSL 130
		const char* glsl_version = "#version 130";
		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
		//glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
		//glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
		///glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE ); // 3.2+ only
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // 3.0+ only
#endif

		PlatformData.pWindow = glfwCreateWindow( uWidth, uHeight, pWindowsName, nullptr, nullptr );
		if ( PlatformData.pWindow == nullptr )
			return false;

		glfwMakeContextCurrent( PlatformData.pWindow );
		glfwSwapInterval( 1 );

#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif

		return true;
	}

	bool ImInitGfxAPI()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		PlatformData.pDevContext = GetDC( PlatformData.pHandle );

		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
			PFD_TYPE_RGBA,		// The kind of framebuffer. RGBA or palette.
			32,					// Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,					// Number of bits for the depthbuffer
			8,					// Number of bits for the stencilbuffer
			0,					// Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		int pixelFormat = ChoosePixelFormat( PlatformData.pDevContext, &pfd );
		SetPixelFormat( PlatformData.pDevContext, pixelFormat, &pfd );
		HGLRC glContext = wglCreateContext( PlatformData.pDevContext );

		wglMakeCurrent( PlatformData.pDevContext, glContext );
#endif

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
		if ( !ImCreateDeviceD3D( PlatformData.pHandle ) )
		{
			ImCleanupDeviceD3D();
			::UnregisterClass(PlatformData.oWinStruct.lpszClassName, PlatformData.oWinStruct.hInstance);
			return false;
		}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

		return true;
	}

	bool ImShowWindow()
	{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		::ShowWindow( PlatformData.pHandle, SW_SHOWDEFAULT );
		::UpdateWindow( PlatformData.pHandle );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)

		

#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif
		return true;
	}

	bool ImInit( char const* pWindowsName, ImU32 const uWidth, ImU32 const uHeight )
	{
		bool bGood = true;

		bGood &= ImWidgets::ImInitWindow( pWindowsName, uWidth, uHeight );
		bGood &= ImWidgets::ImInitGfxAPI();
		bGood &= ImWidgets::ImShowWindow();

		IMGUI_CHECKVERSION();
		bGood &= ImGui::CreateContext() != nullptr;

		return bGood;
	}

	void ImNewFrame()
	{
		ImWidgets::ImGfxAPINewFrame();
		ImWidgets::ImPlatformNewFrame();
		ImGui::NewFrame();
	}

	void ImBegin()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)

		// Initialize OpenGL loader
#if defined(IM_OPENGL_GL3W)
		bool err = gl3wInit() != 0;
#elif defined(IM_OPENGL_GLEW)
		bool err = glewInit() != GLEW_OK;
#elif defined(IM_OPENGL_GLAD)
		bool err = gladLoadGL() == 0;
#elif defined(IM_OPENGL_GLBINDING2)
		bool err = false;
		glbinding::Binding::initialize();
#elif defined(IM_OPENGL_GLBINDING3)
		bool err = false;
		glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
		bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
#endif

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		ImGui_ImplWin32_Init( PlatformData.pHandle );
		ZeroMemory( &PlatformData.oMessage, sizeof( PlatformData.oMessage ) );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
		ImGui_ImplGlfw_InitForOpenGL( PlatformData.pWindow, true );
#endif
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
		ImGui_ImplOpenGL2_Init();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
		ImGui_ImplOpenGL3_Init( PlatformData.pGLSLVersion );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
		ImGui_ImplDX9_Init( PlatformData.pD3DDevice );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ImGui_ImplDX10_Init( PlatformData.pD3DDevice );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
		ImGui_ImplDX11_Init( PlatformData.pD3DDevice, PlatformData.pD3DDeviceContext );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
		ImGui_ImplDX12_Init(...);
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
		ImGui_ImplVulkan_Init(..);
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
		ImGui_ImplMetal_Init(..);
#else
#error IM_CURRENT_TARGET not specified correctly
#endif
	}

	void ImEnd()
	{
		ImShutdownGfxAPI();
		ImShutdownWindow();

		ImGui::DestroyContext();

		ImShutdownPostGfxAPI();

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		::DestroyWindow( PlatformData.pHandle );
		::UnregisterClass( PlatformData.oWinStruct.lpszClassName, PlatformData.oWinStruct.hInstance );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
		glfwDestroyWindow( PlatformData.pWindow );
		glfwTerminate();
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif
	}

	bool ImBeginFrame()
	{
		if ( ImWidgets::ImPlatformEvents() )
			return false;

		ImWidgets::ImNewFrame();

		return true;
	}

	void ImEndFrame( ImVec4 const vClearColor )
	{
		ImWidgets::ImGfxAPIClear( vClearColor );
		ImWidgets::ImGfxAPIRender();
	}

	void ImEndGfx()
	{
		ImWidgets::ImGfxAPISwapBuffer();
	}

	//////////////////////////////////////////////////////////////////////////

	bool ImPlatformContinue()
	{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		return PlatformData.oMessage.message != WM_QUIT;
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
		return !glfwWindowShouldClose( PlatformData.pWindow );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif
	}

	bool ImPlatformEvents()
	{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		if ( ::PeekMessage( &PlatformData.oMessage, nullptr, 0U, 0U, PM_REMOVE ) )
		{
			::TranslateMessage( &PlatformData.oMessage );
			::DispatchMessage( &PlatformData.oMessage );

			return true;
		}
		else
		{
			return false;
		}
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
		glfwPollEvents();
		return false;
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif
	}

	bool ImGfxAPINewFrame()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
		ImGui_ImplOpenGL2_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
		ImGui_ImplOpenGL3_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
		ImGui_ImplDX9_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ImGui_ImplDX10_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
		ImGui_ImplDX11_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
		ImGui_ImplDX12_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
		ImGui_ImplVulkan_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
		ImGui_ImplMetal_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_WGPU)
		ImGui_ImplWGPU_NewFrame();
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

		return true;
	}

	bool ImPlatformNewFrame()
	{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		ImGui_ImplWin32_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
		ImGui_ImplGlfw_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif

		return true;
	}

	bool ImGfxAPIClear( ImVec4 const vClearColor )
	{
		ImGui::EndFrame();

		ImS32 iWidth, iHeight;

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)

		// TODO
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
		glfwGetFramebufferSize( PlatformData.pWindow, &iWidth, &iHeight );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif

		glViewport( 0, 0, iWidth, iHeight );
		glClearColor( vClearColor.x, vClearColor.y, vClearColor.z, vClearColor.w );
		glClear( GL_COLOR_BUFFER_BIT );

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(vClearColor.x * vClearColor.w * 255.0f), (int)(vClearColor.y * vClearColor.w * 255.0f), (int)(vClearColor.z * vClearColor.w * 255.0f), (int)(vClearColor.w * 255.0f));
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		float const pClearColorWithAlpha[4] = { vClearColor.x * vClearColor.w, vClearColor.y * vClearColor.w, vClearColor.z * vClearColor.w, vClearColor.w };
		g_pd3dDevice->OMSetRenderTargets( 1, &PlatformData.pMainRenderTargetView, nullptr );
		g_pd3dDevice->ClearRenderTargetView( PlatformData.pMainRenderTargetView, pClearColorWithAlpha );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
		float const pClearColorWithAlpha[4] = { vClearColor.x * vClearColor.w, vClearColor.y * vClearColor.w, vClearColor.z * vClearColor.w, vClearColor.w };
		g_pd3dDeviceContext->OMSetRenderTargets( 1, &PlatformData.pMainRenderTargetView, nullptr );
		g_pd3dDeviceContext->ClearRenderTargetView( PlatformData.pMainRenderTargetView, pClearColorWithAlpha );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

		return true;
	}

	bool ImGfxAPIRender()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
		ImGui::Render();
		ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
		if ( PlatformData.pD3DDevice->BeginScene() >= 0 )
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData() );
			PlatformData.pD3DDevice->EndScene();
		}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ImGui::Render();
		ImGui_ImplDX10_RenderDrawData( ImGui::GetDrawData() );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData( ImGui::GetDrawData(), ... );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(), ... );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
		ImGui::Render();
		ImGui_ImplMetal_RenderDrawData( ImGui::GetDrawData(), ... );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_WGPU)
		ImGui::Render();
		ImGui_ImplWGPU_RenderDrawData( ImGui::GetDrawData(), ... );
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

		return true;
	}

	bool ImGfxAPISwapBuffer()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)

		//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		//{
		//	HGLRC currentContext = wglGetCurrentContext();
		//	ImGui::UpdatePlatformWindows();
		//	ImGui::RenderPlatformWindowsDefault();
		//	wglMakeCurrent(devContext, currentContext);
		//}

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		SwapBuffers( PlatformData.pDevContext );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)

		ImGuiIO& io = ImGui::GetIO();
		if ( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			GLFWwindow* pBackupCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent( pBackupCurrentContext );
		}

		glfwSwapBuffers( PlatformData.pWindow );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)

#if IMGUI_HAS_VIEWPORT
		if ( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
#endif

		HRESULT uResult = g_pd3dDevice->Present( nullptr, nullptr, nullptr, nullptr );

		// Handle loss of D3D9 device
		if ( uResult == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET )
			ImResetDevice();

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)

		//g_pSwapChain->Present(1, 0); // Present with vsync
		PlatformData.pSwapChain->Present( 0, 0 ); // Present without vsync

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)

		//g_pSwapChain->Present(1, 0); // Present with vsync
		PlatformData.pSwapChain->Present( 0, 0 ); // Present without vsync

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_WGPU)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

		return true;
	}

	void ImShutdownGfxAPI()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
		ImGui_ImplOpenGL2_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
		ImGui_ImplOpenGL3_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
		ImGui_ImplDX9_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ImGui_ImplDX10_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
		ImGui_ImplDX11_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
		ImGui_ImplDX12_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
		ImGui_ImplVulkan_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
		ImGui_ImplMetal_Shutdown();
#else
#error IM_CURRENT_TARGET not specified correctly
#endif
	}

	void ImShutdownPostGfxAPI()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ImCleanupDeviceD3D();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif
	}

	void ImShutdownWindow()
	{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		ImGui_ImplWin32_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
		ImGui_ImplGlfw_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_APPLE)
#else
#endif
	}
}
