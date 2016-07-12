#pragma once

struct IRCUser : public User
{
	std::string ident;
	std::string host;
	std::string realname;

	bool synced;
	bool account_synced;
};