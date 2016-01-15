#include <iostream>
#include "logger.h"

std::string Logger::level_to_string(LogLevel l)
{
    static std::map<LogLevel, std::string> levels = { {LogLevel::FATAL, "FATAL"}, {LogLevel::ERR, "ERROR"}, {LogLevel::WARNING, "WARNING"}, {LogLevel::INFO, "INFO"}, {LogLevel::DEBUG, "DEBUG"} };
    return levels[l];
}

void StderrLogger::log(std::string message, LogLevel level = LogLevel::INFO)
{
    std::cerr << "[" << level_to_string(level) << "] " << message << std::endl;
}

Logger *Logger::instance;


