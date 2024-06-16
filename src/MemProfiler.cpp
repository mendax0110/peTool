#include "../include/MANMON/MemProfiler.h"

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#endif

#if defined(__APPLE__)
#include <sys/sysctl.h>
#endif

/**
 * @brief Construct a new MemProfiler object
 */
MemProfiler::MemProfiler()
{
#if defined(_WIN32)
    profiler = std::make_unique<WinMemProfiler>();
#elif defined(__APPLE__)
    profiler = std::make_unique<MacMemProfiler>();
#else
    throw std::runtime_error("Platform not supported.");
#endif
}

/**
 * @brief Get the memory usage of the process
 * @return A vector of pairs containing the memory usage information
 */
std::vector<std::pair<std::string, size_t>> WinMemProfiler::getMemoryUsage()
{
    std::vector<std::pair<std::string, size_t>> memoryUsage;
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    HANDLE hProcess = GetCurrentProcess();
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
    {
        memoryUsage.push_back(std::make_pair("Pagefile usage", pmc.PagefileUsage));
        memoryUsage.push_back(std::make_pair("Peak working set size", pmc.PeakWorkingSetSize));
        memoryUsage.push_back(std::make_pair("Working set size", pmc.WorkingSetSize));
    }
    else
    {
        std::cerr << "Failed to retrieve process memory info." << std::endl;
    }
#endif
    return memoryUsage;
}

/**
 * @brief Get the RAM usage of the system
 * @return A vector of pairs containing the RAM usage information
 */
std::vector<std::pair<std::string, size_t>> WinMemProfiler::getRAMUsage()
{
    std::vector<std::pair<std::string, size_t>> ramUsage;
#if defined(_WIN32)
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    GlobalMemoryStatusEx(&memInfo);
    ramUsage.push_back(std::make_pair("Total physical memory", memInfo.ullTotalPhys));
    ramUsage.push_back(std::make_pair("Physical memory currently available", memInfo.ullAvailPhys));
    ramUsage.push_back(std::make_pair("Total virtual memory", memInfo.ullTotalVirtual));
    ramUsage.push_back(std::make_pair("Virtual memory currently available", memInfo.ullAvailVirtual));
#endif
    return ramUsage;
}

/**
 * @brief Get the VRAM usage of the system
 * @return A vector of pairs containing the VRAM usage information
 */
std::vector<std::pair<std::string, size_t>> WinMemProfiler::getVRAMUsage()
{
    std::vector<std::pair<std::string, size_t>> vramUsage;
#if defined(_WIN32)
    // TODO: Implement Windows specific VRAM usage retrieval
#endif
    return vramUsage;
}

/**
 * @brief Get the memory usage of the process
 * @return A vector of pairs containing the memory usage information
 */
std::vector<std::pair<std::string, size_t>> MacMemProfiler::getMemoryUsage()
{
    std::vector<std::pair<std::string, size_t>> memoryUsage;
#if defined(__APPLE__)
    // TODO: Implement macOS specific memory usage retrieval
#endif
    return memoryUsage;
}

/**
 * @brief Get the RAM usage of the system
 * @return A vector of pairs containing the RAM usage information
 */
std::vector<std::pair<std::string, size_t>> MacMemProfiler::getRAMUsage()
{
    std::vector<std::pair<std::string, size_t>> ramUsage;
#if defined(__APPLE__)
    int mib[2];
    int64_t physicalMemory;
    size_t length;

    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    length = sizeof(physicalMemory);
    sysctl(mib, 2, &physicalMemory, &length, nullptr, 0);

    ramUsage.emplace_back("Physical memory", physicalMemory);
#endif
    return ramUsage;
}

/**
 * @brief Get the VRAM usage of the system
 * @return A vector of pairs containing the VRAM usage information
 */
std::vector<std::pair<std::string, size_t>> MacMemProfiler::getVRAMUsage()
{
    std::vector<std::pair<std::string, size_t>> vramUsage;
#if defined(__APPLE__)
    // TODO: Implement macOS specific VRAM usage retrieval
#endif
    return vramUsage;
}

/**
 * @brief Populate the memory usage information
 */
void MemProfiler::populateMemoryUsage()
{
    memoryUsage = profiler->getMemoryUsage();
}

/**
 * @brief Populate the RAM usage information
 */
void MemProfiler::populateRAMUsage()
{
    ramUsage = profiler->getRAMUsage();
}

/**
 * @brief Populate the VRAM usage information
 */
void MemProfiler::populateVRAMUsage()
{
    vramUsage = profiler->getVRAMUsage();
}

/**
 * @brief Get the total memory usage
 * @return The total memory usage
 */
size_t MemProfiler::getTotalMemoryUsage()
{
    size_t totalMemoryUsage = 0;
    for (const auto& usage : memoryUsage)
    {
        totalMemoryUsage += usage.second;
    }
    return totalMemoryUsage;
}

/**
 * @brief Get the total RAM usage
 * @return The total RAM usage
 */
size_t MemProfiler::getTotalRAMUsage()
{
    size_t totalRAMUsage = 0;
    for (const auto& usage : ramUsage)
    {
        totalRAMUsage += usage.second;
    }
    return totalRAMUsage;
}

/**
 * @brief Get the total VRAM usage
 * @return The total VRAM usage
 */
size_t MemProfiler::getTotalVRAMUsage()
{
    size_t totalVRAMUsage = 0;
    for (const auto& usage : vramUsage)
    {
        totalVRAMUsage += usage.second;
    }
    return totalVRAMUsage;
}

/**
 * @brief Print the memory usage information
 */
void MemProfiler::printMemoryUsage()
{
    std::cout << "Memory usage:" << std::endl;
    for (const auto& usage : memoryUsage)
    {
        std::cout << "  " << usage.first << ": " << usage.second << " bytes" << std::endl;
    }
    std::cout << "Total memory usage: " << getTotalMemoryUsage() << " bytes" << std::endl;
}

/**
 * @brief Print the RAM usage information
 */
void MemProfiler::printRAMUsage()
{
    std::cout << "RAM usage:" << std::endl;
    for (const auto& usage : ramUsage)
    {
        std::cout << "  " << usage.first << ": " << usage.second << " bytes" << std::endl;
    }
    std::cout << "Total RAM usage: " << getTotalRAMUsage() << " bytes" << std::endl;
}

/**
 * @brief Print the VRAM usage information
 */
void MemProfiler::printVRAMUsage()
{
    std::cout << "VRAM usage:" << std::endl;
    for (const auto& usage : vramUsage)
    {
        std::cout << "  " << usage.first << ": " << usage.second << " bytes" << std::endl;
    }
    std::cout << "Total VRAM usage: " << getTotalVRAMUsage() << " bytes" << std::endl;
}
