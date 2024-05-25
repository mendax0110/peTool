#include "MemoryManager.h"

std::map<void*, MemoryManager::AllocationInfo> MemoryManager::allocations;
std::mutex MemoryManager::allocationMutex;

void* MemoryManager::allocate(size_t size, const char* file, int line)
{
    void* pointer = malloc(size);
    if (pointer)
    {
        std::lock_guard<std::mutex> lock(allocationMutex);
        allocations[pointer] = {size, file, line};
    }
    return pointer;
}

void MemoryManager::deallocate(void* pointer, const char* file, int line)
{
    std::lock_guard<std::mutex> lock(allocationMutex);
    auto it = allocations.find(pointer);
    if (it != allocations.end())
    {
        allocations.erase(it);
    }
    else
    {
        std::cerr << "Double free or invalid free detected at " << file << ":" << line << "\n";
    }
    free(pointer);
}

void MemoryManager::detectMemoryLeaks()
{
    std::lock_guard<std::mutex> lock(allocationMutex);
    if (!allocations.empty())
    {
        std::cerr << "Memory leaks detected:\n";
        for (const auto& entry : allocations)
        {
            std::cerr << "Leaked " << entry.second.size << " bytes at address " << entry.first << " allocated in file " << entry.second.file << " at line " << entry.second.line << "\n";
        }
    }
    else
    {
        std::cerr << "No memory leaks detected.\n";
    }
}

void MemoryManager::reportLeak(void* pointer, size_t size, const char* file, int line)
{
    if (pointer)
    {
        std::lock_guard<std::mutex> lock(allocationMutex);
        allocations[pointer] = {size, file, line};
    }
}

void* MemoryManager::operator new(size_t size, const char* file, int line)
{
    return allocate(size, file, line);
}

void* MemoryManager::operator new[](size_t size, const char* file, int line)
{
    return allocate(size, file, line);
}

void MemoryManager::operator delete(void* pointer, const char* file, int line)
{
    deallocate(pointer, file, line);
}

void MemoryManager::operator delete[](void* pointer, const char* file, int line)
{
    deallocate(pointer, file, line);
}

void MemoryManager::operator delete(void* pointer) noexcept
{
    std::lock_guard<std::mutex> lock(allocationMutex);
    allocations.erase(pointer);
    free(pointer);
}

void MemoryManager::operator delete[](void* pointer) noexcept
{
    std::lock_guard<std::mutex> lock(allocationMutex);
    allocations.erase(pointer);
    free(pointer);
}
