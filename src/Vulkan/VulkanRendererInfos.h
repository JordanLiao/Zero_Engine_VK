#ifndef _VULKANRENDERERINFO_H_
#define _VULKANRENDERERINFO_H_

#include <vulkan/vulkan.h>
#include <vector>

#define FRAMES_IN_FLIGHT 2
#define GLOBAL_DESCSET_ALLOCATOR_BUFFER_SIZE 256

namespace VulkanRendererInfos {

    const std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutInfos = {
        {
            //Using binding 0 for 
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000, VK_SHADER_STAGE_ALL,             VK_NULL_HANDLE},
            {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000, VK_SHADER_STAGE_ALL,             VK_NULL_HANDLE},
            {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000, VK_SHADER_STAGE_FRAGMENT_BIT,    VK_NULL_HANDLE},
        }
    };
}

#endif
