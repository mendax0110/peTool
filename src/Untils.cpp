#include "./include/Utils.h"
#include <iomanip>
#include <sstream>

using namespace UtilsInternals;

Utils::Utils()
{
}

Utils::~Utils()
{
}

void Utils::printBytes(const std::vector<uint8_t>& data)
{
    for (const auto& byte : data)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        if ((&byte - &data[0] + 1) % 16 == 0)
            std::cout << std::endl;
    }
    std::cout << std::endl;
}

uint32_t Utils::bytesToUInt32LE(const std::vector<uint8_t>& data, size_t offset)
{
    uint32_t result = 0;
    for (size_t i = 0; i < sizeof(uint32_t); ++i)
        result |= static_cast<uint32_t>(data[offset + i]) << (8 * i);
    return result;
}

uint16_t Utils::bytesToUInt16LE(const std::vector<uint8_t>& data, size_t offset)
{
    uint16_t result = 0;
    for (size_t i = 0; i < sizeof(uint16_t); ++i)
        result |= static_cast<uint16_t>(data[offset + i]) << (8 * i);
    return result;
}

std::string Utils::bytesToString(const std::vector<uint8_t>& data, size_t offset, size_t length)
{
    std::string result(data.begin() + offset, data.begin() + offset + length);
    return result;
}

uint32_t Utils::calculateChecksum(const std::vector<uint8_t>& fileData)
{
    uint32_t checksum = 0;
    for (size_t i = 0; i < fileData.size(); i += sizeof(uint32_t))
    {
        uint32_t word = Utils::bytesToUInt32LE(fileData, i);
        checksum += word;
    }
    return checksum;
}
