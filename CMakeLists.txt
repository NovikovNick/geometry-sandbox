cmake_minimum_required(VERSION 3.28)
project ("mesh-cutter"
    VERSION 0.0.0
    DESCRIPTION "A lightweight C++ library for cutting arbitrary meshes"
    LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(WIN32)
    include(fetch-sdl3)
    include(fetch-imgui)

    add_subdirectory ("src")
else()
    message(FATAL_ERROR "Support only windows version.")
endif()

message(STATUS "")
message(STATUS "Mesh cutter configuration summary:")
message(STATUS "")
message(STATUS "  Compiler ...........................: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  CMake build type ...................: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
