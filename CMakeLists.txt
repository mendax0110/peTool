cmake_minimum_required(VERSION 3.5)
project(peTool)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(INCLUDE_DIR
        "${CMAKE_SOURCE_DIR}/include/CLI"
        "${CMAKE_SOURCE_DIR}/include/CORE"
        "${CMAKE_SOURCE_DIR}/include/FILEIO"
        "${CMAKE_SOURCE_DIR}/include/MANMON"
        "${CMAKE_SOURCE_DIR}/include/VIEW"
        "${CMAKE_SOURCE_DIR}/include/LIBMAN"
        "${CMAKE_SOURCE_DIR}/include/UTILS"
)
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")

include_directories(${INCLUDE_DIR})
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)

if (APPLE)
    find_package(Doxygen REQUIRED)
    find_package(SDL2 REQUIRED)
    find_library(METAL_LIBRARY Metal)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    find_library(APPKIT_LIBRARY AppKit)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(GL_LIBRARY OpenGL)

    include_directories(${SDL2_INCLUDE_DIRS})

    set(DIRECTX_LIBRARIES ${METAL_LIBRARY} ${COCOA_LIBRARY} ${QUARTZCORE_LIBRARY} ${APPKIT_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY} ${GL_LIBRARY})
elseif (WIN32)
    set(DIRECTX_LIBRARIES d3d12 dxgi d3dcompiler)
elseif (LINUX)
    find_package(SDL2 REQUIRED)
    find_package(OpenGL REQUIRED)

    set(DIRECTX_LIBRARIES ${SDL2_LIBRARIES} OpenGL::GL)
endif()

file(GLOB IMGUI_SOURCES "${IMGUI_DIR}/*.cpp" "${IMGUI_DIR}/*.h")

if (WIN32)
    set(IMGUI_BACKEND_SOURCES "${IMGUI_DIR}/backends/imgui_impl_dx12.cpp" "${IMGUI_DIR}/backends/imgui_impl_win32.cpp")
elseif (APPLE)
    set(IMGUI_BACKEND_SOURCES "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp" "${IMGUI_DIR}/backends/imgui_impl_metal.mm")
elseif (LINUX)
    set(IMGUI_BACKEND_SOURCES "${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp" "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp")
endif()

if (APPLE)
    file(GLOB_RECURSE SOURCES "src/*.cpp" "src/macos/*.mm" "src/linux/*.cpp" "src/windows/*.cpp")
else()
    file(GLOB_RECURSE SOURCES "src/*.cpp")
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} ${IMGUI_BACKEND_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR} ${IMGUI_DIR} ${IMGUI_DIR}/backends)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${DIRECTX_LIBRARIES})
elseif (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${DIRECTX_LIBRARIES} ${SDL2_LIBRARIES})
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE "-fsanitize=undefined")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-fsanitize=undefined")
    target_compile_options(${PROJECT_NAME} PRIVATE "-fno-omit-frame-pointer")
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

    add_custom_target(EnsureImGuiDockingBranch ALL DEPENDS ${IMGUI_DIR}/.git)
endif()

if (APPLE)
    find_package(Doxygen REQUIRED)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/docs/Doxygen)

        get_target_property(DOXYGEN_EXECUTABLE Doxygen::doxygen IMPORTED_LOCATION)

        add_custom_target(doc_doxygen ALL
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Generating API documentation with Doxygen"
                VERBATIM)

        add_custom_command(TARGET doc_doxygen POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_BINARY_DIR}/docs/Doxygen
                ${CMAKE_SOURCE_DIR}/docs/Doxygen
                COMMENT "Copying generated documentation to the target directory")
    endif()
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
