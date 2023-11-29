#ifndef _VULKANDESCRIPTORALLOCATOR_H_
#define _VULKANDESCRIPTORALLOCATOR_H_

#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"

#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;

class VulkanDescriptorAllocator {
public:
    //number of memory backed VulkanDescriptorAllocators created during the runtime of the program.
    static uint32_t numAllocators;
    //Descriptor Buffer binding infos of all the allocators
    static std::vector<VkDescriptorBufferBindingInfoEXT> allocBindingInfos;

    uint32_t bindingIndex; //binding index of this allocator
    VkDeviceAddress deviceAddress;

    VulkanDescriptorAllocator();
    VulkanDescriptorAllocator(uint64_t size, VkBufferUsageFlags usage, VulkanContext* context);

    /**
    * @return VulkanDescriptorSet Allocated VulkanDescriptorSet with resources bound.
    * @param resources A vector of VulkanBuffer pointers to create a single descriptor set for.
    * @param bindingInfos A vector of VkDescriptorSetLayoutBinding for the to-be created descriptor set.
    * @param layout The VkDescriptorSetLayout for the to-be created descriptor set.
    */
    VulkanDescriptorSet createDescriptorSet(const std::vector<std::vector<VulkanBuffer*>>& resources,
                               const std::vector<VkDescriptorSetLayoutBinding>& bindingInfos, VkDescriptorSetLayout layout);

    /**
    * @return VulkanDescriptorSet Allocated VulkanDescriptorSet with no resources bound.
    * @param bindingInfos A vector of VkDescriptorSetLayoutBinding for the to-be created descriptor set.
    * @param layout The VkDescriptorSetLayout for the to-be created descriptor set.
    */
    VulkanDescriptorSet createDescriptorSet(const std::vector<VkDescriptorSetLayoutBinding>& bindingInfos, VkDescriptorSetLayout layout);

    void cleanUp();

private:
    bool initialized = false;

    uint64_t allocatedSize;
    VkDeviceSize offset;
    VulkanBuffer freeBuffer;
    VulkanContext* context;
};

#endif
