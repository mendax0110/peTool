#include "../include/FILEIO/FileIO.h"
#include <fstream>

using namespace FileIoInternals;

/**
 * @brief Read a file into a vector of bytes
 * @param filename The name of the file to read
 * @return The file data
 */
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

/**
 * @brief Write data to a file
 * @param filename The name of the file to write to
 * @param data The data to write
 * @return True if the write was successful, false otherwise
 */
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

/**
 * @brief Get the size of a file
 * @param filename The name of the file
 * @return The size of the file
 */
std::streamsize FileIO::getFileSize(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file)
    {
        return file.tellg();
    }
    return -1;
}

/**
 * @brief Check if a file exists
 * @param filename The name of the file
 * @return True if the file exists, false otherwise
 */
bool FileIO::fileExists(const std::string& filename)
{
    std::ifstream file(filename);
    return file.good();
}

/**
 * @brief Backup a file by copying it to a new file with a .bak extension
 * @param filename The name of the file to backup
 * @return True if the backup was successful, false otherwise
 */
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

/**
 * @brief Delete a file
 * @param filename The name of the file to delete
 * @return True if the file was deleted, false otherwise
 */
bool FileIO::deleteFile(const std::string& filename)
{
    if (std::remove(filename.c_str()) != 0)
    {
        return false;
    }
    return true;
}
