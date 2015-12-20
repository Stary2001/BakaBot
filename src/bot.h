#pragma once
#include "plugin.h"
#include "events.h"
#include "config.h"
#include <thread>

class Bot : public PluginHost, public EventSink
{
public:
	Bot();
	Bot(Config *c);
	void connect(ConnectionDispatcher *d);

	IRCConnection *conn;
	Config *config;
private:

	//BotConfig config;
	IRCState state;

	void init_plugins();

	bool print(Event *e);
	bool end_of_motd(Event *e);
	bool cb_invite(Event *e);
	bool cb_command(Event *e);

	bool check_permissions(User *u, std::string command);

	void event_thread_func();
	std::thread event_thread;
};
