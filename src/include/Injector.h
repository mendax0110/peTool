#pragma once

#if defined(_WIN32)
#include <Windows.h>
#include <TlHelp32.h>
#endif

#if defined(__linux__)
#include <sys/types.h>
#include <signal.h>
#endif

#if defined(__APPLE__)
#include <sys/types.h>
#include <signal.h>
#include <libproc.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_vm.h>
#endif

#include <iostream>
#include <vector>

namespace DllInjector
{
    class InjectorPlatform
    {
    public:
        InjectorPlatform();
        ~InjectorPlatform();

        virtual unsigned int GetProcId(const char* procName)
        {
            return platform->GetProcId(procName);
        }

        virtual bool InjectDLL(unsigned int procId, const char* dllPath)
        {
            return platform->InjectDLL(procId, dllPath);
        }

        InjectorPlatform* getPlatform()
        {
            return platform;
        }
    private:
        InjectorPlatform* platform;
    };

    class InjectorWindows : public InjectorPlatform 
    {
    public:
        unsigned int GetProcId(const char* procName) override;
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };

    class InjectorLinux : public InjectorPlatform
    {
    public:
        unsigned int GetProcId(const char* procName) override;
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };

    class InjectorMacOS : public InjectorPlatform
    {
    public:
        unsigned int GetProcId(const char* procName) override;
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
}