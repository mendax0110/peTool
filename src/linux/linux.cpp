#include "../../include/FILEIO/FileIO.h"
#include "../../include/FILEIO/Utils.h"
#include "../../include/CORE/PE.h"
#include "../../include/CORE/Disassembler.h"
#include "../../include/CORE/Injector.h"
#include "../../include/VIEW/Entropy.h"
#include "../../include/CLI/CLI.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>

#if defined(__linux__)
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

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
    // TODO: Implement file dialog for Linux
    return true;
}

int runGUI()
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
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
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
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