#include "utils/forCurl.h"

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
	size_t newLength = size * nmemb;
	s->append((char*)contents, newLength);
	return newLength;
}

std::string performCurlRequest(const std::string& url) {
	CURL* curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (res != CURLE_OK) {
			std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
			return "";
		}
	}
	return readBuffer;
}

json performCurlRequestJson(const std::string& url) {
	std::string response = performCurlRequest(url);
	if (!response.empty()) {
		try {
			return json::parse(response);
		}
		catch (const json::parse_error& e) {
			std::cerr << "JSON parsing error: " << e.what() << std::endl;
		}
	}
	return nullptr;
}