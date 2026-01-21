message(STATUS "Third-party: load googletest...")
include(FetchContent)

# Add GTest
FetchContent_Declare(
  GTest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG v1.15.2
)

if(WIN32)
    # reference: build/_deps/googletest-src/googletest/README.md
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(GTest)

# If tests files are compiled using ASAN, then gtest/gmock must also be compiled using ASAN.
if(MSVC)
    set(ASAN_FLAG $<$<CONFIG:Debug>:/fsanitize=address>)
else()
    # Need to find way to setup ASAN for clang in DEBUG and tests 
    # set(ASAN_FLAG  $<$<CONFIG:Debug>:-fsanitize=address>)
endif()

target_compile_options(gtest PUBLIC ${ASAN_FLAG}) 
target_compile_options(gtest_main PUBLIC ${ASAN_FLAG}) 
target_compile_options(gmock PUBLIC ${ASAN_FLAG}) 
target_compile_options(gmock_main  PUBLIC ${ASAN_FLAG}) 
