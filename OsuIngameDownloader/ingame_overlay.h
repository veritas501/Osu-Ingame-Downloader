#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_opengl3.h>
#include <GL/gl3w.h>

#define STATUS_WINDOW_NAME "InGameDownload - Status"
#define SETTING_WINDOW_NAME "InGameDownload - Settings"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// short for overlay
namespace OV {
	extern bool showStatus;
	extern bool showSetting;
	extern int statusPinned;
	
	void HelpMarker(const char* desc);
	bool isShowingSettings();
	void ReverseShowSettings();
	bool isShowingStatus();
	void ShowStatus();
	void HideStatus();
	void InitOverlay(HDC hdc);
	void RenderOverlay(HDC hdc);
}