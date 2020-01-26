#pragma once
#include <iostream>

using namespace std;

namespace osuAuth {
	extern char username[0x100];
	extern char password[0x100];
	extern char cookie[0x1000];

	string GetCsrfToken();
	bool login(string& errorDetail);

}