#include "../../include/FILEIO/FileIO.h"
#include "../../include/FILEIO/Utils.h"
#include "../../include/FILEIO/FileEditor.h"
#include "../../include/CORE/Injector.h"
#include "../../include/CORE/PE.h"
#include "../../include/CORE/Disassembler.h"
#include "../../include/CORE/Detector.h"
#include "../../include/CORE/Debugger.h"
#include "../../include/CORE/Logger.h"
#include "../../include/VIEW/Entropy.h"
#include "../../include/VIEW/GraphView.h"
#include "../../include/CLI/CLI.h"
#include "../../include/CLI/Console.h"
#include "../../include/MANMON/MemoryManager.h"
#include "../../include/MANMON/PerfMon.h"
#include "../../include/MANMON/MemProfiler.h"
#include "../../include/LIBMAN/MacOsInjector.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>

#if defined(__APPLE__)
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_metal.h"
#include <cstdio>
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_metal.h"
#include <Cocoa/Cocoa.h>
#include <mach-o/nlist.h>
#include <future>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

using namespace PeInternals;
using namespace FileIoInternals;
using namespace UtilsInternals;
using namespace DllInjector;
using namespace EntropyInternals;
using namespace DissassemblerInternals;
using namespace CliInterface;
using namespace DetectorInternals;
using namespace FileEditorInternals;
using namespace ConsoleInternals;

FileEditor fileEditor;
class Debugger debugger;

struct MenuItemInfo
{
    std::string output;
    bool windowOpen = false;
};

std::string sSelectedFile;
std::string filePath;
bool showEntropyHistogram = false;
bool openFileForEdit = false;
bool showImportTableWindow = false;
bool showConsole = false;
bool showDebugger = false;
bool showFileEdit = false;
bool showMemoryProfiler = false;
bool showProcessId = false;
bool showUtils = false;
bool showInject = false;

void runCLI(int argc, char** argv)
{
    CLI::startCli(argc, argv);
}
std::map<std::string, MenuItemInfo> menuItemInfo;

/**
 * @brief Initialize menu item information
 */
void initMenuItemInfo()
{
    // File path input
    menuItemInfo["Import Table"];
    menuItemInfo["Export Table"];
    menuItemInfo["Resources"];
    menuItemInfo["Section Info"];
    menuItemInfo["Headers"];
    menuItemInfo["Process Id"];
    menuItemInfo["Checksum"];
    menuItemInfo["Entropy Histogram"];
    menuItemInfo["Disassemble"];
    menuItemInfo["UPX Detection"];
    menuItemInfo["Themida Detection"];
    menuItemInfo["Entry Point"];
    menuItemInfo["Find Debugger"];
    menuItemInfo["Nt Global Flag"];
    menuItemInfo["Heap Flags"];
    menuItemInfo["Output Debug String"];
    menuItemInfo["Edit File"];
    menuItemInfo["Console"];
    menuItemInfo["Injector"];
}

/**
 * @brief Update menu item windows
 */
void updateMenuItemWindows()
{
    for (auto& item : menuItemInfo)
    {
        if (item.second.output.empty() && !ImGui::IsItemHovered())
        {
            item.second.windowOpen = false;
        }
    }
}

/**
 * @brief Open file dialog
 * @return true if file is opened successfully, false otherwise
 */
bool openFile()
{
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

    [openDlg setCanChooseFiles:YES];
    [openDlg setCanChooseDirectories:NO];
    [openDlg setAllowsMultipleSelection:NO];

    if ([openDlg runModal] == NSModalResponseOK)
    {
        NSURL* url = [[openDlg URLs] objectAtIndex:0];
        NSString* filePathString = [url path];
        const char* cString = [filePathString UTF8String];
        filePath = cString;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Display output window
 * @param title Window title
 * @param output Output string
 * @param windowOpen Window open flag
 */
void displayOutputWindow(const char* title, std::string& output, bool& windowOpen)
{
    if (!output.empty())
    {
        windowOpen = true;
        ImGui::Begin(title, &windowOpen, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("%s", output.c_str());
        if (!windowOpen)
            output.clear();
        ImGui::End();
    }
}

/**
 * @brief Process menu item
 * @param label Menu item label
 * @param action Menu item action
 * @param itemName Menu item name
 */
void processMenuItem(const char* label, const std::function<void()>& action, const std::string& itemName)
{
    if (ImGui::MenuItem(label))
    {
        std::stringstream output;
        std::streambuf* old_cout = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        action();
        menuItemInfo[itemName].output = output.str();
        std::cout.rdbuf(old_cout);
        menuItemInfo[itemName].windowOpen = true;
    }
}

/**
 * @brief Process entropy menu item
 * @param filePath File path
 * @param histogram Histogram
 * @param entropyOutput Entropy output
 */
void processEntropyMenuItem(const std::string& filePath, std::vector<int>& histogram, std::vector<std::string>& entropyOutput)
{
    std::vector<uint8_t> fileData = FileIO::readFile(filePath);
    Entropy ent;
    histogram = Entropy::createHistogram(fileData);
    entropyOutput.clear();
    for (int i = 0; i < histogram.size(); i++)
    {
        std::stringstream output;
        std::streambuf* old_cout = std::cout.rdbuf();
        std::cout.rdbuf(output.rdbuf());
        ent.printEntropy(fileData, i, 1);
        std::cout.rdbuf(old_cout);
        entropyOutput.push_back(output.str());
    }
}

/**
 * @brief Process file and menu item
 * @param label Menu item label
 * @param filePath File path
 * @param action Action
 */
void processFileAndMenuItem(const char* label, const std::string& filePath, const std::function<void(const std::vector<uint8_t>&)>& action)
{
    processMenuItem(label, [&] {
        std::vector<uint8_t> fileData = FileIO::readFile(filePath);
        action(fileData);
    }, label);
}

/**
 * @brief Show metrics window
 * @param show_metrics Show metrics flag
 */
void showMetricsWindow(bool& show_metrics)
{
    if (show_metrics)
    {
        ImGui::ShowMetricsWindow(&show_metrics);
    }
}

/**
 * @brief Show file selector
 * @param filePathInput File path input
 */
void showFileSelector(std::string& filePathInput)
{
    if (ImGui::Button("Select File"))
    {
        openFile();
        filePathInput = filePath;
    }

    char filePathInputBuffer[256];
    strcpy(filePathInputBuffer, filePathInput.c_str());
    ImGui::InputText("File Path", filePathInputBuffer, sizeof(filePathInputBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
    filePathInput = filePathInputBuffer;
}

/**
 * @brief Show file editor window
 * @param fileEditor File editor
 * @param showFileEdit Show file edit flag
 * @param filename File name
 */
void showFileEditorWindow(FileEditor& fileEditor, bool& showFileEdit, const std::string& filename)
{
    if (!showFileEdit)
        return;

    if (filename.empty())
        return;

    ImGui::Begin("File Editor", &showFileEdit, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

    if (!fileEditor.isOpen())
    {
        if (fileEditor.openFileForRead(filename))
        {
            std::string fileContent = fileEditor.readFile();
            fileEditor.SetText(fileContent);
        }
    }

    if (ImGui::Button("Save"))
    {
        if (fileEditor.openFileForWrite(filename))
        {
            std::string fileContent = fileEditor.GetText();
            fileEditor.writeFile(fileContent);
            fileEditor.closeFile();
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Undo"))
    {
        fileEditor.Undo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo"))
    {
        fileEditor.Redo();
    }

    ImVec2 availSize = ImGui::GetContentRegionAvail();
    fileEditor.Render("File Editor", availSize, true);

    ImGui::End();
}

/**
 * @brief Show extract menu
 * @param filePath File path
 */
void showExtractMenu(const std::string& filePath)
{
    if (ImGui::BeginMenu("Extract"))
    {
        processFileAndMenuItem("Import Table", filePath, PE::extractImportTable);
        processFileAndMenuItem("Export Table", filePath, PE::extractExportTable);
        processFileAndMenuItem("Resources", filePath, PE::extractResources);
        processFileAndMenuItem("Section Info", filePath, PE::extractSectionInfo);
        processFileAndMenuItem("Headers", filePath, PE::parseHeaders);
        ImGui::EndMenu();
    }
}

/**
 * @brief Show process id menu
 * @param filePath File path
 */
void showProcessIdMenu(const std::string& filePath, bool& showProcessId)
{
    if (!showProcessId || filePath.empty())
        return;

    if (showProcessId)
    {
        auto exeName = static_cast<const char *>(filePath.c_str());
#if defined(__APPLE__)
        InjectorMacOS injector;
        InjectorMacOS::CreatePlatform();
        auto id = injector.GetProcId(exeName);
#elif defined(__linux__)
        InjectorLinux injector;
        InjectorLinux::CreatePlatform();
        auto id = injector.GetProcId(exeName);
#elif defined(_WIN32)
        InjectorWin injector;
        InjectorWin::CreatePlatform();
        auto id = injector.GetProcId(exeName);
#endif
        ImGui::Begin("Process Id", &showProcessId);
        ImGui::Text("Process ID: %d of %s", id, exeName);
        ImGui::End();
    }
}

/**
 * @brief Show metrics menu
 * @param show_metrics Show metrics flag
 */
void showMetricsMenu(bool& show_metrics)
{
    if (ImGui::BeginMenu("Metrics"))
    {
        if (ImGui::MenuItem("Show Metrics"))
        {
            show_metrics = true;
        }
        ImGui::EndMenu();
    }
}

/**
 * @brief Show file menu
 * @param filePath File path
 */
void showUtilsMenu(const std::string& filePath, bool& showUtils)
{
    if (!showUtils || filePath.empty())
        return;

    if (showUtils && !filePath.empty())
    {
        auto checksumOutput = Utils::calculateChecksum(FileIO::readFile(filePath));
        auto checksum = std::to_string(checksumOutput);

        ImGui::Begin("Checksum", &showUtils);
        ImGui::Text("Checksum: %s", checksum.c_str());
        ImGui::End();
    }
}

/**
 * @brief Show entropy menu
 * @param filePath File path
 * @param histogram Histogram
 * @param entropyOutput Entropy output
 */
void showEntropyMenu(const std::string& filePath, std::vector<int>& histogram, std::vector<std::string>& entropyOutput)
{
    if (ImGui::BeginMenu("Entropy"))
    {
        if (ImGui::MenuItem("Entropy Histogram"))
        {
            processEntropyMenuItem(filePath, histogram, entropyOutput);
            showEntropyHistogram = true;
        }
        ImGui::EndMenu();
    }
}

/**
 * @brief Show disassembler menu
 * @param filePath File path
 * @param disassemblyOutput Disassembly output
 */
void showDisassemblerMenu(const std::string& filePath, std::vector<std::string>& disassemblyOutput)
{
    if (ImGui::BeginMenu("Disassembler"))
    {
        processFileAndMenuItem("Disassemble", filePath, [&](const std::vector<uint8_t>& fileData)
        {
            Disassembler dis;
            auto exe = dis.getExecutable(filePath);
            dis.printDisassembly(std::get<0>(exe), std::get<1>(exe), std::get<2>(exe));
        });
        ImGui::EndMenu();
    }
}

/**
 * @brief Show detector menu
 * @param filePath File path
 */
void showDetectorMenu(const std::string& filePath)
{
    if (ImGui::BeginMenu("Detector"))
    {
        std::vector<uint8_t> fileData = FileIO::readFile(filePath);
        PackerDetection packerDetection(fileData);
        processMenuItem("UPX", [&] { packerDetection.detectUPX(); }, "UPX Detection");
        processMenuItem("Themdia", [&] { packerDetection.detectThemida(); }, "Themida Detection");
        ImGui::EndMenu();
    }
}

/**
 * @brief Show anti debug menu
 * @param filePath File path
 */
void showAntiDebugMenu(const std::string& filePath)
{
    if (ImGui::BeginMenu("Anti Debug"))
    {
        std::vector<uint8_t> fileData = FileIO::readFile(filePath);
        AntiDebugDetection antiDebugDetection(fileData);
        processMenuItem("Check Entry Point", [&] { antiDebugDetection.checkEntryPoint(fileData); }, "Entry Point");
        processMenuItem("Find Debugger", [&] { antiDebugDetection.detectIsDebuggerPresent(); }, "Find Debugger");
        processMenuItem("Nt Global Flag", [&] { antiDebugDetection.detectNtGlobalFlag(); }, "Nt Global Flag");
        processMenuItem("Heap Flags", [&] { antiDebugDetection.detectHeapFlags(); }, "Heap Flags");
        processMenuItem("Output Debug String", [&] { antiDebugDetection.detectOutputDebugString(); }, "Output Debug String");
        ImGui::EndMenu();
    }
}

/**
 * @brief Show file editor menu
 * @param fileEditor File editor
 * @param showFileEdit Show file edit flag
 * @param filePath File path
 */
void showHistogramWindow(const std::vector<int>& histogram, const std::vector<std::string>& entropyOutput, bool& showEntropyHistogram)
{
    if (showEntropyHistogram && !histogram.empty())
    {
        ImGui::Begin("Entropy Histogram", &showEntropyHistogram, ImGuiWindowFlags_HorizontalScrollbar);

        std::vector<float> histogramFloat(histogram.begin(), histogram.end());
        ImGui::PlotHistogram("Histogram", histogramFloat.data(), histogramFloat.size(), 0, nullptr, 0.0f, *std::max_element(histogramFloat.begin(), histogramFloat.end()), ImVec2(0, 200));

        for (const auto& output : entropyOutput)
        {
            ImGui::Text("%s", output.c_str());
        }

        ImGui::End();
    }
}

/**
 * @brief Show file editor window
 * @param fileEditor File editor
 * @param showFileEdit Show file edit flag
 * @param filePath File path
 */
void showDetailedViewOfImportTable(const std::string& filePath, bool& showImportTableWindow)
{
    if (!showImportTableWindow)
        return;

    if (filePath.empty())
        return;

    PE pe;
    std::vector<uint8_t> fileData = FileIO::readFile(filePath);

    PE::extractImportTable(fileData);
    auto functionNames = PE::getFunctionNames(fileData);
    auto functionAddresses = PE::getFunctionAddresses(fileData);
    auto dllNames = PE::getDllNames(fileData);
    auto dllAddresses = PE::getDllAddresses(fileData);
    size_t funcCount = functionNames.size();
    size_t dllCount = dllNames.size();

    ImGui::Begin("Import Table", &showImportTableWindow);
    ImGui::Text("Number of Functions Imported: %zu", funcCount);

    if (ImGui::BeginTable("ImportTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        ImGui::TableSetupColumn("Function Name");
        ImGui::TableSetupColumn("Function Address");
        ImGui::TableSetupColumn("DLL Name");
        ImGui::TableSetupColumn("DLL Address");
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < funcCount; ++i)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (i < functionNames.size())
                ImGui::Text("%s", functionNames[i].c_str());
            else
                ImGui::Text("N/A");

            ImGui::TableSetColumnIndex(1);
            if (i < functionAddresses.size())
                ImGui::Text("0x%llx", functionAddresses[i]);
            else
                ImGui::Text("N/A");

            ImGui::TableSetColumnIndex(2);
            if (i < dllNames.size())
                ImGui::Text("%s", dllNames[i].c_str());
            else
                ImGui::Text("N/A");

            ImGui::TableSetColumnIndex(3);
            if (i < dllAddresses.size())
                ImGui::Text("0x%llx", dllAddresses[i]);
            else
                ImGui::Text("N/A");
        }

        ImGui::EndTable();
    }

    GraphView graphView;

    for (int i = 0; i < funcCount; ++i)
    {
        graphView.AddNode(i * 2, ImVec2(100, 100 + i * 80), ImVec2(100, 50), functionNames[i]);
    }

    for (int i = 0; i < dllCount; ++i)
    {
        graphView.AddNode(i * 2 + 1, ImVec2(300, 100 + i * 80), ImVec2(100, 50), dllNames[i]);
    }

    size_t minCount = std::min(funcCount, dllCount);
    for (int i = 0; i < minCount; ++i)
    {
        graphView.AddConnection(i * 2, i * 2 + 1);
    }

    graphView.Render();

    ImGui::End();
}

/**
 * @brief Show console window
 * @param showConsole Show console flag
 */
void showConsoleWindow(bool &showConsole)
{
    if (!showConsole)
        return;

    static char inputBuf[256] = "";
    static ImGuiTextBuffer consoleBuf;
    static bool scrollToBottom = false;
    Console con;
    static std::future<std::string> futureResult;
    static bool isExecuting = false;

    if (ImGui::Begin("Console"))
    {
        ImGui::TextWrapped("Enter commands and see the output here.");

        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImGui::TextUnformatted(consoleBuf.begin());

            if (scrollToBottom)
                ImGui::SetScrollHereY(1.0f);

            scrollToBottom = false;
        }
        ImGui::EndChild();

        if (ImGui::InputText("Input", inputBuf, IM_ARRAYSIZE(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            consoleBuf.appendf(">>> %s\n", inputBuf);
            std::string command(inputBuf);
            inputBuf[0] = '\0';
            scrollToBottom = true;

            // Execute command asynchronously
            futureResult = std::async(std::launch::async, [command]() {
                return Console::executeShellCommand(command);
            });
            isExecuting = true;
        }

        // Check if command execution is finished
        if (isExecuting && futureResult.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
        {
            std::string commandOutput = futureResult.get();
            consoleBuf.appendf("%s\n", commandOutput.c_str());
            scrollToBottom = true;
            isExecuting = false;

            if (commandOutput.find("exit") != std::string::npos)
            {
                con.stop();
                showConsole = false;
            }
        }
    }
    ImGui::End();
}

/**
 * @brief Show debugger window
 * @param showDebugger Show debugger flag
 * @param debugger Debugger
 */
void showDebuggerWindow(bool& showDebugger, class Debugger& debugger)
{
    if (!showDebugger)
        return;

    ImGui::Begin("Debugger", &showDebugger);
    ImGui::BeginChild("SourceWindow", ImVec2(ImGui::GetWindowWidth() * 0.6f, 0), true);
    ImGui::Text("Source Code");
    std::vector<uint8_t> fileData = FileIO::readFile(filePath);
    fileEditor.openFileForRead(filePath);
    std::string fileContent = fileEditor.readFile();
    ImGui::Text("%s", fileContent.c_str());
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("ControlArea", ImVec2(0, 0), true);
    ImGui::Text("Control");
    if (ImGui::Button("Start Debugging"))
    {
        debugger.startDebugging("executable_path");
    }
    if (ImGui::Button("Step Into"))
    {
        debugger.stepInto();
    }
    if (ImGui::Button("Step Over"))
    {
        debugger.stepOver();
    }
    if (ImGui::Button("Step Out"))
    {
        debugger.stepOut();
    }
    if (ImGui::Button("Run"))
    {
        debugger.run();
    }
    ImGui::EndChild();

    ImGui::BeginChild("Callstack", ImVec2(ImGui::GetWindowWidth() * 0.3f, 0), true);
    ImGui::Text("Callstack");
    auto callstack = Debugger::getCallStack();
    for (const auto& frame : callstack)
    {
        ImGui::Text("%s", frame.c_str());
    }
    ImGui::EndChild();

    ImGui::BeginChild("Watch", ImVec2(0, 100), true);
    ImGui::Text("Watch");
    auto watch = Debugger::getWatch();
    for (const auto& frame : watch)
    {
        ImGui::Text("%s", frame.c_str());
    }
    ImGui::EndChild();

    ImGui::BeginChild("Locals", ImVec2(0, 100), true);
    ImGui::Text("Locals");
    auto locals = Debugger::getLocals();
    for (const auto& frame : locals)
    {
        ImGui::Text("%s", frame.c_str());
    }
    ImGui::EndChild();

    ImGui::End();
}

void showMemoryProfilerWindow(bool& showMemoryProfiler)
{
    if (!showMemoryProfiler)
        return;

    MemProfiler memProfiler;
    memProfiler.populateMemoryUsage();
    memProfiler.populateRAMUsage();
    memProfiler.populateVRAMUsage();

    auto totalMem = memProfiler.getTotalMemoryUsage();
    auto totalRam = memProfiler.getTotalRAMUsage();
    auto totalVRam = memProfiler.getTotalVRAMUsage();

    ImGui::Begin("Memory Profiler", &showMemoryProfiler);

    /*for (const auto& usage : totalMem)
    {
        ImGui::Text("%s: %zu", usage.first.c_str(), usage.second);
    }*/

    ImGui::Text("Total Memory Usage: %zu", totalMem);

    /*for (const auto& usage : memRam)
    {
        ImGui::Text("%s: %zu", usage.first.c_str(), usage.second);
    }*/

    ImGui::Text("Total RAM Usage: %zu", totalRam);

    /*for (const auto& usage : memVRam)
    {
        ImGui::Text("%s: %zu", usage.first.c_str(), usage.second);
    }*/

    ImGui::Text("Total VRAM Usage: %zu", totalVRam);

    ImGui::End();
}

void showInjectorMenu(bool& showInject)
{
    if (!showInject)
        return;

    std::vector<std::string> processNames = MacInject::GetRunningProcesses();

    ImGui::Begin("Injector", &showInject);
    ImGui::Text("Select a dynamic library to inject into a process.");

    if (ImGui::Button("Select File"))
    {
        openFile();
    }

    ImGui::Text("Selected file: %s", filePath.c_str());

    static int selectedProcess = -1;
    static bool processConfirmed = false;

    if (ImGui::BeginCombo("Select Process", (selectedProcess >= 0 && selectedProcess < processNames.size()) ? processNames[selectedProcess].c_str() : "Select a process"))
    {
        for (int i = 0; i < processNames.size(); i++)
        {
            bool isSelected = (selectedProcess == i);
            if (ImGui::Selectable(processNames[i].c_str(), isSelected))
            {
                selectedProcess = i;
                processConfirmed = false;
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (selectedProcess != -1 && !processConfirmed)
    {
        if (ImGui::Button("Confirm Process"))
        {
            processConfirmed = true;
        }
    }

    if (!filePath.empty() && selectedProcess != -1 && processConfirmed)
    {
        MacInject injector(filePath.c_str(), processNames[selectedProcess].c_str());
        std::vector<Result> results = injector.InjectDylib();

        for (const auto& result : results)
        {
            ImGui::Text("%s", result.message.c_str());
        }
    }

    ImGui::End();
}

/**
 * @brief Log program
 * @param program Program
 */
void logProgram(const std::function<void()>& program)
{
    Logger logger;
    logger.log(Logger::LogLevel::INFO, "Program started.", "");

    std::stringstream programOutput;
    std::streambuf* old_stdout = std::cout.rdbuf();
    std::cout.rdbuf(programOutput.rdbuf());

    program();

    std::cout.rdbuf(old_stdout);

    logger.log(Logger::LogLevel::INFO, programOutput.str(), "");
    logger.log(Logger::LogLevel::INFO, "Program finished.", "");
}

/**
 * @brief Set application icon
 */
void setApplicationIcon()
{
    auto path = "../icon/cog.icns";
    NSString *iconFileName = @(path);
    NSString *iconFilePath = [[NSBundle mainBundle] pathForResource:[iconFileName stringByDeletingPathExtension] ofType:[iconFileName pathExtension]];

    if (iconFilePath == nil)
    {
        NSLog(@"Failed to find image %@", iconFileName);
        return;
    }

    NSImage *iconImage = [[NSImage alloc] initWithContentsOfFile:iconFilePath];
    if (!iconImage)
    {
        NSLog(@"Failed to load image %@", iconFileName);
        return;
    }

    [NSApp setApplicationIconImage:iconImage];
}

/**
 * @brief Run GUI
 * @return 0 if successful, -1 otherwise
 */
int runGUI()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

    // Setup style
    ImGui::StyleColorsDark();

    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Inform SDL that we will be using metal for rendering. Without this hint initialization of metal renderer may fail.
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

    // Enable native IME.
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

    SDL_Window* window = SDL_CreateWindow("PETOOL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (window == nullptr)
    {
        printf("Error creating window: %s\n", SDL_GetError());
        return -2;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        printf("Error creating renderer: %s\n", SDL_GetError());
        return -3;
    }

    // set application icon
    setApplicationIcon();

    // Setup Platform/Renderer backends
    auto layer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(renderer);
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    ImGui_ImplMetal_Init(layer.device);
    ImGui_ImplSDL2_InitForMetal(window);

    id<MTLCommandQueue> commandQueue = [layer.device newCommandQueue];
    MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor new];

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

    static bool show_metrics = false;

    std::string filePathInput;
    std::vector<int> histogram;
    std::vector<std::string> entropyOutput;


    // Main loop
    bool done = false;
    while (!done)
    {
        @autoreleasepool
        {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    done = true;
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                    done = true;
            }

            int width, height;
            SDL_GetRendererOutputSize(renderer, &width, &height);
            layer.drawableSize = CGSizeMake(width, height);
            id<CAMetalDrawable> drawable = [layer nextDrawable];

            id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
            renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0] * clear_color[3], clear_color[1] * clear_color[3], clear_color[2] * clear_color[3], clear_color[3]);
            renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
            [renderEncoder pushDebugGroup:@"ImGui demo"];

            // Start the Dear ImGui frame
            ImGui_ImplMetal_NewFrame(renderPassDescriptor);
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport();

            if (ImGui::BeginMainMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Select File"))
                    {
                        openFile();
                        filePathInput = filePath;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Actions"))
                {
                    if (ImGui::BeginMenu("Extract"))
                    {
                        processFileAndMenuItem("Import Table", filePathInput, PE::extractImportTable);
                        processFileAndMenuItem("Export Table", filePathInput, PE::extractExportTable);
                        processFileAndMenuItem("Resources", filePathInput, PE::extractResources);
                        processFileAndMenuItem("Section Info", filePathInput, PE::extractSectionInfo);
                        processFileAndMenuItem("Headers", filePathInput, PE::parseHeaders);
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Process Id"))
                    {
                        if (ImGui::MenuItem("Get Process Id"))
                        {
                            showProcessIdMenu(filePathInput, showProcessId);
                            showProcessId = true;
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("View"))
                {
                    if (ImGui::MenuItem("Show Metrics"))
                    {
                        show_metrics = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Utils"))
                {
                    if (ImGui::MenuItem("Checksum"))
                    {
                        showUtilsMenu(filePathInput, showUtils);
                        showUtils = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Entropy"))
                {
                    if (ImGui::MenuItem("Entropy Histogram"))
                    {
                        processEntropyMenuItem(filePathInput, histogram, entropyOutput);
                        showEntropyHistogram = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Disassembler"))
                {
                    processFileAndMenuItem("Disassemble", filePathInput, [&](const std::vector<uint8_t>& fileData) {
                        Disassembler dis;
                        auto exe = dis.getExecutable(filePathInput);
                        dis.printDisassembly(std::get<0>(exe), std::get<1>(exe), std::get<2>(exe));
                    });
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Detector"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    PackerDetection packerDetection(fileData);
                    processMenuItem("UPX", [&] { packerDetection.detectUPX(); }, "UPX Detection");
                    processMenuItem("Themdia", [&] { packerDetection.detectThemida(); }, "Themida Detection");
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Anti Debug"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    AntiDebugDetection antiDebugDetection(fileData);
                    processMenuItem("Check Entry Point", [&] { antiDebugDetection.checkEntryPoint(fileData); }, "Entry Point");
                    processMenuItem("Find Debugger", [&] { antiDebugDetection.detectIsDebuggerPresent(); }, "Find Debugger");
                    processMenuItem("Nt Global Flag", [&] { antiDebugDetection.detectNtGlobalFlag(); }, "Nt Global Flag");
                    processMenuItem("Heap Flags", [&] { antiDebugDetection.detectHeapFlags(); }, "Heap Flags");
                    processMenuItem("Output Debug String", [&] { antiDebugDetection.detectOutputDebugString(); }, "Output Debug String");
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit"))
                {
                    if (ImGui::MenuItem("File Editor"))
                    {
                       showFileEditorWindow(fileEditor, showFileEdit, filePathInput);
                       showFileEdit = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Graph"))
                {
                    if (ImGui::MenuItem("Import Table"))
                    {
                        showDetailedViewOfImportTable(filePathInput, showImportTableWindow);
                        showImportTableWindow = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Console"))
                {
                    if (ImGui::MenuItem("Show Console"))
                    {
                        showConsoleWindow(showConsole);
                        showConsole = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Debugger"))
                {
                    if (ImGui::MenuItem("Show Debugger"))
                    {
                        showDebuggerWindow(showDebugger, debugger);
                        showDebugger = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Memory Profiler"))
                {
                    if (ImGui::MenuItem("Show Memory Profiler"))
                    {
                        showMemoryProfilerWindow(showMemoryProfiler);
                        showMemoryProfiler = true;
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Injector"))
                {
                    if (ImGui::MenuItem("Show Injector"))
                    {
                        showInjectorMenu(showInject);
                        showInject = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            showMetricsWindow(show_metrics);
            showFileSelector(filePathInput);
            showExtractMenu(filePathInput);
            showProcessIdMenu(filePathInput, showProcessId);
            showDetectorMenu(filePathInput);
            showAntiDebugMenu(filePathInput);
            showHistogramWindow(histogram, entropyOutput, showEntropyHistogram);
            showFileEditorWindow(fileEditor, showFileEdit, filePathInput);
            showDetailedViewOfImportTable(filePathInput, showImportTableWindow);
            showConsoleWindow(showConsole);
            showDebuggerWindow(showDebugger, debugger);
            showMemoryProfilerWindow(showMemoryProfiler);
            showUtilsMenu(filePathInput, showUtils);
            showInjectorMenu(showInject);

            updateMenuItemWindows();

            for (auto& item : menuItemInfo)
            {
                const std::string& itemName = item.first;
                displayOutputWindow(itemName.c_str(), item.second.output, item.second.windowOpen);
            }

            // Rendering
            ImGui::Render();
            ImGui_ImplMetal_RenderDrawData(ImGui::GetDrawData(), commandBuffer, renderEncoder);

            [renderEncoder popDebugGroup];
            [renderEncoder endEncoding];

            [commandBuffer presentDrawable:drawable];
            [commandBuffer commit];
        }
    }

    // Cleanup
    ImGui_ImplMetal_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

   return 0;
}

/**
 * @brief Main function
 * @param argc Argument count
 * @param argv Argument values
 * @return 0 if successful, -1 otherwise
 */
int main(int argc, char** argv)
{
    bool use_gui = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--gui")
        {
            use_gui = true;
            break;
        }
    }

    if (use_gui)
    {
        PerformanceMonitor::start("GUI Performance");
        logProgram([]() { runGUI(); });
        PerformanceMonitor::stop("GUI Performance");
    }
    else
    {
        PerformanceMonitor::start("CLI Performance");
        logProgram([&]() { runCLI(argc, argv); });
        PerformanceMonitor::stop("CLI Performance");
    }

    PerformanceMonitor::report();
    MemoryManager::detectMemoryLeaks();
    return 0;
}
#endif