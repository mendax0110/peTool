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

/// @brief Logger class, which provides methods for logging messages to a file \class Logger
class Logger
{
public:

    /**
     * @brief Construct a new Logger object
     */
    Logger();

    /**
     * @brief Destroy the Logger object
     */
    ~Logger();

    /// @brief Log levels for the logger \name
    enum class LogLevel
    {
        INFO,
        WARNING,
#ifndef _WIN32
        ERROR
#endif
    };

    /// @brief Struct to hold log messages \struct LogMessage
    struct LogMessage
    {
        LogLevel level;
        std::string message;
        std::string path;
        LogMessage(LogLevel l, std::string m, std::string p)
            : level(l), message(std::move(m)), path(std::move(p)) {}
    };

    /**
     * @brief Log a message with a specific log level
     * @param level The log level
     * @param message The message to log
     * @param path The file path associated with the log message
     */
    void log(LogLevel level, const std::string& message, const std::string& path);

    /**
     * @brief Get the log messages
     * @return A vector of log messages
     */
    void writeLogToFile();

private:
    std::ofstream logFile;
    std::stringstream logStream;
    std::vector<LogMessage> logMessages;
    std::mutex logMutex;
};
