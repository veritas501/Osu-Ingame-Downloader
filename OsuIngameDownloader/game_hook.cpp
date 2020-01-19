#include <string>
#include <ShlObj.h>
#include <MinHook.h>
#include "game_hook.h"
#include "ingame_overlay.h"
#include "downloader.h"
#include "map_db.h"
#include "utils.h"
#include "update.h"
using namespace std;

UINT HK::raw_devices_count = -1;
RAWINPUTDEVICE* HK::raw_devices = NULL;
_SwapBuffers HK::OriSwapBuffers = nullptr;
_SwapBuffers HK::BakOriSwapBuffers = nullptr;
_ShellExcuteExW HK::OriShellExecuteExW = nullptr;
_ShellExcuteExW HK::BakShellExecuteExW = nullptr;
HHOOK HK::msgHook = nullptr;
HWND HK::hwnd = NULL;

BOOL __stdcall InitPlugin(HDC hdc) {
	// hook msg for ingame overlay
	HK::hwnd = WindowFromDC(hdc);
	DWORD tid = GetCurrentThreadId();
	CreateThread(NULL, NULL, MsgHookThread, &tid, 0, NULL);
	// init overlay
	OV::InitOverlay(hdc);
	// init sid database;
	HANDLE InitDatabaseThread = reinterpret_cast<HANDLE>(_beginthreadex(0, 0,
		[](void* pData) -> unsigned int {
			DB::InitDataBase("Songs");
			return 0;
		}, NULL, 0, NULL));
	if (InitDatabaseThread) {
		CloseHandle(InitDatabaseThread);
	}
	// check plugin update
	HANDLE UpdateThread = reinterpret_cast<HANDLE>(_beginthreadex(0, 0,
		[](void* pData) -> unsigned int {
			Update::CheckUpdateService();
			return 0;
		}, NULL, 0, NULL));
	if (UpdateThread) {
		CloseHandle(UpdateThread);
	}
	// rehook swapbuffer
	HK::ReHookSwapBuffers();
	logger::WriteLog("[+] Init Plugin Done");
	return HK::OriSwapBuffers(hdc);
}

BOOL __stdcall DetourSwapBuffers(HDC hdc) {
	OV::RenderOverlay(hdc);
	return HK::OriSwapBuffers(hdc);
}

BOOL CallOriShellExecuteExW(const char* lpFile) {
	LPSHELLEXECUTEINFOW pExecinfo = new _SHELLEXECUTEINFOW;
	ZeroMemory(pExecinfo, sizeof(_SHELLEXECUTEINFOW));
	pExecinfo->cbSize = sizeof(_SHELLEXECUTEINFOW);
	pExecinfo->lpFile = char2wchar(lpFile);
	pExecinfo->nShow = SW_SHOWNORMAL;
	pExecinfo->fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS; // same with osu
	auto res = HK::OriShellExecuteExW(pExecinfo);
	delete pExecinfo->lpFile;
	delete pExecinfo;
	return res;
}

// thread that perform download
DWORD WINAPI DownloadThread(LPVOID lpParam) {
	char* url = (char*)lpParam;
	int res;
	char tmpPath[MAX_PATH];
	UINT64 sid = 0;
	string songName = "";
	string fileName = "";
	int category = 0;
	// already in process, skip
	DL::SetTaskReadLock();
	if (DL::tasks.count(url) > 0) {
		DL::UnsetTaskLock();
		logger::WriteLogFormat("[*] Reject duplicated request: %s", url);
		return 0;
	}
	DL::UnsetTaskLock();
	OV::ShowStatus();
	DL::SetTaskWriteLock();
	DL::tasks[url].dlStatus = PARSE;
	DL::tasks[url].songName = url;
	DL::UnsetTaskLock();
	// parse sid, song name and category
	res = DL::ParseInfo(url, sid, songName, category);
	if (res) {
		CallOriShellExecuteExW(url);
		goto finish;
	}
	// user already has this map
	if (DB::sidExist(sid)) {
		logger::WriteLogFormat("[*] user already has sid %llu, skip", sid);
		CallOriShellExecuteExW(url);
		goto finish;
	}
	DL::SetTaskWriteLock();
	DL::tasks[url].dlStatus = DOWNLOAD;
	DL::tasks[url].songName = songName;
	DL::tasks[url].sid = sid;
	DL::tasks[url].category = (Category)category;
	DL::UnsetTaskLock();
	// download map
	GetTempPathA(MAX_PATH, tmpPath);
	fileName.append(tmpPath);
	fileName.append(to_string(sid));
	fileName.append(".osz");
	res = DL::Download(fileName, sid, url);
	if (res) {
		CallOriShellExecuteExW(url);
		goto finish;
	}
	// open map
	ShellExecuteA(0, NULL, fileName.c_str(), NULL, NULL, SW_HIDE);
	// insert map into database
	DB::insertSid(sid);
finish:
	DL::RemoveTaskInfo(url);
	DL::SetTaskReadLock();
	if (DL::tasks.empty()) {
		OV::HideStatus();
	}
	DL::UnsetTaskLock();
	delete url;
	return 0;
}

BOOL __stdcall DetourShellExecuteExW(LPSHELLEXECUTEINFOW pExecinfo) {
	int findPos1, findPos2, findPos3;
	char* lpFile;
	HANDLE hThread;
	if (DL::dontUseDownloader) {
		goto call_api;
	}
	findPos1 = wstring(pExecinfo->lpFile).find(L"osu.ppy.sh/b/");
	findPos2 = wstring(pExecinfo->lpFile).find(L"osu.ppy.sh/s/");
	findPos3 = wstring(pExecinfo->lpFile).find(L"osu.ppy.sh/beatmapsets/");
	if ((findPos1 == -1) && (findPos2 == -1) && (findPos3 == -1)) {
		goto call_api; // not the target
	}

	lpFile = wchar2char(pExecinfo->lpFile);
	hThread = CreateThread(NULL, NULL, DownloadThread, lpFile, 0, NULL);
	if (hThread) {
		CloseHandle(hThread);
	}
	else {
		goto call_api;
	}

	return true;
call_api:
	return HK::OriShellExecuteExW(pExecinfo);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode != HC_ACTION)	{
		return CallNextHookEx(HK::msgHook, nCode, wParam, lParam);
	}
	MSG* msg = (MSG*)lParam;
	//The message has been removed from the queue.
	if (wParam == PM_REMOVE) {
		// hotkey Alt+M press
		if (msg->message == WM_SYSKEYDOWN) {
			int contextCode = msg->lParam >> 29 & 1;
			int transitionState = msg->lParam >> 31 & 1;
			if (!transitionState && contextCode && (msg->wParam == 'M')) {
				OV::ReverseShowSettings();
			}
		}
		if (OV::isShowingSettings()) {
			ImGui_ImplWin32_WndProcHandler(msg->hwnd, msg->message, msg->wParam, msg->lParam);
		}
	}
	// Block keyboard and mouse message to osu while setting is showing.
	if (OV::isShowingSettings()) {
		if (msg->message == WM_CHAR) {
			msg->message = WM_NULL;
			return 1;
		}
		if ((WM_MOUSEFIRST <= msg->message && msg->message <= WM_MOUSELAST) || (msg->message == WM_NCHITTEST) || (msg->message == WM_SETCURSOR)) {
			msg->message = WM_NULL;
			return 1;
		}
	}
	return CallNextHookEx(HK::msgHook, nCode, wParam, lParam);
}

DWORD WINAPI MsgHookThread(LPVOID lpParam) {
	DWORD Tid = *(DWORD*)lpParam;
	HK::msgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, GetModuleHandle(NULL), Tid);
	if (!HK::msgHook) {
		logger::WriteLog("[-] Set message hook failed");
		return 1;
	}
	else {
		logger::WriteLog("[+] Set message hook success");
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
}

// backup/disable/restore rawinput
int HK::BackupRawInputDevices() {
	raw_devices_count = -1;
	GetRegisteredRawInputDevices(NULL, &raw_devices_count, sizeof(RAWINPUTDEVICE));
	if (raw_devices_count == -1) {
		logger::WriteLog("[-] Can't get registered raw input devices");
		return 1;
	}
	logger::WriteLogFormat("[*] Find %d raw input", raw_devices_count);
	if (!raw_devices_count) {
		return 0;
	}
	if (raw_devices) {
		delete raw_devices;
	}
	raw_devices = new RAWINPUTDEVICE[raw_devices_count];
	if (GetRegisteredRawInputDevices(raw_devices, &raw_devices_count, sizeof(RAWINPUTDEVICE)) == -1) {
		logger::WriteLog("[-] Backup registered raw input devices fail");
		return 2;
	}
	return 0;
}

int HK::DisablRawInputDevices() {
	BackupRawInputDevices();
	if (raw_devices_count == -1) {
		return 1;
	}
	if (!raw_devices_count) {
		return 0;
	}
	logger::WriteLogFormat("[*] Try to disable %d raw input devices", raw_devices_count);
	for (UINT i = 0; i < raw_devices_count; i++) {
		RAWINPUTDEVICE p;
		memcpy(&p, raw_devices + i, sizeof(RAWINPUTDEVICE));
		p.dwFlags = RIDEV_REMOVE;
		p.hwndTarget = NULL;
		if (RegisterRawInputDevices(&p, 1, sizeof(RAWINPUTDEVICE)) == FALSE) {
			logger::WriteLogFormat("[-] Unregister index %d raw input device fail", i);
			logger::WriteLogFormat("[*] info: %p, %p, %p, %p", p.hwndTarget, p.dwFlags, p.usUsage, p.usUsagePage);
		}
	}
	UINT now_count = -1;
	GetRegisteredRawInputDevices(NULL, &now_count, sizeof(RAWINPUTDEVICE));
	if (now_count == -1) {
		logger::WriteLog("[-] Can't get registered raw input devices");
		return 2;
	}
	if (now_count) {
		logger::WriteLogFormat("[*] now has %d raw input", now_count);
		return 3;
	}
	return 0;
}

int HK::RestoreRawInputDevices() {
	if (raw_devices_count == -1) {
		return 1;
	}
	if (!raw_devices_count) {
		return 0;
	}
	logger::WriteLogFormat("[*] Try to enable %d raw input devices", raw_devices_count);
	if (RegisterRawInputDevices(raw_devices, raw_devices_count, sizeof(RAWINPUTDEVICE)) == FALSE) {
		logger::WriteLogFormat("[-] Restore %d raw input device fail",raw_devices_count);
	}
	UINT now_count = -1;
	GetRegisteredRawInputDevices(NULL, &now_count, sizeof(RAWINPUTDEVICE));
	if (now_count == -1) {
		logger::WriteLog("[-] Can't get registered raw input devices");
		return 2;
	}
	if (now_count) {
		logger::WriteLogFormat("[*] now has %d raw input", now_count);
		return 3;
	}
	return 0;
}

int HK::ReHookSwapBuffers() {
	if (MH_RemoveHook(BakOriSwapBuffers) != MH_OK) {
		logger::WriteLog("[-] ReHook£ºRemove old hook fail");
		return 1;
	}
	if (MH_CreateHookApiEx(L"gdi32", "SwapBuffers", DetourSwapBuffers, (LPVOID*)&OriSwapBuffers, (LPVOID*)&BakOriSwapBuffers) != MH_OK) {
		logger::WriteLog("[-] ReHook£ºRnstall new hook fail");
		return 2;
	}
	if (MH_CreateHookApiEx(L"shell32", "ShellExecuteExW", DetourShellExecuteExW, (LPVOID*)&OriShellExecuteExW, (LPVOID*)&BakShellExecuteExW) != MH_OK) {
		logger::WriteLog("[-] ReHook£ºRnstall new hook fail");
		return 2;
	}
	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
		logger::WriteLog("[-] ReHook£ºEnable new hook fail");
		return 3;
	}
	logger::WriteLog("[+] Rehook gdi32.SwapBuffers success");
	return 0;
}

int HK::InitHook() {
	if (MH_Initialize() != MH_OK) {
		logger::WriteLog("[-] MinHook£ºInitialize fail");
		return 1;
	}
	if (MH_CreateHookApiEx(L"gdi32", "SwapBuffers", InitPlugin, (LPVOID*)&OriSwapBuffers, (LPVOID*)&BakOriSwapBuffers) != MH_OK) {
		logger::WriteLog("[-] MinHook£ºCan't hook gdi32.SwapBuffers");
		return 1;
	}
	if (MH_EnableHook(BakOriSwapBuffers) != MH_OK) {
		logger::WriteLog("[-] MinHook£ºEnable hook fail");
		return 1;
	}
	logger::WriteLog("[+] Install hook success");
	return 0;
}

int HK::UninitHook() {
	if (HK::msgHook) {
		UnhookWindowsHookEx(HK::msgHook);
	}
	if (MH_Uninitialize() != MH_OK) {
		logger::WriteLog("[-] MinHook uninitialize fail");
		return 1;
	}
	return 0;
}