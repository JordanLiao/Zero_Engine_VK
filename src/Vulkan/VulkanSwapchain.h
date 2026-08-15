#ifndef _VULKANSWAPCHAIN_H_
#define _VULKANSWAPCHAIN_H_

#include "VulkanImage.h"

#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;
struct GLFWwindow;

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapchain {
public:
    std::vector<VkImageView> imageViews;
    std::vector<VkImage> images;
    uint32_t imageCount;
    VkFormat format;
    VkExtent2D extent;

    VulkanSwapchain();
    VulkanSwapchain(VulkanContext* vulkanContext, VkSwapchainKHR old = VK_NULL_HANDLE);

    VulkanSwapchain(VulkanSwapchain&& other) noexcept;
    VulkanSwapchain& operator=(VulkanSwapchain&& other) noexcept;
    ~VulkanSwapchain();

    static VulkanSwapchain recreateSwapchain(VulkanContext* vulkanContext, VulkanSwapchain& old);

    VkSwapchainKHR getSwapchain() const;

private:
    VulkanContext* context;
    VkSwapchainKHR swapchain;

    void cleanup();
    SwapchainSupportDetails querySwapchainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
};

#endif