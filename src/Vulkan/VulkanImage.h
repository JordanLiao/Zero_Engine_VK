#ifndef _VULKANIMAGE_H_
#define _VULKANIMAGE_H_

#include <vulkan/vulkan.h>
#include "VulkanCommandPool.h"

#include <vector>

class VulkanContext;

struct VulkanImage {
    VkImage vkImage = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;
    VkImageView vkImageView = VK_NULL_HANDLE;

    VulkanImage();
    VulkanImage(VkDevice);

    VulkanImage(VulkanImage&& img) noexcept;
    VulkanImage& operator=(VulkanImage&& img) noexcept;

    void cleanup();
    ~VulkanImage();

private:
    VkDevice logicalDevice = VK_NULL_HANDLE;
};

namespace VulkanImageUtils {

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkImageLayout oldLayout,
                            VkImageLayout newLayout, const VulkanCommandPool& commandPool);

    VulkanImage createImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                              VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VulkanContext* context);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkDevice logicalDevice);

    void transitionImageLayout(VkImage image, VkImageLayout newLayout, VkImageLayout oldLayout, VkCommandBuffer commandBuffer);

    bool hasStencilComponent(VkFormat format);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features, VkPhysicalDevice pDevice);

    VkSampler createSampler(VkFilter filter, VkSamplerAddressMode addressMode, VulkanContext* context);
}

#endif
