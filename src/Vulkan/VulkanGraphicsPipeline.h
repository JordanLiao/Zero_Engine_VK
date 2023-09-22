#ifndef _VULKANGRAPHICSPIPELINE_H_
#define _VULKANGRAPHICSPIPELINE_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanSwapchain.h"
#include "VulkanBuffer.h"

class VulkanGraphicsPipeline {
public:
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;

    VulkanGraphicsPipeline();
    VulkanGraphicsPipeline(const std::string& vert, const std::string& frag, VkDevice& logicalDevice, VkExtent2D& extent, 
                           VkFormat& format, std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
    void cleanup();

private:
    VkDevice logicalDevice;
    //VulkanSwapchain* swapchain;

    VkShaderModule createShaderModule(const std::vector<char>& code);
    static std::vector<char> readFile(const std::string& filename);
};

#endif

