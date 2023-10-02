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
    VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
};

const std::vector<const char*> instanceExtensions = {
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

class VulkanContext {
public:
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;

    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
    VulkanCommon::QueueFamilyIndices queueFamilyIndices;
    GLFWwindow* window;

    //Contain properties infos for different types of descriptors.
    VkPhysicalDeviceDescriptorBufferPropertiesEXT* descriptorBufferProperties;

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR = VK_NULL_HANDLE;
    PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSizeEXT = VK_NULL_HANDLE;
    PFN_vkGetDescriptorEXT vkGetDescriptorEXT = VK_NULL_HANDLE;
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR = VK_NULL_HANDLE;

    VulkanContext();
    VulkanContext(GLFWwindow* window);
    void cleanup();

private:
    void createInstance();
	
    bool checkValidationLayerSupport();

    void createSurface();

    VulkanCommon::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pDevice);
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice pDevice);
    bool checkDeviceExtensionSupport(VkPhysicalDevice pDevice);
	
    void createLogicalDevice();
};

#endif