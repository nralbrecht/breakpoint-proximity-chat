#include "Logger.h"


const std::string Logger::m_sFileName = std::filesystem::temp_directory_path().string() + "grb_latest.log";
Logger* Logger::m_pThis = NULL;
std::ofstream Logger::m_Logfile;
Logger::Logger()
{
}
Logger* Logger::get(){
    if (m_pThis == NULL){
        m_pThis = new Logger();
        // m_Logfile.open(m_sFileName.c_str(), std::ios::out | std::ios::app);
    }
    return m_pThis;
}

void Logger::Log(const char * format, ...)
{
    // char* sMessage = NULL;
    // int nLength = 0;
    // va_list args;
    // va_start(args, format);
    // //  Return the number of characters in the string referenced the list of arguments.
    // // _vscprintf doesn't count terminating '\0' (that's why +1)
    // nLength = _vscprintf(format, args) + 1;
    // sMessage = new char[nLength];
    // vsprintf_s(sMessage, nLength, format, args);
    // //vsprintf(sMessage, format, args);
    // m_Logfile << CurrentDateTime() << ":\t";
    // m_Logfile << sMessage << "\n";
    // va_end(args);

    // m_Logfile.flush();

    // delete [] sMessage;
}

void Logger::Log(const std::string& sMessage)
{
    // m_Logfile <<  CurrentDateTime() << ":\t";
    // m_Logfile << sMessage << "\n";
    // m_Logfile.flush();
}

Logger& Logger::operator<<(const std::string& sMessage)
{
    // m_Logfile << "\n" << CurrentDateTime() << ":\t";
    // m_Logfile << sMessage << "\n";
    // m_Logfile.flush();
    return *this;
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
