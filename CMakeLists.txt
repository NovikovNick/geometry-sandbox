cmake_minimum_required(VERSION 3.25)
project ("mesh-cutter"
    VERSION 0.0.0
    DESCRIPTION "A lightweight C++ library for cutting arbitrary meshes"
    LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(WIN32)
    include(fetch-raylib)
    add_subdirectory ("src")
elseif(EMSCRIPTEN)
    include(fetch-raylib)
    add_subdirectory ("src")
else()
    message(FATAL_ERROR "Supports only windows or web platforms.")
endif()

message(STATUS "")
message(STATUS "${PROJECT_NAME} configuration summary:")
message(STATUS "")
message(STATUS "  Compiler ...........................: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  CMake build type ...................: ${CMAKE_BUILD_TYPE}")
message(STATUS "  CMAKE_CXX_FLAGS ....................: ${CMAKE_CXX_FLAGS}")
message(STATUS "  CMAKE_EXE_LINKER_FLAGS .............: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "  LINK_FLAGS .........................: ${LINK_FLAGS}")
message(STATUS "")
