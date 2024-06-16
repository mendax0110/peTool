#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <tuple>

/**
 * @brief The DissassemblerInternals namespace
 * @namespace DissassemblerInternals
 */
namespace DissassemblerInternals
{
    /**
     * @brief The Disassembler class
     * @class Disassembler
     */
    class Disassembler
    {
    public:
        Disassembler();
        ~Disassembler();
        void disassemble32bit(const std::vector<uint8_t>& data, size_t offset, size_t size);
        void disassemble64bit(const std::vector<uint8_t>& data, size_t offset, size_t size);
        void dissassembleArm(const std::vector<uint8_t>& data, size_t offset, size_t size);
        std::tuple<std::vector<uint8_t>, size_t, size_t> getExecutable(const std::string& path);
        void printDisassembly(const std::vector<uint8_t>& data, size_t offset, size_t size);
    private:
    };
}