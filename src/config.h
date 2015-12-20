#pragma once
#include <string>
#include <map>

class ConfigException : std::exception
{
public:
    ConfigException(std::string message);
    std::string m_message;
};

enum class NodeType
{
	List,
	String,
	Int,
	None
};

class ConfigValue
{
public:
	ConfigValue();
	ConfigValue(std::string s);
	ConfigValue(int i);

	NodeType type;
	std::string string;
	int integer;
	std::vector<std::string> list;
};

class ConfigNode
{
public:
	NodeType type();
	std::string as_string();
	int as_int();
	std::vector<std::string>& as_list();

	std::map<std::string, ConfigNode*> children;
	ConfigValue v;
};

class Config
{
public:
    Config(std::string path);
    static Config* load(std::string path);
    ConfigNode* get(std::string path);

    void set(std::string path, ConfigValue vv);

private:
    ConfigNode *root;
};
