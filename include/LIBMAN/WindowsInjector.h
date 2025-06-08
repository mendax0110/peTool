#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <Tlhelp32.h>

/// @brief Result struct to hold the result of an operation \struct Result
struct Result
{
    bool success;
    std::string message;
};

/// @brief WinInject class, which provides methods for injecting a DLL into a target process on Windows \class WinInject
class WinInject
{
public:

    /**
     * @brief Construct a new WinInject object
     * @param dllName The name of the DLL to inject
     * @param processName The name of the target process to inject the DLL into
     */
    WinInject(const char* dllName, const char* processName);

    /**
     * @brief Destroy the WinInject object
     */
    ~WinInject();

    /**
     * @brief Inject the DLL into the target process
     * @return A vector of Result objects containing the results of the injection operation
     */
    std::vector<Result> InjectDll();
    
    const char* m_dllName;
    const char* m_processName;
    HANDLE m_tragetProcess;
    void* m_pathAddress;
    void* m_loadLibraryAddress;
    char m_fullDllPath[MAX_PATH];

    /**
     * @brief Find the process ID of the target process by its name
     * @param processName The name of the target process
     * @param processId Reference to store the found process ID
     * @return A Result object indicating the success or failure of the operation
     */
    Result FindProcessId(const char* processName, DWORD& processId);

    /**
     * @brief Opens a handle to the target process
     * @param processId The ID of the target process
     * @return A Result object indicating the success or failure of the operation
     */
    Result OpenProcessHandle(DWORD processId);

    /**
     * @brief Allocates memory in the target process for the DLL path
     * @return A Result object indicating the success or failure of the operation
     */
    Result AllocateAndWriteMemory();

    /**
     * @brief Gets the address of the LoadLibraryA function in the target process
     * @return A Result object indicating the success or failure of the operation
     */
    Result GetLoadLibraryAddress();

    /**
     * @brief Creates a remote thread in the target process to load the DLL
     * @return A Result object indicating the success or failure of the operation
     */
    Result CreateRemoteThreadToLoadLibrary();

    /**
     * @brief Prints the results of the injection operation
     * @param results A vector of Result objects containing the results of the injection operation
     */
    void PrintResults(const std::vector<Result>& results);

    /**
     * @brief Converts a byte array to a hexadecimal string representation
     * @param data Pointer to the byte array
     * @param size Size of the byte array
     * @return A string containing the hexadecimal representation of the byte array
     */
    std::string HexDump(const void* data, size_t size);
};
#endif