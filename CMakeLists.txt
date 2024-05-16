cmake_minimum_required(VERSION 3.5)
project(peTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/include")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

include_directories(${INCLUDE_DIR})

include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)

if (APPLE)
    include_directories("/opt/homebrew/Cellar/sdl2/2.30.3/include/SDL2")
endif()

file(GLOB IMGUI_SOURCES "${IMGUI_DIR}/*.cpp" "${IMGUI_DIR}/*.h")

if (WIN32)
    set(IMGUI_BACKEND_SOURCES "${IMGUI_DIR}/backends/imgui_impl_dx12.cpp" "${IMGUI_DIR}/backends/imgui_impl_win32.cpp")
endif()

if (APPLE)
    set(IMGUI_BACKEND_SOURCES "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp" "${IMGUI_DIR}/backends/imgui_impl_metal.mm")
endif()

if (LINUX)
    set(IMGUI_BACKEND_SOURCES "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp" "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")
endif()


if (WIN32)
    set(DirectX_LIBRARIES d3d12 dxgi d3dcompiler)
endif()

if (APPLE)
    find_library(METAL_LIBRARY Metal)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(GL_LIBRARY OpenGL)

    set(DirectX_LIBRARIES ${METAL_LIBRARY} ${COCOA_LIBRARY} ${QUARTZCORE_LIBRARY} ${APPKIT_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY} ${GL_LIBRARY})

    find_library(SDL2_LIBRARY SDL2)
endif()

if (LINUX)
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)

    set(DirectX_LIBRARIES glfw OpenGL::GL)
endif()

file(GLOB SOURCES "src/*.cpp")

if (APPLE)
    # search for .cpp and .mm files
    file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.mm")
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${IMGUI_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR}/backends)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${DirectX_LIBRARIES})
endif()

if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${DirectX_LIBRARIES} ${SDL2_LIBRARY})
endif()
