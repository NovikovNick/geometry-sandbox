message(STATUS "Third-party: load imgui...")

include(FetchContent)

FetchContent_Declare(
    ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.1
)
FetchContent_MakeAvailable(imgui)
# Add library files
add_library(
    imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
)

# target_compile_definitions(imgui PRIVATE  IMGUI_DISABLE_DEFAULT_FONT)

# Set it's include directory
target_include_directories(imgui INTERFACE ${imgui_SOURCE_DIR})
