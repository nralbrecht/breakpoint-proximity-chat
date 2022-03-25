#include "Logger.h"


Logger::Logger()
{
    std::string fileName = "";
    try
    {
        fileName = PluginState::PATH + PluginConfig::get()->getValue<std::string>("log_file");
        maxLogLevel = static_cast<LoggerLogLevel>(PluginConfig::get()->getValue<int>("log_level"));

        if (!logfile.is_open()) {
            logfile.open(fileName, std::ios::out | std::ios::app);
        }
    }
    catch(const std::exception& e)
    {
        MessageBox(0, ("could not get 'log_file' path from config (" + std::string(e.what()) + "): '" + fileName + "'").c_str(), "GRB Logger", 0);
        throw e;
    }
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

    lock.lock();

    logfile << CurrentDateTime() << " [" << loggerLogLevelLables[level] << "]:\t";
    logfile << sMessage << "\n";
    logfile.flush();

    lock.unlock();
}

void Logger::Log(LoggerLogLevel level, const char * message) {
    if (level > maxLogLevel) {
        return;
    }

    lock.lock();

    logfile << CurrentDateTime() << " [" << loggerLogLevelLables[level] << "]:\t";
    logfile << message << "\n";
    logfile.flush();

    lock.unlock();
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

    lock.lock();

    logfile << CurrentDateTime() << " [" << loggerLogLevelLables[level] << "]:\t";
    logfile << sMessage << "\n";
    logfile.flush();

    lock.unlock();

    va_end(args);
    delete [] sMessage;
}
