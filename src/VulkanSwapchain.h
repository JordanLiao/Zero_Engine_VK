#ifndef _VULKANSWAPCHAIN_H_
#define _VULKANSWAPCHAIN_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanCommon.h"
#include "VulkanContext.h"

class VulkanSwapchain {
public:
	VkSwapchainKHR swapChain;
	VulkanCommon::SwapchainSupportDetails support;
	VkFormat format;
	VkExtent2D extent;

	VulkanSwapchain(const VulkanContext& vulkanContext);
	void cleanup();

private:
	VkDevice logicalDevice;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	VulkanCommon::SwapchainSupportDetails querySwapchainSupport(VkSurfaceKHR surface, VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
};

#endif