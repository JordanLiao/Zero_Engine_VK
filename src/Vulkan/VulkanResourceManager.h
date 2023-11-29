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
    enum DescriptorAllocatorRole {
        uboAlloc,
        texSamplerAlloc,
        allocCount,
    };

    std::vector<VulkanDescriptorAllocator> descAllocs;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts; //Layouts and sizes are in the same order as descriptorSetLayoutInfos
    std::vector<VkDeviceSize> descriptorSetLayoutSizes;
    std::vector<VulkanDescriptorSet> descSets;

    VulkanResourceManager();
    VulkanResourceManager(VulkanContext* context);

    std::optional<uint32_t> createTexture2D(VkDescriptorImageInfo& imageInfo);
    std::optional<uint32_t> addLight(glm::vec3 pos, glm::vec3 color);

    void cleanUp();
private:
    VulkanContext* context;
};

#endif // !_VULKANRESOURCEMANAGER_H_
