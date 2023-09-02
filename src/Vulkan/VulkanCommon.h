#ifndef _VULKANCOMMON_H_
#define _VULKANCOMMON_H_

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

namespace VulkanCommon {
	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
}

#endif