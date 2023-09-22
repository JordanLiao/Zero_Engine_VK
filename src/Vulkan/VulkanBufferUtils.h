#ifndef _VULKANBUFFERUTILS_H_
#define _VULKANBUFFERUTILS_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanCommandPool.h"
#include "VulkanBuffer.h"
#include "VulkanBufferArray.h"

#include "GraphicsBuffers.h"

class VulkanBufferUtils {
public:
    static void init(VkDevice lDevice, VkPhysicalDevice pdevice, const VulkanCommandPool& cPool);
    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    static void copyBuffer(VulkanBuffer& dstBuffer, VulkanBuffer& srcBuffer);
    static VulkanBuffer createVulkanBuffer(void* srcData, VkDeviceSize size,
                                            VkBufferUsageFlags usageFlags,
                                            VkMemoryPropertyFlags propertyFlags);

    /*
        Create a Vulkan buffer from a loaded index buffer.
    */
    static VulkanBuffer createIndexBuffer(const IndexBuffer& indexBuffer);

    /*
        Create a VulkanBufferArray from a loaded vertex buffer that may have several different attribute types.
    */
    static VulkanBufferArray createVertexBuffers(VertexBuffer& vertexBuffers);
    static void cleanup();

private:
    static bool initialized;
    static VkDevice logicalDevice;
    static VkPhysicalDevice physicalDevice;
    static VulkanCommandPool commandPool;
};

#endif