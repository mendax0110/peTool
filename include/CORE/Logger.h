#include <iostream>
#include <sstream>
#include <cstring>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include <mutex>

#ifdef _WIN32
#define ERROR_MACRO_ALREADY_DEFINED
#endif

class Logger
{
public:
    Logger();
    ~Logger();

    enum class LogLevel
    {
        INFO,
        WARNING,
#ifndef _WIN32
        ERROR
#endif
    };

    struct LogMessage
    {
        LogLevel level;
        std::string message;
        std::string path;
        LogMessage(LogLevel l, std::string m, std::string p)
            : level(l), message(std::move(m)), path(std::move(p)) {}
    };

    void log(LogLevel level, const std::string& message, const std::string& path);
    void writeLogToFile();

private:
    std::ofstream logFile;
    std::stringstream logStream;
    std::vector<LogMessage> logMessages;
    std::mutex logMutex;
};
