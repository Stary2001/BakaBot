#include "bot.h"
#include "logger.h"
#include "config.h"

int main(int argc, char** argv)
{
	Logger::instance = new StderrLogger();
	ConnectionDispatcher d;
	
	Config *c = Config::load("config.json");
	c->set("server.host", ConfigValue("irc.esper.net"));
	c->set("server.port", ConfigValue(6667));
	c->set("server.nick", ConfigValue("BakaBot"));
	c->set("server.username", ConfigValue("baka"));
	c->set("server.realname", ConfigValue("Powered By Cirno Technology"));
	c->set("prefix", ConfigValue("@"));
	c->set("server.nickserv.username", ConfigValue("BakaBot"));
	c->set("server.nickserv.password", ConfigValue("h8rOuSTM316VHvAO"));

	ConfigValue v;
	v.type = NodeType::List;
	v.list.push_back("group/admins");

	c->set("permissions.load", v);
	c->set("permissions.unload", v);
	c->set("permissions.perm", v);
	c->set("permissions.group", v);

	ConfigValue vv;
	vv.type = NodeType::List;
	vv.list.push_back("Stary2001");

	c->set("groups.admins", vv);

	Bot bot(c);
	bot.connect(&d);
	
	while(true)
	{
		d.handle();
	}

	return 0;
}
