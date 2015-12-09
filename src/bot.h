#pragma once
#include "plugin.h"
#include "events.h"
#include <thread>

struct BotConfig
{
	std::string server;
	unsigned short server_port;
	std::string server_password;

	std::string username;
	std::string nick;
	std::string ident;
	std::string realname;

	std::string nickserv_username;
	std::string nickserv_password;

	std::string command_prefix;
};

class Bot : public PluginHost, public EventSink
{
public:
	Bot();
	Bot(BotConfig c);
	void connect(ConnectionDispatcher *d);

	IRCConnection *conn;
private:
	BotConfig config;
	IRCState state;
	std::string current_nick;

	void init_plugins();

	bool print(Event *e);
	bool end_of_motd(Event *e);
	bool cb_invite(Event *e);
	bool cb_command(Event *e);

	void event_thread_func();
	std::thread event_thread;
};
