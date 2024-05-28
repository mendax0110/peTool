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
        static InjectorPlatform* CreatePlatform();
        virtual ~InjectorPlatform() noexcept = default;

        virtual unsigned int GetProcId(const char* procName) = 0;
        virtual bool InjectDLL(unsigned int procId, const char* dllPath) = 0;
    };

#if defined(_WIN32)
    class InjectorWindows : public InjectorPlatform 
    {
    public:
        unsigned int GetProcId(const char* procName) override;
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
#endif

#if defined(__linux__)
    class InjectorLinux : public InjectorPlatform
    {
    public:
        unsigned int GetProcId(const char* procName) override;
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
#endif

#if defined(__APPLE__)
    class InjectorMacOS : public InjectorPlatform
    {
    public:
        unsigned int GetProcId(const char* procName) override;
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
#endif
}
