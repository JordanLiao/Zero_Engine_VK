#ifndef _VULKANRENDERER_H_
#define _VULKANRENDERER_H_

#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanBufferArray.h"
#include "VulkanRendererUtils.h"

//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "GLM/glm.hpp"
#include <vulkan/vulkan.h>

#include <vector>

class VulkanContext;

#define MAX_FRAMES_IN_FLIGHT 2

const std::vector<std::vector<DescriptorSetBindingInfo>> descriptorSetLayoutInfos {
    { //global descriptor set
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, VK_SHADER_STAGE_FRAGMENT_BIT}
    },
    { //per frame descriptor set, containing projection, view etc.
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT}
    }
};

class VulkanRenderer {
public:
    VulkanRenderer();
    VulkanRenderer(VulkanContext* context);

    std::vector<VulkanBuffer> globalUBO;
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

    void cleanup();

private:
    VulkanContext* context;

    uint32_t imageIndex = 0; //next available imageIndex in the swapchain
    VulkanSwapchain swapchain;

    VulkanCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers; //one commandBuffer per frame in flight
    void createCommandBuffers();

    int currentFrame = 0;
    std::vector<VkSemaphore> imageAvailableSemaphores; //whether an image is available to render to.
    std::vector<VkSemaphore> renderFinishedSemaphores; //whether an image has finished rendering.
    std::vector<VkFence> inFlightFences; //whether cmds for the currentFrame have finished.
    void createSyncObjects();

    //VkDescriptorPool descriptorPool;
    //void createDescriptorPool();
    
    std::vector<VulkanBuffer> perFrameUBOs;
    void createUniformBuffers();

    VulkanImage depthImage;
    VkFormat depthFormat;
    void createDepthResources();
    
    //Layouts and sizes are in the same order as descriptorSetLayoutInfos above
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkDeviceSize> descriptorSetLayoutSizes;

    PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT = VK_NULL_HANDLE;
    PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT = VK_NULL_HANDLE;

    uint32_t uniformDescriptorOffset; //size of a uniform descriptor, used for indexing offset
    VulkanDescriptorBuffer globalDescriptorSetBuffer;
    VulkanDescriptorBuffer perFrameDescriptorSetBuffers; //multiple perFrame descriptor sets buffers packed into one.
    //VkDeviceAddress globalDescriptorSetBufferDeviceAddr;
    //VkDeviceAddress perFrameDescriptorSetBuffersDeviceAddr;
    void createDescriptorSets();

    //initial testing pipeline, there could be many different pipelines
    VulkanGraphicsPipeline pipeline;
    void createPipelines();
};

#endif
