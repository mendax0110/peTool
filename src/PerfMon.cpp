#include "../include/MANMON/PerfMon.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <unordered_map>

#if defined(_APPLE__)
#include <unistd.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#endif


std::vector<PerformanceMonitor::PerformanceData> PerformanceMonitor::performanceData;
std::chrono::time_point<std::chrono::high_resolution_clock> PerformanceMonitor::startTime;
std::chrono::time_point<std::chrono::high_resolution_clock> PerformanceMonitor::endTime;
std::unordered_map<const char*, std::chrono::time_point<std::chrono::high_resolution_clock>> startTimes;

void PerformanceMonitor::start(const char* name)
{
    startTime = std::chrono::high_resolution_clock::now();
}

void PerformanceMonitor::stop(const char* name)
{
    endTime = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000.0;
    performanceData.push_back({name, duration});
}

void PerformanceMonitor::reset()
{
    performanceData.clear();
}

void PerformanceMonitor::report()
{
    std::cout << std::setw(70) << std::setfill('-') << "" << std::setfill(' ') << std::endl;
    std::cout << "Performance Report:" << std::endl;
    std::cout << std::setw(70) << std::setfill('-') << "" << std::setfill(' ') << std::endl;
    std::cout << std::setw(30) << std::left << "Task" << std::setw(20) << "Time (ms)" << std::endl;
    std::cout << std::setw(70) << std::setfill('-') << "" << std::setfill(' ') << std::endl;

    for (const auto& data : performanceData)
    {
        std::cout << std::setw(30) << std::left << data.name << std::fixed << std::setprecision(3) << std::setw(20) << data.time << std::endl;
    }

    std::cout << std::setw(70) << std::setfill('-') << "" << std::setfill(' ') << std::endl;
    std::cout << std::setw(30) << std::left << "CPU Usage:" << std::fixed << std::setprecision(3) << std::setw(20) << getCPUUsage() << "%" << std::endl;
    std::cout << std::setw(30) << std::left << "Memory Usage:" << std::fixed << std::setprecision(3) << std::setw(20) << getMemoryUsage() << " MB" << std::endl;
    std::cout << std::setw(70) << std::setfill('-') << "" << std::setfill(' ') << std::endl;
}

double PerformanceMonitor::getCPUUsage()
{
#if defined(_WIN32)
    DWORD processID = GetCurrentProcessId();
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess == nullptr)
        return -1;

    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (!GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime))
    {
        CloseHandle(hProcess);
        return -1;
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    FILETIME now;
    GetSystemTimeAsFileTime(&now);

    ULARGE_INTEGER kernel, user, nowTime;
    kernel.LowPart = kernelTime.dwLowDateTime;
    kernel.HighPart = kernelTime.dwHighDateTime;
    user.LowPart = userTime.dwLowDateTime;
    user.HighPart = userTime.dwHighDateTime;
    nowTime.LowPart = now.dwLowDateTime;
    nowTime.HighPart = now.dwHighDateTime;

    double kernelTimeMs = (kernel.QuadPart / 10000.0);
    double userTimeMs = (user.QuadPart / 10000.0);
    double nowTimeMs = (nowTime.QuadPart / 10000.0);

    return ((kernelTimeMs + userTimeMs) / nowTimeMs) * 100.0;
#elif defined(__APPLE__)
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    struct kinfo_proc info {};
    size_t size = sizeof(info);
    if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), &info, &size, nullptr, 0) == -1)
        return -1;

    struct timeval user {}, system {};
    user = info.kp_proc.p_rtime;
    system = info.kp_proc.p_rtime;

    double totalTimeMs = (user.tv_sec + system.tv_sec) * 1000.0 + (user.tv_usec + system.tv_usec) / 1000.0;

    struct timeval now {};
    gettimeofday(&now, nullptr);
    double nowTimeMs = now.tv_sec * 1000.0 + now.tv_usec / 1000.0;

    return (totalTimeMs / nowTimeMs) * 100.0;
#elif defined(__linux__)
    struct rusage usage {};
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        struct timeval user = usage.ru_utime;
        struct timeval system = usage.ru_stime;

        double totalTimeMs = (user.tv_sec + system.tv_sec) * 1000.0 + (user.tv_usec + system.tv_usec) / 1000.0;

        struct timeval now {};
        gettimeofday(&now, nullptr);
        double nowTimeMs = now.tv_sec * 1000.0 + now.tv_usec / 1000.0;

        return (totalTimeMs / nowTimeMs) * 100.0;
    }
#endif
}

double PerformanceMonitor::getMemoryUsage()
{
#if defined(_WIN32)
    DWORD processID = GetCurrentProcessId();
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    if (hProcess == nullptr)
        return -1;

    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }
#elif defined(__linux__) || defined(__APPLE__)
    struct rusage usage {};
    if (getrusage(RUSAGE_SELF, &usage) == 0)
        return static_cast<double>(usage.ru_maxrss) / 1024.0;
#endif
    return -1;
}
