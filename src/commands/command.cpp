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
			bot->conn->send_privmsg(ev->target, curr->pop()->to_string());
		}

		delete i;
	}
	catch(CommandNotFoundException e)
	{
		bot->conn->send_privmsg(ev->target, "Command '" + e.command + "' not found!");
	}
}

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
				v.push_back(b);
			}
			else
			{
				curr->in.push_back(new StringData(a.second));
			}
		}
		else if(a.first == type_delim)
		{
			cmd = true;

			curr->sender = ev->sender;
			curr->target = ev->target;

			curr->next = new CommandInfo();
			curr = curr->next;
		}
	}

	curr->sender = ev->sender;
	curr->target = ev->target;

	curr->next = new CommandInfo();
	curr->next->sender = ev->sender;
	curr->next->target = ev->target;

	return std::make_tuple(info, v);
}

CommandBase *Command::get_ptr(std::string n)
{
	std::cout << "Trying to get command " << n << std::endl;
	return NULL;
}