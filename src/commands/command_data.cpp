#include "command.h"

std::string CommandData::to_string()
{
	return type->to_string(this);
}

bool CommandData::is_type(CommandDataType *t) const
{
	return type == t;
}

/*
====================================== 
string type
====================================== 
*/
StringData::StringData(std::string s) : CommandData(CommandData::get_type("string")), str(s)
{}

std::string StringType::to_string(const CommandData* d)
{
	if(!d->is_type(this)) { return ""; }
	return ((StringData*)d)->str;
}

CommandData* StringType::from_string(std::string s)
{
	return new StringData(s);
}

/*
======================================
int type
======================================
*/

IntData::IntData(long l) : CommandData(CommandData::get_type("int")), i(l)
{}

std::string IntType::to_string(const CommandData* d)
{
	if (!d->is_type(this)) { return ""; }
	return std::to_string(((IntData*)d)->i);
}

CommandData* IntType::from_string(std::string s)
{
	return new IntData(std::stol(s));
}

bool CommandData::add_type(std::string name, CommandDataType *t)
{
	if (types.find("name") != types.end()) return false;
	types[name] = t;
	return true;
}

CommandDataType* CommandData::get_type(std::string name)
{
	return types[name];
}

void CommandData::cleanup_types()
{
	for(auto kv: types)
	{
		delete kv.second;
	}
}


std::map <std::string, CommandDataType*> CommandData::types;