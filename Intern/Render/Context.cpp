#include "context.h"
#include <memory>
#include <vector>
#include <iostream>

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
    if (devices.empty()) {
        this->ToReleaseResources();
        throw std::runtime_error("No physical device found");
    }
    /* 实际上可以选择性能最好的那个物理设备, 这里默认就是选择第一个 */
    vkPhysicalDevice = devices.front(); 

    /* 输出调试信息 */
    std::cout << "Selected physical device: " << vkPhysicalDevice.getProperties().deviceName << std::endl;

    /* 查询物理设备 */
    auto properties = vkPhysicalDevice.getQueueFamilyProperties();
    for (int i=0; i < properties.size(); ++i) {
        const auto& property = properties[i];
        if (property.queueFlags | vk::QueueFlagBits::eGraphics) {
            
        }
    }
}

void VulkanContext::Cleanup() {
    instance.reset();
}

VulkanContext& VulkanContext::Instance() {

    return *instance;
}

} // namespace Render
