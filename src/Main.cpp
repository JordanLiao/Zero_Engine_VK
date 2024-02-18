#include "VulkanContext.h"
#include "Window.h"
#include "VulkanResourceManager.h"
#include "VulkanRenderer.h"

#include "Resources/GraphicsBuffers.h"
#include "Resources/Image.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Object.h"
#include "Graphics/Mesh.h"
#include "Material.h"
#include "Cloth.h"

//#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "GLM/gtx/transform.hpp"

#include <chrono>
#include <iostream>

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
    VulkanResourceManager rManager(&vulkanContext);
    ResourceManager::init(&vulkanContext, &rManager);
	VulkanRenderer renderer(&vulkanContext, &rManager);

    glfwSetWindowUserPointer(window.window, &vulkanContext);
    glfwSetFramebufferSizeCallback(window.window, framebufferResizeCallback);
	
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

    int w = 50, h = 50;
    Cloth* cloth = ResourceManager::createCloth(w, h, 10.f / (float)w, 50.f / (float)(w * h), 2000.f, 0.99f);

    glm::vec3 viewPos(0.f, 10.0f, 8.f);
    proj = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glm::mat4 model = glm::scale(glm::vec3(1.f));

    float deltaT = 0.f;
	while (!glfwWindowShouldClose(window.window)) {
    //for(int i = 0; i < 10; i++) {
        //srand((unsigned int)time(nullptr));
        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		glfwPollEvents();

        glm::mat4 projView = proj * glm::lookAt(viewPos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

        renderer.beginCompute();
        renderer.compute(cloth, 0.0001);
        renderer.submitCompute();

		renderer.beginDrawCalls(viewPos, projView, deltaT);
        /*for (Mesh& m: obj->meshList) {
		    renderer.draw(obj->vkIndexBuffer.vkBuffer,  obj->vkVertexBuffers.vkBuffers.data(), 
                          m.size, m.indexOffset, model, pbr.maps);
        }*/

        renderer.drawPhong(cloth->vkIndexBuffer.vkBuffer, cloth->vkVertexBuffers.vkBuffers.data(),
                    cloth->numIndices, 0, model);

		renderer.submitDrawCalls();

        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        //deltaT = (float)(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000000.0);
        //std::cout << deltaT << std::endl;
	}
	vkDeviceWaitIdle(vulkanContext.logicalDevice);

    obj->cleanUp();
    cloth->cleanUp();

    ResourceManager::cleanup();
    rManager.cleanUp();
    renderer.cleanUp();
    vulkanContext.cleanUp();
}