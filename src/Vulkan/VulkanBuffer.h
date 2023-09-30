#ifndef _VULKANBUFFER_
#define _VULKANBUFFER_

#include <vulkan/vulkan.h>
#include <vector>

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
    VkBuffer vkBuffer;
    void* data;
    VkDeviceSize size;
    VkDeviceSize deviceSize;

    VulkanBuffer();
    VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                 VkDevice& logicalDevice, VkPhysicalDevice& physicalDevice);
    VkResult map();
    void unmap();
    void transferData(const void* src, size_t size);
    void cleanup();
 
private:
    VkDevice logicalDevice;
    VkDeviceMemory bufferMemory;
    //mapped device memory handle
};

#endif
