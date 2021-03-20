#pragma once

#include <imgui.h>

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
