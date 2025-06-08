#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

#if defined(__APPLE__)
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <dlfcn.h>
#include <sys/sysctl.h>

/// @brief Result struct to hold the result of an operation \struct Result
struct Result
{
    bool success;
    std::string message;
};

/// @brief MacInject class, which provides methods for injecting a dynamic library into a target process on macOS \class MacInject
class MacInject
{
public:

    /**
     * @brief Construct a new MacInject object
     * @param dylibName The name of the dynamic library to inject
     * @param processName The name of the target process to inject the dynamic library into
     */
    MacInject(const char* dylibName, const char* processName);

    /**
     * @brief Destroy the MacInject object
     */
    ~MacInject();

    /**
     * @brief Inject the dynamic library into the target process
     * @return A vector of Result objects containing the results of the injection operation
     */
    std::vector<Result> InjectDylib();

    const char* m_dylibName;
    const char* m_processName;
    mach_port_t m_targetTask;
    mach_vm_address_t m_pathAddress;
    void* m_dlopenAddress;
    char m_fullDylibPath[PATH_MAX]{};

    /**
     * @brief Find the process ID of the target process by its name
     * @param processName The name of the target process
     * @param processId Reference to store the found process ID
     * @return A Result object indicating the success or failure of the operation
     */
    static Result FindProcessId(const char* processName, pid_t& processId);

    /**
     * @brief Opens a handle to the target process
     * @param processId The ID of the target process
     * @return A Result object indicating the success or failure of the operation
     */
    Result OpenProcessHandle(pid_t processId);

    /**
     * @brief Allocates memory in the target process and writes the path of the dynamic library to it
     * @return A Result object indicating the success or failure of the operation
     */
    Result AllocateAndWriteMemory();

    /**
     * @brief Gets the address of the dlopen function in the target process
     * @return A Result object indicating the success or failure of the operation
     */
    Result GetDlopenAddress();

    /**
     * @brief Creates a remote thread in the target process to load the dynamic library
     * @return A Result object indicating the success or failure of the operation
     */
    Result CreateRemoteThreadToLoadDylib();

    /**
     * @brief Prints the results of the injection operation
     * @param results A vector of Result objects containing the results of the operation
     */
    static void PrintResults(const std::vector<Result>& results);

    /**
     * @brief Converts a vector of bytes to a hexadecimal string representation
     * @param data The vector of bytes to convert
     * @return A string containing the hexadecimal representation of the bytes
     */
    static std::string HexDump(const void* data, size_t size);

    /**
     * @brief Gets Running Processes
     * @return A vector of strings containing the names of running processes
     */
    static std::vector<std::string> GetRunningProcesses();
};
#endif