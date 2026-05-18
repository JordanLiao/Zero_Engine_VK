#include "VulkanResourceManager.h"

#include <stdexcept>

//forward declared helper functions
void createDescriptorSetLayouts(const VulkanContext* context, VkDescriptorSetLayout& layout);
void createDescriptorPool(const VulkanContext* context, VkDescriptorPool& descriptorPool);
void createDescriptorSet(const VulkanContext* context, VkDescriptorPool pool, VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet);

VulkanResourceManager::VulkanResourceManager() {}

VulkanResourceManager::VulkanResourceManager(VulkanContext* context) {
    this->context = context;
    
    createDescriptorSetLayouts(context, descriptorSetLayout);
    createDescriptorPool(context, descriptorPool);
    createDescriptorSet(context, descriptorPool, descriptorSetLayout, descriptorSet);
}

void createDescriptorSetLayouts(const VulkanContext* context, VkDescriptorSetLayout& layout) {
    const auto& descriptorSetLayoutInfos = VulkanRendererInfos::descriptorSetLayoutInfos;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    for (const auto& layoutInfo : descriptorSetLayoutInfos) {
        std::vector<VkDescriptorSetLayoutBinding> bindings(layoutInfo.size());
        std::vector<VkDescriptorBindingFlags> bindingFlags(layoutInfo.size());

        for (size_t i = 0; i < layoutInfo.size(); i++) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = (uint32_t)i;
            layoutBinding.descriptorCount = layoutInfo[i].descriptorCount;
            layoutBinding.descriptorType = layoutInfo[i].descriptorType;
            layoutBinding.stageFlags = layoutInfo[i].stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            bindings[i] = layoutBinding;
            bindingFlags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
            //| VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT; if I am going to use variable desc, then it needs to be the last binding
        }

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
        bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlagsInfo.pNext = nullptr;
        bindingFlagsInfo.pBindingFlags = bindingFlags.data();
        bindingFlagsInfo.bindingCount = layoutInfo.size();

        layoutCreateInfo.bindingCount = (uint32_t)layoutInfo.size();
        layoutCreateInfo.pBindings = bindings.data();
        layoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; //For bindless architecture
        layoutCreateInfo.pNext = &bindingFlagsInfo;

        if (vkCreateDescriptorSetLayout(context->logicalDevice, &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
}

void createDescriptorPool(const VulkanContext* context, VkDescriptorPool& descriptorPool) {
    std::vector<VkDescriptorPoolSize> poolSizes;

    //Only using the first layout info since we only have one global descSet at the moment.
    for (auto& bindingInfo : VulkanRendererInfos::descriptorSetLayoutInfos[0]) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = bindingInfo.descriptorType;
        poolSize.descriptorCount = bindingInfo.descriptorCount;
        poolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.pNext = VK_NULL_HANDLE;

    vkCreateDescriptorPool(context->logicalDevice, &poolInfo, nullptr, &descriptorPool);
}

void createDescriptorSet(const VulkanContext* context, VkDescriptorPool pool, VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkResult result = vkAllocateDescriptorSets(context->logicalDevice, &allocInfo, &descriptorSet);
}

std::optional<uint32_t> VulkanResourceManager::addTexture2D(VkDescriptorImageInfo& imageInfo) {
    static uint32_t texBindingIdx = 0; //incrementor of free idx into texture binding point

    VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write.dstSet = descriptorSet;
    write.dstBinding = 2;                          //need to change the hard coding here
    write.dstArrayElement = texBindingIdx;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(context->logicalDevice, 1, &write, 0, nullptr);

    return texBindingIdx++;
}

std::optional<uint32_t> VulkanResourceManager::addLight(glm::vec3 pos, glm::vec3 color) {
    return std::optional<uint32_t>();
}

VkDescriptorSetLayout VulkanResourceManager::getDescriptorSetLayouts() {
    return descriptorSetLayout;
}

VkDescriptorSet VulkanResourceManager::getDescriptorSets() {
    return descriptorSet;
}

void VulkanResourceManager::cleanUp() {
    vkDestroyDescriptorSetLayout(context->logicalDevice, descriptorSetLayout, nullptr);
}
