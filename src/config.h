#pragma once
#include <string>
#include <map>
#include <fstream>
#include <memory>

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

private:
    std::shared_ptr<ConfigNode> root;
    std::string filename;
};
