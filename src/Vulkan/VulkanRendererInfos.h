#ifndef _VULKANRENDERERINFO_H_
#define _VULKANRENDERERINFO_H_

#include <vulkan/vulkan.h>
#include <vector>

#define FRAMES_IN_FLIGHT 2
#define GLOBAL_DESCSET_ALLOCATOR_BUFFER_SIZE 256

namespace VulkanRendererInfos {
    enum DescriptorSetRole {
        perFrameUniform,
        globalUniform,
        texSampler,
        numDescRoles
    };

    const std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutInfos = {
        {//There are MAX_FRAMES_IN_FLIGHT number of elements for the frames in flight
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, FRAMES_IN_FLIGHT, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE},
        },
        {//global ubos
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE}
        },
        {
            {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE},
        }
    };
}

#endif
