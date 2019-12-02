#pragma once
#include <iostream>
#include <curl/curl.h>
#include <map>
#include "rw_lock.h"
using namespace std;

enum DlType {
	FULL,
	NOVIDEO,
	MINI
};

enum DlStatus {
	NONE,
	PARSE,
	DOWNLOAD
};

struct DlInfo {
	DlStatus dlStatus = NONE;
	UINT64 sid = 0;
	string songName = "";
	double fileSize = 0;
	double downloaded = 0;
	float percent = 0;
};

struct MyProgress {
	string taskKey = "";
};

extern const char* DlTyteName[3];

class DL {
private:
	CURLcode CurlGetReq(const string url, string& response);
	CURLcode CurlDownload(const string url, const string fileName, MyProgress* prog);
	LK taskLock;
public:
	bool dontUseDownloader = false;
	int downloadType = NOVIDEO;
	map<string, DlInfo> tasks;
	DL();
	~DL();
	static DL* inst();
	int ParseInfo(string url, UINT64& sid, string& songName);
	int StartDownload(string fileName, UINT64 sid, string taskKey);
	int RemoveTaskInfo(string url);
	void SetTaskReadLock();
	void SetTaskWriteLock();
	void UnsetTaskLock();

};