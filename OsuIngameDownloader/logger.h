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
	//支持格式化输出多参数输出
	static void WriteLogFormat(const char* format, ...);
	static std::string GetSystemTimes();
private:
	static std::string GetFilePath();
	std::string m_LogFilePath;
	static bool IsPathExist(const std::string FilePath);
};

//支持输出int double 文本 
template <class T> void logger::WriteLog(T x) {
	std::fstream of(GetFilePath(), std::ios::app);
	if (!of.is_open()) {
		return;
	}
	of.seekp(std::ios::end);  //设置文件指针到文件尾部
	of << GetSystemTimes() << ": " << x << std::endl;
	of.close();  //关闭文件；
}
#endif