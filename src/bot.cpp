#include "bot.h"
#include "util.h"
#include "admin.h"
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

	std::cout << "<" << ev->sender.nick << "> " << ev->message << std::endl;
	return false;
}

bool Bot::cb_command(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);
	std::string prefix = config->get("prefix").asString();

	if(ev->message.substr(0, prefix.length()) == prefix)
	{
		ev->message = ev->message.substr(prefix.length());
		std::cout << "got command " << ev->message << std::endl;
		auto bits = util::split(ev->message, ' ');
		std::string name = *(bits.begin());
		bits.erase(bits.begin());

		queue_event(new IRCCommandEvent(ev->sender, name, ev->target, bits));
		return true;
	}

	return false;
}

bool Bot::end_of_motd(Event *e)
{
	//IRCConnectedEvent *ev = reinterpret_cast<IRCConnectedEvent*>(e);
	std::string uname = config->get("server.nickserv.username").asString();
	std::string pass = config->get("server.nickserv.password").asString();

	if(uname != "" && pass != "")
	{
		conn->send_privmsg("NickServ", "identify " + uname + " " + pass);
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
	using namespace std::placeholders;
	std::string server = config->get("server.host").asString();
	short port = config->get("server.port").asInt();

	conn = new IRCConnection(this, server, port);

	active_plugins["admin"] = new AdminPlugin();

	init_plugins();

	add_handler("irc/message", "bot", std::bind(&Bot::cb_command, this, _1));
	add_handler("irc/invite", "bot", std::bind(&Bot::cb_invite, this, _1));
	add_handler("irc/connected", "bot", std::bind(&Bot::end_of_motd, this, _1));

	d->add(conn);
	state = IRCState::CONNECTED;

	std::string pass = config->get("server.password").asString();

	if(pass != "")
	{
		conn->send_line("PASS " + pass);
		state = IRCState::PASS;
	}
	else
	{
		std::string nick = config->get("server.nick").asString();
		std::string uname = config->get("server.username").asString();
		std::string rname = config->get("server.realname").asString();

		conn->nick(nick);
		conn->send_line("USER " + uname + " * * :" + rname);
		state = IRCState::USER;
	}
}