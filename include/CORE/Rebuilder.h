#pragma once

#include <iostream>
#include <vector>
#include <string>

/**
 * @brief The Rebuilder class
 * @class Rebuilder
 */
class Rebuilder
{
public:
    Rebuilder();
    ~Rebuilder();

    static bool fixDump(const std::string& filePath);
    static bool wipeLocations(const std::string& filePath);
    static bool rebuildResourceDirectory(const std::string& filePath);
    static bool validatePEFile(const std::string& filePath);
    static bool bindImports(const std::string& filePath);
    static bool changeImageBase(const std::string& filePath, uint64_t newImageBase);

private:
    static std::vector<uint8_t> readFile(const std::string& filePath);
    static bool writeFile(const std::string& filePath, const std::vector<uint8_t>& data);

    static bool fixDumpInternal(std::vector<uint8_t>& data);
    static bool wipeLocationsInternal(std::vector<uint8_t>& data);
    static bool rebuildResourceDirectoryInternal(std::vector<uint8_t>& data);
    static bool validatePEFileInternal(std::vector<uint8_t>& data);
    static bool bindImportsInternal(std::vector<uint8_t>& data);
    static bool changeImageBaseInternal(std::vector<uint8_t>& data, uint64_t newImageBase);

};