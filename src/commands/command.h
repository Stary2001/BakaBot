#include <string>
#include <map>

class CommandData;

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
	// insert blocking queue of CommandData* here..
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