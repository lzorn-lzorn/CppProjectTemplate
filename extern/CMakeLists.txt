

if(ENABLE_BENCHMARK)
    # 直接关闭所有测试相关选项
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing")
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable benchmark gtest tests")
    set(BENCHMARK_USE_BUNDLED_GTEST OFF CACHE BOOL "Disable bundled Google Test")
    add_subdirectory(benchmark)
    target_link_libraries(${PROJECT_NAME} PRIVATE benchmark::benchmark)
endif()

if(ENABLE_SDL3)
    add_subdirectory(vendored/SDL SDL3)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
endif()
