#include "plugin.h"
#include "bot.h"
#include "events.h"
#include <functional>
#include "admin.h"
#include <iostream>

void AdminPlugin::init(PluginHost *h)
{
	using namespace std::placeholders;
	Bot *b = (Bot*)h;
	b->add_handler("command/load", "admin", std::bind(&AdminPlugin::load, this, _1));
	b->add_handler("command/unload", "admin", std::bind(&AdminPlugin::unload, this, _1));

	bot = b;
}

std::string AdminPlugin::name()
{
	return "admin";
}

bool AdminPlugin::load(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);
	Plugin *p = NULL;

	if((p = bot->load_plugin(ev->params[0])))
	{
		p->init(bot);
		bot->conn->send_privmsg(ev->target, "Loaded plugin " + ev->params[0]);
	}
}

bool AdminPlugin::unload(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);
	if(bot->unload_plugin(ev->params[0]))
	{
		bot->conn->send_privmsg(ev->target, "Unloaded plugin " + ev->params[0]);
	}
}

void AdminPlugin::deinit(PluginHost *h)
{
	bot->remove_handler("command/load", "admin");
	bot->remove_handler("command/unload", "admin");
}
