#ifndef _VULKANBUFFERUTILS_H_
#define _VULKANBUFFERUTILS_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "../Resources/GraphicsBuffers.h"

class VulkanCommandPool;
class VulkanBuffer;
class VulkanBufferArray;
class VulkanContext;

class VulkanBufferUtils {
public:
    static VulkanBuffer createVulkanDataBuffer(void* srcData, VkDeviceSize size, VkBufferUsageFlags usageFlags,
                                            VkMemoryPropertyFlags propertyFlags, VulkanCommandPool& commandPool,
                                             VulkanContext* context);

    static void copyBuffer(VulkanBuffer& dstBuffer, VulkanBuffer& srcBuffer, VulkanCommandPool& commandPool);

    /*
        Create a Vulkan buffer from a loaded index buffer.
    */
    static VulkanBuffer createIndexBuffer(const IndexBuffer& indexBuffer, VulkanCommandPool& commandPool, VulkanContext* context);

    /*
        Create a VulkanBufferArray from a loaded vertex buffer that may have several different attribute types.
    */
    static VulkanBufferArray createVertexBuffers(const VertexBuffer& vertexBuffers, VulkanCommandPool& commandPool, VulkanContext* context);
    
    static uint32_t getAlignedBufferSize(size_t offset, size_t alignment);

    static VkDeviceAddress getBufferDeviceAddress(VkBuffer buffer, VulkanContext* context);

    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VulkanContext* context);
};

#endif