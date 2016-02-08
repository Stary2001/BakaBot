#include "command.h"

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

std::vector<const CommandData*> StringType::select(CommandData *d, std::string t)
{
	if(!d->is_type(this)) { return {}; }

	StringData *s = (StringData*)d;

	if(t != "string")
	{
		CommandData *r = CommandData::get_type(t)->from_string(s->str);
		if(r != NULL)
		{
			return {r};
		}
		return {};
	}
	else
	{
		return {d};
	}
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

std::vector<const CommandData*> IntType::select(CommandData *d, std::string t)
{
	if(t != "int")
	{
		return {};
	}
	else
	{
		return {d};
	}
}

/*
======================================
pair type
======================================
*/

PairData::PairData(CommandData *first, CommandData *second) : CommandData(CommandData::get_type("pair")), p(std::make_pair(first, second))
{}

std::string PairType::to_string(const CommandData* d)
{
	if (!d->is_type(this)) { return ""; }
	PairData *p = (PairData*)d;

	return p->p.first->to_string() + ": " + p->p.second->to_string();
}

CommandData* PairType::from_string(std::string s)
{
	return NULL;
}

std::vector<const CommandData*> PairType::select(CommandData *d, std::string t)
{
	if (!d->is_type(this)) { return {}; }
	return ((PairData*)d)->p.second->select(t);
}

/*
======================================
list type
======================================
*/

ListData::ListData(std::vector<CommandData*> &vec) : CommandData(CommandData::get_type("list")), v(std::move(vec))
{}

std::string ListType::to_string(const CommandData* d)
{
	if (!d->is_type(this)) { return ""; }
	ListData *l = (ListData*)d;

	std::string o;
	for(auto a: l->v)
	{
		o += a->to_string() + ", ";
	}
	o = o.substr(0, o.length() - 2);

	return o;
}

CommandData* ListType::from_string(std::string s)
{
	return NULL;
}

std::vector<const CommandData*> ListType::select(CommandData *d, std::string t)
{
	if (!d->is_type(this)) { return {}; }
	ListData *l = (ListData*)d;

	std::vector<const CommandData*> vv;

	for(auto item: l->v)
	{
		if(item->get_type()->name == t)
		{
			vv += item->select(t);
		}
	}

	return vv;
}

void ListData::push_back(CommandData *d)
{
	v.push_back(d);
}

/*
======================================
misc. funcs
======================================
*/


std::string CommandData::to_string() const
{
	return type->to_string(this);
}

std::vector<const CommandData*> CommandData::select(std::string s)
{
	return type->select(this, s);
}

bool CommandData::is_type(CommandDataType *t) const
{
	return type == t;
}

const CommandDataType * CommandData::get_type()
{
	return type;
}

bool CommandData::add_type(std::string name, CommandDataType *t)
{
	if (types.find(name) != types.end()) return false;
	t->name = name;
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