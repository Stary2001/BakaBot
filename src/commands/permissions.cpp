#include <string>
#include "irc/ircbot.h"
#include "command.h"
#include "events.h"

bool check_int(Bot *bot, User *u, std::string target, std::string realm, std::string perm)
{
	std::shared_ptr<ConfigNode> v = bot->config->get("permissions."+realm+"." + perm);

	if(v->is("list"))
	{
		for(auto a : v->as_list())
		{
			std::string b = a->to_string();
			if(b == u->account)
			{
				return true;
			}
			else if(b.substr(0, 6) == "group/")
			{
				b = b.substr(6);

				if(b == "special/all")
				{
					return true;
				}
				else if(b == "special/none")
				{
					return false;
				}
				else if(bot->type == "irc")
				{
					if(b == "special/ops")
					{
						if(target[0] == '#')
						{
							IRCChannel *c = ((IRCBot*)bot)->conn->get_channel(target);
							IRCChannelUserData *dat = (IRCChannelUserData *)c->users[u->nick]->data;

							if(dat->modes['o'])
							{
								return true;
							}
						}
					}
					else if(b == "special/voice")
					{
						if(target[0] == '#')
						{
							IRCChannel *c = ((IRCBot*)bot)->conn->get_channel(target);
							IRCChannelUserData *dat = (IRCChannelUserData *)c->users[u->nick]->data;

							if(dat->modes['v'])
							{
								return true;
							}
						}
					}
				}

				std::shared_ptr<ConfigNode> v2 = bot->config->get("groups." + b);
				if (v2->is("list"))
				{
					for(auto c : v2->as_list())
					{
						if(c->to_string() == u->account)
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

bool check_permissions(Bot *bot, User *u, std::string target, std::string perm)
{
	if(u->account == "*")
	{
		return false;
	}

	return !check_int(bot, u, target, "deny", perm) && check_int(bot, u, target, "allow", perm); 
}