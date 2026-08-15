#ifndef _VULKANCONTEXT_H_
#define _VULKANCONTEXT_H_

#define VK_USE_PLATFORM_WIN32_KHR
//#define GLFW_INCLUDE_VULKAN
//#include "GLFW/glfw3.h"
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "GLM/glm.hpp"

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
    //VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
};

class VulkanContext {
public:
    VkDevice getLogicalDevice();
    VkPhysicalDevice getPhysicalDevice();
    VkInstance getInstance();
    VkSurfaceKHR getSurface();

    VkQueue getGraphicsQueue();
    VkQueue getPresentQueue();
    VkQueue getTransferQueue();
    VkQueue getComputeQueue();
    VulkanCommon::QueueFamilyIndices getQueueFamilyIndices();

    GLFWwindow* getWindow();
    bool getWindowResized();
    void setWindowResized(bool);

    VmaAllocator getVMAAlloc();

    //properties of the physical device being used
    VkPhysicalDeviceProperties *physicalDeviceProps;

    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR = VK_NULL_HANDLE;
    PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSizeEXT = VK_NULL_HANDLE;
    PFN_vkGetDescriptorEXT vkGetDescriptorEXT = VK_NULL_HANDLE;
    PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR = VK_NULL_HANDLE;
    PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffsetEXT = VK_NULL_HANDLE;

    VulkanContext();
    VulkanContext(GLFWwindow* window);
    ~VulkanContext();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);

private:
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkQueue transferQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE ;
    VulkanCommon::QueueFamilyIndices queueFamilyIndices;

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;

    bool windowResized; //whether window has resized or not
    GLFWwindow* window = nullptr;

    void createInstance();

    VmaAllocator vmAlloc = nullptr;
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