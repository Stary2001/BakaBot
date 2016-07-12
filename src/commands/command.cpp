#include "ircbot.h"
#include "command.h"
#include "events.h"
#include <iostream>

void Command::run(Bot *bot, IRCMessageEvent *ev)
{
	try
	{
		auto t = Command::parse(bot, ev);

		CommandInfo *i = std::get<0>(t);
		std::vector<CommandBase*> &v = std::get<1>(t);

		CommandInfo* curr = i;

		for(auto cmd: v)
		{
			cmd->run(bot, curr);
			if(curr != NULL)
			{
				curr = curr-> next;
			}
		}

		while(curr->in.size() != 0)
		{
			// todo: clean
			std::string s = curr->pop()->to_string();
			s = util::replace(s, "\n", " / ");
			bot->send_message(ev->target, s);
		}

		delete i;
	}
	catch(CommandNotFoundException e)
	{
		bot->send_message(ev->target, "Command '" + e.command + "' not found!");
	}
	catch(PermissionDeniedException e)
	{
		bot->send_message(ev->target, "Permission denied for '" + e.command + "'!");
	}
	catch(CommandErrorException e)
	{
		bot->send_message(ev->target, "Command '" + e.command + "' threw error '" + e.err + "'!");
	}
}

bool check_permissions(Bot *bot, User *u, std::string target, std::string command);

std::tuple<CommandInfo*, std::vector<CommandBase*>> Command::parse(Bot *bot, IRCMessageEvent *ev)
{
	CommandInfo *info = new CommandInfo();
	CommandInfo *curr = info;

	std::vector<CommandBase*> v;

	std::string &s = ev->message;

	const int type_str = 0;
	const int type_delim = 1;

	std::vector<std::pair<int, std::string>> tokens;

	std::string tmp;

	size_t i = 0;

	bool quote = false;
	bool ignore = false;

	for(; i < s.length() ; i++)
	{
		char c = s[i];

		switch(c)
		{
			case '\'':
			case '\"':
				if(quote)
				{
					tokens.push_back(std::make_pair(type_str, tmp));
					tmp.clear();
					ignore = true;
					quote = false;
				}
				else
				{
					quote = true;
				}
			break;

			case '|':
				tokens.push_back(std::make_pair(type_delim, ""));
				tmp.clear();
				ignore = true;
			break;

			case ' ':
				if(!ignore)
				{
					if(!quote && tmp != "")
					{
						tokens.push_back(std::make_pair(type_str, tmp));
						tmp.clear();
						ignore = true;
					}
					else
					{
						tmp += c;
					}
				}
			break;

			default:
				if(ignore) { ignore = false; }
				tmp += c;
			break;
		}
	}

	if(tmp != "")
	{
		tokens.push_back(std::make_pair(type_str, tmp));
	}

	bool cmd = true;

	CommandBase *curr_cmd = nullptr;

	for(auto a: tokens)
	{
		if(a.first == type_str)
		{
			if(cmd)
			{
				cmd = false;

				CommandBase *b = bot->get_command(a.second);
				if(b == NULL)
				{
					delete info;
					throw CommandNotFoundException(a.second);
				}

				if(!check_permissions(bot, ev->sender, ev->target, a.second))
				{
					throw PermissionDeniedException(a.second);
				}

				v.push_back(b);
				curr_cmd = b;
			}
			else
			{
				if((int)curr_cmd->flags & (int)CommandFlags::OneParam)
				{
					if(curr->in.size() == 0)
					{
						curr->in.push_back(new StringData(a.second));	
					}
					else
					{
						((StringData*)curr->in.front())->str += " " + a.second;
					}
				}
				else
				{
					curr->in.push_back(new StringData(a.second));
				}
			}
		}
		else if(a.first == type_delim)
		{
			cmd = true;

			curr->sender = ev->sender;
			curr->target = ev->target;
			curr->cmd = v.back();

			curr->next = new CommandInfo();
			curr = curr->next;
		}
	}

	curr->cmd = v.back();
	curr->sender = ev->sender;
	curr->target = ev->target;

	curr->next = new CommandInfo();
	curr->next->sender = ev->sender;
	curr->next->target = ev->target;

	return std::make_tuple(info, v);
}


bool check_permissions(Bot *bot, User *u, std::string target, std::string command)
{
	if(u->account == "*")
	{
		return false;
	}

	std::shared_ptr<ConfigNode> v = bot->config->get("permissions." + command);

	if (v->is("list"))
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

CommandBase *Command::get_ptr(std::string n)
{
	std::cout << "Trying to get command " << n << std::endl;
	return NULL;
}
