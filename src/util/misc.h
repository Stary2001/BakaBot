namespace util
{
	PLUGINEXPORT std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty = true);
	PLUGINEXPORT int rand(int a, int b);
	PLUGINEXPORT std::string http_request(std::string url);
	PLUGINEXPORT std::string http_request(std::string url, std::map<std::string, std::string> params);
	std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	std::string base64_decode(std::string const& encoded_string);

	namespace fs
	{
		bool is_directory(std::string name);
		bool exists(std::string name);
		void mkdir(std::string name);
		void remove(std::string name);
		void rename(std::string oldname, std::string newname);
		std::vector<std::string> listdir(std::string name);
	}
}