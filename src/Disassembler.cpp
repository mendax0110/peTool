#include "./include/Disassembler.h"
#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>

using namespace DissassemblerInternals;

Disassembler::Disassembler()
{
}

Disassembler::~Disassembler()
{
}

void Disassembler::disassemble32bit(const std::vector<uint8_t>& data, size_t offset, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        std::cout << "x86 Instruction: ";
        for (size_t j = 0; j < 4; ++j)
        {
            size_t index = offset + i + j;
            if (index < data.size())
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[index]) << " ";
        }
        std::cout << std::endl;
        i += 3;
    }
}

void Disassembler::disassemble64bit(const std::vector<uint8_t>& data, size_t offset, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        std::cout << "x86 Instruction: ";
        for (size_t j = 0; j < 8; ++j)
        {
            size_t index = offset + i + j;
            if (index < data.size())
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[index]) << " ";
        }
        std::cout << std::endl;
        i += 7;
    }
}

void Disassembler::dissassembleArm(const std::vector<uint8_t>& data, size_t offset, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        std::cout << "ARM Instruction: ";
        for (size_t j = 0; j < 4; ++j)
        {
            size_t index = offset + i + j;
            if (index < data.size())
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[index]) << " ";
        }
        std::cout << std::endl;
        i += 3;
    }
}

void Disassembler::printDisassembly(const std::vector<uint8_t>& data, size_t offset, size_t size)
{
    std::cout << "Name\tOffset\tSize\tDisassembly" << std::endl;
    std::cout << "----\t------\t----\t-----------" << std::endl;
    for (size_t i = 0; i < size; ++i)
    {
        std::cout << "Inst\t" << std::hex << std::setw(4) << std::setfill('0') << (offset + i) << "\t1\t";
        if (i + 4 <= size)
        {
            disassemble32bit(data, offset + i, 4);
            i += 3;
        }
        else if (i + 8 <= size)
        {
            disassemble64bit(data, offset + i, 8);
            i += 7;
        }
        else
        {
            dissassembleArm(data, offset + i, size - i);
            break;
        }
    }
}

std::tuple<std::vector<uint8_t>, size_t, size_t> Disassembler::getExecutable(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error: Failed to open file " << path << std::endl;
        return std::make_tuple(std::vector<uint8_t>(), 0, 0);
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> executableContent(fileSize);
    file.read(reinterpret_cast<char*>(&executableContent[0]), fileSize);
    file.close();

    return std::make_tuple(executableContent, 0, fileSize);
}
