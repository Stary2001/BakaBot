#pragma once
#include "connection.h"
#include "lineconnection.h"
#include "user.h"
#include "export.h"
#include <vector>
#include <set>

#include <map>
#include <functional>

class EventSink;
class Event;

enum IRCState
{
	CONNECTED,
	CAP,
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

	bool supports_cap;
	std::set<std::string> caps;
	std::set<std::string> enabled_caps;
	std::set<std::string> requested_caps;

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

	bool supports_whox;
	bool supports_ns_status;
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

PLUGINCLASS Channel
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

PLUGINCLASS IRCConnection : public LineConnection
{
public:
	IRCConnection(EventSink *e, std::string host, unsigned short port, bool ssl = false);
	~IRCConnection();

	void send_line(std::string line);
	void send_privmsg(std::string nick, std::string msg);
	void send_notice(std::string nick, std::string msg);
	void nick(std::string nick);
	void join(std::string chan);

	void quit(std::string msg);

	
	std::string antiping(std::string c, std::string msg);

	std::string current_nick;

	Channel &get_channel(std::string n);
	void do_cap(std::vector<std::string> &caps);
	bool has_cap(std::string name);

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
	bool cb_rewrite_message(Event *e);
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
	
	bool cb_unk_command(Event *e);
	bool cb_cap(Event *e);
	bool cb_cap_done(Event *e);

	bool cb_ns_notice(Event *e);

	void end_cap();
	void sync_nickserv(User *u);

	User* get_user(std::string name);
};
