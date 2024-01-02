#ifndef _VULKANBUFFER_
#define _VULKANBUFFER_

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "GLM/glm.hpp"

#include <vector>

namespace VulkanVertexBufferInfo {
    /*
    * binding is the indices of the buffers
    * binding 0: position buffer; binding 1: normal buffer; binding 2: texCoords buffer
    */
    const std::vector<VkVertexInputBindingDescription> vertexBindings = {
        {//vertex
            0,                          // binding
            sizeof(glm::vec3),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
        {//normal
            1,                          // binding
            sizeof(glm::vec3),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
        {//texture
            2,                          // binding
            sizeof(glm::vec2),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
        {//tangent
            3,                          // binding
            sizeof(glm::vec3),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
        {//bitangent
            4,                          // binding
            sizeof(glm::vec3),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
    };

    const std::vector<VkVertexInputBindingDescription> clothVertexBindings = {
        {//vertex
            0,                          // binding
            sizeof(glm::vec4),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
        {//normal
            1,                          // binding
            sizeof(glm::vec3),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
        {//texture
            2,                          // binding
            sizeof(glm::vec2),          // stride
            VK_VERTEX_INPUT_RATE_VERTEX // inputRate, not per instance
        },
    };

    /**
    * location specifies which location in the shader an attribute can be accessed.
    * binding specifies the index of the buffer that store the attribute's data.
    * NOTE: two attributes be use the same binding index if their data are batched into the same
    * buffer, but they still need different locations.
    */
    const std::vector<VkVertexInputAttributeDescription> vertexAttributes = {
        {
            0,                          // location
            0,                          // binding
            VK_FORMAT_R32G32B32_SFLOAT, // format
            0                           // offset
        },
        {
            1,                          // location
            1,                          // binding
            VK_FORMAT_R32G32B32_SFLOAT, // format
            0                           // offset
        },
        {
            2,                          // location
            2,                          // binding
            VK_FORMAT_R32G32_SFLOAT,    // format
            0
        },
        {
            3,                          // location
            3,                          // binding
            VK_FORMAT_R32G32B32_SFLOAT,    // format
            0
        },
        {
            4,                          // location
            4,                          // binding
            VK_FORMAT_R32G32B32_SFLOAT,    // format
            0
        },
    };

    const std::vector<VkVertexInputAttributeDescription> clothVertexAttributes = {
        {
            0,                          // location
            0,                          // binding
            VK_FORMAT_R32G32B32A32_SFLOAT, // format
            0                           // offset
        },
        {
            1,                          // location
            1,                          // binding
            VK_FORMAT_R32G32B32_SFLOAT, // format
            0                           // offset
        },
        {
            2,                          // location
            2,                          // binding
            VK_FORMAT_R32G32_SFLOAT,    // format
            0
        },
    };
}

class VulkanContext;

class VulkanBuffer {
public:
    VkBuffer vkBuffer;
    void* data;
    VkDeviceSize hostSize;

    VulkanBuffer();
    VulkanBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags properties, VulkanContext* context);
    VulkanBuffer(VkDeviceSize size, VkBufferCreateFlags createFlags ,VkBufferUsageFlags usageFlags, 
                 VkMemoryPropertyFlags properties, VulkanContext* context);

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

class VulkanBufferArray {
public:
    std::vector<VulkanBuffer> buffers;
    std::vector<VkBuffer> vkBuffers;

    VulkanBufferArray();

    void addBuffer(const VulkanBuffer& buffer);
    void cleanUp();
};

#endif
