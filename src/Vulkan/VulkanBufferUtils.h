#ifndef _VULKANBUFFERUTILS_H_
#define _VULKANBUFFERUTILS_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanCommandPool.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "GLM/glm.hpp"

namespace VulkanBufferUtils {
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice pDevice);
    void copyBuffer(VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer, VkQueue queue, VulkanCommandPool& commandPool);
    VulkanBuffer* createIndexBuffer(std::vector<glm::ivec3>& triangles, VulkanContext& context, VulkanCommandPool& commandPool);
    void createVertexBuffer(std::vector<VulkanBuffer>);
}

#endif