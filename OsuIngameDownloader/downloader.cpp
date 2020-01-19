#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <time.h>
#include "Downloader.h"
#include "logger.h"
#include "utils.h"
#include "osu_auth.h"
using namespace rapidjson;

const char* DL::DlTypeName[3] = { "Full Version", "No Video", "Mini" };
LK DL::taskLock;
bool DL::dontUseDownloader = false;
bool DL::downloadFromCDN = false;
int DL::sayobotDownloadType = NOVIDEO;
int DL::ppyDownloadType = NOVIDEO;
int DL::serverId = SAYOBOT;
map<string, DlInfo> DL::tasks;
int DL::manualDlType = 0;
char DL::manualDlId[0x10] = "";
bool DL::useProxy = false;
char DL::proxyServer[0x50] = "";

// xferinfo callback function
// write out real time information to struct
int xferinfoCB(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	MyProgress* myp = (struct MyProgress*)clientp;
	DL::SetTaskReadLock();
	if (DL::tasks.count(myp->taskKey) < 0) {
		DL::UnsetTaskLock();
		return 1;
	}
	DL::UnsetTaskLock();
	DL::SetTaskWriteLock();
	DL::tasks[myp->taskKey].fileSize = (double)dltotal;
	DL::tasks[myp->taskKey].downloaded = (double)dlnow;
	DL::tasks[myp->taskKey].percent = (float)((double)dlnow / (double)dltotal);
	DL::UnsetTaskLock();
	return 0;
}

// GET requests, return string
CURLcode DL::CurlGetReq(const string url, string& response, const string cookie) {
	CURL* curl = curl_easy_init();
	CURLcode res = CURL_LAST;
	if (curl) {
		struct curl_slist* header_list = NULL;
		header_list = curl_slist_append(header_list, "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko");
		if (!cookie.empty()) {
			string tmpCookie = "cookie: " + cookie;
			header_list = curl_slist_append(header_list, tmpCookie.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stringWriter);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 8000);
		if(useProxy){
			curl_easy_setopt(curl, CURLOPT_PROXY, proxyServer);
			curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
		}
		res = curl_easy_perform(curl);
		if (res == CURLE_OK) {
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			if (response_code != 200) {
				logger::WriteLogFormat("[-] CurlDownload: response_code %ld", response_code);
				res = CURL_LAST;
			}
		}
	}
	curl_easy_cleanup(curl);
	return res;
}

// GET requests, write output to file
CURLcode DL::CurlDownload(const string url, const string fileName, MyProgress* prog, const string cookie) {
	CURL* curl = curl_easy_init();
	CURLcode res = CURL_LAST;
	FILE* fp;
	if (curl) {
		struct curl_slist* header_list = NULL;
		header_list = curl_slist_append(header_list, "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko");
		if (!cookie.empty()) {
			string tmpCookie = "cookie: "+ cookie;
			header_list = curl_slist_append(header_list, tmpCookie.c_str());
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fileWriter);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfoCB);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, prog);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 8000);
		if (useProxy) {
			curl_easy_setopt(curl, CURLOPT_PROXY, proxyServer);
			curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
		}
		auto err = fopen_s(&fp, fileName.c_str(), "wb");
		if (fp) {
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			res = curl_easy_perform(curl);
			fclose(fp);
		}
		if (res == CURLE_OK) {
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			if (response_code != 200) {
				logger::WriteLogFormat("[-] CurlDownload: response_code %ld", response_code);
				res = CURL_LAST;
			}
		}
	}
	curl_easy_cleanup(curl);
	return res;
}

// use sayobot api to parse sid,song name,category by osu beatmap url
int DL::SayobotParseInfo(string url, UINT64& sid, string& songName, int& category) {
	string parseApiUrl = "https://api.sayobot.cn/v2/beatmapinfo?0=" + url;
	string content;
	Document jContent;
	int status;

	auto res = CurlGetReq(parseApiUrl, content);
	if (res) {
		logger::WriteLogFormat("[-] SayobotParseInfo: can't get json content, %s", curl_easy_strerror(res));
		return 1;
	}

	string utf8Content = GB2312toUTF8(content.c_str());
	jContent.Parse(utf8Content.c_str());
	if (jContent.HasParseError()) {
		logger::WriteLogFormat("[-] SayobotParseInfo: unknown parsing error");
		return 6;
	}
	if (!jContent.HasMember("status")) {
		logger::WriteLogFormat("[-] SayobotParseInfo: Wrong json format: doesn't contain member 'status'");
		return 2;
	}
	status = jContent["status"].GetInt();
	if (status) {
		logger::WriteLogFormat("[-] SayobotParseInfo: Sayobot err-code: %d", status);
		return 3;
	}
	if (!jContent.HasMember("data")) {
		logger::WriteLogFormat("[-] SayobotParseInfo: Wrong json format: doesn't contain member 'data'");
		return 4;
	}
	if (!jContent["data"].HasMember("sid") || !jContent["data"].HasMember("title") || !jContent["data"].HasMember("approved")) {
		logger::WriteLogFormat("[-] SayobotParseInfo: Wrong json format: doesn't contain member 'sid' or 'title' or 'approved'");
		return 5;
	}
	sid = jContent["data"]["sid"].GetUint64();
	songName = jContent["data"]["title"].GetString();
	category = jContent["data"]["approved"].GetInt();
	return 0;
}

// use official server api to parse sid,song name,category by osu beatmap url
int DL::OfficialParseInfo(string url, UINT64& sid, string& songName, int& category) {
	string content;
	Document jContent;
	size_t offStart = 0;
	size_t offEnd = 0;

	auto res = CurlGetReq(url, content);
	if (res) {
		logger::WriteLogFormat("[-] OfficialParseInfo: can't get page content, %s", curl_easy_strerror(res));
		return 1;
	}
	offStart = content.find("json-beatmapset");
	if (offStart == -1) {
		logger::WriteLogFormat("[-] OfficialParseInfo: can't get json content, %s", curl_easy_strerror(res));
		return 2;
	}
	offStart = content.find("{", offStart);
	if (offStart == -1) {
		logger::WriteLogFormat("[-] OfficialParseInfo: can't get json content, %s", curl_easy_strerror(res));
		return 2;
	}
	offEnd = content.find("</script>", offStart);
	if (offEnd == -1) {
		logger::WriteLogFormat("[-] OfficialParseInfo: can't get json content, %s", curl_easy_strerror(res));
		return 2;
	}
	offEnd = content.rfind("}", offEnd);
	if (offEnd == -1) {
		logger::WriteLogFormat("[-] OfficialParseInfo: can't get json content, %s", curl_easy_strerror(res));
		return 2;
	}
	offEnd += 1;
	content = content.substr(offStart, offEnd - offStart);
	jContent.Parse(content.c_str());
	if (jContent.HasParseError()) {
		logger::WriteLogFormat("[-] OfficialParseInfo: unknown parsing error");
		return 3;
	}
	if (!jContent.HasMember("id") || !jContent.HasMember("title") || !jContent.HasMember("ranked")) {
		logger::WriteLogFormat("[-] OfficialParseInfo: Wrong json format: doesn't contain member 'id' or 'title' or 'ranked'");
		return 4;
	}
	sid = jContent["id"].GetUint64();
	songName = jContent["title"].GetString();
	category = jContent["ranked"].GetInt();
	return 0;
}

int DL::ParseInfo(string url, UINT64& sid, string& songName, int& category) {
	logger::WriteLogFormat("[*] parsing %s", url.c_str());
	if (serverId == SAYOBOT) {
		return SayobotParseInfo(url,sid,songName,category);
	}
	else if (serverId == OFFICIAL) {
		return OfficialParseInfo(url, sid, songName, category);
	}
	return 1;
}

// download beatmap from sayobot server to file by using sid
int DL::SayobotDownload(string fileName, UINT64 sid, string taskKey) {
	string downloadApiUrl;
	string server;
	if (downloadFromCDN) {
		server = "CDN";
	}
	else {
		server = "0";
	}
	switch (DL::sayobotDownloadType) {
	case FULL:
		downloadApiUrl = "https://txy1.sayobot.cn/beatmaps/download/full/" + to_string(sid) + "?server=" + server;
		break;
	case MINI:
		downloadApiUrl = "https://txy1.sayobot.cn/beatmaps/download/mini/" + to_string(sid) + "?server=" + server;
		break;
	case NOVIDEO:
	default:
		downloadApiUrl = "https://txy1.sayobot.cn/beatmaps/download/novideo/" + to_string(sid) + "?server=" + server;
		break;
	}
	MyProgress* myp = new MyProgress();
	myp->taskKey = taskKey;
	auto res = DL::CurlDownload(downloadApiUrl, fileName, myp);
	if (res) {
		logger::WriteLogFormat("[-] SayobotDownload: err while downloading, %s", curl_easy_strerror(res));
		delete myp;
		return 2;
	}
	delete myp;
	return 0;
}

// download beatmap from official server to file by using sid
int DL::OfficialDownload(string fileName, UINT64 sid, string taskKey) {
	string downloadApiUrl;
	switch (ppyDownloadType) {
	case FULL:
		downloadApiUrl = "https://osu.ppy.sh/beatmapsets/" + to_string(sid) + "/download";
		break;
	case NOVIDEO:
	default:
		downloadApiUrl = "https://osu.ppy.sh/beatmapsets/" + to_string(sid) + "/download?noVideo=1";
		break;
	}
	//check cookie
	if (!strlen(osuAuth::cookie)) {
		MessageBoxA(0, "To download from osu.ppy.sh, \nyou need login in download settings.", "Ingame downloader", 0);
		return 1;
	}
	MyProgress* myp = new MyProgress();
	myp->taskKey = taskKey;
	auto res = DL::CurlDownload(downloadApiUrl, fileName, myp, osuAuth::cookie);
	if (res == CURL_LAST) {
		printf("[-] OfficialDownload: Cookie expired");
		MessageBoxA(0, "Cookie expired, please login again or update your cookie.", "Ingame downloader", 0);
		delete myp;
		return 2;
	}
	else if (res) {
		printf("[-] OfficialDownload: err while downloading, %s", curl_easy_strerror(res));
		delete myp;
		return 3;
	}
	delete myp;
	return 0;
}

int DL::Download(string fileName, UINT64 sid, string taskKey) {
	logger::WriteLogFormat("[*] downloading sid %llu", sid);
	if (serverId == SAYOBOT) {
		return SayobotDownload(fileName, sid, taskKey);
	}
	else if (serverId == OFFICIAL) {
		return OfficialDownload(fileName, sid, taskKey);
	}
	return 1;
}

int DL::ManualDownload(string id, int idType) {
	if (id.empty()) {
		return 0;
	}
	string url = idType == 0 ? "https://osu.ppy.sh/s/" + id : "https://osu.ppy.sh/b/" + id;
	LPCWSTR w_url = char2wchar(url.c_str());
	ShellExecute(0, 0, w_url, 0, 0, SW_HIDE);
	delete w_url;
	return 0;
}

int DL::RemoveTaskInfo(string url) {
	SetTaskWriteLock();
	auto keyIter = tasks.find(url);
	if (keyIter != tasks.end()) {
		tasks.erase(keyIter);
	}
	UnsetTaskLock();
	return 0;
}

void DL::SetTaskReadLock() {
	taskLock.ReadLock();
}

void DL::SetTaskWriteLock() {
	taskLock.WriteLock();
}

void DL::UnsetTaskLock() {
	taskLock.Unlock();
}