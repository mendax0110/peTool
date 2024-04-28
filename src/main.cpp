#include "./include/PE.h"
#include "./include/FileIO.h"
#include "./include/Untils.h"
#include <iostream>
#include <vector>
#include <string>

using namespace PeInternals;
using namespace FileIoInternals;
using namespace UntilsInternals;


void displayMenu()
{
    std::string menu =  "PE Tool Command-Line Interface\n"
                        "Usage: pe_tool <option> <file_path>\n"
                        "Options:\n"
                        "  1. Extract Import Table\n"
                        "  2. Extract Export Table\n"
                        "  3. Extract Resources\n"
                        "  4. Extract Section Info\n"
                        "  5. Parse Headers\n";

    std::cout << menu;
}

int main(int argc, char* argv[])
{
    if (argc < 3 || std::string(argv[1]) == "--help")
    {
        displayMenu();
        return 1;
    }

    int option = std::stoi(argv[1]);
    std::string filePath = argv[2];

    std::vector<uint8_t> fileData = FileIO::readFile(filePath);

    if (fileData.empty())
    {
        std::cerr << "Error: Failed to read the file.\n";
        return 1;
    }

    switch (option)
    {
        case 1:
            std::cout << "Extracting Import Table...\n";
            PE::extractImportTable(fileData);
            break;
        case 2:
            std::cout << "Extracting Export Table...\n";
            PE::extractExportTable(fileData);
            break;
        case 3:
            std::cout << "Extracting Resources...\n";
            PE::extractResources(fileData);
            break;
        case 4:
            std::cout << "Extracting Section Info...\n";
            PE::extractSectionInfo(fileData);
            break;
        case 5:
            std::cout << "Parsing Headers...\n";
            PE::parseHeaders(fileData);
            break;
        default:
            std::cerr << "Error: Invalid option.\n";
            displayMenu();
            return 1;
    }

    return 0;
}
