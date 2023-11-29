#include "VulkanDescriptorSet.h"
#include "VulkanContext.h"

#include <stdexcept>

VulkanDescriptorSet::VulkanDescriptorSet() : context(nullptr), allocAddress(nullptr), allocIndex(0), setOffset(0){}

VulkanDescriptorSet::VulkanDescriptorSet(VulkanContext* context) : context(context), allocAddress(nullptr), 
                                         allocIndex(0), setOffset(0) {}

std::optional<uint32_t> VulkanDescriptorSet::insertDescriptor(uint32_t binding, VkDescriptorDataEXT descData) {
    if (bindingCounter[binding] >= bindingInfos[binding].descriptorCount)
        return {};

    VkDescriptorGetInfoEXT getInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
    getInfo.data = descData;
    getInfo.type = bindingInfos[binding].descriptorType;

    uint32_t index = bindingCounter[binding]++;

    char* addr = (*allocAddress);
    context->vkGetDescriptorEXT(context->logicalDevice, &getInfo, descriptorSizes[binding],
                      (char*)(*allocAddress) + setOffset + bindingOffsets[binding] + index * descriptorSizes[binding]);

    return index;
}

void VulkanDescriptorSet::updateDescriptor(uint32_t binding, uint32_t index, VkDescriptorDataEXT descData) {
    VkDescriptorGetInfoEXT getInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT};
    getInfo.data = descData;
    getInfo.type = bindingInfos[binding].descriptorType;

    char* addr = (*allocAddress);
    context->vkGetDescriptorEXT(context->logicalDevice, &getInfo, descriptorSizes[binding],
                     (char*)(*allocAddress) + setOffset + bindingOffsets[binding] + index * descriptorSizes[binding]);
}

void VulkanDescriptorSet::createDescriptorSetLayouts(std::vector<VkDeviceSize>& descriptorSetLayoutSizes, 
                                                    std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                                                    const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutInfos,
                                                    VulkanContext* context) {
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
