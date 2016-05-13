#pragma once
#include <string>
#include <vector>
#include <map>

class Data;

class DataType
{
public:
	std::string name;
	virtual ~DataType() {};
	virtual std::string to_string(const Data *d) = 0;
	virtual Data* from_string(std::string s) = 0;
	virtual std::vector<const Data*> select(Data *d, std::string type) = 0;
};

class Data
{
public:
	Data() : type(Data::get_type("null")) {}

	std::string to_string() const;
	bool is_type(DataType *t) const;
	std::vector<const Data*> select(std::string type);

	static bool add_type(std::string name, DataType *t);
	static DataType* get_type(std::string name);
	const DataType* get_type();

	static void cleanup_types();

	static std::string serialize(Data *d);
	static Data* deserialize(std::string s);
protected:
	Data(DataType *t) : type(t) {}
	DataType *type;
private:
	static std::map <std::string, DataType*> types;
};

struct DataEquals
{
	DataEquals(const std::string s) : one(s) {}
	const std::string one;
	bool operator()(const Data *other) const
	{
		return other->to_string() == one;
	}
};

/* ====================================== 
 d r a g o n s 
 ====================================== */

class IntType : public DataType
{
public:
	virtual std::string to_string(const Data *d);
	virtual Data* from_string(std::string s);
	virtual std::vector<const Data*> select(Data *d, std::string type);
};

class IntData : public Data
{
	friend class IntType;

public:
	IntData(long l);
	long i;
};

class StringType : public DataType
{
public:
	virtual std::string to_string(const Data *d);
	virtual Data* from_string(std::string s);
	virtual std::vector<const Data*> select(Data *d, std::string type);
};

class StringData : public Data
{
	friend class StringType;

public:
	StringData(std::string s);
	std::string str;
};

class PairType : public DataType
{
public:
	virtual std::string to_string(const Data *d);
	virtual Data* from_string(std::string s);
	virtual std::vector<const Data*> select(Data *d, std::string type);
};

class PairData : public Data
{
	friend class PairType;

public:
	PairData(Data *first, Data *second);
	std::pair<Data*, Data*> p;
};

class ListType : public DataType
{
public:
	virtual std::string to_string(const Data *d);
	virtual Data* from_string(std::string s);
	virtual std::vector<const Data*> select(Data *d, std::string type);
};

class ListData : public Data
{
	friend class ListType;

public:
	ListData(std::vector<Data*> &v);
	void push_back(Data *d);
	std::vector<Data*> v;
};

class MapType : public DataType
{
public:
	virtual std::string to_string(const Data *d);
	virtual Data* from_string(std::string s);
	virtual std::vector<const Data*> select(Data *d, std::string type);
};

class MapData : public Data
{
	friend class MapType;

public:
	MapData(std::map<std::string, Data*> &v);
	void insert(std::string k, Data *v);
	std::map<std::string, Data*> map;
};
