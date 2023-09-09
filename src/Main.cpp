#include <iostream>

#include "Window.h"
#include "Resources/VertexBuffer.h"
#include "Resources/IndexBuffer.h"

#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanBuffer.h"
#include "Vulkan/VulkanBufferArray.h"
#include "Vulkan/VulkanBufferUtils.h"
#include "Vulkan/VulkanCommandPool.h"
#include "Vulkan/VulkanBufferUtils.h"
#include "Vulkan/VulkanRenderer.h"

int main(int argc, char* argv[]) {
	Window window(900, 600, "Zero Engine VK");
	VulkanContext vulkanContext(window.window);

	VertexBuffer vertices{
		{glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f), glm::vec3(0.f, 0.7f, 0.f)},
		{glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f)}
	};

	IndexBuffer indices{
		{glm::ivec3(2,1,0)},
	};

	VulkanCommandPool transferCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, 
		                          vulkanContext.queueFamilyIndices.transferFamily.value(), 
		                          vulkanContext);

	VulkanBuffer* indexBuffer = VulkanBufferUtils::createIndexBuffer(indices, transferCommandPool, vulkanContext);
	VulkanBufferArray vertexBuffers = VulkanBufferUtils::createVertexBuffers(vertices,transferCommandPool, vulkanContext);

	std::cout << sizeof(VulkanBuffer) << std::endl;

	VulkanRenderer renderer(vulkanContext);
	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();
		renderer.beginDrawCalls();
		renderer.draw(3, indexBuffer->buffer, vertexBuffers.vulkanBuffers.data());
		renderer.submitDrawCalls();
	}
	vkDeviceWaitIdle(vulkanContext.logicalDevice);
}