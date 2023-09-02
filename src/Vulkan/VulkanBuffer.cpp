#include "VulkanBuffer.h"

#include <stdexcept>
#include "VulkanBufferUtils.h"

VulkanBuffer::VulkanBuffer() {
    buffer = VK_NULL_HANDLE;
    bufferMemory = VK_NULL_HANDLE;
}

VulkanBuffer::VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, 
                           VulkanContext& context){
    this->logicalDevice = context.logicalDevice;
    this->size = size;
    this->usage = usage;
    this->properties = properties;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(context.logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = VulkanBufferUtils::findMemoryType(memRequirements.memoryTypeBits, properties, context.physicalDevice);

    if (vkAllocateMemory(context.logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(context.logicalDevice, buffer, bufferMemory, 0);
}

VkResult VulkanBuffer::map() {
    VkResult result = vkMapMemory(logicalDevice, bufferMemory, 0, size, 0, &data);
    mapped = (result == VK_SUCCESS);
    return result;
}

void  VulkanBuffer::unmap(){
    mapped = false;
    vkUnmapMemory(logicalDevice, bufferMemory);
}

void VulkanBuffer::transferData(const void* src, size_t size) {
    if (mapped)
        memcpy(data, src, size);
}

void VulkanBuffer::cleanup() {
    vkDestroyBuffer(logicalDevice, buffer, nullptr);
    vkFreeMemory(logicalDevice, bufferMemory, nullptr);
}
