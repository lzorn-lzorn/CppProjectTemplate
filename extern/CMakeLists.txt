# 直接关闭所有测试相关选项
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable benchmark gtest tests")
set(BENCHMARK_USE_BUNDLED_GTEST OFF CACHE BOOL "Disable bundled Google Test")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(benchmark)
endif()