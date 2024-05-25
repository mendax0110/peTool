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

class MemoryManager
{
public:
    static void* allocate(size_t size, const char* file, int line);
    static void deallocate(void* pointer, const char* file, int line);

    static void detectMemoryLeaks();
    static void reportLeak(void* pointer, size_t size, const char* file, int line);

    static void* operator new(size_t size, const char* file, int line);
    static void* operator new[](size_t size, const char* file, int line);
    static void operator delete(void* pointer, const char* file, int line);
    static void operator delete[](void* pointer, const char* file, int line);
    static void operator delete(void* pointer) noexcept;
    static void operator delete[](void* pointer) noexcept;

private:
    struct AllocationInfo {
        size_t size;
        const char* file;
        int line;
    };

    static std::map<void*, AllocationInfo> allocations;
    static std::mutex allocationMutex;
};
