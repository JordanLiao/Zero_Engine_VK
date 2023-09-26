#ifndef _VULKANBUFFERUTILS_H_
#define _VULKANBUFFERUTILS_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "GraphicsBuffers.h"

class VulkanCommandPool;
class VulkanBuffer;
class VulkanBufferArray;

class VulkanBufferUtils {
public:
    static void init(VkDevice lDevice, VkPhysicalDevice pdevice, const VulkanCommandPool& cPool);
    static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    static void copyBuffer(VulkanBuffer& dstBuffer, VulkanBuffer& srcBuffer);
    static VulkanBuffer createVulkanBuffer(void* srcData, VkDeviceSize size,
                                            VkBufferUsageFlags usageFlags,
                                            VkMemoryPropertyFlags propertyFlags);

    /*
        Create a Vulkan buffer from a loaded index buffer.
    */
    static VulkanBuffer createIndexBuffer(const IndexBuffer& indexBuffer);

    /*
        Create a VulkanBufferArray from a loaded vertex buffer that may have several different attribute types.
    */
    static VulkanBufferArray createVertexBuffers(VertexBuffer& vertexBuffers);
    
    static uint32_t getAlignedBufferSize(size_t bufferSize, size_t alignment);

    static uint64_t getBufferDeviceAddress(VkBuffer buffer);

    static void cleanup();

private:
    static PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
    static bool initialized;
    static VkDevice logicalDevice;
    static VkPhysicalDevice physicalDevice;
    static VulkanCommandPool commandPool;
};

#endif