#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>

/**
 * @brief The base memory profiler class
 * @class BaseMemProfiler
 * @details This class is an interface for memory profiling, a base class for platform-specific memory profilings
 */
class BaseMemProfiler
{
public:
    virtual std::vector<std::pair<std::string, size_t>> getMemoryUsage() = 0;
    virtual std::vector<std::pair<std::string, size_t>> getRAMUsage() = 0;
    virtual std::vector<std::pair<std::string, size_t>> getVRAMUsage() = 0;
    virtual ~BaseMemProfiler() = default;
};


/**
 * @brief The Windows memory profiler class
 * @class WinMemProfiler
 * @details This class is a platform-specific memory profiler for Windows
 */
class WinMemProfiler : public BaseMemProfiler
{
public:
    std::vector<std::pair<std::string, size_t>> getMemoryUsage() override;
    std::vector<std::pair<std::string, size_t>> getRAMUsage() override;
    std::vector<std::pair<std::string, size_t>> getVRAMUsage() override;
};

/**
 * @brief The MacOS memory profiler class
 * @class MacMemProfiler
 * @details This class is a platform-specific memory profiler for MacOS
 */
class MacMemProfiler : public BaseMemProfiler
{
public:
    std::vector<std::pair<std::string, size_t>> getMemoryUsage() override;
    std::vector<std::pair<std::string, size_t>> getRAMUsage() override;
    std::vector<std::pair<std::string, size_t>> getVRAMUsage() override;
};

/**
 * @brief The Memory profiler class
 * @class MemProfiler
 * @details This class is a memory profiler that uses platform-specific memory profilers
 */
class MemProfiler
{
private:
    std::unique_ptr<BaseMemProfiler> profiler;
    std::vector<std::pair<std::string, size_t>> memoryUsage;
    std::vector<std::pair<std::string, size_t>> ramUsage;
    std::vector<std::pair<std::string, size_t>> vramUsage;

public:
    MemProfiler();
    void populateMemoryUsage();
    void populateRAMUsage();
    void populateVRAMUsage();
    size_t getTotalMemoryUsage();
    size_t getTotalRAMUsage();
    size_t getTotalVRAMUsage();
    void printMemoryUsage();
    void printRAMUsage();
    void printVRAMUsage();
};
