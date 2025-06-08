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

/// @brief PeInternals namespace, which contains the PE class and its related classes for parsing and traversing PE files \namespace PeInternals
namespace PeInternals
{
    /// Forward declaration of ResourceDirectory class \class ResourceDirectory
    class ResourceDirectory;

    /// @brief The ResourceDirectoryTraverser class, an abstract class for traversing resource directories \class ResourceDirectoryTraverser
    class ResourceDirectoryTraverser
    {
    public:

        /**
         * @brief Traverse the resource directory
         * @param fileData The file data to traverse
         * @param resourceDirectory The resource directory to traverse
         * @param level The current level of traversal
         * @param parentName The name of the parent resource directory
         */
        virtual void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) = 0;

        /**
         * @brief Virtual destructor for ResourceDirectoryTraverser
         */
        virtual ~ResourceDirectoryTraverser() {}
    };

#if defined(_WIN32)
    /// @brief The WinResourceDirectoryTraverser class, a class for traversing the resource directory on Windows \class WinResource
    class WinResourceDirectoryTraverser : public ResourceDirectoryTraverser
    {
    public:

        /**
         * @brief Traverse the resource directory on Windows
         * @param fileData The file data to traverse
         * @param resourceDirectory The resource directory to traverse
         * @param level The current level of traversal
         * @param parentName The name of the parent resource directory
         */
        void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) override;
    };
#endif

#if defined(__linux__)
    /// @brief The LinuxResourceDirectoryTraverser class, a class for traversing the resource directory on Linux \class LinuxResourceDirectoryTraverser
    class LinuxResourceDirectoryTraverser : public ResourceDirectoryTraverser
    {
    public:

        /**
         * @brief Traverse the resource directory on Linux
         * @param fileData The file data to traverse
         * @param resourceDirectory The resource directory to traverse
         * @param level The current level of traversal
         * @param parentName The name of the parent resource directory
         */
        void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) override;
    };
#endif

#if defined(__APPLE__)
    /// @brief The AppleResourceDirectoryTraverser class, a class for traversing the resource directory on macOS \class AppleResourceDirectoryTraverser
    class AppleResourceDirectoryTraverser : public ResourceDirectoryTraverser
    {
    public:

        /**
         * @brief Traverse the resource directory on macOS
         * @param fileData The file data to traverse
         * @param resourceDirectory The resource directory to traverse
         * @param level The current level of traversal
         * @param parentName The name of the parent resource directory
         */
        void traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName) override;
    };
#endif

    /// @brief The ResourceDirectoryTraverserFactory class, a factory class for creating ResourceDirectoryTraverser instances \class ResourceDirectoryTraverserFactory
    class ResourceDirectoryTraverserFactory
    {
    public:

        /**
         * @brief Create a ResourceDirectoryTraverser instance based on the platform
         * @return A pointer to the created ResourceDirectoryTraverser instance
         */
        static ResourceDirectoryTraverser* createTraverser();
    };

    /// @brief The ResourceDirectory class, a class for travers \class ResourceDirectory
    class ResourceDirectory
    {
    public:

        /**
         * @brief Traverse the resource directory
         * @param fileData The file data to traverse
         * @param level The current level of traversal
         * @param parentName The name of the parent resource directory
         */
        void traverse(const std::vector<uint8_t>& fileData, int level, const std::string& parentName);

        /**
         * @brief Sets the traverser for the resource directory
         * @param traverser The ResourceDirectoryTraverser to set
         */
        void setTraverser(ResourceDirectoryTraverser* traverser);
    private:
        ResourceDirectoryTraverser* traverser;
    };

    /// @brief The PE class, a class for parsing and extracting information from PE files \class PE
    class PE
    {
    public:

        /**
         * @brief Parse the PE headers and extract information
         * @param fileData The file data to parse
         */
        static void parseHeaders(const std::vector<uint8_t>& fileData);

        /**
         * @brief Extract section information from the PE file
         * @param fileData The file data to extract section information from
         */
        static void extractSectionInfo(const std::vector<uint8_t>& fileData);

        /**
         * @brief Extract the Import Table from the PE file
         * @param fileData The file data to extract the Import Table from
         */
        static void extractImportTable(const std::vector<uint8_t>& fileData);

        /**
         * @brief Extract the Export Table from the PE file
         * @param fileData The file data to extract the Export Table from
         */
        static void extractExportTable(const std::vector<uint8_t>& fileData);

        /**
         * @brief Extract resources from the PE file
         * @param fileData The file data to extract resources from
         */
        static void extractResources(const std::vector<uint8_t>& fileData);

        /**
         * @brief Get the offset of the Resource Directory in the PE file
         * @param fileData The file data to get the Resource Directory offset from
         * @return The offset of the Resource Directory
         */
        static uint32_t GetResourceDirectoryOffset(const std::vector<uint8_t>& fileData);

        /**
         * @brief Getter for the function names in the Import Table
         * @param fileData the file data to extract function names from
         * @return A vector of strings containing the function names
         */
        static std::vector<std::string> getFunctionNames(const std::vector<uint8_t>& fileData);

        /**
         * @brief Getter for the DLL names in the Import Table
         * @param fileData the file data to extract DLL names from
         * @return A vector of strings containing the DLL names
         */
        static std::vector<std::string> getDllNames(const std::vector<uint8_t>& fileData);

        /**
         * @brief Getter for the function addresses in the Import Table
         * @param fileData the file data to extract function addresses from
         * @return A vector of uint64_t containing the function addresses
         */
        static std::vector<uint64_t> getFunctionAddresses(const std::vector<uint8_t>& fileData);

        /**
         * @brief Getter for the DLL addresses in the Import Table
         * @param fileData the file data to extract DLL addresses from
         * @return A vector of uint64_t containing the DLL addresses
         */
        static std::vector<uint64_t> getDllAddresses(const std::vector<uint8_t>& fileData);
    };
}
