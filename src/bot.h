#pragma once
#include "plugin.h"
#include "events.h"
#include "config.h"
#include <thread>

class CommandBase;

class Bot : public PluginHost, public EventSink
{
public:
	Bot();
	Bot(Config *c, Config *l);
	void connect(ConnectionDispatcher *d);
	CommandBase *get_command(std::string n);

	IRCConnection *conn;
	Config *config;
    Config *locale;
private:

	//BotConfig config;
	IRCState state;

	void init_plugins();

	bool print(Event *e);
	bool end_of_motd(Event *e);
	bool cb_cap_done(Event *e);
	bool cb_sasl(Event *e);
	bool cb_invite(Event *e);
	bool cb_command(Event *e);

	void end_sasl();
	bool cb_sasl_done(Event *e);

	bool check_permissions(User *u, Channel &c, std::string command);

	void event_thread_func();
	std::thread event_thread;

	bool nickserv_done;

	std::map<std::string, CommandBase*> commands;
};
