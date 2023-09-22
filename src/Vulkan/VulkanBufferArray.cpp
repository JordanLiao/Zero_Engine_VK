
#include "VulkanBufferArray.h"

VulkanBufferArray::VulkanBufferArray() {}

VulkanBufferArray::VulkanBufferArray(uint32_t count, VkDeviceSize size, VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags properties,
                                     VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice) {
    buffers.reserve(count);
    vkBuffers.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        VulkanBuffer buffer(VulkanBuffer(size, usage, properties, logicalDevice, physicalDevice));
        buffers.push_back(buffer);
        vkBuffers.push_back(buffer.vkBuffer);
    }
}

void VulkanBufferArray::cleanup() {
    for (VulkanBuffer buffer : buffers) {
	    buffer.cleanup();
    }
}
