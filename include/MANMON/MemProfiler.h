#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <memory>

class BaseMemProfiler
{
public:
    virtual std::vector<std::pair<std::string, size_t>> getMemoryUsage() = 0;
    virtual std::vector<std::pair<std::string, size_t>> getRAMUsage() = 0;
    virtual std::vector<std::pair<std::string, size_t>> getVRAMUsage() = 0;
    virtual ~BaseMemProfiler() = default;
};


class WinMemProfiler : public BaseMemProfiler
{
public:
    std::vector<std::pair<std::string, size_t>> getMemoryUsage() override;
    std::vector<std::pair<std::string, size_t>> getRAMUsage() override;
    std::vector<std::pair<std::string, size_t>> getVRAMUsage() override;
};

class MacMemProfiler : public BaseMemProfiler
{
public:
    std::vector<std::pair<std::string, size_t>> getMemoryUsage() override;
    std::vector<std::pair<std::string, size_t>> getRAMUsage() override;
    std::vector<std::pair<std::string, size_t>> getVRAMUsage() override;
};

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
