#pragma once
#include "export.h"

namespace util
{
	PLUGINEXPORT std::vector<std::string> split(const std::string &s, char delim);
	PLUGINEXPORT std::string http_request(std::string url);
	PLUGINEXPORT std::string http_request(std::string url, std::map<std::string, std::string> params);
}