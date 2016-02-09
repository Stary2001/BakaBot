#include <curl/curl.h>
#include "bot.h"
#include "logger.h"
#include "config.h"
#include "util.h"
#include <gnutls/gnutls.h>

int main(int argc, char** argv)
{
	gnutls_global_init();
	curl_global_init(CURL_GLOBAL_ALL);

	Logger::instance = new StderrLogger();
	ConnectionDispatcher d;
	std::vector<Bot*> bots;


	if (util::fs::exists("./bot.conf")) // move old bot.conf to networks/default.conf
	{
		if (!util::fs::is_directory("./networks"))
		{
			util::fs::mkdir("./networks");
		}

		util::fs::rename("./bot.conf", "./networks/default.conf");
	}

	Config *c = NULL;

	auto networks = util::fs::listdir("./networks");
	for (auto name : networks)
	{
		c = Config::load("./networks/" + name);
		Config *l = Config::load("./lang/" + c->get("locale.language")->as_string() + ".conf");
		Bot *b = new Bot(c, l);
		bots.push_back(b);
		b->connect(&d);
	}
	
	while(true)
	{
		d.handle();
	}

	curl_global_cleanup();
	gnutls_global_deinit();

	return 0;
}
