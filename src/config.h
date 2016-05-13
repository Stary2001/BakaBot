#pragma once
#include <string>
#include <map>
#include <fstream>
#include <memory>
#include "export.h"
#include "data/data.h"

PLUGINCLASS ConfigException : public std::exception
{
public:
    ConfigException(std::string message);
    std::string m_message;
};

PLUGINCLASS ConfigNode
{
public:
	std::string as_string();
	long as_int();
	std::vector<Data*>& as_list();
	std::map<std::string, Data*>& as_map();

    bool is(std::string type);

	std::map<std::string, std::shared_ptr<ConfigNode>> children;
	Data *v;

	void save(std::string prefix, std::ofstream &f);
};

PLUGINCLASS Config
{
public:
    Config(std::string path);
    static Config* load(std::string path);
    std::shared_ptr<ConfigNode> get(std::string path);

    void set(std::string path, Data *vv);
    void save();

    static std::string serialize(Data &v);
    static Data deserialize(std::string val);
	
	std::string filename;
private:
    std::shared_ptr<ConfigNode> root;
};