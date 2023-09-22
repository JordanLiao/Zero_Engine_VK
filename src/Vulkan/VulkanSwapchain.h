#ifndef _VULKANSWAPCHAIN_H_
#define _VULKANSWAPCHAIN_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanContext.h"
#include "VulkanTexture.h"

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapchain {
public:
    VkSwapchainKHR swapchain;
    std::vector<VkImageView> imageViews;
    std::vector<VkImage> images;
    uint32_t imageCount;
    VkFormat format;
    VkExtent2D extent;

    VulkanSwapchain();
    VulkanSwapchain(const VulkanContext& vulkanContext);
    void cleanup();

private:
    VkDevice logicalDevice;
    //VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;

    SwapchainSupportDetails querySwapchainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
};

#endif