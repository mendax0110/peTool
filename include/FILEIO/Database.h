#pragma once

#include <iostream>
#include <filesystem>
#include <string>
#include <map>

class Database
{
public:
    Database();
    ~Database();

    using Row = std::map<std::string, std::string>;

    bool load(const std::string& fileName);
    [[nodiscard]] bool save(const std::string& fileName) const;

    void addRow(const Row& row);
    [[nodiscard]] const std::vector<Row>& getRows() const;

    void databasePath(const std::filesystem::path& path);
    [[nodiscard]] const std::filesystem::path& databasePath() const;

private:
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string join(const std::vector<std::string>& elements, char delimiter);
    static std::string trim(const std::string& str);

    std::filesystem::path filePath;
    std::vector<std::string> headers;
    std::vector<Row> rows;
};