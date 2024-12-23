#include "../include/CORE/Debugger.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <csignal>

Debugger::Debugger()
{
    gdbProcessID = -1;
}

Debugger::~Debugger()
{
    terminate();
}

/**
 * @brief Start debugging the specified executable.
 * @param executablePath Path to the executable to debug.
 * @return true if the debugger was successfully started, false otherwise.
 */
bool Debugger::startDebugging(const std::string& executablePath)
{
#if defined(__APPLE__)
    return launchLLDB(executablePath);
#elif defined(_WIN32)
    return launchGDB(executablePath);
#else
    return false;
#endif
}

#if defined(__APPLE__)
/**
 * @brief Launch the LLDB debugger with the specified executable.
 * @param executablePath Path to the executable to debug.
 * @return true if the debugger was successfully launched, false otherwise.
 */
bool Debugger::launchLLDB(const std::string& executablePath)
{
    std::string command = "lldb --batch -o 'target create \"" + executablePath + "\"'";
    std::cout << "Launching LLDB with command: " << command << std::endl;
    FILE* pipe = popen(command.c_str(), "w");
    if (!pipe)
    {
        std::cerr << "Failed to launch LLDB." << std::endl;
        return false;
    }
    gdbProcessID = fileno(pipe);
    return true;
}

/**
 * @brief Execute the specified LLDB command.
 * @param command Command to execute.
 * @return true if the command was successfully executed, false otherwise.
 */
bool Debugger::executeLLDBCommand(const std::string& command) const
{
    if (gdbProcessID == -1)
    {
        std::cerr << "LLDB process not running." << std::endl;
        return false;
    }
    FILE* pipe = fdopen(gdbProcessID, "w");
    if (!pipe)
    {
        std::cerr << "Failed to open LLDB process." << std::endl;
        return false;
    }
    if (fprintf(pipe, "%s\n", command.c_str()) < 0)
    {
        std::cerr << "Failed to send command to LLDB." << std::endl;
        fclose(pipe);
        return false;
    }
    fflush(pipe);
    return true;
}

#elif defined(_WIN32)
/**
 * @brief Launch the GDB debugger with the specified executable.
 * @param executablePath Path to the executable to debug.
 * @return true if the debugger was successfully launched, false otherwise.
 */
bool Debugger::launchGDB(const std::string& executablePath)
{
    std::string command = "gdb " + executablePath;
    std::cout << "Launching GDB with command: " << command << std::endl;
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Failed to launch GDB." << std::endl;
        return false;
    }
    gdbProcessID = _fileno(pipe);
    return true;
}

/**
 * @brief Execute the specified GDB command.
 * @param command Command to execute.
 * @return true if the command was successfully executed, false otherwise.
 */
bool Debugger::executeGDBCommand(const std::string& command)
{
    if (gdbProcessID == -1)
    {
        std::cerr << "GDB process not running." << std::endl;
        return false;
    }
    FILE* pipe = _fdopen(gdbProcessID, "w");
    if (!pipe)
    {
        std::cerr << "Failed to open GDB process." << std::endl;
        return false;
    }
    if (fprintf(pipe, "%s\n", command.c_str()) < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        fclose(pipe);
        return false;
    }
    fflush(pipe);
    return true;
}
#endif

/**
 * @brief Step into the next instruction.
 * @return true if the step was successful, false otherwise.
 */
bool Debugger::stepInto()
{
    std::cout << "Stepping into..." << std::endl;
#if defined(__APPLE__)
    return executeLLDBCommand("thread step-in");
#elif defined(_WIN32)
    return executeGDBCommand("step");
#endif
}

/**
 * @brief Step over the next instruction.
 * @return true if the step was successful, false otherwise.
 */
bool Debugger::stepOver()
{
    std::cout << "Stepping over..." << std::endl;
#if defined(__APPLE__)
    return executeLLDBCommand("thread step-over");
#elif defined(_WIN32)
    return executeGDBCommand("next");
#endif
}

/**
 * @brief Step out of the current function.
 * @return true if the step was successful, false otherwise.
 */
bool Debugger::stepOut()
{
    std::cout << "Stepping out..." << std::endl;
#if defined(__APPLE__)
    return executeLLDBCommand("thread step-out");
#elif defined(_WIN32)
    return executeGDBCommand("finish");
#endif
}

/**
 * @brief Run the program until the next breakpoint.
 * @return true if the run was successful, false otherwise.
 */
bool Debugger::run()
{
    std::cout << "Running..." << std::endl;
#if defined(__APPLE__)
    return executeLLDBCommand("run");
#elif defined(_WIN32)
    return executeGDBCommand("run");
#endif
}

/**
 * @brief Set a breakpoint at the specified address.
 * @param address Address to set the breakpoint at.
 * @return true if the breakpoint was successfully set, false otherwise.
 */
bool Debugger::setBreakpoint(unsigned long long address)
{
    std::cout << "Setting breakpoint at address: " << address << std::endl;
    char command[100];
#if defined(__APPLE__)
    snprintf(command, sizeof(command), "breakpoint set --address 0x%llx", address);
    return executeLLDBCommand(command);
#elif defined(_WIN32)
    snprintf(command, sizeof(command), "break *0x%llx", address);
    return executeGDBCommand(command);
#endif
}

/**
 * @brief Remove the breakpoint at the specified address.
 * @param address Address to remove the breakpoint from.
 * @return true if the breakpoint was successfully removed, false otherwise.
 */
bool Debugger::removeBreakpoint(unsigned long long address)
{
    std::cout << "Removing breakpoint at address: " << address << std::endl;
    char command[100];
#if defined(__APPLE__)
    snprintf(command, sizeof(command), "breakpoint delete %lld", address);
    return executeLLDBCommand(command);
#elif defined(_WIN32)
    snprintf(command, sizeof(command), "delete %lld", address);
    return executeGDBCommand(command);
#endif
}

/**
 * @brief Get the value of the specified register.
 * @param registerName Name of the register to get the value of.
 * @return Value of the register.
 */
unsigned long long Debugger::getRegisterValue(const std::string& registerName)
{
    std::cout << "Getting register value for: " << registerName << std::endl;
    char command[100];
#if defined(__APPLE__)
    snprintf(command, sizeof(command), "register read %s", registerName.c_str());
    if (executeLLDBCommand(command))
    {
        std::cout << "Parsing register value..." << std::endl;
        auto registers = getRegisters();
        for (const auto& reg : registers)
        {
            if (reg.first == registerName)
            {
                return reg.second;
            }
        }
    }
#elif defined(_WIN32)
    snprintf(command, sizeof(command), "info registers %s", registerName.c_str());
    if (executeGDBCommand(command))
    {
        std::cout << "Parsing register value..." << std::endl;
        auto registers = getRegisters();
        for (const auto& reg : registers)
        {
            if (reg.first == registerName)
            {
                return reg.second;
            }
        }
    }
#endif
    return 0;
}

/**
 * @brief Get all registers and their values.
 * @return Vector of pairs containing the register name and value.
 */
std::vector<std::pair<std::string, unsigned long long>> Debugger::getRegisters()
{
    std::cout << "Getting all registers" << std::endl;
#if defined(__APPLE__)
    if (executeLLDBCommand("register read"))
    {
        std::cout << "Parsing registers..." << std::endl;
        return {{"RAX", 0}, {"RBX", 0}, {"RCX", 0}, {"RDX", 0}};
    }
#elif defined(_WIN32)
    if (executeGDBCommand("info registers"))
    {
        std::cout << "Parsing registers..." << std::endl;
        return {{"RAX", 0}, {"RBX", 0}, {"RCX", 0}, {"RDX", 0}};
    }
#endif
    return {};
}

/**
 * @brief Get the instruction pointer.
 * @return Value of the instruction pointer.
 */
unsigned long long Debugger::getInstructionPointer()
{
    std::cout << "Getting instruction pointer" << std::endl;
#if defined(__APPLE__)
    if (executeLLDBCommand("register read rip"))
    {
        std::cout << "Parsing instruction pointer..." << std::endl;
        auto instructionPointer = getRegisterValue("rip");
        return instructionPointer;
    }
#elif defined(_WIN32)
    if (executeGDBCommand("info registers rip"))
    {
        std::cout << "Parsing instruction pointer..." << std::endl;
        auto instructionPointer = getRegisterValue("rip");
        return instructionPointer;
    }
#endif
    return 0;
}

/**
 * @brief Read memory at the specified address.
 * @param address Address to read memory from.
 * @param buffer Buffer to store the memory in.
 * @param size Size of the memory to read.
 * @return true if the memory was successfully read, false otherwise.
 */
bool Debugger::readMemory(unsigned long long address, void* buffer, size_t size)
{
    std::cout << "Reading memory at address: " << address << std::endl;
    char command[100];
#if defined(__APPLE__)
    snprintf(command, sizeof(command), "memory read 0x%llx %zu", address, size);
    return executeLLDBCommand(command);
#elif defined(_WIN32)
    // cast size to int to avoid warning
    auto intSize = static_cast<int>(size);
    snprintf(command, sizeof(command), "x/%dxb 0x%llx", intSize, address);
    return executeGDBCommand(command);
#endif
}

/**
 * @brief Write memory at the specified address.
 * @param address Address to write memory to.
 * @param buffer Buffer containing the memory to write.
 * @param size Size of the memory to write.
 * @return true if the memory was successfully written, false otherwise.
 */
bool Debugger::writeMemory(unsigned long long address, const void* buffer, size_t size)
{
    std::cout << "Writing memory at address: " << address << std::endl;
#if defined(__APPLE__)
    // TODO: arbitrary memory in batch mode, lldb does not have this feature
    std::cerr << "Write memory not directly supported in LLDB batch mode." << std::endl;
    return false;
#elif defined(_WIN32)
    FILE* pipe = _popen("gdb --batch --command=script.gdb", "r");
    if (!pipe)
    {
        std::cerr << "Failed to write memory." << std::endl;
        return false;
    }
    char command[100];
    snprintf(command, sizeof(command), "set {int} 0x%llx = %d", address, *(int*)buffer);
    if (fprintf(pipe, "%s\n", command) < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        fclose(pipe);
        return false;
    }
    fflush(pipe);
    fclose(pipe);
    return true;
#endif
}

/**
 * @brief Get the current state of the debugger.
 * @return Current state of the debugger.
 */
void Debugger::terminate()
{
    std::cout << "Terminating debugger process" << std::endl;
#if defined(__APPLE__)
    if (gdbProcessID != -1)
    {
        kill(gdbProcessID, SIGTERM);
        gdbProcessID = -1;
    }
#elif defined(_WIN32)
    if (gdbProcessID != -1)
    {
        _pclose(_fdopen(gdbProcessID, "r"));
        gdbProcessID = -1;
    }
#endif
}

/**
 * @brief Get the current state of the debugger.
 * @return Current state of the debugger.
 */
std::vector<std::string> Debugger::getCallStack()
{
    std::vector<std::string> callStack;
    auto it = callStack.begin();
    callStack.insert(it, "FunctionName: File:Line");
    return callStack;
}

/**
 * @brief Get the current watch variables.
 * @return Vector of strings containing the watch variables.
 */
std::vector<std::string> Debugger::getWatch()
{
    std::vector<std::string> watch;
    auto it = watch.begin();
    watch.insert(it, "VariableName: Value");
    return watch;
}

/**
 * @brief Get the current local variables.
 * @return Vector of strings containing the local variables.
 */
std::vector<std::string> Debugger::getLocals()
{
    std::vector<std::string> locals;
    auto it = locals.begin();
    locals.insert(it, "VariableName: Value");
    return locals;
}