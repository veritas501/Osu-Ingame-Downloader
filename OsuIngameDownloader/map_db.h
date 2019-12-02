#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include "rw_lock.h"
using namespace std;

class DB {
public:
	DB();
	~DB();
	static DB* inst();
	void InitDataBase(string songDir);
	bool sidExist(UINT64 sid);
	void insertSid(UINT64 sid);
private:
	vector<UINT64> sids;
	LK lock;
	void topDirSearch(string path);
	void findSingleMap(string path);
	int ParseMapSid(string fileName);
	string GetSuffix(string fileName);
};