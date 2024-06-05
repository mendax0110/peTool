#include "../include/FILEIO/Database.h"

#include <fstream>
#include <sstream>
#include <iostream>

Database::Database() = default;

Database::~Database() = default;

bool Database::load(const std::string& fileName)
{
    std::ifstream file(fileName);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << fileName << std::endl;
        return false;
    }

    std::string line;
    if (std::getline(file, line))
    {
        headers = split(line, ',');
    }

    while (std::getline(file, line))
    {
        Row row;
        auto values = split(line, ',');
        for (size_t i = 0; i < headers.size() && i < values.size(); ++i)
        {
            row[headers[i]] = values[i];
        }
        rows.push_back(row);
    }

    file.close();
    return true;
}

bool Database::save(const std::string& fileName) const
{
    std::ofstream file(fileName);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << fileName << std::endl;
        return false;
    }

    file << join(headers, ',') << std::endl;
    for (const auto& row : rows)
    {
        std::vector<std::string> values;
        for (const auto& header : headers)
        {
            auto it = row.find(header);
            if (it != row.end())
            {
                values.push_back(it->second);
            }
            else
            {
                values.emplace_back("");
            }
        }
        file << join(values, ',') << std::endl;
    }

    file.close();
    return true;
}

void Database::addRow(const Row& row)
{
    rows.push_back(row);
}

const std::vector<Database::Row>& Database::getRows() const
{
    return rows;
}

void Database::databasePath(const std::filesystem::path& path)
{
    filePath = path;
}

const std::filesystem::path& Database::databasePath() const
{
    return filePath;
}

std::vector<std::string> Database::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);

    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }

    return tokens;
}

std::string Database::join(const std::vector<std::string>& elements, char delimiter)
{
    std::ostringstream os;
    for (auto it = elements.begin(); it != elements.end(); ++it)
    {
        if (it != elements.begin())
        {
            os << delimiter;
        }
        os << *it;
    }

    return os.str();
}

std::string Database::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first)
    {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

