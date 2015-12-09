#pragma once
#include "connection.h"
#include <vector>
#include <map>

class EventSink;
class Event;

enum IRCState
{
	CONNECTED,
	PASS,
	NICK,
	NICK_TAKEN,
	USER,
	ONLINE
};

//AWAYLEN=200 CALLERID=g CASEMAPPING=rfc1459 CHANMODES=IXZbew,k,FHJLdfjl,ABCDKMNOPQRTcimnprstuz CHANNELLEN=64 CHANTYPES=# CHARSET=ascii ELIST=MU EXCEPTS=e EXTBAN=,ABCNOQRTUcz FNC INVEX=I KICKLEN=255

struct IRCServerState
{
	std::string name;
	std::string version;

	bool supports_map;
	int awaylen;
	int kicklen;
	int nicklen;
	int topiclen;
	int modes_per_line;
	int maxbans;

	char invex;
	char banex;

	std::vector<char> usermodes;
	std::vector<char> list_chanmodes;
	std::vector<char> alwaysparam_chanmodes;
	std::vector<char> setparam_chanmodes;
	std::vector<char> flag_chanmodes;

	std::vector<char> chantypes;
};

struct User
{
	std::string nick;
	std::string ident;
	std::string host;
};

#define SCRATCH_LENGTH 1024

class IRCConnection : public Connection
{
public:
	IRCConnection(EventSink *e, std::string host, unsigned short port);
	void send_line(std::string line);
	void send_privmsg(std::string nick, std::string msg);
	void send_notice(std::string nick, std::string msg);
	void join(std::string chan);

protected:
	virtual void handle(uint32_t events);

private:
	bool reading_line;
	std::string buffer;
	char *scratch;
	uint32_t scratch_off;
	uint32_t scratch_len;

	IRCServerState irc_server;

	EventSink *sink;

	void handle_line(std::string line);
	void parse_line(std::string line, std::string& sender, std::string& command, std::vector<std::string>& params);
	User parse_hostmask(std::string hostmask);

	bool cb_myinfo(Event *e);
	bool cb_isupport(Event *e);
	bool cb_yourid(Event *e);
	bool cb_print(Event *e);
	bool cb_ctcp(Event *e);
	bool cb_ping(Event *e);
	bool cb_end_of_motd(Event *e);
	bool cb_rewrite_privmsg(Event *e);
	bool cb_rewrite_invite(Event *e);

	// sync callbacks
	bool cb_mode(Event *e);
	bool cb_join(Event *e);
	bool cb_topic(Event *e);
	bool cb_topic_change_time(Event *e);
	bool cb_names(Event *e);
	bool cb_end_names(Event *e);
};
