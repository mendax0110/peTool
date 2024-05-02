#include "./include/Injector.h"

using namespace DllInjector;

#if defined(_WIN32)
#include <Windows.h>
#include <TlHelp32.h>
#endif

#if defined(__linux__)
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#endif 

#if defined(__APPLE__)
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <libproc.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/mach_vm.h>
#include <mach/thread_act.h>
#include <mach/thread_status.h>
#include <dlfcn.h>
#endif


InjectorPlatform::InjectorPlatform()
{
#if defined(_WIN32)
    platform = new InjectorWindows();
#elif defined(__linux__)
    platform = new InjectorLinux();
#elif defined(__APPLE__)
    platform = new InjectorMacOS();
#endif
}

InjectorPlatform::~InjectorPlatform()
{
    delete platform;
}

unsigned int InjectorWindows::GetProcId(const char* procName, unsigned int pid)
{
#if (_WIN32)
    DWORD processId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32))
        {
            do
            {
                if (pid > 0 && pe32.th32ProcessID == pid)
                {
                    processId = pe32.th32ProcessID;
                    break;
                }
                else if (procName != nullptr && strcmp(pe32.szExeFile, procName) == 0)
                {
                    processId = pe32.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return processId;
#endif
}

bool InjectorWindows::InjectDLL(unsigned int procId, const char* dllPath)
{
#if (_WIN32)
    if (procId == 0 || dllPath == nullptr)
        return false;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    if (hProcess == NULL)
        return false;

    LPVOID pLibRemote = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (pLibRemote == NULL)
    {
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pLibRemote, dllPath, strlen(dllPath) + 1, NULL))
    {
        VirtualFreeEx(hProcess, pLibRemote, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pLibRemote, 0, NULL);
    if (hThread == NULL)
    {
        VirtualFreeEx(hProcess, pLibRemote, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pLibRemote, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return true;
#endif
}

unsigned int InjectorLinux::GetProcId(const char* procName, unsigned int pid)
{
#if (__linux__)
    if (pid > 0)
        return pid;

    if (procName == nullptr)
        return 0;

    FILE* file = fopen("/proc", "r");
    if (file == nullptr)
        return 0;

    unsigned int processId = 0;
    char path[PATH_MAX];
    while (fgets(path, PATH_MAX, file))
    {
        if (strstr(path, procName) != nullptr)
        {
            char* end;
            unsigned int tempPid = strtol(path, &end, 10);
            if (*end == '\n')
            {
                processId = tempPid;
                break;
            }
        }
    }
    fclose(file);
    return processId;
#endif
}

bool InjectorLinux::InjectDLL(unsigned int procId, const char* dllPath)
{
#if (__linux__)
    if (procId == 0 || dllPath == nullptr)
        return false;

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "/proc/%u/maps", procId);
    FILE* maps = fopen(buffer, "r");
    if (!maps)
        return false;

    bool found = false;
    while (!found && fgets(buffer, sizeof(buffer), maps))
    {
        if (strstr(buffer, dllPath))
            found = true;
    }
    fclose(maps);

    if (!found)
        return false;

    snprintf(buffer, sizeof(buffer), "/proc/%u/fd/1", procId);
    FILE* log = fopen(buffer, "w");
    if (!log)
        return false;

    fprintf(log, "Injected %s into PID %u\n", dllPath, procId);
    fclose(log);

    return true;
#endif
}

unsigned int InjectorMacOS::GetProcId(const char* procName, unsigned int pid)
{
#if (__APPLE__)
    if (pid > 0)
        return pid;

    if (procName == nullptr)
        return 0;

    pid_t processId = 0;
    const int MAX_PID_COUNT = 4096;
    pid_t pids[MAX_PID_COUNT];

    int numberOfProcesses = proc_listallpids(nullptr, 0);
    
    if (numberOfProcesses > 0 && numberOfProcesses <= MAX_PID_COUNT)
    {
        proc_listallpids(pids, sizeof(pids));

        for (int i = 0; i < numberOfProcesses; ++i)
        {
            char nameBuffer[PROC_PIDPATHINFO_MAXSIZE];
            proc_pidpath(pids[i], nameBuffer, sizeof(nameBuffer));
            if (strstr(nameBuffer, procName) != nullptr)
            {
                processId = pids[i];
                break;
            }
        }
    }

    return processId;
#endif
}

bool InjectorMacOS::InjectDLL(unsigned int procId, const char* dllPath)
{
#if (__APPLE__)
    if (procId == 0 || dllPath == nullptr)
        return false;

    task_t task;
    kern_return_t kret = task_for_pid(mach_task_self(), procId, &task);
    if (kret != KERN_SUCCESS)
        return false;

    mach_vm_address_t remoteAddress;
    size_t pathLength = strlen(dllPath) + 1;
    kret = mach_vm_allocate(task, &remoteAddress, pathLength, VM_FLAGS_ANYWHERE);
    if (kret != KERN_SUCCESS)
        return false;

    kret = mach_vm_write(task, remoteAddress, (vm_offset_t)dllPath, pathLength);
    if (kret != KERN_SUCCESS)
    {
        mach_vm_deallocate(task, remoteAddress, pathLength);
        return false;
    }

    kret = pthread_create((pthread_t*)remoteAddress, nullptr, (void*(*)(void*))dlopen, (void*)remoteAddress);
    if (kret != KERN_SUCCESS)
    {
        mach_vm_deallocate(task, remoteAddress, pathLength);
        return false;
    }

    return true;
#endif
}
