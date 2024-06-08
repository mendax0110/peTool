#include "../include/MANMON/MemProfiler.h"

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#endif

#if defined(__APPLE__)
#include <sys/sysctl.h>
#endif

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

std::vector<std::pair<std::string, size_t>> WinMemProfiler::getVRAMUsage()
{
    std::vector<std::pair<std::string, size_t>> vramUsage;
#if defined(_WIN32)
    // TODO: Implement Windows specific VRAM usage retrieval
#endif
    return vramUsage;
}


std::vector<std::pair<std::string, size_t>> MacMemProfiler::getMemoryUsage()
{
    std::vector<std::pair<std::string, size_t>> memoryUsage;
#if defined(__APPLE__)
    // TODO: Implement macOS specific memory usage retrieval
#endif
    return memoryUsage;
}

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

std::vector<std::pair<std::string, size_t>> MacMemProfiler::getVRAMUsage()
{
    std::vector<std::pair<std::string, size_t>> vramUsage;
#if defined(__APPLE__)
    // TODO: Implement macOS specific VRAM usage retrieval
#endif
    return vramUsage;
}

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

void MemProfiler::populateMemoryUsage()
{
    memoryUsage = profiler->getMemoryUsage();
}

void MemProfiler::populateRAMUsage()
{
    ramUsage = profiler->getRAMUsage();
}

void MemProfiler::populateVRAMUsage()
{
    vramUsage = profiler->getVRAMUsage();
}

size_t MemProfiler::getTotalMemoryUsage()
{
    size_t totalMemoryUsage = 0;
    for (const auto& usage : memoryUsage)
    {
        totalMemoryUsage += usage.second;
    }
    return totalMemoryUsage;
}

size_t MemProfiler::getTotalRAMUsage()
{
    size_t totalRAMUsage = 0;
    for (const auto& usage : ramUsage)
    {
        totalRAMUsage += usage.second;
    }
    return totalRAMUsage;
}

size_t MemProfiler::getTotalVRAMUsage()
{
    size_t totalVRAMUsage = 0;
    for (const auto& usage : vramUsage)
    {
        totalVRAMUsage += usage.second;
    }
    return totalVRAMUsage;
}

void MemProfiler::printMemoryUsage()
{
    std::cout << "Memory usage:" << std::endl;
    for (const auto& usage : memoryUsage)
    {
        std::cout << "  " << usage.first << ": " << usage.second << " bytes" << std::endl;
    }
    std::cout << "Total memory usage: " << getTotalMemoryUsage() << " bytes" << std::endl;
}

void MemProfiler::printRAMUsage()
{
    std::cout << "RAM usage:" << std::endl;
    for (const auto& usage : ramUsage)
    {
        std::cout << "  " << usage.first << ": " << usage.second << " bytes" << std::endl;
    }
    std::cout << "Total RAM usage: " << getTotalRAMUsage() << " bytes" << std::endl;
}

void MemProfiler::printVRAMUsage()
{
    std::cout << "VRAM usage:" << std::endl;
    for (const auto& usage : vramUsage)
    {
        std::cout << "  " << usage.first << ": " << usage.second << " bytes" << std::endl;
    }
    std::cout << "Total VRAM usage: " << getTotalVRAMUsage() << " bytes" << std::endl;
}
