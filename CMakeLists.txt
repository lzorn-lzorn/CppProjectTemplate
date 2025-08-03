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

file(GLOB_RECURSE sources PUBLIC
    source/*.cpp
    source/*.c
)

if(WIN32)
    # , Windows下的输出路径
    # note: Windows 下 dll 和 exe 必须在同一个目录下
    message("===========Current OS: Windows===========")
elseif(UNIX)
    if(APPLE)
        # , IOS下的输出路径
        message("===========Current OS: IOS===========")
    else()
        # , Linux下的输出路径
        message("===========Current OS: Linux===========")
    endif()
else()
    message(FATAL_ERROR "Unknown Platform")
endif()

add_subdirectory(intern)
add_subdirectory(extern)

add_executable(${PROJECT_NAME} ${sources})


target_include_directories(${PROJECT_NAME} PUBLIC include/)
