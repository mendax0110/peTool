#pragma once

#include <iostream>
#include <string>
#include <vector>

class ProcessMonitor
{
public:
    ProcessMonitor();
    ~ProcessMonitor();
    static std::vector<std::string> GetRunningProcesses();
    static bool IsProcessRunning(const std::string &processName);

private:

protected:
};
