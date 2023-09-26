#include "VulkanBufferUtils.h"
#include "VulkanCommandUtils.h"
#include "VulkanCommandPool.h"
#include "VulkanBufferArray.h"
#include "VulkanBuffer.h"

#include "GLM/glm.hpp"

#include <stdexcept>

bool VulkanBufferUtils::initialized = false;
VkDevice VulkanBufferUtils::logicalDevice = VK_NULL_HANDLE;
VkPhysicalDevice VulkanBufferUtils::physicalDevice = VK_NULL_HANDLE;
VulkanCommandPool VulkanBufferUtils::commandPool;

PFN_vkGetBufferDeviceAddressKHR VulkanBufferUtils::vkGetBufferDeviceAddressKHR;

void VulkanBufferUtils::init(VkDevice lDevice, VkPhysicalDevice pdevice, const VulkanCommandPool& cPool) {
    if (cPool.commandPool == VK_NULL_HANDLE) {
        throw std::runtime_error("VulkanBufferUtils cannot initialize because provided CommandPool is not valid!");
    }
    logicalDevice = lDevice;
    physicalDevice = pdevice;
    commandPool = cPool;
    initialized = true;

    vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(logicalDevice, 
                                                                                    "vkGetBufferDeviceAddressKHR"));
}

uint32_t VulkanBufferUtils::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    if (!initialized) {
        throw std::runtime_error("VulkanBufferUtils is not initialized!");
    }

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanBufferUtils::copyBuffer(VulkanBuffer& dstBuffer, VulkanBuffer& srcBuffer) {
    if (!initialized) {
        throw std::runtime_error("VulkanBufferUtils is not initialized!");
    }

    VkCommandBuffer commandBuffer = VulkanCommandUtils::beginSingleTimeCommands(commandPool);

    VkBufferCopy copyRegion{};
    copyRegion.size = srcBuffer.size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.vkBuffer, dstBuffer.vkBuffer, 1, &copyRegion);

    VulkanCommandUtils::endSingleTimeCommands(commandBuffer, commandPool);
}

VulkanBuffer VulkanBufferUtils::createVulkanBuffer(void* srcData, VkDeviceSize size, 
                                                    VkBufferUsageFlags usageFlags,
                                                    VkMemoryPropertyFlags propertyFlags) {
    if (!initialized) {
        throw std::runtime_error("VulkanBufferUtils is not initialized!");
    }

    VulkanBuffer stagingBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                               logicalDevice, physicalDevice);
    stagingBuffer.map();
    stagingBuffer.transferData(srcData, (size_t)size);
    stagingBuffer.unmap();

    VulkanBuffer buffer(size, usageFlags, propertyFlags, logicalDevice, physicalDevice);
    copyBuffer(buffer, stagingBuffer);
    stagingBuffer.cleanup();

    return buffer;
}

VulkanBuffer VulkanBufferUtils::createIndexBuffer(const IndexBuffer& indexBuffer) {
    if (!initialized) {
        throw std::runtime_error("VulkanBufferUtils is not initialized!");
    }

    if (indexBuffer.triangles.size() == 0)
        return VulkanBuffer{};
    VkDeviceSize bufferSize = sizeof(glm::ivec3) * indexBuffer.triangles.size();

    return createVulkanBuffer((void*)indexBuffer.triangles.data(), bufferSize, 
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

VulkanBufferArray VulkanBufferUtils::createVertexBuffers(VertexBuffer& vertexBuffers) {
    if (!initialized) {
        throw std::runtime_error("VulkanBufferUtils is not initialized!");
    }

    VulkanBufferArray result;
    VkDeviceSize bufferSize;
    if(vertexBuffers.positions.size() > 0) {
        bufferSize = sizeof(vertexBuffers.positions[0]) * vertexBuffers.positions.size();
        result.buffers.push_back(createVulkanBuffer((void*)vertexBuffers.positions.data(),
                                                    bufferSize,
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        result.vkBuffers.push_back(result.buffers.back().vkBuffer); //need a separate VkBuffer handle
    }

    if(vertexBuffers.normals.size() > 0) {
        bufferSize = sizeof(vertexBuffers.normals[0]) * vertexBuffers.normals.size();
        result.buffers.push_back(createVulkanBuffer((void*)vertexBuffers.normals.data(),
                                                    bufferSize,
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
        result.vkBuffers.push_back(result.buffers.back().vkBuffer);
    }

    return result;
}

//based on Sascha Willems' method
uint32_t VulkanBufferUtils::getAlignedBufferSize(size_t bufferSize, size_t alignment) {
    size_t alignedSize = bufferSize;
    if (alignment > 0) {
        alignedSize = (alignedSize + alignment - 1) & ~(alignment - 1);
    }
    return (uint32_t)alignedSize;
}

uint64_t VulkanBufferUtils::getBufferDeviceAddress(VkBuffer buffer) {
    VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
    bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAI.buffer = buffer;
    return vkGetBufferDeviceAddressKHR(logicalDevice, &bufferDeviceAI);
}

void VulkanBufferUtils::cleanup() {
    commandPool.cleanup();
    initialized = false;
}