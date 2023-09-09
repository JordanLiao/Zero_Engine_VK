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

void VulkanBufferUtils::copyBuffer(VulkanBuffer& srcBuffer, VulkanBuffer& dstBuffer, VulkanCommandPool& commandPool) {
    VkCommandBuffer commandBuffer = VulkanCommandUtils::beginSingleTimeCommands(commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = srcBuffer.size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    VulkanCommandUtils::endSingleTimeCommands(commandBuffer, commandPool);
}

VulkanBuffer* VulkanBufferUtils::createVulkanBuffer(void* srcData, VkDeviceSize size, 
                                                    VkBufferUsageFlags usageFlags,
                                                    VkMemoryPropertyFlags propertyFlags,
                                                    VulkanCommandPool& commandPool,
                                                    VulkanContext& context) {
    VulkanBuffer stagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context);

    stagingBuffer.map();
    stagingBuffer.transferData(srcData, (size_t)size);
    stagingBuffer.unmap();

    VulkanBuffer* buffer = new VulkanBuffer(size, usageFlags,propertyFlags, context);
    copyBuffer(stagingBuffer, *buffer, commandPool);
    stagingBuffer.cleanup();

    return buffer;
}

VulkanBuffer* VulkanBufferUtils::createIndexBuffer(const IndexBuffer& indexBuffer, VulkanCommandPool& commandPool, 
                                                   VulkanContext& context) {
    if (indexBuffer.triangles.size() == 0)
        return nullptr;
    VkDeviceSize bufferSize = sizeof(glm::ivec3) * indexBuffer.triangles.size();

    return createVulkanBuffer((void*)indexBuffer.triangles.data(), bufferSize, 
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                              commandPool, context);
}

VulkanBufferArray VulkanBufferUtils::createVertexBuffers(VertexBuffer& vertexBuffers, VulkanCommandPool& commandPool,
                                                                  VulkanContext& context) {
    VulkanBufferArray result{};
    VkDeviceSize bufferSize;
    if(vertexBuffers.positions.size() > 0) {
        bufferSize = sizeof(vertexBuffers.positions[0]) * vertexBuffers.positions.size();
        result.buffers.push_back(createVulkanBuffer((void*)vertexBuffers.positions.data(),
                                                    bufferSize,
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    commandPool, context));
        result.vulkanBuffers.push_back(result.buffers.back()->buffer); //need a separate VkBuffer handle
    }

    if(vertexBuffers.normals.size() > 0) {
        bufferSize = sizeof(vertexBuffers.normals[0]) * vertexBuffers.normals.size();
        result.buffers.push_back(createVulkanBuffer((void*)vertexBuffers.normals.data(),
                                                    bufferSize,
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    commandPool, context));
        result.vulkanBuffers.push_back(result.buffers.back()->buffer);
    }

    return result;
}
