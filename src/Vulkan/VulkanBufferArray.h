#ifndef _VULKANBUFFERARRAY_H_
#define _VULKANBUFFERARRAY_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanBuffer.h"

class VulkanBufferArray {
public:
    std::vector<VulkanBuffer> buffers;
    std::vector<VkBuffer> vkBuffers;

    VulkanBufferArray();
    VulkanBufferArray(uint32_t count, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
                      VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice);

    void cleanup();
};

#endif