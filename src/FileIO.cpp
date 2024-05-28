#include "../include/FILEIO/FileIO.h"
#include <fstream>

using namespace FileIoInternals;

std::vector<uint8_t> FileIO::readFile(const std::string& filename)
{
    std::vector<uint8_t> fileData;
    std::ifstream file(filename, std::ios::binary);
    if (file)
    {
        file.seekg(0, std::ios::end);
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        fileData.resize(fileSize);
        if (!file.read(reinterpret_cast<char*>(fileData.data()), fileSize))
        {
            fileData.clear();
        }
    }
    return fileData;
}

bool FileIO::writeFile(const std::string& filename, const std::vector<uint8_t>& data)
{
    std::ofstream file(filename, std::ios::binary);
    if (file)
    {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }
    return false;
}

std::streamsize FileIO::getFileSize(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file)
    {
        return file.tellg();
    }
    return -1;
}

bool FileIO::fileExists(const std::string& filename)
{
    std::ifstream file(filename);
    return file.good();
}

bool FileIO::backupFile(const std::string& filename)
{
    std::ifstream source(filename, std::ios::binary);
    if (!source)
    {
        return false;
    }

    std::ofstream dest(filename + ".bak", std::ios::binary);
    if (!dest)
    {
        return false;
    }

    dest << source.rdbuf();
    return true;
}

bool FileIO::deleteFile(const std::string& filename)
{
    if (std::remove(filename.c_str()) != 0)
    {
        return false;
    }
    return true;
}
