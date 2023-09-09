#include "VulkanCommandUtils.h"

#include <vector>

VkCommandBuffer VulkanCommandUtils::beginSingleTimeCommands(VulkanCommandPool& commandPool) {
    std::vector<VkCommandBuffer> singleCommandBuffer(1);
    commandPool.createCommandBuffers(singleCommandBuffer, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(singleCommandBuffer[0], &beginInfo);

    return singleCommandBuffer[0];
}

void VulkanCommandUtils::endSingleTimeCommands(VkCommandBuffer commandBuffer, VulkanCommandPool& commandPool) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(commandPool.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(commandPool.queue);

    commandPool.freeCommandBuffers(&commandBuffer, 1);
}
