#include "map_db.h"
#include "logger.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <regex>
#include <vector>
#include <io.h>

set<UINT64> DB::sidDB;
set<UINT64> DB::bidDB;
LK DB::databaseLock;

unsigned int UnpackULEB128(ifstream& fs) {
	char flag = 0;
	fsRead(fs, flag);
	if (flag != 0xb) {
		return 0;
	}
	unsigned char tmpByte = 0;
	unsigned int decodeNum = 0;
	int i = 0;
	do {
		fsRead(fs, tmpByte);
		decodeNum += (tmpByte & 0x7f) << (7 * i);
		i++;
	} while (tmpByte >= 0x80);
	return decodeNum;
}

string UnpackOsuStr(ifstream& fs) {
	int size = UnpackULEB128(fs);
	if (!size) {
		return string("");
	}
	char* tmpStr = new char[size + 1];
	fs.read(tmpStr, size);
	tmpStr[size] = 0;

	string ans(tmpStr);
	delete[] tmpStr;
	return ans;
}

void PassOsuStr(ifstream& fs) {
	int size = UnpackULEB128(fs);
	if (!size) {
		return;
	}
	fsPass(fs, size);
}

void ParseOsuDB(string dbName) {
	ifstream ifs;
	int bid, sid;

	ifs.open(dbName, ios::binary | ios::in);
	fsPass(ifs, 0x11);
	PassOsuStr(ifs);
	int sumBeatmaps;
	fsRead(ifs, sumBeatmaps);
	for (int i = 0; i < sumBeatmaps; i++) {
		for (int j = 0; j < 9; j++) {
			PassOsuStr(ifs);
		}
		fsPass(ifs, 1 + 2 * 3 + 8 + 4 * 4 + 8);
		for (int j = 0; j < 4; j++) {
			int length = 0;
			fsRead(ifs, length);
			for (int k = 0; k < length; k++) {
				fsPass(ifs, 1 + 4 + 1 + 8);
			}
		}
		fsPass(ifs, 4 * 3);
		int timingPointsLength;
		fsRead(ifs, timingPointsLength);
		for (int j = 0; j < timingPointsLength; j++) {
			fsPass(ifs, 8 * 2 + 1);
		}
		fsRead(ifs, bid);
		fsRead(ifs, sid);
		fsPass(ifs, 4 + 1 * 4 + 2 + 4 + 1);
		PassOsuStr(ifs);
		PassOsuStr(ifs);
		fsPass(ifs, 2);
		PassOsuStr(ifs);
		fsPass(ifs, 1 + 8 + 1);
		PassOsuStr(ifs);
		fsPass(ifs, 8 + 1 * 5 + 4 + 1);
		if (bid != -1 && bid != 0) {
			DB::bidDB.insert(bid);
		}
		if (sid != -1 && sid != 0) {
			DB::sidDB.insert(sid);
		}
	}
}

void DB::InitDataBase(string osuDB) {
	auto oldClock = clock();
	databaseLock.WriteLock();
	ParseOsuDB(osuDB);
	databaseLock.Unlock();
	auto newClock = clock();
	logger::WriteLogFormat("[*] init sid and bid database in %dms", newClock - oldClock);
}

bool DB::mapExistFast(string url) {
	databaseLock.ReadLock();
	vector<regex> e;
	e.push_back(regex("osu\.ppy\.sh/b/(\\d{1,})"));
	e.push_back(regex("osu\.ppy\.sh/s/(\\d{1,})"));
	e.push_back(regex("osu\.ppy\.sh/beatmapsets/(\\d{1,})"));
	smatch m;
	bool found = false;
	int sid = 0, bid = 0;
	for (int i = 0; i < 3; i++) {
		found = regex_search(url, m, e[i]);
		if (found) {
			if (i == 0) {
				bid = atoi(m.str(1).c_str());
			}
			else {
				sid = atoi(m.str(1).c_str());
			}
			break;
		}
	}
	if (bid != 0) {
		auto iter = bidDB.find(bid);
		if (iter != bidDB.end()) {
			databaseLock.Unlock();
			return true;
		}
	}
	if (sid != 0) {
		auto iter = sidDB.find(sid);
		if (iter != sidDB.end()) {
			databaseLock.Unlock();
			return true;
		}
	}
	databaseLock.Unlock();
	return false;
}

bool DB::mapExist(UINT64 sid) {
	databaseLock.ReadLock();
	auto iter = sidDB.find(sid);
	if (iter != sidDB.end()) {
		databaseLock.Unlock();
		return true;
	}
	databaseLock.Unlock();
	return false;
}


void DB::insertSid(UINT64 sid) {
	databaseLock.WriteLock();
	sidDB.insert(sid);
	databaseLock.Unlock();
}