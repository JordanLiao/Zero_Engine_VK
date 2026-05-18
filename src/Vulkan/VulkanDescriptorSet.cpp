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