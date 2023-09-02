#include "VulkanBufferUtils.h"
#include "VulkanCommandUtils.h"

#include <stdexcept>

uint32_t VulkanBufferUtils::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice pDevice) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(pDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanBufferUtils::copyBuffer(VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer, VkQueue queue, 
                                   VulkanCommandPool& commandPool) {
    VkCommandBuffer commandBuffer = VulkanCommandUtils::beginSingleTimeCommands(commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = srcBuffer.size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    VulkanCommandUtils::endSingleTimeCommands(commandBuffer, queue, commandPool);
}

VulkanBuffer* VulkanBufferUtils::createIndexBuffer(std::vector<glm::ivec3>& triangles, VulkanContext& context,
                                                   VulkanCommandPool& commandPool) {
    if (triangles.size() == 0)
        return nullptr;

    VkDeviceSize bufferSize = sizeof(glm::ivec3) * triangles.size();

    VulkanBuffer stagingBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context);

    stagingBuffer.map();
    stagingBuffer.transferData(triangles.data(), (size_t)bufferSize);
    stagingBuffer.unmap();

    VulkanBuffer* indexBuffer = new VulkanBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context);

    copyBuffer(stagingBuffer, *indexBuffer, context.graphicsQueue, commandPool);
    
    stagingBuffer.cleanup();

    return indexBuffer;
}
