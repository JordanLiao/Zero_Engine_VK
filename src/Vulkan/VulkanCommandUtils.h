#ifndef _VULKANCOMMANDUTILS_H_
#define _VULKANCOMMANDUTILS_H_

#include <vulkan/vulkan.h>

class VulkanCommandPool;

namespace VulkanCommandUtils {
    VkCommandBuffer beginSingleTimeCommands(VulkanCommandPool& commandPool);
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VulkanCommandPool& commandPool);
}

#endif