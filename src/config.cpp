#include <string>
#include <vector>
#include <fstream>
#include "config.h"
#include "logger.h"
#include "util.h"

Config::Config(std::string path)
{
    std::ifstream config_doc(path, std::ios::binary | std::ios::in );
    if(config_doc.good())
    {
        Json::CharReaderBuilder rbuilder;
        std::string errs;
        bool success = Json::parseFromStream(rbuilder, config_doc, &m_root, &errs);
        if(!success)
        {
            throw ConfigException(errs);
        }
    }
    else
    {
        throw ConfigException("Couldn't open file '" + path + "'!");
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

Json::Value Config::get(std::string path)
{
    Json::Value v = m_root;

    std::vector<std::string> p = util::split(path, '.');
    for(std::string entry : p)
    {
        v = v[entry];
    }
    return v;
}

Json::Value Config::get(std::string path, Json::Value def)
{
    Json::Value v = m_root;
    std::vector<std::string> p = util::split(path, '.');
    for(std::string entry : p)
    {
        v = v[entry];
        if(v.isNull())
            return def;
    }
    return v;
}

ConfigException::ConfigException(std::string message)
{
    m_message = message;
}
