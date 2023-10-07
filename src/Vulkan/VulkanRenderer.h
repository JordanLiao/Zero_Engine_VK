#ifndef _VULKANRENDERER_H_
#define _VULKANRENDERER_H_

#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanBufferArray.h"
#include "VulkanRendererUtils.h"
#include "VulkanDescriptorAllocator.h"

//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "GLM/glm.hpp"
#include <vulkan/vulkan.h>

#include <vector>

class VulkanContext;

#define MAX_FRAMES_IN_FLIGHT 2

//size of the descriptor allocator buffer to hold all descriptor sets
#define DESCRIPTOR_ALLOCATOR_BUFFER_SIZE 10000

const std::vector<std::vector<VkDescriptorSetLayoutBinding>> descriptorSetLayoutInfos  = {
    { //global descriptor set
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE}
    },
    { //per frame descriptor set, containing projection, view etc.
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, VK_NULL_HANDLE}
    }
};

class VulkanRenderer {
public:
    VulkanRenderer();
    VulkanRenderer(VulkanContext* context);

    std::vector<VulkanBuffer> globalUBO;
    /*
	    Resets commandBuffers[currentFrame], and make ready the renderer to record draw commands.
    */
    void beginDrawCalls(const glm::mat4& projView);

    /* 
	    records a single draw command into commandBuffers[currentFrame].
    */
    void draw(uint32_t numIndices, VkBuffer indexBuffer, VkBuffer* vertexBuffers);
	
    /*
	    submits currentFrame's commandBuffer
    */
    void submitDrawCalls();

    void cleanUp();

private:
    VulkanContext* context;

    uint32_t imageIndex = 0; //next available imageIndex in the swapchain
    VulkanSwapchain swapchain;

    VulkanCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers; //one commandBuffer per frame in flight
    void createCommandBuffers();

    uint32_t currentFrame = 0;
    std::vector<VkSemaphore> imageAvailableSemaphores; //whether an image is available to render to.
    std::vector<VkSemaphore> renderFinishedSemaphores; //whether an image has finished rendering.
    std::vector<VkFence> inFlightFences; //whether cmds for the currentFrame have finished.
    void createSyncObjects();
    
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

    VulkanDescriptorBuffer globalDescriptorSetBuffer;
    VulkanDescriptorBuffer perFrameDescriptorSetBuffers; //multiple perFrame descriptor sets buffers packed into one.
    VulkanDescriptorAllocator descAllocator;
    std::vector<VkDeviceSize> perFrameDescOffsets;
    VkDeviceSize globalDescOffset;
    void createDescriptorSets();

    //initial testing pipeline, there could be many different pipelines
    VulkanGraphicsPipeline pipeline;
    void createPipelines();

    //VkDescriptorPool descriptorPool;
    //void createDescriptorPool();
};

#endif
