#ifndef _VULKANCOMMANDUTILS_H_
#define _VULKANCOMMANDUTILS_H_

#include <vulkan/vulkan.h>

class VulkanCommandPool;

namespace VulkanCommandUtils {
    VkCommandBuffer beginSingleTimeCommands(const VulkanCommandPool& commandPool);
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, const VulkanCommandPool& commandPool);
}

#endif