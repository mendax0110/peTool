#pragma once

#include <iostream>
#include <filesystem>
#include <string>
#include <map>

/// @brief Database class, which provides methods for loading, saving, and managing a simple database of rows \class Database
class Database
{
public:

    /**
     * @brief Construct a new Database object
     */
    Database();

    /**
     * @brief Destroy the Database object
     */
    ~Database();

    /// @brief A row in the database, represented as a map of string key-value pairs
    using Row = std::map<std::string, std::string>;

    /**
     * @brief Load a database from a file
     * @param fileName The name of the file to load the database from
     * @return true if the database was successfully loaded, false otherwise
     */
    bool load(const std::string& fileName);

    /**
     * @brief Save the database to a file
     * @param fileName The name of the file to save the database to
     * @return true if the database was successfully saved, false otherwise
     */
    [[nodiscard]] bool save(const std::string& fileName) const;

    /**
     * @brief Add a header to the database
     * @param header The header to add
     */
    void addRow(const Row& row);

    /**
     * @brief Geter the headers of the database
     * @return A vector of Rows containing the headers
     */
    [[nodiscard]] const std::vector<Row>& getRows() const;

    /**
     * @brief Set the database path
     * @param path The path to the database file
     */
    void databasePath(const std::filesystem::path& path);

    /**
     * @brief Get the database path
     * @return The path to the database file
     */
    [[nodiscard]] const std::filesystem::path& databasePath() const;

private:

    /**
     * @brief Split a string by a delimiter
     * @param str The string to split
     * @param delimiter The delimiter to split by
     * @return A vector of strings containing the split elements
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief Join a vector of strings into a single string with a delimiter
     * @param elements The vector of strings to join
     * @param delimiter The delimiter to use for joining
     * @return A single string containing the joined elements
     */
    static std::string join(const std::vector<std::string>& elements, char delimiter);

    /**
     * @brief Trim whitespace from the beginning and end of a string
     * @param str The string to trim
     * @return A trimmed string
     */
    static std::string trim(const std::string& str);

    std::filesystem::path filePath;
    std::vector<std::string> headers;
    std::vector<Row> rows;
};