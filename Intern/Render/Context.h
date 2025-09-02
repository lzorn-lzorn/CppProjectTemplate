#pragma once
#include "vulkan/vulkan.hpp"
#include "Utilities.hpp"
#include <memory>

namespace Render {

class VulkanContext final{
private:
    struct QueueFamliyIndices{};
public:
    /* Initialize Vulkan context */ 
    static void Init();
    /* Cleanup Vulkan context */ 
    static void Cleanup();
    static VulkanContext& Instance();
    ~VulkanContext();
    vk::Instance vkInstance;
    vk::PhysicalDevice vkPhysicalDevice;
    vk::Device device;
    vk::Queue graphcisQueue;
    QueueFamliyIndices queueFamilyIndices;
private:
    void ToReleaseResources();
private:
    VulkanContext();
    void InitPhysicalDevice();
    static std::unique_ptr<VulkanContext> instance;
};
}
