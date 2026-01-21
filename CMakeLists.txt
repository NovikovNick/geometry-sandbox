cmake_minimum_required(VERSION 3.25)
project ("geometry-sandbox"
    VERSION 0.1.0
    DESCRIPTION "Yet another geometry sandbox project"
    LANGUAGES CXX)

option(GSAND_BUILD_SAMPLES       "Build samples"                          ON)
option(GSAND_ENABLE_TESTS        "Enable unit tests"                      ON)
option(GSAND_BUILD_DOCS          "Build docs"                             ON)
option(GSAND_USE_CLANG_TIDY      "Enable static analysis with clang-tidy" OFF)
option(GSAND_ENABLE_BENCHMARKS   "Enable unit benchmarks"                 OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

include(cmake/fetch-raylib.cmake)
include(cmake/fetch-rlimgui.cmake)
include(cmake/fetch-imgui.cmake)
include(cmake/fetch-eigen.cmake)
add_subdirectory ("third_party")

if (GSAND_USE_CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy")
    if(MSVC AND NOT (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang"))
        set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};--extra-arg=/EHsc") # fix https://gitlab.kitware.com/cmake/cmake/-/work_items/20512
    endif()
endif()

add_subdirectory ("src")

if (GSAND_ENABLE_TESTS)
    enable_testing()
    include(cmake/fetch-googletest.cmake)
    add_subdirectory(tests)
endif()

if (GSAND_ENABLE_BENCHMARKS)
    include(cmake/fetch-googlebench.cmake)
    add_subdirectory(benchmarks)
endif()

if (GSAND_BUILD_DOCS)
    message(STATUS "Configuring Doxygen docs")
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        message("Doxygen build started")

        set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

        add_custom_target(docs ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else()
        message(WARNING "Doxygen not found - documentation will not be generated")
        message(WARNING "Install Doxygen: https://www.doxygen.nl/download.html")
    endif()
endif()

if (GSAND_BUILD_SAMPLES)
    add_subdirectory ("samples")
endif()

message(STATUS "")
message(STATUS "${PROJECT_NAME} configuration summary:")
message(STATUS "")
message(STATUS "  Compiler .....................................: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  CMake build type .............................: ${CMAKE_BUILD_TYPE}")
message(STATUS "  CMAKE_CXX_FLAGS ..............................: ${CMAKE_CXX_FLAGS}")
message(STATUS "  CMAKE_EXE_LINKER_FLAGS .......................: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "  LINK_FLAGS ...................................: ${LINK_FLAGS}")
message(STATUS "  Enable static analysis with clang-tidy .......: ${GSAND_USE_CLANG_TIDY}")
message(STATUS "  Enable unit tests ............................: ${GSAND_ENABLE_TESTS}")
message(STATUS "  Enable unit benchmarks .......................: ${GSAND_ENABLE_BENCHMARKS}")
message(STATUS "  Build docs ...................................: ${GSAND_BUILD_DOCS}")
message(STATUS "  Build samples ................................: ${GSAND_BUILD_SAMPLES}")
message(STATUS "")
