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
#endif

#if defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif


namespace DetectorInternals
{
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

    class AntiDebugDetection
    {
    public:
        AntiDebugDetection(const std::vector<uint8_t>& fileData) : fileData(fileData) {}

        bool checkEntryPoint(const std::vector<uint8_t>& pattern);
        bool detectIsDebuggerPresent();
        bool detectNtGlobalFlag();
        bool detectHeapFlags();
        bool detectOutputDebugString();

    private:
        const std::vector<uint8_t>& fileData;

    protected:
    };
}