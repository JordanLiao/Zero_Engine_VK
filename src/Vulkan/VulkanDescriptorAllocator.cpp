#include "VulkanDescriptorAllocator.h"
#include "VulkanBufferUtils.h"
#include "VulkanContext.h"

#include <stdexcept>

VulkanDescriptorAllocator::VulkanDescriptorAllocator() {}

VulkanDescriptorAllocator::VulkanDescriptorAllocator(uint64_t size, VkBufferUsageFlags usage, VulkanContext* context) {
    if(usage != VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT && usage != VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT && 
       usage != (VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT)) {
        throw std::runtime_error("Vulkan Descriptor Buffer must have Descriptor Buffer bits of only either Resource or(and) Sampler Bits");
    }
    this->context = context;
    descriptorBuffer = VulkanBuffer(size, usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context);
    descriptorBuffer.map();
    deviceAddress = VulkanBufferUtils::getBufferDeviceAddress(descriptorBuffer.vkBuffer, context);

    offset = 0;
    this->allocatedSize = descriptorBuffer.hostSize;
    initialized = true;
}

bool VulkanDescriptorAllocator::getDescriptor(VkDeviceSize& descOffset, const std::vector<std::vector<VulkanBuffer*>>& resources,
                                     const std::vector<VkDescriptorSetLayoutBinding>& bindingInfos, VkDescriptorSetLayout layout) {
    if (!initialized) {
        throw std::runtime_error("VulkanDescriptorAllocator has not been initialized with allocated memory!");
    }

    VkDeviceSize layoutSize;
    context->vkGetDescriptorSetLayoutSizeEXT(context->logicalDevice, layout, &layoutSize);
    layoutSize = VulkanBufferUtils::getAlignedBufferSize(layoutSize, context->descBufferProps->descriptorBufferOffsetAlignment);

    offset = VulkanBufferUtils::getAlignedBufferSize(offset, context->descBufferProps->descriptorBufferOffsetAlignment);
    descOffset = offset;

    //pre allocated buffer does not have enough space for another descriptor set
    if (offset + layoutSize > allocatedSize) {
        return false;
    }

    //offsets of each binding into the descriptor layout
    std::vector<VkDeviceSize> bindingOffsets(bindingInfos.size());
    for (int i = 0; i < bindingInfos.size(); i++) {
        context->vkGetDescriptorSetLayoutBindingOffsetEXT(context->logicalDevice, layout, i, &(bindingOffsets[i]));
    }

    VkDescriptorAddressInfoEXT addressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
    addressInfo.format = VK_FORMAT_UNDEFINED;

    VkDescriptorGetInfoEXT getInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
    getInfo.data.pUniformBuffer = &addressInfo;

    VkDeviceAddress startingOffset = offset;
    for (int i = 0; i < bindingInfos.size(); i++) {
        offset = startingOffset + bindingOffsets[i];
        getInfo.type = bindingInfos[i].descriptorType;

        size_t descriptorSize = 0; //each binding can only have one type of descriptor
        if (getInfo.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
            descriptorSize = context->descBufferProps->uniformBufferDescriptorSize;
        }
        else {
            throw std::runtime_error("Descriptor type currently not supported.");
        }

        for (uint32_t j = 0; j < bindingInfos[i].descriptorCount; j++) {
            addressInfo.range = resources[i][j]->hostSize;
            addressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(resources[i][j]->vkBuffer, context);
            context->vkGetDescriptorEXT(context->logicalDevice, &getInfo, descriptorSize,
                (char*)descriptorBuffer.data + offset + j * descriptorSize);
        }
        offset += (bindingInfos[i].descriptorCount * descriptorSize);
    }
    return true;
}

void VulkanDescriptorAllocator::cleanUp() {}
