#include "osu_auth.h"
#include "downloader.h"
#include <curl/curl.h>
#include "logger.h"
#include "utils.h"

char osuAuth::username[0x100] = "";
char osuAuth::password[0x100] = "";
char osuAuth::cookie[0x1000] = "";

// return csrf token
string osuAuth::GetCsrfToken() {
	string token = "";
	string content;
	size_t off = 0;

	auto res = DL::CurlGetReq("https://osu.ppy.sh", content);
	if (res) {
		logger::WriteLogFormat("[-] GetCsrfToken: can't get csrf token, %s", curl_easy_strerror(res));
	}
	off = content.find("csrf-token");
	size_t offStart = content.find("content=\"", off);
	size_t offEnd = content.find("\">", off);
	if (offStart == -1 || offEnd == -1) {
		return "";
	}
	offStart += 9;
	token = content.substr(offStart, offEnd - offStart);
	return token;
}

// if login success, cookie will be set
bool osuAuth::login(string& errorDetail) {
	memset(cookie,0,sizeof(cookie));
	string token = GetCsrfToken();
	if (token.empty()) {
		errorDetail = "Network stuck, get CSRF token fail.\nPlease try again.";
		return false;
	}
	string postUrl = "https://osu.ppy.sh/session";
	string tokenHeader = "x-csrf-token: " + token;
	string postData = "_token=" + token + "&username=" + username + "&password=" + password;
	string header = "";
	string content = "";
	CURL* curl = curl_easy_init();
	CURLcode resCode = CURL_LAST;
	if (curl) {
		struct curl_slist* header_list = NULL;
		header_list = curl_slist_append(header_list, "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko");
		header_list = curl_slist_append(header_list, tokenHeader.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
		curl_easy_setopt(curl, CURLOPT_URL, postUrl.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, stringWriter);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*)&header);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stringWriter);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&content);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 8000);
		if (DL::useProxy) {
			curl_easy_setopt(curl, CURLOPT_PROXY, DL::proxyServer);
			curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
		}
		resCode = curl_easy_perform(curl);
		if (resCode == CURLE_OK) {
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			if (response_code != 200) {
				// password not correct
				logger::WriteLogFormat("[-] login: response_code %ld", response_code);
				memset(password, 0, sizeof(password));
				errorDetail = "Password not correct, login fail.";
				return false;
			}
		}
	}
	curl_easy_cleanup(curl);
	if (resCode) {
		errorDetail = string("Network stuck, ").append(curl_easy_strerror(resCode));
		return false;
	}
	size_t offStart = 0;
	size_t offEnd = 0;
	while (true) {
		if (offEnd) {
			offStart = offEnd + 1;
		}
		offEnd = header.find("\n", offEnd + 1);
		if (offEnd == -1) {
			break;
		}
		string tmp = header.substr(offStart, offEnd - offStart - 1);
		if (tmp.length() > 11 && tmp.find("domain=.ppy.sh") != -1 && tmp.find("osu_session=") != -1) {
			// login success, find cookie
			offStart = tmp.find("osu_session=");
			offEnd = tmp.find(";", offStart)+1;
			string tmpCookie = tmp.substr(offStart, offEnd - offStart);
			strncpy_s(cookie, tmpCookie.c_str(), tmpCookie.length());
			memset(username, 0, sizeof(username));
			memset(password, 0, sizeof(password));
			return true;
		}
	}
	errorDetail = "Internal error, login success but can't find cookie.\n";
	return false;
}