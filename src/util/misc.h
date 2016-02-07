namespace util
{
	PLUGINEXPORT std::vector<std::string> split(const std::string &s, char delim);
	PLUGINEXPORT std::string http_request(std::string url);
	PLUGINEXPORT std::string http_request(std::string url, std::map<std::string, std::string> params);
	std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	std::string base64_decode(std::string const& encoded_string);
}