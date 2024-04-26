#include "PEHeader.h"
#include <iostream>
#include <windows.h>
#include <winnt.h>
#include <string>
#include <cstdint>

using namespace PeHeaderInternals;

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

	traverseResourceDirectory(fileData, resourceDirectory, 0, "");
}

void PEHeader::traverseResourceDirectory(const std::vector<uint8_t>& fileData, PIMAGE_RESOURCE_DIRECTORY resourceDirectory, int level, const std::string& parentName)
{
	size_t resourceEntryOffset = sizeof(IMAGE_RESOURCE_DIRECTORY) + level * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
	DWORD numberOfEntries = resourceDirectory->NumberOfIdEntries + resourceDirectory->NumberOfNamedEntries;
	uintptr_t resourceDirectoryAddress = reinterpret_cast<uintptr_t>(resourceDirectory);
	PIMAGE_RESOURCE_DIRECTORY_ENTRY resourceEntry = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY_ENTRY>(const_cast<uint8_t*>(&fileData[resourceDirectoryAddress + resourceEntryOffset]));

	for (DWORD i = 0; i < numberOfEntries; ++i)
	{
		if (resourceEntry->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY)
		{
			PIMAGE_RESOURCE_DIRECTORY nextDirectory = reinterpret_cast<PIMAGE_RESOURCE_DIRECTORY>(const_cast<uint8_t*>(&fileData[resourceEntry->OffsetToDirectory & ~IMAGE_RESOURCE_DATA_IS_DIRECTORY]));
			if (resourceEntry->NameIsString)
			{
				PIMAGE_RESOURCE_DIR_STRING_U resourceName = reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(const_cast<uint8_t*>(&fileData[resourceEntry->NameOffset]));
				std::wstring name(resourceName->Length, L'\0');
				std::copy(resourceName->NameString, resourceName->NameString + resourceName->Length, name.begin());
				traverseResourceDirectory(fileData, nextDirectory, level + 1, parentName + std::string(name.begin(), name.end()) + "/");
			}
			else
			{
				std::cout << "Level " << level << ": " << parentName << "ID: " << resourceEntry->Name << "\n";
				traverseResourceDirectory(fileData, nextDirectory, level + 1, parentName + std::to_string(resourceEntry->Name) + "/");
			}
		}
		else
		{
			PIMAGE_RESOURCE_DATA_ENTRY resourceDataEntry = reinterpret_cast<PIMAGE_RESOURCE_DATA_ENTRY>(const_cast<uint8_t*>(&fileData[resourceEntry->OffsetToData]));
			std::cout << "Level " << level << ": " << parentName;
			if (resourceEntry->NameIsString)
			{
				PIMAGE_RESOURCE_DIR_STRING_U resourceName = reinterpret_cast<PIMAGE_RESOURCE_DIR_STRING_U>(const_cast<uint8_t*>(&fileData[resourceEntry->NameOffset]));
				std::wstring name(resourceName->Length, L'\0');
				std::copy(resourceName->NameString, resourceName->NameString + resourceName->Length, name.begin());
				std::wcout << std::wstring(name.begin(), name.end());
			}
			else
			{
				std::cout << resourceEntry->Name;
			}
			std::cout << " (Data RVA: 0x" << std::hex << resourceDataEntry->OffsetToData << ", Size: " << std::dec << resourceDataEntry->Size << ")\n";
		}
		++resourceEntry;
	}
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