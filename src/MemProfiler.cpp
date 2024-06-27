#include "../include/MANMON/MemProfiler.h"

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#include <dxgi.h>
#include <iostream>
#include <vector>
#include <utility>
#endif

#if defined(__APPLE__)
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#include <mach/mach_error.h>
#include <mach/task_info.h>
#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CFNumber.h>
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
        memoryUsage.emplace_back(std::make_pair("Pagefile usage", pmc.PagefileUsage));
        memoryUsage.emplace_back(std::make_pair("Peak working set size", pmc.PeakWorkingSetSize));
        memoryUsage.emplace_back(std::make_pair("Working set size", pmc.WorkingSetSize));
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
    if (GlobalMemoryStatusEx(&memInfo))
    {
        ramUsage.emplace_back(std::make_pair("Total physical memory", memInfo.ullTotalPhys));
        ramUsage.emplace_back(std::make_pair("Physical memory currently available", memInfo.ullAvailPhys));
        ramUsage.emplace_back(std::make_pair("Total virtual memory", memInfo.ullTotalVirtual));
        ramUsage.emplace_back(std::make_pair("Virtual memory currently available", memInfo.ullAvailVirtual));
    }
    else
    {
        std::cerr << "Failed to retrieve memory status." << std::endl;
    }

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
    IDXGIFactory* pFactory = nullptr;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);

    if (SUCCEEDED(hr))
    {
        IDXGIAdapter* pAdapter = nullptr;

        if (pFactory->EnumAdapters(0, &pAdapter) != DXGI_ERROR_NOT_FOUND)
        {
            DXGI_ADAPTER_DESC adapterDesc;
            pAdapter->GetDesc(&adapterDesc);
            vramUsage.emplace_back(std::make_pair("Dedicated video memory", adapterDesc.DedicatedVideoMemory));
            vramUsage.emplace_back(std::make_pair("Dedicated system memory", adapterDesc.DedicatedSystemMemory));
            vramUsage.emplace_back(std::make_pair("Shared system memory", adapterDesc.SharedSystemMemory));
            pAdapter->Release();
        }
        pFactory->Release();
    }
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
    auto task = mach_task_self();
    struct task_basic_info t_info{};
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    if (task_info(task, TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count) == KERN_SUCCESS)
    {
        memoryUsage.emplace_back("Virtual size", t_info.virtual_size);
        memoryUsage.emplace_back("Resident size", t_info.resident_size);
    }

    struct vm_statistics64 vm_stat{};
    mach_msg_type_number_t vm_stat_count = HOST_VM_INFO64_COUNT;
    if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vm_stat, &vm_stat_count) == KERN_SUCCESS)
    {
        memoryUsage.emplace_back("Free memory", vm_stat.free_count * vm_page_size);
        memoryUsage.emplace_back("Active memory", vm_stat.active_count * vm_page_size);
        memoryUsage.emplace_back("Inactive memory", vm_stat.inactive_count * vm_page_size);
        memoryUsage.emplace_back("Wired memory", vm_stat.wire_count * vm_page_size);
    }
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
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    int64_t physicalMemory;
    size_t length = sizeof(physicalMemory);
    if (sysctl(mib, 2, &physicalMemory, &length, nullptr, 0) == 0)
    {
        ramUsage.emplace_back("Physical memory", physicalMemory);
    }
    else
    {
        std::cerr << "Failed to retrieve RAM info." << std::endl;
    }
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
    io_service_t device = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("IOAccelerator"));
    if (device)
    {
        uint64_t vram;
        auto vramNumber = (CFNumberRef)IORegistryEntryCreateCFProperty(device, CFSTR("VRAMTotal"), kCFAllocatorDefault, 0);
        if (vramNumber)
        {
            CFNumberGetValue(vramNumber, kCFNumberSInt64Type, &vram);
            vramUsage.emplace_back("Total VRAM", vram);
            CFRelease(vramNumber);
        }
        IOObjectRelease(device);
    }
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
