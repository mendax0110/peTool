#include "./include/PE.h"
#include "./include/FileIO.h"
#include "./include/Utils.h"
#include "./include/Injector.h"
#include "./include/Entropy.h"
#include "./include/Disassembler.h"
#include "./include/CLI.h"
#include "./include/Detector.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

using namespace PeInternals;
using namespace FileIoInternals;
using namespace UtilsInternals;
using namespace DllInjector;
using namespace EntropyInternals;
using namespace DissassemblerInternals;
using namespace CliInterface;
using namespace DetectorInternals;

void CLI::printHelp()
{
    const std::string menu = R"(
        Extract Import Table: -eip <path_to_pe>
        Extract Export Table: -eep <path_to_pe>
        Extract Resources: -er <path_to_pe>
        Section Information: -si <path_to_pe>
        Header Information: -hi <path_to_pe>
        Process ID: -pid <path_to_pe>
        Calculate Checksum: -cc <path_to_pe>
        Show Entropy: -se <path_to_pe>
        Disassemble: -d <path_to_pe>
        Detector: -det <path_to_pe>
        Anti-Debug: -ad <path_to_pe>)";

    std::cout << menu << std::endl;
}

void CLI::printError(const std::string& message)
{

}

void CLI::printMessage(const std::string& message)
{

}

void extractImportTable(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    PE::extractImportTable(fileData);
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void extractExportTable(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    PE::extractExportTable(fileData);
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void extractResources(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    PE::extractResources(fileData);
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void extractSectionInfo(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    PE::extractSectionInfo(fileData);
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void parseHeaders(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    PE::parseHeaders(fileData);
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void getProcessId(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    auto exename = static_cast<const char*>(filePathInput.c_str());                
    std::unique_ptr<InjectorPlatform> injector(InjectorPlatform::CreatePlatform());
    if (injector)
    {
        unsigned int procId = injector->GetProcId(exename);
        output << "Process ID: " << procId;
    }
    else
    {
        output << "Failed to create platform-specific injector.";
    }
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void calculateChecksum(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    Utils unt;
    uint32_t checksum = unt.calculateChecksum(fileData);
    std::cout << "Checksum: " << checksum << std::endl;
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void showEntropy(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    Entropy ent;
    std::vector<int> histogram = ent.createHistogram(fileData);
    for (int i = 0; i < histogram.size(); i++)
    {
        std::stringstream output;
        std::streambuf* old_cout = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        ent.printEntropy(fileData, i, 1);
        std::cout.rdbuf(old_cout);
        std::cout << output.str();
    }
}

void disassemble(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    Disassembler dis;
    auto exe = dis.getExecutable(filePathInput);
    dis.printDisassembly(std::get<0>(exe), std::get<1>(exe), std::get<2>(exe));
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void detector(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    PackerDetection packerDetection(fileData);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    packerDetection.detectThemida() && packerDetection.detectUPX();
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void antiDebug(const std::string& filePathInput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
    AntiDebugDetection antiDebugDetection(fileData);
    std::stringstream output;
    std::streambuf* old_cout = std::cout.rdbuf();
    std::cout.rdbuf(output.rdbuf());
    antiDebugDetection.detectIsDebuggerPresent();
    antiDebugDetection.checkEntryPoint(fileData);
    antiDebugDetection.detectHeapFlags();
    antiDebugDetection.detectNtGlobalFlag();
    antiDebugDetection.detectOutputDebugString();
    std::cout.rdbuf(old_cout);
    std::cout << output.str();
}

void CLI::startCli(int argc, char** argv)
{
    if (argc < 2)
    {
        printHelp();
        return;
    }

    std::string command = argv[1];
    if (argc < 3)
    {
        std::cerr << "Error: Missing file path argument." << std::endl;
        printHelp();
        return;
    }

    std::string filePathInput = argv[2];

    if (command == "-eip")
    {
        extractImportTable(filePathInput);
    }
    else if (command == "-eep")
    {
        extractExportTable(filePathInput);
    }
    else if (command == "-er")
    {
        extractResources(filePathInput);
    }
    else if (command == "-si")
    {
        extractSectionInfo(filePathInput);
    }
    else if (command == "-hi")
    {
        parseHeaders(filePathInput);
    }
    else if (command == "-pid")
    {
        getProcessId(filePathInput);
    }
    else if (command == "-cc")
    {
        calculateChecksum(filePathInput);
    }
    else if (command == "-se")
    {
        showEntropy(filePathInput);
    }
    else if (command == "-d")
    {
        disassemble(filePathInput);
    }
    else if (command == "-det")
    {
        detector(filePathInput);
    }
    else if (command == "-ad")
    {
        antiDebug(filePathInput);
    }
    else
    {
        std::cerr << "Error: Unknown command." << std::endl;
        printHelp();
    }
}