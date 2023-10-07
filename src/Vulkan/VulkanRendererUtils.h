#ifndef _VULKANRENDERERUTILS_H_
#define _VULKANRENDERERUTILS_H_

#include "VulkanDescriptorBuffer.h"

#include "GLM/glm.hpp"
#include <vulkan/vulkan.h>

#include <vector>

//ordering of the descriptor set layouts by usage
enum DescriptorSetLayoutIndex {
    global,
    perFrame,
};

struct UniformBufferObject {
    glm::mat4 projView;
    glm::vec3 viewPos;
};

struct GlobalUniformBufferObject {
    glm::vec3 lightPosition;
    alignas(16) glm::vec3 light;
};

class VulkanContext;

namespace VulkanRendererUtils {
    /*
        descriptorSetLayoutInfos: structure is [num of descriptorSet Layout, num of bindings for each layout]
        descriptorSetLayouts: a vector of descriptor set layouts to be created.
        descriptorSetLayoutSizes: a vector of device memory sizes of the corresponding desciptor sets created.
    */
    void createDescriptorSetLayouts(const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutInfos,
                                            std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                                            std::vector<VkDeviceSize>& descriptorSetLayoutSizes, VulkanContext* context);
}

#endif
