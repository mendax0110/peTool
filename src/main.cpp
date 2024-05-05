#include "./include/PE.h"
#include "./include/FileIO.h"
#include "./include/Untils.h"
#include "./include/Injector.h"
#include <iostream>
#include <vector>
#include <string>

using namespace PeInternals;
using namespace FileIoInternals;
using namespace UntilsInternals;
using namespace DllInjector;

void displayMenu()
{
    std::string menu =  "PE Tool Command-Line Interface\n"
                        "Usage: pe_tool <option> <file_path>\n"
                        "Options:\n"
                        "  1. Extract Import Table\n"
                        "  2. Extract Export Table\n"
                        "  3. Extract Resources\n"
                        "  4. Extract Section Info\n"
                        "  5. Parse Headers\n"
                        "-----------Injector Options-----------\n"
                        "  6. Get Process ID\n"
                        "  7. Inject DLL\n";


    std::cout << menu;
}

int main(int argc, char* argv[])
{
    if (argc < 2 || std::string(argv[1]) == "--help")
    {
        displayMenu();
        return 1;
    }

    int option = std::stoi(argv[1]);

    if (option >= 1 && option <= 5)
    {
        if (argc < 3)
        {
            std::cerr << "Error: Insufficient arguments for option " << option << std::endl;
            return 1;
        }
        
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
        }
        return 0;
    }
    else if (option >= 6 && option <= 7)
    {
        InjectorPlatform* platform = new InjectorPlatform();
        
        switch (option)
        {
            case 6:
            {
                std::cout << "Getting Process ID...\n";
                unsigned int processId = platform->getPlatform()->GetProcId(argv[2]);
                if (processId > 0)
                    std::cout << "Process ID: " << processId << std::endl;
                else
                    std::cerr << "Failed to get process ID.\n";
            }
            break;
            case 7:
            {
                std::cout << "Injecting DLL...\n";
                bool success = platform->getPlatform()->InjectDLL(std::stoi(argv[2]), argv[3]);
                if (success)
                    std::cout << "DLL Injected successfully.\n";
                else
                    std::cerr << "Failed to inject DLL.\n";
            }
            break;
        }
        delete platform;
        return 0;
    }
}
