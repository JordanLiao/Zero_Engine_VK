#include "VulkanCommon.h"

PFN_vkGetDescriptorSetLayoutSizeEXT VulkanCommon::vkGetDescriptorSetLayoutSizeEXT = VK_NULL_HANDLE;
PFN_vkGetDescriptorSetLayoutBindingOffsetEXT VulkanCommon::vkGetDescriptorSetLayoutBindingOffsetEXT = VK_NULL_HANDLE;
PFN_vkCmdBindDescriptorBuffersEXT VulkanCommon::vkCmdBindDescriptorBuffersEXT = VK_NULL_HANDLE;
PFN_vkCmdSetDescriptorBufferOffsetsEXT VulkanCommon::vkCmdSetDescriptorBufferOffsetsEXT = VK_NULL_HANDLE;
PFN_vkGetPhysicalDeviceProperties2KHR VulkanCommon::vkGetPhysicalDeviceProperties2KHR = VK_NULL_HANDLE;
PFN_vkGetDescriptorEXT VulkanCommon::vkGetDescriptorEXT = VK_NULL_HANDLE;

void VulkanCommon::init(const VkDevice& logicalDevice, const VkInstance& instance) {
    vkGetDescriptorSetLayoutSizeEXT = reinterpret_cast<PFN_vkGetDescriptorSetLayoutSizeEXT>(
        vkGetDeviceProcAddr(logicalDevice, "vkGetDescriptorSetLayoutSizeEXT"));
    vkGetDescriptorSetLayoutBindingOffsetEXT = reinterpret_cast<PFN_vkGetDescriptorSetLayoutBindingOffsetEXT>(
        vkGetDeviceProcAddr(logicalDevice, "vkGetDescriptorSetLayoutBindingOffsetEXT"));
    vkCmdBindDescriptorBuffersEXT = reinterpret_cast<PFN_vkCmdBindDescriptorBuffersEXT>(
        vkGetDeviceProcAddr(logicalDevice, "vkCmdBindDescriptorBuffersEXT"));
    vkCmdSetDescriptorBufferOffsetsEXT = reinterpret_cast<PFN_vkCmdSetDescriptorBufferOffsetsEXT>(
        vkGetDeviceProcAddr(logicalDevice, "vkCmdSetDescriptorBufferOffsetsEXT"));
    vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(
        vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR"));
    vkGetDescriptorEXT = reinterpret_cast<PFN_vkGetDescriptorEXT>(vkGetDeviceProcAddr(logicalDevice,
        "vkGetDescriptorEXT"));
}
