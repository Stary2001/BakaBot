#include <string>
#include <map>
#include <vector>
#include <deque>
#include "util.h"
#include "bot.h"

#define NAME(n) n ## Command 

#define COMMAND(n) class n ## Command : public CommandBase { virtual std::string name() { return #n; }; virtual void run(Bot *bot, CommandInfo *info)

#define END_COMMAND };

#define REGISTER_COMMAND(b, n) b->register_command(#n, new n ## Command ())
#define REMOVE_COMMAND(b, n) b->remove_command(#n)

class CommandData;
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

class CommandDataType
{
public:
	std::string name;
	virtual ~CommandDataType() {};
	virtual std::string to_string(const CommandData *d) = 0;
	virtual CommandData* from_string(std::string s) = 0;
	virtual std::vector<const CommandData*> select(CommandData *d, std::string type) = 0;
};

class CommandData
{
public:
	std::string to_string() const;
	bool is_type(CommandDataType *t) const;
	std::vector<const CommandData*> select(std::string type);

	static bool add_type(std::string name, CommandDataType *t);
	static CommandDataType* get_type(std::string name);
	const CommandDataType* get_type();

	static void cleanup_types();


protected:
	CommandData(CommandDataType *t) : type(t) {}
	CommandDataType *type;
private:
	static std::map <std::string, CommandDataType*> types;
};

/* ====================================== 
 d r a g o n s 
 ====================================== */

class IntType : public CommandDataType
{
public:
	virtual std::string to_string(const CommandData *d);
	virtual CommandData* from_string(std::string s);
	virtual std::vector<const CommandData*> select(CommandData *d, std::string type);
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
	virtual std::vector<const CommandData*> select(CommandData *d, std::string type);
};

class StringData : public CommandData
{
	friend class StringType;

public:
	StringData(std::string s);
private:
	std::string str;
};

class PairType : public CommandDataType
{
public:
	virtual std::string to_string(const CommandData *d);
	virtual CommandData* from_string(std::string s);
	virtual std::vector<const CommandData*> select(CommandData *d, std::string type);
};

class PairData : public CommandData
{
	friend class PairType;

public:
	PairData(CommandData *first, CommandData *second);
private:
	std::pair<CommandData*, CommandData*> p;
};

class ListType : public CommandDataType
{
public:
	virtual std::string to_string(const CommandData *d);
	virtual CommandData* from_string(std::string s);
	virtual std::vector<const CommandData*> select(CommandData *d, std::string type);
};

class ListData : public CommandData
{
	friend class ListType;

public:
	ListData(std::vector<CommandData*> &v);
	void push_back(CommandData *d);
private:
	std::vector<CommandData*> v;
};

class CommandInfo;

class CommandBase
{
public:
	virtual void run(Bot *b, CommandInfo *i) = 0;
	virtual ~CommandBase() {};
	virtual std::string name() = 0;
private:
};

class CommandInfo
{
public:
	CommandInfo() : sender(NULL), next(NULL) {}
	~CommandInfo() { if(next != NULL) { delete next; } }
	CommandData *pop() { CommandData *tmp = in.front(); in.pop_front(); return tmp; }
	// template <typename T> T *checked_pop() { CommandData *tmp = in.front(); in.pop_front(); return tmp; }

	void error(std::string s) { throw CommandErrorException( (cmd != NULL ? cmd->name() : "unknown"), s); }

	User *sender;
	std::string target;

	CommandBase *cmd;

	std::deque<CommandData*> in;
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