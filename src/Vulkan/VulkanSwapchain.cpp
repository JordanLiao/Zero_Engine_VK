#include "VulkanSwapchain.h"
#include "VulkanContext.h"

#include "GLFW/glfw3.h"

#include <stdexcept>

VulkanSwapchain::VulkanSwapchain() {
    swapchain = VK_NULL_HANDLE;
    format = VK_FORMAT_B8G8R8A8_SRGB;
    extent = { 1,1 };
    logicalDevice = VK_NULL_HANDLE;
    surface = VK_NULL_HANDLE;
}

VulkanSwapchain::VulkanSwapchain(const VulkanContext& context) {
    logicalDevice = context.logicalDevice;
    //physicalDevice = context.physicalDevice;
    surface = context.surface;

    SwapchainSupportDetails support = querySwapchainSupport(context.surface, context.physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(support.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(support.presentModes);
    VkExtent2D extent = chooseSwapExtent(context.window, support.capabilities);

    //stored for later
    this->extent = extent;
    this->format = surfaceFormat.format;

    //maxImageCount could be equal to the minImageCount, or it could be 0 to indicate there 
    //is no limit on the number of images in the swapchain
    imageCount = support.capabilities.minImageCount + 1;
    if(support.capabilities.maxImageCount > 0 && imageCount > support.capabilities.maxImageCount) {
        imageCount = support.capabilities.maxImageCount;
    }
      
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = context.surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { context.queueFamilyIndices.graphicsFamily.value(), 
                                      context.queueFamilyIndices.presentFamily.value() };
    if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = support.capabilities.currentTransform; //no transform
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //ignore alpha
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, images.data());

    imageViews.resize(images.size());
    for (uint32_t i = 0; i < images.size(); i++) {
        imageViews[i] = VulkanImageUtils::createImageView(images[i], format, VK_IMAGE_ASPECT_COLOR_BIT, logicalDevice);
    }
}

VkSurfaceFormatKHR VulkanSwapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR VulkanSwapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanSwapchain::chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

SwapchainSupportDetails VulkanSwapchain::querySwapchainSupport(VkSurfaceKHR surface, VkPhysicalDevice device) {
    SwapchainSupportDetails swapchainSupport;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapchainSupport.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        swapchainSupport.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapchainSupport.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        swapchainSupport.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, swapchainSupport.presentModes.data());
    }

    return swapchainSupport;
}

void VulkanSwapchain::cleanup() {
    for (size_t i = 0; i < imageViews.size(); i++)
        vkDestroyImageView(logicalDevice, imageViews[i], nullptr);

    if(swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
}
