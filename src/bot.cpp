#include "bot.h"
#include "util.h"
#include "admin.h"
#include "logger.h"
#include <iostream>

void Bot::event_thread_func()
{
	while(true)
	{
		handle_event();
	}
}

Bot::Bot() : conn(NULL), config(NULL), event_thread(std::bind(&Bot::event_thread_func, this))
{}

Bot::Bot(Config *c) : conn(NULL), config(c), event_thread(std::bind(&Bot::event_thread_func, this))
{}

void Bot::init_plugins()
{
	// Plugin::register_plugin(new SedPlugin()); // todo: dynamic plugins

	for(std::pair<std::string, Plugin *> kv : active_plugins)
	{
		kv.second->init(this);
	}
}

bool Bot::print(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);

	std::cout << "<" << ev->sender->nick << "> " << ev->message << std::endl;
	return false;
}

bool Bot::cb_command(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);
	std::string prefix = config->get("prefix")->as_string();

	if(ev->message.substr(0, prefix.length()) == prefix)
	{
		ev->message = ev->message.substr(prefix.length());
		std::cout << "got command " << ev->message << std::endl;
		auto bits = util::split(ev->message, ' ');
		std::string name = *(bits.begin());
		bits.erase(bits.begin());

		auto it = handlers.end();

		if((it=handlers.find("command/" + name)) != handlers.end())
		{
			if(it->second.size() == 0)
			{
				conn->send_privmsg(ev->target, "No handler for command '" + name + "'!");
			}
			else if(check_permissions(ev->sender, conn->get_channel(ev->target), name))
			{
				queue_event(new IRCCommandEvent(ev->sender, name, ev->target, bits));
			}
			else
			{
				conn->send_privmsg(ev->target, "***REMOVED***");
			}
		}
		else
		{
			conn->send_privmsg(ev->target, "No handler for command '" + name + "'!");
		}

		return true;
	}

	return false;
}

bool Bot::end_of_motd(Event *e)
{
	//IRCConnectedEvent *ev = reinterpret_cast<IRCConnectedEvent*>(e);
	std::string uname = config->get("server.nickserv.username")->as_string();
	std::string pass = config->get("server.nickserv.password")->as_string();

	if(uname != "" && pass != "")
	{
		conn->send_privmsg("NickServ", "identify " + uname + " " + pass);
	}
	std::shared_ptr<ConfigNode> v = config->get("channels.join");
	if (v->type()!=NodeType::Null) {
		for (auto chan: v->as_list()) {
			conn->join(chan);
		}
	}
	return false;
}

bool Bot::cb_invite(Event *e)
{
	IRCInviteEvent *ev = reinterpret_cast<IRCInviteEvent*>(e);
	if(ev->target == conn->current_nick)
	{
		conn->join(ev->channel);
	}
	return false;
}

void Bot::connect(ConnectionDispatcher *d)
{
	Plugin *p;
	using namespace std::placeholders;
	std::string server = config->get("server.host")->as_string();
	short port = config->get("server.port")->as_int();

	conn = new IRCConnection(this, server, port);

	active_plugins["admin"] = new AdminPlugin();
	std::shared_ptr<ConfigNode> v = config->get("modules.load");
	if (v->type()!=NodeType::Null) {
		for (auto plugin: v->as_list()) {
			if (!(p=load_plugin(plugin))) {
				Logger::instance->log("Could not load plugin "+plugin, LogLevel::ERROR);
			}
		}
	}
	init_plugins();

	add_handler("irc/message", "bot", std::bind(&Bot::cb_command, this, _1));
	add_handler("irc/invite", "bot", std::bind(&Bot::cb_invite, this, _1));
	add_handler("irc/connected", "bot", std::bind(&Bot::end_of_motd, this, _1));

	d->add(conn);
	state = IRCState::CONNECTED;

	std::string pass = config->get("server.password")->as_string();

	if(pass != "")
	{
		conn->send_line("PASS " + pass);
		state = IRCState::PASS;
	}
	else
	{
		std::string nick = config->get("server.nick")->as_string();
		std::string uname = config->get("server.username")->as_string();
		std::string rname = config->get("server.realname")->as_string();

		conn->nick(nick);
		conn->send_line("USER " + uname + " * * :" + rname);
		state = IRCState::USER;
	}
}

bool Bot::check_permissions(User *u, Channel &c, std::string command)
{
	if(!u->synced || u->account == "*")
	{
		return false;
	}

	std::shared_ptr<ConfigNode> v = config->get("permissions." + command);

	if(v->type() == NodeType::List)
	{
		for(auto a : v->as_list())
		{
			std::string b = a;
			if(b == u->account)
			{
				return true;
			}
			else if(b.substr(0, 6) == "group/")
			{
				b = b.substr(6);

				if(b == "special/all")
				{
					return true;
				}
				else if(b == "special/none")
				{
					return false;
				}
				else if(b == "special/ops")
				{
					return c.users[u->nick].modes['o'];
				}
				else if(b == "special/voice")
				{
					return c.users[u->nick].modes['v'];
				}

				std::shared_ptr<ConfigNode> v2 = config->get("groups." + b);
				if(v2->type() == NodeType::List)
				{
					for(auto c : v2->as_list())
					{
						if(c == u->account)
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}
