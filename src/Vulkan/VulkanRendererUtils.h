#ifndef _VULKANRENDERERUTILS_H_
#define _VULKANRENDERERUTILS_H_

#include "VulkanDescriptorBuffer.h"

#include "GLM/glm.hpp"
#include <vulkan/vulkan.h>

#include <vector>

struct DescriptorSetBindingInfo {
    VkDescriptorType type;
    int numDescriptor; //number of descriptor at this binding
    VkShaderStageFlags stageFlags;
};

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

class VulkanRendererUtils {
public:
    /*
        descriptorSetLayoutInfos: structure is [num of descriptorSet Layout, num of bindings for each layout]
        descriptorSetLayouts: a vector of descriptor set layouts to be created.
        descriptorSetLayoutSizes: a vector of device memory sizes of the corresponding desciptor sets created.
    */
    static void createDescriptorSetLayouts(const std::vector<std::vector<DescriptorSetBindingInfo>>& descriptorSetLayoutInfos,
                                            std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                                            std::vector<VkDeviceSize>& descriptorSetLayoutSizes, VulkanContext* context);

    /*
        Creates a descriptor buffer using binding infos of a single descriptor layout.
        descBuffer: The descriptor buffer to be created.
        bindingInfos: binding points information for a single descriptor layout.
        resourceAddrs: device addresses of VulkanBuffer resources for each array indices of every binding point. 
                       NOTE IMPORTANT, resourceAddrs has one more dimension than (bindingInfos' size * numDescriptors) because 
                       the descriptor buffer created here can actually have more than one descriptor set concatenated together 
                       in one buffer.
        descriptorSetLayoutSize: size of the descriptor set
    */
    static bool createDescriptorSet(VulkanDescriptorBuffer& descBuffer, const std::vector<DescriptorSetBindingInfo>& bindingInfos,
                                    std::vector<std::vector<std::vector<VulkanBuffer*>>>& resourceAddrs,
                                    VkDeviceSize descriptorSetLayoutSize, VkBufferUsageFlags usage,
                                    VkMemoryPropertyFlags properties, VulkanContext* context);
};

#endif
