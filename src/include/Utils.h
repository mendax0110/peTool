#pragma once

#include <iostream>
#include <vector>

namespace UtilsInternals
{
    class Utils
    {
    public:
        Utils();
        ~Utils();
        static void printBytes(const std::vector<uint8_t>& data);
        static uint32_t bytesToUInt32LE(const std::vector<uint8_t>& data, size_t offset);
        static uint16_t bytesToUInt16LE(const std::vector<uint8_t>& data, size_t offset);
        static std::string bytesToString(const std::vector<uint8_t>& data, size_t offset, size_t length);
        static uint32_t calculateChecksum(const std::vector<uint8_t>& fileData);
    };
}
