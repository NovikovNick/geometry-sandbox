set(APP_NAME ${PROJECT_NAME}-tests)

include(GoogleTest)

set(TEST_FILES
    slot_map_test.cpp
    animation_test.cpp
    reflection_test.cpp
    dsl_test.cpp
    json_service_parse_coord_2d_array_string_test.cpp
    json_service_is_valid_coord_2d_array_string_test.cpp
)

add_executable(${APP_NAME} ${TEST_FILES})
target_compile_features(${APP_NAME} PUBLIC cxx_std_20)
target_link_libraries(${APP_NAME} 
    PRIVATE 
        ${PROJECT_NAME}-core
        gtest_main
        gmock_main)

add_test(NAME "${APP_NAME}" COMMAND "${APP_NAME}")