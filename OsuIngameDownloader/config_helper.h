#pragma once
// config helper class
class CH {
public:
	CH() {}
	~CH() {}
	static CH* inst();
	int LoadConfig();
	int SaveConfig();
};