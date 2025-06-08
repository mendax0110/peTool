#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>

/// @brief Base class for memory profilers, which defines the interface for platform-specific memory profiling classes \class BaseMemProfiler
class BaseMemProfiler
{
public:

    /**
     * @brief Get the memory usage information
     * @return A vector of pairs containing the memory usage information
     */
    virtual std::vector<std::pair<std::string, size_t>> getMemoryUsage() = 0;

    /**
     * @brief Get the RAM usage information
     * @return A vector of pairs containing the RAM usage information
     */
    virtual std::vector<std::pair<std::string, size_t>> getRAMUsage() = 0;

    /**
     * @brief Get the VRAM usage information
     * @return A vector of pairs containing the VRAM usage information
     */
    virtual std::vector<std::pair<std::string, size_t>> getVRAMUsage() = 0;

    /**
     * @brief Virtual destructor for the base memory profiler class
     */
    virtual ~BaseMemProfiler() = default;
};

/// @brief The Windows memory profiler class, which provides methods for profiling memory usage on Windows \class WinMemProfiler
class WinMemProfiler : public BaseMemProfiler
{
public:

    /**
     * @brief Get the memory usage information on Windows
     * @return A vector of pairs containing the memory usage information
     */
    std::vector<std::pair<std::string, size_t>> getMemoryUsage() override;

    /**
     * @brief Get the RAM usage information on Windows
     * @return A vector of pairs containing the RAM usage information
     */
    std::vector<std::pair<std::string, size_t>> getRAMUsage() override;

    /**
     * @brief Get the VRAM usage information on Windows
     * @return A vector of pairs containing the VRAM usage information
     */
    std::vector<std::pair<std::string, size_t>> getVRAMUsage() override;
};

/// @brief The macOS memory profiler class, which provides methods for profiling memory usage on macOS \class MacMemProfiler
class MacMemProfiler : public BaseMemProfiler
{
public:

    /**
     * @brief Get the memory usage information on macOS
     * @return A vector of pairs containing the memory usage information
     */
    std::vector<std::pair<std::string, size_t>> getMemoryUsage() override;

    /**
     * @brief Get the RAM usage information on macOS
     * @return A vector of pairs containing the RAM usage information
     */
    std::vector<std::pair<std::string, size_t>> getRAMUsage() override;

    /**
     * @brief Get the VRAM usage information on macOS
     * @return A vector of pairs containing the VRAM usage information
     */
    std::vector<std::pair<std::string, size_t>> getVRAMUsage() override;
};

/// @brief The MemProfiler class, which provides methods for profiling memory usage across different platforms \class MemProfiler
class MemProfiler
{
private:
    std::unique_ptr<BaseMemProfiler> profiler;
    std::vector<std::pair<std::string, size_t>> memoryUsage;
    std::vector<std::pair<std::string, size_t>> ramUsage;
    std::vector<std::pair<std::string, size_t>> vramUsage;

public:

    /**
     * @brief Construct a new MemProfiler object
     */
    MemProfiler();

    /**
     * @brief Populate memory usage information
     */
    void populateMemoryUsage();

    /**
     * @brief Populate RAM usage information
     */
    void populateRAMUsage();

    /**
     * @brief Populate VRAM usage information
     */
    void populateVRAMUsage();

    /**
     * @brief Get the memory usage information
     * @return A vector of pairs containing the memory usage information
     */
    size_t getTotalMemoryUsage();

    /**
     * @brief Get the RAM usage information
     * @return A vector of pairs containing the RAM usage information
     */
    size_t getTotalRAMUsage();

    /**
     * @brief Get the VRAM usage information
     * @return A vector of pairs containing the VRAM usage information
     */
    size_t getTotalVRAMUsage();

    /**
     * @brief Print the memory usage information
     */
    void printMemoryUsage();

    /**
     * @brief Print the RAM usage information
     */
    void printRAMUsage();

    /**
     * @brief Print the VRAM usage information
     */
    void printVRAMUsage();
};
