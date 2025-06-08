#pragma once

#include <iostream>
#include <string>
#include <vector>

#if defined(__linux__)
#include <sys/types.h>

/// @brief Result struct to hold the result of an operation \struct Result
struct Result
{
    bool success;
    std::string message;
};

/// @brief LinInject class, which provides methods for injecting a shared library into a target process on Linux \class LinInject
class LinInject
{
public:

    /**
     * @brief Construct a new LinInject object
     * @param soName The name of the shared library to inject
     * @param processName The name of the target process to inject the shared library into
     */
    LinInject(const char* soName, const char* processName);

    /**
     * @brief Destroy the LinInject object
     */
    ~LinInject();

    /**
     * @brief Inject the shared library into the target process
     * @return A vector of Result objects containing the results of the injection operation
     */
    std::vector<Result> InjectSo();

    std::string m_soName;
    std::string m_processName;
    pid_t m_targetProcess;
    void* m_pathAddress;
    void* m_dlopenAddress;

    /**
     * @brief Find the process ID of the target process by its name
     * @param processName The name of the target process
     * @param processId Reference to store the found process ID
     * @return A vector of Result objects containing the results of the operation
     */
    std::vector<Result> FindProcessId(const char* processName, pid_t& processId);

    /**
     * @brief Opens a handle to the target process
     * @param processId The ID of the target process
     * @return A Result object indicating the success or failure of the operation
     */
    Result OpenProcessHandle(pid_t processId);

    /**
     * @brief Allocates memory in the target process for the shared library path
     * @return A Result object indicating the success or failure of the operation
     */
    Result AllocateAndWriteMemory();

    /**
     * @brief Gets the address of the dlopen function in the target process
     * @return A Result object containing the address of dlopen or an error message
     */
    Result GetDlopenAddress();

    /**
     * @brief Injects the shared library into the target process
     * @return A Result object indicating the success or failure of the injection operation
     */
    Result InjectSharedLibrary();

    /**
     * @brief Prints the results of the injection operation
     * @param results A vector of Result objects containing the results of the operation
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