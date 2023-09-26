#ifndef _VULKANRENDERER_H_
#define _VULKANRENDERER_H_

#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanBufferArray.h"

#include "GLM/glm.hpp"
#include <vulkan/vulkan.h>

#include <vector>

class VulkanContext;

#define MAX_FRAMES_IN_FLIGHT 2

//the types of descriptor/uniform the renderer would need.
const std::vector<VkDescriptorType> descriptorTypes {
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
};

struct DescriptorSetBindingInfo {
    VkDescriptorType type;
    int numDescriptor; //number of descriptor at this binding
    VkShaderStageFlags stageFlags;
};

//ordering of the descriptor set layouts by usage
enum DescriptorSetLayoutIndex {
    global,
    perFrame,
};

const std::vector<std::vector<DescriptorSetBindingInfo>> descriptorSetLayoutInfos {
    { //global descriptor set
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT}
    },
    { //per frame descriptor set
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT}
    } 
};

struct UniformBufferObject {
    glm::mat4 projView;
};

struct GlobalUniformBufferObject {
    glm::vec4 light;
    glm::vec3 what;
};

class VulkanRenderer {
public:
    VulkanRenderer();
    VulkanRenderer(VulkanContext& context);

    VulkanBuffer globalUBO;
    /*
	    Resets the currentFrame's commandBuffer, and make ready the renderer to record
	    draw commands.
    */
    void beginDrawCalls(const glm::mat4& projView);

    /* 
	    records a single draw command into the currentFrame's commandBuffer
    */
    void draw(uint32_t numIndices, VkBuffer indexBuffer, VkBuffer* vertexBuffers);
	
    /*
	    submits currentFrame's commandBuffer
    */
    void submitDrawCalls();

    ~VulkanRenderer();
    void cleanup();

private:
    static PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSizeEXT;
    static PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffsetEXT;
    static PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT;
    static PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT;

    VkInstance instance;
    VkDevice logicalDevice;
    VkPhysicalDevice physicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    //next available imageIndex in the swapchain
    uint32_t imageIndex = 0;
    VulkanSwapchain swapchain;

    VulkanCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    int currentFrame = 0;
    std::vector<VkSemaphore> imageAvailableSemaphores; //whether an image is available to render to.
    std::vector<VkSemaphore> renderFinishedSemaphores; //whether an image has finished rendering.
    std::vector<VkFence> inFlightFences; //whether cmds for the currentFrame have finished.
    void createSyncObjects();

    void createCommandBuffers();

    VkDescriptorPool descriptorPool;
    void createDescriptorPool();
    
    //in the same order as descriptorSetLayoutInfos above
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkDeviceSize> descriptorSetLayoutSizes;
    void createDescriptorSetLayouts();

    //VulkanBufferArray perFrameUBOs;
    std::vector<VulkanBuffer> perFrameUBOs;
    void createUniformBuffers();

    VkDescriptorSet globalDescriptorSet;
    std::vector<VkDescriptorSet> perFrameDescriptorSets;

    uint32_t uniformDescriptorOffset;
    VulkanBuffer globalDescriptorSetBuffer;
    VulkanBuffer perFrameDescriptorSetBuffers;
    VkDeviceAddress globalDescriptorSetBufferDeviceAddr;
    VkDeviceAddress perFrameDescriptorSetBuffersDeviceAddr;
    void createDescriptorSets();

    //initial testing pipeline, there could be many different pipelines
    VulkanGraphicsPipeline pipeline;
    void createPipelines();

};

#endif
