#pragma once

#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdarg>
#include <string>
#include <ctime>

#include "PluginConfig.h"


typedef enum LoggerLogLevel {
    Off = 0,
    Error,
    Warn,
    Info,
    Verbose
} LoggerLogLevel;

class Logger
{
public:
    void Log(LoggerLogLevel level, const std::string& message);
    void Log(LoggerLogLevel level, const char * message);
    void LogF(LoggerLogLevel level, const char * format, ...);

    static Logger* get();
private:
    Logger();
    Logger(const Logger&){};             // copy constructor is private
    Logger& operator=(const Logger&){ return *this; };  // assignment operator is private

    const std::string CurrentDateTime();

    LoggerLogLevel maxLogLevel;

    static Logger* instance;
    static std::ofstream logfile;
};
