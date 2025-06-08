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

bool Debugger::stepInto()
{
    std::cout << "Stepping into..." << std::endl;
#if defined(__APPLE__)
    return executeLLDBCommand("thread step-in");
#elif defined(_WIN32)
    return executeGDBCommand("step");
#endif
}

bool Debugger::stepOver()
{
    std::cout << "Stepping over..." << std::endl;
#if defined(__APPLE__)
    return executeLLDBCommand("thread step-over");
#elif defined(_WIN32)
    return executeGDBCommand("next");
#endif
}

bool Debugger::stepOut()
{
    std::cout << "Stepping out..." << std::endl;
#if defined(__APPLE__)
    return executeLLDBCommand("thread step-out");
#elif defined(_WIN32)
    return executeGDBCommand("finish");
#endif
}

bool Debugger::run()
{
    std::cout << "Running..." << std::endl;
#if defined(__APPLE__)
    return executeLLDBCommand("run");
#elif defined(_WIN32)
    return executeGDBCommand("run");
#endif
}

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

std::vector<std::string> Debugger::getCallStack()
{
    std::vector<std::string> callStack;
    auto it = callStack.begin();
    callStack.insert(it, "FunctionName: File:Line");
    return callStack;
}

std::vector<std::string> Debugger::getWatch()
{
    std::vector<std::string> watch;
    auto it = watch.begin();
    watch.insert(it, "VariableName: Value");
    return watch;
}

std::vector<std::string> Debugger::getLocals()
{
    std::vector<std::string> locals;
    auto it = locals.begin();
    locals.insert(it, "VariableName: Value");
    return locals;
}