#include <ImPlatform.h>

// INTERNAL_MACRO
#define IM_GFX_OPENGL2		( 1u << 0u )
#define IM_GFX_OPENGL3		( 1u << 1u )
#define IM_GFX_DIRECTX9		( 1u << 2u )
#define IM_GFX_DIRECTX10	( 1u << 3u )
#define IM_GFX_DIRECTX11	( 1u << 4u )
//#define IM_GFX_DIRECTX12	( 1u << 5u )
#define IM_GFX_VULKAN		( 1u << 6u )
#define IM_GFX_METAL		( 1u << 7u )

#define IM_GFX_MASK			0x000000FFu

#define IM_PLATFORM_WIN32	( ( 1u << 0u ) << 8u )
#define IM_PLATFORM_GLFW	( ( 1u << 1u ) << 8u )
#define IM_PLATFORM_APPLE	( ( 1u << 2u ) << 8u )

#define IM_PLATFORM_MASK	0x0000FF00u

// Possible Permutation
#define IM_TARGET_WIN32_DX9		( IM_PLATFORM_WIN32 | IM_GFX_DIRECTX9 )
#define IM_TARGET_WIN32_DX10	( IM_PLATFORM_WIN32 | IM_GFX_DIRECTX10 )
#define IM_TARGET_WIN32_DX11	( IM_PLATFORM_WIN32 | IM_GFX_DIRECTX11 )

#define IM_TARGET_APPLE_METAL	( IM_PLATFORM_APPLE | IM_GFX_METAL )
#define IM_TARGET_APPLE_OPENGL2	( IM_PLATFORM_APPLE | IM_GFX_OPENGL2 )

#define IM_TARGET_GLFW_OPENGL2	( IM_PLATFORM_GLFW | IM_GFX_OPENGL2 )
#define IM_TARGET_GLFW_OPENGL2	( IM_PLATFORM_GLFW | IM_GFX_OPENGL2 )
#define IM_TARGET_GLFW_OPENGL2	( IM_PLATFORM_GLFW | IM_GFX_OPENGL2 )
#define IM_TARGET_GLFW_OPENGL2	( IM_PLATFORM_GLFW | IM_GFX_OPENGL2 )

#define IM_TARGET_GLFW_OPENGL3	( IM_PLATFORM_GLFW | IM_GFX_OPENGL3 )
#define IM_TARGET_GLFW_OPENGL3	( IM_PLATFORM_GLFW | IM_GFX_OPENGL3 )
#define IM_TARGET_GLFW_OPENGL3	( IM_PLATFORM_GLFW | IM_GFX_OPENGL3 )
#define IM_TARGET_GLFW_OPENGL3	( IM_PLATFORM_GLFW | IM_GFX_OPENGL3 )

#define IM_TARGET_GLFW_VULKAN	( IM_PLATFORM_GLFW | IM_GFX_VULKAN )
#define IM_TARGET_GLFW_VULKAN	( IM_PLATFORM_GLFW | IM_GFX_VULKAN )
#define IM_TARGET_GLFW_VULKAN	( IM_PLATFORM_GLFW | IM_GFX_VULKAN )
#define IM_TARGET_GLFW_VULKAN	( IM_PLATFORM_GLFW | IM_GFX_VULKAN )

#define IM_TARGET_GLFW_METAL	( IM_PLATFORM_GLFW | IM_GFX_METAL )
#define IM_TARGET_GLFW_METAL	( IM_PLATFORM_GLFW | IM_GFX_METAL )
#define IM_TARGET_GLFW_METAL	( IM_PLATFORM_GLFW | IM_GFX_METAL )
#define IM_TARGET_GLFW_METAL	( IM_PLATFORM_GLFW | IM_GFX_METAL )

#ifdef __DEAR_WIN__
#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
#elif defined(__DEAR_LINUX__)
#define IM_CURRENT_PLATFORM IM_PLATFORM_GLFW
#elif defined(__DEAR_MAC__)
#define IM_CURRENT_PLATFORM IM_PLATFORM_GLFW
#else
#endif

#ifdef __DEAR_GFX_DX9__
#define IM_CURRENT_GFX IM_GFX_DIRECTX9
#elif defined(__DEAR_GFX_DX10__)
#define IM_CURRENT_GFX IM_GFX_DIRECTX10
#elif defined(__DEAR_GFX_DX11__)
#define IM_CURRENT_GFX IM_GFX_DIRECTX11
#elif defined(__DEAR_GFX_DX12__)
#define IM_CURRENT_GFX IM_GFX_DIRECTX12
#elif defined(__DEAR_GFX_OGL2__)
#define IM_CURRENT_GFX IM_GFX_OPENGL2
#elif defined(__DEAR_GFX_OGL3__)
#define IM_CURRENT_GFX IM_GFX_OPENGL3
#else
#endif

#if defined(IM_CURRENT_PLATFORM) && defined(IM_CURRENT_GFX)
#define IM_CURRENT_TARGET ( IM_CURRENT_PLATFORM | IM_CURRENT_GFX )
#endif

#ifndef IM_CURRENT_TARGET
#ifdef _WIN32
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX10
#elif defined(__APPLE__)
#define IM_CURRENT_TARGET IM_TARGET_APPLE_METAL
#elif defined(UNIX)
#define IM_CURRENT_TARGET IM_TARGET_GLFW_OPENGL3
#endif
#endif

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_win32.cpp>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
#include <imgui/backends/imgui_impl_glfw.h>
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#include <imgui/backends/imgui_impl_osx.h>
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
#include <imgui/backends/imgui_impl_dx9.h>
#include <imgui/backends/imgui_impl_dx9.cpp>
#include <d3d9.h>

#pragma comment( lib, "d3d9.lib" )

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
#include <imgui/backends/imgui_impl_dx10.h>
#include <imgui/backends/imgui_impl_dx10.cpp>
#include <d3d10_1.h>
#include <d3d10.h>

#pragma comment( lib, "d3d10.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxgi.lib" )

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#include <imgui/backends/imgui_impl_dx11.h>
#include <imgui/backends/imgui_impl_dx11.cpp>
#include <d3d11.h>

#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxgi.lib" )

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#include <imgui/backends/imgui_impl_dx12.h>
#include <imgui/backends/imgui_impl_dx12.cpp>
#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment( lib, "d3d12.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxgi.lib" )

#endif

static struct
{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
	HWND		pHandle		= nullptr;
	WNDCLASSEX	oWinStruct;
	MSG			oMessage;
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
	LPDIRECT3D9				pD3D		= nullptr;
	LPDIRECT3DDEVICE9		pD3DDevice	= nullptr;
	D3DPRESENT_PARAMETERS	oD3Dpp		= {};
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
	ID3D10Device*			pD3DDevice				= nullptr;
	IDXGISwapChain*			pSwapChain				= nullptr;
	ID3D10RenderTargetView*	pMainRenderTargetView	= nullptr;
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
//#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#endif

} PlatformData;

namespace ImWidgets
{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11) //|| ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11) || ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
	void	ImCreateRenderTarget()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ID3D10Texture2D* pBackBuffer;
		PlatformData.pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		PlatformData.pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &PlatformData.pMainRenderTargetView);
		pBackBuffer->Release();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#endif
	}
#endif

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
		if ( D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, uCreateDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &oSwapDesc, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext ) != S_OK )
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
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
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

	void	ImCleanupRenderTarget()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		if ( PlatformData.pMainRenderTargetView )
		{
			PlatformData.pMainRenderTargetView->Release();
			PlatformData.pMainRenderTargetView = nullptr;
		}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#endif
	}
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
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
			if (PlatformData.pD3DDevice != NULL && wParam != SIZE_MINIMIZED)
			{
				PlatformData.oD3Dpp.BackBufferWidth = LOWORD(lParam);
				PlatformData.oD3Dpp.BackBufferHeight = HIWORD(lParam);
				ImResetDevice();
			}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)

			if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
			{
				ImCleanupRenderTarget();
				PlatformData.pSwapChain->ResizeBuffers( 0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0 );
				ImCreateRenderTarget();
			}
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif
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

		//ImGui_ImplWin32_EnableDpiAwareness();

		PlatformData.oWinStruct = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, szName, nullptr };
		::RegisterClassEx(&PlatformData.oWinStruct);

		PlatformData.pHandle = ::CreateWindow(PlatformData.oWinStruct.lpszClassName, szName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, uWidth, uHeight, nullptr, nullptr, PlatformData.oWinStruct.hInstance, nullptr);
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif
		return true;
	}

	bool ImInitGfxAPI()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
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
		::ShowWindow(PlatformData.pHandle, SW_SHOWDEFAULT);
		::UpdateWindow(PlatformData.pHandle);
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
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		ImGui_ImplWin32_Init( PlatformData.pHandle );
		ZeroMemory( &PlatformData.oMessage, sizeof( PlatformData.oMessage ) );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
		ImGui_ImplDX9_Init( PlatformData.pD3DDevice );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ImGui_ImplDX10_Init( PlatformData.pD3DDevice );
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
//#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif
	}

	void ImEnd()
	{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		ImShutdownGfxAPI();
		ImShutdownWindow();
		ImGui::DestroyContext();

		ImShutdownPostGfxAPI();

		::DestroyWindow( PlatformData.pHandle );
		::UnregisterClass( PlatformData.oWinStruct.lpszClassName, PlatformData.oWinStruct.hInstance );
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif
	}

	bool ImBeginFrame()
	{
		if (ImWidgets::ImPlatformEvents())
			return false;

		ImWidgets::ImNewFrame();

		return true;
	}

	void ImEndFrame( ImVec4 const vClearColor )
	{
		ImWidgets::ImGfxAPIClear( vClearColor );
		ImWidgets::ImGfxAPIRender();
		ImWidgets::ImGfxAPISwapBuffer();
	}

	//////////////////////////////////////////////////////////////////////////

	bool ImPlatformContinue()
	{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
		return PlatformData.oMessage.message != WM_QUIT;
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
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
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif
	}

	bool ImGfxAPINewFrame()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
		ImGui_ImplDX9_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ImGui_ImplDX10_NewFrame();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
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
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#endif

		return true;
	}

	bool ImGfxAPIClear( ImVec4 const vClearColor )
	{
		ImGui::EndFrame();

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
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
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
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
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

		return true;
	}

	bool ImGfxAPISwapBuffer()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)

#if IMGUI_HAS_VIEWPORT
		if ( ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
#endif

		HRESULT uResult = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		// Handle loss of D3D9 device
		if ( uResult == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET )
			ImResetDevice();

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)

		//g_pSwapChain->Present(1, 0); // Present with vsync
		PlatformData.pSwapChain->Present(0, 0); // Present without vsync

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

		return true;
	}

	void ImShutdownGfxAPI()
	{
#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
		ImGui_ImplDX9_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
		ImGui_ImplDX10_Shutdown();
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_VULKAN)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK)) == IM_GFX_METAL)
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
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_APPLE)
#else
#endif
	}
}
