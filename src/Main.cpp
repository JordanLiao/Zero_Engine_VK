#include "VulkanContext.h"
#include "Window.h"
#include "VulkanResourceManager.h"
#include "VulkanRenderer.h"
#include "VulkanCommandUtils.h"

#include "Resources/GraphicsBuffers.h"
#include "Resources/Image.h"
#include "Resources/ResourceManager.h"
#include "Graphics/Object.h"
#include "Graphics/Mesh.h"
#include "Material.h"

//#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "GLM/gtx/transform.hpp"

#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_vulkan.h"

#include <chrono>
#include <iostream>

static glm::mat4 proj;

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    if (width == 0 || height == 0)
        return;
    VulkanContext* context = (VulkanContext*)glfwGetWindowUserPointer(window);
    context->setWindowResized(true);
    proj = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 1000.0f);
}

void initImGui(GLFWwindow* window, VulkanContext* context, VulkanResourceManager* rManager,
               VulkanRenderer* renderer) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = context->getInstance();
    init_info.PhysicalDevice = context->getPhysicalDevice();
    init_info.Device = context->getLogicalDevice();;
    init_info.QueueFamily = context->getQueueFamilyIndices().graphicsFamily.value();
    init_info.Queue = context->getGraphicsQueue();
    init_info.PipelineCache = nullptr;
    init_info.DescriptorPool = rManager->getImGuiDescriptorPool();
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = 2; //need to know why 2
    init_info.ImageCount = 2;
    init_info.CheckVkResultFn = nullptr;
    init_info.UseDynamicRendering = true;

    static VkFormat colorFormat = renderer->getSwapChainColorFormat();
    static VkFormat depthFormat = renderer->getDepthFormat();
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;

    ImGui_ImplVulkan_Init(&init_info);
}

void shutDownImgui() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

int main(int argc, char* argv[]) {
    uint32_t width = 1920, height = 1080;
	Window window(width, height, "Zero Engine VK");

	VulkanContext vulkanContext(window.window);
    VulkanResourceManager rManager(&vulkanContext);
    ResourceManager::init(&vulkanContext, &rManager);
	VulkanRenderer renderer(&vulkanContext, &rManager);

    glfwSetWindowUserPointer(window.window, &vulkanContext);
    glfwSetFramebufferSizeCallback(window.window, framebufferResizeCallback);

    initImGui(window.window, &vulkanContext, &rManager, &renderer);
	
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

    //cloth sim only
    /*int w = 50, h = 50;
    Cloth* cloth = ResourceManager::createCloth(w, h, 10.f / (float)w, 50.f / (float)(w * h), 2000.f, 0.99f);*/

    glm::vec3 viewPos(0.f, 0.0f, 5.f);
    glm::vec3 lookPos(0.f, 0.f, 0.f);
    glm::vec3 viewDir = lookPos - viewPos;
    proj = glm::perspective(glm::radians(50.0f), (float)width / (float)height, 0.1f, 1000.0f);
    glm::mat4 model = glm::scale(glm::vec3(1.f));

    float deltaT = 0.f;
	while (!glfwWindowShouldClose(window.window)) {
		glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- Build Your UI ---
        ImGui::Begin("Debug Panel");
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        if (ImGui::Button("Close App")) {
            glfwSetWindowShouldClose(window.window, GLFW_TRUE);
        }
        ImGui::End();

        glm::mat4 projView = proj * glm::lookAt(viewPos, lookPos, glm::vec3(0.f, 1.f, 0.f));
		renderer.beginRendering(viewPos, viewDir, projView, deltaT);

        renderer.beginDrawCalls();
        for (Mesh& m: obj->meshList) {
		    renderer.drawPBR(obj->vkIndexBuffer.vkBuffer,  obj->vkVertexBuffers.vkBuffers.data(), 
                          m.size, m.indexOffset, model, pbr.maps);
        }

        //render imgui last to make the gui stay on top of in-scene models
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(draw_data, renderer.getDrawCmdBuf());

		renderer.submitDrawCalls();
	}
	vkDeviceWaitIdle(vulkanContext.getLogicalDevice());

    shutDownImgui();

    obj->cleanUp();

    ResourceManager::cleanup();
}