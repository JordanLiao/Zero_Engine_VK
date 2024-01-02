#include "VulkanBuffer.h"
#include "VulkanContext.h"

#include "VulkanBufferUtils.h"

#include <stdexcept>
#include <iostream>

int VulkanBuffer::bufferCount = 0;

VulkanBuffer::VulkanBuffer() : mapped(false), vkBuffer(VK_NULL_HANDLE), context(nullptr), data(nullptr),
                               hostSize(0), allocation(nullptr){}

VulkanBuffer::VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags properties, 
                           VulkanContext* context){
    mapped = false;
    this->hostSize = size;
    data = nullptr;
    this->context = context;

    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    allocInfo.requiredFlags = properties;

    vmaCreateBuffer(context->vmAlloc, &bufferInfo, &allocInfo, &vkBuffer, &allocation, nullptr);
    bufferCount++;
}

VulkanBuffer::VulkanBuffer(VkDeviceSize size, VkBufferCreateFlags createFlags, VkBufferUsageFlags usageFlags, 
                           VkMemoryPropertyFlags properties, VulkanContext* context) {
    mapped = false;
    this->hostSize = size;
    data = nullptr;
    this->context = context;

    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = usageFlags;
    bufferInfo.flags = createFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    allocInfo.requiredFlags = properties;

    vmaCreateBuffer(context->vmAlloc, &bufferInfo, &allocInfo, &vkBuffer, &allocation, nullptr);
    bufferCount++;
}

VkResult VulkanBuffer::map() {
    if (mapped)
        return VK_SUCCESS;

    VkResult res = vmaMapMemory(context->vmAlloc, allocation, &data);
    if (res == VK_SUCCESS)
        mapped = true;

    return res;
}

void  VulkanBuffer::unmap(){
    if (!mapped)
        return;

    vmaUnmapMemory(context->vmAlloc, allocation);
    mapped = false;
}

void VulkanBuffer::transferData(const void* src, size_t size) {
    if (size > hostSize)
        throw std::runtime_error("Cannot transfer more data than that is mapped on device!");
    memcpy(data,src, size);
}

void VulkanBuffer::cleanUp() {
    unmap();
    vmaDestroyBuffer(context->vmAlloc, vkBuffer, allocation);
    bufferCount--;
}

VulkanBufferArray::VulkanBufferArray() {}

void VulkanBufferArray::addBuffer(const VulkanBuffer& buffer) {
    buffers.push_back(buffer);
    vkBuffers.push_back(buffer.vkBuffer);
}

void VulkanBufferArray::cleanUp() {
    for (VulkanBuffer buffer : buffers) {
        buffer.cleanUp();
    }
}
