#pragma once
#include <Windows.h>
#include <iostream>

using namespace std;

char* wchar2char(LPCWSTR wc);

LPCWSTR char2wchar(const char* c);

size_t stringWriter(char* data, size_t size, size_t nmemb, string* writerData);

size_t fileWriter(void* ptr, size_t size, size_t nmemb, void* stream);

char* UTF8toGB2312(const char* utf8);

char* GB2312toUTF8(const char* gb2312);