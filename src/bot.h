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
	~Bot();

	void connect(ConnectionDispatcher *d);
	CommandBase *get_command(std::string n);
	void register_command(std::string n, CommandBase *b);
	void remove_command(std::string n);

	bool should_stop;

	Config *config;
    Config *locale;

    virtual void __connect(ConnectionDispatcher *d, std::string server, short port, bool use_ssl) = 0;
    virtual void join(std::string channel) = 0;
    virtual void leave(std::string channel) = 0;
    virtual void quit(std::string message) = 0;
    virtual void send_message(std::string target, std::string message) = 0;

	std::string type;

	bool cb_command(Event *e);
protected:
	void init_plugins();
	void destroy_plugins();

	void end_sasl();
	bool cb_sasl_done(Event *e);

	void event_thread_func();
	std::thread event_thread;
	std::map<std::string, CommandBase*> commands;
	Plugin *open_lua(std::string filename);
};
