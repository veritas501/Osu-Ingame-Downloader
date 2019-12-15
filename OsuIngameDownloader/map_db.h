#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include "rw_lock.h"
using namespace std;

namespace DB {
	extern vector<UINT64> sidDatabase;
	extern LK databaseLock;

	void topDirSearch(string path);
	void findSingleMap(string path);
	int ParseMapSid(string fileName);
	string GetSuffix(string fileName);

	void InitDataBase(string songDir);
	bool sidExist(UINT64 sid);
	void insertSid(UINT64 sid);

};