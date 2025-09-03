#pragma once
#include "vulkan/vulkan.hpp"
#include "Utilities.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>
using CreateSurfaceFunc = std::function<vk::SurfaceKHR(vk::Instance)>;
namespace Render {

class Swapchain final {
public:
    struct SwapchainInfo{
        /* 图像大小 */
        vk::Extent2D imageExtent;
        vk::SurfaceFormatKHR format;
        vk::SurfaceTransformFlagBitsKHR transform;
        vk::PresentModeKHR present;
        uint32_t imageCount;
    };
public:
    Swapchain(int width, int height);
    ~Swapchain();
    vk::SwapchainKHR vkSwapchain;
    SwapchainInfo info;
    /* vk::Image 和 vk::ImageView 是一样多的(应该封装下) */
    std::vector<vk::Image> images;
    std::vector<vk::ImageView> imageViews;
private:
    void QueryInfo(int width, int height);
    void InitImages();
    void CreateImageViews(); 
};



class VulkanContext final{
public:
    struct QueueFamilyIndices final {
        /* 表示队列, 用于控制显示与格式 */
        std::optional<uint32_t> presentQueue;
        /* 图像操作的命令队列 */
        std::optional<uint32_t> graphicsQueue;

        operator bool() const {
            return Check();
        }
        bool Check() const {
            return graphicsQueue.has_value() && presentQueue.has_value();
        }
    };
public:
    /* usage: 
        SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr);
        std::vector<const char*> extensions(count);
        SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data());
        Init(
            extensions,
            [&](vk::Instance instance){
                VkSurfaceKHR surface;
                // 与底层无关的表示
                if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) {
                    throw std::runtime_error("can't create surface");
                }
                return surface;
            }, 
            1024, 
            720
        );
    */
    /* Initialize Vulkan context */ 
    static void Init(const std::vector<const char*>& extensions, CreateSurfaceFunc func);
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
    vk::Queue vkPresentQueue;
    vk::SurfaceKHR vkSurface;
    std::unique_ptr<Swapchain> swapchain;
    QueueFamilyIndices queueFamilyIndices;
public:
    void InitSwapchain(int width, int height){
        swapchain = std::make_unique<Swapchain>(width, height);
    }
    void DestroySwapchain(){
        swapchain.reset();
    }
private:
    void ToReleaseResources();
private:
    VulkanContext(const std::vector<const char*>& extensions, CreateSurfaceFunc func);
    void InitPhysicalDevice(CreateSurfaceFunc func);
    static std::unique_ptr<VulkanContext> instance;
};
}
