#ifndef _VULKANGRAPHICSPIPELINE_H_
#define _VULKANGRAPHICSPIPELINE_H_

#include "VulkanUniformInfos.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <string>

class VulkanGraphicsPipeline {
public:
    VkPipeline graphicsPipeline;
    VkPipelineLayout layout;

    VulkanGraphicsPipeline();
    VulkanGraphicsPipeline(const std::string& vert, const std::string& frag, const VkPipelineCreateFlags& flags, 
                           VkExtent2D& extent, VkFormat& format, VkFormat& depthFormat,
                           std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, VkDevice logicalDevice);
    void cleanUp();

private:
    VkDevice logicalDevice;

    VkShaderModule createShaderModule(const std::vector<char>& code);
    std::vector<char> readFile(const std::string& filename);
};

#endif

