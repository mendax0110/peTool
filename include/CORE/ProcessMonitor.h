#pragma once

#include <iostream>
#include <string>
#include <vector>

/// @brief ProcessMonitor class, which provides methods for monitoring running processes \class ProcessMonitor
class ProcessMonitor
{
public:

    /**
     * @brief Construct a new ProcessMonitor object
     */
    ProcessMonitor();

    /**
     * @brief Destroy the ProcessMonitor object
     */
    ~ProcessMonitor();

    /**
     * @brief Get the list of running processes
     * @return A vector of strings containing the names of running processes
     */
    static std::vector<std::string> GetRunningProcesses();

    /**
     * @brief Check if a process is running
     * @param processName The name of the process to check
     * @return true if the process is running, false otherwise
     */
    static bool IsProcessRunning(const std::string &processName);

private:

protected:
};
