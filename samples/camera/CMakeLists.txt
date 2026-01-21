set(ALGORITHM_NAME camera)
set(APP_NAME ${PROJECT_NAME}-${ALGORITHM_NAME})

if(EMSCRIPTEN)

    add_executable(${APP_NAME} src/web_endpoint.cpp src/camera_sample.cpp)
    
    set_target_properties(${APP_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${APP_NAME} PUBLIC 
        -O3                                                        # optimization, if used, also set --memory-init-file 0    
        --preload-file resources                                   # specify a resources folder for packages in a bundle on the side
        -sUSE_GLFW=3                                               # raylib requires glfw3 library (context/input management) 
        -sALLOW_MEMORY_GROWTH=1                                    # to allow memory resizing -> WARNING: Audio buffers could FAIL!
        -lembind                                                   # for EMSCRIPTEN_BINDINGS
        --shell-file "${CMAKE_CURRENT_SOURCE_DIR}/html/shell.html"
    )

    set(DEMO_DIR ${PROJECT_SOURCE_DIR}/docs/html/demo)

    # workaround for ci/cd build. Without copy resource folder, --preload-file wouldn't work. The strangest thing is that everything works localy on Windows.
    add_custom_command(
        TARGET ${APP_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    )
    
    add_custom_command(
        TARGET ${APP_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DEMO_DIR}/${ALGORITHM_NAME}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/samples/templates/root ${DEMO_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/${APP_NAME}.js   ${DEMO_DIR}/${ALGORITHM_NAME}/${APP_NAME}.js
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/${APP_NAME}.wasm ${DEMO_DIR}/${ALGORITHM_NAME}/${APP_NAME}.wasm
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/${APP_NAME}.data ${DEMO_DIR}/${ALGORITHM_NAME}/${APP_NAME}.data
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_BINARY_DIR}/${APP_NAME}.html ${DEMO_DIR}/${ALGORITHM_NAME}/index.html
    )

else()

    add_executable(${APP_NAME} src/desktop_endpoint.cpp src/camera_sample.cpp)

endif()

target_link_libraries(${APP_NAME} PUBLIC ${PROJECT_NAME}-core)
target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
