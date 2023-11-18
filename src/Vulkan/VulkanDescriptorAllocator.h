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
    //Descriptor Buffer binding infos
    static std::vector<VkDescriptorBufferBindingInfoEXT> allocBindingInfos;

    uint32_t bindingIndex; //binding index of this allocator
    VkDeviceAddress deviceAddress;

    VulkanDescriptorAllocator();
    VulkanDescriptorAllocator(uint64_t size, VkBufferUsageFlags usage, VulkanContext* context);

    /**
    * @return Whether the allocator has successfully get the descriptor into the buffer
    * @param descOffset The offset in bytes into this allocator's memory that holds the descriptor set to the given resources.
    * @param resources A vector of VulkanBuffer pointers to create a single descriptor set for.
    * @param bindingInfos A vector of VkDescriptorSetLayoutBinding for the to-be created descriptor set.
    * @param layout The VkDescriptorSetLayout for the to-be created descriptor set.
    */
    VkDeviceSize getDescriptor(const std::vector<std::vector<VulkanBuffer*>>& resources, 
                               const std::vector<VkDescriptorSetLayoutBinding>& bindingInfos, VkDescriptorSetLayout layout);

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
