#include "../include/MANMON/MemoryManager.h"

std::map<void*, MemoryManager::AllocationInfo> MemoryManager::allocations;
std::mutex MemoryManager::allocationMutex;

/**
 * @brief Allocate memory and track the allocation
 * @param size The size of the memory to allocate
 * @param file The file where the allocation was made
 * @param line The line where the allocation was made
 * @return The pointer to the allocated memory
 */
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

/**
 * @brief Deallocate memory and remove the allocation from tracking
 * @param pointer The pointer to the memory to deallocate
 * @param file The file where the deallocation was made
 * @param line The line where the deallocation was made
 */
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

/**
 * @brief Detect memory leaks and print the results
 */
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

/**
 * @brief Report a memory leak
 * @param pointer The pointer to the leaked memory
 * @param size The size of the leaked memory
 * @param file The file where the leak was detected
 * @param line The line where the leak was detected
 */
void MemoryManager::reportLeak(void* pointer, size_t size, const char* file, int line)
{
    if (pointer)
    {
        std::lock_guard<std::mutex> lock(allocationMutex);
        allocations[pointer] = {size, file, line};
    }
}

/**
 * @brief Overloaded new operator that tracks memory allocations
 * @param size The size of the memory to allocate
 * @param file The file where the allocation was made
 * @param line The line where the allocation was made
 * @return The pointer to the allocated memory
 */
void* MemoryManager::operator new(size_t size, const char* file, int line)
{
    return allocate(size, file, line);
}

/**
 * @brief Overloaded new[] operator that tracks memory allocations
 * @param size The size of the memory to allocate
 * @param file The file where the allocation was made
 * @param line The line where the allocation was made
 * @return The pointer to the allocated memory
 */
void* MemoryManager::operator new[](size_t size, const char* file, int line)
{
    return allocate(size, file, line);
}

/**
 * @brief Overloaded delete operator that tracks memory deallocations
 * @param pointer The pointer to the memory to deallocate
 * @param file The file where the deallocation was made
 * @param line The line where the deallocation was made
 */
void MemoryManager::operator delete(void* pointer, const char* file, int line)
{
    deallocate(pointer, file, line);
}

/**
 * @brief Overloaded delete[] operator that tracks memory deallocations
 * @param pointer The pointer to the memory to deallocate
 * @param file The file where the deallocation was made
 * @param line The line where the deallocation was made
 */
void MemoryManager::operator delete[](void* pointer, const char* file, int line)
{
    deallocate(pointer, file, line);
}

/**
 * @brief Overloaded delete operator that tracks memory deallocations
 * @param pointer The pointer to the memory to deallocate
 */
void MemoryManager::operator delete(void* pointer) noexcept
{
    std::lock_guard<std::mutex> lock(allocationMutex);
    allocations.erase(pointer);
    free(pointer);
}

/**
 * @brief Overloaded delete[] operator that tracks memory deallocations
 * @param pointer The pointer to the memory to deallocate
 */
void MemoryManager::operator delete[](void* pointer) noexcept
{
    std::lock_guard<std::mutex> lock(allocationMutex);
    allocations.erase(pointer);
    free(pointer);
}
