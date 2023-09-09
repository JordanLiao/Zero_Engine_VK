#ifndef _VULKANCONTEXT_H_
#define _VULKANCONTEXT_H_

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include "GLFW/glfw3.h"

#include "VulkanCommon.h"

#include <vector>
#include <optional>
#include <algorithm>
#include <string>
#include <memory>

#ifdef _DEBUG
#define ENABLE_VALIDATION_LAYER true
#else
#define ENABLE_VALIDATION_LAYER false
#endif

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
};

class VulkanContext {
public:
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue transferQueue;
	VulkanCommon::QueueFamilyIndices queueFamilyIndices;
	GLFWwindow* window;

	VulkanContext(GLFWwindow* window);
	void cleanup();

private:

	VkInstance vulkanInstance;
	void createVulkanInstance();
	
	bool checkValidationLayerSupport();

	void createSurface();

	VulkanCommon::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pDevice);
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice pDevice);
	bool checkDeviceExtensionSupport(VkPhysicalDevice pDevice); 
	
	void createLogicalDevice();
};

#endif