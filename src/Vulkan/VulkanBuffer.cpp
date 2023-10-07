#include "VulkanBuffer.h"
#include "VulkanContext.h"

#include "VulkanBufferUtils.h"

#include <stdexcept>
#include <iostream>

VulkanBuffer::VulkanBuffer() {
    vkBuffer = VK_NULL_HANDLE;
    bufferMemory = VK_NULL_HANDLE;
}

VulkanBuffer::VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
                           VulkanContext* context){
    this->hostSize = size;
    data = nullptr;
    this->context = context;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context->logicalDevice, &bufferInfo, nullptr, &vkBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context->logicalDevice, vkBuffer, &memRequirements);

    deviceSize = memRequirements.size;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanBufferUtils::findMemoryType(memRequirements.memoryTypeBits, properties, context);

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkMemoryAllocateFlagsInfo flagsInfo{};
        flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        allocInfo.pNext = &flagsInfo;
    }

    if (vkAllocateMemory(context->logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    if (vkBindBufferMemory(context->logicalDevice, vkBuffer, bufferMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("failed to bind VkBuffer to VkMemory!");
    }
}

VkResult VulkanBuffer::map() {
    VkResult result = vkMapMemory(context->logicalDevice, bufferMemory, 0, hostSize, 0, &data);
    return result;
}

void  VulkanBuffer::unmap(){
    vkUnmapMemory(context->logicalDevice, bufferMemory);
}

void VulkanBuffer::transferData(const void* src, size_t size) {
    memcpy(data,src, size);
}

void VulkanBuffer::cleanUp() {
    vkDestroyBuffer(context->logicalDevice, vkBuffer, nullptr);
    vkFreeMemory(context->logicalDevice, bufferMemory, nullptr);
}
