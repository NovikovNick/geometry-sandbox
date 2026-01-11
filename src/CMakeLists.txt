set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${PROJECT_NAME} main.cpp)

find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        OpenGL::GL
        SDL3::SDL3
        imgui
)

message(SDL3_BINARY_DIR ${SDL3_BINARY_DIR})

add_custom_command(
    TARGET ${PROJECT_NAME}
    COMMENT "Copy SDL3 DLL"
    PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/SDL3.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
    VERBATIM)