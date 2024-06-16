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

/**
 * @brief The InjectorPlatform class
 * @namespace InjectorPlatform
 */
namespace DllInjector
{
    /**
     * @brief The InjectorPlatform class
     * @class InjectorPlatform, the base class for the platform specific implementations
     */
    class InjectorPlatform
    {
    public:
        static InjectorPlatform* CreatePlatform();
        virtual ~InjectorPlatform() noexcept = default;

        virtual unsigned int GetProcId(const char* procName) = 0;
        virtual bool InjectDLL(unsigned int procId, const char* dllPath) = 0;
    };

#if defined(_WIN32)
    /**
     * @brief The InjectorWindows class
     * @class InjectorWindows, the Windows specific implementation of the InjectorPlatform
     */
    class InjectorWindows : public InjectorPlatform 
    {
    public:
        unsigned int GetProcId(const char* procName) override;
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
#endif

#if defined(__linux__)
    /**
     * @brief The InjectorLinux class
     * @class InjectorLinux, the Linux specific implementation of the InjectorPlatform
     */
    class InjectorLinux : public InjectorPlatform
    {
    public:
        unsigned int GetProcId(const char* procName) override;
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
#endif

#if defined(__APPLE__)
    /**
     * @brief The InjectorMacOS class
     * @class InjectorMacOS, the MacOS specific implementation of the InjectorPlatform
     */
    class InjectorMacOS : public InjectorPlatform
    {
    public:
        unsigned int GetProcId(const char* procName) override;
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
#endif
}
