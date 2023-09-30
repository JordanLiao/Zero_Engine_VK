#include "VulkanCommon.h"

PFN_vkGetPhysicalDeviceProperties2KHR VulkanCommon::vkGetPhysicalDeviceProperties2KHR = VK_NULL_HANDLE;

void VulkanCommon::init(const VkDevice& logicalDevice, const VkInstance& instance) {
    vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR"));
}
