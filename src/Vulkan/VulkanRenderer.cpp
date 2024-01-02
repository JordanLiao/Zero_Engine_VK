#include "VulkanRenderer.h"
#include "VulkanBufferUtils.h"
#include "VulkanContext.h"
#include "VulkanResourceManager.h"
#include "VulkanUniformInfos.h"

#include "../Resources/ResourceManager.h"

#include "GLFW/glfw3.h"
#include <stdexcept>
#include <iostream>

VulkanRenderer::VulkanRenderer(){}

VulkanRenderer::VulkanRenderer(VulkanContext* context, VulkanResourceManager* rManager) {
    this->context = context;
    this->rManager = rManager;

	swapchain = VulkanSwapchain(context);
    graphicsCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                    context->queueFamilyIndices.graphicsFamily.value(), context->logicalDevice);
    transferCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                    context->queueFamilyIndices.transferFamily.value(), context->logicalDevice);
    computeCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                    context->queueFamilyIndices.computeFamily.value(), context->logicalDevice);

    vkCmdBindDescriptorBuffersEXT = reinterpret_cast<PFN_vkCmdBindDescriptorBuffersEXT>(
                            vkGetDeviceProcAddr(context->logicalDevice, "vkCmdBindDescriptorBuffersEXT"));
    vkCmdSetDescriptorBufferOffsetsEXT = reinterpret_cast<PFN_vkCmdSetDescriptorBufferOffsetsEXT>(
                            vkGetDeviceProcAddr(context->logicalDevice, "vkCmdSetDescriptorBufferOffsetsEXT"));

    descAllocator = VulkanDescriptorAllocator(GLOBAL_DESCSET_ALLOCATOR_BUFFER_SIZE, VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, context);

    //sampler2D = VulkanImageUtils::createSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, context);
    createDepthMap();
    createUniformBuffers();
    createDescriptorSets();
    createPipelines();
    createSyncObjects();
    createCommandBuffers();
}

void VulkanRenderer::beginDrawCalls(const glm::vec3& viewPos, const glm::mat4& projView, float deltaT) {
    vkWaitForFences(context->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->logicalDevice, 1, &inFlightFences[currentFrame]);

    VulkanUniformInfos::PerFrameUBO ubo{};
    ubo.projView = projView;
    ubo.viewPos = viewPos;
    ubo.deltaT = deltaT;
    perFrameUBOs[currentFrame].transferData(&ubo, sizeof(ubo));
    
    /*VulkanUniformInfos::GlobalUBO gubo[4];
    gubo[0].light = glm::vec3(1.f, 0.f, 0.f);
    gubo[1].light = glm::vec3(0.5f, 0.5f, 0.f);
    gubo[2].light = glm::vec3(0.f, 0.5f, 0.5f);
    gubo[3].light = glm::vec3(0.f, 0.f, 1.f);
    gubo[0].lightPos = glm::vec3(-10.f, 10.f, 10.f);
    gubo[1].lightPos = glm::vec3(-10.f, -10.f, 10.f);
    gubo[2].lightPos = glm::vec3(10.f, 10.f, 10.f);
    gubo[3].lightPos = glm::vec3(10.f, -10.f, 10.f);

    int s = sizeof(VulkanUniformInfos::GlobalUBO);
    globalUBO[0].transferData(gubo, sizeof(VulkanUniformInfos::GlobalUBO));
    globalUBO[1].transferData(gubo + 1, sizeof(VulkanUniformInfos::GlobalUBO));
    globalUBO[2].transferData(gubo + 2, sizeof(VulkanUniformInfos::GlobalUBO));
    globalUBO[3].transferData(gubo + 3, sizeof(VulkanUniformInfos::GlobalUBO));*/

    //imageIndex returned by vkAcquireNextImageKHR is only guranteed to be availble next, but it may not 
    //be available immediately, so a semaphore is needed to synchronize vkcommands that depend on the image.
    VkResult result = vkAcquireNextImageKHR(context->logicalDevice, swapchain.swapchain, UINT64_MAX,
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    } 

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VulkanImageUtils::transitionImageLayout(swapchain.images[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, 
                                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffers[currentFrame]);

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachment.imageView = swapchain.imageViews[imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { 0.0f,0.0f,0.0f,0.0f };

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthAttachment.imageView = depthImage.vkImageView;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil = {1.f, 0};

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = { 0, 0, swapchain.extent };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;
    renderingInfo.pStencilAttachment = VK_NULL_HANDLE;

    vkCmdBeginRendering(commandBuffers[currentFrame], &renderingInfo);
    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, simplePipeline.pipeline);

    /***dynamic states************/
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (float)(swapchain.extent.height);
    viewport.width = (float)(swapchain.extent.width);
    viewport.height = -(float)(swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain.extent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);
    /****************************/

    vkCmdBindDescriptorBuffersEXT(commandBuffers[currentFrame], VulkanDescriptorAllocator::numAllocators, 
                                                                VulkanDescriptorAllocator::allocBindingInfos.data());

    uint32_t bufferIndices[VulkanRendererInfos::numDescRoles] = {2, 0, 1};
    VkDeviceSize offsets[VulkanRendererInfos::numDescRoles] = {rManager->descSets[VulkanRendererInfos::globalUniform].setOffset,
                                                               rManager->descSets[VulkanRendererInfos::texSampler].setOffset,
                                                               perframeDescSet.setOffset};
    vkCmdSetDescriptorBufferOffsetsEXT(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        simplePipeline.layout, 0, VulkanRendererInfos::numDescRoles, bufferIndices, offsets);
}

void VulkanRenderer::drawPBR(VkBuffer indexBuffer, VkBuffer* vertexBuffers, uint32_t numIndices, uint32_t indexOffset, 
                          glm::mat4& model, glm::ivec4& pbrMat) {
    VulkanUniformInfos::PBRConstant pConst;
    pConst.frameIndex = currentFrame;
    pConst.model = model;
    pConst.maps = pbrMat;
    vkCmdPushConstants(commandBuffers[currentFrame], pbrPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(VulkanUniformInfos::PBRConstant), &pConst);

    VkDeviceSize offsets[5] = {0, 0, 0, 0, 0};
    vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 5, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffers[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffers[currentFrame], numIndices, 1, indexOffset, 0, 0);
}

void VulkanRenderer::drawPhong(VkBuffer indexBuffer, VkBuffer* vertexBuffers, uint32_t numIndices, uint32_t indexOffset,
                                glm::mat4& model) {
    VulkanUniformInfos::PhongConstant pConst;
    pConst.frameIndex = currentFrame;
    pConst.model = model;
    vkCmdPushConstants(commandBuffers[currentFrame], simplePipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(VulkanUniformInfos::PhongConstant), &pConst);

    VkDeviceSize offsets[3] = { 0, 0, 0};
    vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 3, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffers[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffers[currentFrame], numIndices, 1, indexOffset, 0, 0);
}

void VulkanRenderer::submitDrawCalls() {
    vkCmdEndRendering(commandBuffers[currentFrame]);

    VulkanImageUtils::transitionImageLayout(swapchain.images[imageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, commandBuffers[currentFrame]);

    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::vector<VkSemaphore> waitSemaphores = { computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame] };
    //std::vector<VkSemaphore> waitSemaphores = {imageAvailableSemaphores[currentFrame] };
    //we cannot output color until image becomes availble
    std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapchain.swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    VkResult result = vkQueuePresentKHR(context->presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || context->resized) {
        recreateSwapchain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
}

void VulkanRenderer::beginCompute() {
    vkWaitForFences(context->logicalDevice, 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->logicalDevice, 1, &computeInFlightFences[currentFrame]);

    vkResetCommandBuffer(computeCmdBuffers[currentFrame],  0);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(computeCmdBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    vkCmdBindDescriptorBuffersEXT(computeCmdBuffers[currentFrame], 1,
                                  &VulkanDescriptorAllocator::allocBindingInfos[VulkanResourceManager::uboAlloc]);
}

void VulkanRenderer::compute(Cloth* cloth, float deltaTime) {
    uint32_t bufferIndices[VulkanRendererInfos::numDescRoles] = {0};
    VkDeviceSize offsets[1] = {cloth->descSet.setOffset};
    vkCmdSetDescriptorBufferOffsetsEXT(computeCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE,
                                clothDamperPipeline.layout, 0, 1, bufferIndices, offsets);

    VulkanUniformInfos::DeltaTimeConstant dTime;
    dTime.dt = deltaTime;

    vkCmdBindPipeline(computeCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, clothDamperPipeline.pipeline);
    vkCmdPushConstants(computeCmdBuffers[currentFrame], clothDamperPipeline.layout, VK_SHADER_STAGE_COMPUTE_BIT,
                        0, sizeof(VulkanUniformInfos::DeltaTimeConstant), &dTime);

    vkCmdDispatch(computeCmdBuffers[currentFrame], cloth->numDampers / 256 + 1, 1, 1);

    VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
    barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(computeCmdBuffers[currentFrame],
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            1, &barrier,
            0, nullptr,
            0, nullptr);

    vkCmdBindPipeline(computeCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, clothParticlePipeline.pipeline);
    vkCmdPushConstants(computeCmdBuffers[currentFrame], clothParticlePipeline.layout, VK_SHADER_STAGE_COMPUTE_BIT,
                        0, sizeof(VulkanUniformInfos::DeltaTimeConstant), &dTime);

    vkCmdDispatch(computeCmdBuffers[currentFrame], cloth->numPart / 256 + 1 , 1, 1);
}

void VulkanRenderer::submitCompute() {
    if (vkEndCommandBuffer(computeCmdBuffers[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &computeCmdBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &computeFinishedSemaphores[currentFrame];

    if (vkQueueSubmit(context->computeQueue, 1, &submitInfo, computeInFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit compute command buffer!");
    };
}


void VulkanRenderer::createDepthMap() {
    depthFormat = VulkanImageUtils::findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                          VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL,
                                                       VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, context->physicalDevice);
    VulkanImageUtils::createImage2D(depthImage, swapchain.extent.width, swapchain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context);
}

void VulkanRenderer::createUniformBuffers() {
    perFrameUBOs.reserve(FRAMES_IN_FLIGHT);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        perFrameUBOs.push_back(VulkanBuffer(sizeof(VulkanUniformInfos::PerFrameUBO),
                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context));
        perFrameUBOs.back().map();
    }
}

void VulkanRenderer::createDescriptorSets() {
    std::vector<std::vector<VulkanBuffer*>> bufferPtrs(2, std::vector<VulkanBuffer*>());
    bufferPtrs[0].reserve(FRAMES_IN_FLIGHT);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
       bufferPtrs[0].push_back(&perFrameUBOs[i]);
    }

    perframeDescSet = descAllocator.createDescriptorSet(bufferPtrs, 
                                                        VulkanRendererInfos::descriptorSetLayoutInfos[VulkanRendererInfos::perFrameUniform],
                                                        rManager->descriptorSetLayouts[VulkanRendererInfos::perFrameUniform]);
}

void VulkanRenderer::createPipelines() {
    simplePipeline = VulkanPipeline("./src/shaders/clothVert.spv", "./src/shaders/clothFrag.spv",
                                        VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                                        swapchain.extent, swapchain.format, depthFormat, sizeof(VulkanUniformInfos::PhongConstant),
                                        rManager->descriptorSetLayouts, context);
    /*pbrPipeline = VulkanPipeline("./src/shaders/pbr_vert.spv", "./src/shaders/pbr_frag.spv", 
                                      VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, 
                                      swapchain.extent, swapchain.format, depthFormat, sizeof(VulkanUniformInfos::PBRConstant),
                                      rManager->descriptorSetLayouts, context);*/
    clothDamperPipeline = VulkanPipeline("./src/shaders/cloth_damper.spv",
                                        VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                                        rManager->clothDescSetLayouts, context);

    clothParticlePipeline = VulkanPipeline("./src/shaders/cloth_particle.spv",
                                        VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,
                                        rManager->clothDescSetLayouts, context);
}

void VulkanRenderer::createSyncObjects() {
    imageAvailableSemaphores.resize(FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(FRAMES_IN_FLIGHT);
    inFlightFences.resize(FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context->logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    computeInFlightFences.resize(FRAMES_IN_FLIGHT);
    computeFinishedSemaphores.resize(FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context->logicalDevice, &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->logicalDevice, &fenceInfo, nullptr, &computeInFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute synchronization objects for a frame!");
        }
    }
}

void VulkanRenderer::recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(context->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(context->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(context->logicalDevice);
    swapchain.cleanUp();
    swapchain = VulkanSwapchain(context);

    vkDestroyImageView(context->logicalDevice, depthImage.vkImageView, nullptr);
    createDepthMap();

    context->resized = false;
}

void VulkanRenderer::createCommandBuffers() {
    commandBuffers.resize(FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    allocInfo.commandPool = graphicsCmdPool.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(context->logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
    //-------------------------------------------//
    computeCmdBuffers.resize(FRAMES_IN_FLIGHT);

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = (uint32_t)computeCmdBuffers.size();
    allocInfo.commandPool = computeCmdPool.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(context->logicalDevice, &allocInfo, computeCmdBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanRenderer::cleanUp() {
    descAllocator.cleanUp();

    for (VulkanBuffer& pfUbo : perFrameUBOs) {
        pfUbo.cleanUp();
    }

    pbrPipeline.cleanUp();
    clothDamperPipeline.cleanUp();
    clothParticlePipeline.cleanUp();

    vkDestroyImageView(context->logicalDevice, depthImage.vkImageView, nullptr);
    vkDestroyImage(context->logicalDevice, depthImage.vkImage, nullptr);
    vkFreeMemory(context->logicalDevice, depthImage.vkDeviceMemory, nullptr);

    for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context->logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context->logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(context->logicalDevice, inFlightFences[i], nullptr);
    }
    graphicsCmdPool.cleanUp();
    transferCmdPool.cleanUp();
	swapchain.cleanUp();
}