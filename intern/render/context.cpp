#include "context.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_structs.hpp"
#include <memory>

namespace Render {
std::unique_ptr<VulkanContext> VulkanContext::instance = nullptr;

VulkanContext::VulkanContext() {
    vk::InstanceCreateInfo info;
    vk::ApplicationInfo appInfo;
    /* 以下设置 vk 的信息(基本上都是版本信息, 不是很重要) */
    appInfo.setApiVersion(VK_API_VERSION_1_3);
    
    /* 设置完成 */
    info.setPApplicationInfo(&appInfo);
    vkInstance = vk::createInstance(info);
}

VulkanContext::~VulkanContext() {
    vkInstance.destroy();
}

void VulkanContext::Init() {
    instance.reset(new VulkanContext());
}

void VulkanContext::Cleanup() {
    instance.reset();
}

VulkanContext& VulkanContext::Instance() {

    return *instance;
}

} // namespace Render
