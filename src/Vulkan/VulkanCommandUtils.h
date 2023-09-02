#ifndef _VULKANCOMMANDUTILS_H_
#define _VULKANCOMMANDUTILS_H_

#include <vulkan/vulkan.h>

#include "VulkanCommandPool.h"

namespace VulkanCommandUtils {
	VkCommandBuffer beginSingleTimeCommands(VulkanCommandPool& commandPool);
	void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue, VulkanCommandPool& commandPool);
}

#endif