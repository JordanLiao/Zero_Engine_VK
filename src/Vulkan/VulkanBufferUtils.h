#ifndef _VULKANBUFFERUTILS_H_
#define _VULKANBUFFERUTILS_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanCommandPool.h"
#include "VulkanBuffer.h"
#include "VulkanBufferArray.h"
#include "VulkanContext.h"

#include "GLM/glm.hpp"
#include "../Resources/VertexBuffer.h"
#include "../Resources/IndexBuffer.h"

namespace VulkanBufferUtils {
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice pDevice);
    void copyBuffer(VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer, VulkanCommandPool& commandPool);
    VulkanBuffer* createVulkanBuffer(void* srcData, VkDeviceSize size,
                                     VkBufferUsageFlags usageFlags,
                                     VkMemoryPropertyFlags propertyFlags,
                                     VulkanCommandPool& commandPool,
                                     VulkanContext& context);

    /*
        Create a Vulkan buffer from a loaded index buffer.
    */
    VulkanBuffer* createIndexBuffer(const IndexBuffer& indexBuffer, VulkanCommandPool& commandPool, VulkanContext& context);

    /*
        Create a VulkanBufferArray from a loaded vertex buffer that may have several different attribute types.
    */
    VulkanBufferArray createVertexBuffers(VertexBuffer& vertexBuffers, VulkanCommandPool& commandPool, 
                                          VulkanContext& context);
}

#endif