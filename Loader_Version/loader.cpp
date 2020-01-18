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
		MessageBoxA(0, "Loader: Can't get current path", "Error", 0);
		return 0;
	}
	strrchr(currentPath, '\\')[0] = 0;
	snprintf(oldVersionName, MAX_PATH + 20, "%s\\IngameDL.dll", currentPath);
	snprintf(newVersionName, MAX_PATH + 20, "%s\\IngameDL.new.dll", currentPath);

	// update IngameDL module
	hasNewVersion = !_access(newVersionName, 0);
	hasOldVersion = !_access(oldVersionName, 0);
	if (!hasNewVersion && !hasOldVersion) {
		MessageBoxA(0, "Loader: Can't find IngameDL.dll", "Error", 0);
		return 0;
	}
	if (hasNewVersion) {
		if (hasOldVersion) {
			if (!DeleteFileA(oldVersionName)) {
				MessageBoxA(0, "Loader: Can't delete old IngameDL.dll", "Update", 0);
				return 1;
			}
			if (rename(newVersionName, oldVersionName)) {
				MessageBoxA(0, "Loader: Can't rename IngameDL.new.dll to IngameDL.dll", "Update", 0);
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
		MessageBoxA(0, "Loader: Can't load IngameDL.dll", "Error", 0);
		return 1;
	}
	return 0;
}

int LoaderUninitialize() {
	return FreeLibrary(hm);
}