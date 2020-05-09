#pragma once
#include <iostream>
#include <set>
#include <Windows.h>
#include "rw_lock.h"
using namespace std;

#define fsRead(fs,obj) fs.read((char*)&obj,sizeof(obj));
#define fsPass(fs,size) fs.seekg(size,std::ios::cur);

namespace DB {
	extern set<UINT64> sidDB;
	extern set<UINT64> bidDB;
	extern LK databaseLock;

	void InitDataBase(string osuDB);
	bool mapExistFast(string url);
	bool mapExist(UINT64 sid);
	void insertSid(UINT64 sid);
};