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

/**
 * @brief The DetectorInternals namespace
 * @namespace DetectorInternals
 */
namespace DetectorInternals
{
    /**
     * @brief The PackerDetection class
     * @class PackerDetection
     */
    class PackerDetection
    {
    public:
        PackerDetection(const std::vector<uint8_t>& fileData) : fileData(fileData) {}

        bool detectUPX();
        bool detectThemida();
        std::vector<std::string> checkSectionNames(const std::vector<std::string>& sectionNames);

    private:
        const std::vector<uint8_t>& fileData;

    protected: 
    };

    /**
     * @brief The AntiDebugDetection class
     * @class AntiDebugDetection
     */
    class AntiDebugDetection
    {
    public:
        AntiDebugDetection(const std::vector<uint8_t>& fileData) : fileData(fileData) {}

        bool checkEntryPoint(const std::vector<uint8_t>& pattern);
        static bool detectIsDebuggerPresent();
        static bool detectNtGlobalFlag();
        static bool detectHeapFlags();
        bool detectOutputDebugString();

    private:
        const std::vector<uint8_t>& fileData;

    protected:
    };
}