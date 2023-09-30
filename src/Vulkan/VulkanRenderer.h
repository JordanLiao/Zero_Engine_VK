#ifndef _VULKANRENDERER_H_
#define _VULKANRENDERER_H_

#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanBufferArray.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
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

/*
    From top level to bottom: per set layout, per binding
*/
const std::vector<std::vector<DescriptorSetBindingInfo>> descriptorSetLayoutInfos {
    { //global descriptor set
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, VK_SHADER_STAGE_FRAGMENT_BIT}
    },
    { //per frame descriptor set
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT}
    } 
};

struct UniformBufferObject {
    glm::mat4 projView;
    glm::vec3 viewPos;
};

struct GlobalUniformBufferObject {
    glm::vec3 lightPosition;
    alignas(16) glm::vec3 light;
};

const VkImageSubresourceRange coloredImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1,};

class VulkanRenderer {
public:
    VulkanRenderer();
    VulkanRenderer(VulkanContext& context);

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
    PFN_vkGetDescriptorSetLayoutSizeEXT vkGetDescriptorSetLayoutSizeEXT = VK_NULL_HANDLE;
    PFN_vkGetDescriptorSetLayoutBindingOffsetEXT vkGetDescriptorSetLayoutBindingOffsetEXT = VK_NULL_HANDLE;
    PFN_vkCmdBindDescriptorBuffersEXT vkCmdBindDescriptorBuffersEXT = VK_NULL_HANDLE;
    PFN_vkCmdSetDescriptorBufferOffsetsEXT vkCmdSetDescriptorBufferOffsetsEXT = VK_NULL_HANDLE;
    PFN_vkGetDescriptorEXT vkGetDescriptorEXT = VK_NULL_HANDLE;

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

    VulkanImage depthImage;
    VkFormat depthFormat;
    void createDepthResources();

    VkDescriptorSet globalDescriptorSet;
    std::vector<VkDescriptorSet> perFrameDescriptorSets;

    uint32_t uniformDescriptorOffset; //size of a uniform descriptor, used for indexing offset
    VulkanBuffer globalDescriptorSetBuffer;
    VulkanBuffer perFrameDescriptorSetBuffers; //multiple perFrame descriptor sets buffers packed into one.
    VkDeviceAddress globalDescriptorSetBufferDeviceAddr;
    VkDeviceAddress perFrameDescriptorSetBuffersDeviceAddr;
    void createDescriptorSets();

    //initial testing pipeline, there could be many different pipelines
    VulkanGraphicsPipeline pipeline;
    void createPipelines();

};

#endif
