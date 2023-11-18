#ifndef _VULKANDESCRIPTORSET_H_
#define _VULKANDESCRIPTORSET_H_

#include "VulkanDescriptor.h"

#include <vulkan/vulkan.h>
#include <vector>

class VulkanDescriptorSet {
public:
    uint32_t allocIndex;
    VkDescriptorSetLayout layout;

    VkDeviceAddress* allocAddress; //pointer to the allocator's device memory
    VkDeviceSize setOffset; //offset of this descSet within the allocator's memory 

    std::vector<VkDeviceSize> bindingOffsets;
    std::vector<std::vector<VulkanDescriptor>> descriptors;
};

#endif
