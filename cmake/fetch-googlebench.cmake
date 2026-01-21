message(STATUS "Third-party: load google benchmark...")
include(FetchContent)

set(BENCHMARK_ENABLE_TESTING FALSE)
set(BENCHMARK_ENABLE_INSTALL FALSE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_USE_BUNDLED_GTEST OFF CACHE BOOL "" FORCE)

FetchContent_Declare(googlebench
    GIT_REPOSITORY      https://github.com/google/benchmark.git
    GIT_TAG             v1.9.1
)

FetchContent_MakeAvailable(googlebench)
