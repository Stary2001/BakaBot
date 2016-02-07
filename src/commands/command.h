#include <string>
#include <map>
#include <vector>
#include <deque>
#include "util.h"
#include "bot.h"

class CommandData;
class IRCMessageEvent;

class CommandDataType
{
public:
	virtual std::string to_string(CommandData *d) = 0;
	virtual CommandData* from_string(std::string s) = 0;
};

class CommandData
{
public:
	std::string to_string();
	bool is_type(CommandDataType *t) const;

	static bool add_type(std::string name, CommandDataType *t);
	static CommandDataType* get_type(std::string name);

private:
	CommandDataType *type;
	static std::map <std::string, CommandDataType*> types;
};

class CommandInfo
{
public:
	CommandInfo() : sender(NULL), next(NULL) {}

	User *sender;
	std::string target;

	std::deque<CommandData*> in;
	CommandInfo *next;
};

/* ====================================== 
 d r a g o n s 
 ====================================== */

class IntType : public CommandDataType
{
public:
	virtual std::string to_string(const CommandData *d);
	virtual CommandData* from_string(std::string s);
};

class IntData : public CommandData
{
	friend class IntType;

public:
	IntData(long l);
private:
	long i;
};

class StringType : public CommandDataType
{
public:
	virtual std::string to_string(const CommandData *d);
	virtual CommandData* from_string(std::string s);
};

class StringData : public CommandData
{
	friend class StringType;

public:
	StringData(std::string s);
private:
	std::string str;
};

class CommandBase
{
public:
	virtual void run(Bot *b, CommandInfo *i) = 0;
};

class Command
{
public:
	static void run (Bot *b, IRCMessageEvent *ev);
	static CommandBase *get_ptr(std::string name);
private:
	static std::tuple<CommandInfo*, std::vector<CommandBase*>> parse(Bot *b, IRCMessageEvent *ev);
};