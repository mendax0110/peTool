#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(__LINUX)
#include <elf.h>
#endif

#if defined(__APPLE__)
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#endif

namespace PeInternals
{
    class ResourceDirectory;
    class ResourceDirectoryTraverser
    {
    public:
        virtual void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) = 0;
        virtual ~ResourceDirectoryTraverser() {}
    };

#if defined(_WIN32)
    class WinResourceDirectoryTraverser : public ResourceDirectoryTraverser
    {
    public:
        void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) override;
    };
#endif

#if defined(__linux__)
    class LinuxResourceDirectoryTraverser : public ResourceDirectoryTraverser
    {
    public:
        void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) override;
    };
#endif

#if defined(__APPLE__)
    class AppleResourceDirectoryTraverser : public ResourceDirectoryTraverser
    {
    public:
        void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) override;
    };
#endif

    class ResourceDirectoryTraverserFactory
    {
    public:
        static ResourceDirectoryTraverser* createTraverser();
    };

    class ResourceDirectory
    {
    public:
        void traverse(const std::vector<uint8_t>& fileData, int level, const std::string& parentName);
        void setTraverser(ResourceDirectoryTraverser* traverser);
    private:
        ResourceDirectoryTraverser* traverser;
    };

    class PE
    {
    public:
        static void parseHeaders(const std::vector<uint8_t>& fileData);
        static void extractSectionInfo(const std::vector<uint8_t>& fileData);
        static void extractImportTable(const std::vector<uint8_t>& fileData);
        static void extractExportTable(const std::vector<uint8_t>& fileData);
        static void extractResources(const std::vector<uint8_t>& fileData);
        static uint32_t GetResourceDirectoryOffset(const std::vector<uint8_t>& fileData);
        static std::vector<std::string> getFunctionNames(const std::vector<uint8_t>& fileData);
        static std::vector<std::string> getDllNames(const std::vector<uint8_t>& fileData);
        static std::vector<uint64_t> getFunctionAddresses(const std::vector<uint8_t>& fileData);
        static std::vector<uint64_t> getDllAddresses(const std::vector<uint8_t>& fileData);
    };
}
