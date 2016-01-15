#pragma once

#include <map>
#include <string>

enum class LogLevel
{
    FATAL,
    ERR,
    WARNING,
    INFO,
    DEBUG
};

class Logger
{
public:
    virtual void log(std::string message, LogLevel level) = 0;
    static Logger *instance;
protected:
    std::string level_to_string(LogLevel l);
};

class StderrLogger : public Logger
{
    virtual void log(std::string message, LogLevel level);
};