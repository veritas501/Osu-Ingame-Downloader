#include <iostream>
#include <fstream>
#include <sstream>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/document.h>
#include "config_helper.h"
#include "ingame_overlay.h"
#include "downloader.h"
#include "osu_auth.h"

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
	// load Sayobot download type(full, no video, mini)
	if (d.HasMember("sayobotDownloadType")) {
		DL::sayobotDownloadType = d["sayobotDownloadType"].GetInt();
		if (DL::sayobotDownloadType > 2 || DL::sayobotDownloadType < 0) {
			DL::sayobotDownloadType = 1;
		}
	}
	// load osu.ppy.sh download type(full, no video)
	if (d.HasMember("ppyDownloadType")) {
		DL::ppyDownloadType = d["ppyDownloadType"].GetInt();
		if (DL::ppyDownloadType > 1 || DL::ppyDownloadType < 0) {
			DL::ppyDownloadType = 1;
		}
	}
	// load server id (sayobot or official ppy)
	if (d.HasMember("serverId")) {
		DL::serverId = (ServerId)d["serverId"].GetInt();
		if (DL::serverId != SAYOBOT && DL::serverId != OFFICIAL) {
			DL::serverId = SAYOBOT;
		}
	}

	// load flag dontUseDownloader
	if (d.HasMember("dontUseDownloader")) {
		DL::dontUseDownloader = d["dontUseDownloader"].GetBool();
	}
	// load flag downloadFromCDN
	if (d.HasMember("downloadFromCDN")) {
		DL::downloadFromCDN = d["downloadFromCDN"].GetBool();
	}
	// load flag useProxy
	if (d.HasMember("useProxy")) {
		DL::useProxy = d["useProxy"].GetBool();
	}
	// load flag proxyServer
	if (d.HasMember("proxyServer")) {
		const char * tmpProxyServer = d["proxyServer"].GetString();
		int length = strlen(tmpProxyServer);
		if (length < sizeof(DL::proxyServer)) {
			strncpy_s(DL::proxyServer,tmpProxyServer,length);
		}
	}
	// load osu cookie
	if (d.HasMember("osuCookie")) {
		const char* tmpOsuCookie = d["osuCookie"].GetString();
		int length = strlen(tmpOsuCookie);
		if (length < sizeof(osuAuth::cookie)) {
			strncpy_s(osuAuth::cookie, tmpOsuCookie, length);
		}
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
	writer.Key("sayobotDownloadType");
	writer.Int(DL::sayobotDownloadType);
	writer.Key("ppyDownloadType");
	writer.Int(DL::ppyDownloadType);
	writer.Key("serverId");
	writer.Int(DL::serverId);
	writer.Key("dontUseDownloader");
	writer.Bool(DL::dontUseDownloader);
	writer.Key("downloadFromCDN");
	writer.Bool(DL::downloadFromCDN);
	writer.Key("useProxy");
	writer.Bool(DL::useProxy);
	writer.Key("proxyServer");
	writer.String(DL::proxyServer);
	writer.Key("osuCookie");
	writer.String(osuAuth::cookie);
	writer.EndObject();
	result = sb.GetString();
	ofs << result << endl;
	ofs.close();
	return 0;
}