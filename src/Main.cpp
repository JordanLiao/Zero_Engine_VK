#include <iostream>

#include "Window.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanBufferArray.h"
#include "VulkanBufferUtils.h"
#include "VulkanCommandPool.h"
#include "VulkanBufferUtils.h"
#include "VulkanRenderer.h"
#include "resources/GraphicsBuffers.h"
#include "Graphics/Object.h"

//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "GLM/gtx/transform.hpp"

#include "../Resources/ResourceManager.h"

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    VulkanContext* context = (VulkanContext*)glfwGetWindowUserPointer(window);
    context->resized = true;
}

int main(int argc, char* argv[]) {

    uint32_t width = 900, height = 600;
	Window window(width, height, "Zero Engine VK");
	VulkanContext vulkanContext(window.window);
    glfwSetWindowUserPointer(window.window, &vulkanContext);
    glfwSetFramebufferSizeCallback(window.window, framebufferResizeCallback);
    ResourceManager::init(&vulkanContext);
	VulkanRenderer renderer(&vulkanContext);
	VulkanCommandPool transferCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, 
		                                  vulkanContext.queueFamilyIndices.transferFamily.value(), 
		                                  vulkanContext.logicalDevice);

	VertexBuffer vertices{
		{glm::vec3(-0.5f, -0.5f, 0.f), glm::vec3(-0.5f, 0.5f, 0.f), glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(0.5f, -0.5f, 0.f)},
		{glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f)}
	};

	IndexBuffer indices{
		{glm::ivec3(0, 1, 2), glm::ivec3(3,0,2)},
	};

	//VulkanBuffer indexBuffer = VulkanBufferUtils::createIndexBuffer(indices);
	//VulkanBufferArray vertexBuffers = VulkanBufferUtils::createVertexBuffers(vertices);
	
    Object* obj = ResourceManager::loadObject("./assets/bunny.obj");

    glm::mat4 projView = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 1.0f, 100.0f) *
                         glm::lookAt(glm::vec3(0.f, 0.f, 5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();
		renderer.beginDrawCalls(projView);
		//renderer.draw(6, indexBuffer.buffer, vertexBuffers.vkBuffers.data());
		renderer.draw(obj->vulkanIndexBuffer.hostSize / sizeof(glm::ivec3) * 3, obj->vulkanIndexBuffer.vkBuffer,  obj->vulkanVertexBuffers.vkBuffers.data());
		renderer.submitDrawCalls();
	}
	vkDeviceWaitIdle(vulkanContext.logicalDevice);
}