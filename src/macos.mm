#include "./include/FileIO.h"
#include "./include/Utils.h"
#include "./include/Injector.h"
#include "./include/PE.h"
#include "./include/Entropy.h"
#include "./include/Disassembler.h"
#include "./include/CLI.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>

#if defined(__APPLE__)
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_metal.h"
#include <stdio.h>
#include "SDL.h"
#include <Cocoa/Cocoa.h>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

using namespace PeInternals;
using namespace FileIoInternals;
using namespace UtilsInternals;
using namespace DllInjector;
using namespace EntropyInternals;
using namespace DissassemblerInternals;
using namespace CliInterface;

std::string sSelectedFile;
std::string filePath;

void runCLI(int argc, char** argv)
{
    CLI::startCli(argc, argv);
}

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

int runGUI()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

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

    // Setup Platform/Renderer backends
    CAMetalLayer* layer = (__bridge CAMetalLayer*)SDL_RenderGetMetalLayer(renderer);
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    ImGui_ImplMetal_Init(layer.device);
    ImGui_ImplSDL2_InitForMetal(window);

    id<MTLCommandQueue> commandQueue = [layer.device newCommandQueue];
    MTLRenderPassDescriptor* renderPassDescriptor = [MTLRenderPassDescriptor new];

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

    // File path input
    static std::string importTableOutput;
    static std::string exportTableOutput;
    static std::string resourcesOutput;
    static std::string sectionInfoOutput;
    static std::string headersOutput;
    static std::string processIdOutput;
    static std::string checksumOutput;
    std::vector<std::string> entropyOutput;
    std::vector<std::string> disassemblyOutput;

    // Define boolean variables to track window states
    static bool importTableWindowOpen = false;
    static bool exportTableWindowOpen = false;
    static bool resourcesWindowOpen = false;
    static bool sectionInfoWindowOpen = false;
    static bool headersWindowOpen = false;
    static bool processIdWindowOpen = false;
    static bool checkSumWindowOpen = false;
    bool showEntropyHistogram = false;
    static bool disassemblyWindowOpen = false;

    static bool show_metrics = false;

    std::string filePathInput;
    std::vector<int> histogram;


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

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            ImGui::Begin("PE Tool");

            if (ImGui::Button("Open File"))
            {
                openFile();
                filePathInput = filePath;
            }

            char filePathInputBuffer[256];
            strcpy(filePathInputBuffer, filePathInput.c_str());
            ImGui::InputText("File Path", filePathInputBuffer, sizeof(filePathInputBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
            filePathInput = filePathInputBuffer;

            if (ImGui::BeginMenu("Extract"))
            {
                if (ImGui::MenuItem("Import Table"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    std::stringstream output;
                    std::streambuf* old_cout = std::cout.rdbuf();
                    std::cout.rdbuf(output.rdbuf());
                    PE::extractImportTable(fileData);
                    importTableOutput = output.str();
                    std::cout.rdbuf(old_cout);
                }
                if (ImGui::MenuItem("Export Table"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    std::stringstream output;
                    std::streambuf* old_cout = std::cout.rdbuf();
                    std::cout.rdbuf(output.rdbuf());
                    PE::extractExportTable(fileData);
                    exportTableOutput = output.str();
                    std::cout.rdbuf(old_cout);
                }
                if (ImGui::MenuItem("Resources"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    std::stringstream output;
                    std::streambuf* old_cout = std::cout.rdbuf();
                    std::cout.rdbuf(output.rdbuf());
                    PE::extractResources(fileData);
                    resourcesOutput = output.str();
                    std::cout.rdbuf(old_cout);
                }
                if (ImGui::MenuItem("Section Info"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    std::stringstream output;
                    std::streambuf* old_cout = std::cout.rdbuf();
                    std::cout.rdbuf(output.rdbuf());
                    PE::extractSectionInfo(fileData);
                    sectionInfoOutput = output.str();
                    std::cout.rdbuf(old_cout);
                }
                if (ImGui::MenuItem("Headers"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    std::stringstream output;
                    std::streambuf* old_cout = std::cout.rdbuf();
                    std::cout.rdbuf(output.rdbuf());
                    PE::parseHeaders(fileData);
                    headersOutput = output.str();
                    std::cout.rdbuf(old_cout);
                }
                ImGui::EndMenu();
            }
            else if (ImGui::BeginMenu("Process Id"))
            {
                if (ImGui::MenuItem("Get Procees Id"))
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
                    processIdOutput = output.str();
                }
                ImGui::EndMenu();
            }
            else if (ImGui::BeginMenu("Metrics"))
            {
                if (ImGui::MenuItem("Show Metrics"))
                {
                    show_metrics = true;
                }
                ImGui::EndMenu();
            }
            else if (ImGui::BeginMenu("Utils"))
            {
                if (ImGui::MenuItem("Calculate Checksum"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    std::stringstream output;
                    std::streambuf* old_cout = std::cout.rdbuf();
                    std::cout.rdbuf(output.rdbuf());
                    Utils unt;
                    checksumOutput = unt.calculateChecksum(fileData);
                    std::cout.rdbuf(old_cout);
                }
                ImGui::EndMenu();
            }
            else if (ImGui::BeginMenu("Entropy"))
            {
                if (ImGui::MenuItem("Entropy Histogram"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    std::stringstream output;
                    std::streambuf* old_cout = std::cout.rdbuf();
                    std::cout.rdbuf(output.rdbuf());
                    Entropy ent;
                    histogram = ent.createHistogram(fileData);
                    std::cout.rdbuf(old_cout);

                    for (int i = 0; i < histogram.size(); i++)
                    {
                        std::vector<uint8_t> data = FileIO::readFile(filePathInput);
                        std::stringstream output;
                        std::streambuf* old_cout = std::cout.rdbuf();
                        std::cout.rdbuf(output.rdbuf());
                        ent.printEntropy(data, i, 1);
                        std::cout.rdbuf(old_cout);
                        entropyOutput.push_back(output.str());
                    }
                }
                ImGui::EndMenu();
            }
            else if (ImGui::BeginMenu("Disassembler"))
            {
                if (ImGui::MenuItem("Disassemble"))
                {
                    std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                    std::stringstream output;
                    std::streambuf* old_cout = std::cout.rdbuf();
                    std::cout.rdbuf(output.rdbuf());
                    Disassembler dis;
                    std::tuple exe = dis.getExecutable(filePathInput);
                    dis.printDisassembly(std::get<0>(exe), std::get<1>(exe), std::get<2>(exe));
                    std::cout.rdbuf(old_cout);
                    disassemblyOutput.push_back(output.str());
                }
                ImGui::EndMenu();
            }

            if (show_metrics)
            {
                ImGui::ShowMetricsWindow(&show_metrics);
            }

            ImGui::End();

            if (!importTableOutput.empty())
            {
                importTableWindowOpen = true;
                ImGui::Begin("Import Table", &importTableWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s", importTableOutput.c_str());
                if (!importTableWindowOpen)
                    importTableOutput.clear();
                ImGui::End();
            }

            if (!exportTableOutput.empty())
            {
                exportTableWindowOpen = true;
                ImGui::Begin("Export Table", &exportTableWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s", exportTableOutput.c_str());
                if (!exportTableWindowOpen)
                    exportTableOutput.clear();
                ImGui::End();
            }

            if (!resourcesOutput.empty())
            {
                resourcesWindowOpen = true;
                ImGui::Begin("Resources", &resourcesWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s", resourcesOutput.c_str());
                if (!resourcesWindowOpen)
                    resourcesOutput.clear();
                ImGui::End();
            }

            if (!sectionInfoOutput.empty())
            {
                sectionInfoWindowOpen = true;
                ImGui::Begin("Section Info", &sectionInfoWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s", sectionInfoOutput.c_str());
                if (!sectionInfoWindowOpen)
                    sectionInfoOutput.clear();
                ImGui::End();
            }

            if (!headersOutput.empty())
            {
                headersWindowOpen = true;
                ImGui::Begin("Headers", &headersWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s", headersOutput.c_str());
                if (!headersWindowOpen)
                    headersOutput.clear();
                ImGui::End();
            }

            if (!processIdOutput.empty())
            {
                processIdWindowOpen = true;
                ImGui::Begin("Process Id", &processIdWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s", processIdOutput.c_str());
                if (!processIdWindowOpen)
                    processIdOutput.clear();
                ImGui::End();
            }

            if (!checksumOutput.empty())
            {
                checkSumWindowOpen = true;
                ImGui::Begin("Bytes", &checkSumWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s", checksumOutput.c_str());
                if (!checkSumWindowOpen)
                    checksumOutput.clear();
                ImGui::End();
            }

            if (!histogram.empty())
            {
                showEntropyHistogram = true;
                ImGui::Begin("Entropy Histogram", &showEntropyHistogram, ImGuiWindowFlags_HorizontalScrollbar);
                std::vector<float> histogram_float(histogram.begin(), histogram.end());
                ImGui::PlotHistogram("Histogram", histogram_float.data(), histogram_float.size(), 0, nullptr, 0.0f, FLT_MAX, ImVec2(0, 200));
                for (const auto& output : entropyOutput)
                {
                    ImGui::Text("%s", output.c_str());
                }
                if (!showEntropyHistogram)
                    histogram.clear();
                ImGui::End();
            }

            if (!disassemblyOutput.empty())
            {
                disassemblyWindowOpen = true;
                ImGui::Begin("Disassembly", &disassemblyWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                for (const auto& output : disassemblyOutput)
                {
                    ImGui::Text("%s", output.c_str());
                }
                if (!disassemblyWindowOpen)
                    disassemblyOutput.clear();
                ImGui::End();
            }


            if (importTableOutput.empty() && !ImGui::IsItemHovered())
                importTableWindowOpen = false;

            if (exportTableOutput.empty() && !ImGui::IsItemHovered())
                exportTableWindowOpen = false;

            if (resourcesOutput.empty() && !ImGui::IsItemHovered())
                resourcesWindowOpen = false;

            if (sectionInfoOutput.empty() && !ImGui::IsItemHovered())
                sectionInfoWindowOpen = false;

            if (headersOutput.empty() && !ImGui::IsItemHovered())
                headersWindowOpen = false;

            if (processIdOutput.empty() && !ImGui::IsItemHovered())
                processIdWindowOpen = false;

            if (checksumOutput.empty() && !ImGui::IsItemHovered())
                checkSumWindowOpen = false;

            if (histogram.empty() && !ImGui::IsItemHovered())
                showEntropyHistogram = false;

            if (disassemblyOutput.empty() && !ImGui::IsItemHovered())
                disassemblyWindowOpen = false;

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
        runGUI();
    }
    else
    {
        runCLI(argc, argv);
    }
    return 0;
}
#endif