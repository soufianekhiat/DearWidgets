#pragma once

#include <imgui.h>

// TODO: OpenGL Loader

// INTERNAL_MACRO
#define IM_GFX_OPENGL2		( 1u << 0u )
#define IM_GFX_OPENGL3		( 1u << 1u )
#define IM_GFX_DIRECTX9		( 1u << 2u )
#define IM_GFX_DIRECTX10	( 1u << 3u )
#define IM_GFX_DIRECTX11	( 1u << 4u )
#define IM_GFX_DIRECTX12	( 1u << 5u )
#define IM_GFX_VULKAN		( 1u << 6u )
#define IM_GFX_METAL		( 1u << 7u )
#define IM_GFX_WGPU			( 1u << 8u )

#define IM_GFX_MASK			0x0000FFFFu

#define IM_PLATFORM_WIN32	( ( 1u << 0u ) << 16u )
#define IM_PLATFORM_GLFW	( ( 1u << 1u ) << 16u )
#define IM_PLATFORM_APPLE	( ( 1u << 2u ) << 16u )

#define IM_PLATFORM_MASK	0xFFFF0000u

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

#define IM_CURRENT_TARGET IM_TARGET_GLFW_OPENGL3

#ifdef __DEAR_WIN__
#define IM_CURRENT_PLATFORM IM_PLATFORM_WIN32
#elif defined(__DEAR_LINUX__)
#define IM_CURRENT_PLATFORM IM_PLATFORM_GLFW
#elif defined(__DEAR_MAC__)
#define IM_CURRENT_PLATFORM IM_PLATFORM_APPLE
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
#elif defined(__DEAR_GFX_VULKAN__)
#define IM_CURRENT_GFX IM_GFX_VULKAN
#elif defined(__DEAR_METAL__)
#define IM_CURRENT_GFX IM_GFX_METAL
#else
#endif

#ifndef IM_CURRENT_TARGET
#ifdef _WIN32
#define IM_CURRENT_TARGET IM_TARGET_WIN32_DX11
#elif defined(__APPLE__)
#define IM_CURRENT_TARGET IM_TARGET_APPLE_METAL
#elif defined(UNIX)
#define IM_CURRENT_TARGET IM_TARGET_GLFW_OPENGL3
#endif
#endif

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
#include <imgui/backends/imgui_impl_win32.h>
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
#include <d3d9.h>

#pragma comment( lib, "d3d9.lib" )

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
#include <imgui/backends/imgui_impl_dx10.h>
#include <d3d10_1.h>
#include <d3d10.h>

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
#include <imgui/backends/imgui_impl_dx11.h>
#include <d3d11.h>

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#include <imgui/backends/imgui_impl_dx12.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_VULKAN)
#include <imgui/backends/imgui_impl_vulkan.h>
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_METAL)
#include <imgui/backends/imgui_impl_metal.h>
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#include <imgui/backends/imgui_impl_opengl2.h>

#ifndef IM_OPENGL_LOADER
#define IM_OPENGL_GLAD
#endif
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)
#include <imgui/backends/imgui_impl_opengl3.h>

#ifndef IM_OPENGL_LOADER
#define IM_OPENGL_GLAD
#endif
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_WGPU)
#include <imgui/backends/imgui_impl_wgpu.h>
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

static struct
{
#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
	HWND		pHandle		= nullptr;
	WNDCLASSEX	oWinStruct;
	MSG			oMessage;
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_GLFW)
	GLFWwindow*	pWindow		=	nullptr;
#elif ((IM_CURRENT_TARGET & IM_PLATFORM_MASK)) == IM_PLATFORM_APPLE)
#else
#error IM_CURRENT_TARGET not specified correctly
#endif

#if ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL2)
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_OPENGL3)

#if ((IM_CURRENT_TARGET & IM_PLATFORM_MASK) == IM_PLATFORM_WIN32)
	HDC						pDevContext				= nullptr;
#endif

	char const*				pGLSLVersion			= "#version 130";
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX9)
	LPDIRECT3D9				pD3D					= nullptr;
	LPDIRECT3DDEVICE9		pD3DDevice				= nullptr;
	D3DPRESENT_PARAMETERS	oD3Dpp					= {};
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX10)
	ID3D10Device*			pD3DDevice				= nullptr;
	IDXGISwapChain*			pSwapChain				= nullptr;
	ID3D10RenderTargetView*	pMainRenderTargetView	= nullptr;
#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX11)
	ID3D11Device*			pD3DDevice				= nullptr;
	ID3D11DeviceContext*	pD3DDeviceContext		= nullptr;
	IDXGISwapChain*			pSwapChain				= nullptr;
	ID3D11RenderTargetView*	pMainRenderTargetView	= nullptr;
//#elif ((IM_CURRENT_TARGET & IM_GFX_MASK) == IM_GFX_DIRECTX12)
#endif
} PlatformData;

namespace ImWidgets
{
	// Functions as FrontEnd;
	bool ImInit( char const* pWindowsName, ImU32 const uWidth, ImU32 const uHeight );
	void ImBegin();
	void ImEnd();
	bool ImBeginFrame();
	void ImEndFrame( ImVec4 const vClearColor );

	// Internal-Use Only
	bool ImPlatformContinue();
	bool ImPlatformEvents();

	void ImNewFrame();

	bool ImInitWindow( char const* pWindowsName, ImU32 const uWidth, ImU32 const uHeight );
	bool ImInitGfxAPI();
	bool ImShowWindow();

	bool ImGfxAPINewFrame();
	bool ImPlatformNewFrame();

	bool ImGfxAPIClear( ImVec4 const vClearColor );

	bool ImGfxAPIRender();
	bool ImGfxAPISwapBuffer();

	void ImShutdownGfxAPI();
	void ImShutdownPostGfxAPI();
	void ImShutdownWindow();
}
