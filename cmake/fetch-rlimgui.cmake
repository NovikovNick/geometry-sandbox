message(STATUS "Third-party: load rlimgui...")

include(FetchContent)

FetchContent_Declare(
    rlImGui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui
    # Git Tag can be a release or a commit (like this)
    GIT_TAG dc7f97679a024eee8f5f009e77cc311748200415 
)
FetchContent_MakeAvailable(rlImGui)
  
# For libraries not compiled with CMake, it's necessary to manually add the source files and set the include directory
add_library(rlimgui STATIC ${rlimgui_SOURCE_DIR}/rlImGui.cpp)
target_compile_options(rlimgui PRIVATE -w)
target_include_directories(rlimgui INTERFACE ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PRIVATE imgui raylib) # Tells the compiler that one library depends on other


