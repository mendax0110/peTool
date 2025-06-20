#include "../include/CORE/Rebuilder.h"
#include "../include/UTILS/PrintUtils.h"
#include <iostream>
#include <fstream>

#if defined(_WIN32)
#include <Windows.h>
#include <cstdint>
#include <winnt.h>
#endif

#if defined(__linux__)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#endif

#if defined(__APPLE__)
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#endif

Rebuilder::Rebuilder() = default;

Rebuilder::~Rebuilder() = default;

bool Rebuilder::fixDump(const std::string& filePath)
{
    auto data = readFile(filePath);
    if (data.empty())
        return false;
    bool result = fixDumpInternal(data);
    if (result)
        result = writeFile(filePath, data);
    return result;
}

bool Rebuilder::wipeLocations(const std::string& filePath)
{
    auto data = readFile(filePath);
    if (data.empty())
        return false;
    bool result = wipeLocationsInternal(data);
    if (result)
        result = writeFile(filePath, data);
    return result;
}

bool Rebuilder::rebuildResourceDirectory(const std::string& filePath)
{
    auto data = readFile(filePath);
    if (data.empty())
        return false;
    bool result = rebuildResourceDirectoryInternal(data);
    if (result)
        result = writeFile(filePath, data);
    return result;
}

bool Rebuilder::validatePEFile(const std::string& filePath)
{
    auto data = readFile(filePath);
    if (data.empty())
        return false;
    return validatePEFileInternal(data);
}

bool Rebuilder::bindImports(const std::string& filePath)
{
    auto data = readFile(filePath);
    if (data.empty())
        return false;
    bool result = bindImportsInternal(data);
    if (result)
    {
        result = writeFile(filePath, data);
    }
    return result;
}

bool Rebuilder::changeImageBase(const std::string& filePath, uint64_t newImageBase)
{
    auto data = readFile(filePath);
    if (data.empty())
        return false;
    bool result = changeImageBaseInternal(data, newImageBase);
    if (result)
        result = writeFile(filePath, data);
    return result;
}

std::vector<uint8_t> Rebuilder::readFile(const std::string& filePath)
{
    std::ifstream fileStream(filePath, std::ios::binary);
    if (!fileStream.is_open())
    {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return {};
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    if (fileStream.fail() && !fileStream.eof())
    {
        std::cerr << "Error reading file: " << getErrorMessage() << std::endl;
        return {};
    }
    return data;
}

bool Rebuilder::writeFile(const std::string& filePath, const std::vector<uint8_t>& data)
{
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
        return false;
    }
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (file.fail())
    {
        std::cerr << "Error writing file: " << getErrorMessage() << std::endl;
        return false;
    }
    return true;
}

bool Rebuilder::fixDumpInternal(std::vector<uint8_t>& data)
{
    std::cout << "Fixing dump file" << std::endl;
#if defined(_WIN32)
    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(data.data());

    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        std::cerr << "Invalid DOS header signature." << std::endl;
        return false;
    }

    auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(data.data() + dosHeader->e_lfanew);

    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
    {
        std::cerr << "Invalid NT header signature." << std::endl;
        return false;
    }

    auto sectionHeader = IMAGE_FIRST_SECTION(ntHeader);
    for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++, ++sectionHeader)
    {
        if (sectionHeader->PointerToRawData == 0)
        {
            std::cerr << "Invalid section header." << std::endl;
            return false;
        }

        sectionHeader->PointerToRawData = sectionHeader->VirtualAddress;
        sectionHeader->SizeOfRawData = sectionHeader->Misc.VirtualSize;

        if (sectionHeader->SizeOfRawData == 0)
        {
            std::cerr << "Invalid section size." << std::endl;
            return false;
        }

        // Clear all unwanted characteristics flags in one operation
        const uint32_t unwantedFlags = IMAGE_SCN_MEM_DISCARDABLE |
                                       IMAGE_SCN_MEM_NOT_CACHED |
                                       IMAGE_SCN_MEM_NOT_PAGED |
                                       IMAGE_SCN_MEM_SHARED |
                                       IMAGE_SCN_MEM_EXECUTE |
                                       IMAGE_SCN_MEM_READ |
                                       IMAGE_SCN_MEM_WRITE |
                                       IMAGE_SCN_CNT_CODE |
                                       IMAGE_SCN_CNT_INITIALIZED_DATA |
                                       IMAGE_SCN_CNT_UNINITIALIZED_DATA |
                                       IMAGE_SCN_LNK_INFO |
                                       IMAGE_SCN_LNK_REMOVE |
                                       IMAGE_SCN_LNK_COMDAT |
                                       IMAGE_SCN_ALIGN_1BYTES |
                                       IMAGE_SCN_ALIGN_2BYTES |
                                       IMAGE_SCN_ALIGN_4BYTES |
                                       IMAGE_SCN_ALIGN_8BYTES;

        sectionHeader->Characteristics &= ~unwantedFlags;
    }
    return true;
#endif

#if defined(__APPLE__)
    auto machHeader = reinterpret_cast<mach_header*>(data.data());
    auto command = reinterpret_cast<load_command*>(data.data() + sizeof(mach_header));

    if (machHeader->magic != MH_MAGIC)
    {
        std::cerr << "Invalid Mach-O header magic." << std::endl;
        return false;
    }

    auto segmentCommand = reinterpret_cast<segment_command*>(command);

    if (segmentCommand->cmd != LC_SEGMENT)
    {
        std::cerr << "Invalid Mach-O segment command." << std::endl;
        return false;
    }

    auto section = reinterpret_cast<struct section*>(segmentCommand + 1);

    for (int i = 0; i < segmentCommand->nsects; i++, ++section)
    {
        if (section->offset == 0)
        {
            std::cerr << "Invalid section offset." << std::endl;
            return false;
        }

        section->offset = section->addr;
        section->size = section->size;

        if (section->size == 0)
        {
            std::cerr << "Invalid section size." << std::endl;
            return false;
        }

        // Clear all unwanted flags in one operation
        const uint32_t unwantedFlags = S_REGULAR |
                                       S_ZEROFILL |
                                       S_CSTRING_LITERALS |
                                       S_4BYTE_LITERALS |
                                       S_8BYTE_LITERALS |
                                       S_LITERAL_POINTERS |
                                       S_NON_LAZY_SYMBOL_POINTERS |
                                       S_LAZY_SYMBOL_POINTERS |
                                       S_SYMBOL_STUBS |
                                       S_MOD_INIT_FUNC_POINTERS |
                                       S_MOD_TERM_FUNC_POINTERS |
                                       S_COALESCED |
                                       S_GB_ZEROFILL |
                                       S_INTERPOSING |
                                       S_DTRACE_DOF |
                                       S_LAZY_DYLIB_SYMBOL_POINTERS |
                                       S_THREAD_LOCAL_REGULAR |
                                       S_THREAD_LOCAL_ZEROFILL |
                                       S_THREAD_LOCAL_VARIABLES |
                                       S_THREAD_LOCAL_INIT_FUNCTION_POINTERS;

        section->flags &= ~unwantedFlags;
    }
    return true;
#endif
}

bool Rebuilder::wipeLocationsInternal(std::vector<uint8_t>& data) {
    std::cout << "Wiping locations in file" << std::endl;
#if defined(_WIN32)
    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(data.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        std::cerr << "Invalid DOS header signature." << std::endl;
        return false;
    }

    auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(data.data() + dosHeader->e_lfanew);
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
    {
        std::cerr << "Invalid NT header signature." << std::endl;
        return false;
    }

    memset(&ntHeader->OptionalHeader, 0, sizeof(ntHeader->OptionalHeader));
    return true;
#endif

#if defined(__APPLE__)
    auto machHeader = reinterpret_cast<mach_header*>(data.data());
    if (machHeader->magic != MH_MAGIC)
    {
        std::cerr << "Invalid Mach-O header magic." << std::endl;
        return false;
    }

    auto command = reinterpret_cast<load_command*>(data.data() + sizeof(mach_header));
    auto segmentCommand = reinterpret_cast<segment_command*>(command);
    auto section = reinterpret_cast<struct section*>(segmentCommand + 1);

    for (int i = 0; i < segmentCommand->nsects; i++, ++section)
    {
        if (section->offset == 0)
        {
            std::cerr << "Invalid section offset." << std::endl;
            return false;
        }

        memset(section, 0, sizeof(struct section));
    }
    return true;
#endif
}

bool Rebuilder::rebuildResourceDirectoryInternal(std::vector<uint8_t>& data)
{
    std::cout << "Rebuilding resource directory" << std::endl;
#if defined(__WIN32) || defined(_WIN32)
    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(data.data());
    auto ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(data.data() + dosHeader->e_lfanew);

    auto& resourceDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE];
    resourceDirectory.VirtualAddress = 0;
    resourceDirectory.Size = 0;

    return true;
#elif defined(__APPLE__)
    auto command = reinterpret_cast<load_command*>(data.data() + sizeof(mach_header));

    auto segmentCommand = reinterpret_cast<segment_command*>(command);
    auto section = reinterpret_cast<struct section*>(segmentCommand + 1);

    for (int i = 0; i < segmentCommand->nsects; i++, ++section)
    {
        if (strcmp(section->sectname, "__text") == 0)
        {
            section->offset = 0;
            section->size = 0;
            return true;
        }
    }
    return false;
#endif
}

bool Rebuilder::validatePEFileInternal(std::vector<uint8_t>& data)
{
    std::cout << "Validating PE file" << std::endl;
    auto it = data.data();

#if defined(_WIN32)
    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(it);
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        std::cerr << "Invalid DOS header signature." << std::endl;
        return false;
    }

    auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(it + dosHeader->e_lfanew);
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
    {
        std::cerr << "Invalid NT header signature." << std::endl;
        return false;
    }

    return true;
#endif

#if defined(__APPLE__)
    auto machHeader = reinterpret_cast<mach_header*>(it);
    if (machHeader->magic != MH_MAGIC)
    {
        std::cerr << "Invalid Mach-O header magic." << std::endl;
        return false;
    }

    return true;
#endif
}

bool Rebuilder::bindImportsInternal(std::vector<uint8_t>& data)
{
    std::cout << "Binding imports" << std::endl;
#if defined(_WIN32)
    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(data.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        std::cerr << "Invalid DOS header signature." << std::endl;
        return false;
    }

    auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(data.data() + dosHeader->e_lfanew);
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
    {
        std::cerr << "Invalid NT header signature." << std::endl;
        return false;
    }

    auto importDescriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(data.data() + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while (!importDescriptor->Name)
    {
        auto name = reinterpret_cast<char*>(data.data() + importDescriptor->Name);
        auto thunk = reinterpret_cast<IMAGE_THUNK_DATA*>(data.data() + importDescriptor->OriginalFirstThunk);
        auto firstThunk = reinterpret_cast<IMAGE_THUNK_DATA*>(data.data() + importDescriptor->FirstThunk);

        HMODULE module = LoadLibraryA(name);
        if (!module)
        {
            std::cerr << "Failed to load library: " << name << std::endl;
            return false;
        }

        while (thunk->u1.AddressOfData)
        {
            auto importByName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(data.data() + thunk->u1.AddressOfData);
            auto function = GetProcAddress(module, importByName->Name);
            if (!function)
            {
                std::cerr << "Failed to get function address: " << importByName->Name << std::endl;
                return false;
            }
            firstThunk->u1.Function = reinterpret_cast<uintptr_t>(function);

            ++thunk;
            ++firstThunk;
        }

        ++importDescriptor;
        return true;
    }
#endif

#if defined(__APPLE__)
    auto machHeader = reinterpret_cast<mach_header*>(data.data());
    if (machHeader->magic != MH_MAGIC)
    {
        std::cerr << "Invalid Mach-O header magic." << std::endl;
        return false;
    }

    auto command = reinterpret_cast<load_command*>(data.data() + sizeof(mach_header));
    auto segmentCommand = reinterpret_cast<segment_command*>(command);
    auto section = reinterpret_cast<struct section*>(segmentCommand + 1);

    for (int i = 0; i < segmentCommand->nsects; i++, ++section)
    {
        if (strcmp(section->sectname, "__text") == 0)
        {
            section->offset = 0;
            section->size = 0;
            return true;
        }
    }
    return false;
#endif

    return false;
}

bool Rebuilder::changeImageBaseInternal(std::vector<uint8_t>& data, uint64_t newImageBase)
{
    std::cout << "Changing image base" << std::endl;

#if defined(_WIN32)
    auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(data.data());
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
    {
        std::cerr << "Invalid DOS header signature." << std::endl;
        return false;
    }

    auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(data.data() + dosHeader->e_lfanew);
    if (ntHeader->Signature != IMAGE_NT_SIGNATURE)
    {
        std::cerr << "Invalid NT header signature." << std::endl;
        return false;
    }

    ntHeader->OptionalHeader.ImageBase = newImageBase;
    return true;
#endif

#if defined(__APPLE__)
    auto machHeader = reinterpret_cast<mach_header*>(data.data());
    if (machHeader->magic != MH_MAGIC)
    {
        std::cerr << "Invalid Mach-O header magic." << std::endl;
        return false;
    }

    machHeader->ncmds = newImageBase;
    return true;
#endif
}