#pragma once

#include <iostream>
#include <vector>
#include <string>

/// @brief Debugger class, which provides methods for debugging applications \class Debugger
class Debugger
{
public:

    /**
     * @brief Construct a new Debugger object
     */
    Debugger();

    /**
     * @brief Destroy the Debugger object
     */
    ~Debugger();

    /**
     * @brief Start debugging the specified executable.
     * @param executablePath Path to the executable to debug.
     * @return true if the debugger was successfully started, false otherwise.
     */
    bool startDebugging(const std::string& executablePath);

    /**
     * @brief Step into the next instruction.
     * @return true if the step was successful, false otherwise.
     */
    bool stepInto();

    /**
     * @brief Step over the next instruction.
     * @return true if the step was successful, false otherwise.
     */
    bool stepOver();

    /**
     * @brief Step out of the current function.
     * @return true if the step was successful, false otherwise.
     */
    bool stepOut();

    /**
     * @brief Run the program until the next breakpoint.
     * @return true if the run was successful, false otherwise.
     */
    bool run();

    /**
     * @brief Set a breakpoint at the specified address.
     * @param address Address to set the breakpoint at.
     * @return true if the breakpoint was successfully set, false otherwise.
     */
    bool setBreakpoint(unsigned long long address);

    /**
     * @brief Remove the breakpoint at the specified address.
     * @param address Address to remove the breakpoint from.
     * @return true if the breakpoint was successfully removed, false otherwise.
     */
    bool removeBreakpoint(unsigned long long address);

    /**
     * @brief Get the value of the specified register.
     * @param registerName Name of the register to get the value of.
     * @return Value of the register.
     */
    unsigned long long getRegisterValue(const std::string& registerName);

    /**
     * @brief Get all registers and their values.
     * @return Vector of pairs containing the register name and value.
     */
    std::vector<std::pair<std::string, unsigned long long>> getRegisters();

    /**
     * @brief Get the instruction pointer.
     * @return Value of the instruction pointer.
     */
    unsigned long long getInstructionPointer();

    /**
     * @brief Read memory at the specified address.
     * @param address Address to read memory from.
     * @param buffer Buffer to store the memory in.
     * @param size Size of the memory to read.
     * @return true if the memory was successfully read, false otherwise.
     */
    bool readMemory(unsigned long long address, void* buffer, size_t size);

    /**
     * @brief Write memory at the specified address.
     * @param address Address to write memory to.
     * @param buffer Buffer containing the memory to write.
     * @param size Size of the memory to write.
     * @return true if the memory was successfully written, false otherwise.
     */
    static bool writeMemory(unsigned long long address, const void* buffer, size_t size);

    /**
     * @brief Get the current call stack.
     * @return Vector of strings containing the call stack.
     */
    static std::vector<std::string> getCallStack();

    /**
     * @brief Get the current registers.
     * @return Vector of pairs containing the register name and value.
     */
    static std::vector<std::string> getWatch();

    /**
     * @brief Get the current locals.
     * @return Vector of strings containing the local variables.
     */
    static std::vector<std::string> getLocals();

    /// @brief Get the current state of the debugger. \enum DebuggerState
    enum class DebuggerState
    {
        Running,
        Paused,
        Exited
    };

    /**
     * @brief Get the current state of the debugger.
     * @return Current state of the debugger.
     */
    DebuggerState getState();

    /**
     * @brief Terminate the debugger process.
     * This will close the debugger and clean up any resources used.
     */
    void terminate();

private:
#if defined(__APPLE__)

    /**
     * @brief Launch the LLDB debugger with the specified executable.
     * @param executablePath The path to the executable to debug.
     * @return A boolean indicating if the debugger was successfully launched.
     */
    bool launchLLDB(const std::string& executablePath);

    /**
     * @brief Execute the specified LLDB command.
     * @param command The command to execute.
     * @return true if the command was successfully executed, false otherwise.
     */
    bool executeLLDBCommand(const std::string& command) const;

    int gdbProcessID;
#elif defined(_WIN32)
    bool launchGDB(const std::string& executablePath);
    bool executeGDBCommand(const std::string& command);
    int gdbProcessID;
#else
    #error "Debugger module not supported on this platform"
#endif

    /**
     * @brief Open the text search mode.
     * This will allow the user to search for text in the current file.
     */
    void openTextSearchMode();

    /**
     * @brief Open the goto line window.
     * This will allow the user to jump to a specific line in the current file.
     */
    void openGotoLineWindow();
};
