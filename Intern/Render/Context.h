#pragma once
#include "vulkan/vulkan.hpp"
#include "Utilities.hpp"
#include <cstdint>
#include <memory>
#include <optional>

namespace Render {

class VulkanContext final{
public:
    struct QueueFamilyIndices final {
        /* 图像操作的命令队列 */
        std::optional<uint32_t> graphicsQueue;
    };
public:
    /* Initialize Vulkan context */ 
    static void Init();
    /* Cleanup Vulkan context */ 
    static void Cleanup();
    static VulkanContext& Instance();
    ~VulkanContext();
    vk::Instance vkInstance;
    /* 物理信息: 只支持查询信息 */
    vk::PhysicalDevice vkPhysicalDevice;
    /* 逻辑设备: Vulkan 规定不能直接和物理设备(vkPhysicalDevice) 交互 */
    vk::Device vkDevice;
    /* 图像操作的命令队列 */
    vk::Queue vkGraphicsQueue;
    QueueFamilyIndices queueFamilyIndices;
private:
    void ToReleaseResources();
private:
    VulkanContext();
    void InitPhysicalDevice();
    static std::unique_ptr<VulkanContext> instance;
};
}
