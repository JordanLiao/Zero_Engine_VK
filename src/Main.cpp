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
#include "Graphics/Mesh.h"
#include "Image.h"

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
	
    Object* obj = ResourceManager::loadObject("./assets/lowpolypine.obj");
    Image texture = ResourceManager::loadImage("./assets/texture.jpg", EngineFormats::RGBA);

    glm::mat4 projView = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 1.0f, 100.0f) *
                         glm::lookAt(glm::vec3(0.f, 2.f, 6.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();
		renderer.beginDrawCalls(projView);
        for (Mesh& m: obj->meshList) {
		    renderer.draw(obj->vkIndexBuffer.vkBuffer,  obj->vkVertexBuffers.vkBuffers.data(), 
                          m.size, m.indexOffset);
        }
		renderer.submitDrawCalls();
	}
	vkDeviceWaitIdle(vulkanContext.logicalDevice);

    obj->cleanUp();

    ResourceManager::cleanup();
    renderer.cleanUp();
    vulkanContext.cleanUp();
}