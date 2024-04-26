#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <winnt.h>


namespace PeHeaderInternals
{
    class PEHeader
    {
    public:
        static void parseHeaders(const std::vector<uint8_t>& fileData);
        static void extractSectionInfo(const std::vector<uint8_t>& fileData);
        static void extractImportTable(const std::vector<uint8_t>& fileData);
        static void extractExportTable(const std::vector<uint8_t>& fileData);
        static void extractResources(const std::vector<uint8_t>& fileData);
        static void traverseResourceDirectory(const std::vector<uint8_t>& fileData, PIMAGE_RESOURCE_DIRECTORY resourceDirectory, int level, const std::string& parentName);
    };
}
