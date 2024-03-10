#ifndef _VULKANSHADER_H_

#include <string>
#include <vector>
#include <unordered_map>

#include <vulkan/vulkan.h>

class VulkanContext;

class VulkanShader {
public:
    struct BindMetaData {
        uint32_t set;
        uint32_t binding;
        uint32_t count;
        VkDescriptorType descriptorType;
        VkShaderStageFlags shaderStageFlag;
    };

    struct PushConstantMetaData {
        uint32_t size;
        uint32_t offset;
        VkShaderStageFlags shadeshaderStageFlag;
    };

    VulkanShader(std::string vertexShaderfilePath, std::string fragmentShaderFilePath, VulkanContext* context);
    ~VulkanShader();

private:
    VulkanContext* context;

    VkShaderModule vertexShader;
    VkShaderModule fragShader;

    std::unordered_map<std::string, BindMetaData> reflectionData;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkDescriptorSet> descriptorSets;
};

#endif