#include "../include/CORE/ProcessMonitor.h"

#if defined(_WIN32)
#include <windows.h>
#include <tlhelp32.h>
#endif

#if defined(__linux__)
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sysctl.h>
#endif


ProcessMonitor::ProcessMonitor() = default;

ProcessMonitor::~ProcessMonitor() = default;

std::vector<std::string> ProcessMonitor::GetRunningProcesses()
{
    std::vector<std::string> processes;

#if defined(_WIN32)
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Error: CreateToolhelp32Snapshot failed" << std::endl;
        return processes;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        std::cerr << "Error: Process32First failed" << std::endl;
        CloseHandle(hProcessSnap);
        return processes;
    }

    do
    {
        std::string ws(pe32.szExeFile);
        std::string processName(ws.begin(), ws.end());
        processes.push_back(processName);
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
#endif

#if defined(__linux__)
    DIR *dir;
    struct dirent *ent;
    char name[1024];

    if ((dir = opendir("/proc")) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            std::string pname = ent->d_name;

            if (pname.find_first_not_of("0123456789") == std::string::npos)
            {
                std::ifstream cmdlineFile("/proc/" + pname + "/cmdline");
                if (cmdlineFile)
                {
                    if (cmdlineFile.getline(name, sizeof(name)))
                    {
                        std::string processName(name);
                        processes.push_back(processName);
                    }
                    cmdlineFile.close();
                }
            }
        }
        closedir(dir);
    }
#endif

#if defined(__APPLE__)
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    size_t len;
    struct kinfo_proc *procs;
    int status;

    status = sysctl(mib, 4, NULL, &len, NULL, 0);
    if (status == -1)
    {
        perror("sysctl");
        return processes;
    }

    procs = (struct kinfo_proc *)malloc(len);
    status = sysctl(mib, 4, procs, &len, NULL, 0);
    if (status == -1)
    {
        perror("sysctl");
        free(procs);
        return processes;
    }

    int count = len / sizeof(struct kinfo_proc);
    for (int i = 0; i < count; ++i)
    {
        std::string processName(procs[i].kp_proc.p_comm);
        processes.push_back(processName);
    }

    free(procs);
#endif

    return processes;
}

bool ProcessMonitor::IsProcessRunning(const std::string& processName)
{
    std::vector<std::string> processes = GetRunningProcesses();

    for (const std::string& process : processes)
    {
        if (process == processName)
        {
            return true;
        }
    }
    return false;
}