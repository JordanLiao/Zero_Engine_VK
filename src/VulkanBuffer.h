#ifndef _VULKANBUFFER_
#define _VULKANBUFFER_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanContext.h"

namespace VulkanVertexBufferInfo {
    const std::vector<VkVertexInputBindingDescription> vertexBindings = {
        {
            0,                          // binding
            3 * sizeof(float),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
        {
            1,                          // bindin
            3 * sizeof(float),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate
        }
    };

    const std::vector<VkVertexInputAttributeDescription> vertexAttributes = {
        {
            0,                          // location
            vertexBindings[0].binding,  // binding
            VK_FORMAT_R32G32B32_SFLOAT, // format
            0                           // offset
        },
        {
            1,                          // location
            vertexBindings[0].binding,  // binding
            VK_FORMAT_R32G32B32_SFLOAT, // format
            0                           // offset
        }
    };
}

class VulkanBuffer {
public:
    VkDeviceSize size = 0;
    VkBuffer buffer;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags properties;

    VulkanBuffer();
    VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory, VulkanContext& context);
    void cleanup();
private:
    VkDevice logicalDevice;
    VkDeviceMemory bufferMemory;
};

namespace VulkanBufferUtils {
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice pDevice);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
}

#endif
