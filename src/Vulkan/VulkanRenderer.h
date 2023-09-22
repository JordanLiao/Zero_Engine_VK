#ifndef _VULKANRENDERER_H_
#define _VULKANRENDERER_H_

#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanCommon.h"
#include "VulkanBufferUtils.h"

#include "GLM/glm.hpp"
#include <vulkan/vulkan.h>

#include <vector>

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

struct DescriptorSetLayoutInfo {
    int numSet; //num of descriptor set of this layout. Layouts of the same kind will be placed adjacent to each other
    std::vector<DescriptorSetBindingInfo> bindingInfos; //info at each binding points
};

//ordering of the descriptor set layouts by usage
enum DescriptorSetLayoutIndex {
    global,
    perFrame,
};

const std::vector<DescriptorSetLayoutInfo> descriptorSetLayoutInfos {
    {//global descriptor set layout, just 1
        1, 
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT} //binding0
        }
    }, 
    {//per frame descriptor set layouts, in MAX_FRAMES_IN_FLIGHT
        MAX_FRAMES_IN_FLIGHT,
        {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT}
        } 
    }
};

struct UniformBufferObject {
    glm::mat4 projView;
};

struct GlobalUniformBufferObject {
    glm::vec4 light;
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
    void createDescriptorSetLayouts();

    //VulkanBufferArray perFrameUBOs;
    std::vector<VulkanBuffer> perFrameUBOs;
    void createUniformBuffers();

    VkDescriptorSet globalDescriptorSet;
    std::vector<VkDescriptorSet> perFrameDescriptorSets;
    void createDescriptorSets();

    //initial testing pipeline, there could be many different pipelines
    VulkanGraphicsPipeline pipeline;
    void createPipelines();

};

#endif
