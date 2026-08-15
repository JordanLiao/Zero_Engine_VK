#include "VulkanResourceManager.h"

#include <stdexcept>
#include <iostream>

//forward declared helper functions
void createDescriptorSetLayouts(VulkanContext* context, VkDescriptorSetLayout& layout, 
                                DescriptorIndexCounterMap& descIdxCounters, DescriptorBindingMap& descBindings);
void createDescriptorPool(VulkanContext* context, VkDescriptorPool& descriptorPool);
void createDescriptorSet(VulkanContext* context, VkDescriptorPool pool, VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet);

VulkanResourceManager::VulkanResourceManager() {}

VkDescriptorPool createImGuiDescriptorPool(VulkanContext* context) {
    // Define the sizes for the different types of descriptors ImGui might request.
    // 1000 is a generous default used in most ImGui Vulkan examples to ensure 
    // you don't run out of descriptors if your UI has many unique textures.
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

    // CRITICAL: You must include the FREE_DESCRIPTOR_SET_BIT.
    // ImGui needs this to be able to dynamically free descriptor sets 
    // when you delete textures or rebuild the font atlas during runtime.
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    // The maximum number of descriptor sets that can be allocated from this pool.
    pool_info.maxSets = 1000;

    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imguiPool = VK_NULL_HANDLE;
    VkResult err = vkCreateDescriptorPool(context->getLogicalDevice(), &pool_info, nullptr, &imguiPool);

    if (err != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan Descriptor Pool for ImGui." << std::endl;
        // In a real application, you would assert or throw an exception here
    }

    return imguiPool;
}

VulkanResourceManager::VulkanResourceManager(VulkanContext* context) {
    this->context = context;
    
    createDescriptorSetLayouts(context, descriptorSetLayout, descIdxCounters, descBindings);
    createDescriptorPool(context, descriptorPool);
    createDescriptorSet(context, descriptorPool, descriptorSetLayout, descriptorSet);

    imGuiDescPool = createImGuiDescriptorPool(context);
}

void createDescriptorSetLayouts(VulkanContext* context, VkDescriptorSetLayout& layout, 
                                DescriptorIndexCounterMap& descIdxCounters, DescriptorBindingMap& descBindings) {
    const auto& descriptorSetLayoutInfos = VulkanRendererInfos::descriptorSetLayoutInfos;

    std::vector<VkDescriptorSetLayoutBinding> bindings(descriptorSetLayoutInfos.size());
    std::vector<VkDescriptorBindingFlags> bindingFlags(descriptorSetLayoutInfos.size());

    for (size_t i = 0; i < descriptorSetLayoutInfos.size(); i++) {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = (uint32_t)i;
        layoutBinding.descriptorCount = descriptorSetLayoutInfos[i].descriptorCount;
        layoutBinding.descriptorType = descriptorSetLayoutInfos[i].descriptorType;
        layoutBinding.stageFlags = descriptorSetLayoutInfos[i].stageFlags;
        layoutBinding.pImmutableSamplers = nullptr;
        bindings[i] = layoutBinding;
        bindingFlags[i] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        //| VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT; if I am going to use variable desc, then it needs to be the last binding

        descIdxCounters[descriptorSetLayoutInfos[i].descriptorType] = 0;
        descBindings[descriptorSetLayoutInfos[i].descriptorType] = i;
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{};
    bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsInfo.pNext = nullptr;
    bindingFlagsInfo.pBindingFlags = bindingFlags.data();
    bindingFlagsInfo.bindingCount = descriptorSetLayoutInfos.size();

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = (uint32_t)descriptorSetLayoutInfos.size();
    layoutCreateInfo.pBindings = bindings.data();
    layoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; //For bindless architecture
    layoutCreateInfo.pNext = &bindingFlagsInfo;

    if (vkCreateDescriptorSetLayout(context->getLogicalDevice(), &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void createDescriptorPool(VulkanContext* context, VkDescriptorPool& descriptorPool) {
    std::vector<VkDescriptorPoolSize> poolSizes;

    //Only using the first layout info since we only have one global descSet at the moment.
    for (auto& bindingInfo : VulkanRendererInfos::descriptorSetLayoutInfos) {
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

    vkCreateDescriptorPool(context->getLogicalDevice(), &poolInfo, nullptr, &descriptorPool);
}

void createDescriptorSet(VulkanContext* context, VkDescriptorPool pool, VkDescriptorSetLayout layout, VkDescriptorSet& descriptorSet) {
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkResult result = vkAllocateDescriptorSets(context->getLogicalDevice(), &allocInfo, &descriptorSet);
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

    vkUpdateDescriptorSets(context->getLogicalDevice(), 1, &write, 0, nullptr);

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

void VulkanResourceManager::addDescriptor(VkDescriptorType descType, VulkanBuffer* uboBuffer) {
    VkDescriptorBufferInfo uboInfo{};
    uboInfo.buffer = uboBuffer->vkBuffer;
    uboInfo.offset = 0;
    uboInfo.range  = uboBuffer->hostSize;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSet;
    write.dstBinding = descBindings[descType];
    write.dstArrayElement = descIdxCounters[descType]++;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = &uboInfo;

    vkUpdateDescriptorSets(context->getLogicalDevice(), 1, &write, 0, nullptr);
}

VkDescriptorPool VulkanResourceManager::getImGuiDescriptorPool() const {
    return imGuiDescPool;
}

VulkanResourceManager::~VulkanResourceManager() {
    vkDestroyDescriptorSetLayout(context->getLogicalDevice(), descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(context->getLogicalDevice(), descriptorPool, nullptr);
    vkDestroyDescriptorPool(context->getLogicalDevice(), imGuiDescPool, nullptr);
}
