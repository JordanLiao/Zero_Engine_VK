#ifndef _VULKANCOMMANDPOOL_H_
#define _VULKANCOMMANDPOOL_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanContext.h"

class VulkanCommandPool {
public:
	VkQueue queue;
	VkCommandPool commandPool;

	VulkanCommandPool();
	VulkanCommandPool(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex, VulkanContext& context);
	void createCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, VkCommandBufferLevel level);
	void freeCommandBuffers(VkCommandBuffer* commandBuffers, uint32_t count);
	void cleanup();

private:
	VkDevice logicalDevice;
};

#endif
