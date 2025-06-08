#pragma once

#include <iostream>
#include <vector>
#include <string>

/// @brief Rebuilder class, which provides methods for fixing, wiping, rebuilding, validating, binding imports, and changing the image base of PE files \class Rebuilder
class Rebuilder
{
public:

    /**
     * @brief Construct a new Rebuilder object
     */
    Rebuilder();

    /**
     * @brief Destroy the Rebuilder object
     */
    ~Rebuilder();

    /**
     * @brief Fix a dumped PE file
     * @param filePath The path to the PE file to fix
     * @return true if the fix was successful, false otherwise
     */
    static bool fixDump(const std::string& filePath);

    /**
     * @brief Wipe locations in a PE file
     * @param filePath The path to the PE file to wipe
     * @return true if the wipe was successful, false otherwise
     */
    static bool wipeLocations(const std::string& filePath);

    /**
     * @brief Rebuild the resource directory in a PE file
     * @param filePath The path to the PE file to rebuild the resource directory
     * @return true if the rebuild was successful, false otherwise
     */
    static bool rebuildResourceDirectory(const std::string& filePath);

    /**
     * @brief Validate a PE file
     * @param filePath The path to the PE file to validate
     * @return true if the validation was successful, false otherwise
     */
    static bool validatePEFile(const std::string& filePath);

    /**
     * @brief Bind imports in a PE file
     * @param filePath The path to the PE file to bind imports
     * @return true if the binding was successful, false otherwise
     */
    static bool bindImports(const std::string& filePath);

    /**
     * @brief Change the image base of a PE file
     * @param filePath The path to the PE file to change the image base
     * @param newImageBase The new image base to set
     * @return true if the change was successful, false otherwise
     */
    static bool changeImageBase(const std::string& filePath, uint64_t newImageBase);

private:

    /**
     * @brief Read a file into a vector of bytes
     * @param filePath The path to the file to read
     * @return A vector of bytes containing the file data
     */
    static std::vector<uint8_t> readFile(const std::string& filePath);

    /**
     * @brief Write a vector of bytes to a file
     * @param filePath The path to the file to write
     * @param data The vector of bytes to write to the file
     * @return true if the write was successful, false otherwise
     */
    static bool writeFile(const std::string& filePath, const std::vector<uint8_t>& data);

    /**
     * @brief Fixed the dumped PE file internally
     * @param data The vector of bytes containing the PE file data
     * @return A boolean indicating if the fix was successful
     */
    static bool fixDumpInternal(std::vector<uint8_t>& data);

    /**
     * @brief Wipe locations in the PE file internally
     * @param data The vector of bytes containing the PE file data
     * @return A boolean indicating if the wipe was successful
     */
    static bool wipeLocationsInternal(std::vector<uint8_t>& data);

    /**
     * @brief Rebuild the resource directory in the PE file internally
     * @param data The vector of bytes containing the PE file data
     * @return A boolean indicating if the rebuild was successful
     */
    static bool rebuildResourceDirectoryInternal(std::vector<uint8_t>& data);

    /**
     * @brief Validate the PE file internally
     * @param data The vector of bytes containing the PE file data
     * @return A boolean indicating if the validation was successful
     */
    static bool validatePEFileInternal(std::vector<uint8_t>& data);

    /**
     * @brief Bind imports in the PE file internally
     * @param data The vector of bytes containing the PE file data
     * @return A boolean indicating if the binding was successful
     */
    static bool bindImportsInternal(std::vector<uint8_t>& data);

    /**
     * @brief Change the image base of the PE file internally
     * @param data The vector of bytes containing the PE file data
     * @param newImageBase The new image base to set
     * @return A boolean indicating if the change was successful
     */
    static bool changeImageBaseInternal(std::vector<uint8_t>& data, uint64_t newImageBase);

};