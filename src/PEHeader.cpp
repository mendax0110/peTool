#include "./include/PEHeader.h"

#if defined(_WIN32)
#include <windows.h>
#include <winnt.h>
#endif

#if defined(__LINUX)
#include <elf.h>
#endif

#if defined(__APPLE__)
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#endif

#include <iostream>
#include <string>
#include <cstdint>

using namespace PeHeaderInternals;


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

void ResourceDirectory::traverse(const std::vector<uint8_t>& fileData, int level, const std::string& parentName)
{
    if (traverser != nullptr)
    {
        traverser->traverse(fileData, this, level, parentName);
    }
}

void ResourceDirectory::setTraverser(ResourceDirectoryTraverser* traverser)
{
    this->traverser = traverser;
}

uint32_t PEHeader::GetResourceDirectoryOffset(const std::vector<uint8_t>& fileData)
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
}

#if defined(_WIN32)
void WinResourceDirectoryTraverser::traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName)
{
    uint32_t RESOURCE_DIRECTORY_OFFSET = PEHeader::GetResourceDirectoryOffset(fileData);
    std::vector<uint8_t> mutableFileData(fileData.begin(), fileData.end());
    const PIMAGE_RESOURCE_DIRECTORY resourceDirectoryData = reinterpret_cast<const PIMAGE_RESOURCE_DIRECTORY>(&mutableFileData[RESOURCE_DIRECTORY_OFFSET]);
    
    const PIMAGE_RESOURCE_DIRECTORY_ENTRY resourceEntries = reinterpret_cast<const PIMAGE_RESOURCE_DIRECTORY_ENTRY>(resourceDirectoryData + 1);
    for (DWORD i = 0; i < resourceDirectoryData->NumberOfNamedEntries + resourceDirectoryData->NumberOfIdEntries; ++i)
    {
        const PIMAGE_RESOURCE_DIRECTORY_ENTRY entry = &resourceEntries[i];
        if (entry->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
        {
            const PIMAGE_RESOURCE_DIRECTORY subDirectory = reinterpret_cast<const PIMAGE_RESOURCE_DIRECTORY>(&mutableFileData[entry->OffsetToDirectory & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY]);
            WinResourceDirectoryTraverser traverser;
            resourceDirectory->traverse(mutableFileData, level + 1, parentName + std::to_string(i) + "/");
            std::cout << "Level " << level << ": " << parentName << "Directory\n";
            std::cout << "OffsetToData: 0x" << std::hex << entry->OffsetToData << "\n";
            std::cout << "OffsetToDirectory: 0x" << std::hex << entry->OffsetToDirectory << "\n";
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
void LinuxResourceDirectoryTraverser::traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName)
{

}
#endif

#if defined(__APPLE__)
void AppleResourceDirectoryTraverser::traverse(const std::vector<uint8_t>& fileData, ResourceDirectory* resourceDirectory, int level, const std::string& parentName)
{

}
#endif


#if defined(_WIN32)
void PEHeader::extractImportTable(const std::vector<uint8_t>& fileData)
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
			PIMAGE_IMPORT_BY_NAME importByName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(const_cast<uint8_t*>(&fileData[thunk->u1.AddressOfData]));
			std::cout << importByName->Name << std::endl;
			++thunk;
		}
		++importDescriptor;
	}
}

void PEHeader::extractExportTable(const std::vector<uint8_t>& fileData)
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

void PEHeader::extractResources(const std::vector<uint8_t>& fileData)
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

void PEHeader::extractSectionInfo(const std::vector<uint8_t>& fileData)
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

void PEHeader::parseHeaders(const std::vector<uint8_t>& fileData)
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
#endif

#if defined(__LINUX)
void PEHeader::extractImportTable(const std::vector<uint8_t>& fileData)
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

void PEHeader::extractExportTable(const std::vector<uint8_t>& fileData)
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

void PEHeader::extractResources(const std::vector<uint8_t>& fileData)
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

	traverseResourceDirectory(fileData, elfHeader->e_shoff, 0, "");
    std::cout << "No resource directory found.\n";
}

void PEHeader::traverseResourceDirectory(const std::vector<uint8_t>& fileData, uintptr_t resourceDirectoryAddress, int level, const std::string& parentName)
{
    size_t resourceEntryOffset = sizeof(Elf64_Word) * (level + 1);
    uintptr_t resourceEntryAddress = resourceDirectoryAddress + resourceEntryOffset;

    Elf64_Word nameOffset;
    Elf64_Word offsetToData;

    memcpy(&nameOffset, &fileData[resourceEntryAddress], sizeof(Elf64_Word));
    memcpy(&offsetToData, &fileData[resourceEntryAddress + sizeof(Elf64_Word)], sizeof(Elf64_Word));

    bool isDirectory = (offsetToData & ELF64_ST_BIND(STT_SECTION));

    if (isDirectory)
	{
        std::cout << "Level " << level << ": " << parentName << "Directory (OffsetToData: " << std::hex << offsetToData << ")\n";
        traverseResourceDirectory(fileData, offsetToData, level + 1, parentName);
    } 
	else
	{
        std::cout << "Level " << level << ": " << parentName << "Data (OffsetToData: " << std::hex << offsetToData << ", Size: " << sizeof(Elf64_Word) << ")\n";
    }

    traverseResourceDirectory(fileData, resourceEntryAddress + 2 * sizeof(Elf64_Word), level, parentName);
}

void PEHeader::extractSectionInfo(const std::vector<uint8_t>& fileData)
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

void PEHeader::parseHeaders(const std::vector<uint8_t>& fileData)
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
void PEHeader::extractImportTable(const std::vector<uint8_t>& fileData)
{
    const mach_header_64* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const load_command* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
	{
        if (loadCmd->cmd == LC_SYMTAB)
		{
            const symtab_command* symtab = reinterpret_cast<const symtab_command*>(loadCmd);
            const nlist_64* symbols = reinterpret_cast<const nlist_64*>(&fileData[symtab->symoff]);
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

void PEHeader::extractExportTable(const std::vector<uint8_t>& fileData)
{
    const mach_header_64* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const load_command* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
	{
        if (loadCmd->cmd == LC_SYMTAB)
		{
            const symtab_command* symtab = reinterpret_cast<const symtab_command*>(loadCmd);
            const nlist_64* symbols = reinterpret_cast<const nlist_64*>(&fileData[symtab->symoff]);
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

void PEHeader::extractResources(const std::vector<uint8_t>& fileData)
{
    const mach_header_64* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const load_command* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    uint32_t ncmds = machHeader->ncmds;
    for (uint32_t i = 0; i < ncmds; ++i)
	{
        if (loadCmd->cmd == LC_SEGMENT_64)
		{
            const segment_command_64* segmentCmd = reinterpret_cast<const segment_command_64*>(loadCmd);
            const section_64* sections = reinterpret_cast<const section_64*>(segmentCmd + 1);

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

void PEHeader::extractSectionInfo(const std::vector<uint8_t>& fileData)
{
    const mach_header_64* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);
    const load_command* loadCmd = reinterpret_cast<const load_command*>(&fileData[sizeof(mach_header_64)]);

    std::cout << "Section Info:\n";
    for (uint32_t i = 0; i < machHeader->ncmds; ++i)
	{
        if (loadCmd->cmd == LC_SEGMENT_64)
		{
            const segment_command_64* segmentCmd = reinterpret_cast<const segment_command_64*>(loadCmd);
            const section_64* sections = reinterpret_cast<const section_64*>(segmentCmd + 1);

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

void PEHeader::parseHeaders(const std::vector<uint8_t>& fileData)
{
    const mach_header_64* machHeader = reinterpret_cast<const mach_header_64*>(&fileData[0]);

    std::cout << "PE Header Info:\n";
    std::cout << "Signature: 0x" << std::hex << machHeader->magic << std::endl;
    std::cout << "CPU Type: " << std::hex << machHeader->cputype << std::endl;
    std::cout << "Number of Commands: " << machHeader->ncmds << std::endl;
    std::cout << "Size of Header: " << machHeader->sizeofcmds << std::endl;
    std::cout << std::endl;
}
#endif