#include <sys/epoll.h>
#include "ircconnection.h"
#include <cstring>
#include <iostream>
#include <algorithm> // for tolower
#include "util.h"

bool cb_null(User &sender, std::vector<std::string> &params)
{
	return false;
}

bool IRCConnection::cb_myinfo(User &sender, std::vector<std::string> &params)
{
	irc_server.name = params[1];
	irc_server.version = params[2];
	std::copy(params[2].begin(), params[2].end(), std::back_inserter(irc_server.usermodes));
	//std::copy(params[3].begin(), params[3].end(), std::back_inserter(server_chanmodes)); // we get these in 005
	return true;
}

bool IRCConnection::cb_isupport(User &sender, std::vector<std::string> &params)
{
	std::vector<std::string>::iterator it = params.begin() + 1;
	for(; it != (params.end()-1) ; ++it)
	{
		std::string s = *it;
		int pos = s.find('=');
		std::string k = s.substr(0, pos);
		if(k == "MAP")
		{
			irc_server.supports_map = true;
			continue;
		}
		std::string v = s.substr(pos+1, s.length() - pos - 1);

		if(k == "PREFIX")
		{
			std::cout << "prefix " << v << std::endl;
		}
		else if(k == "CHANTYPES")
		{
			std::copy(v.begin(), v.end(), std::back_inserter(irc_server.chantypes));
		}
		else if(k == "CHANMODES")
		{
			std::vector<std::string> a = util::split(v, ',');
			std::copy(a[0].begin(), a[0].end(), std::back_inserter(irc_server.list_chanmodes));
			std::copy(a[1].begin(), a[1].end(), std::back_inserter(irc_server.alwaysparam_chanmodes));
			std::copy(a[2].begin(), a[2].end(), std::back_inserter(irc_server.setparam_chanmodes));
			std::copy(a[3].begin(), a[3].end(), std::back_inserter(irc_server.flag_chanmodes));
		}
		else if(k == "NETWORK")
		{
			irc_server.name = v;
		}
		else if(k == "AWAYLEN")
		{
			irc_server.awaylen = stoi(v);
		}
		else if(k == "NICKLEN")
		{
			irc_server.nicklen = stoi(v);
		}
		else if(k == "KICKLEN")
		{
			irc_server.kicklen = stoi(v);
		}
		else if(k == "MODES")
		{
			irc_server.modes_per_line = stoi(v);
		}
		else if(k == "MAXBANS")
		{
			irc_server.maxbans = stoi(v);
		}
	}
	return true;
}

bool IRCConnection::cb_yourid(User &sender, std::vector<std::string> &params)
{
	std::cout << "id " << params[1] << std::endl;
	return true;
}

bool IRCConnection::cb_print(User &sender, std::vector<std::string> &params)
{
	std::cout << params[1] << std::endl;
	return true;
}

bool IRCConnection::cb_ctcp(User &sender, std::vector<std::string> &params)
{
	std::string s = params[1];

	if(s.at(0) == '\x01' && s.at(s.length() - 1) == '\x01')
	{
		s = s.substr(1, s.length() - 2);
		std::cout << "got ctcp " << s << std::endl;
		if(s == "VERSION")
		{
			send_notice(sender.nick, "\x01VERSION BakaBot v0.0000000000000001\x01");
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool IRCConnection::cb_ping(User &sender, std::vector<std::string> &params)
{
	send_line("PONG " + params[0]);
	return true;
}

bool IRCConnection::cb_mode(User &sender, std::vector<std::string> &params)
{
	// todo: sync modes.
	return false;
}

bool IRCConnection::cb_join(User &sender, std::vector<std::string> &params)
{
	// todo: sync joins.
	return false;
}

bool IRCConnection::cb_topic(User &sender, std::vector<std::string> &params)
{
	// todo: sync topics.
	return false;
}

bool IRCConnection::cb_topic_change_time(User &sender, std::vector<std::string> &params)
{
	// todo: sync topics.
	return false;
}

bool IRCConnection::cb_names(User &sender, std::vector<std::string> &params)
{
	// todo: sync names.
	return false;
}

IRCConnection::IRCConnection(std::string host, unsigned short port) : Connection(host, port)
{
	using namespace std::placeholders;
	scratch = (char*) malloc(SCRATCH_LENGTH);
	scratch_off = 0;

/*	add_callback("001", std::bind(&IRCConnection::cb_print, this, _1, _2));
	add_callback("002", std::bind(&IRCConnection::cb_print, this, _1, _2));
	add_callback("003", std::bind(&IRCConnection::cb_print, this, _1, _2));*/
	add_callback("001", cb_null);
	add_callback("002", cb_null);
	add_callback("003", cb_null);

	add_callback("004", std::bind(&IRCConnection::cb_myinfo, this, _1, _2));
	add_callback("005", std::bind(&IRCConnection::cb_isupport, this, _1, _2));
	add_callback("042", std::bind(&IRCConnection::cb_yourid, this, _1, _2));

	/*add_callback("250", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_STATSCONN
	add_callback("251", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSERCLIENT
	add_callback("252", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSEROP
	add_callback("253", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSERUNKNOWN
	add_callback("254", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSERCHANNELS
	add_callback("255", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSERME
	add_callback("265", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LOCALUSERS
	add_callback("266", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_GLOBALUSERS */

	add_callback("250", cb_null);
	add_callback("251", cb_null);
	add_callback("252", cb_null);
	add_callback("253", cb_null);
	add_callback("254", cb_null);
	add_callback("255", cb_null);
	add_callback("265", cb_null);
	add_callback("266", cb_null);

	/*add_callback("372", std::bind(&IRCConnection::cb_print, this, _1, _2));
	add_callback("375", std::bind(&IRCConnection::cb_print, this, _1, _2));*/

	add_callback("372", cb_null);
	add_callback("375", cb_null);

	add_callback("privmsg", std::bind(&IRCConnection::cb_ctcp, this, _1, _2));
	add_callback("ping", std::bind(&IRCConnection::cb_ping, this, _1, _2));

	//sync callbacks

	add_callback("mode", std::bind(&IRCConnection::cb_mode, this, _1, _2));
	add_callback("join", std::bind(&IRCConnection::cb_join, this, _1, _2));
	add_callback("332", std::bind(&IRCConnection::cb_topic, this, _1, _2));
	add_callback("333", std::bind(&IRCConnection::cb_topic_change_time, this, _1, _2));
	add_callback("353", std::bind(&IRCConnection::cb_topic_change_time, this, _1, _2));
	add_callback("366", std::bind(&IRCConnection::cb_topic_change_time, this, _1, _2));

	add_callback("notice", cb_null);
}

void IRCConnection::send_line(std::string line)
{
	line += "\r\n";
	write(line.c_str(), line.length());
}

void IRCConnection::send_privmsg(std::string nick, std::string msg)
{
	send_line("PRIVMSG " + nick + " :" + msg);
}

void IRCConnection::send_notice(std::string nick, std::string msg)
{
	send_line("NOTICE " + nick + " :" + msg);
}

void IRCConnection::join(std::string chan)
{
	send_line("JOIN :" + chan);
}

void IRCConnection::handle(uint32_t events)
{
	if(events & EPOLLRDHUP) // if server disconnected, die. f.
	{
		dispatcher->remove(this);
		delete this;
		return;
	}

	if(events & EPOLLIN)
	{
		int r = read(scratch + scratch_len, SCRATCH_LENGTH - scratch_len);
		if(r == 0)
		{
			// ??
			std::cerr << "read returned 0?" << std::endl;
		}
		scratch_len += r;

		char *line_end = NULL;
		bool found = false;
		int len = 0;
		char *last_line_end = NULL;

		while(true)
		{
			last_line_end = line_end;
			line_end = (char*)memmem(scratch + scratch_off, SCRATCH_LENGTH - scratch_off, "\r\n", 2);
			if(line_end == NULL)
			{
				if(found)
				{
					len = scratch_len - (last_line_end+2 - scratch);
					memmove(scratch, last_line_end + 2, len);
					memset(scratch + len, 0, SCRATCH_LENGTH - len);
					scratch_off = 0;
					scratch_len = len;
				}
				break;
			}

			found = true;
			len = (line_end - scratch) - scratch_off;
			std::string line(scratch + scratch_off, len);
			handle_line(line);

			len += 2;
			scratch_off += len;
		}
		
	}
}

void IRCConnection::parse_line(std::string line_s, std::string& sender, std::string& command, std::vector<std::string>& params)
{
	char *line = (char*) line_s.c_str(); // thanks, c++11

    int len = line_s.length();
    int i = 0;
    bool read_sender = false;
    bool reading_params = false;
    int end = -1;

    for(; i < len; i++)
    {
        if(line[i] == ' ')
        {
            line[i] = 0;
            if(reading_params)
            {
                params.push_back(line + end + 1);
            }
            else
            {
                if(line[0] == ':' && !read_sender)
                {
                    sender = line + 1;
                    read_sender = true;
                }
                else
                {
                    command = line + end + 1;
                    reading_params = true;
                }
            }
            end = i;
            line[i] = ' ';
        }
        else if(line[i] == ':' && i != 0 && (line[0] != ':' || read_sender))
        {
            params.push_back(line + i + 1);
            break;
        }
    }
}

User IRCConnection::parse_hostmask(std::string hostmask)
{
	User u;
	int bang = hostmask.find('!');
	int at = hostmask.find('@');
	if(bang == std::string::npos || at == std::string::npos)
	{
		u.nick = hostmask;
		u.host = hostmask;
		return u;
	}

	u.nick = hostmask.substr(0, bang);
	u.ident = hostmask.substr(bang+1, at - bang);
	u.host = hostmask.substr(at, hostmask.length() - at);
	return u;
}

void IRCConnection::handle_line(std::string line)
{
	std::string sender;
	std::string command;
	std::vector<std::string> params;

	parse_line(line, sender, command, params);

	User u = parse_hostmask(sender);

	std::transform(command.begin(), command.end(), command.begin(), tolower);

	if(callbacks.find(command) == callbacks.end())
	{
		std::cerr << "no handlers for " << command << std::endl;
	}
	else
	{
		std::vector<IRCCallback>::iterator it = callbacks[command].begin();
		for(; it != callbacks[command].end(); ++it)
		{
			if((*it)(u, params))
			{
				break;
			}
		}
	}
}

void IRCConnection::add_callback(std::string type, IRCCallback c)
{
	std::transform(type.begin(), type.end(), type.begin(), tolower);

	if(callbacks.find(type) == callbacks.end())
	{
		callbacks[type] = std::vector<IRCCallback>();
	}

	callbacks[type].push_back(c);
}