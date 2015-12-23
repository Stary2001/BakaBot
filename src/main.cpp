#include "bot.h"
#include "logger.h"
#include "config.h"

int main(int argc, char** argv)
{
	Logger::instance = new StderrLogger();
	ConnectionDispatcher d;

	Config *c = Config::load("bot.conf");
    Config *l = Config::load("./lang/" + c->get("locale.language")->as_string() + ".conf"); 
	Bot bot(c, l);
	bot.connect(&d);
	
	while(true)
	{
		d.handle();
	}

	return 0;
}
