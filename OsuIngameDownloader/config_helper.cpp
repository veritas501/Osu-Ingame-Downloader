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

CH* CH::inst() {
	static CH _ch;
	return &_ch;
}

int CH::LoadConfig() {
	ifstream ifs("Ingame.cfg");
	string result;
	Document d;

	if (!ifs.is_open()) {
		return 1;
	}
	ifs >> result;
	ifs.close();
	d.Parse(result.c_str());
	if (d.HasMember("downloadType")) {
		DL::inst()->downloadType = d["downloadType"].GetInt();
	}
	if (d.HasMember("dontUseDownloader")) {
		DL::inst()->dontUseDownloader = d["dontUseDownloader"].GetBool();
	}
	return 0;
}

int CH::SaveConfig() {
	ofstream ofs("Ingame.cfg");
	StringBuffer sb;
	Writer<StringBuffer> writer(sb);
	string result;

	if (!ofs.is_open()) {
		return 1;
	}
	writer.StartObject();
	writer.Key("downloadType");
	writer.Int(DL::inst()->downloadType);
	writer.Key("dontUseDownloader");
	writer.Bool(DL::inst()->dontUseDownloader);
	writer.EndObject();
	result = sb.GetString();
	ofs << result << endl;
	ofs.close();
	return 0;
}