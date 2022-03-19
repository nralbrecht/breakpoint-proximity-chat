#pragma once

#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdarg>
#include <string>
#include <ctime>
#include "Windows.h"

#include "PluginConfig.h"


typedef enum LoggerLogLevel {
    Off = 0,
    Error,
    Warn,
    Info,
    Verbose
} LoggerLogLevel;

static const char *loggerLogLevelLables[] = { "Off", "Error", "Warn", "Info", "Verbose" };

class Logger
{
private:
    Logger();
    Logger(const Logger&){};             // copy constructor is private
    Logger& operator=(const Logger&){ return *this; };  // assignment operator is private

    const std::string CurrentDateTime();

    inline static Logger* instance = nullptr;
    inline static std::ofstream logfile;
    inline static LoggerLogLevel maxLogLevel = LoggerLogLevel::Verbose;
public:
    void Log(LoggerLogLevel level, const std::string& message);
    void Log(LoggerLogLevel level, const char * message);
    void LogF(LoggerLogLevel level, const char * format, ...);

    static Logger* get();
};
