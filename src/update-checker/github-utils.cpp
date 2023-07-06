#include <curl/curl.h>

#include <cstddef>
#include <string>

#include <obs.h>

#include "github-utils.h"

static const std::string GITHUB_LATEST_RELEASE_URL =
	"https://api.github.com/repos/royshil/obs-backgroundremoval/releases/latest";

extern "C" const char *PLUGIN_NAME;
extern "C" const char *PLUGIN_VERSION;

static const std::string USER_AGENT =
	std::string(PLUGIN_NAME) + "/" + std::string(PLUGIN_VERSION);

std::size_t writeFunctionStdString(void *ptr, std::size_t size, size_t nmemb,
				   std::string *data)
{
	data->append(static_cast<char *>(ptr), size * nmemb);
	return size * nmemb;
}

const char *github_utils_get_release(void)
{
	CURL *curl = curl_easy_init();
	if (!curl) {
		blog(LOG_INFO, "Failed to initialize curl");
		return NULL;
	}
	CURLcode code;
	curl_easy_setopt(curl, CURLOPT_URL, GITHUB_LATEST_RELEASE_URL.c_str());
	curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunctionStdString);
	std::string responseBody;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
	code = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	if (code != CURLE_OK) {
		blog(LOG_INFO, "Failed to get latest release info");
		return NULL;
	}

	// Parse the JSON response
	obs_data_t *data = obs_data_create_from_json(responseBody.c_str());
	if (!data) {
		blog(LOG_INFO, "Failed to parse latest release info");
		return NULL;
	}

	// The version is in the "tag_name" property
	char *version = strdup(obs_data_get_string(data, "tag_name"));
	obs_data_release(data);

	// remove the "v" prefix, if it exists
	if (version[0] == 'v') {
		char *newVersion = strdup(version + 1);
		free(version);
		version = newVersion;
	}

	return version;
}