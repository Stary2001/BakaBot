#include "bot.h"
#include "logger.h"
#include "config.h"

int main(int argc, char** argv)
{
	Logger::instance = new StderrLogger();
	ConnectionDispatcher d;
	
	Config *c = Config::load("bot.conf");

	Bot bot(c);
	bot.connect(&d);
	
	while(true)
	{
		d.handle();
	}

	return 0;
}
