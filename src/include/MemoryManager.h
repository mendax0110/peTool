#pragma once

#include <iostream>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cstring>

class MemoryManager
{
public:
    MemoryManager();
    ~MemoryManager();

    uint8_t* allocateMemory(size_t size);
    void deallocateMemory(uint8_t* ptr);
    uint8_t* reallocateMemory(uint8_t* ptr, size_t size);
    void copyToMemory(uint8_t* dest, const uint8_t* src, size_t size);
    void setMemory(uint8_t* dest, uint8_t value, size_t size);
    int compareMemory(const uint8_t* ptr1, const uint8_t* ptr2, size_t size);
    void zeroMemory(uint8_t* ptr, size_t size);
    uint8_t* duplicateMemory(const uint8_t* ptr, size_t size);

private:
    size_t allocatedMemory;

protected:
};