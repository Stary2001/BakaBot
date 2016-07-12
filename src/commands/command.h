#pragma once
#include <string>
#include <map>
#include <vector>
#include <deque>
#include "util.h"
#include "bot.h"
#include "data/data.h"

#define NAME(n) n ## Command 

#define COMMAND(n, f, net) class n ## Command : public CommandBase { public: n ## Command () : CommandBase(f) {}; virtual std::string name() { return #n; }; virtual void run(Bot *bot, CommandInfo *info)

#define END_COMMAND };

#define REGISTER_COMMAND(b, n) b->register_command(#n, new n ## Command ())
#define REMOVE_COMMAND(b, n) b->remove_command(#n)

class IRCMessageEvent;

class CommandException : public std::exception
{};

class CommandNotFoundException : public CommandException
{
public:
	CommandNotFoundException(std::string s) : command(s) {}
	std::string command;
};

class PermissionDeniedException : public CommandException
{
public:
	PermissionDeniedException(std::string s) : command(s) {}
	std::string command;
};

class CommandErrorException : public CommandException
{
public:
	CommandErrorException(std::string cmd, std::string e) : command(cmd), err(e) {}
	std::string command;
	std::string err;
};

class CommandInfo;

enum class CommandFlags
{
	None = 0,
	OneParam = 1
};

class CommandBase
{
public:
	CommandBase(CommandFlags f) : flags(f) {}
	virtual void run(Bot *b, CommandInfo *i) = 0;
	virtual ~CommandBase() {};
	virtual std::string name() = 0;
	CommandFlags flags;
private:
};


class CommandInfo
{
public:
	CommandInfo() : sender(NULL), next(NULL) {}
	~CommandInfo() { if(next != NULL) { delete next; } }
	Data *pop() { Data *tmp = in.front(); in.pop_front(); return tmp; }
	// template <typename T> T *checked_pop() { CommandData *tmp = in.front(); in.pop_front(); return tmp; }

	void error(std::string s) { throw CommandErrorException( (cmd != NULL ? cmd->name() : "unknown"), s); }

	User *sender;
	std::string target;

	CommandBase *cmd;

	std::deque<Data*> in;
	CommandInfo *next;
};

class Command
{
public:
	static void run (Bot *b, IRCMessageEvent *ev);
	static CommandBase *get_ptr(std::string name);
private:
	static std::tuple<CommandInfo*, std::vector<CommandBase*>> parse(Bot *b, IRCMessageEvent *ev);
};