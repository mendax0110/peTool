#include "../include/CORE/Detector.h"
#include <algorithm>
#include <iostream>

using namespace DetectorInternals;

/**
 * @brief Detects if the file is packed with UPX
 * @return True if UPX is detected, false otherwise
 */
bool PackerDetection::detectUPX()
{
    std::vector<std::string> upxSignatures = {"UPX0", "UPX1", "UPX2" };

    for (const auto& signature : upxSignatures)
    {
        if (std::search(fileData.begin(), fileData.end(), signature.begin(), signature.end()) != fileData.end())
        {
            std::cout << "UPX detected: " << signature << std::endl;
            return true;
        }
    }

    std::cout << "UPX not detected" << std::endl;
    return false;
}

/**
 * @brief Detects if the file is packed with Themida
 * @return True if Themida is detected, false otherwise
 */
bool PackerDetection::detectThemida()
{
    std::vector<std::string> themidaSignatures = {
        "OLLYDBG",
        "GBDYLLO",
        "pediy06",
        "FilemonClass",
        "File Monitor - Sysinternals: www.sysinternals.com",
        "PROCMON_WINDOW_CLASS",
        "Process Monitor - Sysinternals: www.sysinternals.com",
        "RegmonClass",
        "Registry Monitor - Sysinternals: www.sysinternals.com",
        "18467-41"};

    for (const auto& signature : themidaSignatures)
    {
        auto it = std::search(fileData.begin(), fileData.end(), signature.begin(), signature.end());

        if (it != fileData.end())
        {
            std::cout << "Themida detected: " << signature << std::endl;
            return true;
        }
    }

    std::cout << "Themida not detected" << std::endl;
    return false;
}

/**
 * @brief Check the Section names for suspicious names
 * @param sectionNames The names of the sections to check
 * @return A vector of suspicious section names
 */
std::vector<std::string> PackerDetection::checkSectionNames(const std::vector<std::string>& sectionNames)
{
    std::vector<std::string> suspiciousSections;

    for (const auto& sectionName : sectionNames)
    {
        std::vector<uint8_t> sectionNameBytes(sectionName.begin(), sectionName.end());
        if (std::search(fileData.begin(), fileData.end(), sectionNameBytes.begin(), sectionNameBytes.end()) != fileData.end())
        {
            std::cout << "Suspicious section detected: " << sectionName << std::endl;
            suspiciousSections.push_back(sectionName);
        }
    }

    std::cout << "No suspicious sections detected" << std::endl;
    return suspiciousSections;
}

/**
 * @brief Check the Entry Point for a specific pattern
 * @param pattern The pattern to search for
 * @return A boolean indicating if the pattern was found
 */
bool AntiDebugDetection::checkEntryPoint(const std::vector<uint8_t>& pattern)
{
    auto it = std::search(fileData.begin(), fileData.end(), pattern.begin(), pattern.end());

    for (size_t i = 0; i < pattern.size(); i++)
    {
        if (fileData[it - fileData.begin() + i] != pattern[i])
        {
            std::cout << "Failed to find pattern at offset: " << it - fileData.begin() << std::endl;
            return false;
        }
    }

    std::cout << "Pattern found at offset: " << it - fileData.begin() << std::endl;
    return (it != fileData.end());
}

/**
 * @brief Detect if a Debugger is present
 * @return True if a debugger is detected, false otherwise
 */
bool AntiDebugDetection::detectIsDebuggerPresent()
{
#if defined(_WIN32)
    HMODULE hKernel32 = GetModuleHandle("kernel32.dll");

    if (hKernel32 == nullptr)
        std::cout << "Failed to get kernel32.dll handle" << std::endl;
        return false;

    if (hKernel32)
    {
        FARPROC pIsDebuggerPresent = GetProcAddress(hKernel32, "IsDebuggerPresent");

        if (pIsDebuggerPresent)
        {
            std::cout << "IsDebuggerPresent detected" << pIsDebuggerPresent << std::endl;
            return true;
        }
    }

    std::cout << "Debugger not detected" << std::endl;
    return false;
#endif

#if defined(__APPLE__)
    return false;
#endif

#if defined(__linux__)
    return false;
#endif
}

/**
 * @brief Detect NT-specific flags
 * @return True if the flags are detected, false otherwise
 */
bool AntiDebugDetection::detectNtGlobalFlag()
{
#if defined(_WIN32)

#ifdef _M_X64
    PPEB peb = (PPEB)__readgsqword(0x60);
#else
    PPEB peb = (PPEB)__readfsdword(0x30);
#endif
    DWORD NtGlobalFlag = *(DWORD*)((PBYTE)peb + 0xBC);
    std::cout << "NtGlobalFlag: " << NtGlobalFlag << std::endl;
    return (NtGlobalFlag & 0x70) != 0;
#else
    std::cout << "NtGlobalFlag not detected" << std::endl;
    return false;
#endif
}

/**
 * @brief Detect Heap Flags
 * @return True if the flags are detected, false otherwise
 */
bool AntiDebugDetection::detectHeapFlags()
{
#if defined(_WIN32)
#ifdef _M_X64
    PPEB peb = (PPEB)__readgsqword(0x60);
#else
    PPEB peb = (PPEB)__readfsdword(0x30);
#endif
    DWORD HeapFlags = *(DWORD*)((PBYTE)peb + 0x18);
    std::cout << "HeapFlags: " << HeapFlags << std::endl;
    return (HeapFlags & 0x70) != 0;
#else
    std::cout << "HeapFlags not detected" << std::endl;
    return false;
#endif
}

/**
 * @brief Detect if the file is being debugged
 * @return True if the file is being debugged, false otherwise
 */
bool AntiDebugDetection::detectOutputDebugString()
{
    std::vector<uint8_t> pattern = { 0xCC };

    for (size_t i = 0; i < fileData.size(); i++)
    {
        if (fileData[i] == pattern[0])
        {
            std::cout << "OutputDebugString detected: " << i << std::endl;
            return true;
        }
    }

    std::cout << "OutputDebugString not detected" << std::endl;
    return false;
}

