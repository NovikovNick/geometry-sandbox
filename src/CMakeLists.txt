add_library(${PROJECT_NAME}-core STATIC
    "core/application.cpp"
    
    "core/camera_controller_service.cpp"
    "core/camera_service.cpp"
    "core/scene_service.cpp"
    "core/json_service.cpp"
    
    "core/resource_manager.cpp"
    "core/window_manager.cpp"
    "core/input_manager_raylib.cpp"

    "render/facade.cpp"
    "render/mesh_instanced_draw_service.cpp"
    "render/frustum_draw_service.cpp"
    "render/viewport_manager.cpp"
    "render/low_level_service.cpp"
    
    "animation/service.cpp"
    "animation/manager.cpp"
    "animation/manager_apply.cpp"
    "animation/manager_build.cpp"
    "animation/manager_integrate.cpp"
    "animation/player_manager.cpp"
    "animation/handle.cpp"
    
    "ui/state_manager.cpp"
    "ui/slider.cpp"
    "ui/service.cpp"
    "ui/manager.cpp"
    
    "interaction/service.cpp"
    "interaction/translate_gizmo_render_service.cpp"
    "interaction/translate_gizmo_update_service.cpp"

    "feature/camera_idle_rotation_manager.cpp"
    "feature/ortho_projection_view_manager.cpp"
)

target_link_libraries(${PROJECT_NAME}-core
    PRIVATE
        rlimgui 
    PUBLIC
        imgui
        raylib
        dependencies::icons
        dependencies::di
        dependencies::eigen
        dependencies::entt
        dependencies::easing
        dependencies::json  
)

target_compile_features(${PROJECT_NAME}-core PUBLIC cxx_std_20)

if(MSVC)         
    target_compile_options(${PROJECT_NAME}-core PUBLIC $<IF:$<CONFIG:Release>,/O2,/Od>)       # optimizations
    target_compile_options(${PROJECT_NAME}-core PUBLIC $<$<CONFIG:Release>:/DNDEBUG>)         # disable asserts
    target_compile_options(${PROJECT_NAME}-core PRIVATE /W4 /WX)                              # Enable high warning level and treat them as errors             
    target_compile_options(${PROJECT_NAME}-core PUBLIC /arch:AVX2)                            # AVX instructions
    target_compile_options(${PROJECT_NAME}-core PUBLIC /fp:fast)                              # float point fast math
    target_compile_options(${PROJECT_NAME}-core PUBLIC $<$<CONFIG:Release>:/GL>)              # link-time optimization
    target_compile_options(${PROJECT_NAME}-core PUBLIC $<$<CONFIG:Release>:/LTCG:incremental>)# link-time optimization
    target_compile_options(${PROJECT_NAME}-core PUBLIC $<$<CONFIG:Debug>:/fsanitize=address>) # asan
else()
    target_compile_options(${PROJECT_NAME}-core PUBLIC $<IF:$<CONFIG:Release>,-O3,-O0>)
    target_compile_options(${PROJECT_NAME}-core PUBLIC $<$<CONFIG:Release>:-DNDEBUG>)
    target_compile_options(${PROJECT_NAME}-core PUBLIC $<$<CONFIG:Release>:-flto>)
    target_compile_options(${PROJECT_NAME}-core PUBLIC -pthread)
    target_compile_options(${PROJECT_NAME}-core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_include_directories(${PROJECT_NAME}-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(
    TARGET ${PROJECT_NAME}-core PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}-core>/resources
)


 