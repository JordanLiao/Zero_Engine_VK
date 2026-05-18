#ifndef _VULKANRESOURCEMANAGER_H_
#define _VULKANRESOURCEMANAGER_H_

#include "VulkanContext.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDescriptorAllocator.h"
#include "VulkanRendererInfos.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

//size of the descriptor allocator buffer to hold all descriptor sets
#define DESCRIPTOR_ALLOCATOR_BUFFER_SIZE 10000

class VulkanResourceManager {
public:
    VulkanResourceManager();
    VulkanResourceManager(VulkanContext* context);

    std::optional<uint32_t> addTexture2D(VkDescriptorImageInfo& imageInfo);
    std::optional<uint32_t> addLight(glm::vec3 pos, glm::vec3 color);

    VkDescriptorSetLayout getDescriptorSetLayouts();
    VkDescriptorSet getDescriptorSets();

    void cleanUp();
private:
    VulkanContext* context = nullptr;

    //PBR pipeline descSets
    VkDescriptorSetLayout descriptorSetLayout = nullptr; //Layouts and sizes are in the same order as descriptorSetLayoutInfos
    VkDescriptorPool descriptorPool = nullptr;
    VkDescriptorSet descriptorSet = nullptr;

    //void createDescriptorSetLayouts(const std::vector<std::vector<VkDescriptorSetLayoutBinding>>& descriptorSetLayoutInfos);
    //void createDescriptorSet();
};

#endif
