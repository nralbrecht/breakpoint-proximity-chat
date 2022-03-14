#pragma once

#include <fstream>
#include <filesystem>
#include <iostream>
#include <cstdarg>
#include <string>
#include <ctime>


class Logger
{
public:
    void Log(const std::string& sMessage);
    void Log(const char * format, ...);

    Logger& operator<<(const std::string& sMessage);
    static Logger* get();
private:
    Logger();
    Logger(const Logger&){};             // copy constructor is private
    Logger& operator=(const Logger&){ return *this; };  // assignment operator is private

    const std::string CurrentDateTime();

    static const std::string m_sFileName;
    static Logger* m_pThis;
    static std::ofstream m_Logfile;
};
