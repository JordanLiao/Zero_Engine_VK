#include "VulkanCommandPool.h"

#include <stdexcept>

VulkanCommandPool::VulkanCommandPool() {}

VulkanCommandPool::VulkanCommandPool(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex, const VkDevice& logicalDevice) {
    this->logicalDevice = logicalDevice;
    vkGetDeviceQueue(logicalDevice, queueFamilyIndex, 0, &(this->queue));

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = flags;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");
}

void VulkanCommandPool::cleanUp() {
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
}

void VulkanCommandPool::createCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, VkCommandBufferLevel level) const{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    allocInfo.commandPool = commandPool;
    allocInfo.level = level;

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");
}

void VulkanCommandPool::freeCommandBuffers(VkCommandBuffer* commandBuffers, uint32_t count) const {
    vkFreeCommandBuffers(logicalDevice, commandPool, count, commandBuffers);
}
