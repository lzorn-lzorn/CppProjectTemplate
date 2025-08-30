#pragma once
#include "vulkan/vulkan.hpp"
#include <memory>

namespace Render {

class VulkanContext final{
public:
    /* Initialize Vulkan context */ 
    static void Init();
    /* Cleanup Vulkan context */ 
    static void Cleanup();
    static VulkanContext& Instance();
    ~VulkanContext();
    vk::Instance vkInstance;

private:
    VulkanContext();
    static std::unique_ptr<VulkanContext> instance;
};
}
