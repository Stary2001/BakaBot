#include "ircconnection.h"

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
};

class Bot
{
public:
	Bot();
	Bot(BotConfig c);
	void connect(ConnectionDispatcher *d);

private:
	IRCConnection *conn;
	BotConfig config;
	IRCState state;
	std::string current_nick;

	bool print(User &sender, std::vector<std::string> &params);
	bool end_of_motd(User &sender, std::vector<std::string> &params);
	bool cb_invite(User &sender, std::vector<std::string> &params);
};
