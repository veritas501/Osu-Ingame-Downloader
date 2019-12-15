#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <time.h>
#include "Downloader.h"
#include "logger.h"
using namespace rapidjson;

const char* DL::DlTypeName[3] = { "Full Version", "No Video", "Mini" };
LK DL::taskLock;
bool DL::dontUseDownloader = false;
int DL::downloadType = NOVIDEO;
map<string, DlInfo> DL::tasks;

// writer used by curl CURLOPT_WRITEFUNCTION
size_t stringWriter(char* data, size_t size, size_t nmemb, std::string* writerData) {
	if (writerData == NULL)
		return 0;

	writerData->append(data, size * nmemb);
	return size * nmemb;
}

size_t fileWriter(void* ptr, size_t size, size_t nmemb, void* stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
	return written;
}

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
CURLcode DL::CurlGetReq(const string url, string& response) {
	CURL* curl = curl_easy_init();
	CURLcode res = CURL_LAST;
	if (curl) {
		struct curl_slist* header_list = NULL;
		header_list = curl_slist_append(header_list, "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stringWriter);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		res = curl_easy_perform(curl);
	}
	curl_easy_cleanup(curl);
	return res;
}

// GET requests, write output to file
CURLcode DL::CurlDownload(const string url, const string fileName, MyProgress* prog) {
	CURL* curl = curl_easy_init();
	CURLcode res = CURL_LAST;
	FILE* fp;
	if (curl) {
		struct curl_slist* header_list = NULL;
		header_list = curl_slist_append(header_list, "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko");
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
		auto err = fopen_s(&fp, fileName.c_str(), "wb");
		if (fp) {
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			res = curl_easy_perform(curl);
			fclose(fp);
		}
	}
	curl_easy_cleanup(curl);
	return res;
}

// use sayobot api to parse sid,song name,category by osu beatmap url
int DL::ParseInfo(string url, UINT64& sid, string& songName, int& category) {
	logger::WriteLogFormat("[*] parsing %s", url.c_str());
	string parseApiUrl = "https://api.sayobot.cn/v2/beatmapinfo?0=" + url;
	string content;
	Document jContent;
	int status;

	auto res = CurlGetReq(parseApiUrl, content);
	if (res) {
		logger::WriteLogFormat("[-] ParseSid: can't get json content, %s", curl_easy_strerror(res));
		return 1;
	}
	jContent.Parse(content.c_str());
	if (!jContent.HasMember("status")) {
		logger::WriteLogFormat("[-] ParseSid: Wrong json format: doesn't contain member 'status'");
		return 2;
	}
	status = jContent["status"].GetInt();
	if (status) {
		logger::WriteLogFormat("[-] ParseSid: Sayobot err-code: %d", status);
		return 3;
	}
	if (!jContent.HasMember("data")) {
		logger::WriteLogFormat("[-] ParseSid: Wrong json format: doesn't contain member 'data'");
		return 4;
	}
	if (!jContent["data"].HasMember("sid") || !jContent["data"].HasMember("title") || !jContent["data"].HasMember("approved")) {
		logger::WriteLogFormat("[-] ParseSid: Wrong json format: doesn't contain member 'sid' or 'title' or 'approved'");
		return 5;
	}
	sid = jContent["data"]["sid"].GetUint64();
	songName = jContent["data"]["title"].GetString();
	category = jContent["data"]["approved"].GetInt();
	return 0;
}

// download beatmap from sayobot server to file by using sid
int DL::StartDownload(string fileName, UINT64 sid, string taskKey) {
	logger::WriteLogFormat("[*] downloading sid %llu", sid);
	string downloadApiUrl;
	switch (DL::downloadType) {
	case FULL:
		downloadApiUrl = "https://txy1.sayobot.cn/beatmaps/download/full/" + to_string(sid) + "?server=0";
		break;
	case NOVIDEO:
		downloadApiUrl = "https://txy1.sayobot.cn/beatmaps/download/novideo/" + to_string(sid) + "?server=0";
		break;
	case MINI:
		downloadApiUrl = "https://txy1.sayobot.cn/beatmaps/download/mini/" + to_string(sid) + "?server=0";
		break;
	default:
		downloadApiUrl = "https://txy1.sayobot.cn/beatmaps/download/novideo/" + to_string(sid) + "?server=0";
		break;
	}
	MyProgress* myp = new MyProgress;
	myp->taskKey = taskKey;
	auto res = DL::CurlDownload(downloadApiUrl, fileName, myp);
	if (res) {
		logger::WriteLogFormat("[-] StartDownload: err while downloading, %s", curl_easy_strerror(res));
		delete myp;
		return 2;
	}
	delete myp;
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