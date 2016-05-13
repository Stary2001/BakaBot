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
		info->error("Usage: load [plugin path]");
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
		info->error("Usage: unload [plugin path]");
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
		info->error(usage);
	}

	std::string mode = info->pop()->to_string();

	if(info->in.size() < (mode != "list" ? 2 : 1))
	{
		info->error(usage);
	}

	std::string k = info->pop()->to_string();

	if(mode == "add")
	{
		Data *vstr = info->pop();

		std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + k);
		if (v->is("null"))
		{
			std::vector<Data*> v = {vstr};
			ListData *d = new ListData(v);
			bot->config->set("permissions." + k, d);
		}
		else if(v->is("list"))
		{
			ListData *l = (ListData*)v->v;
			l->push_back(vstr);
		}
	}
	else if(mode == "del")
	{
		std::string k = info->pop()->to_string();
		std::string vstr = info->pop()->to_string();

		std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + k);

		if (v->is("null"))
		{
			info->error("'" + k + "' not found!");
			return;
		}

		DataEquals f(vstr);
		auto it = std::find_if(v->as_list().begin(), v->as_list().end(), f);

		if(it != v->as_list().end())
		{
			v->as_list().erase(it);
		}
	}
	else if(mode == "list")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + k);
		if (v->is("null"))
		{
			info->error("'" + k + "' not found!");
			return;
		}

		std::string l;
		for(auto s: v->as_list())
		{
			l += s->to_string() + (s != *v->as_list().rbegin() ? ", " : "");
		}
		info->next->in.push_back(new StringData(l));
	}
	else
	{
		info->error(usage);
	}
}
END_COMMAND

COMMAND(group)
{
	std::string usage = "Usage: group [add|del|list] [group] [user]";

	if(info->in.size() == 0)
	{
		info->error(usage);
		return;
	}

	std::string mode = info->pop()->to_string();

	if(info->in.size() < (mode != "list" ? 2 : 1))
	{
		info->error(usage);
		return;
	}

	std::string k = info->pop()->to_string();

	if(mode == "add")
	{
		Data *vstr = info->pop();

		std::shared_ptr<ConfigNode> v = bot->config->get("groups." + k);
		if (!v->is("null"))
		{
			std::vector<Data*> v = {vstr};
			ListData *d = new ListData(v);
			bot->config->set("groups." + k, d);
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
		if (v->is("null"))
		{
			info->error("'" + k + "' not found!");
		}

		DataEquals f(vstr);
		auto it = std::find_if(v->as_list().begin(), v->as_list().end(), f);
		if(it != v->as_list().end())
		{
			v->as_list().erase(it);
		}
	}
	else if(mode == "list")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get("groups." + k);
		if (v->is("null"))
		{
			info->error("'" + k + "' not found!");
		}

		std::string l;
		for(auto s: v->as_list())
		{
			l += s->to_string() + (s != *v->as_list().rbegin() ? ", " : "");
		}
		info->next->in.push_back(new StringData(l));
	}
	else
	{
		info->error(usage);
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
		if (v->is("null"))
		{
			info->next->in.push_back(new StringData("null"));
		}
		else
		{
			info->next->in.push_back(v->v);
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

		bot->config->set(k, Data::deserialize(v));
	}
	else if(mode == "add")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get(k);
		if (v->is("null")) { return; }
		if (v->is("list"))
		{
			std::string s;

			for(auto a : info->in)
			{
				s += a->to_string() + " ";
			}
			s = s.substr(0, s.length()-1);

			v->as_list().push_back(new StringData(s));
		}
		if (v->is("map"))
		{
			std::string s;

			info->in.erase(info->in.begin());
			info->in.erase(info->in.begin());

			for(auto a : info->in)
			{
				s += a->to_string() + " ";
			}
			s = s.substr(0, s.length()-1);

			v->as_map()[k] = Data::deserialize(s);
		}
	}
	else if(mode == "del")
	{
		std::shared_ptr<ConfigNode> v = bot->config->get(k);
		if(v->is("null")) { return; }
		std::string s;
		info->in.erase(info->in.begin());

		for(auto a : info->in)
		{
			s += a->to_string() + " ";
		}
		s = s.substr(0, s.length()-1);

		if (v->is("list"))
		{
			DataEquals f(s);
			auto it = std::find_if(v->as_list().begin(), v->as_list().end(), f);
			if(it != v->as_list().end())
			{
				v->as_list().erase(it);
			}
		}
		else if (v->is("map"))
		{
			v->as_map().erase(s);
		}
	}
	else
	{
		info->error(usage);
	}
}
END_COMMAND

COMMAND(quit)
{
	bot->should_stop = true;
	bot->conn->quit("Quit issued by " + info->sender->nick + "..");
}
END_COMMAND

COMMAND(op)
{
	if(info->in.size() == 0)
	{
		info->error("Usage: op [names]");
	}

	if(info->target[0] == '#')
	{
		while(info->in.size() != 0)
		{
			bot->conn->mode(info->target, "+o", info->pop()->to_string());
		}
	}
	else
	{
		info->error("Must be used in a channel!");
	}
}
END_COMMAND

COMMAND(deop)
{
	if(info->in.size() == 0)
	{
		info->error("Usage: deop [names]");
	}

	if(info->target[0] == '#')
	{
		while(info->in.size() != 0)
		{
			bot->conn->mode(info->target, "-o", info->pop()->to_string());
		}
	}
	else
	{
		info->error("Must be used in a channel!");
	}
}
END_COMMAND

COMMAND(voice)
{
	if(info->in.size() == 0)
	{
		info->error("Usage: voice [names]");
	}

	if(info->target[0] == '#')
	{
		while(info->in.size() != 0)
		{
			bot->conn->mode(info->target, "+v", info->pop()->to_string());
		}
	}
	else
	{
		info->error("Must be used in a channel!");
	}
}
END_COMMAND

COMMAND(devoice)
{
	if(info->in.size() == 0)
	{
		info->error("Usage: devoice [names]");
	}


	if(info->target[0] == '#')
	{
		while(info->in.size() != 0)
		{
			bot->conn->mode(info->target, "-v", info->pop()->to_string());
		}
	}
	else
	{
		info->error("Must be used in a channel!");
	}
}
END_COMMAND

COMMAND(join)
{
	if(info->in.size() == 0)
	{
		info->error("Usage: join [channel]");
	}

	bot->conn->join(info->pop()->to_string());
}
END_COMMAND

COMMAND(part)
{
	if(info->in.size() == 0)
	{
		info->error("Usage: part [channel]");
	}
	bot->conn->part(info->pop()->to_string());
}
END_COMMAND

/*COMMAND(alias)
{
	std::string usage = "Usage: alias [add|del|list] [name]";
	if(info->in.size() == 0)
	{
		info->error(usage);
	}

	std::string mode = info->pop()->to_string();
	if(mode == "list")
	{

	}
	else if(mode == "add")
	{
		if(info->in.size() == 2)
		{
			std::strign k = info->pop()->to_string();
			std::string v = info->pop()->to_string();

			bot->aliases[k] = v;
		}
		else
		{
			info->error(usage);
		}
	}
	else if(mode == "del")
	{
		if(info->in.size() == 1)
		{
			std::string n = info->pop()->to_string();
			bot->aliases.erase(n);
		}
		else
		{
			info->error(usage);
		}
	}
}
END_COMMAND*/

void AdminPlugin::init(PluginHost *h)
{
	using namespace std::placeholders;
	Bot *b = (Bot*)h;
	REGISTER_COMMAND(b, load);
	REGISTER_COMMAND(b, unload);

	b->register_command("perm", new permissionsCommand());
	b->register_command("permissions", new permissionsCommand());
	b->register_command("perms", new permissionsCommand());

	REGISTER_COMMAND(b, group);
	REGISTER_COMMAND(b, save);
	REGISTER_COMMAND(b, config);
	REGISTER_COMMAND(b, quit);

	REGISTER_COMMAND(b, op);
	REGISTER_COMMAND(b, deop);
	REGISTER_COMMAND(b, voice);
	REGISTER_COMMAND(b, devoice);
	REGISTER_COMMAND(b, join);
	REGISTER_COMMAND(b, part);

	//REGISTER_COMMAND(b, alias);
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
	REMOVE_COMMAND(_bot, quit);

	REMOVE_COMMAND(_bot, op);
	REMOVE_COMMAND(_bot, deop);
	REMOVE_COMMAND(_bot, voice);
	REMOVE_COMMAND(_bot, devoice);
	REMOVE_COMMAND(_bot, join);
	REMOVE_COMMAND(_bot, part);

	//REMOVE_COMMAND(_bot, alias);
}