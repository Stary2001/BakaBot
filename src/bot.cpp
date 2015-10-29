#include "bot.h"
#include <iostream>

Bot::Bot() : conn(NULL)
{}

Bot::Bot(BotConfig b) : conn(NULL), config(b)
{}

bool Bot::print(User &sender, std::vector<std::string> &params)
{
	std::cout << "<" << sender.nick << "> " << params[1] << std::endl;
	return false;
}

bool Bot::end_of_motd(User &sender, std::vector<std::string> &params)
{
	if(config.nickserv_username != "" && config.nickserv_password != "")
	{
		conn->send_privmsg("NickServ", "identify " + config.nickserv_username + " " + config.nickserv_password);
	}
	return false;
}

bool Bot::cb_invite(User &sender, std::vector<std::string> &params)
{
	if(params[0] == current_nick)
	{
		conn->join(params[1]);
	}
	return false;
}

void Bot::connect(ConnectionDispatcher *d)
{
	using namespace std::placeholders;
	conn = new IRCConnection(config.server, config.server_port);

	//conn->add_callback("notice", std::bind(&Bot::print, this, _1, _2));
	conn->add_callback("invite", std::bind(&Bot::cb_invite, this, _1, _2));
	conn->add_callback("376", std::bind(&Bot::end_of_motd, this, _1, _2));

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

