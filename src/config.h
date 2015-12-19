#pragma once
#include <json/json.h>

class ConfigException : std::exception
{
public:
    ConfigException(std::string message);
    std::string m_message;
};

class Config
{
public:
    Config(std::string path);
    static Config* load(std::string path);
    Json::Value get(std::string path);
    Json::Value get(std::string path, Json::Value def);
    bool m_loaded;
private:
    Json::Value m_root;
};
