cmake_minimum_required(VERSION 3.10)
set(TARGET_NAME Render)
# 导入findGraphicAPI模块API
include(../../Cmake/FindGraphicAPI.cmake)
# 选项：是否构建为动态库（默认 OFF -> 构建静态库）
option(PROJ_BUILD_SHARED "Build ${TARGET_NAME} as shared library" OFF)

# 寻找编译glsl的编译器 glslc
find_program(GLSLC_PROGRAM glslc REQUIRED)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/Shader/shader.vert -o ${CMAKE_CURRENT_SOURCE_DIR}/Shader/Out/vert.spv)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_CURRENT_SOURCE_DIR}/Shader/shader.frag -o ${CMAKE_CURRENT_SOURCE_DIR}/Shader/Out/frag.spv)





# 找到头/源文件（仅用于 IDE/file listing）注意：include dirs 下面要使用目录而不是文件列表
file(GLOB_RECURSE INC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# 公共 include 目录（把目录传递给使用者）
set(PUBLIC_INCLUDES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:Include>
)

# 创建动态库
if(PROJ_BUILD_SHARED)
    message(STATUS "${TARGET_NAME}: creating SHARED library")
    add_library(${TARGET_NAME} SHARED ${SRC} ${INC})

    # 构建库时，库内部需要定义导出宏 -> Proj_EXPORTS（PRIVATE，仅对库本身生效）
    target_compile_definitions(${TARGET_NAME} PRIVATE Proj_EXPORTS)

    # 对非 Windows 平台，建议隐藏默认可见性并用 attribute 控制导出
    if(NOT WIN32)
        target_compile_options(${TARGET_NAME} PRIVATE -fvisibility=hidden)
    endif()

# 创建静态库
else()
    message(STATUS "${TARGET_NAME}: creating STATIC library")
    add_library(${TARGET_NAME} STATIC ${SRC} ${INC})

    # 构建静态库时，消费者需要知道是静态库（头里会把 Proj_API 设为空）
    # 使用 PUBLIC 让 link 的 downstream targets 也看到此宏
    target_compile_definitions(${TARGET_NAME} PUBLIC Proj_STATIC)
endif()

# include dirs 暴露给 consumers（库的 public include）
target_include_directories(${TARGET_NAME} PUBLIC ${PUBLIC_INCLUDES})

TargetLinkGraphicAPIWith(${TARGET_NAME})

# -----------------------
# 示例：如何在上层 target 使用 render
# 假设你的可执行 target 名为 "app"（在 top-level CMakeLists 中）
# add_executable(app ../app/main.cpp)
# target_link_libraries(app PRIVATE render)
# app 会自动获得 render 的 include 目录和 Proj_STATIC / Proj_USE_STDCALL 等宏
# -----------------------

# 如果你希望使用 BUILD_SHARED_LIBS 全局开关而非自定义 PROJ_BUILD_SHARED：
# if(BUILD_SHARED_LIBS)
# set(PROJ_BUILD_SHARED ON)
# endif()

# origin:
# add_library(${TARGET_NAME} ${INC} ${SRC})
# target_include_directories(${TARGET_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})
# # add_library(${TARGET_NAME} INTERFACE)
# target_link_libraries(${PROJECT_NAME} PRIVATE render)
# target_include_directories(${PROJECT_NAME} PUBLIC ${INC})
