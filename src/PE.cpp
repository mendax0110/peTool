#include "../include/CORE/PE.h"

#if defined(_WIN32)
#include <windows.h>
#include <winnt.h>
#include <cstdint> 
#endif

#if defined(__linux__)
#include <linux/elf.h>
#include <cstring>
#include <string>
#endif

#if defined(__APPLE__)
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#endif

#include <iostream>
#include <string>

using namespace PeInternals;

/**
 * @brief Create a new ResourceDirectoryTraverser object
 * @return A new ResourceDirectoryTraverser object
 */
ResourceDirectoryTraverser* ResourceDirectoryTraverserFactory::createTraverser()
{
#if defined(_WIN32)
    return new WinResourceDirectoryTraverser();
#elif defined(__linux__)
    return new LinuxResourceDirectoryTraverser();
#elif defined(__APPLE__)
    return new AppleResourceDirectoryTraverser();
#else
    return nullptr;
#endif
}

/**
 * @brief Traverse the resource directory
 * @param fileData The file data
 * @param level The level of the resource directory
 * @param parentName The name of the parent directory
 */
void ResourceDirectory::traverse(const std::vector<uint8_t>& fileData, int level, const std::string& parentName)
{
    if (traverser != nullptr)
    {
        traverser->traverse(fileData, this, level, parentName);
    }
}

/**
 * @brief Set the traverser
 * @param traverser The traverser to set
 */
void ResourceDirectory::setTraverser(ResourceDirectoryTraverser* traverser)
{
    this->traverser = traverser;
}

/**
 * @brief Get the resource directory offset
 * @param fileData The file data
 * @return The resource directory offset
 */
uint32_t PE::GetResourceDirectoryOffset(const std::vector<uint8_t>& fileData)
{
    std::vector<uint8_t> mutableFileData(fileData.begin(), fileData.end());

    #if defined(_WIN32)
    if (mutableFileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
    {
        std::cerr << "Error: File is too small to contain a valid PE header.\n";
        return 0;
    }
    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(&mutableFileData[0]);
    DWORD peHeaderOffset = dosHeader->e_lfanew;

    PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(&mutableFileData[peHeaderOffset]);

    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        std::cerr << "Error: Invalid PE header signature.\n";
        return 0;
    }

    DWORD resourceRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
    DWORD resourceSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;

    if (resourceRVA == 0 || resourceSize == 0)
    {
        std::cout << "No resource directory found.\n";
        return 0;
    }

    DWORD resourceOffset = resourceRVA;
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
    {
        PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(&mutableFileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]);
        if (resourceRVA >= sectionHeader->VirtualAddress && resourceRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
        {
            resourceOffset = resourceRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
            break;
        }
    }

    return resourceOffset;
    #endif

    #if defined(__linux__)
    // implementation to get resourceOffset for Linux
    return 0;
    #endif

    #if defined(__APPLE__)
    // implementation to get resourceOffset for Apple
    if (mutableFileData.size() < sizeof(struct mach_header))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return 0;
    }

    const auto* machHeader = reinterpret_cast<const struct mach_header*>(&mutableFileData[0]);
    const auto* loadCommand = reinterpret_cast<const struct load_command*>(&mutableFileData[sizeof(struct mach_header)]);

    for (uint32_t i = 0; i < machHeader->ncmds; ++i)
    {
        if (loadCommand->cmd == LC_SEGMENT)
        {
            const auto* segmentCommand = reinterpret_cast<const struct segment_command*>(loadCommand);
            const auto* section = reinterpret_cast<const struct section*>(segmentCommand + 1);
            for (uint32_t j = 0; j < segmentCommand->nsects; ++j)
            {
                if (strcmp(section->sectname, "__text") == 0)
                {
                    return section->offset;
                }
                section = reinterpret_cast<const struct section*>(section + 1);
            }
        }
        loadCommand = reinterpret_cast<const struct load_command*>(reinterpret_cast<const uint8_t*>(loadCommand) + loadCommand->cmdsize);
    }

    return 0;

    #endif
}

#if defined(_WIN32)
/**
 * @brief Traverse the resource directory
 * @param fileData The file data
 * @param resourceDirectory The resource directory
 * @param level The level of the resource directory
 * @param parentName The name of the parent directory
 */
void WinResourceDirectoryTraverser::traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName)
{
    uint32_t RESOURCE_DIRECTORY_OFFSET = PE::GetResourceDirectoryOffset(fileData);
    std::vector<uint8_t> mutableFileData(fileData.begin(), fileData.end());
    const PIMAGE_RESOURCE_DIRECTORY resourceDirectoryData = reinterpret_cast<const PIMAGE_RESOURCE_DIRECTORY>(&mutableFileData[RESOURCE_DIRECTORY_OFFSET]);
    
    const PIMAGE_RESOURCE_DIRECTORY_ENTRY resourceEntries = reinterpret_cast<const PIMAGE_RESOURCE_DIRECTORY_ENTRY>(resourceDirectoryData + 1);
    for (DWORD i = 0; i < resourceDirectoryData->NumberOfNamedEntries + resourceDirectoryData->NumberOfIdEntries; ++i)
    {
        const PIMAGE_RESOURCE_DIRECTORY_ENTRY entry = &resourceEntries[i];
        if (entry->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
        {
            const PIMAGE_RESOURCE_DIRECTORY subDirectory = reinterpret_cast<const PIMAGE_RESOURCE_DIRECTORY>(&mutableFileData[entry->OffsetToDirectory & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY]);
            std::string directoryName = std::to_string(entry->Name);

            std::cout << "Level " << level << ": " << parentName << "Directory\n";
            std::cout << "OffsetToData: 0x" << std::hex << entry->OffsetToData << "\n";
            std::cout << "OffsetToDirectory: 0x" << std::hex << entry->OffsetToDirectory << "\n";
            std::cout << "Directory Name: " << directoryName << "\n";
            std::cout << "Number of Entries: " << subDirectory->NumberOfNamedEntries + subDirectory->NumberOfIdEntries << "\n";
        }
        else
        {
            const PIMAGE_RESOURCE_DATA_ENTRY dataEntry = reinterpret_cast<const PIMAGE_RESOURCE_DATA_ENTRY>(&mutableFileData[entry->OffsetToData]);
            std::cout << "Level " << level << ": " << parentName << "ID: " << entry->Name << "\n";

            std::cout << "Data RVA: 0x" << std::hex << dataEntry->OffsetToData << ", Size: " << std::dec << dataEntry->Size << "\n";
        }
    }
}
#endif


#if defined(__linux__)
/**
 * @brief Traverse the resource directory
 * @param fileData The file data
 * @param resourceDirectory The resource directory
 * @param level The level of the resource directory
 * @param parentName The name of the parent directory
 */
void LinuxResourceDirectoryTraverser::traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName)
{
    auto elfHeader = reinterpret_cast<const Elf64_Ehdr*>(fileData.data());
    auto sectionHeader = reinterpret_cast<const Elf64_Shdr*>(fileData.data() + elfHeader->e_shoff);

    for (int i = 0; i < elfHeader->e_shnum; ++i)
    {
        if (sectionHeader->sh_type == SHT_PROGBITS &&
            sectionHeader->sh_flags & SHF_ALLOC &&
            sectionHeader->sh_size > 0 &&
            sectionHeader->sh_addr == elfHeader->e_entry)
        {
            std::cout << "Resource directory found.\n";
            std::cout << "Section Name: " << sectionHeader->sh_name << std::endl;
            std::cout << "Virtual Size: " << sectionHeader->sh_size << std::endl;
            std::cout << "Virtual Address: 0x" << std::hex << sectionHeader->sh_addr << std::endl;
            std::cout << "Size of Raw Data: " << sectionHeader->sh_size << std::endl;
            std::cout << "Pointer to Raw Data: 0x" << std::hex << sectionHeader->sh_offset << std::endl;
            std::cout << "Characteristics: 0x" << std::hex << sectionHeader->sh_flags << std::endl;
            return;
        }
        sectionHeader = reinterpret_cast<const Elf64_Shdr*>(reinterpret_cast<const uint8_t*>(sectionHeader) + elfHeader->e_shentsize);
    }
    std::cout << "No resource directory found.\n";
}
#endif

#if defined(__APPLE__)
/**
 * @brief Traverse the resource directory
 * @param fileData The file data
 * @param resourceDirectory The resource directory
 * @param level The level of the resource directory
 * @param parentName The name of the parent directory
 */
void AppleResourceDirectoryTraverser::traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName)
{
    auto resourceDirectoryData = reinterpret_cast<const struct mach_header*>(fileData.data());
    auto loadCommand = reinterpret_cast<const struct load_command*>(resourceDirectoryData + 1);
    
    for (uint32_t i = 0; i < resourceDirectoryData->ncmds; ++i)
    {
        if (loadCommand->cmd == LC_SEGMENT)
        {
            auto segmentCommand = reinterpret_cast<const struct segment_command*>(loadCommand);
            auto section = reinterpret_cast<const struct section*>(segmentCommand + 1);
            for (uint32_t j = 0; j < segmentCommand->nsects; ++j)
            {
                if (strcmp(section->sectname, "__text") == 0)
                {
                    std::cout << "Resource directory found.\n";
                    std::cout << "Section Name: " << section->sectname << std::endl;
                    std::cout << "Virtual Size: " << section->size << std::endl;
                    std::cout << "Virtual Address: 0x" << std::hex << section->addr << std::endl;
                    std::cout << "Size of Raw Data: " << section->size << std::endl;
                    std::cout << "Pointer to Raw Data: 0x" << std::hex << section->offset << std::endl;
                    std::cout << "Characteristics: 0x" << std::hex << section->flags << std::endl;
                    return;
                }
                section = reinterpret_cast<const struct section*>(section + 1);
            }
        }
        loadCommand = reinterpret_cast<const struct load_command*>(reinterpret_cast<const uint8_t*>(loadCommand) + loadCommand->cmdsize);
    }
    std::cout << "No resource directory found.\n";
}
#endif


#if defined(_WIN32)
/**
 * @brief Extract the import table from a PE file
 * @param fileData The file data
 */
void PE::extractImportTable(const std::vector<uint8_t>& fileData)
{
	if (fileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
	{
		std::cerr << "Error: File is too small to contain a valid PE header.\n";
		return;
	}

	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<uint8_t*>(&fileData[0]));
	DWORD peHeaderOffset = dosHeader->e_lfanew;

	PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<uint8_t*>(&fileData[peHeaderOffset]));

	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		std::cerr << "Error: Invalid PE header signature.\n";
		return;
	}

	DWORD importTableRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD importTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

	if (importTableRVA == 0 || importTableSize == 0)
	{
		std::cout << "No import table found.\n";
		return;
	}

	DWORD importTableOffset = importTableRVA;
	for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
	{
		PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
		if (importTableRVA >= sectionHeader->VirtualAddress && importTableRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
		{
			importTableOffset = importTableRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
            std::cout << "Import Table Offset: " << importTableOffset << std::endl;
			break;
		}
	}

	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(const_cast<uint8_t*>(&fileData[importTableOffset]));

	std::cout << "Imported Functions:\n";
	while (importDescriptor->OriginalFirstThunk != 0)
	{
        PIMAGE_THUNK_DATA thunk = reinterpret_cast<PIMAGE_THUNK_DATA>(const_cast<uint8_t*>(&fileData[importDescriptor->OriginalFirstThunk]));
		while (thunk->u1.AddressOfData != 0)
		{
            uint64_t addressOfData = thunk->u1.AddressOfData;
            if (addressOfData >= fileData.size())
            {
                std::cerr << "AddressOfData out of bounds: " << addressOfData << std::endl;
                break;
            }
            PIMAGE_IMPORT_BY_NAME importByName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(const_cast<uint8_t*>(fileData.data() + addressOfData));

			std::cout << importByName->Name << std::endl;
			++thunk;
		}
		++importDescriptor;
	}
}

/**
 * @brief Extract the export table from a PE file
 * @param fileData The file data
 */
void PE::extractExportTable(const std::vector<uint8_t>& fileData)
{
	if (fileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
	{
		std::cerr << "Error: File is too small to contain a valid PE header.\n";
		return;
	}

	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<uint8_t*>(&fileData[0]));
	DWORD peHeaderOffset = dosHeader->e_lfanew;

	PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<uint8_t*>(&fileData[peHeaderOffset]));

	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		std::cerr << "Error: Invalid PE header signature.\n";
		return;
	}

	DWORD exportTableRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	DWORD exportTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

	if (exportTableRVA == 0 || exportTableSize == 0)
	{
		std::cout << "No export table found.\n";
		return;
	}

	DWORD exportTableOffset = exportTableRVA;
	for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
	{
		PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
		if (exportTableRVA >= sectionHeader->VirtualAddress && exportTableRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
		{
			exportTableOffset = exportTableRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
			break;
		}
	}

	PIMAGE_EXPORT_DIRECTORY exportDirectory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(const_cast<uint8_t*>(&fileData[exportTableOffset]));

	DWORD* exportedFunctions = reinterpret_cast<DWORD*>(const_cast<uint8_t*>(&fileData[exportDirectory->AddressOfFunctions]));
	DWORD numberOfFunctions = exportDirectory->NumberOfFunctions;

	std::cout << "Exported Functions:\n";
	for (DWORD i = 0; i < numberOfFunctions; ++i)
	{
		DWORD functionRVA = exportedFunctions[i];
		if (functionRVA == 0)
			continue;

		DWORD functionOffset = functionRVA;
		for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
		{
			PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
			if (functionRVA >= sectionHeader->VirtualAddress && functionRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
			{
				functionOffset = functionRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
				break;
			}
		}

		const char* functionName = reinterpret_cast<const char*>(&fileData[functionOffset]);
		std::cout << functionName << std::endl;
	}
}

/**
 * @brief Extract the resources from a PE file
 * @param fileData The file data
 */
void PE::extractResources(const std::vector<uint8_t>& fileData)
{
	if (fileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
	{
		std::cerr << "Error: File is too small to contain a valid PE header.\n";
		return;
	}

	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<uint8_t*>(&fileData[0]));
	DWORD peHeaderOffset = dosHeader->e_lfanew;

	PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<uint8_t*>(&fileData[peHeaderOffset]));

	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		std::cerr << "Error: Invalid PE header signature.\n";
		return;
	}

	DWORD resourceRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	DWORD resourceSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;

	if (resourceRVA == 0 || resourceSize == 0)
	{
		std::cout << "No resource directory found.\n";
		return;
	}

	DWORD resourceOffset = resourceRVA;
	for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
	{
		PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
		if (resourceRVA >= sectionHeader->VirtualAddress && resourceRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
		{
			resourceOffset = resourceRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
			break;
		}
	}

	PIMAGE_RESOURCE_DIRECTORY resourceDirectory = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(const_cast<uint8_t*>(&fileData[resourceOffset]));

    ResourceDirectory resourceDir;
    ResourceDirectoryTraverser* traverser = ResourceDirectoryTraverserFactory::createTraverser();
    WinResourceDirectoryTraverser resourceTraverser;
    resourceTraverser.traverse(fileData, &resourceDir, 0, "");
}

/**
 * @brief Extract the section info from a PE file
 * @param fileData The file data
 */
void PE::extractSectionInfo(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
    {
        std::cerr << "Error: File is too small to contain a valid PE header.\n";
        return;
    }

    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<uint8_t*>(&fileData[0]));
    DWORD peHeaderOffset = dosHeader->e_lfanew;

    PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<uint8_t*>(&fileData[peHeaderOffset]));

    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        std::cerr << "Error: Invalid PE header signature.\n";
        return;
    }

    std::cout << "Section Info:\n";
    for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
    {
        PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
        std::cout << "Section Name: " << sectionHeader->Name << std::endl;
        std::cout << "Virtual Size: " << sectionHeader->Misc.VirtualSize << std::endl;
        std::cout << "Virtual Address: 0x" << std::hex << sectionHeader->VirtualAddress << std::endl;
        std::cout << "Size of Raw Data: " << sectionHeader->SizeOfRawData << std::endl;
        std::cout << "Pointer to Raw Data: 0x" << std::hex << sectionHeader->PointerToRawData << std::endl;
        std::cout << "Characteristics: 0x" << std::hex << sectionHeader->Characteristics << std::endl;
        std::cout << std::endl;
    }
}

/**
 * @brief Extract the headers from a PE file
 * @param fileData The file data
 */
void PE::parseHeaders(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
    {
        std::cerr << "Error: File is too small to contain a valid PE header.\n";
        return;
    }

    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<uint8_t*>(&fileData[0]));
    DWORD peHeaderOffset = dosHeader->e_lfanew;

    PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<uint8_t*>(&fileData[peHeaderOffset]));

    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
    {
        std::cerr << "Error: Invalid PE header signature.\n";
        return;
    }

    std::cout << "PE Header Info:\n";
    std::cout << "Signature: 0x" << std::hex << ntHeaders->Signature << std::endl;
    std::cout << "Machine: 0x" << std::hex << ntHeaders->FileHeader.Machine << std::endl;
    std::cout << "Number of Sections: " << ntHeaders->FileHeader.NumberOfSections << std::endl;
    std::cout << "Time Date Stamp: " << ntHeaders->FileHeader.TimeDateStamp << std::endl;
    std::cout << "Pointer to Symbol Table: 0x" << std::hex << ntHeaders->FileHeader.PointerToSymbolTable << std::endl;
    std::cout << "Number of Symbols: " << ntHeaders->FileHeader.NumberOfSymbols << std::endl;
    std::cout << "Size of Optional Header: " << ntHeaders->FileHeader.SizeOfOptionalHeader << std::endl;
    std::cout << "Characteristics: 0x" << std::hex << ntHeaders->FileHeader.Characteristics << std::endl;
    std::cout << std::endl;

    std::cout << "Optional Header Info:\n";
    std::cout << "Magic: 0x" << std::hex << ntHeaders->OptionalHeader.Magic << std::endl;
    std::cout << "Major Linker Version: " << static_cast<int>(ntHeaders->OptionalHeader.MajorLinkerVersion) << std::endl;
    std::cout << "Minor Linker Version: " << static_cast<int>(ntHeaders->OptionalHeader.MinorLinkerVersion) << std::endl;
    std::cout << "Size of Code: " << ntHeaders->OptionalHeader.SizeOfCode << std::endl;
    std::cout << "Size of Initialized Data: " << ntHeaders->OptionalHeader.SizeOfInitializedData << std::endl;
    std::cout << "Size of Uninitialized Data: " << ntHeaders->OptionalHeader.SizeOfUninitializedData << std::endl;
    std::cout << "Address of Entry Point: 0x" << std::hex << ntHeaders->OptionalHeader.AddressOfEntryPoint << std::endl;
    std::cout << "Base of Code: 0x" << std::hex << ntHeaders->OptionalHeader.BaseOfCode << std::endl;
    std::cout << "Image Base: 0x" << std::hex << ntHeaders->OptionalHeader.ImageBase << std::endl;
    std::cout << "Section Alignment: " << ntHeaders->OptionalHeader.SectionAlignment << std::endl;
    std::cout << "File Alignment: " << ntHeaders->OptionalHeader.FileAlignment << std::endl;
    std::cout << "Major Operating System Version: " << ntHeaders->OptionalHeader.MajorOperatingSystemVersion << std::endl;
    std::cout << "Minor Operating System Version: " << ntHeaders->OptionalHeader.MinorOperatingSystemVersion << std::endl;
    std::cout << "Major Image Version: " << ntHeaders->OptionalHeader.MajorImageVersion << std::endl;
    std::cout << "Minor Image Version: " << ntHeaders->OptionalHeader.MinorImageVersion << std::endl;
    std::cout << "Major Subsystem Version: " << ntHeaders->OptionalHeader.MajorSubsystemVersion << std::endl;
    std::cout << "Minor Subsystem Version: " << ntHeaders->OptionalHeader.MinorSubsystemVersion << std::endl;
    std::cout << "Win32 Version Value: " << ntHeaders->OptionalHeader.Win32VersionValue << std::endl;
    std::cout << "Size of Image: " << ntHeaders->OptionalHeader.SizeOfImage << std::endl;
    std::cout << "Size of Headers: " << ntHeaders->OptionalHeader.SizeOfHeaders << std::endl;
    std::cout << "CheckSum: 0x" << std::hex << ntHeaders->OptionalHeader.CheckSum << std::endl;
    std::cout << "Subsystem: 0x" << std::hex << ntHeaders->OptionalHeader.Subsystem << std::endl;
}

/**
 * @brief Extract the function names from a PE file
 * @param fileData The file data
 * @return The function names
 */
std::vector<std::string> PE::getFunctionNames(const std::vector<uint8_t>& fileData)
{
    std::vector<std::string> functionNames;

	if (fileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
	{
		std::cerr << "Error: File is too small to contain a valid PE header.\n";
		return functionNames;
	}

	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<uint8_t*>(&fileData[0]));
	DWORD peHeaderOffset = dosHeader->e_lfanew;

	PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<uint8_t*>(&fileData[peHeaderOffset]));

	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		std::cerr << "Error: Invalid PE header signature.\n";
		return functionNames;
	}

	DWORD exportTableRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	DWORD exportTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

	if (exportTableRVA == 0 || exportTableSize == 0)
	{
		std::cout << "No export table found.\n";
		return functionNames;
	}

	DWORD exportTableOffset = exportTableRVA;
	for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
	{
		PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
		if (exportTableRVA >= sectionHeader->VirtualAddress && exportTableRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
		{
			exportTableOffset = exportTableRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
			break;
		}
	}

	PIMAGE_EXPORT_DIRECTORY exportDirectory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(const_cast<uint8_t*>(&fileData[exportTableOffset]));

	DWORD* exportedFunctions = reinterpret_cast<DWORD*>(const_cast<uint8_t*>(&fileData[exportDirectory->AddressOfFunctions]));
	DWORD numberOfFunctions = exportDirectory->NumberOfFunctions;

    for (DWORD i = 0; i < numberOfFunctions; ++i)
	{
		DWORD functionRVA = exportedFunctions[i];
		if (functionRVA == 0)
			continue;

		DWORD functionOffset = functionRVA;
		for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
		{
			PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
			if (functionRVA >= sectionHeader->VirtualAddress && functionRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
			{
				functionOffset = functionRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
				break;
			}
		}

		const char* functionName = reinterpret_cast<const char*>(&fileData[functionOffset]);
		functionNames.push_back(functionName);
	}

    return functionNames;
}

/**
 * @brief Extract the function addresses from a PE file
 * @param fileData The file data
 * @return The function addresses
 */
std::vector<uint64_t> PE::getFunctionAddresses(const std::vector<uint8_t>& fileData)
{
    std::vector<uint64_t> functionAddresses;

	if (fileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
	{
		std::cerr << "Error: File is too small to contain a valid PE header.\n";
		return functionAddresses;
	}

	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<uint8_t*>(&fileData[0]));
	DWORD peHeaderOffset = dosHeader->e_lfanew;

	PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<uint8_t*>(&fileData[peHeaderOffset]));

	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		std::cerr << "Error: Invalid PE header signature.\n";
		return functionAddresses;
	}

	DWORD exportTableRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	DWORD exportTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

	if (exportTableRVA == 0 || exportTableSize == 0)
	{
		std::cout << "No export table found.\n";
		return functionAddresses;
	}

	DWORD exportTableOffset = exportTableRVA;
	for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
	{
		PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
		if (exportTableRVA >= sectionHeader->VirtualAddress && exportTableRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
		{
			exportTableOffset = exportTableRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
			break;
		}
	}

	PIMAGE_EXPORT_DIRECTORY exportDirectory = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(const_cast<uint8_t*>(&fileData[exportTableOffset]));

	DWORD* exportedFunctions = reinterpret_cast<DWORD*>(const_cast<uint8_t*>(&fileData[exportDirectory->AddressOfFunctions]));
	DWORD numberOfFunctions = exportDirectory->NumberOfFunctions;
    
    for (DWORD i = 0; i < numberOfFunctions; ++i)
	{
		DWORD functionRVA = exportedFunctions[i];
		if (functionRVA == 0)
			continue;

		functionAddresses.push_back(functionRVA);
	}

	return functionAddresses;
}

/**
 * @brief Extract the DLL names from a PE file
 * @param fileData The file data
 * @return The DLL names
 */
std::vector<std::string> PE::getDllNames(const std::vector<uint8_t>& fileData)
{
    std::vector<std::string> dllNames;

	if (fileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
	{
		std::cerr << "Error: File is too small to contain a valid PE header.\n";
		return dllNames;
	}

	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<uint8_t*>(&fileData[0]));
	DWORD peHeaderOffset = dosHeader->e_lfanew;

	PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<uint8_t*>(&fileData[peHeaderOffset]));

	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		std::cerr << "Error: Invalid PE header signature.\n";
		return dllNames;
	}

	DWORD importTableRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD importTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

	if (importTableRVA == 0 || importTableSize == 0)
	{
		std::cout << "No import table found.\n";
		return dllNames;
	}

	DWORD importTableOffset = importTableRVA;
	for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
	{
		PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
		if (importTableRVA >= sectionHeader->VirtualAddress && importTableRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
		{
			importTableOffset = importTableRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
			break;
		}
	}

	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(const_cast<uint8_t*>(&fileData[importTableOffset]));

	while (importDescriptor->OriginalFirstThunk != 0)
	{
		const char* dllName = reinterpret_cast<const char*>(&fileData[importDescriptor->Name]);
		dllNames.push_back(dllName);
		++importDescriptor;
	}

	return dllNames;
}

/**
 * @brief Extract the DLL addresses from a PE file
 * @param fileData The file data
 * @return The DLL addresses
 */
std::vector<uint64_t> PE::getDllAddresses(const std::vector<uint8_t>& fileData)
{
	std::vector<uint64_t> dllAddresses;

	if (fileData.size() < sizeof(IMAGE_DOS_HEADER) + sizeof(DWORD) + sizeof(IMAGE_NT_HEADERS))
	{
		std::cerr << "Error: File is too small to contain a valid PE header.\n";
		return dllAddresses;
	}

	PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(const_cast<uint8_t*>(&fileData[0]));
	DWORD peHeaderOffset = dosHeader->e_lfanew;

	PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(const_cast<uint8_t*>(&fileData[peHeaderOffset]));

	if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
	{
		std::cerr << "Error: Invalid PE header signature.\n";
		return dllAddresses;
	}

	DWORD importTableRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	DWORD importTableSize = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

	if (importTableRVA == 0 || importTableSize == 0)
	{
		std::cout << "No import table found.\n";
		return dllAddresses;
	}

	DWORD importTableOffset = importTableRVA;
	for (int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i)
	{
		PIMAGE_SECTION_HEADER sectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>(const_cast<uint8_t*>(&fileData[peHeaderOffset + sizeof(IMAGE_NT_HEADERS) + (i * sizeof(IMAGE_SECTION_HEADER))]));
		if (importTableRVA >= sectionHeader->VirtualAddress && importTableRVA < sectionHeader->VirtualAddress + sectionHeader->Misc.VirtualSize)
		{
			importTableOffset = importTableRVA - sectionHeader->VirtualAddress + sectionHeader->PointerToRawData;
			break;
		}
	}

	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(const_cast<uint8_t*>(&fileData[importTableOffset]));

	while (importDescriptor->OriginalFirstThunk != 0)
	{
		dllAddresses.push_back(importDescriptor->OriginalFirstThunk);
		++importDescriptor;
	}

	return dllAddresses;
}
#endif

#if defined(__linux__)
/**
 * @brief Extract the import table from an ELF file
 * @param fileData The file data
 */
void PE::extractImportTable(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(Elf64_Ehdr))
    {
        std::cerr << "Error: File is too small to contain a valid ELF header.\n";
        return;
    }

    const Elf64_Ehdr* elfHeader = reinterpret_cast<const Elf64_Ehdr*>(&fileData[0]);

    if (memcmp(elfHeader->e_ident, ELFMAG, SELFMAG) != 0)
    {
        std::cerr << "Error: Invalid ELF header.\n";
        return;
    }

    off_t sectionHeaderOffset = elfHeader->e_shoff;

    const Elf64_Shdr* sectionHeader = reinterpret_cast<const Elf64_Shdr*>(&fileData[sectionHeaderOffset]);
    for (int i = 0; i < elfHeader->e_shnum; ++i)
    {
        if (sectionHeader->sh_type == SHT_DYNSYM)
        {
            std::cout << "Imported Functions:\n";
            const Elf64_Sym* symEntry = reinterpret_cast<const Elf64_Sym*>(&fileData[sectionHeader->sh_offset]);
            for (size_t j = 0; j < sectionHeader->sh_size / sizeof(Elf64_Sym); ++j)
            {
                if (ELF64_ST_TYPE(symEntry[j].st_info) == STT_FUNC)
                {
                    std::cout << &fileData[sectionHeader->sh_offset + symEntry[j].st_name] << std::endl;
                }
            }
            return;
        }
        sectionHeader = reinterpret_cast<const Elf64_Shdr*>(&fileData[sectionHeaderOffset + elfHeader->e_shentsize * (i + 1)]);
    }
    std::cout << "No import table found.\n";
}

/**
 * @brief Extract the export table from an ELF file
 * @param fileData The file data
 */
void PE::extractExportTable(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(Elf64_Ehdr))
    {
        std::cerr << "Error: File is too small to contain a valid ELF header.\n";
        return;
    }

    const Elf64_Ehdr* elfHeader = reinterpret_cast<const Elf64_Ehdr*>(&fileData[0]);

    if (memcmp(elfHeader->e_ident, ELFMAG, SELFMAG) != 0)
    {
        std::cerr << "Error: Invalid ELF header.\n";
        return;
    }

    off_t sectionHeaderOffset = elfHeader->e_shoff;

    const Elf64_Shdr* sectionHeader = reinterpret_cast<const Elf64_Shdr*>(&fileData[sectionHeaderOffset]);
    for (int i = 0; i < elfHeader->e_shnum; ++i)
    {
        if (sectionHeader->sh_type == SHT_DYNSYM)
        {
            std::cout << "Exported Functions:\n";
            const Elf64_Sym* symEntry = reinterpret_cast<const Elf64_Sym*>(&fileData[sectionHeader->sh_offset]);
            for (size_t j = 0; j < sectionHeader->sh_size / sizeof(Elf64_Sym); ++j)
            {
                if (ELF64_ST_TYPE(symEntry[j].st_info) == STT_FUNC)
                {
                    std::cout << &fileData[sectionHeader->sh_offset + symEntry[j].st_name] << std::endl;
                }
            }
            return;
        }
        sectionHeader = reinterpret_cast<const Elf64_Shdr*>(&fileData[sectionHeaderOffset + elfHeader->e_shentsize * (i + 1)]);
    }
    std::cout << "No export table found.\n";
}

/**
 * @brief Extract the resources from an ELF file
 * @param fileData The file data
 */
void PE::extractResources(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(Elf64_Ehdr))
    {
        std::cerr << "Error: File is too small to contain a valid ELF header.\n";
        return;
    }

    const Elf64_Ehdr* elfHeader = reinterpret_cast<const Elf64_Ehdr*>(&fileData[0]);

    if (memcmp(elfHeader->e_ident, ELFMAG, SELFMAG) != 0)
    {
        std::cerr << "Error: Invalid ELF header.\n";
        return;
    }

    off_t sectionHeaderOffset = elfHeader->e_shoff;

    const Elf64_Shdr* sectionHeader = reinterpret_cast<const Elf64_Shdr*>(&fileData[sectionHeaderOffset]);
    for (int i = 0; i < elfHeader->e_shnum; ++i)
    {
        if (sectionHeader->sh_type == SHT_PROGBITS &&
            sectionHeader->sh_flags & SHF_ALLOC &&
            sectionHeader->sh_size > 0 &&
            sectionHeader->sh_addr == elfHeader->e_entry)
        {
            std::cout << "Resource directory found.\n";
            return;
        }
        sectionHeader = reinterpret_cast<const Elf64_Shdr*>(&fileData[sectionHeaderOffset + elfHeader->e_shentsize * (i + 1)]);
    }

    ResourceDirectory resourceDir;
    ResourceDirectoryTraverser* traverser = ResourceDirectoryTraverserFactory::createTraverser();
    LinuxResourceDirectoryTraverser resourceTraverser;
    resourceTraverser.traverse(fileData, &resourceDir, 0, "");
}

/**
 * @brief Extract the section info from an ELF file
 * @param fileData The file data
 */
void PE::extractSectionInfo(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(Elf64_Ehdr))
    {
        std::cerr << "Error: File is too small to contain a valid ELF header.\n";
        return;
    }

    const Elf64_Ehdr* elfHeader = reinterpret_cast<const Elf64_Ehdr*>(&fileData[0]);

    if (memcmp(elfHeader->e_ident, ELFMAG, SELFMAG) != 0)
    {
        std::cerr << "Error: Invalid ELF header.\n";
        return;
    }

    off_t sectionHeaderOffset = elfHeader->e_shoff;

    std::cout << "Section Info:\n";

    for (int i = 0; i < elfHeader->e_shnum; ++i)
    {
        const Elf64_Shdr* sectionHeader = reinterpret_cast<const Elf64_Shdr*>(&fileData[sectionHeaderOffset + elfHeader->e_shentsize * i]);

        std::cout << "Section Name: " << &fileData[sectionHeader->sh_name] << std::endl;
        std::cout << "Virtual Size: " << sectionHeader->sh_size << std::endl;
        std::cout << "Virtual Address: 0x" << std::hex << sectionHeader->sh_addr << std::endl;
        std::cout << "Size of Raw Data: " << sectionHeader->sh_size << std::endl;
        std::cout << "Pointer to Raw Data: 0x" << std::hex << sectionHeader->sh_offset << std::endl;
        std::cout << "Characteristics: 0x" << std::hex << sectionHeader->sh_flags << std::endl;
        std::cout << std::endl;
    }
}

/**
 * @brief Extract the headers from an ELF file
 * @param fileData The file data
 */
void PE::parseHeaders(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(Elf64_Ehdr))
    {
        std::cerr << "Error: File is too small to contain a valid ELF header.\n";
        return;
    }

    const Elf64_Ehdr* elfHeader = reinterpret_cast<const Elf64_Ehdr*>(&fileData[0]);

    if (memcmp(elfHeader->e_ident, ELFMAG, SELFMAG) != 0)
    {
        std::cerr << "Error: Invalid ELF header.\n";
        return;
    }

    std::cout << "PE Header Info:\n";
    std::cout << "Signature: 0x" << std::hex << elfHeader->e_ident[EI_MAG0] << elfHeader->e_ident[EI_MAG1] << elfHeader->e_ident[EI_MAG2] << elfHeader->e_ident[EI_MAG3] << std::endl;
    std::cout << "Machine: 0x" << std::hex << elfHeader->e_machine << std::endl;
    std::cout << "Number of Sections: " << elfHeader->e_shnum << std::endl;
    std::cout << "Size of Optional Header: " << elfHeader->e_ehsize << std::endl;
    std::cout << std::endl;
}
#endif

#if defined(__APPLE__)
/**
 * @brief Extract the import table from a Mach-O file
 * @param fileData The file data
 */
void PE::extractImportTable(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(mach_header_64))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return;
    }

    const auto* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const auto* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    if (machHeader->magic != MH_MAGIC_64 || machHeader->ncmds == 0)
    {
        std::cerr << "Error: Invalid Mach-O header.\n";
        return;
    }

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
	{
        if (loadCmd->cmd == LC_SYMTAB)
		{
            const auto* symtab = reinterpret_cast<const symtab_command*>(loadCmd);
            const auto* symbols = reinterpret_cast<const nlist_64*>(&fileData[symtab->symoff]);
            const char* strings = reinterpret_cast<const char*>(&fileData[symtab->stroff]);

            std::cout << "Imported Functions:\n";
            for (uint32_t j = 0; j < symtab->nsyms; ++j)
			{
                if (symbols[j].n_type & N_EXT)
				{
                    std::cout << &strings[symbols[j].n_un.n_strx] << std::endl;
                }
            }
            return;
        }
        loadCmd = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(loadCmd) + loadCmd->cmdsize);
    }
    std::cout << "No import table found.\n";
}

/**
 * @brief Extract the export table from a Mach-O file
 * @param fileData The file data
 */
void PE::extractExportTable(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(mach_header_64))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return;
    }

    const auto* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const auto* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
	{
        if (loadCmd->cmd == LC_SYMTAB)
		{
            const auto* symtab = reinterpret_cast<const symtab_command*>(loadCmd);
            const auto* symbols = reinterpret_cast<const nlist_64*>(&fileData[symtab->symoff]);
            const char* strings = reinterpret_cast<const char*>(&fileData[symtab->stroff]);

            std::cout << "Exported Functions:\n";
            for (uint32_t j = 0; j < symtab->nsyms; ++j)
			{
                if (symbols[j].n_type & N_EXT)
				{
                    std::cout << &strings[symbols[j].n_un.n_strx] << std::endl;
                }
            }
            return;
        }
        loadCmd = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(loadCmd) + loadCmd->cmdsize);
    }
    std::cout << "No export table found.\n";
}

/**
 * @brief Extract the resources from a Mach-O file
 * @param fileData The file data
 */
void PE::extractResources(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(mach_header_64))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return;
    }

    const auto* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const auto* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
	{
        if (loadCmd->cmd == LC_SEGMENT_64)
		{
            const auto* segmentCmd = reinterpret_cast<const segment_command_64*>(loadCmd);
            const auto* sections = reinterpret_cast<const section_64*>(segmentCmd + 1);

            for (uint32_t j = 0; j < segmentCmd->nsects; ++j)
			{
                const section_64* section = &sections[j];
                if (strcmp(section->segname, "__TEXT") == 0 && strcmp(section->sectname, "__text") == 0)
				{
                    std::cout << "Resource directory found.\n";
                    return;
                }
            }
        }
        loadCmd = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(loadCmd) + loadCmd->cmdsize);
    }
    std::cout << "No resource directory found.\n";
}

/**
 * @brief Extract the section info from a Mach-O file
 * @param fileData The file data
 */
void PE::extractSectionInfo(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(mach_header_64))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return;
    }

    const auto* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const auto* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    std::cout << "Section Info:\n";
    for (uint32_t i = 0; i < machHeader->ncmds; ++i)
	{
        if (loadCmd->cmd == LC_SEGMENT_64)
		{
            const auto* segmentCmd = reinterpret_cast<const segment_command_64*>(loadCmd);
            const auto* sections = reinterpret_cast<const section_64*>(segmentCmd + 1);

            for (uint32_t j = 0; j < segmentCmd->nsects; ++j)
			{
                const section_64* section = &sections[j];
                std::cout << "Section Name: " << section->sectname << std::endl;
                std::cout << "Virtual Size: " << section->size << std::endl;
                std::cout << "Virtual Address: 0x" << std::hex << section->addr << std::endl;
                std::cout << "Size of Raw Data: " << section->size << std::endl;
                std::cout << "Pointer to Raw Data: 0x" << std::hex << section->offset << std::endl;
                std::cout << "Characteristics: 0x" << std::hex << section->flags << std::endl;
                std::cout << std::endl;
            }
        }
        loadCmd = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(loadCmd) + loadCmd->cmdsize);
    }
}

/**
 * @brief Extract the headers from a Mach-O file
 * @param fileData The file data
 */
void PE::parseHeaders(const std::vector<uint8_t>& fileData)
{
    if (fileData.size() < sizeof(mach_header_64))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return;
    }

    const auto* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);

    std::cout << "PE Header Info:\n";
    std::cout << "Signature: 0x" << std::hex << machHeader->magic << std::endl;
    std::cout << "CPU Type: " << std::hex << machHeader->cputype << std::endl;
    std::cout << "Number of Commands: " << machHeader->ncmds << std::endl;
    std::cout << "Size of Header: " << machHeader->sizeofcmds << std::endl;
    std::cout << std::endl;
}

/**
 * @brief Extract the function names from a Mach-O file
 * @param fileData The file data
 * @return The function names
 */
std::vector<std::string> PE::getFunctionNames(const std::vector<uint8_t>& fileData)
{
    std::vector<std::string> functionNames;
    if (fileData.size() < sizeof(mach_header_64))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return functionNames;
    }

    const auto* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const auto* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    if (machHeader->magic != MH_MAGIC_64 || machHeader->ncmds == 0)
    {
        std::cerr << "Error: Invalid Mach-O header.\n";
        return functionNames;
    }

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
    {
        if (loadCmd->cmd == LC_SYMTAB)
        {
            const auto* symtab = reinterpret_cast<const symtab_command*>(loadCmd);
            const auto* symbols = reinterpret_cast<const nlist_64*>(&fileData[symtab->symoff]);
            const char* strings = reinterpret_cast<const char*>(&fileData[symtab->stroff]);

            for (uint32_t j = 0; j < symtab->nsyms; ++j)
            {
                if (symbols[j].n_type & N_EXT)
                {
                    functionNames.emplace_back(&strings[symbols[j].n_un.n_strx]);
                }
            }
            return functionNames;
        }
        loadCmd = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(loadCmd) + loadCmd->cmdsize);
    }
    return functionNames;
}

/**
 * @brief Extract the DLL names from a Mach-O file
 * @param fileData The file data
 * @return The DLL names
 */
std::vector<std::string> PE::getDllNames(const std::vector<uint8_t>& fileData)
{
    std::vector<std::string> dllNames;
    if (fileData.size() < sizeof(mach_header_64))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return dllNames;
    }

    const auto* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const auto* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    if (machHeader->magic != MH_MAGIC_64 || machHeader->ncmds == 0)
    {
        std::cerr << "Error: Invalid Mach-O header.\n";
        return dllNames;
    }

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
    {
        if (loadCmd->cmd == LC_LOAD_DYLIB)
        {
            const auto* dylibCmd = reinterpret_cast<const dylib_command*>(loadCmd);
            const char* dylibName = reinterpret_cast<const char*>(&fileData[dylibCmd->dylib.name.offset]);
            dllNames.emplace_back(dylibName);
        }
        loadCmd = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(loadCmd) + loadCmd->cmdsize);
    }
    return dllNames;
}

/**
 * @brief Extract the function addresses from a Mach-O file
 * @param fileData The file data
 * @return The function addresses
 */
std::vector<uint64_t> PE::getFunctionAddresses(const std::vector<uint8_t>& fileData)
{
    std::vector<uint64_t> functionAddresses;
    if (fileData.size() < sizeof(mach_header_64))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return functionAddresses;
    }

    const auto* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const auto* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    if (machHeader->magic != MH_MAGIC_64 || machHeader->ncmds == 0)
    {
        std::cerr << "Error: Invalid Mach-O header.\n";
        return functionAddresses;
    }

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
    {
        if (loadCmd->cmd == LC_SYMTAB)
        {
            const auto* symtab = reinterpret_cast<const symtab_command*>(loadCmd);
            const auto* symbols = reinterpret_cast<const nlist_64*>(&fileData[symtab->symoff]);
            const char* strings = reinterpret_cast<const char*>(&fileData[symtab->stroff]);

            for (uint32_t j = 0; j < symtab->nsyms; ++j)
            {
                if (symbols[j].n_type & N_EXT)
                {
                    functionAddresses.push_back(symbols[j].n_value);
                }
            }
            return functionAddresses;
        }
        loadCmd = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(loadCmd) + loadCmd->cmdsize);
    }
    return functionAddresses;
}

/**
 * @brief Extract the DLL addresses from a Mach-O file
 * @param fileData The file data
 * @return The DLL addresses
 */
std::vector<uint64_t> PE::getDllAddresses(const std::vector<uint8_t>& fileData)
{
    std::vector<uint64_t> dllAddresses;
    if (fileData.size() < sizeof(mach_header_64))
    {
        std::cerr << "Error: File is too small to contain a valid Mach-O header.\n";
        return dllAddresses;
    }

    const auto* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const auto* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    if (machHeader->magic != MH_MAGIC_64 || machHeader->ncmds == 0)
    {
        std::cerr << "Error: Invalid Mach-O header.\n";
        return dllAddresses;
    }

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
    {
        if (loadCmd->cmd == LC_LOAD_DYLIB)
        {
            const auto* dylibCmd = reinterpret_cast<const dylib_command*>(loadCmd);
            dllAddresses.push_back(dylibCmd->dylib.timestamp);
        }
        loadCmd = reinterpret_cast<const load_command*>(reinterpret_cast<const char*>(loadCmd) + loadCmd->cmdsize);
    }
    return dllAddresses;
}
#endif