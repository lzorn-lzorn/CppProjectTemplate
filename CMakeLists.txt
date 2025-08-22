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

include(cmake/cmaketools.cmake)
include(cmake/options.cmake)

add_executable(${PROJECT_NAME} ${sources})

add_subdirectory(intern)
add_subdirectory(extern)

if(WIN32)
    # 获取可执行文件的输出目录
    get_target_property(EXECUTABLE_OUTPUT_DIR ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY)
    if(NOT EXECUTABLE_OUTPUT_DIR)
        set(EXECUTABLE_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
    if(NOT EXECUTABLE_OUTPUT_DIR)
        set(EXECUTABLE_OUTPUT_DIR "${CMAKE_BINARY_DIR}")
    endif()

    # 查找所有dll
    file(GLOB_RECURSE DLL_FILES
        "${CMAKE_BINARY_DIR}/*.dll"
        "${CMAKE_BINARY_DIR}/extern/**/*.dll"
        "${CMAKE_BINARY_DIR}/intern/**/*.dll"
    )

    # 为每个dll生成拷贝命令
    foreach(DLL_FILE ${DLL_FILES})
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_FILE}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
        )
    endforeach()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE extra_functions)

target_include_directories(${PROJECT_NAME} PUBLIC include/)
