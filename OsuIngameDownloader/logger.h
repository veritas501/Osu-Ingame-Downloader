#pragma once
#ifndef __CLOG__
#define __CLOG__
#include <string>
#include <fstream>
#include <tchar.h>
#include "rw_lock.h"
class logger {
public:
	logger();
	~logger();
	template <class T>
	static void WriteLog(T x);
	static void WriteLogFormat(const char* format, ...);
	static std::string GetSystemTimes();
};

template <class T> void logger::WriteLog(T x) {
	std::fstream of("InGameLog.txt", std::ios::app);
	if (!of.is_open()) {
		return;
	}
	of.seekp(std::ios::end);
	of << GetSystemTimes() << ": " << x << std::endl;
	of.close();
}
#endif