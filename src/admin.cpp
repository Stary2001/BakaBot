#include "plugin.h"
#include "bot.h"
#include "events.h"
#include "commands/command.h"
#include <algorithm>
#include <functional>
#include "admin.h"
#include <iostream>

std::string AdminPlugin::name()
{
	return "admin";
}

COMMAND(load)
{
	Plugin *p = NULL;

	if (info->in.size() == 0)
	{
		//todo: error
		bot->conn->send_privmsg(info->target, "Usage: load [plugin path]");
		return;
	}

	std::string n = info->pop()->to_string();

	if((p = bot->load_plugin(n)))
	{
		p->init(bot);
		info->next->in.push_back(new StringData("Loaded plugin " + n));
	}
}
END_COMMAND

COMMAND(unload)
{
	if (info->in.size() == 0)
	{
		//todo: error
		bot->conn->send_privmsg(info->target, "Usage: unload [plugin path]");
		return;
	}

	std::string n = info->pop()->to_string();

	if(bot->unload_plugin(n))
	{
		info->next->in.push_back(new StringData("Unloaded plugin " + n));
	}
}
END_COMMAND

COMMAND(save)
{
	bot->config->save();
	info->next->in.push_back(new StringData("Saved config!"));
}
END_COMMAND

COMMAND(permissions)
{
	std::string usage = "Usage: perms [add|del|list] [command] [user]";

	if(info->in.size() == 0)
	{
		//todo: error
		bot->conn->send_privmsg(info->target, usage);
		return;
	}

	std::string mode = info->pop()->to_string();

	if(info->in.size() < (mode != "list" ? 2 : 1))
	{
		//todo: error
		bot->conn->send_privmsg(info->target, usage);
		return;
	}

	std::string k = info->pop()->to_string();

	if(mode == "add")
	{
		std::string vstr = info->pop()->to_string();

		std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + k);
		if(v->type() == NodeType::Null)
		{
			ConfigValue vv = ConfigValue();
			vv.type = NodeType::List;

			vv.list.push_back(vstr);
			bot->config->set("permissions." + k, vv);
		}
		else
		{
			v->as_list().push_back(vstr);
		}
	}
	else if(mode == "del")
	{
		std::string k = info->pop()->to_string();
		std::string vstr = info->pop()->to_string();

		std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + k);

		if(v->type() == NodeType::Null) 
		{
			//todo: error
			bot->conn->send_privmsg(info->target, "'" + k + "' not found!");
			return;
		}

		auto it = std::find(v->as_list().begin(), v->as_list().end(), vstr);
		if(it != v->as_list().end())
		{
			v->as_list().erase(it);
		}
	}
	else if(mode == "list")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + k);
		if(v->type() == NodeType::Null) 
		{
			//todo: error
			bot->conn->send_privmsg(info->target, "'" + k + "' not found!");
			return;
		}

		std::string l;
		for(auto s: v->as_list())
		{
			l += s + (s != *v->as_list().rbegin() ? ", " : "");
		}
		info->next->in.push_back(new StringData(l));
	}
	else
	{
		//todo: error
		bot->conn->send_privmsg(info->target, usage);
	}
}
END_COMMAND

COMMAND(group)
{
	std::string usage = "Usage: group [add|del|list] [group] [user]";

	if(info->in.size() == 0)
	{
		//todo: error
		bot->conn->send_privmsg(info->target, usage);
		return;
	}

	std::string mode = info->pop()->to_string();

	if(info->in.size() < (mode != "list" ? 2 : 1))
	{
		//todo: error
		bot->conn->send_privmsg(info->target, usage);
		return;
	}

	std::string k = info->pop()->to_string();

	if(mode == "add")
	{
		std::string vstr = info->pop()->to_string();

		std::shared_ptr<ConfigNode> v = bot->config->get("groups." + k);
		if(v->type() == NodeType::Null)
		{
			ConfigValue vv = ConfigValue();
			vv.type = NodeType::List;
			vv.list.push_back(vstr);
			bot->config->set("groups." + k, vv);
		}
		else
		{
			v->as_list().push_back(vstr);
		}
	}
	else if(mode == "del")
	{
		std::string vstr = info->pop()->to_string();

		std::shared_ptr<ConfigNode> v = bot->config->get("groups." + k);
		if(v->type() == NodeType::Null) 
		{
			//todo: error
			bot->conn->send_privmsg(info->target, "'" + k + "' not found!");
			return;
		}

		auto it = std::find(v->as_list().begin(), v->as_list().end(), vstr);
		if(it != v->as_list().end())
		{
			v->as_list().erase(it);
		}
	}
	else if(mode == "list")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get("groups." + k);
		if(v->type() == NodeType::Null) 
		{
			//todo: error
			bot->conn->send_privmsg(info->target, "'" + k + "' not found!");
			return;
		}

		std::string l;
		for(auto s: v->as_list())
		{
			l += s + (s != *v->as_list().rbegin() ? ", " : "");
		}
		info->next->in.push_back(new StringData(l));
	}
	else
	{
		//todo: error
		bot->conn->send_privmsg(info->target, usage);
		return;
	}
}
END_COMMAND

COMMAND(config)
{
	std::string usage = "Usage: config [get/set/add/del] [name] [value]";
	if(info->in.size() < 2)
	{
		bot->conn->send_privmsg(info->target, usage);
		return;
	}

	std::string mode = info->pop()->to_string();
	std::string k = info->pop()->to_string();

	if(mode == "get")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get(k);
		if(v->type() == NodeType::Null)
		{
			bot->conn->send_privmsg(info->target, "null");
			return;
		}
		else
		{
			bot->conn->send_privmsg(info->target, Config::serialize(v->v));
			return;
		}
	}
	else if(mode == "set")
	{
		std::string v;
		info->in.erase(info->in.begin());

		for(auto a : info->in)
		{
			v += a->to_string() + " ";
		}
		v = v.substr(0, v.length()-1);

		ConfigValue vv = Config::deserialize(v);
		bot->config->set(k, vv);
	}
	else if(mode == "add")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get(k);
		if(v->type() == NodeType::Null) { return; }
		if(v->type() == NodeType::List)
		{
			std::string s;

			for(auto a : info->in)
			{
				s += a->to_string() + " ";
			}
			s = s.substr(0, s.length()-1);

			v->as_list().push_back(s);
		}
		else if(v->type() == NodeType::Map)
		{
			std::string s;

			info->in.erase(info->in.begin());
			info->in.erase(info->in.begin());

			for(auto a : info->in)
			{
				s += a->to_string() + " ";
			}
			s = s.substr(0, s.length()-1);

			v->as_map()[k] = Config::deserialize(s);
		}
	}
	else if(mode == "del")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get(k);
		if(v->type() == NodeType::Null) { return; }
		std::string s;
		info->in.erase(info->in.begin());

		for(auto a : info->in)
		{
			s += a->to_string() + " ";
		}
		s = s.substr(0, s.length()-1);

		if(v->type() == NodeType::List)
		{
			auto it = std::find(v->as_list().begin(), v->as_list().end(), s);
			if(it != v->as_list().end())
			{
				v->as_list().erase(it);
			}
		}
		else if(v->type() == NodeType::Map)
		{
			v->as_map().erase(s);
		}
	}
	else
	{
		bot->conn->send_privmsg(info->target, usage);
		return;
	}
}
END_COMMAND



void AdminPlugin::init(PluginHost *h)
{
	using namespace std::placeholders;
	Bot *b = (Bot*)h;
	REGISTER_COMMAND(b, load);
	REGISTER_COMMAND(b, unload);
	CommandBase *p = new permissionsCommand();

	b->register_command("perm", p);
	b->register_command("permissions", p);
	b->register_command("perms", p);

	REGISTER_COMMAND(b, group);
	REGISTER_COMMAND(b, save);
	REGISTER_COMMAND(b, config);
	_bot = b;
}

void AdminPlugin::deinit(PluginHost *h)
{
	REMOVE_COMMAND(_bot, load);
	REMOVE_COMMAND(_bot, unload);
	
	_bot->remove_command("perm");
	_bot->remove_command("permissions");
	_bot->remove_command("perms");

	REMOVE_COMMAND(_bot, group);
	REMOVE_COMMAND(_bot, save);
	REMOVE_COMMAND(_bot, config);
}