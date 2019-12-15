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

// hook class
class HK {
private:
	UINT raw_devices_count = -1;
	RAWINPUTDEVICE* raw_devices = NULL;
public:
	_SwapBuffers OriSwapBuffers = nullptr;
	_SwapBuffers BakOriSwapBuffers = nullptr;
	_ShellExcuteExW OriShellExecuteExW = nullptr;
	_ShellExcuteExW BakShellExecuteExW = nullptr;
	HHOOK msgHook = nullptr;
	HWND hwnd = NULL;
	HK() {}
	~HK() {}
	static HK* inst();
	int InitHook();
	int ReHookSwapBuffers();
	int UninitHook();
	int BackupRawInputDevices();
	int DisablRawInputDevices();
	int RestoreRawInputDevices();
	int ManualDownload(string id, int idType);
};
