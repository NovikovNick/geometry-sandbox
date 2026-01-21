message(STATUS "Third-party: load raylib...")

include(FetchContent)

if(EMSCRIPTEN)
    set(PLATFORM "Web" CACHE STRING "" FORCE)
    set(GRAPHICS "GRAPHICS_API_OPENGL_ES2" CACHE STRING "" FORCE)
else()
    set(PLATFORM "Desktop" CACHE STRING "" FORCE)
endif()

set(RAYLIB_SHARED OFF CACHE BOOL "" FORCE)
set(RAYLIB_STATIC ON CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_UTILS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)
FetchContent_MakeAvailable(raylib)
target_compile_options(raylib PRIVATE -w)

