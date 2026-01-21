set(APP_NAME ${PROJECT_NAME}-unit-benchmarks)

add_executable(${APP_NAME}
    # sample.cpp        # execution time of simple operations as a reference for comparison
    # simd_sample.cpp   # execution time of simd operations as a reference for comparison
    "main.cpp"
    "animation.cpp" 
)
target_link_libraries(${APP_NAME} 
    PRIVATE
        benchmark::benchmark
        ${PROJECT_NAME}-core)
