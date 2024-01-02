#ifndef _VULKANPIPELINE_H_
#define _VULKANPIPELINE_H_

#include "VulkanUniformInfos.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanContext;

class VulkanPipeline {
public:
    VkPipeline pipeline;
    VkPipelineLayout layout;

    VulkanPipeline();
    VulkanPipeline(const std::string& comp, const VkPipelineCreateFlags& flags,
                    std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, VulkanContext* context);
    VulkanPipeline(const std::string& vert, const std::string& frag, const VkPipelineCreateFlags& flags, 
                           VkExtent2D& extent, VkFormat& format, VkFormat& depthFormat, int pushConstSize,
                           std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, VulkanContext* context);
    void cleanUp();

private:
    VulkanContext* context;

    VkShaderModule createShaderModule(const std::vector<char>& code);
    std::vector<char> readFile(const std::string& filename);
};

#endif

