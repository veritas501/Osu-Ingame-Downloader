#include <iostream>
#include <fstream>
#include <sstream>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include "config_helper.h"
#include "ingame_overlay.h"
#include "downloader.h"

using namespace std;
using namespace rapidjson;

// load config from file
int Config::LoadConfig() {
	ifstream ifs("Ingame.cfg");
	string result;
	Document d;

	if (!ifs.is_open()) {
		return 1;
	}
	ifs >> result;
	ifs.close();
	d.Parse(result.c_str());
	// save download type(full, no video, mini)
	if (d.HasMember("downloadType")) {
		DL::downloadType = d["downloadType"].GetInt();
	}
	// save flag dontUseDownloader
	if (d.HasMember("dontUseDownloader")) {
		DL::dontUseDownloader = d["dontUseDownloader"].GetBool();
	}
	return 0;
}

// save config to file
int Config::SaveConfig() {
	ofstream ofs("Ingame.cfg");
	StringBuffer sb;
	Writer<StringBuffer> writer(sb);
	string result;

	if (!ofs.is_open()) {
		return 1;
	}
	writer.StartObject();
	writer.Key("downloadType");
	writer.Int(DL::downloadType);
	writer.Key("dontUseDownloader");
	writer.Bool(DL::dontUseDownloader);
	writer.EndObject();
	result = sb.GetString();
	ofs << result << endl;
	ofs.close();
	return 0;
}