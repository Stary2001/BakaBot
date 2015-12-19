#include <sys/epoll.h>
#include "ircconnection.h"
#include <cstring>
#include <iostream>
#include "util.h"
#include "events.h"
#include <algorithm> // for tolower

bool cb_null(Event *e)
{
	return false;
}

bool IRCConnection::cb_myinfo(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	irc_server.name = ev->params[1];
	irc_server.version = ev->params[2];
	std::copy(ev->params[3].begin(), ev->params[3].end(), std::inserter(irc_server.usermodes, irc_server.usermodes.begin()));
	//std::copy(params[3].begin(), params[3].end(), std::back_inserter(server_chanmodes)); // we get these in 005
	return true;
}

bool IRCConnection::cb_isupport(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::vector<std::string>::iterator it = ev->params.begin() + 1;
	for(; it != (ev->params.end()-1) ; ++it)
	{
		std::string s = *it;
		unsigned int pos = s.find('=');
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
			v = v.substr(1); // (modes)prefixes
							// ov(@+)
			auto b = util::split(v, ')');
			unsigned int i = 0;
			// 0 = ov, 1 = @+

			for(; i < b[0].length(); i++)
			{
				irc_server.prefixes[b[1][i]] = b[0][i];
				irc_server.prefix_modes[b[0][i]] = b[1][i];
				irc_server.alwaysparam_chanmodes.insert(b[0][i]);
			}
		}
		else if(k == "CHANTYPES")
		{
			std::copy(v.begin(), v.end(), std::inserter(irc_server.chantypes, irc_server.chantypes.begin()));
		}
		else if(k == "CHANMODES")
		{
			std::vector<std::string> a = util::split(v, ',');
			std::copy(a[0].begin(), a[0].end(), std::inserter(irc_server.list_chanmodes, irc_server.list_chanmodes.begin()));
			std::copy(a[1].begin(), a[1].end(), std::inserter(irc_server.alwaysparam_chanmodes, irc_server.alwaysparam_chanmodes.begin()));
			std::copy(a[2].begin(), a[2].end(), std::inserter(irc_server.setparam_chanmodes,irc_server.setparam_chanmodes.begin()));
			std::copy(a[3].begin(), a[3].end(), std::inserter(irc_server.flag_chanmodes,irc_server.flag_chanmodes.begin()));
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

bool IRCConnection::cb_yourid(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::cout << "id " << ev->params[1] << std::endl;
	return true;
}

bool IRCConnection::cb_print(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);
	std::cout << ev->params[1] << std::endl;
	return true;
}

bool IRCConnection::cb_ctcp(Event *e)
{
	IRCMessageEvent *ev = reinterpret_cast<IRCMessageEvent*>(e);
	std::string s = ev->message;

	if(s.at(0) == '\x01' && s.at(s.length() - 1) == '\x01')
	{
		s = s.substr(1, s.length() - 2);
		std::cout << "got ctcp " << s << std::endl;
		if(s == "VERSION")
		{
			send_notice(ev->sender.nick, "\x01VERSION BakaBot v0.0000000000000001\x01");
		}
		return true;
	}
	else
	{
		return false;
	}
}

bool IRCConnection::cb_end_of_motd(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);
	sink->queue_event(new IRCConnectedEvent(ev->sender));
	return true;
}

bool IRCConnection::cb_rewrite_invite(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);
	sink->queue_event(new IRCInviteEvent(ev->sender, ev->params[0], ev->params[1]));
	return true;
}

bool IRCConnection::cb_rewrite_privmsg(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);
	sink->queue_event(new IRCMessageEvent(ev->sender, ev->params[0], ev->params[1]));
	return true;
}

bool IRCConnection::cb_ping(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);
	send_line("PONG " + ev->params[0]);
	return true;
}

bool IRCConnection::cb_mode(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::string chan = ev->params[0];
	if(chan.substr(0, 1) != "#") // TODO: USERMODES
	{
		return true;
	}

	if(channels.find(chan) == channels.end())
	{
		channels[chan] = Channel(chan);
	}

	auto it = ev->params.begin() + 1; // [2]

	bool add = true;

	for(; it != ev->params.end(); it++)
	{
		auto it2 = it;

		size_t i = 0;
		if(it2->substr(0,1) == "+" || it2->substr(0,1) == "-")
		{
			for(i = 0; i < it2->length(); i++)
			{
				char c = (*it2)[i];
				std::set<char> &list = irc_server.list_chanmodes;
				std::set<char> &flag = irc_server.flag_chanmodes;
				std::set<char> &setparam = irc_server.setparam_chanmodes;
				std::set<char> &alwaysparam = irc_server.alwaysparam_chanmodes;

				if(c == '+')
				{
					add = true;
				}
				else if(c == '-')
				{
					add = false;
				}
				else if(list.find(c) != list.end())
				{
					auto param = *(++it);
					Mode& m = channels[chan].get_mode(c, LIST);
					if(add)
					{
						if(std::find(m.list.begin(), m.list.end(), param) == m.list.end())
						{
							m.list.push_back(param);
						}
					}
					else
					{
						std::vector<std::string>::iterator itt;

						if((itt = std::find(m.list.begin(), m.list.end(), param)) != m.list.end())
						{
							m.list.erase(itt);
						}
					}
				}
				else if(setparam.find(c) != setparam.end() || alwaysparam.find(c) != alwaysparam.end()) // doable because alwaysparam is USUALLY just k (also prefixes)
				{
					auto param = *(++it);
					if(irc_server.prefix_modes.find(c) != irc_server.prefix_modes.end())
					{
						if(channels[chan].users.find(param) != channels[chan].users.end())
						{
							if(add)
							{
								channels[chan].users[param].modes[c] = true;
							}
							else
							{
								channels[chan].users[param].modes[c] = false;
							}
						}
					}
					else
					{
						Mode& m = channels[chan].get_mode(c, VALUE);

						if(add)
						{
							m.value = param;
						}
						else
						{
							// remove setting
							if(channels[chan].modes.find(c) != channels[chan].modes.end())
							{
								channels[chan].modes.erase(c);
							}
						}
					}
				}
				else if(flag.find(c) != flag.end())
				{
					auto param = *(++it);
					Mode& m = channels[chan].get_mode(c, FLAG);
					m.active = add;
				}
			}
		}
	}

	return true;
}

bool IRCConnection::cb_join(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::string c = ev->params[0];

	if(channels.find(c) == channels.end())
	{
		channels[c] = Channel(c);
	}

	if(ev->sender.nick == current_nick)
	{
		send_line("WHO " + c + " %cuhnarsf");
		channels[c].syncing	= true;
		joined_channels.push_back(c);
	}

	channels[c].users[ev->sender.nick] = ChannelUser(get_user(ev->sender.nick));
	send_line("WHO " + ev->sender.nick + " %cuhnarsf");

	return false;
}

bool IRCConnection::cb_part(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::string c = ev->params[0];

	if(channels.find(c) == channels.end())
	{
		channels[c] = Channel(c);
	}

	if(ev->sender.nick == current_nick)
	{
		auto it = std::remove(joined_channels.begin(), joined_channels.end(), c);
		if(it != joined_channels.end())
		{
			joined_channels.erase(it);
		}
		channels.erase(c);
	}
	else
	{
		channels[c].users.erase(ev->sender.nick);
	}

	return false;
}

bool IRCConnection::cb_topic(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::string c = ev->params[1];

	if(channels.find(c) == channels.end())
	{
		channels[c] = Channel(c);
	}
	channels[c].topic = ev->params[2];

	return false;
}

bool IRCConnection::cb_topic_change(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::string c = ev->params[0];

	if(channels.find(c) == channels.end())
	{
		channels[c] = Channel(c);
	}
	channels[c].topic = ev->params[1];
	// TODO: topic_change_time = now

	return false;
}

bool IRCConnection::cb_no_topic(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::string c = ev->params[0];

	if(channels.find(c) == channels.end())
	{
		channels[c] = Channel(c);
	}
	channels[c].topic = "";

	return false;
}

bool IRCConnection::cb_topic_change_time(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::string c = ev->params[1];
	if(channels.find(c) == channels.end())
	{
		channels[c] = Channel(c);
	}

	channels[c].topic_changed_by = ev->params[2];
	channels[c].topic_time = stoi(ev->params[3]);

	return false;
}

bool IRCConnection::cb_who(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);

	std::string n = ev->params[5];

	User *u = get_user(n);
	if(!u->synced)
	{
		u->ident = ev->params[2];
		u->host = ev->params[3];
		u->server = ev->params[4];
		u->account = ev->params[7];
		u->realname = ev->params[8];
		u->synced = true;
	}

	std::string c = ev->params[1];
	if(c != "*")
	{
		if(channels.find(c) == channels.end())
		{
			channels[c] = Channel(c);
		}
		if(channels[c].users.find(n) == channels[c].users.end())
		{
			channels[c].users[n] = ChannelUser(u);
		}
		
		ChannelUser &cu = channels[c].users[n];

		for(char c : ev->params[6])
		{
			if(c != 'H' && c != 'G' && c != '*')
			{
				if(irc_server.prefixes.find(c) != irc_server.prefixes.end())
				{
					cu.modes[irc_server.prefixes[c]] = true;
				}
			}
		}
	}

	return false;
}

bool IRCConnection::cb_end_who(Event *e)
{
	RawIRCEvent *ev = reinterpret_cast<RawIRCEvent*>(e);
	
	std::string c = ev->params[1];

	if(c != "*")
	{
		if(channels.find(c) == channels.end())
		{
			channels[c] = Channel(c);
		}

		channels[c].syncing = false;
	}

	return false;
}

IRCConnection::IRCConnection(EventSink *e, std::string host, unsigned short port) : Connection(host, port), sink(e)
{
	using namespace std::placeholders;
	scratch = new char[SCRATCH_LENGTH];
	scratch_off = 0;
	scratch_len = 0;

/*	sink->add_handler("raw/001", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2));
	sink->add_handler("raw/002", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2));
	sink->add_handler("raw/003", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2));*/
	sink->add_handler("raw/001", "ircconnection", cb_null);
	sink->add_handler("raw/002", "ircconnection", cb_null);
	sink->add_handler("raw/003", "ircconnection", cb_null);

	sink->add_handler("raw/004", "ircconnection", std::bind(&IRCConnection::cb_myinfo, this, _1));
	sink->add_handler("raw/005", "ircconnection", std::bind(&IRCConnection::cb_isupport, this, _1));
	sink->add_handler("raw/042", "ircconnection", std::bind(&IRCConnection::cb_yourid, this, _1));

	/*sink->add_handler("raw/250", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_STATSCONN
	sink->add_handler("raw/251", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSERCLIENT
	sink->add_handler("raw/252", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSEROP
	sink->add_handler("raw/253", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSERUNKNOWN
	sink->add_handler("raw/254", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSERCHANNELS
	sink->add_handler("raw/255", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LUSERME
	sink->add_handler("raw/265", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_LOCALUSERS
	sink->add_handler("raw/266", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2)); //RPL_GLOBALUSERS */

	sink->add_handler("raw/250", "ircconnection", cb_null);
	sink->add_handler("raw/251", "ircconnection", cb_null);
	sink->add_handler("raw/252", "ircconnection", cb_null);
	sink->add_handler("raw/253", "ircconnection", cb_null);
	sink->add_handler("raw/254", "ircconnection", cb_null);
	sink->add_handler("raw/255", "ircconnection", cb_null);
	sink->add_handler("raw/265", "ircconnection", cb_null);
	sink->add_handler("raw/266", "ircconnection", cb_null);

	/*sink->add_handler("raw/372", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2));
	sink->add_handler("raw/375", "ircconnection", std::bind(&IRCConnection::cb_print, this, _1, _2));*/

	sink->add_handler("raw/372", "ircconnection", cb_null);
	sink->add_handler("raw/375", "ircconnection", cb_null);
	sink->add_handler("raw/376", "ircconnection", std::bind(&IRCConnection::cb_end_of_motd, this, _1));

	sink->add_handler("irc/message", "ircconnection", std::bind(&IRCConnection::cb_ctcp, this, _1));
	sink->add_handler("raw/privmsg", "ircconnection", std::bind(&IRCConnection::cb_rewrite_privmsg, this, _1));
	sink->add_handler("raw/ping", "ircconnection", std::bind(&IRCConnection::cb_ping, this, _1));

	//sync callbacks

	sink->add_handler("raw/mode", "ircconnection", std::bind(&IRCConnection::cb_mode, this, _1));
	sink->add_handler("raw/join", "ircconnection", std::bind(&IRCConnection::cb_join, this, _1));
	sink->add_handler("raw/part", "ircconnection", std::bind(&IRCConnection::cb_part, this, _1));
	sink->add_handler("raw/topic", "ircconnection", std::bind(&IRCConnection::cb_topic_change, this, _1));
	sink->add_handler("raw/332", "ircconnection", std::bind(&IRCConnection::cb_topic, this, _1));
	sink->add_handler("raw/333", "ircconnection", std::bind(&IRCConnection::cb_topic_change_time, this, _1));

	//sink->add_handler("raw/353", "ircconnection", std::bind(&IRCConnection::cb_names, this, _1));
	//sink->add_handler("raw/366", "ircconnection", std::bind(&IRCConnection::cb_end_of_names, this, _1));
	sink->add_handler("raw/353", "ircconnection", cb_null);
	sink->add_handler("raw/366", "ircconnection", cb_null);

	sink->add_handler("raw/352", "ircconnection", std::bind(&IRCConnection::cb_who, this, _1));
	sink->add_handler("raw/354", "ircconnection", std::bind(&IRCConnection::cb_who, this, _1));
	sink->add_handler("raw/315", "ircconnection", std::bind(&IRCConnection::cb_end_who, this, _1));

	sink->add_handler("raw/notice", "ircconnection", cb_null);
	sink->add_handler("raw/invite", "ircconnection", std::bind(&IRCConnection::cb_rewrite_invite, this, _1));
}


IRCConnection::~IRCConnection() 
{
	delete[] scratch;
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

void IRCConnection::nick(std::string nick)
{
	current_nick = nick;
	send_line("NICK " + nick);
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
		if(r == -1)
		{
			std::cerr << errno << std::endl;
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

// here be dragons
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
        else if(line[i] == ':' && i != 0 && (line[0] != ':' || read_sender) && line[i-1] == ' ')
        {
            params.push_back(line + i + 1);
            break;
        }
    }

    if(i == len && end != -1)
    {
    	params.push_back(line + end + 1);
    }
}

User IRCConnection::parse_hostmask(std::string hostmask)
{
	User u;
	size_t bang = hostmask.find('!');
	size_t at = hostmask.find('@');
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

	/*if(callbacks.find(command) == callbacks.end())
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
	}*/

	RawIRCEvent *ev = new RawIRCEvent("raw/" + command, u, params);
	sink->queue_event(ev);
}

User* IRCConnection::get_user(std::string name)
{
	if(global_users.find(name) == global_users.end())
	{
		User *u = new User();
		u->synced = false;
		u->nick = name;
		global_users[name] = u;
	}

	return global_users[name];
}

Channel::Channel()
{}

Channel::Channel(std::string n) : name(n)
{}

Mode& Channel::get_mode(char c, ModeType t)
{
	if(modes.find(c) == modes.end())
	{
		Mode m;
		m.type = t;
		modes[c] = m;
	}
	return modes[c];
}

Mode::Mode() {}
Mode::Mode(char c) : mode(c) {}
Mode::Mode(char c, bool flag): type(FLAG), mode(c)  {}
Mode::Mode(char c, std::string v): type(VALUE), mode(c), value(v) {}

ChannelUser::ChannelUser() : user(NULL) {}
ChannelUser::ChannelUser(User *u) : user(u) {}