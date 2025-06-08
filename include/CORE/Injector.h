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

/// @brief DllInjector namespace, which contains the InjectorPlatform class and its platform-specific implementations \namespace DllInjector
namespace DllInjector
{
    /// @brief The InjectorPlatform class \class InjectorPlatform
    class InjectorPlatform
    {
    public:

        /**
         * @brief Create a platform-specific InjectorPlatform instance
         * @return Pointer to the created InjectorPlatform instance
         */
        static InjectorPlatform* CreatePlatform();

        /**
         * @brief Destroy the InjectorPlatform object
         */
        virtual ~InjectorPlatform() noexcept = default;

        /**
         * @brief Get the process ID of the specified process name
         * @param procName The name of the process
         * @return The process ID if found, 0 otherwise
         */
        virtual unsigned int GetProcId(const char* procName) = 0;

        /**
         * @brief Inject a DLL into the specified process
         * @param procId The process ID to inject the DLL into
         * @param dllPath The path to the DLL to inject
         * @return true if the injection was successful, false otherwise
         */
        virtual bool InjectDLL(unsigned int procId, const char* dllPath) = 0;
    };

#if defined(_WIN32)
    /// @brief The InjectorWindows class \class InjectorWindows, the Windows specific implementation of the InjectorPlatform
    class InjectorWindows : public InjectorPlatform 
    {
    public:

        /**
         * @brief Get the process ID of the specified process name
         * @param procName The name of the process
         * @return The process ID if found, 0 otherwise
         */
        unsigned int GetProcId(const char* procName) override;

        /**
         * @brief Inject a DLL into the specified process
         * @param procId The process ID to inject the DLL into
         * @param dllPath The path to the DLL to inject
         * @return true if the injection was successful, false otherwise
         */
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
#endif

#if defined(__linux__)
    /// @brief The InjectorLinux class \class InjectorLinux, the Linux specific implementation of the InjectorPlatform
    class InjectorLinux : public InjectorPlatform
    {
    public:

        /**
         * @brief Get the process ID of the specified process name
         * @param procName The name of the process
         * @return The process ID if found, 0 otherwise
         */
        unsigned int GetProcId(const char* procName) override;

        /**
         * @brief Inject a DLL into the specified process
         * @param procId The process ID to inject the DLL into
         * @param dllPath The path to the DLL to inject
         * @return true if the injection was successful, false otherwise
         */
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
#endif

#if defined(__APPLE__)
    /// @brief The InjectorMacOS class \class InjectorMacOS, the macOS specific implementation of the InjectorPlatform
    class InjectorMacOS : public InjectorPlatform
    {
    public:

        /**
         * @brief Get the process ID of the specified process name
         * @param procName The name of the process
         * @return The process ID if found, 0 otherwise
         */
        unsigned int GetProcId(const char* procName) override;

        /**
         * @brief Inject a DLL into the specified process
         * @param procId The process ID to inject the DLL into
         * @param dllPath The path to the DLL to inject
         * @return true if the injection was successful, false otherwise
         */
        bool InjectDLL(unsigned int procId, const char* dllPath) override;
    };
#endif
}
