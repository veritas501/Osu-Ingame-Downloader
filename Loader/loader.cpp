#include "pch.h"
#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include "loader.h"

HMODULE hm = nullptr;

int LoaderInitialize() {
	char currentPath[MAX_PATH] = "";
	char newVersionName[MAX_PATH + 20] = "";
	char oldVersionName[MAX_PATH + 20] = "";
	bool hasNewVersion;
	bool hasOldVersion;

	if (!GetModuleFileNameA(NULL, currentPath, MAX_PATH)) {
		MessageBox(0, L"Loader: Can't get current path", L"Error", 0);
		return 0;
	}
	strrchr(currentPath, '\\')[0] = 0;
	snprintf(oldVersionName, MAX_PATH + 20, "%s\\IngameDL.dll", currentPath);
	snprintf(newVersionName, MAX_PATH + 20, "%s\\IngameDL.new.dll", currentPath);

	// update IngameDL module
	hasNewVersion = !_access(newVersionName, 0);
	hasOldVersion = !_access(oldVersionName, 0);
	if (!hasNewVersion && !hasOldVersion) {
		MessageBox(0, L"Loader: Can't find IngameDL.dll", L"Error", 0);
		return 0;
	}
	if (hasNewVersion) {
		if (hasOldVersion) {
			if (!DeleteFileA(oldVersionName)) {
				MessageBox(0, L"Loader: Can't delete old IngameDL.dll", L"Update", 0);
				return 1;
			}
			if (rename(newVersionName, oldVersionName)) {
				MessageBox(0, L"Loader: Can't rename IngameDL.new.dll to IngameDL.dll", L"Update", 0);
				return 1;
			}
		}
	}

	// module already loaded.
	if (GetModuleHandleA("IngameDL.dll")) {
		return 0;
	}
	// load main module
	hm = LoadLibraryA(oldVersionName);
	if (!hm) {
		MessageBox(0, L"Loader: Can't load IngameDL.dll", L"Error", 0);
		return 1;
	}
	return 0;
}

int LoaderUninitialize() {
	return FreeLibrary(hm);
}