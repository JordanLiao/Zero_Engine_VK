#ifndef _VULKANRENDERER_H_
#define _VULKANRENDERER_H_

#include <vulkan/vulkan.h>
#include <vector>

#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanCommandPool.h"
#include "VulkanGraphicsPipeline.h"

#define MAX_FRAMES_IN_FLIGHT 2

class VulkanRenderer {
public:
	VulkanRenderer();
	VulkanRenderer(VulkanContext& context);

	/*
		Resets the currentFrame's commandBuffer, and make ready the renderer to record
		draw commands.
	*/
	void beginDrawCalls();

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

	//initial testing pipeline, there could be many different pipelines
	VulkanGraphicsPipeline pipeline;

};

#endif
