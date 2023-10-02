#include "VulkanRendererUtils.h"
#include "VulkanBufferUtils.h"
#include "VulkanContext.h"

#include <stdexcept>

void VulkanRendererUtils::createDescriptorSetLayouts(const std::vector<std::vector<DescriptorSetBindingInfo>>& descriptorSetLayoutInfos, 
                                                     std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, 
                                                     std::vector<VkDeviceSize>& descriptorSetLayoutSizes, VulkanContext* context) {
    descriptorSetLayouts.reserve(descriptorSetLayoutInfos.size());
    descriptorSetLayoutSizes.reserve(descriptorSetLayoutInfos.size());

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    for (const std::vector<DescriptorSetBindingInfo>& layoutInfo : descriptorSetLayoutInfos) {
        std::vector<VkDescriptorSetLayoutBinding> bindings(layoutInfo.size());
        int i = 0;
        for (size_t i = 0; i < layoutInfo.size(); i++) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = (uint32_t)i;
            layoutBinding.descriptorCount = layoutInfo[i].numDescriptor;
            layoutBinding.descriptorType = layoutInfo[i].type;
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

bool VulkanRendererUtils::createDescriptorSet(VulkanDescriptorBuffer& descBuffer, const std::vector<DescriptorSetBindingInfo>& bindingInfos,
                                              std::vector<std::vector<std::vector<VulkanBuffer*>>>& resources,
                                              VkDeviceSize descriptorSetLayoutSize, VkBufferUsageFlags usage, 
                                              VkMemoryPropertyFlags properties, VulkanContext* context) {
    //binding infos structure and resources structure must match at every binding level
    if (bindingInfos.size() != resources[0].size()) //num of bindings must match
        return false;
    for (size_t i = 0; i < bindingInfos.size(); i++) { //num of descriptors in each binding must also match
        if (bindingInfos[i].numDescriptor != resources[0][i].size())
            return false;
    }

    descBuffer.buffer = VulkanBuffer(descriptorSetLayoutSize * resources.size(),
                                    VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context);
    descBuffer.deviceAddress = VulkanBufferUtils::getBufferDeviceAddress(descBuffer.buffer.vkBuffer, context);
    descBuffer.buffer.map();

    VkDescriptorAddressInfoEXT addressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
    addressInfo.format = VK_FORMAT_UNDEFINED;
    VkDescriptorGetInfoEXT getInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
    getInfo.data.pUniformBuffer = &addressInfo;
    for (int k = 0; k < resources.size(); k++) {
        for (int i = 0; i < bindingInfos.size(); i++) {
            getInfo.type = bindingInfos[i].type;

            size_t uniformDescBufferSize = 0; //each binding can only have one type of descriptor
            if (getInfo.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                uniformDescBufferSize = context->descriptorBufferProperties->uniformBufferDescriptorSize;
            }
            else {
                throw std::runtime_error("Descriptor type currently not supported.");
            }

            for (int j = 0; j < bindingInfos[i].numDescriptor; j++) {
                addressInfo.range = resources[k][i][j]->hostSize;
                addressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(resources[k][i][j]->vkBuffer, context);
                context->vkGetDescriptorEXT(context->logicalDevice, &getInfo, uniformDescBufferSize,
                    (char*)descBuffer.buffer.data + k * descriptorSetLayoutSize + j * uniformDescBufferSize);
            }
        }
    }
}
