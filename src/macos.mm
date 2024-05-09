#include "./include/PE.h"
#include "./include/FileIO.h"
#include "./include/Untils.h"
#include "./include/Injector.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>

#if defined(__APPLE__)
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_metal.h"
#include <stdio.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**)
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

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Create window with graphics context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "PETOOL", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    id <MTLDevice> device = MTLCreateSystemDefaultDevice();
    id <MTLCommandQueue> commandQueue = [device newCommandQueue];

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplMetal_Init(device);

    NSWindow *nswin = glfwGetCocoaWindow(window);
    CAMetalLayer *layer = [CAMetalLayer layer];
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;

    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor new];

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

    // Define boolean variables to track window states
    bool importTableWindowOpen = false;
    bool exportTableWindowOpen = false;
    bool resourcesWindowOpen = false;
    bool sectionInfoWindowOpen = false;
    bool headersWindowOpen = false;

    std::string filePathInput;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        @autoreleasepool
        {
            glfwPollEvents();

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            layer.drawableSize = CGSizeMake(width, height);
            id<CAMetalDrawable> drawable = [layer nextDrawable];

            id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
            renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0] * clear_color[3], clear_color[1] * clear_color[3], clear_color[2] * clear_color[3], clear_color[3]);
            renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
            [renderEncoder pushDebugGroup:@"PETOOL"];

            // Start the Dear ImGui frame
            ImGui_ImplMetal_NewFrame(renderPassDescriptor);
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            char filePathInputBuffer[256];
            strcpy_s(filePathInputBuffer, filePathInput.c_str());
            ImGui::InputText("File Path", filePathInputBuffer, sizeof(filePathInputBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
            filePathInput = filePathInputBuffer;

            if (ImGui::Button("Extract Import Table"))
            {
                std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                std::stringstream output;
                std::streambuf* old_cout = std::cout.rdbuf();
                std::cout.rdbuf(output.rdbuf());
                PE::extractImportTable(fileData);
                importTableOutput = output.str();
                std::cout.rdbuf(old_cout);
            }

            if (ImGui::Button("Extract Export Table"))
            {
                std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                std::stringstream output;
                std::streambuf* old_cout = std::cout.rdbuf();
                std::cout.rdbuf(output.rdbuf());
                PE::extractExportTable(fileData);
                exportTableOutput = output.str();
                std::cout.rdbuf(old_cout);
            }

            if (ImGui::Button("Extract Resources"))
            {
                std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                std::stringstream output;
                std::streambuf* old_cout = std::cout.rdbuf();
                std::cout.rdbuf(output.rdbuf());
                PE::extractResources(fileData);
                resourcesOutput = output.str();
                std::cout.rdbuf(old_cout);
            }

            if (ImGui::Button("Extract Section Info"))
            {
                std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                std::stringstream output;
                std::streambuf* old_cout = std::cout.rdbuf();
                std::cout.rdbuf(output.rdbuf());
                PE::extractSectionInfo(fileData);
                sectionInfoOutput = output.str();
                std::cout.rdbuf(old_cout);
            }

            if (ImGui::Button("Parse Headers"))
            {
                std::vector<uint8_t> fileData = FileIO::readFile(filePathInput);
                std::stringstream output;
                std::streambuf* old_cout = std::cout.rdbuf();
                std::cout.rdbuf(output.rdbuf());
                PE::parseHeaders(fileData);
                headersOutput = output.str();
                std::cout.rdbuf(old_cout);
            }

            ImGui::End();

            if (!importTableOutput.empty())
            {
                importTableWindowOpen = true;
                ImGui::Begin("Import Table", &importTableWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s",importTableOutput.c_str());
                if (!importTableWindowOpen)
                    importTableOutput.clear();
                ImGui::End();
            }

            if (!exportTableOutput.empty())
            {
                exportTableWindowOpen = true;
                ImGui::Begin("Export Table", &exportTableWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s",exportTableOutput.c_str());
                if (!exportTableWindowOpen)
                    exportTableOutput.clear();
                ImGui::End();
            }

            if (!resourcesOutput.empty())
            {
                resourcesWindowOpen = true;
                ImGui::Begin("Resources", &resourcesWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s",resourcesOutput.c_str());
                if (!resourcesWindowOpen)
                    resourcesOutput.clear();
                ImGui::End();
            }

            if (!sectionInfoOutput.empty())
            {
                sectionInfoWindowOpen = true;
                ImGui::Begin("Section Info", &sectionInfoWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s",sectionInfoOutput.c_str());
                if (!sectionInfoWindowOpen)
                    sectionInfoOutput.clear();
                ImGui::End();
            }

            if (!headersOutput.empty())
            {
                headersWindowOpen = true;
                ImGui::Begin("Headers", &headersWindowOpen, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Text("%s",headersOutput.c_str());
                if (!headersWindowOpen)
                    headersOutput.clear();
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
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
#endif