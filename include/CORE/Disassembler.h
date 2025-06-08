#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <tuple>

/// @brief DissassemblerInternals namespace, which contains the Disassembler class for disassembling executable files \namespace DissassemblerInternals
namespace DissassemblerInternals
{
    /// @brief The Disassembler class \class Disassembler
    class Disassembler
    {
    public:

        /**
         * @brief Construct a new Disassembler object
         */
        Disassembler();

        /**
         * @brief Destroy the Disassembler object
         */
        ~Disassembler();

        /**
         * @brief Disassemble 32-bit x86 instructions
         * @param data The executable data
         * @param offset The offset to start disassembling
         * @param size The size of the executable data
         */
        static void disassemble32bit(const std::vector<uint8_t>& data, size_t offset, size_t size);

        /**
         * @brief Disassemble 64-bit x86 instructions
         * @param data The executable data
         * @param offset The offset to start disassembling
         * @param size The size of the executable data
         */
        static void disassemble64bit(const std::vector<uint8_t>& data, size_t offset, size_t size);

        /**
         * @brief Disassemble ARM instructions
         * @param data The executable data
         * @param offset The offset to start disassembling
         * @param size The size of the executable data
         */
        static void dissassembleArm(const std::vector<uint8_t>& data, size_t offset, size_t size);

        /**
         * @brief Get the executable data from a file
         * @param path The path to the executable
         * @return A tuple containing the executable data, offset, and size
         */
        static std::tuple<std::vector<uint8_t>, size_t, size_t> getExecutable(const std::string& path);

        /**
         * @brief Print the disassembly of an executable
         * @param data The executable data
         * @param offset The offset to start disassembling
         * @param size The size of the executable data
         */
        static void printDisassembly(const std::vector<uint8_t>& data, size_t offset, size_t size);

    private:
    };
}