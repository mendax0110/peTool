#pragma once

#include <iostream>
#include <vector>
#include <string>

class Debugger
{
public:
    Debugger();
    ~Debugger();

    bool startDebugging(const std::string& executablePath);
    bool stepInto();
    bool stepOver();
    bool stepOut();
    bool run();

    bool setBreakpoint(unsigned long long address);
    bool removeBreakpoint(unsigned long long address);
    unsigned long long getRegisterValue(const std::string& registerName);
    std::vector<std::pair<std::string, unsigned long long>> getRegisters();
    unsigned long long getInstructionPointer();
    bool readMemory(unsigned long long address, void* buffer, size_t size);
    bool writeMemory(unsigned long long address, const void* buffer, size_t size);
    bool executeGDBCommand(const std::string& command);
    std::vector<std::string> getCallStack();
    std::vector<std::string> getWatch();
    std::vector<std::string> getLocals();

    enum class DebuggerState
    {
        Running,
        Paused,
        Exited
    };

    DebuggerState getState();
    void terminate();

private:
#if defined(__APPLE__)
    bool launchGDB(const std::string& executablePath);
    int gdbProcessID;
#elif defined(__WIN32__)
    bool launchGDB(const std::string& executablePath);
    int gdbProcessID;
#else
    #error "Debugger module not supported on this platform"
#endif

    void openTextSearchMode();
    void openGotoLineWindow();

};