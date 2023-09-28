#ifndef _VULKANIMAGE_H_
#define _VULKANIMAGE_H_

#include <vulkan/vulkan.h>

#include <vector>

class VulkanCommandPool;

struct VulkanImage {
    uint32_t width, height;
    VkImage vkImage;
    VkDeviceMemory vkDeviceMemory;
    VkImageView vkImageView;

    void cleanup();
};

namespace VulkanImageUtils {
    void createImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VulkanImage& image, VkDevice logicalDevice);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkDevice logicalDevice);

    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
                               VkCommandBuffer commandBuffer);

    bool hasStencilComponent(VkFormat format);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features, VkPhysicalDevice pDevice);
}

#endif
