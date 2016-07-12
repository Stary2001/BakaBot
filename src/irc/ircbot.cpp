#include "ircbot.h"
#include "util.h"
#include "admin.h"
#include "logger.h"
#include "lua_plugin.h"
#include "commands/command.h"
#include <iostream>

IRCBot::IRCBot() : Bot(), conn(NULL)
{
	type = "irc";
}

IRCBot::IRCBot(Config *c, Config *l) : Bot(c, l), conn(NULL)
{
	type = "irc";
}

bool IRCBot::print(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);

	std::cout << "<" << ev->sender->nick << "> " << ev->message << std::endl;
	return false;
}

void IRCBot::end_sasl()
{
	conn->send_line("CAP END");
	queue_event(new Event("irc/sasl_done"));
}

bool IRCBot::cb_sasl_done(Event *e)
{
	remove_handler("raw/authenticate", "bot");
	remove_handler("raw/900", "bot");
	remove_handler("raw/902", "bot");
	remove_handler("raw/903", "bot");
	remove_handler("raw/904", "bot");
	queue_event(new Event("irc/cap_done"));

	return false;
}

bool IRCBot::cb_sasl(Event *e)
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
		add_handler("raw/authenticate", "bot", std::bind(&IRCBot::cb_sasl, this, _1));
		add_handler("raw/900", "bot", std::bind(&IRCBot::cb_sasl, this, _1));
		add_handler("raw/902", "bot", std::bind(&IRCBot::cb_sasl, this, _1));
		add_handler("raw/903", "bot", std::bind(&IRCBot::cb_sasl, this, _1));
		add_handler("raw/904", "bot", std::bind(&IRCBot::cb_sasl, this, _1));
		add_handler("irc/sasl_done", "bot", std::bind(&IRCBot::cb_sasl_done, this, _1));

		conn->send_line("AUTHENTICATE PLAIN");
	}

	return false;
}

bool IRCBot::cb_cap_done(Event *e)
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

bool IRCBot::end_of_motd(Event *e)
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
	if (!v->is("null"))
	{
		for (auto chan: v->as_list()) 
		{
			conn->join(chan->to_string());
		}
	}
	return false;
}

bool IRCBot::cb_invite(Event *e)
{
	IRCInviteEvent *ev = reinterpret_cast<IRCInviteEvent*>(e);
	if(ev->target == conn->current_nick)
	{
		conn->join(ev->channel);
	}
	return false;
}

void IRCBot::__connect(ConnectionDispatcher *d, std::string server, short port, bool use_ssl)
{
	using namespace std::placeholders;

	conn = new IRCConnection(this, server, port, use_ssl);

	add_handler("irc/privmsg", "bot", std::bind(&Bot::cb_command, this, _1));
	add_handler("irc/invite", "bot", std::bind(&IRCBot::cb_invite, this, _1));
	add_handler("irc/connected", "bot", std::bind(&IRCBot::end_of_motd, this, _1));

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

	add_handler("irc/cap_done", "bot", std::bind(&IRCBot::cb_cap_done, this, _1));
	add_handler("irc/sasl", "bot", std::bind(&IRCBot::cb_sasl, this, _1));
}

void IRCBot::join(std::string channel)
{
	conn->join(channel);
}

void IRCBot::leave(std::string channel)
{
	conn->part(channel);
}

void IRCBot::quit(std::string message)
{
	conn->quit(message);
}

void IRCBot::send_message(std::string target, std::string message)
{
	conn->send_privmsg(target, message);
}