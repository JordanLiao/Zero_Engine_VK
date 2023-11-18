#ifndef _VULKANBUFFER_
#define _VULKANBUFFER_

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <vector>

namespace VulkanVertexBufferInfo {
    const std::vector<VkVertexInputBindingDescription> vertexBindings = {
        {
            0,                          // binding
            3 * sizeof(float),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
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

class VulkanContext;

class VulkanBuffer {
public:
    VkBuffer vkBuffer;
    void* data;
    VkDeviceSize hostSize;

    VulkanBuffer();
    VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VulkanContext* context);

    //Maps the bound device memory to the data pointer handle
    VkResult map();
    //Unmaps the data pointer handle
    void unmap();

    void transferData(const void* src, size_t size);
    void cleanUp();
 
private:
    bool mapped = false;
    VulkanContext* context;
    VmaAllocation allocation;

    static int bufferCount;
};

#endif
