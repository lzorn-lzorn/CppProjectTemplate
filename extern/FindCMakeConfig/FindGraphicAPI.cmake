find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    # Vulkan_INCLUDE_DIR 在下载时同时包含了 glm/glm.hpp
    target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
else()
    message(FATAL_ERROR "Vulkan not found")
endif()
