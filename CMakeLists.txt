cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# note: 启用生成 compile_commands.json
set(EXPORT_COMPILE_COMMANDS ON)

project(MyProject            # 项目名称
    LANGUAGES CXX C          # 使用语言
    DESCRIPTION "Stellatus!" # 项目描述信息
    VERSION 0.1.0            # 项目版本    
)

# 设置install路径
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

file(GLOB_RECURSE sources PUBLIC
    source/*.cpp
    source/*.c
)

if(WIN32)
    message("===========Current OS: Windows===========")

elseif(UNIX)
    if(APPLE)
        message("===========Current OS: IOS===========")
    else()
        message("===========Current OS: Linux===========")
    endif()
else()
    message(FATAL_ERROR "Unknown Platform")
endif()

include(cmake/cmaketools.cmake)
include(cmake/options.cmake)

add_executable(${PROJECT_NAME} ${sources})

add_subdirectory(intern)
add_subdirectory(extern)

# target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
target_link_libraries(${PROJECT_NAME} PRIVATE extra_functions)

target_include_directories(${PROJECT_NAME} PUBLIC include/)
