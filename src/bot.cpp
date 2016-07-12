#include "bot.h"
#include "util.h"
#include "admin.h"
#include "logger.h"
#include "lua_plugin.h"
#include "commands/command.h"
#include <iostream>

void Bot::event_thread_func()
{
	while(true)
	{
		if(should_stop)
		{
			break;
		}

		handle_event();
	}
}

Bot::Bot() : should_stop(false), config(NULL), locale(NULL), event_thread(std::bind(&Bot::event_thread_func, this))
{
	using namespace std::placeholders;
	add_extension_handler("lua", std::bind(&Bot::open_lua, this, _1));
}

Bot::Bot(Config *c, Config *l) : should_stop(false), config(c), locale(l), event_thread(std::bind(&Bot::event_thread_func, this))
{
	using namespace std::placeholders;
	add_extension_handler("lua", std::bind(&Bot::open_lua, this, _1));
}

Bot::~Bot()
{
	event_thread.join();

	delete config;
	delete locale;
	
	destroy_plugins();
}

Plugin *Bot::open_lua(std::string filename)
{
	return new LuaPlugin(filename);
}

void Bot::init_plugins()
{
	for(std::pair<std::string, Plugin *> kv : active_plugins)
	{
		kv.second->init(this);
	}
}

void Bot::destroy_plugins()
{
	for(std::pair<std::string, Plugin *> kv : active_plugins)
	{
		kv.second->deinit(this);
		delete kv.second;
	}
}

bool Bot::cb_command(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);

	std::vector<Data*> prefixes = config->get("prefixes")->as_list();

	auto v = config->get("prefixes." + ev->target);
	if (!v->is("null"))
	{
		prefixes = v->as_list();
	}

	for(auto prefix_dat: prefixes)
	{
		std::string prefix = prefix_dat->to_string();
		if (ev->message.length() > prefix.length() && ev->message.substr(0, prefix.length()) == prefix)
		{
			ev->message = ev->message.substr(prefix.length());
			std::cout << "got command " << ev->message << std::endl;
			Command::run(this, ev);
			return true;
		}
	}

	return false;
}

CommandBase *Bot::get_command(std::string s)
{
	if(commands.find(s) != commands.end())
	{
		return commands[s];
	}
	return NULL;
}

void Bot::register_command(std::string s, CommandBase *b)
{
	commands[s] = b;
}

void Bot::remove_command(std::string s)
{
	if(commands.find(s) != commands.end())
	{
		delete commands[s];
		commands.erase(s);
	}
}

void Bot::connect(ConnectionDispatcher *d)
{
	Plugin *p;

	std::string server = config->get("server.host")->as_string();
	short port = (short)config->get("server.port")->as_int();
	bool ssl = (config->get("server.ssl")->as_string() == "true");

	active_plugins["admin"] = new AdminPlugin();

	std::shared_ptr<ConfigNode> v = config->get("modules.load");
	if (!v->is("null"))
	{
		for (auto path: v->as_list()) 
		{
			p = load_plugin(path->to_string());
			if (p == NULL) 
			{
				Logger::instance->log(locale->get("plugins.noload")->as_string()+path->to_string(), LogLevel::ERR);
			}
		}
	}
	init_plugins();
	__connect(d, server, port, ssl);
}
