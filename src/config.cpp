#include <string>
#include <vector>
#include <fstream>
#include "config.h"
#include "logger.h"
#include "util.h"


ConfigValue deserialize(std::string val);

// todo: abuse the everloving shit out of msgpack
Config::Config(std::string path)
{
    root = std::shared_ptr<ConfigNode>(new ConfigNode());

    std::ifstream config_file(path, std::ios::binary | std::ios::in);
    if(config_file.good())
    {
        std::string s;
        while(std::getline(config_file, s))
        {
            int eq = s.find("=");
            std::string k = s.substr(0, eq);
            std::string v = s.substr(eq+1);
            ConfigValue vv = deserialize(v);
            set(k, vv);
        }
    }
    else
    {
        throw ConfigException("Couldn't open file '" + path + "'!");
    }

    filename = path;
}

ConfigValue::ConfigValue() : type(NodeType::Null), integer(0)
{}

ConfigValue::ConfigValue(std::string s) : type(NodeType::String), string(s), integer(0)
{}

ConfigValue::ConfigValue(int i) : type(NodeType::Int), integer(i)
{}

// todo: error checking?

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

std::string serialize(ConfigValue &v)
{
    std::string val;
    if(v.type == NodeType::String)
    {
       val = "str:" + v.string;
    }
    else if(v.type == NodeType::Int)
    {
       val = "int:" + std::to_string(v.integer);
    }
    else if(v.type == NodeType::List)
    {
        val = "list:[";
        for(auto s : v.list)
        {
            val += s + (s != *v.list.rbegin() ? "|" : "");
        }
        val += "]";
    }

    return val;
}

ConfigValue deserialize(std::string val)
{
    ConfigValue v;

    int c = val.find(":");
    if(c == std::string::npos)
    {
        return v;
    }
    std::string type = val.substr(0, c);
    val = val.substr(c+1);

    if(type == "str")
    {
        v.type = NodeType::String;
        v.string = val;
    }
    else if(type == "int")
    {
        v.type = NodeType::Int;
        v.integer = stoi(val);
    }
    else if(type == "list")
    {
        v.type = NodeType::List;
        val = val.substr(1);
        val = val.substr(0, val.length() - 1);
        v.list = util::split(val, '|');
    }

    return v;
}

void ConfigNode::save(std::string prefix, std::ofstream &f)
{
    if(prefix != "" && v.type != NodeType::Null)
    {
        std::string s = prefix + "=" + serialize(v) + "\n";
        f.write(s.c_str(), s.size());
    }

    for(auto kv: children)
    {
        kv.second->save(prefix + (prefix != "" ? "." : "") + kv.first, f);
    }
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

std::shared_ptr<ConfigNode> Config::get(std::string path)
{
    std::shared_ptr<ConfigNode> v = root;

    std::vector<std::string> p = util::split(path, '.');
    for(std::string entry : p)
    {
        if(v->children.find(entry) == v->children.end())
        {
            v->children[entry] = std::shared_ptr<ConfigNode>(new ConfigNode());
        }
        v = v->children[entry];
    }
    return v;
}

void Config::set(std::string path, ConfigValue vv)
{
    std::shared_ptr<ConfigNode> v = get(path);
    v->v = vv;
}

void Config::save()
{
    std::ofstream config_file(filename, std::ios::binary | std::ios::out);
    if(config_file.good())
    {
        root->save("", config_file);
        config_file.close();
    }
    else
    {
        throw ConfigException("Couldn't open file '" + filename + "'!");
    }
}

ConfigException::ConfigException(std::string message)
{
    m_message = message;
}
