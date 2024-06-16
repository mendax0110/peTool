cmake_minimum_required(VERSION 3.5)
project(peTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIR
        "${CMAKE_SOURCE_DIR}/src/CLI"
        "${CMAKE_SOURCE_DIR}/src/CORE"
        "${CMAKE_SOURCE_DIR}/src/FILEIO"
        "${CMAKE_SOURCE_DIR}/src/MANMON"
        "${CMAKE_SOURCE_DIR}/src/VIEW")

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

include_directories(${INCLUDE_DIR})
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)

if (WIN32)
    find_package(Doxygen REQUIRED)
    get_target_property(DOXYGEN_EXECUTABLE Doxygen::doxygen IMPORTED_LOCATION)
    set(DOXYGEN_DIRS
            ${CMAKE_SOURCE_DIR}/src
            ${CMAKE_SOURCE_DIR}/include
    )
    string(REPLACE ";" " " DOXYGEN_DIRS "${DOXYGEN_DIRS}")
endif()

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
    set(IMGUI_BACKEND_SOURCES "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp" "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")
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
    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)

    set(DirectX_LIBRARIES SDL2 OpenGL::GL)
endif()

if (APPLE)
    file(GLOB_RECURSE SOURCES "src/*.cpp" "src/macos/*.mm" "src/linux/*.cpp" "src/windows/*.cpp")
else()
    file(GLOB_RECURSE SOURCES "src/*.cpp")
endif()

if (APPLE)
    add_custom_command(
        OUTPUT ${IMGUI_DIR}/.git
        COMMAND git -C ${IMGUI_DIR} fetch --all
        COMMAND git -C ${IMGUI_DIR} checkout docking || git -C ${IMGUI_DIR} checkout -b docking origin/docking
        COMMAND git -C ${IMGUI_DIR} pull origin docking
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Ensuring ImGui submodule is on the docking branch"
    )

    add_custom_target(EnsureImGuiDockingBranch ALL
        DEPENDS ${IMGUI_DIR}/.git
    )
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})

if (APPLE)
    add_dependencies(${PROJECT_NAME} EnsureImGuiDockingBranch)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${IMGUI_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${IMGUI_DIR}/backends)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${DirectX_LIBRARIES})
endif()

if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${DirectX_LIBRARIES} ${SDL2_LIBRARY})
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE "-fsanitize=undefined")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-fsanitize=undefined")
    target_compile_options(${PROJECT_NAME} PRIVATE "-fno-omit-frame-pointer")
endif()

if (APPLE)
    file(WRITE "${CMAKE_BINARY_DIR}/run_leaks.sh"
            "#!/bin/bash\n"
            "./cmake-build-debug/peTool\n"
            "leaks -atExit -- ./peTool\n"
    )
    execute_process(COMMAND chmod +x "${CMAKE_BINARY_DIR}/run_leaks.sh")
elseif (LINUX)
    file(WRITE "${CMAKE_BINARY_DIR}/run_leaks.sh"
            "#!/bin/bash\n"
            "./cmake-build-debug/peTool\n"
            "valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./peTool\n"
    )
    execute_process(COMMAND chmod +x "${CMAKE_BINARY_DIR}/run_leaks.sh")
elseif (WIN32)
    file(WRITE "${CMAKE_BINARY_DIR}/run_leaks.bat"
            "./build/peTool.exe\n"
    )
endif()

if (WIN32)
    find_package(Doxygen REQUIRED)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/docs/Doxygen)

        if (WIN32)
            get_target_property(DOXYGEN_EXECUTABLE Doxygen::doxygen IMPORTED_LOCATION)
        endif()

        add_custom_target(doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)

        add_custom_command(TARGET doc_doxygen POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_BINARY_DIR}/docs/Doxygen
            ${CMAKE_SOURCE_DIR}/docs/Doxygen
            COMMENT "Copying generated documentation to the target directory"
        )
    endif()
endif()
