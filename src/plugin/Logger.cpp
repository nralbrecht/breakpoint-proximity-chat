#include "Logger.h"


Logger* Logger::instance = nullptr;
std::ofstream Logger::logfile;

Logger::Logger()
{
    std::string fileName;
    // try
    // {
    //     fileName = PluginConfig::get()->getValue<std::string>("log_file");
    // }
    // catch(const std::exception& e)
    // {
    //     fileName = std::filesystem::temp_directory_path().string() + "grb_latest.log";

    //     logfile.open(fileName.c_str(), std::ios::out | std::ios::app);
    //     logfile << CurrentDateTime() << ":\t Logger could not get 'log_file' path from config (" << e.what() << ")\n";
    //     logfile.flush();
    //     logfile.close();

    //     throw e;
    // }
    fileName = std::filesystem::temp_directory_path().string() + "grb_latest.log";

    logfile.open(fileName.c_str(), std::ios::out | std::ios::app);
}
Logger* Logger::get(){
    if (instance == NULL){
        instance = new Logger();
    }
    return instance;
}

const std::string Logger::CurrentDateTime()
{
    time_t     now = time(NULL);
    struct tm  tstruct;
    char       buf[80];
    localtime_s(&tstruct, &now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}


void Logger::Log(LoggerLogLevel level, const std::string& sMessage) {
    if (level > maxLogLevel) {
        return;
    }

    logfile << CurrentDateTime() << ":\t";
    logfile << sMessage << "\n";
    logfile.flush();
}

void Logger::Log(LoggerLogLevel level, const char * message) {
    if (level > maxLogLevel) {
        return;
    }

    logfile << CurrentDateTime() << ":\t";
    logfile << message << "\n";

    logfile.flush();
}

void Logger::LogF(LoggerLogLevel level, const char * format, ...) {
    if (level > maxLogLevel) {
        return;
    }

    char* sMessage = NULL;
    int nLength = 0;
    va_list args;
    va_start(args, format);
    //  Return the number of characters in the string referenced the list of arguments.
    // _vscprintf doesn't count terminating '\0' (that's why +1)
    nLength = _vscprintf(format, args) + 1;
    sMessage = new char[nLength];
    vsprintf_s(sMessage, nLength, format, args);
    //vsprintf(sMessage, format, args);
    logfile << CurrentDateTime() << ":\t";
    logfile << sMessage << "\n";
    va_end(args);

    logfile.flush();

    delete [] sMessage;
}
