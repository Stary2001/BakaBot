#pragma once
#include "connection.h"
#include <vector>
#include <set>

#include <map>
#include <functional>

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

enum UserMode
{
	NONE = 0,
	OP = 1,
	HOP = 2,
	VOICE = 4
};

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

	std::set<char> usermodes;
	std::set<char> list_chanmodes;
	std::set<char> alwaysparam_chanmodes;
	std::set<char> setparam_chanmodes;
	std::set<char> flag_chanmodes;

	std::set<char> chantypes;

	std::map<char, char> prefixes;
	std::map<char, char> prefix_modes;
};

struct User
{
	std::string nick;
	std::string ident;
	std::string host;
};

enum ModeType
{
	FLAG,
	LIST,
	VALUE
};

class Mode
{
public:
	Mode();
	Mode(char c);
	Mode(char c, bool flag);
	Mode(char c, std::string v);

	ModeType type;
	char mode;
	bool active;
	std::string value;
	std::vector<std::string> list;
};

class ChannelUser
{
public:
	ChannelUser();
	ChannelUser(User *u);
	User *user;
	std::map<char, bool> modes;
};

class Channel
{
public:
	Channel();
	Channel(std::string n);
	std::string name;
	std::map<std::string, ChannelUser> users;
	std::string topic;
	std::string topic_changed_by;
	unsigned int topic_time;
	std::map<char, Mode> modes;
	Mode& get_mode(char c, ModeType m);
};

#define SCRATCH_LENGTH 1024

class IRCConnection : public Connection
{
public:
	IRCConnection(EventSink *e, std::string host, unsigned short port);
	virtual ~IRCConnection();
	void send_line(std::string line);
	void send_privmsg(std::string nick, std::string msg);
	void send_notice(std::string nick, std::string msg);
	void nick(std::string nick);
	void join(std::string chan);
	
	std::string current_nick;

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

	std::map<std::string, Channel> channels;
	std::vector<std::string> joined_channels;

	std::map<std::string, User*> global_users;

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
	bool cb_part(Event *e);
	bool cb_topic(Event *e);
	bool cb_topic_change(Event *e);
	bool cb_no_topic(Event *e);
	bool cb_topic_change_time(Event *e);
	bool cb_names(Event *e);
	bool cb_end_of_names(Event *e);

	User* get_user(std::string name);
	User* get_user(User u);
};
