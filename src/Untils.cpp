#include "./include/Untils.h"
#include <iomanip>
#include <sstream>

using namespace UntilsInternals;

void Untils::printBytes(const std::vector<uint8_t>& data)
{
    for (const auto& byte : data)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        if ((&byte - &data[0] + 1) % 16 == 0)
            std::cout << std::endl;
    }
    std::cout << std::endl;
}

uint32_t Untils::bytesToUInt32LE(const std::vector<uint8_t>& data, size_t offset)
{
    uint32_t result = 0;
    for (size_t i = 0; i < sizeof(uint32_t); ++i)
        result |= static_cast<uint32_t>(data[offset + i]) << (8 * i);
    return result;
}

uint16_t Untils::bytesToUInt16LE(const std::vector<uint8_t>& data, size_t offset)
{
    uint16_t result = 0;
    for (size_t i = 0; i < sizeof(uint16_t); ++i)
        result |= static_cast<uint16_t>(data[offset + i]) << (8 * i);
    return result;
}

std::string Untils::bytesToString(const std::vector<uint8_t>& data, size_t offset, size_t length)
{
    std::string result(data.begin() + offset, data.begin() + offset + length);
    return result;
}

uint32_t Untils::calculateChecksum(const std::vector<uint8_t>& fileData)
{
    uint32_t checksum = 0;
    for (size_t i = 0; i < fileData.size(); i += sizeof(uint32_t))
    {
        uint32_t word = Untils::bytesToUInt32LE(fileData, i);
        checksum += word;
    }
    return checksum;
}
