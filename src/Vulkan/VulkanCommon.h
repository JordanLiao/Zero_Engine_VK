#ifndef _VULKANCOMMON_H_
#define _VULKANCOMMON_H_

#include <optional>

class VulkanCommon {
public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> transferFamily;
        std::optional<uint32_t> computeFamily;

        bool isComplete() {
            return graphicsFamily.has_value() &&
                presentFamily.has_value() &&
                transferFamily.has_value() &&
                computeFamily.has_value();
        }
    };

};

#endif