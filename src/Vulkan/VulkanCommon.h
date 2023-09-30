#ifndef _VULKANCOMMON_H_
#define _VULKANCOMMON_H_

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

class VulkanCommon {
public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;

        bool isComplete() {
            return graphicsFamily.has_value() &&
                presentFamily.has_value() &&
                transferFamily.has_value();
        }
    };

    static PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR;;
    static void init(const VkDevice& logicalDevice, const VkInstance& instance);

};

#endif