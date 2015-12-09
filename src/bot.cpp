#include "bot.h"
#include <iostream>

void Bot::event_thread_func()
{
	while(true)
	{
		handle_event();
	}
}

Bot::Bot() : conn(NULL), event_thread(std::bind(&Bot::event_thread_func, this))
{}

Bot::Bot(BotConfig b) : conn(NULL), config(b), event_thread(std::bind(&Bot::event_thread_func, this))
{}

void Bot::init_plugins()
{
	// Plugin::register_plugin(new SedPlugin()); // todo: dynamic plugins

	for(Plugin *p : active_plugins)
	{
		p->init(this);
	}
}

bool Bot::print(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);

	std::cout << "<" << ev->sender.nick << "> " << ev->message << std::endl;
	return false;
}

bool Bot::end_of_motd(Event *e)
{
	IRCConnectedEvent *ev = reinterpret_cast<IRCConnectedEvent*>(e);

	if(config.nickserv_username != "" && config.nickserv_password != "")
	{
		conn->send_privmsg("NickServ", "identify " + config.nickserv_username + " " + config.nickserv_password);
	}
	return false;
}

bool Bot::cb_invite(Event *e)
{
	IRCInviteEvent *ev = reinterpret_cast<IRCInviteEvent*>(e);
	if(ev->target == current_nick)
	{
		conn->join(ev->channel);
	}
	return false;
}

void Bot::connect(ConnectionDispatcher *d)
{
	using namespace std::placeholders;
	conn = new IRCConnection(this, config.server, config.server_port);

	load_plugin("./sed.so");

	init_plugins();

	add_handler("irc/message", std::bind(&Bot::print, this, _1));
	add_handler("irc/invite", std::bind(&Bot::cb_invite, this, _1));
	add_handler("irc/connected", std::bind(&Bot::end_of_motd, this, _1));

	d->add(conn);
	state = IRCState::CONNECTED;
	if(config.server_password != "")
	{
		conn->send_line("PASS " + config.server_password);
		state = IRCState::PASS;
	}
	else
	{
		conn->send_line("NICK " + config.nick);
		current_nick = config.nick;
		conn->send_line("USER " + config.username + " * * :" + config.realname);
		state = IRCState::USER;
	}
}