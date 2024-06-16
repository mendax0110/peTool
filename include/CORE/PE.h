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

/**
 * @brief The namespace for the Portable Executable (PE) file format
 * @namespace PeInternals

 */
namespace PeInternals
{
    class ResourceDirectory;

    /**
     * @brief The ResourceDirectoryTraverser class, a base class for traversing the resource directory
     * @class ResourceDirectoryTraverser
     */
    class ResourceDirectoryTraverser
    {
    public:
        virtual void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) = 0;
        virtual ~ResourceDirectoryTraverser() {}
    };

#if defined(_WIN32)
    /**
     * @brief The WinResourceDirectoryTraverser class, a class for traversing the resource directory on Windows
     * @class WinResourceDirectoryTraverser
     */
    class WinResourceDirectoryTraverser : public ResourceDirectoryTraverser
    {
    public:
        void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) override;
    };
#endif

#if defined(__linux__)
    /**
     * @brief The LinuxResourceDirectoryTraverser class, a class for traversing the resource directory on Linux
     * @class LinuxResourceDirectoryTraverser
     */
    class LinuxResourceDirectoryTraverser : public ResourceDirectoryTraverser
    {
    public:
        void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) override;
    };
#endif

#if defined(__APPLE__)
    /**
     * @brief The AppleResourceDirectoryTraverser class, a class for traversing the resource directory on Apple
     * @class AppleResourceDirectoryTraverser
     */
    class AppleResourceDirectoryTraverser : public ResourceDirectoryTraverser
    {
    public:
        void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) override;
    };
#endif

    /**
     * @brief The ResourceDirectoryTraverserFactory class, a factory class for creating resource directory traversers
     * @class ResourceDirectoryTraverserFactory
     */
    class ResourceDirectoryTraverserFactory
    {
    public:
        static ResourceDirectoryTraverser* createTraverser();
    };

    /**
     * @brief The ResourceDirectory class, a class for traverse the resource directory
     * @class ResourceDirectory
     */
    class ResourceDirectory
    {
    public:
        void traverse(const std::vector<uint8_t>& fileData, int level, const std::string& parentName);
        void setTraverser(ResourceDirectoryTraverser* traverser);
    private:
        ResourceDirectoryTraverser* traverser;
    };

    /**
     * @brief The PE class, a class for parsing
     * @class PE
     */
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
