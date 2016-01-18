#include <curl/curl.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <stdio.h>
#include "tinydir.h"

#ifdef WIN32
#include <direct.h>
#define real_mkdir _mkdir
#define stat _stat
#define S_ISDIR(m) ((m & _S_IFDIR) != 0)

#else
#define real_mkdir ::mkdir
#endif

#include <sys/stat.h>

#include "util.h"
#include "export.h"

namespace util
{
	std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
	{
    	std::stringstream ss(s);
    	std::string item;
    	while (std::getline(ss, item, delim))
    	{
    	    elems.push_back(item);
    	}
    	return elems;
	}

	PLUGINEXPORT std::vector<std::string> split(const std::string &s, char delim)
	{
		std::vector<std::string> elems;
		split(s, delim, elems);
		return elems;
	}

	size_t writefunc(char *ptr, size_t size, size_t nmemb, void *userdata)
	{
	    std::string *d = (std::string*)userdata;
	    d->append(ptr, size * nmemb);
	    return size * nmemb;
	}

	std::string http_request(std::string url, CURL *easy)
	{
	    curl_easy_setopt(easy, CURLOPT_URL, url.c_str());

	    std::string dat;

	    curl_easy_setopt(easy, CURLOPT_WRITEDATA, &dat);
	    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, writefunc);

	    if(curl_easy_perform(easy) == CURLE_OK)
	    {
	        curl_easy_cleanup(easy);
	        return dat;
	    }
	    else
	    {
	        return "";
	    }
	}

	PLUGINEXPORT std::string http_request(std::string url, std::map<std::string, std::string> params)
	{
		CURL *easy = curl_easy_init();
	    bool first = false;
	    for(auto pair: params)
	    {
	        char *q = curl_easy_escape(easy, pair.second.c_str(), pair.second.length());
	        if(first)
	        {
	            first = true;
	            url += "?";
	        }
	        else
	        {
	            url += "&";
	        }
	        url += pair.first + "=" + q ;
	        curl_free(q);
	    }
	    return http_request(url, easy);
	}

	PLUGINEXPORT std::string http_request(std::string url)
	{
		return http_request(url, curl_easy_init());
	}

	// shamelessly taken from http://www.adp-gmbh.ch/cpp/common/base64.html

	static const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";


	static inline bool is_base64(unsigned char c) {
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

	std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) 
	{
		std::string ret;
		int i = 0;
		int j = 0;
		unsigned char char_array_3[3];
		unsigned char char_array_4[4];

		while (in_len--) {
			char_array_3[i++] = *(bytes_to_encode++);
			if (i == 3) {
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;

				for (i = 0; (i <4); i++)
					ret += base64_chars[char_array_4[i]];
				i = 0;
			}
		}

		if (i)
		{
			for (j = i; j < 3; j++)
				char_array_3[j] = '\0';

			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (j = 0; (j < i + 1); j++)
				ret += base64_chars[char_array_4[j]];

			while ((i++ < 3))
				ret += '=';

		}

		return ret;

	}

	std::string base64_decode(std::string const& encoded_string) {
		int in_len = encoded_string.size();
		int i = 0;
		int j = 0;
		int in_ = 0;
		unsigned char char_array_4[4], char_array_3[3];
		std::string ret;

		while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
			char_array_4[i++] = encoded_string[in_]; in_++;
			if (i == 4) {
				for (i = 0; i <4; i++)
					char_array_4[i] = base64_chars.find(char_array_4[i]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
					ret += char_array_3[i];
				i = 0;
			}
		}

		if (i) {
			for (j = i; j <4; j++)
				char_array_4[j] = 0;

			for (j = 0; j <4; j++)
				char_array_4[j] = base64_chars.find(char_array_4[j]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
		}

		return ret;
	}

	namespace fs
	{
		bool is_directory(std::string name)
		{
			struct stat s;
			if (stat(name.c_str(), &s) != -1)
			{
				return S_ISDIR(s.st_mode);
			}
			else
			{
				errno = 0;
				return false;
			}
		}

		bool exists(std::string name)
		{
			struct stat s;
			if (stat(name.c_str(), &s) != -1)
			{
				return true;
			}
			else
			{
				errno = 0;
				return false;
			}
		}

		void mkdir(std::string name)
		{
			real_mkdir(name.c_str());
		}

		void remove(std::string name)
		{
			std::remove(name.c_str());
		}

		void rename(std::string oldname, std::string newname)
		{
			std::rename(oldname.c_str(), newname.c_str());
		}

		std::vector<std::string> listdir(std::string name)
		{
			std::vector<std::string> v;
			if (!exists(name)) return v;

			tinydir_dir dir;
			int i;
			tinydir_open_sorted(&dir, name.c_str());

			for (i = 0; i < dir.n_files; i++)
			{
				tinydir_file file;
				tinydir_readfile_n(&dir, &file, i);
				if (!(file.name[0] == '.' && (file.name[1] == '\0' || file.name[1] == '.')))
				{
					v.push_back(file.name);
				}
			}
			tinydir_close(&dir);

			return v;
		}
	}
}