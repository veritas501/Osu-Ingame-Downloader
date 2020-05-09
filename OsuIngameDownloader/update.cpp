#include "update.h"
#include "downloader.h"
#include "ingame_overlay.h"
#include "logger.h"
#include <io.h>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <zlib/zlib.h>

using namespace rapidjson;

void Update::CheckUpdateService() {
	string githubReleaseUrl;
	string cnblogMirrorUrl;
	bool needUpdate = false;
	int res;
	needUpdate = Update::CheckUpdate(cnblogMirrorUrl, githubReleaseUrl);
	if (needUpdate) {
		// 1. try to download from cnblog mirror
		res = Update::DownloadUpdate(cnblogMirrorUrl);
		if (res) {
			// 2. download update from github page
			res = Update::DownloadUpdate(githubReleaseUrl);
			if (res) {
				MessageBoxA(0, "Ingame Downloader更新失败! 请查看日志InGameLog.txt手动下载链接", "Error", 0);
			}
			else {
				MessageBoxA(0, "Ingame Downloader更新成功! 请重启Osu!以完成更新", "Success", 0);
			}
		}
		else {
			MessageBoxA(0, "Ingame Downloader更新成功! 请重启Osu!以完成更新", "Success", 0);
		}
	}
}

bool Update::CheckUpdate(string& giteeUrl, string& githubReleaseUrl) {
	string content;
	Document jContent;
	string tagName;

	logger::WriteLogFormat("[*] CheckUpdate: checking update");
	auto res = DL::CurlGetReq("https://api.github.com/repos/veritas501/Osu-Ingame-Downloader/releases/latest", content);
	if (res) {
		logger::WriteLogFormat("[-] CheckUpdate: can't get json content, %s", curl_easy_strerror(res));
		return false;
	}

	jContent.Parse(content.c_str());
	if (jContent.HasParseError()) {
		logger::WriteLogFormat("[-] CheckUpdate: unknown parsing error");
		return false;
	}
	if (!jContent.HasMember("tag_name")) {
		logger::WriteLogFormat("[-] CheckUpdate: Wrong json format: doesn't contain member 'tag_name'");
		return false;
	}
	tagName = jContent["tag_name"].GetString();

	if (NeedUpdate(VERSION, tagName)) {
		logger::WriteLogFormat("[*] CheckUpdate: Find new version %s", tagName.c_str());
		if (!jContent.HasMember("assets")) {
			logger::WriteLogFormat("[-] CheckUpdate: Wrong json format: doesn't contain member 'assets'");
			return false;
		}
		for (auto& assets : jContent["assets"].GetArray()) {
			if (!assets.HasMember("name")) {
				logger::WriteLogFormat("[-] CheckUpdate: Wrong json format: doesn't contain member 'name'");
				return false;
			}
			if (!strcmp(assets["name"].GetString(), "IngameDL.dll")) {
				if (!assets.HasMember("browser_download_url")) {
					logger::WriteLogFormat("[-] CheckUpdate: Wrong json format: doesn't contain member 'browser_download_url'");
					return false;
				}
				githubReleaseUrl = assets["browser_download_url"].GetString();
				giteeUrl = string("https://gitee.com/hxzene/Osu-Ingame-Downloader-build/raw/master/").append(tagName).append("/IngameDL.dll");
				return true;
			}
		}
		logger::WriteLogFormat("[-] CheckUpdate: Can't find target assets 'IngameDL.dll'");
		return false;
	}
	else {
		logger::WriteLogFormat("[*] CheckUpdate: No need to update");
	}

	return false;
}

bool Update::NeedUpdate(string localVersion, string remoteVersion) {
	vector<int> vecLocalVersion, vecRemoteVersion;
	string tmpLocalVersion = "", tmpRemoteVersion = "";

	// strip
	for (auto ch : localVersion) {
		if (isdigit(ch) || ch == '.') {
			tmpLocalVersion.push_back(ch);
		}
	}
	for (auto ch : remoteVersion) {
		if (isdigit(ch) || ch == '.') {
			tmpRemoteVersion.push_back(ch);
		}
	}

	// split
	VersionStringtoArray(tmpLocalVersion, vecLocalVersion);
	VersionStringtoArray(tmpRemoteVersion, vecRemoteVersion);

	// compare
	for (size_t i = 0; i < min(vecLocalVersion.size(), vecRemoteVersion.size()); i++) {
		if (vecLocalVersion[i] < vecRemoteVersion[i]) {
			return true;
		}
		if (vecLocalVersion[i] > vecRemoteVersion[i]) {
			return false;
		}
	}
	if (vecLocalVersion.size() < vecRemoteVersion.size()) {
		return true;
	}
	return false;
}

void Update::VersionStringtoArray(string version, vector<int>& vecVersion) {
	string::size_type pos1 = 0, pos2 = 0;
	pos1 = 0;
	pos2 = version.find('.');
	while (string::npos != pos2)
	{
		vecVersion.push_back(atoi(version.substr(pos1, pos2 - pos1).c_str()));
		pos1 = pos2 + 1;
		pos2 = version.find('.', pos1);
	}
	if (pos1 != version.length()) {
		vecVersion.push_back(atoi(version.substr(pos1).c_str()));
	}
}

int Update::DownloadUpdate(string url) {
	string taskKey = "Plugin Update";
	string targetFile = "IngameDL.new.dll.tmp";

	if (!_access(targetFile.c_str(), 0)) {
		DeleteFileA(targetFile.c_str());
	}

	OV::ShowStatus();
	DL::SetTaskWriteLock();
	DL::tasks[taskKey].dlStatus = DOWNLOAD;
	DL::tasks[taskKey].songName = taskKey;
	DL::tasks[taskKey].sid = 0;
	DL::tasks[taskKey].category = UNKNOWN;
	DL::UnsetTaskLock();

	MyProgress* myp = new MyProgress();
	myp->taskKey = taskKey;
	logger::WriteLogFormat("[*] DownloadUpdate: %s", url.c_str());
	auto res = DL::CurlDownload(url, targetFile, myp);
	if (res) {
		if (!_access(targetFile.c_str(), 0)) {
			DeleteFileA(targetFile.c_str());
		}
		logger::WriteLogFormat("[-] DownloadUpdate: Can't download update, error: %s", curl_easy_strerror(res));
	}
	else {
		if (rename(targetFile.c_str(), "IngameDL.new.dll")) {
			logger::WriteLogFormat("[-] DownloadUpdate: Can't rename IngameDL.new.dll.tmp to IngameDL.new.dll");
		}
	}
	DL::RemoveTaskInfo(taskKey);
	DL::SetTaskReadLock();
	if (DL::tasks.empty()) {
		OV::HideStatus();
	}
	DL::UnsetTaskLock();
	return res;
}