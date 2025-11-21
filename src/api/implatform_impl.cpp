// ImPlatform Implementation for DearWidgets Library
// This file includes the ImPlatform implementation to provide the graphics backend
// functions needed by the DearWidgets API.

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
#elif defined(__DEAR_LINUX__)
	// For Linux builds, default to OpenGL3
	#define IM_CURRENT_PLATFORM IM_PLATFORM_SDL2
	#define IM_CURRENT_GFX IM_GFX_OPENGL3
#else
	#error "No graphics API defined. Expected __DEAR_GFX_* define"
#endif

// Define Implementation and include ImPlatform
// This will include the ImPlatform graphics backend implementation
#define IMPLATFORM_IMPLEMENTATION
#include <ImPlatform.h>
