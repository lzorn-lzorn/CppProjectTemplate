cmake_minimum_required(VERSION 3.10)
set(TARGET_NAME render)

# 选项：是否构建为动态库（默认 OFF -> 构建静态库）
option(PROJ_BUILD_SHARED "Build ${TARGET_NAME} as shared library" OFF)

# 选项：在 x86 上使用 __stdcall（默认 OFF -> 使用 __cdecl）
option(PROJ_USE_STDCALL "Use __stdcall calling convention on x86" OFF)

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
    $<INSTALL_INTERFACE:include>
)

# 如果没有任何 .cpp，则认为是 header-only，创建 INTERFACE target
if(NOT SRC)
    message(STATUS "${TARGET_NAME}: no .cpp found -> creating header-only INTERFACE target")
    add_library(${TARGET_NAME} INTERFACE)
    target_include_directories(${TARGET_NAME} INTERFACE ${PUBLIC_INCLUDES})

    # header-only 当作静态处理：让头文件中的 Proj_API 选择 Proj_STATIC 分支（即空）
    target_compile_definitions(${TARGET_NAME} INTERFACE Proj_STATIC)

    # 如果头文件依赖 Vulkan include，传播给消费者
    if(TARGET Vulkan::Vulkan)
        target_link_libraries(${TARGET_NAME} INTERFACE Vulkan::Vulkan)
    elseif(DEFINED Vulkan_INCLUDE_DIR)
        target_include_directories(${TARGET_NAME} INTERFACE ${Vulkan_INCLUDE_DIR})
    endif()

    # 如果选择 stdcall，传播此宏（但只有在配置时已开启）
    if(PROJ_USE_STDCALL)
        # 实际生效只在 x86 Windows；这里先传播宏，到 consumers 时会在头里根据架构选择生效路径
        target_compile_definitions(${TARGET_NAME} INTERFACE Proj_USE_STDCALL)
    endif()

else()
    # 有 .cpp，创建静态或动态库
    if(PROJ_BUILD_SHARED)
        message(STATUS "${TARGET_NAME}: creating SHARED library")
        add_library(${TARGET_NAME} SHARED ${SRC} ${INC})

        # 构建库时，库内部需要定义导出宏 -> Proj_EXPORTS（PRIVATE，仅对库本身生效）
        target_compile_definitions(${TARGET_NAME} PRIVATE Proj_EXPORTS)

        # 对非 Windows 平台，建议隐藏默认可见性并用 attribute 控制导出
        if(NOT WIN32)
            target_compile_options(${TARGET_NAME} PRIVATE -fvisibility=hidden)
        endif()
    else()
        message(STATUS "${TARGET_NAME}: creating STATIC library")
        add_library(${TARGET_NAME} STATIC ${SRC} ${INC})

        # 构建静态库时，消费者需要知道是静态库（头里会把 Proj_API 设为空）
        # 使用 PUBLIC 让 link 的 downstream targets 也看到此宏
        target_compile_definitions(${TARGET_NAME} PUBLIC Proj_STATIC)
    endif()

    # include dirs 暴露给 consumers（库的 public include）
    target_include_directories(${TARGET_NAME} PUBLIC ${PUBLIC_INCLUDES})

    # Vulkan include/link（如果你的 render 实现依赖 Vulkan）
    if(TARGET Vulkan::Vulkan)
        target_link_libraries(${TARGET_NAME} PUBLIC Vulkan::Vulkan)
    elseif(DEFINED Vulkan_INCLUDE_DIR)
        target_include_directories(${TARGET_NAME} PUBLIC ${Vulkan_INCLUDE_DIR})
    endif()

    # 如果希望 consumers 在 x86 上也使用 stdcall（且在配置时勾选了选项），传播宏
    if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4 AND PROJ_USE_STDCALL)
        target_compile_definitions(${TARGET_NAME} PUBLIC Proj_USE_STDCALL)
    endif()
endif()

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
