#include <string>
#include <vector>
#include <fstream>
#include "config.h"
#include "logger.h"
#include "util.h"

Config::Config(std::string path)
{
    root = new ConfigNode();
}

ConfigValue::ConfigValue() : type(NodeType::None)
{}

ConfigValue::ConfigValue(std::string s) : type(NodeType::None), string(s)
{}

ConfigValue::ConfigValue(int i) : type(NodeType::None), integer(i)
{}

// todo: apply error checking?

NodeType ConfigNode::type()
{
    return v.type;
}

std::string ConfigNode::as_string()
{
    return v.string;
}

int ConfigNode::as_int()
{
    return v.integer;
}

std::vector<std::string> & ConfigNode::as_list()
{
    return v.list;
}

Config* Config::load(std::string path)
{
    try
    {
        Config *c = new Config(path);
        return c;
    }
    catch(ConfigException e)
    {
        Logger::instance->log(e.m_message, LogLevel::ERROR);
    }
    return NULL;
}

ConfigNode* Config::get(std::string path)
{
    ConfigNode *v = root;

    std::vector<std::string> p = util::split(path, '.');
    for(std::string entry : p)
    {
        if(v->children.find(entry) == v->children.end())
        {
            v->children[entry] = new ConfigNode();
        }
        v = v->children[entry];
    }
    return v;
}

void Config::set(std::string path, ConfigValue vv)
{
    ConfigNode *v = get(path);
    v->v = vv;
}

ConfigException::ConfigException(std::string message)
{
    m_message = message;
}
