#include "command.h"
#include "events.h"
#include <iostream>

void Command::run(Bot *bot, IRCMessageEvent *ev)
{
	auto t = Command::parse(bot, ev);

	CommandInfo *i = std::get<0>(t);
	std::vector<CommandBase*> &v = std::get<1>(t);
	
	for(auto cmd: v)
	{
		cmd->run(bot, i);
		if(i != NULL)
		{
			i = i -> next;
		}
	}
}

std::tuple<CommandInfo*, std::vector<CommandBase*>> Command::parse(Bot *bot, IRCMessageEvent *ev)
{
	CommandInfo *info = new CommandInfo();
	CommandInfo *curr = info;

	std::vector<CommandBase*> v;

	std::string &s = ev->message;

	const char *cs = s.c_str();

	bool parsing_name = true;
	bool parsing_args = false;

	bool eof = false;

	size_t j = 0;
	size_t i = 0;

	for(; i < s.length() + 1; i++)
	{
		if(i == s.length()) { eof = true; }

		char c;
		if(!eof)
		{
			c = cs[i];
		}
		else
		{
			c = ' ';
		}

		switch(c)
		{
			case '|':
				// break? :D
				parsing_args = false;
				parsing_name = true;
				j = i + 1;
			break;

			case ' ':
			case '\t': // in case..?
				if(parsing_name)
				{
					CommandBase *b = bot->get_command(s.substr(j, i-j));
					if(b == NULL)
					{
						//APPLY ERRRORORRRRORORO HERE
						throw std::exception();
					}
					v.push_back(b);

					curr->sender = ev->sender;
					curr->target = ev->target;

					curr->next = new CommandInfo();
					curr = curr->next;
				}
				else if(parsing_args)
				{
					curr->in.push_back(new StringData(s.substr()));
				}

				j = i + 1;
			break;
		}
	}

	return std::make_tuple(info, v);
}

CommandBase *Command::get_ptr(std::string n)
{
	std::cout << "Trying to get command " << n << std::endl;
	return NULL;
}