#include "context.h"
#include <cstddef>
#include <memory>
#include <vector>
#include <iostream>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Render {
std::unique_ptr<VulkanContext> VulkanContext::instance = nullptr;

VulkanContext::VulkanContext() {
    vk::InstanceCreateInfo info;
    vk::ApplicationInfo appInfo;
    /* 以下设置 appInfo 的信息(基本上都是版本信息, 不是很重要) */
    
    /* 设置完成 */
    /* 以下设置 info 的信息 */
    info.setPApplicationInfo(&appInfo);

    /* 检查 extension support */
    std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();
    /* 输出 extension support 信息 */
    for (const auto& ext : extensions) {
        std::cout << "Extension: " << ext.extensionName << ", Version: " << ext.specVersion << std::endl;
    }

    /* 添加验证层 */
    std::vector<const char*> layers = {
        "VK_LAYER_KHRONOS_validation"
    };
    RemoveUnsupportedElems<const char*, vk::LayerProperties>(
        layers,
        vk::enumerateInstanceLayerProperties(),
        [](const char* e1, const vk::LayerProperties& e2) {
            return strcmp(e1, e2.layerName) == 0;
        }
    );

    info.setPEnabledLayerNames(layers);
    vkInstance = vk::createInstance(info);
    if (vkInstance == nullptr) {
        this->ToReleaseResources();
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

void VulkanContext::ToReleaseResources() {
    vkDevice.destroy();
    vkInstance.destroy();
}


VulkanContext::~VulkanContext() {
    this->ToReleaseResources();
}

void VulkanContext::Init() {
    instance.reset(new VulkanContext());
}

void VulkanContext::InitPhysicalDevice() {
    auto devices = vkInstance.enumeratePhysicalDevices();
    /* 查询当前显卡支持的特性 */
    // for (auto& device : devices) {
    //     auto features = device.getFeatures();
    // }
    if (devices.empty()) {
        this->ToReleaseResources();
        throw std::runtime_error("No physical device found");
    }
    /* 实际上可以选择性能最好的那个物理设备, 这里默认就是选择第一个 */
    vkPhysicalDevice = devices.front(); 

    /* 输出调试信息 */
    /* device type: vkPhysicalDevice.getProperties().deviceType
        - eCPU: 软渲染(CPU 作为显卡)
        - eDiscreteGPU: 独立显卡
        - eIntegratedGPU: 集成显卡
        - eOther: 其他
        - eVirtualGPU: 虚拟显卡
    */
    std::cout << "Selected physical device: " << vkPhysicalDevice.getProperties().deviceName << std::endl;

    /* 查询物理设备 */
    auto properties = vkPhysicalDevice.getQueueFamilyProperties();
    for (int i=0; i < properties.size(); ++i) {
        const auto& property = properties[i];
        /* property.queueCount; 最多能创建多少个队列 */
        /* 查询队列图形操作的命令队列 */
        if (property.queueFlags | vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsQueue = i;
            break;
        }
    }

    /* 创建物理设备 */
    vk::DeviceCreateInfo deviceInfo; /* 与 vk::Instance 相同的信息会延续下来 */
    vk::DeviceQueueCreateInfo queueInfo;
    float priority = 1.0f; /* 低到高 0 ~ 1f */
    queueInfo.setPQueuePriorities(&priority)
             .setQueueCount(1) /* 创建队列时使用的数量 */
             .setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value());
    deviceInfo.setQueueCreateInfos(queueInfo);
    vkDevice = vkPhysicalDevice.createDevice(deviceInfo);

    /* 获取设备队列 */
    /* param1: 命令队列下标 */
    /* param2: 拿第几个队列出来 */
    vkGraphicsQueue = vkDevice.getQueue(queueFamilyIndices.graphicsQueue.value(), 0);
}

void VulkanContext::Cleanup() {
    instance.reset();
}

VulkanContext& VulkanContext::Instance() {
    return *instance;
}

} // namespace Render
