#include "connection.h"
#include <vector>
#include <map>
#include <functional>

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

//typedef void (*IRCCallback)(std::string&, std::vector<std::string> &);
typedef std::function<bool(User&, std::vector<std::string> &)> IRCCallback;

class IRCConnection : public Connection
{
public:
	IRCConnection(std::string host, unsigned short port);
	void send_line(std::string line);
	void send_privmsg(std::string nick, std::string msg);
	void send_notice(std::string nick, std::string msg);
	void join(std::string chan);

	void add_callback(std::string type, IRCCallback c);

protected:
	virtual void handle(uint32_t events);

private:
	bool reading_line;
	std::string buffer;
	char *scratch;
	uint32_t scratch_off;
	uint32_t scratch_len;
	std::map<std::string, std::vector<IRCCallback>> callbacks;

	IRCServerState irc_server;

	void handle_line(std::string line);
	void parse_line(std::string line, std::string& sender, std::string& command, std::vector<std::string>& params);
	User parse_hostmask(std::string hostmask);

	bool cb_myinfo(User &sender, std::vector<std::string> &params);
	bool cb_isupport(User &sender, std::vector<std::string> &params);
	bool cb_yourid(User &sender, std::vector<std::string> &params);
	bool cb_print(User &sender, std::vector<std::string> &params);
	bool cb_ctcp(User &sender, std::vector<std::string> &params);
	bool cb_ping(User &sender, std::vector<std::string> &params);
	bool cb_end_of_motd(User &sender, std::vector<std::string> &params);

	// sync callbacks
	bool cb_mode(User &sender, std::vector<std::string> &params);
	bool cb_join(User &sender, std::vector<std::string> &params);
	bool cb_topic(User &sender, std::vector<std::string> &params);
	bool cb_topic_change_time(User &sender, std::vector<std::string> &params);
	bool cb_names(User &sender, std::vector<std::string> &params);
	bool cb_end_names(User &sender, std::vector<std::string> &params);
};
