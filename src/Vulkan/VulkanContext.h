#ifndef _VULKANCONTEXT_H_
#define _VULKANCONTEXT_H_

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include "GLFW/glfw3.h"

#include "VulkanCommon.h"
#include "VulkanTexture.h"
#include "VulkanBuffer.h"

//for test
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <vector>
#include <optional>
#include <algorithm>
#include <string>
#include <memory>

#ifdef _DEBUG
#define ENABLE_VALIDATION_LAYER true
#else
#define ENABLE_VALIDATION_LAYER false
#endif

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
};

const int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanContext {
public:
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VulkanCommon::QueueFamilyIndices queueFamilyIndices;
	GLFWwindow* window;

	VulkanContext(GLFWwindow* window);
	void cleanup();

	void mainLoop();

private:

	VkInstance vulkanInstance;
	void createVulkanInstance();
	
	bool checkValidationLayerSupport();

	void createSurface();

	VulkanCommon::QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pDevice);
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice pDevice);
	bool checkDeviceExtensionSupport(VkPhysicalDevice pDevice); 
	
	void createLogicalDevice();

	VkCommandPool primaryCommandPool;
	std::vector<VkCommandBuffer> primaryCommandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	void createSyncObjects();

	int currentFrame = 0;

	//for experiment
	VkBuffer vertexBuffer[2];
	VkDeviceMemory vertexPositionMemory;
	VkDeviceMemory vertexNormalMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VertexBuffer vertices{
		{glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.f, 0.7f, 0.f)},
		{glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f)}
	};

	IndexBuffer indices{
		{glm::ivec3(2,1,0)},
	};

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void drawFrame();
};

#endif