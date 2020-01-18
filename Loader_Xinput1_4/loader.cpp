#include "pch.h"
#include <Windows.h>
#include <io.h>
#include "loader.h"

HMODULE hm = nullptr;

int LoaderInitialize() {
	bool hasNewVersion;
	bool hasOldVersion;

	// update IngameDL module
	hasNewVersion = !_access("IngameDL.new.dll", 0);
	hasOldVersion = !_access("IngameDL.dll", 0);
	if (!hasNewVersion && !hasOldVersion) {
		MessageBoxA(0, "Can't find IngameDL.dll", "Error", 0);
		return 0;
	}
	if (hasNewVersion) {
		if (hasOldVersion) {
			if (!DeleteFileA("IngameDL.dll")) {
				MessageBoxA(0, "Can't delete old IngameDL.dll", "Update", 0);
				return 1;
			}
			if (rename("IngameDL.new.dll", "IngameDL.dll")) {
				MessageBoxA(0, "Can't rename IngameDL.new.dll to IngameDL.dll", "Update", 0);
				return 1;
			}
		}
	}

	// module already loaded.
	if (GetModuleHandleA("IngameDL.dll")) {
		return 0;
	}
	// load main module
	hm = LoadLibraryA("IngameDL.dll");
	if (!hm) {
		MessageBoxA(0,"Can't load IngameDL.dll","Error",0);
		return 1;
	}
	return 0;
}

int LoaderUninitialize() {
	return FreeLibrary(hm);
}