#ifndef _VULKANCOMMANDPOOL_H_
#define _VULKANCOMMANDPOOL_H_

#include <vulkan/vulkan.h>
#include <vector>

class VulkanCommandPool {
public:
    VkQueue queue;
    VkCommandPool commandPool;

    VulkanCommandPool();
    VulkanCommandPool(VkCommandPoolCreateFlags flags, uint32_t queueFamilyIndex, const VkDevice& logicalDevice);
    void createCommandBuffers(std::vector<VkCommandBuffer>& commandBuffers, VkCommandBufferLevel level) const;
    void freeCommandBuffers(VkCommandBuffer* commandBuffers, uint32_t count) const;
    void cleanUp();
    
private:
    VkDevice logicalDevice;
};

#endif
