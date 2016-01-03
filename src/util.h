#pragma once

namespace util
{
	std::vector<std::string> split(const std::string &s, char delim);
	std::string http_request(std::string url);
	std::string http_request(std::string url, std::map<std::string, std::string> params);
}