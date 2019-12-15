#include "map_db.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <io.h>

vector<UINT64> DB::sidDatabase;
LK DB::databaseLock;


string DB::GetSuffix(string fileName) {
	auto off = fileName.find_last_of('.');
	if (off == -1) {
		return "";
	}
	return fileName.substr(off);
}

int DB::ParseMapSid(string fileName) {
	ifstream ifs(fileName);
	if (!ifs.is_open()) {
		return 1;
	}
	ostringstream oss;
	oss << ifs.rdbuf();
	string content = oss.str();
	size_t off = content.find("BeatmapSetID:");
	if (off == -1)
	{
		return 1;
	}
	auto off2 = content.find_first_of("\n", off);
	string sidString = content.substr(off + 13, off2 - (off + 13));
	UINT64 sid = atoll(sidString.c_str());
	if (sid != -1) {
		sidDatabase.push_back(sid);
	}
	return 0;
}

void DB::findSingleMap(string path) {
	long hFile = 0;
	struct _finddata_t fileInfo;
	string pathName, exdName;

	if ((hFile = _findfirst(pathName.assign(path).
		append("\\*").c_str(), &fileInfo)) == -1) {
		return;
	}
	do {
		if (!(fileInfo.attrib & _A_SUBDIR) && GetSuffix(fileInfo.name) == ".osu") {
			if (!ParseMapSid(path + "\\" + fileInfo.name)) {
				//find, skip other map
				break;
			}

		}
	} while (_findnext(hFile, &fileInfo) == 0);
	_findclose(hFile);
	return;
}

void DB::topDirSearch(string path)
{
	long hFile = 0;
	struct _finddata_t fileInfo;
	string pathName;

	if ((hFile = _findfirst(pathName.assign(path).
		append("\\*").c_str(), &fileInfo)) == -1) {
		return;
	}
	do {
		if (fileInfo.attrib & _A_SUBDIR && strcmp(fileInfo.name, ".") && strcmp(fileInfo.name, "..")) {
			findSingleMap(path + "\\" + fileInfo.name);
		}
	} while (_findnext(hFile, &fileInfo) == 0);
	_findclose(hFile);
	return;
}

void DB::InitDataBase(string songDir) {
	auto oldClock = clock();
	databaseLock.WriteLock();
	topDirSearch(songDir);
	databaseLock.Unlock();
	auto newClock = clock();
	logger::WriteLogFormat("[*] init sid database in %dms", newClock - oldClock);
}

bool DB::sidExist(UINT64 sid) {
	databaseLock.ReadLock();
	auto iter = find(sidDatabase.begin(), sidDatabase.end(), sid);
	if (iter != sidDatabase.end()) {
		databaseLock.Unlock();
		return true;
	}
	databaseLock.Unlock();
	return false;
}

void DB::insertSid(UINT64 sid) {
	databaseLock.WriteLock();
	sidDatabase.push_back(sid);
	databaseLock.Unlock();
}