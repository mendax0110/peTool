#pragma once

#ifdef USE_CUSTOM_MEMORY_MANAGER
#define new new(__FILE__, __LINE__)
#define delete MemoryManager::reportLeak(__FILE__, __LINE__), delete
#endif

#include <map>
#include <mutex>
#include <cstdlib>
#include <iostream>
#include <new>

/// @brief MemoryManager class, which provides methods for memory allocation, deallocation, and leak detection \class MemoryManager
class MemoryManager
{
public:

    /**
     * @brief Allocates memory of the specified size and tracks the allocation for leak detection.
     * @param size The size of memory to allocate in bytes.
     * @param file The file where the allocation is made, for tracking purposes.
     * @param line The line number where the allocation is made, for tracking purposes.
     * @return A pointer to the allocated memory.
     */
    static void* allocate(size_t size, const char* file, int line);

    /**
     * @brief Deallocates memory and removes the allocation from tracking.
     * @param pointer The pointer to the memory to deallocate.
     * @param file The file where the deallocation is made, for tracking purposes.
     * @param line The line number where the deallocation is made, for tracking purposes.
     */
    static void deallocate(void* pointer, const char* file, int line);

    /**
     * @brief Checks for memory leaks and reports them.
     */
    static void detectMemoryLeaks();

    /**
     * @brief Reports a memory leak.
     * @param pointer The pointer to the leaked memory.
     * @param size The size of the leaked memory.
     * @param file The file where the leak occurred.
     * @param line The line number where the leak occurred.
     */
    static void reportLeak(void* pointer, size_t size, const char* file, int line);


    static void* operator new(size_t size, const char* file, int line);
    static void* operator new[](size_t size, const char* file, int line);
    static void operator delete(void* pointer, const char* file, int line);
    static void operator delete[](void* pointer, const char* file, int line);
    static void operator delete(void* pointer) noexcept;
    static void operator delete[](void* pointer) noexcept;

private:

    /// @brief Struct to hold information about each memory allocation for leak detection \struct AllocationInfo
    struct AllocationInfo
    {
        size_t size;
        const char* file;
        int line;
    };

    static std::map<void*, AllocationInfo> allocations;
    static std::mutex allocationMutex;
};
