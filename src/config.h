#pragma once
#include <string>
#include <map>
#include <fstream>
#include <memory>

class ConfigException : public std::exception
{
public:
    ConfigException(std::string message);
    std::string m_message;
};

enum class NodeType
{
	List,
	Map,
	String,
	Int,
	Null
};

class ConfigValue
{
public:
	ConfigValue();
	ConfigValue(std::string s);
	ConfigValue(int i);
	NodeType type;
	std::string string;
	long integer;
	std::vector<std::string> list;
	std::map<std::string, ConfigValue> map;
};

class ConfigNode
{
public:
	NodeType type();
	std::string as_string();
	long as_int();
	std::vector<std::string>& as_list();
	std::map<std::string, ConfigValue>& as_map();

	std::map<std::string, std::shared_ptr<ConfigNode>> children;
	ConfigValue v;

	void save(std::string prefix, std::ofstream &f);
};

class Config
{
public:
    Config(std::string path);
    static Config* load(std::string path);
    std::shared_ptr<ConfigNode> get(std::string path);

    void set(std::string path, ConfigValue vv);
    void save();

    static std::string serialize(ConfigValue &v);
    static ConfigValue deserialize(std::string val);

private:
    std::shared_ptr<ConfigNode> root;
    std::string filename;
};


inline bool operator==(const ConfigValue &lhs, const ConfigValue &rhs)
{
    if(lhs.type != rhs.type) return false;
    if(lhs.type == NodeType::String)
    {
        if(lhs.string != rhs.string) return false;
    }
    else if(lhs.type == NodeType::Int)
    {
        if(lhs.integer != rhs.integer) return false;
    }  
    else if(lhs.type == NodeType::List)
    {
        if(lhs.list != rhs.list) return false;
    }
    else if(lhs.type == NodeType::Map)
    {
        if(lhs.map != rhs.map) return false;
    }

    return true;
}