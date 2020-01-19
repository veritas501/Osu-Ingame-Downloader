#include <ctime>
#include "logger.h"
logger::logger() {}

logger::~logger() {}

void logger::WriteLogFormat(const char* format, ...) {
	va_list arglist;
	std::string strArgData;
	char szBuffer[0x1024];
	ZeroMemory(szBuffer, 0x1024);
	va_start(arglist, format);
	vsprintf_s(szBuffer, format, arglist);
	va_end(arglist);
	strArgData = szBuffer;
	std::fstream of("InGameLog.txt", std::ios::app);
	if (!of.is_open()) {
		return;
	}
	of << GetSystemTimes() << ": " << strArgData << std::endl;
	of.close();
}

std::string logger::GetSystemTimes() {
	time_t Time;
	tm t;
	CHAR strTime[MAX_PATH];
	ZeroMemory(strTime, MAX_PATH);
	time(&Time);
	localtime_s(&t, &Time);
	strftime(strTime, 100, "%m-%d %H:%M:%S ", &t);
	std::string strTimes = strTime;
	return strTimes;
}