#pragma once

#include <iostream>
#include <vector>

/// UtilsInternals namespace, which contains the Utils class for various utility functions \namespace UtilsInternals
namespace UtilsInternals
{
    /// @brief The Utils class, a class for various utility functions \class Utils
    class Utils
    {
    public:

        /**
         * @brief Construct a new Utils object
         */
        Utils();

        /**
         * @brief Destroy the Utils object
         */
        ~Utils();

        /**
         * @brief Prints the contents of a vector of bytes.
         * @param data The vector of bytes to print.
         */
        static void printBytes(const std::vector<uint8_t>& data);

        /**
         * @brief Converts Bytes to a uint32_t in little-endian format.
         * @param data A vector of bytes.
         * @param offset The offset in the vector where the uint32_t starts.
         * @return A uint32_t value converted from the bytes at the specified offset.
         */
        static uint32_t bytesToUInt32LE(const std::vector<uint8_t>& data, size_t offset);

        /**
         * @brief Converts Bytes to a uint16_t in little-endian format.
         * @param data A vector of bytes.
         * @param offset The offset in the vector where the uint16_t starts.
         * @return A uint16_t value converted from the bytes at the specified offset.
         */
        static uint16_t bytesToUInt16LE(const std::vector<uint8_t>& data, size_t offset);

        /**
         * @brief Converts Bytes to a string.
         * @param data A vector of bytes.
         * @param offset The offset in the vector where the string starts.
         * @param length The length of the string to convert.
         * @return A string converted from the bytes at the specified offset and length.
         */
        static std::string bytesToString(const std::vector<uint8_t>& data, size_t offset, size_t length);

        /**
         * @brief Calculates the checksum of a file.
         * @param fileData A vector of bytes representing the file data.
         * @return A uint32_t representing the checksum of the file.
         */
        static uint32_t calculateChecksum(const std::vector<uint8_t>& fileData);
    };
}
