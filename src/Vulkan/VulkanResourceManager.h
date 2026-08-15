#ifndef _VULKANRESOURCEMANAGER_H_
#define _VULKANRESOURCEMANAGER_H_

#include "VulkanContext.h"
#include "VulkanRendererInfos.h"
#include "VulkanBuffer.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <unordered_map>

//size of the descriptor allocator buffer to hold all descriptor sets
#define DESCRIPTOR_ALLOCATOR_BUFFER_SIZE 10000

//map from desc type to its counter or first usable idx
typedef std::unordered_map<VkDescriptorType, int> DescriptorIndexCounterMap;
//map from desc type to its binding location
typedef std::unordered_map<VkDescriptorType, int> DescriptorBindingMap;

class VulkanResourceManager {
public:
    VulkanResourceManager();
    VulkanResourceManager(VulkanContext* context);
    ~VulkanResourceManager();

    std::optional<uint32_t> addTexture2D(VkDescriptorImageInfo& imageInfo);
    std::optional<uint32_t> addLight(glm::vec3 pos, glm::vec3 color);

    VkDescriptorSetLayout getDescriptorSetLayouts();
    VkDescriptorSet getDescriptorSets();

    void addDescriptor(VkDescriptorType descType, VulkanBuffer* uboBuffer);

    VkDescriptorPool getImGuiDescriptorPool() const;

private:
    VulkanContext* context = nullptr;

    //PBR pipeline descSets
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE; //Layouts and sizes are in the same order as descriptorSetLayoutInfos
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

    VkDescriptorPool imGuiDescPool = VK_NULL_HANDLE;

    //free idex to write descriptor to in the bindless descriptor set per binding type
    DescriptorIndexCounterMap descIdxCounters;
    DescriptorBindingMap descBindings;
};

#endif
