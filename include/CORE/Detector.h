#include <iostream>
#include <vector>
#include <cmath>

#if defined(_WIN32)
#include <windows.h>
#include <winnt.h>
#include <winternl.h>
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <mach/mach_init.h>
#include <mach/task.h>
#include <sys/sysctl.h>
#endif

#if defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

/// @brief Namespace for internal detector classes \namespace DetectorInternals
namespace DetectorInternals
{
    /// @brief The PackerDetection class \class PackerDetection
    class PackerDetection
    {
    public:

        /**
         * @brief Construct a new PackerDetection object
         * @param fileData The file data to analyze
         */
        PackerDetection(const std::vector<uint8_t>& fileData) : fileData(fileData) {}

        /**
         * @brief Detect if the file is packed with UPX
         * @return True if UPX is detected, false otherwise
         */
        bool detectUPX();

        /**
         * @brief Detect if the file is packed with Themida
         * @return True if Themida is detected, false otherwise
         */
        bool detectThemida();

        /**
         * @brief Check for suspicious sections in the file
         * @return A vector of strings containing the names of suspicious sections
         */
        std::vector<std::string> checkSectionNames(const std::vector<std::string>& sectionNames);

    private:
        const std::vector<uint8_t>& fileData;

    protected: 
    };

    /// @brief The AntiDebugDetection class \class AntiDebugDetection
    class AntiDebugDetection
    {
    public:

        /**
         * @brief Construct a new AntiDebugDetection object
         * @param fileData The file data to analyze
         */
        AntiDebugDetection(const std::vector<uint8_t>& fileData) : fileData(fileData) {}

        /**
         * @brief Check the Entry Point for a specific pattern
         * @param pattern The pattern to search for
         * @return A boolean indicating if the pattern was found
         */
        bool checkEntryPoint(const std::vector<uint8_t>& pattern);

        /**
         * @brief Detect if a Debugger is present
         * @return True if a debugger is detected, false otherwise
         */
        static bool detectIsDebuggerPresent();

        /**
         * @brief Detect NtGlobalFlag
         * @return True if NtGlobalFlag is detected, false otherwise
         */
        static bool detectNtGlobalFlag();

        /**
         * @brief Detect Heap Flags
         * @return True if the flags are detected, false otherwise
         */
        static bool detectHeapFlags();

        /**
         * @brief Detect OutputDebugString
         * @return True if OutputDebugString is detected, false otherwise
         */
        bool detectOutputDebugString();

    private:
        const std::vector<uint8_t>& fileData;

    protected:
    };
}