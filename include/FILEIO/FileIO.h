#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

/// @brief FileIoInternals namespace, which contains the FileIO class for reading and writing files \namespace FileIoInternals
namespace FileIoInternals
{
    /// @brief The FileIO class, a class for reading and writing files \class FileIO
    class FileIO
    {
    public:

        /**
         * @brief Reads the contents of a file into a vector of bytes.
         * @param filename The name of the file to read
         * @return A vector of bytes containing the file data
         */
        static std::vector<uint8_t> readFile(const std::string& filename);

        /**
         * @brief Writes the contents of a vector of bytes to a file.
         * @param filename The name of the file to write to
         * @param data The vector of bytes to write
         * @return true if the write operation was successful, false otherwise
         */
        static bool writeFile(const std::string& filename, const std::vector<uint8_t>& data);

        /**
         * @brief Gets the size of a file.
         * @param filename The name of the file to get the size of
         * @return A streamsize representing the size of the file in bytes
         */
        static std::streamsize getFileSize(const std::string& filename);

        /**
         * @brief Checks if a file exists.
         * @param filename The name of the file to check
         * @return true if the file exists, false otherwise
         */
        static bool fileExists(const std::string& filename);

        /**
         * @brief Backs up a file by copying it.
         * @param filename The name of the file to back up
         * @return A boolean indicating whether the backup was successful
         */
        static bool backupFile(const std::string& filename);

        /**
         * @brief Deletes a file.
         * @param filename The name of the file to delete
         * @return true if the file was successfully deleted, false otherwise
         */
        static bool deleteFile(const std::string& filename);
    };
}
