#ifndef _VULKANCOMMON_H_
#define _VULKANCOMMON_H_

#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

namespace VulkanCommon {
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
}

#endif