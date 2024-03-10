#ifndef _VULKANCONTEXT_H_
#define _VULKANCONTEXT_H_

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

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

struct GLFWwindow;

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME,
    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
};

const std::vector<const char*> instanceExtensions = {
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
};

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<VkValidationFeatureEnableEXT> validationFeatures = {
    VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
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
    VkQueue computeQueue;
    VulkanCommon::QueueFamilyIndices queueFamilyIndices;

    bool resized; //whether window has resized or not
    GLFWwindow* window;

    VmaAllocator vmAlloc;

    //properties for different types of descriptors.
    VkPhysicalDeviceDescriptorBufferPropertiesEXT* descBufferProps;
    //properties of the physical device being used
    VkPhysicalDeviceProperties *physicalDeviceProps;

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR = VK_NULL_HANDLE;
    PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSizeEXT = VK_NULL_HANDLE;
    PFN_vkGetDescriptorEXT vkGetDescriptorEXT = VK_NULL_HANDLE;
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR = VK_NULL_HANDLE;
    PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffsetEXT = VK_NULL_HANDLE;

    VulkanContext();
    VulkanContext(GLFWwindow* window);
    void cleanUp();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

private:
    void createInstance();
    void initVMA();
	
    VkDebugUtilsMessengerEXT debugMessenger;
    bool checkValidationLayerSupport();
    void setupDebugMessenger();
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                   const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    void createSurface();

    VulkanCommon::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pDevice);
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice pDevice);
    bool checkDeviceExtensionSupport(VkPhysicalDevice pDevice);
	
    void createLogicalDevice();
};

#endif