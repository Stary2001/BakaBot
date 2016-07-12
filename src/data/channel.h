#pragma once
#include "user.h"

struct ChannelUserData
{};

struct ChannelUser
{
	User *user;
	ChannelUserData *data;
};

class Channel
{
public:
	Channel() {}
	Channel(std::string n) : name(n) {}
	
	std::string name;
	std::map<std::string, ChannelUser*> users;
};
