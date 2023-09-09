#ifndef _VULKANBUFFERARRAY_H_
#define _VULKANBUFFERARRAY_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanBuffer.h"

class VulkanBufferArray {
public:
	std::vector<VulkanBuffer*> buffers;
	std::vector<VkBuffer> vulkanBuffers;

	VulkanBufferArray();
	~VulkanBufferArray();
	void cleanup();
};

#endif