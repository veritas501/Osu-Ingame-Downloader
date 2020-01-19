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

// writer used by curl CURLOPT_WRITEFUNCTION
size_t stringWriter(char* data, size_t size, size_t nmemb, string* writerData) {
	if (writerData == NULL) {
		return 0;
	}
	writerData->append(data, size * nmemb);
	return size * nmemb;
}

size_t fileWriter(void* ptr, size_t size, size_t nmemb, void* stream) {
	size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
	return written;
}

//convert UTF-8 to GB2312
char* UTF8toGB2312(const char* utf8) {
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

//convert GB2312 to UTF-8
char* GB2312toUTF8(const char* gb2312) {
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}