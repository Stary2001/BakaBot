#include "bot.h"
#include "util.h"
#include "admin.h"
#include "logger.h"
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

Bot::Bot() : should_stop(false), conn(NULL), config(NULL), locale(NULL), event_thread(std::bind(&Bot::event_thread_func, this))
{}

Bot::Bot(Config *c, Config *l) : should_stop(false), conn(NULL), config(c), locale(l), event_thread(std::bind(&Bot::event_thread_func, this))
{}

Bot::~Bot()
{
	event_thread.join();

	delete config;
	delete locale;
	
	destroy_plugins();
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

bool Bot::print(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);

	std::cout << "<" << ev->sender->nick << "> " << ev->message << std::endl;
	return false;
}

bool Bot::cb_command(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);


	std::vector<std::string> prefixes = config->get("prefixes")->as_list();

	auto v = config->get("prefixes." + ev->target);
	if (v->type() != NodeType::Null)
	{
		prefixes = v->as_list();
	}

	if(ev->message.length() > prefix.length() && ev->message.substr(0, prefix.length()) == prefix)
	{
		ev->message = ev->message.substr(prefix.length());
		std::cout << "got command " << ev->message << std::endl;
		Command::run(this, ev);

		/*auto it = handlers.end();
	for (auto prefix : prefixes)
	{
		if (ev->message.length() > prefix.length() && ev->message.substr(0, prefix.length()) == prefix)
		{
			ev->message = ev->message.substr(prefix.length());
			std::cout << "got command " << ev->message << std::endl;
			auto bits = util::split(ev->message, ' ');
			std::string name = *(bits.begin());
			bits.erase(bits.begin());

			auto it = handlers.end();

			if ((it = handlers.find("command/" + name)) != handlers.end())
			{
				if (it->second.size() == 0)
				{
					conn->send_privmsg(ev->target, locale->get("commands.nohandler")->as_string() + " '" + name + "'!");
				}
				else if (check_permissions(ev->sender, conn->get_channel(ev->target), name))
				{
					queue_event(new IRCCommandEvent(ev->sender, name, ev->target, bits));
				}
				else
				{
					conn->send_privmsg(ev->target, locale->get("commands.nopermission")->as_string());
				}
			}
			else
			{
				conn->send_privmsg(ev->target, locale->get("commands.nohandler")->as_string() + " '" + name + "'!");
			}

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

void Bot::end_sasl()
{
	conn->send_line("CAP END");
	queue_event(new Event("irc/sasl_done"));
}

bool Bot::cb_sasl_done(Event *e)
{
	remove_handler("raw/authenticate", "bot");
	remove_handler("raw/900", "bot");
	remove_handler("raw/902", "bot");
	remove_handler("raw/903", "bot");
	remove_handler("raw/904", "bot");
	queue_event(new Event("irc/cap_done"));

	return false;
}

bool Bot::cb_sasl(Event *e)
{
	using namespace std::placeholders;

	if (e->type == "raw/authenticate")
	{
		std::string uname = config->get("server.nickserv.username")->as_string();
		std::string pass = config->get("server.nickserv.password")->as_string();

		RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);
		if (ev->params[0] == "+") // ok go
		{
			std::string s = "baka";
			s.push_back('\0');
			s += uname;
			s.push_back('\0');
			s += pass;

			conn->send_line("AUTHENTICATE " + util::base64_encode((const unsigned char*)s.data(), s.length()));
		}
	}
	else if (e->type == "raw/900")
	{
		nickserv_done = true;
	}
	else if (e->type == "raw/903")
	{
		end_sasl();
	}
	else if (e->type == "raw/902" || e->type == "raw/904" || e->type == "raw/906") // fail
	{
		end_sasl();
	}
	else if (e->type == "irc/sasl")
	{
		add_handler("raw/authenticate", "bot", std::bind(&Bot::cb_sasl, this, _1));
		add_handler("raw/900", "bot", std::bind(&Bot::cb_sasl, this, _1));
		add_handler("raw/902", "bot", std::bind(&Bot::cb_sasl, this, _1));
		add_handler("raw/903", "bot", std::bind(&Bot::cb_sasl, this, _1));
		add_handler("raw/904", "bot", std::bind(&Bot::cb_sasl, this, _1));
		add_handler("irc/sasl_done", "bot", std::bind(&Bot::cb_sasl_done, this, _1));

		conn->send_line("AUTHENTICATE PLAIN");
	}

	return false;
}

bool Bot::cb_cap_done(Event *e)
{
	std::string pass = config->get("server.password")->as_string();

	if (pass != "")
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

	return false;
}

bool Bot::end_of_motd(Event *e)
{
	//IRCConnectedEvent *ev = reinterpret_cast<IRCConnectedEvent*>(e);
	std::string uname = config->get("server.nickserv.username")->as_string();
	std::string pass = config->get("server.nickserv.password")->as_string();

	if(!nickserv_done && uname != "" && pass != "")
	{
		conn->send_privmsg("NickServ", "identify " + pass); // fucking. anope.
		nickserv_done = true;
	}

	std::shared_ptr<ConfigNode> v = config->get("channels.join");
	if (v->type()!=NodeType::Null)
	{
		for (auto chan: v->as_list()) 
		{
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
	short port = (short)config->get("server.port")->as_int();
	bool ssl = (config->get("server.ssl")->as_string() == "true");

	conn = new IRCConnection(this, server, port, ssl);

	active_plugins["admin"] = new AdminPlugin();

	std::shared_ptr<ConfigNode> v = config->get("modules.load");
	if (v->type()!=NodeType::Null) 
	{
		for (auto plugin: v->as_list()) 
		{
			p = load_plugin(plugin);
			if (p == NULL) 
			{
				Logger::instance->log(locale->get("plugins.noload")->as_string()+plugin, LogLevel::ERR);
			}
		}
	}
	init_plugins();

	add_handler("irc/privmsg", "bot", std::bind(&Bot::cb_command, this, _1));
	add_handler("irc/invite", "bot", std::bind(&Bot::cb_invite, this, _1));
	add_handler("irc/connected", "bot", std::bind(&Bot::end_of_motd, this, _1));

	d->add(conn);
	state = IRCState::CONNECTED;

	std::vector<std::string> caps;

	std::string uname = config->get("server.nickserv.username")->as_string();
	std::string pass = config->get("server.nickserv.password")->as_string();

	if (uname != "" && pass != "")
	{
		caps.push_back("sasl");
	}

	conn->do_cap(caps);
	state = IRCState::CAP;

	add_handler("irc/cap_done", "bot", std::bind(&Bot::cb_cap_done, this, _1));
	add_handler("irc/sasl", "bot", std::bind(&Bot::cb_sasl, this, _1));
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
