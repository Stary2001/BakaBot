#pragma once
#include "connection.h"
#include "lineconnection.h"
#include "user.h"
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

	bool syncing;
};

class IRCConnection : public LineConnection
{
public:
	IRCConnection(EventSink *e, std::string host, unsigned short port);
	void send_line(std::string line);
	void send_privmsg(std::string nick, std::string msg);
	void send_notice(std::string nick, std::string msg);
	void nick(std::string nick);
	void join(std::string chan);
	
	std::string antiping(std::string c, std::string msg);

	std::string current_nick;

	Channel &get_channel(std::string n);

protected:
	virtual void handle_line(std::string line);

private:
	IRCServerState irc_server;

	EventSink *sink;

	std::map<std::string, Channel> channels;
	std::vector<std::string> joined_channels;

	std::map<std::string, User*> global_users;

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
	bool cb_who(Event *e);
	bool cb_end_who(Event *e);

	User* get_user(std::string name);
};
