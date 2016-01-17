#include <curl/curl.h>
#include <string>
#include <vector>
#include <map>
#include <sstream>

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
}