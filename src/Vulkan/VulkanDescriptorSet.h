#ifndef _VULKANDESCRIPTORSET_H_
#define _VULKANDESCRIPTORSET_H_

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

class VulkanContext;

class VulkanDescriptorSet {
public:
    VulkanDescriptorSet();
    VulkanDescriptorSet(VulkanContext* context);

    uint32_t allocIndex; //index of the allocator that created this desc set

    char** allocAddress; //pointer to the allocator's device memory
    VkDeviceSize setOffset; //offset of this descSet within the allocator's memory 

    std::vector<VkDeviceSize> bindingOffsets; //Binding offsets relative to set offset
    std::vector<VkDeviceSize> descriptorSizes; //Each binding's descriptor memory size.
    std::vector<VkDescriptorSetLayoutBinding> bindingInfos;
    //The first free slot at each binding(0 - descriptor count). Updated in a linear manner. 
    std::vector<uint32_t> bindingCounter;

    /*
    * @return An optional index depending on whether the descriptor can be created or not.
    * @param index Index of the to-be inserted descriptor. It will be returned via reference.
    * @param binding Which binding to insert the new descriptor.
    * @param descData The data needed to insert a new descriptor.
    */
    std::optional<uint32_t> insertDescriptor(uint32_t binding, VkDescriptorDataEXT descData);

    /*
    * @param binding The binding number the descriptor belongs to.
    * @param index The index of the descriptor under the given binding.
    * @param descData Descriptor data used to update the existing one.
    */
    void updateDescriptor(uint32_t binding, uint32_t index, VkDescriptorDataEXT descData);

    /*
    * @param descriptorSetLayoutSizes A reference returned vector of device memory sizes of the desciptor sets created.
    * @param descriptorSetLayouts A vector of descriptor set layouts needed to create a corresponding array of descriptor sets.
    * @param descriptorSetLayoutInfos Each descriptor sets binding infos, per descriptor set, per binding.
    * @param context A pointer to the VulkanContext variable.
    */
    static void createDescriptorSetLayouts(std::vector<VkDeviceSize>& descriptorSetLayoutSizes,
                                            std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
                                            const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutInfos,
                                            VulkanContext* context);

private:
    VulkanContext* context;
};

#endif
