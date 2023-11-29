#include <iostream>

#include "Window.h"
#include "VulkanContext.h"
#include "VulkanResourceManager.h"
#include "VulkanBuffer.h"
#include "VulkanBufferUtils.h"
#include "VulkanCommandPool.h"
#include "VulkanBufferUtils.h"
#include "VulkanRenderer.h"
#include "resources/GraphicsBuffers.h"
#include "Graphics/Object.h"
#include "Graphics/Mesh.h"
#include "../Resources/Image.h"
#include "Z3D_Material.h"

#include "GLM/gtx/transform.hpp"
#include "../Resources/ResourceManager.h"

static glm::mat4 proj;

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    VulkanContext* context = (VulkanContext*)glfwGetWindowUserPointer(window);
    context->resized = true;
    proj = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 1000.0f);
}


int main(int argc, char* argv[]) {
    uint32_t width = 900, height = 600;
	Window window(width, height, "Zero Engine VK");
	VulkanContext vulkanContext(window.window);
    glfwSetWindowUserPointer(window.window, &vulkanContext);
    glfwSetFramebufferSizeCallback(window.window, framebufferResizeCallback);
    VulkanResourceManager rManager(&vulkanContext);
    ResourceManager::init(&vulkanContext, &rManager);
	VulkanRenderer renderer(&vulkanContext, &rManager);
	
    Object* obj = ResourceManager::loadObject("./assets/sphere.obj");

    Image baseColor = ResourceManager::loadImage("./assets/rustediron/rust_basecolor.png", Formats::R8G8B8A8);
    Image normalMap = ResourceManager::loadImage("./assets/rustediron/rust_normal.png", Formats::R8G8B8);
    Image roughness = ResourceManager::loadImage("./assets/rustediron/rust_roughness.png", Formats::R8);
    Image metallic = ResourceManager::loadImage("./assets/rustediron/rust_metallic.png", Formats::R8);
    /*Image baseColor = ResourceManager::loadImage("./assets/stainlesssteel/used-stainless-steel2_albedo.png", Formats::R8G8B8A8);
    Image normalMap = ResourceManager::loadImage("./assets/stainlesssteel/used-stainless-steel2_normal-ogl.png", Formats::R8G8B8);
    Image roughness = ResourceManager::loadImage("./assets/stainlesssteel/used-stainless-steel2_roughness.png", Formats::R8);
    Image metallic = ResourceManager::loadImage("./assets/stainlesssteel/used-stainless-steel2_metallic.png", Formats::R8);*/

    PBRMaterial pbr;
    pbr.maps.r = baseColor.texId.value();
    pbr.maps.g = normalMap.texId.value();
    pbr.maps.b = roughness.texId.value();
    pbr.maps.a = metallic.texId.value();

    glm::vec3 viewPos(0.f, 0.0f, 3.f);
    proj = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glm::mat4 model = glm::scale(glm::vec3(1.f));

	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();
        glm::mat4 projView = proj * glm::lookAt(viewPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		renderer.beginDrawCalls(viewPos, projView);
        for (Mesh& m: obj->meshList) {
		    renderer.draw(obj->vkIndexBuffer.vkBuffer,  obj->vkVertexBuffers.vkBuffers.data(), 
                          m.size, m.indexOffset, model, pbr.maps);
        }
		renderer.submitDrawCalls();
	}
	vkDeviceWaitIdle(vulkanContext.logicalDevice);

    obj->cleanUp();

    ResourceManager::cleanup();
    rManager.cleanUp();
    renderer.cleanUp();
    vulkanContext.cleanUp();
}