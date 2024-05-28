#include "../include/MANMON/PerfMon.h"
#include <algorithm>
#include <iomanip>
#include <unordered_map>

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
    FILETIME createTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime) != -1) {
        ULARGE_INTEGER kernel, user;
        kernel.LowPart = kernelTime.dwLowDateTime;
        kernel.HighPart = kernelTime.dwHighDateTime;
        user.LowPart = userTime.dwLowDateTime;
        user.HighPart = userTime.dwHighDateTime;
        auto kernelTimeMs = kernel.QuadPart / 10000;
        auto userTimeMs = user.QuadPart / 10000;

        FILETIME now;
        GetSystemTimeAsFileTime(&now);
        ULARGE_INTEGER nowTime;
        nowTime.LowPart = now.dwLowDateTime;
        nowTime.HighPart = now.dwHighDateTime;
        auto nowTimeMs = nowTime.QuadPart / 10000;

        auto totalTimeMs = nowTimeMs - createTime.dwLowDateTime / 10000;
        auto processTimeMs = kernelTimeMs + userTimeMs;

        return (processTimeMs / static_cast<double>(totalTimeMs)) * 100.0;
    }
#elif defined(__linux__) || defined(__APPLE__)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        double totalTimeMs = (usage.ru_utime.tv_sec + usage.ru_stime.tv_sec) * 1000.0 +
                             (usage.ru_utime.tv_usec + usage.ru_stime.tv_usec) / 1000.0;
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        double nowTimeMs = now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;

        return (totalTimeMs / nowTimeMs) * 100.0;
    }
#endif
    return -1;
}

double PerformanceMonitor::getMemoryUsage()
{
#if defined(_WIN32)
    // Windows implementation
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
        return virtualMemUsedByMe / (1024.0 * 1024.0); // Convert bytes to MB
    }
#elif defined(__linux__) || defined(__APPLE__)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        return usage.ru_maxrss / 1024.0;
    }
#endif
    return -1;
}