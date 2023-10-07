#include "VulkanRendererUtils.h"
#include "VulkanBufferUtils.h"
#include "VulkanContext.h"

#include <stdexcept>

void VulkanRendererUtils::createDescriptorSetLayouts(const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutInfos,
                                                     std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, 
                                                     std::vector<VkDeviceSize>& descriptorSetLayoutSizes, VulkanContext* context) {
    descriptorSetLayouts.reserve(descriptorSetLayoutInfos.size());
    descriptorSetLayoutSizes.reserve(descriptorSetLayoutInfos.size());

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    for (const std::vector<VkDescriptorSetLayoutBinding>& layoutInfo : descriptorSetLayoutInfos) {
        std::vector<VkDescriptorSetLayoutBinding> bindings(layoutInfo.size());
        int i = 0;
        for (size_t i = 0; i < layoutInfo.size(); i++) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = (uint32_t)i;
            layoutBinding.descriptorCount = layoutInfo[i].descriptorCount;
            layoutBinding.descriptorType = layoutInfo[i].descriptorType;
            layoutBinding.stageFlags = layoutInfo[i].stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            bindings[i] = layoutBinding;
        }

        layoutCreateInfo.bindingCount = (uint32_t)layoutInfo.size();
        layoutCreateInfo.pBindings = bindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(context->logicalDevice, &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        descriptorSetLayouts.push_back(layout);
        VkDeviceSize layoutSize;
        context->vkGetDescriptorSetLayoutSizeEXT(context->logicalDevice, layout, &layoutSize);
        descriptorSetLayoutSizes.push_back(layoutSize);
    }
}
