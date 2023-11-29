#include "VulkanResourceManager.h"

VulkanResourceManager::VulkanResourceManager() : context(nullptr) {}

VulkanResourceManager::VulkanResourceManager(VulkanContext* context) {
    this->context = context;

    descAllocs.resize((size_t)allocCount);
    descAllocs[uboAlloc] = VulkanDescriptorAllocator(DESCRIPTOR_ALLOCATOR_BUFFER_SIZE,
                                               VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, context);
    descAllocs[texSamplerAlloc] = VulkanDescriptorAllocator(DESCRIPTOR_ALLOCATOR_BUFFER_SIZE,
                                               VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, context);
    
    VulkanDescriptorSet::createDescriptorSetLayouts(descriptorSetLayoutSizes, descriptorSetLayouts,
                                                    VulkanRendererInfos::descriptorSetLayoutInfos, context);
    descSets.resize(VulkanRendererInfos::descriptorSetLayoutInfos.size());
    descSets[VulkanRendererInfos::globalUniform] = descAllocs[uboAlloc].createDescriptorSet(
                                                VulkanRendererInfos::descriptorSetLayoutInfos[VulkanRendererInfos::globalUniform],
                                                descriptorSetLayouts[VulkanRendererInfos::globalUniform]);
    descSets[VulkanRendererInfos::texSampler] = descAllocs[texSamplerAlloc].createDescriptorSet(
                                                VulkanRendererInfos::descriptorSetLayoutInfos[VulkanRendererInfos::texSampler],
                                                descriptorSetLayouts[VulkanRendererInfos::texSampler]);
}

std::optional<uint32_t> VulkanResourceManager::createTexture2D(VkDescriptorImageInfo& imageInfo) {
    VkDescriptorDataEXT descData{};
    descData.pCombinedImageSampler = &imageInfo;

    return descSets[VulkanRendererInfos::texSampler].insertDescriptor(0, descData);
}

std::optional<uint32_t> VulkanResourceManager::addLight(glm::vec3 pos, glm::vec3 color) {
    return std::optional<uint32_t>();
}

void VulkanResourceManager::cleanUp() {
    for (VulkanDescriptorAllocator& alloc : descAllocs) {
        alloc.cleanUp();
    }

    for (VkDescriptorSetLayout layout : descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(context->logicalDevice, layout, nullptr);
    }
}
