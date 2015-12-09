#include <iostream>
#include <regex>
#include "plugin.h"
#include "bot.h"

#define SCROLLBACK_SIZE 100

class SedPlugin : public Plugin
{
public:
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);

private:
    bool msg(Bot *b, User &sender, std::vector<std::string> &params);
    std::map<Bot *, std::map<std::string, std::vector<std::string>>> scrollback;
};

extern "C" Plugin* plugin_init(PluginHost *h)
{
	return new SedPlugin();
}

void SedPlugin::init(PluginHost *h)
{
	std::cout << "init" << std::endl;
	Bot *b = (Bot*)h;

	using namespace std::placeholders;
	b->add_callback("privmsg", std::bind(&SedPlugin::msg, this, b, _1, _2));

	//scrollback = std::map<Bot *, std::map<std::string, std::vector<std::string>>>();
	scrollback[b] = std::map<std::string, std::vector<std::string>>();
}

void SedPlugin::deinit(PluginHost *h)
{
	std::cout << "deinit" << std::endl;
}

bool SedPlugin::msg(Bot *b, User &sender, std::vector<std::string> &params)
{
	std::cout << params[0] << std::endl;
	std::cout << params[0][0] << std::endl;

	if(params[0][0] == '#') // to a channel?
	{
		if(scrollback[b].find(params[0]) == scrollback[b].end())
		{
			scrollback[b][params[0]] = std::vector<std::string>();
		}

		std::vector<std::string> &scroll = scrollback[b][params[0]];
		
		if(params[1].substr(0, 2) == "s/")
		{
			std::cout << params[1] << " " << scroll.size() << std::endl;
			int beg = 2; 
			int middle = params[1].find('/', beg + 1);

			while(true)
			{
				if(middle == std::string::npos)
				{
					return false;
				}
				else if(params[1][middle - 1] == '\\')
				{
					middle = params[1].find('/', middle + 1);
				}
				else
				{
					break;
				}
			}

			int end = params[1].find('/', middle + 1);
			while(true)
			{	
				if(end != std::string::npos && params[1][middle - 1] == '\\') 
				{
					end = params[1].find('/', end + 1);
				}
				else
				{
					break;
				}
			}

			if(end == std::string::npos)
			{
				end = params[1].length();
			}

			std::string regex = params[1].substr(beg, middle - beg);
			std::string replacement = params[1].substr(middle + 1, end - middle - 1);
			std::string flags;
			if(end != params[1].length())
			{
				flags = params[1].substr(end + 1);
			}
			
			//std::cout << regex << " / " << replacement << " / " << flags << std::endl;

			auto syn_flags = std::regex_constants::ECMAScript;
			if(flags.find("i") != std::string::npos)
			{
				syn_flags |= std::regex_constants::icase;
			}

			try
			{
				std::regex r(regex.c_str(), syn_flags);

				auto match_flags = std::regex_constants::format_default;

				if(flags.find("g") == std::string::npos)
				{
					match_flags |= std::regex_constants::format_first_only;
				}

				auto it = scroll.rbegin();
				for(; it != scroll.rend(); it++)
				{
					if(std::regex_search(*it, r, match_flags))
					{
						std::string resp = std::regex_replace(*it, r, replacement, match_flags);
						b->conn->send_privmsg(params[0], resp);
						scroll.push_back(resp);
						break;
					}
				}
			}
			catch(std::regex_error &e)
			{
				b->conn->send_privmsg(params[0], "you broke it! gg!");
			}
		}
		else
		{
			scroll.push_back(params[1]);
			if(scroll.size() >= SCROLLBACK_SIZE)
			{
				scroll.erase(scroll.begin());
			}
		}

		
	}
	// BakaBot :msg
	return false;
}
