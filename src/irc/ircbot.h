#pragma once
#include "bot.h"
#include <thread>

class CommandBase;

class IRCBot : public Bot
{
public:
	IRCBot();
	IRCBot(Config *c, Config *l);
	~IRCBot();

	IRCConnection *conn;

	virtual void __connect(ConnectionDispatcher *d, std::string server, short port, bool use_ssl);
	virtual void join(std::string channel);
    virtual void leave(std::string channel);
    virtual void quit(std::string message);
    virtual void send_message(std::string target, std::string message);

private:
	IRCState state;

	bool print(Event *e);
	bool end_of_motd(Event *e);
	bool cb_cap_done(Event *e);
	bool cb_sasl(Event *e);
	bool cb_invite(Event *e);

	void end_sasl();
	bool cb_sasl_done(Event *e);

	void event_thread_func();
	std::thread event_thread;

	bool nickserv_done;
};
