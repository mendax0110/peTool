#include "../include/CORE/Debugger.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <signal.h>

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
#if defined(__APPLE__) || defined(__WIN32)
    return launchGDB(executablePath);
#else
    return false;
#endif
}

bool Debugger::launchGDB(const std::string& executablePath)
{
    std::string command = "gdb " + executablePath;
    std::cout << "Launching GDB with command: " << command << std::endl;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "Failed to launch GDB." << std::endl;
        return false;
    }
    gdbProcessID = fileno(pipe);
    return true;
}

bool Debugger::executeGDBCommand(const std::string &command)
{
    if (gdbProcessID == -1)
    {
        std::cerr << "GDB process not running." << std::endl;
        return false;
    }
    FILE* pipe = fdopen(gdbProcessID, "w");
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
    fclose(pipe);
    return true;
}

bool Debugger::stepInto()
{
    std::cout << "Stepping into..." << std::endl;
    return executeGDBCommand("step");
}

bool Debugger::stepOver()
{
    std::cout << "Stepping over..." << std::endl;
    return executeGDBCommand("next");
}

bool Debugger::stepOut()
{
    std::cout << "Stepping out..." << std::endl;
    return executeGDBCommand("finish");
}

bool Debugger::run()
{
    std::cout << "Running..." << std::endl;
    return executeGDBCommand("run");
}

bool Debugger::setBreakpoint(unsigned long long address)
{
    std::cout << "Setting breakpoint at address: " << address << std::endl;
    char command[100];
    snprintf(command, sizeof(command), "break *0x%llx", address);
    return executeGDBCommand(command);
}

bool Debugger::removeBreakpoint(unsigned long long address)
{
    std::cout << "Removing breakpoint at address: " << address << std::endl;
    char command[100];
    snprintf(command, sizeof(command), "delete %lld", address);
    return executeGDBCommand(command);
}

unsigned long long Debugger::getRegisterValue(const std::string& registerName)
{
    std::cout << "Getting register value for: " << registerName << std::endl;
    char command[100];
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
    return 0;
}

std::vector<std::pair<std::string, unsigned long long>> Debugger::getRegisters()
{
    std::cout << "Getting all registers" << std::endl;
    if (executeGDBCommand("info registers"))
    {
        std::cout << "Parsing registers..." << std::endl;
        return {{"RAX", 0}, {"RBX", 0}, {"RCX", 0}, {"RDX", 0}};
    }
    return {};
}

unsigned long long Debugger::getInstructionPointer()
{
    std::cout << "Getting instruction pointer" << std::endl;
    if (executeGDBCommand("info registers rip"))
    {
        std::cout << "Parsing instruction pointer..." << std::endl;
        auto instructionPointer = getRegisterValue("rip");
        return instructionPointer;
    }
    return 0;
}

bool Debugger::readMemory(unsigned long long address, void* buffer, size_t size)
{
    std::cout << "Reading memory at address: " << address << std::endl;
    FILE* pipe = popen("gdb --batch --command=script.gdb", "r");
    if (!pipe)
    {
        std::cerr << "Failed to read memory." << std::endl;
        return false;
    }
    std::string command = "x/" + std::to_string(size) + "gx " + std::to_string(address);
    if (fprintf(pipe, "%s\n", command.c_str()) < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        pclose(pipe);
        return false;
    }
    char output[256];
    if (fgets(output, sizeof(output), pipe) != nullptr)
    {
        std::cout << "Memory content: " << output << std::endl;
    }
    pclose(pipe);
    return true;
}

bool Debugger::writeMemory(unsigned long long address, const void* buffer, size_t size)
{
    std::cout << "Writing memory at address: " << address << std::endl;
    FILE* pipe = popen("gdb --batch --command=script.gdb", "r");
    if (!pipe)
    {
        std::cerr << "Failed to write memory." << std::endl;
        return false;
    }
    std::string command = "set {int} " + std::to_string(address) + " = " + std::to_string(*(int*)buffer);
    if (fprintf(pipe, "%s\n", command.c_str()) < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        pclose(pipe);
        return false;
    }
    pclose(pipe);
    return true;
}

Debugger::DebuggerState Debugger::getState()
{
    std::cout << "Getting debugger state" << std::endl;
    FILE* pipe = popen("gdb --batch --command=script.gdb", "r");
    if (!pipe)
    {
        std::cerr << "Failed to get debugger state." << std::endl;
        return DebuggerState::Exited;
    }
    if (fprintf(pipe, "info program\n") < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        pclose(pipe);
        return DebuggerState::Exited;
    }
    char output[256];
    if (fgets(output, sizeof(output), pipe) != nullptr)
    {
        std::cout << "Debugger state: " << output << std::endl;
        if (strstr(output, "running") != nullptr)
        {
            return DebuggerState::Running;
        }
        else if (strstr(output, "paused") != nullptr)
        {
            return DebuggerState::Paused;
        }
    }
    pclose(pipe);
    return DebuggerState::Exited;
}

void Debugger::terminate()
{
    std::cout << "Terminating debugger..." << std::endl;
    if (gdbProcessID != -1)
    {
        // kill the GDB process
        kill(gdbProcessID, SIGKILL);
        gdbProcessID = -1;
    }
    auto cleanup = []() {
    };
}

void Debugger::openTextSearchMode()
{
    std::cout << "Opening text search mode..." << std::endl;
    FILE* pipe = popen("gdb --batch --command=script.gdb", "r");
    if (!pipe)
    {
        std::cerr << "Failed to open text search mode." << std::endl;
        return;
    }
    if (fprintf(pipe, "search\n") < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        pclose(pipe);
        return;
    }
    pclose(pipe);

    auto commands = {"search", "search next", "search prev"};
    for (const auto& cmd : commands)
    {
        executeGDBCommand(cmd);
    }
    executeGDBCommand("quit");
}

void Debugger::openGotoLineWindow()
{
    std::cout << "Opening goto line window..." << std::endl;
    FILE* pipe = popen("gdb --batch --command=script.gdb", "r");
    if (!pipe)
    {
        std::cerr << "Failed to open goto line window." << std::endl;
        return;
    }
    if (fprintf(pipe, "goto\n") < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        pclose(pipe);
        return;
    }
    pclose(pipe);

    auto commands = {"goto", "goto next", "goto prev"};
    for (const auto& cmd : commands)
    {
        executeGDBCommand(cmd);
    }
    executeGDBCommand("quit");
}

std::vector<std::string> Debugger::getCallStack()
{
    std::cout << "Getting call stack" << std::endl;
    FILE* pipe = popen("gdb --batch --command=script.gdb", "r");
    if (!pipe)
    {
        std::cerr << "Failed to get call stack." << std::endl;
        return {};
    }
    if (fprintf(pipe, "bt\n") < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        pclose(pipe);
        return {};
    }
    std::vector<std::string> callStack;
    char output[256];
    while (fgets(output, sizeof(output), pipe) != nullptr)
    {
        callStack.push_back(output);
    }
    pclose(pipe);
    return callStack;
}

std::vector<std::string> Debugger::getWatch()
{
    std::cout << "Getting watch" << std::endl;
    FILE* pipe = popen("gdb --batch --command=script.gdb", "r");
    if (!pipe)
    {
        std::cerr << "Failed to get watch." << std::endl;
        return {};
    }
    if (fprintf(pipe, "info watch\n") < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        pclose(pipe);
        return {};
    }
    std::vector<std::string> watch;
    char output[256];
    while (fgets(output, sizeof(output), pipe) != nullptr)
    {
        watch.push_back(output);
    }
    pclose(pipe);
    return watch;
}

std::vector<std::string> Debugger::getLocals()
{
    std::cout << "Getting locals" << std::endl;
    FILE* pipe = popen("gdb --batch --command=script.gdb", "r");
    if (!pipe)
    {
        std::cerr << "Failed to get locals." << std::endl;
        return {};
    }
    if (fprintf(pipe, "info locals\n") < 0)
    {
        std::cerr << "Failed to send command to GDB." << std::endl;
        pclose(pipe);
        return {};
    }
    std::vector<std::string> locals;
    char output[256];
    while (fgets(output, sizeof(output), pipe) != nullptr)
    {
        locals.push_back(output);
    }
    pclose(pipe);
    return locals;
}