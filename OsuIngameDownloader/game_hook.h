#pragma once
#include <Windows.h>
#include <shellapi.h>
#include "logger.h"

typedef BOOL(__stdcall* _SwapBuffers)(HDC);
typedef BOOL(__stdcall* _ShellExcuteExW)(LPSHELLEXECUTEINFOW);

BOOL __stdcall InitPlugin(HDC hdc);
BOOL __stdcall DetourSwapBuffers(HDC hdc);

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
DWORD WINAPI MsgHookThread(LPVOID lpParam);

// short for hook
namespace HK {
	extern UINT raw_devices_count;
	extern RAWINPUTDEVICE* raw_devices;
	extern _SwapBuffers OriSwapBuffers;
	extern _SwapBuffers BakOriSwapBuffers;
	extern _ShellExcuteExW OriShellExecuteExW;
	extern _ShellExcuteExW BakShellExecuteExW;
	extern HHOOK msgHook;
	extern HWND hwnd;
	
	int InitHook();
	int ReHookSwapBuffers();
	int UninitHook();
	int BackupRawInputDevices();
	int DisablRawInputDevices();
	int RestoreRawInputDevices();
};
