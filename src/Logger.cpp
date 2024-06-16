#include "../include/CORE/Logger.h"
#include <iostream>
#include <iomanip>

Logger::Logger() = default;

Logger::~Logger()
{
    writeLogToFile();
}

/**
 * @brief Log a message
 * @param level The log level
 * @param message The message to log
 * @param path The path to save the log file
 */
void Logger::log(LogLevel level, const std::string& message, const std::string& path /* = "" */)
{
    std::lock_guard<std::mutex> lock(logMutex);
    logMessages.emplace_back(level, message, path);
}

/**
 * @brief Write Log to file
 */
void Logger::writeLogToFile()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    std::string dateTimeLog = ss.str();

    std::string logFileName;
    if (!logMessages.empty() && !logMessages[0].path.empty())
    {
        logFileName = logMessages[0].path + "/log_" + dateTimeLog + ".txt";
    }
    else
    {
        logFileName = "../log/log_" + dateTimeLog + ".txt";
    }

    logFile.open(logFileName, std::ios::out);

    if (!logFile.is_open())
    {
        std::cerr << "Failed to open log file: " << logFileName << std::endl;
        return;
    }

    std::streambuf* coutBuffer = std::cout.rdbuf(logFile.rdbuf());
    std::streambuf* cerrBuffer = std::cerr.rdbuf(logFile.rdbuf());

    for (const auto& message : logMessages)
    {
        logFile << message.message << std::endl;
    }

    std::cout.rdbuf(coutBuffer);
    std::cerr.rdbuf(cerrBuffer);

    logFile.close();
}
