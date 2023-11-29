#ifndef _VULKANRENDERER_H_
#define _VULKANRENDERER_H_

#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorAllocator.h"
#include "VulkanUniformInfos.h"
#include "VulkanDescriptorSet.h"
#include "VulkanRendererInfos.h"
#include "../Resources/Image.h"

//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "GLM/glm.hpp"
#include <vulkan/vulkan.h>

#include <vector>

class VulkanContext;
class VulkanResourceManager;

class VulkanRenderer {
public:
    VulkanRenderer();
    VulkanRenderer(VulkanContext* context, VulkanResourceManager* rManager);

    //std::vector<VulkanBuffer> globalUBO;

    /**
	*   Resets commandBuffers[currentFrame], and make ready the renderer to record draw commands.
    *   @param projView The projection view matrix used for drawing frame of index currentFrame.
    */
    void beginDrawCalls(const glm::vec3& viewPos, const glm::mat4& projView);

    /**
    * @param indexBuffer VkBuffer handle to the indexBuffer.
    * @param vertexBuffers A pointer to the Vkbuffer array.
    * @param numIndices The number of indices to draw.
    * @param indexOffset The offset to the first index to draw in the indexBuffer.
    */
    void draw(VkBuffer indexBuffer, VkBuffer* vertexBuffers, uint32_t numIndices, uint32_t indexOffset, 
              glm::mat4& model, glm::ivec4& pbrMat);
	
    /*
	    submits currentFrame's commandBuffer
    */
    void submitDrawCalls();

    void cleanUp();

private:
    VulkanContext* context;

    uint32_t imageIndex = 0; //next available imageIndex in the swapchain
    VulkanSwapchain swapchain;
    void recreateSwapchain();

    VulkanCommandPool graphicsCmdPool, transferCmdPool;
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
    void createDepthMap();
    

    PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT = VK_NULL_HANDLE;
    PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT = VK_NULL_HANDLE;

    VulkanResourceManager* rManager;

    VulkanDescriptorAllocator descAllocator;
    VulkanDescriptorSet perframeDescSet;
    void createDescriptorSets();

    //initial testing pipeline, there could be many different pipelines
    VulkanGraphicsPipeline pipeline;
    void createPipelines();
};

#endif
