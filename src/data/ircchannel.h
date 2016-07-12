#pragma once

#include "channel.h"
#include "ircuser.h"

enum ModeType
{
	FLAG,
	LIST,
	VALUE
};

class Mode
{
public:
	Mode();
	Mode(char c);
	Mode(char c, bool flag);
	Mode(char c, std::string v);

	ModeType type;
	char mode;
	bool active;
	std::string value;
	std::vector<std::string> list;
};


class IRCChannel : public Channel
{
public:
	IRCChannel();
	IRCChannel(std::string n);

	std::string name;
	std::string topic;
	std::string topic_changed_by;

	unsigned int topic_time;
	std::map<char, Mode> modes;
	Mode& get_mode(char c, ModeType m);

	bool syncing;
};

struct IRCChannelUserData : public ChannelUserData
{
public:
	std::map<char, bool> modes;
};
