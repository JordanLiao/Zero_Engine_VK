#include "VulkanBufferUtils.h"
#include "VulkanCommandUtils.h"
#include "VulkanCommandPool.h"
#include "VulkanBuffer.h"
#include "VulkanContext.h"

#include "GLM/glm.hpp"

#include <stdexcept>

uint32_t VulkanBufferUtils::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VulkanContext* context) {

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(context->physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanBufferUtils::copyBuffer(VulkanBuffer& dstBuffer, VulkanBuffer& srcBuffer, VulkanCommandPool& commandPool) {
    VkCommandBuffer commandBuffer = VulkanCommandUtils::beginSingleTimeCommands(commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = srcBuffer.hostSize;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.vkBuffer, dstBuffer.vkBuffer, 1, &copyRegion);

    VulkanCommandUtils::endSingleTimeCommands(commandBuffer, commandPool);
}

VulkanBuffer VulkanBufferUtils::createVulkanDataBuffer(void* srcData, VkDeviceSize size, VkBufferUsageFlags usageFlags,
                                                    VkMemoryPropertyFlags propertyFlags, VulkanCommandPool& commandPool,
                                                    VulkanContext* context) {
    VulkanBuffer stagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                               context);
    stagingBuffer.map();
    stagingBuffer.transferData(srcData, (size_t)size);
    stagingBuffer.unmap();

    VulkanBuffer buffer(size, usageFlags, propertyFlags, context);
    copyBuffer(buffer, stagingBuffer, commandPool);
    stagingBuffer.cleanUp();

    return buffer;
}

VulkanBuffer VulkanBufferUtils::createIndexBuffer(const IndexBuffer& indexBuffer, VulkanCommandPool& commandPool, 
                                                  VulkanContext* context) {
    if (indexBuffer.triangles.size() == 0)
        return VulkanBuffer{};
    VkDeviceSize bufferSize = sizeof(glm::ivec3) * indexBuffer.triangles.size();

    return createVulkanDataBuffer((void*)indexBuffer.triangles.data(), bufferSize, 
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, commandPool, context);
}

VulkanBufferArray VulkanBufferUtils::createVertexBuffers(const VertexBuffer& vertexBuffers, VulkanCommandPool& commandPool, 
                                                         VulkanContext* context) {
    VulkanBufferArray result;
    if(vertexBuffers.positions.size() > 0) {
        VkDeviceSize bufferSize = sizeof(vertexBuffers.positions[0]) * vertexBuffers.positions.size();
        result.addBuffer(createVulkanDataBuffer((void*)vertexBuffers.positions.data(), bufferSize,
                                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, commandPool, context));
    }

    if(vertexBuffers.normals.size() > 0) {
        VkDeviceSize bufferSize = sizeof(vertexBuffers.normals[0]) * vertexBuffers.normals.size();
        result.addBuffer(createVulkanDataBuffer((void*)vertexBuffers.normals.data(), bufferSize,
                                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, commandPool, context));
    }

    if (vertexBuffers.texCoords.size() > 0) {
        VkDeviceSize bufferSize = sizeof(vertexBuffers.texCoords[0]) * vertexBuffers.texCoords.size();
        result.addBuffer(createVulkanDataBuffer((void*)vertexBuffers.texCoords.data(), bufferSize,
                                                VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, commandPool, context));
    }

    if (vertexBuffers.tangents.size() > 0) {
        VkDeviceSize bufferSize = sizeof(vertexBuffers.tangents[0]) * vertexBuffers.tangents.size();
        result.addBuffer(createVulkanDataBuffer((void*)vertexBuffers.tangents.data(), bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, commandPool, context));
    }

    if (vertexBuffers.bitangents.size() > 0) {
        VkDeviceSize bufferSize = sizeof(vertexBuffers.bitangents[0]) * vertexBuffers.bitangents.size();
        result.addBuffer(createVulkanDataBuffer((void*)vertexBuffers.bitangents.data(), bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, commandPool, context));
    }

    return result;
}

//based on Sascha Willems' method
uint32_t VulkanBufferUtils::getAlignedBufferSize(size_t offset, size_t alignment) {
    size_t alignedOffset = offset;
    if (alignment > 0) {
        alignedOffset = (alignedOffset + alignment - 1) & ~(alignment - 1);
    }
    return (uint32_t)alignedOffset;
}

VkDeviceAddress VulkanBufferUtils::getBufferDeviceAddress(VkBuffer buffer, VulkanContext* context) {
    VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = buffer;
    return context->vkGetBufferDeviceAddressKHR(context->logicalDevice, &bufferDeviceAI);
}