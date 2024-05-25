#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#include <sys/resource.h>
#endif

class PerformanceMonitor
{
public:
    static void start(const char* name);
    static void stop(const char* name);
    static void reset();
    static void report();

private:
    struct PerformanceData {
        const char* name;
        double time;
    };

    static std::vector<PerformanceData> performanceData;
    static std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    static std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

    static double getCPUUsage();
    static double getMemoryUsage();
};