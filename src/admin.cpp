#include "plugin.h"
#include "bot.h"
#include "events.h"
#include <algorithm>
#include <functional>
#include "admin.h"
#include <iostream>

void AdminPlugin::init(PluginHost *h)
{
	using namespace std::placeholders;
	Bot *b = (Bot*)h;
	b->add_handler("command/load", "admin", std::bind(&AdminPlugin::load, this, _1));
	b->add_handler("command/unload", "admin", std::bind(&AdminPlugin::unload, this, _1));
	b->add_handler("command/perms", "admin", std::bind(&AdminPlugin::permissions, this, _1));
	b->add_handler("command/group", "admin", std::bind(&AdminPlugin::group, this, _1));
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

	return true;
}

bool AdminPlugin::unload(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);
	if(bot->unload_plugin(ev->params[0]))
	{
		bot->conn->send_privmsg(ev->target, "Unloaded plugin " + ev->params[0]);
	}

	return true;
}

bool AdminPlugin::permissions(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);
	std::string usage = "Usage: perms [add|del|list] [command] [user]";
	if(ev->params.size() < 3)
	{
		bot->conn->send_privmsg(ev->target, usage);
		return true;
	}

	if(ev->params[0] == "add")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + ev->params[1]);
		if(v->type() == NodeType::Null)
		{
			ConfigValue vv = ConfigValue();
			vv.type = NodeType::List;
			vv.list.push_back(ev->params[2]);
			bot->config->set("permissions." + ev->params[1], vv);
		}
		else
		{
			v->as_list().push_back(ev->params[2]);
		}
	}
	else if(ev->params[0] == "del")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + ev->params[1]);
		if(v->type() == NodeType::Null) return true;

		auto it = std::find(v->as_list().begin(), v->as_list().end(), ev->params[2]);
		if(it != v->as_list().end())
		{
			v->as_list().erase(it);
		}
	}
	else if(ev->params[0] == "list")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + ev->params[1]);
		if(v->type() == NodeType::Null) return true;
		// todo :D
		
	}
	else
	{
		bot->conn->send_privmsg(ev->target, usage);
	}

	return true;
}

bool AdminPlugin::group(Event *e)
{
	IRCCommandEvent *ev = reinterpret_cast<IRCCommandEvent*>(e);

	if(ev->params[0] == "add")
	{

	}
	else if(ev->params[0] == "del")
	{

	}
	else if(ev->params[0] == "list")
	{

	}
	else
	{

	}

	return true;
}

void AdminPlugin::deinit(PluginHost *h)
{
	bot->remove_handler("command/load", "admin");
	bot->remove_handler("command/unload", "admin");
}
