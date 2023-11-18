#include "VulkanDescriptorAllocator.h"
#include "VulkanBufferUtils.h"
#include "VulkanContext.h"

#include <stdexcept>

uint32_t VulkanDescriptorAllocator::numAllocators = 0;
std::vector<VkDescriptorBufferBindingInfoEXT> VulkanDescriptorAllocator::allocBindingInfos;

VulkanDescriptorAllocator::VulkanDescriptorAllocator() {}

VulkanDescriptorAllocator::VulkanDescriptorAllocator(uint64_t size, VkBufferUsageFlags usage, VulkanContext* context) {
    if(usage != VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT && 
       usage != VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT && 
       usage != (VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT)) {
        throw std::runtime_error("Vulkan Descriptor Buffer usage must have only either Resource or(and) Sampler Bits");
    }
    this->context = context;
    freeBuffer = VulkanBuffer(size, usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context);
    freeBuffer.map();
    deviceAddress = VulkanBufferUtils::getBufferDeviceAddress(freeBuffer.vkBuffer, context);

    offset = 0;
    this->allocatedSize = freeBuffer.hostSize;
    initialized = true;

    VkDescriptorBufferBindingInfoEXT bindingInfo{};
    bindingInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
    bindingInfo.address = deviceAddress;
    bindingInfo.usage = usage;
    allocBindingInfos.push_back(bindingInfo);

    bindingIndex = numAllocators;
    numAllocators++;
}

VkDeviceSize VulkanDescriptorAllocator::getDescriptor(const std::vector<std::vector<VulkanBuffer*>>& resources,
                                     const std::vector<VkDescriptorSetLayoutBinding>& bindingInfos, VkDescriptorSetLayout layout) {
    if (!initialized) {
        throw std::runtime_error("Attempting to use an unallocated VulkanDescriptorAllocator!");
    }

    VkDeviceSize layoutSize;
    context->vkGetDescriptorSetLayoutSizeEXT(context->logicalDevice, layout, &layoutSize);
    layoutSize = VulkanBufferUtils::getAlignedBufferSize(layoutSize, context->descBufferProps->descriptorBufferOffsetAlignment);


    offset = VulkanBufferUtils::getAlignedBufferSize(offset, context->descBufferProps->descriptorBufferOffsetAlignment);
    VkDeviceSize descOffset = offset;

    //if the reserved free buffer does not have enough space for another descriptor set
    //IMPORTANT might want to add the feature of automatically scale the allocator up in size.
    if (offset + layoutSize > allocatedSize) {
        throw std::runtime_error("A VulkanDescriptorAllocator has run out of memory for new descriptors!");
    }

    //offsets of each binding into the descriptor layout
    std::vector<VkDeviceSize> bindingOffsets(bindingInfos.size());
    for (int i = 0; i < bindingInfos.size(); i++) {
        context->vkGetDescriptorSetLayoutBindingOffsetEXT(context->logicalDevice, layout, i, &(bindingOffsets[i]));
    }

    VkDescriptorAddressInfoEXT addressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
    addressInfo.format = VK_FORMAT_UNDEFINED;

    VkDeviceAddress startingOffset = offset;
    for (int i = 0; i < bindingInfos.size(); i++) {
        offset = startingOffset + bindingOffsets[i];

        VkDescriptorGetInfoEXT getInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
        getInfo.data.pUniformBuffer = &addressInfo;
        getInfo.type = bindingInfos[i].descriptorType;

        size_t descriptorSize = 0; //each binding can only have one type of descriptor
        if (getInfo.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            descriptorSize = context->descBufferProps->uniformBufferDescriptorSize;
        }
        else if (getInfo.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            descriptorSize = context->descBufferProps->combinedImageSamplerDescriptorSize;
        }
        else {
            throw std::runtime_error("Descriptor type currently not supported.");
        }

        for (uint32_t j = 0; j < bindingInfos[i].descriptorCount; j++) {
            addressInfo.range = resources[i][j]->hostSize;
            addressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(resources[i][j]->vkBuffer, context);
            context->vkGetDescriptorEXT(context->logicalDevice, &getInfo, descriptorSize,
                                        (char*)freeBuffer.data + offset + j * descriptorSize);
        }
        offset += (bindingInfos[i].descriptorCount * descriptorSize); //need this line to delimit the last binding
    }
    
    return descOffset;
}

VulkanDescriptorSet VulkanDescriptorAllocator::createDescriptorSet(const std::vector<VkDescriptorSetLayoutBinding>& bindingInfos, 
                                                                   VkDescriptorSetLayout layout){
    if (!initialized) {
        throw std::runtime_error("Attempting to use an unallocated VulkanDescriptorAllocator!");
    }

    VkDeviceSize layoutSize;
    context->vkGetDescriptorSetLayoutSizeEXT(context->logicalDevice, layout, &layoutSize);
    layoutSize = VulkanBufferUtils::getAlignedBufferSize(layoutSize, context->descBufferProps->descriptorBufferOffsetAlignment);

    offset = VulkanBufferUtils::getAlignedBufferSize(offset, context->descBufferProps->descriptorBufferOffsetAlignment);

    VulkanDescriptorSet desc;
    desc.layout = layout;
    desc.setOffset = offset;
    desc.allocIndex = bindingIndex;
    desc.allocAddress = &deviceAddress;

    //if the reserved free buffer does not have enough space for another descriptor set
    //IMPORTANT might want to add the feature of automatically scale the allocator up in size.
    if (offset + layoutSize > allocatedSize) {
        throw std::runtime_error("A VulkanDescriptorAllocator has run out of memory for new descriptors!");
    }

    //offsets of each binding into the descriptor layout
    desc.bindingOffsets.resize(bindingInfos.size());
    for (int i = 0; i < bindingInfos.size(); i++) {
        context->vkGetDescriptorSetLayoutBindingOffsetEXT(context->logicalDevice, layout, i, &(desc.bindingOffsets[i]));
    }

    desc.descriptors.resize(bindingInfos.size());

    VkDeviceAddress startingOffset = offset;
    for (int i = 0; i < bindingInfos.size(); i++) {
        offset = startingOffset + desc.bindingOffsets[i];

        size_t descriptorSize = 0; //each binding can only have one type of descriptor
        if (bindingInfos[i].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            descriptorSize = context->descBufferProps->uniformBufferDescriptorSize;
        }
        else if (bindingInfos[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            descriptorSize = context->descBufferProps->combinedImageSamplerDescriptorSize;
        }
        else {
            throw std::runtime_error("Descriptor type currently not supported.");
        }

        desc.descriptors[i].resize(bindingInfos[i].descriptorCount);

        for (uint32_t j = 0; j < bindingInfos[i].descriptorCount; j++) {
            desc.descriptors[i][j].offset = offset + j * descriptorSize;
        }
        offset += (bindingInfos[i].descriptorCount * descriptorSize);
    }

    return desc;
}

void VulkanDescriptorAllocator::cleanUp() {
    freeBuffer.cleanUp();
}
