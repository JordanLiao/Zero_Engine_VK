#ifndef _VULKANDESCRIPTORBUFFER_H_
#define _VULKANDESCRIPTORBUFFER_H_

#include "VulkanBuffer.h"

#include <vulkan/vulkan.h>

struct VulkanDescriptorBuffer {
    VulkanBuffer buffer;
    VkDeviceAddress deviceAddress;
    
    void cleanUp() {
        buffer.cleanUp();
    }
};

#endif
