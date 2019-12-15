#include "utils.h"

char* wchar2char(LPCWSTR wc) {
	char* c;
	int size;
	size = WideCharToMultiByte(CP_ACP, 0, wc, -1, NULL, 0, NULL, NULL);
	c = new char[size];
	WideCharToMultiByte(CP_ACP, 0, wc, -1, c, size, NULL, NULL);
	return c;
}

LPCWSTR char2wchar(const char* c) {
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	size_t res;
	mbstowcs_s(&res, wc, cSize, c, cSize);
	return wc;
}