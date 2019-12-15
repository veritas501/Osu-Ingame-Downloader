#include "game_hook.h"
#include "logger.h"
#include "config_helper.h"

// DLL hijack header
#ifdef _VERSIONHIJACK
#include "version_hijack.h"
#endif
#ifdef _XINPUTHIJACK
#include "xinput_hijack.h"
#endif

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		logger::WriteLog("======================= Inject Success =======================");
		Config::LoadConfig();
		HK::InitHook();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Config::SaveConfig();
		HK::UninitHook();
		logger::WriteLog("===================== See you next time ======================");
		break;
	}
	return TRUE;
}