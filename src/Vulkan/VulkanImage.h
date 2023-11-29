#ifndef _VULKANIMAGE_H_
#define _VULKANIMAGE_H_

#include <vulkan/vulkan.h>
#include "VulkanCommandPool.h"

#include <vector>

class VulkanContext;

struct VulkanImage {
    VkImage vkImage;
    VkDeviceMemory vkDeviceMemory;
    VkImageView vkImageView;
};

namespace VulkanImageUtils {

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkImageLayout oldLayout,
                            VkImageLayout newLayout, const VulkanCommandPool& commandPool);

    void createImage2D(VulkanImage& image, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
                       VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VulkanContext* context);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkDevice logicalDevice);

    void transitionImageLayout(VkImage image, VkImageLayout newLayout, VkImageLayout oldLayout, VkCommandBuffer commandBuffer);

    bool hasStencilComponent(VkFormat format);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features, VkPhysicalDevice pDevice);

    VkSampler createSampler(VkFilter filter, VkSamplerAddressMode addressMode, VulkanContext* context);
}

#endif
