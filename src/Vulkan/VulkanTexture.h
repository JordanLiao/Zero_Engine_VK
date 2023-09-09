#ifndef _VULKANTEXTURE_H_
#define _VULKANTEXTURE_H_

#include <vulkan/vulkan.h>

#include "VulkanContext.h"
#include "VulkanCommandUtils.h"

namespace VulkanTextureUtils {
	VkImageView createImageView(VkImage image, VkDevice vDevice, VkFormat format, VkImageAspectFlags aspectFlags);

	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
		                       VulkanCommandPool& commandPool);

	bool hasStencilComponent(VkFormat format);
}

#endif
