#include "VulkanBufferArray.h"
#include "VulkanBuffer.h"

VulkanBufferArray::VulkanBufferArray() {}

VulkanBufferArray::VulkanBufferArray(uint32_t count, VkDeviceSize size, VkBufferUsageFlags usage,
                                     VkMemoryPropertyFlags properties,
                                     VulkanContext* context) {
    buffers.reserve(count);
    vkBuffers.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        VulkanBuffer buffer(VulkanBuffer(size, usage, properties, context));
        buffers.push_back(buffer);
        vkBuffers.push_back(buffer.vkBuffer);
    }
}

void VulkanBufferArray::cleanup() {
    for (VulkanBuffer buffer : buffers) {
	    buffer.cleanup();
    }
}
