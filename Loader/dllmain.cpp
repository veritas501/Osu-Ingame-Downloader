#include "pch.h"
#include "dllhijack.h"
#include "loader.h"
#include "shlwapi.h"
#include <process.h>
#pragma comment(lib, "shlwapi")

VOID DllHijack(HMODULE hMod) {
	TCHAR tszDllName[MAX_PATH] = { 0 };
	TCHAR tszDllPath[MAX_PATH] = { 0 };
	LPWSTR tszDllNamePtr = nullptr;
	GetModuleFileName(hMod, tszDllName, MAX_PATH);
	GetModuleFileName(hMod, tszDllPath, MAX_PATH);
	tszDllNamePtr = PathFindFileName(tszDllName);
	wcscat_s(tszDllPath, L".dll");
	if (SuperDllHijack(tszDllNamePtr, tszDllPath)) {
		wsprintf(tszDllPath, L"C:\\Windows\\system32\\%s", tszDllNamePtr);
		if (SuperDllHijack(tszDllNamePtr, tszDllPath)) {
			HANDLE LoginThread = reinterpret_cast<HANDLE>(_beginthreadex(0, 0,
				[](void* pData) -> unsigned int {
					MessageBox(NULL, L"loader: Can't load original dll", L"Error", MB_ICONERROR);
					return 0;
				}, NULL, 0, NULL));
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DllHijack(hModule);
		LoaderInitialize();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		LoaderUninitialize();
		break;
	}
	return TRUE;
}