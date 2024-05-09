#include "./include/PE.h"
#include "./include/FileIO.h"
#include "./include/Untils.h"
#include "./include/Injector.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>

#if defined(_WIN32)
#include <Windows.h>
#include <string>
#include <shobjidl.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

using namespace PeInternals;
using namespace FileIoInternals;
using namespace UntilsInternals;
using namespace DllInjector;

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64                  FenceValue;
};

// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT                         g_frameIndex = 0;

static int const                    NUM_BACK_BUFFERS = 3;
static ID3D12Device*                g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap*        g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap*        g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue*          g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList*   g_pd3dCommandList = nullptr;
static ID3D12Fence*                 g_fence = nullptr;
static HANDLE                       g_fenceEvent = nullptr;
static UINT64                       g_fenceLastSignaledValue = 0;
static IDXGISwapChain3*             g_pSwapChain = nullptr;
static HANDLE                       g_hSwapChainWaitableObject = nullptr;
static ID3D12Resource*              g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::string sSelectedFile;
std::string sFilePath;
bool openFile()
{
    //  CREATE FILE OBJECT INSTANCE
    HRESULT f_SysHr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(f_SysHr))
        return FALSE;

    // CREATE FileOpenDialog OBJECT
    IFileOpenDialog* f_FileSystem;
    f_SysHr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&f_FileSystem));
    if (FAILED(f_SysHr)) {
        CoUninitialize();
        return FALSE;
    }

    //  SHOW OPEN FILE DIALOG WINDOW
    f_SysHr = f_FileSystem->Show(NULL);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  RETRIEVE FILE NAME FROM THE SELECTED ITEM
    IShellItem* f_Files;
    f_SysHr = f_FileSystem->GetResult(&f_Files);
    if (FAILED(f_SysHr)) {
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  STORE AND CONVERT THE FILE NAME
    PWSTR f_Path;
    f_SysHr = f_Files->GetDisplayName(SIGDN_FILESYSPATH, &f_Path);
    if (FAILED(f_SysHr)) {
        f_Files->Release();
        f_FileSystem->Release();
        CoUninitialize();
        return FALSE;
    }

    //  FORMAT AND STORE THE FILE PATH
    std::wstring path(f_Path);
    std::string c(path.begin(), path.end());
    sFilePath = c;

    //  FORMAT STRING FOR EXECUTABLE NAME
    const size_t slash = sFilePath.find_last_of("/\\");
    sSelectedFile = sFilePath.substr(slash + 1);

    //  SUCCESS, CLEAN UP
    CoTaskMemFree(f_Path);
    f_Files->Release();
    f_FileSystem->Release();
    CoUninitialize();
    return TRUE;
}

int main(int, char**)
{
    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"PETOOL", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"PETOOL", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    
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
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Begin("PE Tool");

        if (ImGui::Button("Select File"))
        {
            openFile();
            filePathInput = sFilePath;
        }

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


        ImGui::Render();

        FrameContext* frameCtx = WaitForNextFrameResources();
        UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
        frameCtx->CommandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource   = g_mainRenderTargetResource[backBufferIdx];
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
        g_pd3dCommandList->ResourceBarrier(1, &barrier);

        // Render Dear ImGui graphics
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
        g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
        g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
        g_pd3dCommandList->ResourceBarrier(1, &barrier);
        g_pd3dCommandList->Close();

        g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        UINT64 fenceValue = g_fenceLastSignaledValue + 1;
        g_pd3dCommandQueue->Signal(g_fence, fenceValue);
        g_fenceLastSignaledValue = fenceValue;
        frameCtx->FenceValue = fenceValue;
    }

    WaitForLastSubmittedFrame();

    // Cleanup
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = NUM_BACK_BUFFERS;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
    ID3D12Debug* pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != nullptr)
    {
        ID3D12InfoQueue* pInfoQueue = nullptr;
        g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        pInfoQueue->Release();
        pdx12Debug->Release();
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            g_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
        g_pd3dCommandList->Close() != S_OK)
        return false;

    if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
        return false;

    g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (g_fenceEvent == nullptr)
        return false;

    {
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain1* swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->SetFullscreenState(false, nullptr); g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_hSwapChainWaitableObject != nullptr) { CloseHandle(g_hSwapChainWaitableObject); }
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = nullptr; }
    if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = nullptr; }
    if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
    if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
    if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
    if (g_fence) { g_fence->Release(); g_fence = nullptr; }
    if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
    IDXGIDebug1* pDebug = nullptr;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

void CreateRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_mainRenderTargetDescriptor[i]);
        g_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = nullptr; }
}

void WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

FrameContext* WaitForNextFrameResources()
{
    UINT nextFrameIndex = g_frameIndex + 1;
    g_frameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0) // means no fence was signaled
    {
        frameCtx->FenceValue = 0;
        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        waitableObjects[1] = g_fenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            WaitForLastSubmittedFrame();
            CleanupRenderTarget();
            HRESULT result = g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
            assert(SUCCEEDED(result) && "Failed to resize swapchain.");
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
#endif

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