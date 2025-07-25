#include "../include/LIBMAN/WindowsInjector.h"

#if defined(_WIN32) || defined(_WIN64)

WinInject::WinInject(const char* dllName, const char* processName)
    : m_dllName(dllName), m_processName(processName), m_tragetProcess(nullptr), m_pathAddress(nullptr), m_loadLibraryAddress(nullptr) {}

WinInject::~WinInject()
{
    if (m_pathAddress != nullptr)
    {
        VirtualFreeEx(m_tragetProcess, m_pathAddress, 0, MEM_RELEASE);
    }

    if (m_tragetProcess != nullptr && m_tragetProcess != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_tragetProcess);
    }
}

std::vector<Result> WinInject::InjectDll()
{
    std::vector<Result> results;
    DWORD processId;

    results.push_back(FindProcessId(m_processName, processId));
    if (!results.back().success)
    {
        return results;
    }

    DWORD fullPathResult = GetFullPathNameA(m_dllName, MAX_PATH, m_fullDllPath, nullptr);
    if (fullPathResult == 0)
    {
        results.push_back({ false, "Failed to get the full path of the DLL: " + std::string(m_dllName) });
        return results;
    }

    results.push_back(OpenProcessHandle(processId));
    if (!results.back().success)
    {
        return results;
    }

    results.push_back(AllocateAndWriteMemory());
    if (!results.back().success)
    {
        return results;
    }

    results.push_back(GetLoadLibraryAddress());
    if (!results.back().success)
    {
        return results;
    }

    results.push_back(CreateRemoteThreadToLoadLibrary());
    if (!results.back().success)
    {
        return results;
    }

    results.push_back({ true, "DLL injected successfully" });
    return results;
}

Result WinInject::FindProcessId(const char* processName, DWORD& processId)
{
    PROCESSENTRY32 processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return { false, "Failed to create a snapshot of processes" };
    }

    if (Process32First(snapshot, &processEntry))
    {
        do
        {
            if (_stricmp(processEntry.szExeFile, processName) == 0)
            {
                processId = processEntry.th32ProcessID;
                CloseHandle(snapshot);
                return { true, "Process found: " + std::string(processName) + ", PID: " + std::to_string(processId) };
            }
        } while (Process32Next(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return { false, "Process not found: " + std::string(processName) };
}

Result WinInject::OpenProcessHandle(DWORD processId)
{
    m_tragetProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (m_tragetProcess == INVALID_HANDLE_VALUE)
    {
        return { false, "Failed to open a handle to the process: " + std::string(m_processName) };
    }

    return { true, "Process handle opened successfully" };
}

Result WinInject::AllocateAndWriteMemory()
{
    size_t dllPathLength = strlen(m_fullDllPath) + 1;
    m_pathAddress = VirtualAllocEx(m_tragetProcess, nullptr, dllPathLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (m_pathAddress == nullptr) 
    {
        return { false, "Failed to allocate memory in the process" };
    }

    std::ostringstream memoryAllocatedMessage;
    memoryAllocatedMessage << "Memory allocated at: 0x" << std::hex << reinterpret_cast<uintptr_t>(m_pathAddress);

    SIZE_T bytesWritten;
    BOOL writeResult = WriteProcessMemory(m_tragetProcess, m_pathAddress, m_fullDllPath, dllPathLength, &bytesWritten);
    if (!writeResult || bytesWritten != dllPathLength)
    {
        VirtualFreeEx(m_tragetProcess, m_pathAddress, 0, MEM_RELEASE);
        return { false, "Failed to write the DLL path to the process memory" };
    }

    std::ostringstream memoryWrittenMessage;
    memoryWrittenMessage << "DLL path written to process memory\n" << HexDump(m_fullDllPath, dllPathLength);

    return { true, memoryAllocatedMessage.str() + "\n" + memoryWrittenMessage.str() };
}

Result WinInject::GetLoadLibraryAddress()
{
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (kernel32 == nullptr)
    {
        return { false, "Failed to get the handle of kernel32.dll" };
    }

    m_loadLibraryAddress = (void*)GetProcAddress(kernel32, "LoadLibraryA");
    if (m_loadLibraryAddress == nullptr)
    {
        return { false, "Failed to get the address of LoadLibraryA" };
    }

    std::ostringstream loadLibraryAddressMessage;
    loadLibraryAddressMessage << "LoadLibraryA address: 0x" << std::hex << reinterpret_cast<uintptr_t>(m_loadLibraryAddress);

    return { true, loadLibraryAddressMessage.str() };
}

Result WinInject::CreateRemoteThreadToLoadLibrary()
{
    HANDLE threadHandle = CreateRemoteThread(m_tragetProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)m_loadLibraryAddress, m_pathAddress, 0, nullptr);
    if (threadHandle == INVALID_HANDLE_VALUE)
    {
        VirtualFreeEx(m_tragetProcess, m_pathAddress, 0, MEM_RELEASE);
        return { false, "Failed to create a remote thread" };
    }

    WaitForSingleObject(threadHandle, INFINITE);
    CloseHandle(threadHandle);

    std::ostringstream remoteThreadMessage;
    remoteThreadMessage << "Remote thread created and DLL injected";

    return { true, remoteThreadMessage.str() };
}

void WinInject::PrintResults(const std::vector<Result>& results)
{
    for (const auto& result : results)
    {
        std::cout << (result.success ? "[SUCCESS] " : "[ERROR] ") << result.message << std::endl;
    }
}

std::string WinInject::HexDump(const void* data, size_t size)
{
    std::ostringstream oss;
    const unsigned char* byteData = static_cast<const unsigned char*>(data);

    for (size_t i = 0; i < size; ++i)
    {
        if (i % 16 == 0)
            oss << std::endl;

        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byteData[i]) << " ";
    }

    return oss.str();
}
#endif