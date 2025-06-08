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
#include <sys/time.h>
#include <sys/sysctl.h>
#endif

/// @brief PerformanceMonitor class, which provides methods for monitoring performance metrics such as CPU usage and memory usage \class PerformanceMonitor
class PerformanceMonitor
{
public:

    /**
     * @brief Start monitoring performance for a specific named task.
     * @param name The name of the task to monitor.
     */
    static void start(const char* name);

    /**
     * @brief Stop monitoring performance for a specific named task.
     * @param name The name of the task to stop monitoring.
     */
    static void stop(const char* name);

    /**
     * @brief Reset the performance data collected so far.
     */
    static void reset();

    /**
     * @brief Report the performance data collected so far, including CPU usage and memory usage.
     * This function will print the performance data to the console.
     */
    static void report();

private:

    /// @brief Struct to hold performance data for a specific task \struct PerformanceData
    struct PerformanceData
    {
        const char* name;
        double time;
    };

    static std::vector<PerformanceData> performanceData;
    static std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    static std::chrono::time_point<std::chrono::high_resolution_clock> endTime;

    /**
     * @brief Get the current CPU usage as a percentage.
     * @return A double representing the CPU usage percentage.
     */
    static double getCPUUsage();

    /**
     * @brief Get the current memory usage in bytes.
     * @return A double representing the memory usage in bytes.
     */
    static double getMemoryUsage();
};