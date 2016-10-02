#include <algorithm>
#include <functional>

#include "data.h"
#include "util.h"

/*
====================================== 
string type
====================================== 
*/
StringData::StringData(std::string s) : Data(Data::get_type("string")), str(s)
{}

std::string StringType::to_string(const Data* d)
{
	if(!d->is_type(this)) { return ""; }
	return ((StringData*)d)->str;
}

Data* StringType::from_string(std::string s)
{
	return new StringData(s);
}

std::vector<const Data*> StringType::select(Data *d, std::string t)
{
	if(!d->is_type(this)) { return {}; }

	StringData *s = (StringData*)d;

	if(t != "string")
	{
		Data *r = Data::get_type(t)->from_string(s->str);
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

IntData::IntData(long l) : Data(Data::get_type("int")), i(l)
{}

std::string IntType::to_string(const Data* d)
{
	if (!d->is_type(this)) { return ""; }
	return std::to_string(((IntData*)d)->i);
}

Data* IntType::from_string(std::string s)
{
	return new IntData(std::stol(s));
}

std::vector<const Data*> IntType::select(Data *d, std::string t)
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

PairData::PairData(Data *first, Data *second) : Data(Data::get_type("pair")), p(std::make_pair(first, second))
{}

std::string PairType::to_string(const Data* d)
{
	if (!d->is_type(this)) { return ""; }
	PairData *p = (PairData*)d;

	return p->p.first->to_string() + ": " + p->p.second->to_string();
}

Data* PairType::from_string(std::string s)
{
	return NULL;
}

std::vector<const Data*> PairType::select(Data *d, std::string t)
{
	if (!d->is_type(this)) { return {}; }
	return ((PairData*)d)->p.second->select(t);
}

/*
======================================
list type
======================================
*/

ListData::ListData(std::vector<Data*> &vec) : Data(Data::get_type("list")), v(std::move(vec))
{}

std::string ListType::to_string(const Data* d)
{
	if (!d->is_type(this)) { return ""; }
	ListData *l = (ListData*)d;

	std::string o;
	for(auto a: l->v)
	{
		o += Data::serialize(a) + "|";
	}
	o = o.substr(0, o.length() - 1);
	return o;
}

Data* ListType::from_string(std::string val)
{
	std::vector<std::string> v_s = util::split(val, "|");
	std::vector<Data*> v;

	if(v_s.size() > 1 || v_s[0] != "") // if we don't get an empty string, fill the list
	{
		v.resize(v_s.size());
		std::transform(v_s.begin(), v_s.end(), v.begin(), [](std::string s) -> Data* { return Data::deserialize(s); } );
	}

	ListData *d = new ListData(v);
	return d;
}

std::vector<const Data*> ListType::select(Data *d, std::string t)
{
	if (!d->is_type(this)) { return {}; }
	ListData *l = (ListData*)d;

	std::vector<const Data*> vv;

	for(auto item: l->v)
	{
		if(item->get_type()->name == t)
		{
			vv += item->select(t);
		}
	}

	return vv;
}

void ListData::push_back(Data *d)
{
	v.push_back(d);
}

/*
======================================
map type
======================================
*/

MapData::MapData(std::map<std::string, Data*> &m) : Data(Data::get_type("map")), map(std::move(m))
{}

std::string MapType::to_string(const Data* d)
{
	if (!d->is_type(this)) { return ""; }
	MapData *l = (MapData*)d;

	std::string o;
	for(auto a: l->map)
	{
		o += a.first + "=" + a.second->to_string() + ", ";
	}
	o = o.substr(0, o.length() - 2);

	return o;
}

Data* MapType::from_string(std::string s)
{
	return NULL;
}

std::vector<const Data*> MapType::select(Data *d, std::string t)
{
	/*if (!d->is_type(this)) { return {}; }
	ListData *l = (ListData*)d;

	std::vector<const Data*> vv;

	for(auto item: l->v)
	{
		if(item->get_type()->name == t)
		{
			vv += item->select(t);
		}
	}

	return vv;*/

	return {};
}

void MapData::insert(std::string k, Data *v)
{
	map[k] = v;
}

/*
======================================
misc. funcs
======================================
*/

std::string Data::to_string() const
{
	return type->to_string(this);
}

std::vector<const Data*> Data::select(std::string s)
{
	return type->select(this, s);
}

bool Data::is_type(DataType *t) const
{
	return type == t;
}

const DataType * Data::get_type()
{
	return type;
}

bool Data::add_type(std::string name, DataType *t)
{
	if (types.find(name) != types.end()) return false;
	t->name = name;
	types[name] = t;
	return true;
}

DataType* Data::get_type(std::string name)
{
	return types[name];
}

void Data::cleanup_types()
{
	for(auto kv: types)
	{
		delete kv.second;
	}
}

std::string Data::serialize(Data *d)
{
	if(d == nullptr) { return "null"; }
	return d->get_type()->name + ":" + d->to_string();
}

Data *Data::deserialize(std::string v)
{
	int i = v.find(':');
	if(i == std::string::npos)
	{
		return nullptr;
	}

	DataType *d = Data::get_type(v.substr(0, i));
	if(d != nullptr)
	{
		return d->from_string(v.substr(i+1));
	}
	return nullptr;
}

std::map <std::string, DataType*> Data::types;