#include "VulkanBuffer.h"

#include "VulkanBufferUtils.h"

#include <stdexcept>
#include <iostream>

VulkanBuffer::VulkanBuffer() {
    vkBuffer = VK_NULL_HANDLE;
    bufferMemory = VK_NULL_HANDLE;
    logicalDevice = VK_NULL_HANDLE;
}

VulkanBuffer::VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
                           VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice){
    this->size = size;
    data = nullptr;
    this->logicalDevice = logicalDevice;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &vkBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, vkBuffer, &memRequirements);

    deviceSize = memRequirements.size;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanBufferUtils::findMemoryType(memRequirements.memoryTypeBits, properties);

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkMemoryAllocateFlagsInfo flagsInfo{};
        flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        allocInfo.pNext = &flagsInfo;
    }

    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    if (vkBindBufferMemory(logicalDevice, vkBuffer, bufferMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("failed to bind VkBuffer to VkMemory!");
    }
}

VkResult VulkanBuffer::map() {
    VkResult result = vkMapMemory(logicalDevice, bufferMemory, 0, size, 0, &data);
    return result;
}

void  VulkanBuffer::unmap(){
    vkUnmapMemory(logicalDevice, bufferMemory);
}

void VulkanBuffer::transferData(const void* src, size_t size) {
    memcpy(data,src, size);
}

void VulkanBuffer::cleanup() {
    vkDestroyBuffer(logicalDevice, vkBuffer, nullptr);
    vkFreeMemory(logicalDevice, bufferMemory, nullptr);
}
