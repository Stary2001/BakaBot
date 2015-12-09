#include "bot.h"

int main(int argc, char** argv)
{
	ConnectionDispatcher d;

	BotConfig config;
	config.server = "irc.esper.net";
	config.server_port = 6667;
	config.nick = "BakaBot";
	config.username = "BakaBot";
	config.realname = "ur all bakaaaa~";

	config.nickserv_username = "BakaBot";
	config.nickserv_password = "mPQTxLoWVvDaItwVL8KU";

	config.command_prefix = "@";
	
	Bot bot(config);
	bot.connect(&d);
	
	while(true)
	{
		d.handle();
	}

	return 0;
}
