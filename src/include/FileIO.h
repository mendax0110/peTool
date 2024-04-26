#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace FileIoInternals
{
    class FileIO
    {
    public:
        static std::vector<uint8_t> readFile(const std::string& filename);
        static bool writeFile(const std::string& filename, const std::vector<uint8_t>& data);
        static std::streamsize getFileSize(const std::string& filename);
        static bool fileExists(const std::string& filename);
        static bool backupFile(const std::string& filename);
        static bool deleteFile(const std::string& filename);
    };
}
