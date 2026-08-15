#ifndef _VULKANRENDERER_H_
#define _VULKANRENDERER_H_

#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanUniformInfos.h"
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

    ~VulkanRenderer();

    std::vector<VulkanBuffer> globalUBO;

    /**
	*   Sync with previous frame, and reset and begin new command buffer
    */
    void beginRendering(const glm::vec3& viewPos, const glm::vec3& viewDir, const glm::mat4& projView, float deltaT);

    void beginDrawCalls();
    void submitDrawCalls();

    /**
    * Draw with PBR, should stay between begin and submit draw calls functions;
    * @param indexBuffer VkBuffer handle to the indexBuffer.
    * @param vertexBuffers A pointer to the Vkbuffer array.
    * @param numIndices The number of indices to draw.
    * @param indexOffset The offset to the first index to draw in the indexBuffer.
    */
    void drawPBR(VkBuffer indexBuffer, VkBuffer* vertexBuffers, uint32_t numIndices, uint32_t indexOffset, glm::mat4& model, glm::ivec4& pbrMat);
    void drawPhong(VkBuffer indexBuffer, VkBuffer* vertexBuffers, uint32_t numIndices, uint32_t indexOffset, glm::mat4& model);
	

    VkCommandBuffer getDrawCmdBuf() const;

    void cleanUp();

    VkFormat getSwapChainColorFormat() const;
    VkFormat getDepthFormat() const;

private:
    VulkanContext* context;

    uint32_t imageIndex = 0; //next available imageIndex in the swapchain
    VulkanSwapchain swapchain;
    void recreateSwapchain();

    VulkanCommandPool graphicsCmdPool, transferCmdPool;// computeCmdPool;
    std::vector<VkCommandBuffer> graphicsCmdBuffers; //one commandBuffer per frame in flight
    std::vector<VkCommandBuffer> computeCmdBuffers;
    void createCommandBuffers();

    uint32_t currentFrame = 0;
    //std::vector<VkFence> computeInFlightFences;
    //std::vector<VkSemaphore> computeFinishedSemaphores;
    std::vector<VkSemaphore> imageAvailableSemaphores; //whether an image is available to render to.
    std::vector<VkSemaphore> renderFinishedSemaphores; //whether an image has finished rendering.
    std::vector<VkFence> inFlightFences; //whether cmds for the currentFrame have finished.
    void createSyncObjects();
    
    std::vector<VulkanBuffer> perFrameUBOs;
    void createUniformBuffers();

    VulkanImage depthImage;
    VkFormat depthFormat;
    void createDepthMap();
    
    //PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT = VK_NULL_HANDLE;
    //PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT = VK_NULL_HANDLE;

    VulkanResourceManager* vkRourceManager;

    //initial testing pipeline, there could be many different pipelines
    VulkanPipeline simplePipeline;
    VulkanPipeline pbrPipeline;

    void createPipelines();
};

#endif
