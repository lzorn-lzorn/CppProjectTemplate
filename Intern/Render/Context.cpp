#include "context.h"
#include "Context.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <vector>
#include <iostream>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace Render {
/* ============================= Swapchain =============================*/
void Swapchain::QueryInfo(int width, int height){
    auto& phyDevice = VulkanContext::Instance().vkPhysicalDevice;
    auto& surface = VulkanContext::Instance().vkSurface;
    auto formats = phyDevice.getSurfaceFormatsKHR(surface);
    info.format = formats.front();
    for (const auto& format : formats){
        if (format.format == vk::Format::eB8G8R8A8Sint &&
            /* 颜色空间: 非线性SRGB */
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            info.format = format;
            break;
        }
    }
    auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
    
    info.imageCount = std::clamp<uint32_t>(
        2, /* 两个图像交替绘制, 相当于双缓冲 */
        capabilities.minImageCount, 
        capabilities.maxImageCount
    );
    info.imageExtent.width = std::clamp<uint32_t>(
        width, 
        capabilities.minImageExtent.width, 
        capabilities.maxImageExtent.width
    );
    info.imageExtent.height = std::clamp<uint32_t>(
        height, 
        capabilities.minImageExtent.height, 
        capabilities.maxImageExtent.height
    );

    /* Vulkan 把 vk::Image 贴到屏幕之前可以对图像做一个变换 */
    info.transform = capabilities.currentTransform; // 不变换

    auto presents = phyDevice.getSurfacePresentModesKHR(surface);
    /*
        FIFO: 图像队列先进先出(先绘制好但在后面的不能提前显示)
        RELAX: 允许图像队列中断, 前一个没有绘制好, 但是后一个已经好了, 就停止绘制当前的直接绘制下一个 (会有撕裂, tearing)
        IMMEDIATE: 立即呈现, 不进行队列(一定会撕裂, 但性能高)
        MAILBOX: 每次绘制从 mailbox 中取图像进行绘制, 同时 mailbox 中会保存最新的图像, 如果 mailbox 中有多个图像, 那么就会丢弃旧的图像不会绘制过时图像
    */

    info.present = vk::PresentModeKHR::eFifo; // 默认值 Vulkan 一定会支持的格式
    for (const auto& present : presents) {
        if (present == vk::PresentModeKHR::eMailbox) {
            info.present = present;
            break;
        }
    }
}
Swapchain::Swapchain(int width, int height) {
    QueryInfo(width, height);

    vk::SwapchainCreateInfoKHR createInfo;
            /* 启用超出屏幕图像裁切 */
    createInfo.setClipped(true) 
            /* 图像数组层数(不使用3D Image为1即可) */
              .setImageArrayLayers(1) 
            /* 图像使用方式: 颜色附件 附加至 FrameBuffer */
              .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            /* 指定 vk::Image 颜色和屏幕显示的颜色混合方式: Opaque 不混合 */
              .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
              .setSurface(VulkanContext::Instance().vkSurface)
              .setImageColorSpace(info.format.colorSpace)
              .setImageExtent(info.imageExtent)
              .setImageFormat(info.format.format)
              .setMinImageCount(info.imageCount)
              .setPreTransform(info.transform)
              .setPresentMode(info.present);

    auto& queueIndices = VulkanContext::Instance().queueFamilyIndices;
    if(queueIndices.graphicsQueue.value() == queueIndices.presentQueue.value()){
        /* 图像同时只能被一个命令队列使用 */
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }else{
        std::array indices = {
            queueIndices.graphicsQueue.value(), 
            queueIndices.presentQueue.value()
        };
        createInfo.setQueueFamilyIndices(indices)
                /* 图像可以被并行使用 */
                  .setImageSharingMode(vk::SharingMode::eConcurrent);
    }
    
    vkSwapchain = VulkanContext::Instance().vkDevice.createSwapchainKHR(createInfo);
    
    InitImages();
    CreateImageViews();
}
Swapchain::~Swapchain(){
    for (auto& view : imageViews){
        VulkanContext::Instance().vkDevice.destroyImageView(view);
    }
    VulkanContext::Instance().vkDevice.destroySwapchainKHR(vkSwapchain);
}
void Swapchain::InitImages(){
    images = VulkanContext::Instance().vkDevice.getSwapchainImagesKHR(vkSwapchain);
}

/* 针对每个Image创建对应的ImageView */
void Swapchain::CreateImageViews(){
    imageViews.resize(images.size());
    for (size_t i=0; i < images.size(); ++i){
        vk::ImageViewCreateInfo createInfo;
        vk::ComponentMapping mapping; // 看修改颜色之间的映射
        // 默认 mapping.setA(vk::ComponentSwizzle::eIdentity); 

        vk::ImageSubresourceRange range;
        range.setBaseMipLevel(0)  // 设置 MipMap Level (就是图片本身)
             .setLevelCount(1)    // 多级界面纹理 (就是图片本身只有一级 )
             .setBaseArrayLayer(0)
             .setLayerCount(1)
             .setAspectMask(vk::ImageAspectFlagBits::eColor);
        createInfo.setImage(images[i])
                  .setViewType(vk::ImageViewType::e2D) /* 2D 图像 */
                  .setComponents(mapping) /* 用于设置RGBA的映射 */
                  .setFormat(info.format.format)
                  .setSubresourceRange(range);
        imageViews[i] = VulkanContext::Instance().vkDevice.createImageView(createInfo);
    }
}
/* =========================== VulkanContext ===========================*/
std::unique_ptr<VulkanContext> VulkanContext::instance = nullptr;

VulkanContext::VulkanContext(const std::vector<const char*>& extensions, CreateSurfaceFunc func) {
    vk::InstanceCreateInfo info;
    vk::ApplicationInfo appInfo;
    /* 以下设置 appInfo 的信息(基本上都是版本信息, 不是很重要) */
    
    /* 设置完成 */
    /* 以下设置 info 的信息 */
    info.setPApplicationInfo(&appInfo);

    /* 检查 extension support */
    std::vector<vk::ExtensionProperties> vkExtensions = vk::enumerateInstanceExtensionProperties();
    /* 输出 extension support 信息 */
    for (const auto& ext : vkExtensions) {
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

    info.setPEnabledLayerNames(layers)
        .setPEnabledExtensionNames(extensions);

    vkInstance = vk::createInstance(info);
    if (vkInstance == nullptr) {
        this->ToReleaseResources();
        throw std::runtime_error("Failed to create Vulkan instance");
    }
    InitPhysicalDevice(func);
}

void VulkanContext::ToReleaseResources() {
    vkInstance.destroySurfaceKHR(vkSurface);
    vkDevice.destroy();
    vkInstance.destroy();
}

VulkanContext::~VulkanContext() {
    this->ToReleaseResources();
}

void VulkanContext::Init(const std::vector<const char*>& extensions, CreateSurfaceFunc func) {
    instance.reset(new VulkanContext(extensions, func));
}

void VulkanContext::InitPhysicalDevice(CreateSurfaceFunc func) {
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

    /* 初始化交换链 */
    vkSurface = func(vkInstance);
    if (vkSurface == nullptr) {
        this->ToReleaseResources();
        throw std::runtime_error("Failed to create surface");
    }
    /* 查询物理设备 */
    auto properties = vkPhysicalDevice.getQueueFamilyProperties();
    for (int i=0; i < properties.size(); ++i) {
        const auto& property = properties[i];
        /* property.queueCount; 最多能创建多少个队列 */
        /* 查询队列图形操作的命令队列 */
        if (property.queueFlags | vk::QueueFlagBits::eGraphics) {
            queueFamilyIndices.graphicsQueue = i;
        }
        /* 查询表示队列 */
        if (vkPhysicalDevice.getSurfaceSupportKHR(i, vkSurface)) {
            queueFamilyIndices.presentQueue = i;
        }
        if (queueFamilyIndices.Check()){
            break;
        }
    }

    /* 创建物理设备 */
    std::array extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    vk::DeviceCreateInfo deviceInfo; /* 与 vk::Instance 相同的信息会延续下来 */
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    float priority = 1.0f; /* 低到高 0 ~ 1f */
    vk::DeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.setPQueuePriorities(&priority)
            .setQueueCount(1) /* 创建队列时使用的数量 */
            .setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value());
    if(queueFamilyIndices.presentQueue.value() == queueFamilyIndices.graphicsQueue.value()){
        /* 如果两个队列同时支持, 创建一个队列即可 */
        queueCreateInfos.push_back(std::move(queueCreateInfo));
    } else {
        /* 如果是两个不同的队列, 先pushback一个再重新设置另一个 */
        queueCreateInfos.push_back(queueCreateInfo); // 拷贝构造
        queueCreateInfo.setPQueuePriorities(&priority)
                       .setQueueCount(1)
                       .setQueueFamilyIndex(queueFamilyIndices.presentQueue.value());
        queueCreateInfos.push_back(queueCreateInfo);
    }
    deviceInfo.setQueueCreateInfos(queueCreateInfos)
              .setPEnabledExtensionNames(extensions);

    vkDevice = vkPhysicalDevice.createDevice(deviceInfo);

    /* 获取设备队列 */
    /* param1: 命令队列下标 */
    /* param2: 拿第几个队列出来 */
    vkGraphicsQueue = vkDevice.getQueue(queueFamilyIndices.graphicsQueue.value(), 0);
    vkPresentQueue = vkDevice.getQueue(queueFamilyIndices.presentQueue.value(), 0);
}

void VulkanContext::Cleanup() {
    instance.reset();
}

VulkanContext& VulkanContext::Instance() {
    return *instance;
}

} // namespace Render
