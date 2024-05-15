#include "./include/MemoryManager.h"


MemoryManager::MemoryManager() : allocatedMemory(0)
{
}

MemoryManager::~MemoryManager()
{
}

uint8_t* MemoryManager::allocateMemory(size_t size)
{
    uint8_t* ptr = new (std::nothrow) uint8_t[size];
    if (ptr == nullptr)
    {
        std::cerr << "ERROR: Memory allocation failed" << std::endl;
        return nullptr;
    }
    return ptr;
}

void MemoryManager::deallocateMemory(uint8_t* ptr)
{
    delete[] ptr;
}

uint8_t* MemoryManager::reallocateMemory(uint8_t* ptr, size_t newSize)
{
    if (!ptr)
        return allocateMemory(newSize);

    uint8_t* newPtr = allocateMemory(newSize);
    if (newPtr)
    {
        copyToMemory(newPtr, ptr, newSize);
        deallocateMemory(ptr);
    }
    return newPtr;
}

void MemoryManager::copyToMemory(uint8_t* dest, const uint8_t* src, size_t size)
{
    if (dest != nullptr && src != nullptr)
    {
        std::memcpy(dest, src, size);
    }
    else
    {
        std::cerr << "ERROR: Null pointer passed to copyToMemory" << std::endl;
    }
}

void MemoryManager::setMemory(uint8_t* dest, uint8_t value, size_t size)
{
    if (dest != nullptr)
    {
        std::memset(dest, value, size);
    }
    else
    {
        std::cerr << "ERROR: Null pointer passed to setMemory" << std::endl;
    }
}

int MemoryManager::compareMemory(const uint8_t* ptr1, const uint8_t* ptr2, size_t size)
{
    if (ptr1 != nullptr && ptr2 != nullptr)
    {
        return std::memcmp(ptr1, ptr2, size);
    }
    else
    {
        std::cerr << "ERROR: Null pointer passed to compareMemory" << std::endl;
        return -1;
    }
}

void MemoryManager::zeroMemory(uint8_t* ptr, size_t size)
{
    if (ptr != nullptr)
    {
        std::memset(ptr, 0, size);
    }
    else
    {
        std::cerr << "ERROR: Null pointer passed to zeroMemory" << std::endl;
    }
}

uint8_t* MemoryManager::duplicateMemory(const uint8_t* src, size_t size)
{
    uint8_t* newPtr = allocateMemory(size);
    if (newPtr)
    {
        copyToMemory(newPtr, src, size);
    }
    return newPtr;
}