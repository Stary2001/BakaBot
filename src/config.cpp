#include <string>
#include <vector>
#include <fstream>
#include "config.h"
#include "logger.h"
#include "util.h"

Config::Config(std::string path)
{
    root = std::shared_ptr<ConfigNode>(new ConfigNode());

    std::ifstream config_file(path, std::ios::in);
    if(config_file.good())
    {
        std::string s;
        while(std::getline(config_file, s))
        {
            int eq = s.find("=");
            std::string k = s.substr(0, eq);
            std::string v = s.substr(eq+1);
            int type = v.find(":");
            std::string t = v.substr(0, type);
            v = v.substr(type+1);
            set(k, Data::get_type(t)->from_string(v));
        }
    }
    else
    {
        throw ConfigException("Couldn't open file '" + path + "'!");
    }

	config_file.close();

    filename = path;
}

// todo: error checking?

std::string ConfigNode::as_string()
{
    if(v == nullptr) { return ""; }
    return v->to_string();
}

long ConfigNode::as_int()
{
    if(v == nullptr) { return 0; }
    static DataType *t = Data::get_type("int");
    if(v->get_type() != t) { return 0; }
    return ((IntData*)v)->i;
}

std::vector<Data*> & ConfigNode::as_list()
{
    if(v == nullptr) { throw new ConfigException("null"); }
    static DataType *t = Data::get_type("list");
    if(v->get_type() != t) { throw ConfigException("not a list god damnit"); }
    return ((ListData*)v)->v;
}

std::map<std::string, Data*> & ConfigNode::as_map()
{
    if(v == nullptr) { throw new ConfigException("null"); }
    static DataType *t = Data::get_type("map");
    if(v->get_type() != t) { throw ConfigException("not a map god damnit"); }
    return ((MapData*)v)->map;
}

bool ConfigNode::is(std::string type)
{
    if(v == nullptr)
    {
        return type == "null";
    }  
    else
    {
        return v->get_type()->name == type;
    }
}

/*std::string Config::serialize(ConfigValue &v)
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
    else if(v.type == NodeType::Map)
    {
        val = "map:[";
        for(auto kv : v.map)
        {
            val += kv.first + "=" + serialize(kv.second) + (kv != *v.map.rbegin() ? "|" : "");
        }
        val += "]";
    }

    return val;
}

ConfigValue Config::deserialize(std::string val)
{
    ConfigValue v;

    size_t c = val.find(":");
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
        v.integer = stol(val);
    }
    else if(type == "list")
    {
        v.type = NodeType::List;
        val = val.substr(1);
        val = val.substr(0, val.length() - 1);
        v.list = util::split(val, "|");
    }
    else if(type == "map")
    {
        v.type = NodeType::Map;
        val = val.substr(1);
        val = val.substr(0, val.length() - 1);

        auto parts = util::split(val, "|");
        for(auto kv : parts)
        {
            auto partss = util::split(kv, "=");
            v.map[partss[0]] = deserialize(partss[1]);
        }
    }

    return v;
}*/

void ConfigNode::save(std::string prefix, std::ofstream &f)
{
    if(prefix != "" && !is("null"))
    {
        std::string s = prefix + "=" + Data::serialize(v) + "\n";
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
        Logger::instance->log(e.m_message, LogLevel::ERR);
    }
    return NULL;
}

std::shared_ptr<ConfigNode> Config::get(std::string path)
{
    std::shared_ptr<ConfigNode> v = root;

    std::vector<std::string> p = util::split(path, ".");
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

void Config::set(std::string path, Data *vv)
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
