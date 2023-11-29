#include "VulkanRenderer.h"
#include "VulkanBufferUtils.h"
#include "VulkanContext.h"
#include "VulkanResourceManager.h"

#include "../Resources/ResourceManager.h"

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

void VulkanRenderer::beginDrawCalls(const glm::vec3& viewPos, const glm::mat4& projView) {
    vkWaitForFences(context->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->logicalDevice, 1, &inFlightFences[currentFrame]);

    VulkanUniformInfos::PerFrameUBO ubo{};
    ubo.projView = projView;
    ubo.viewPos = viewPos;
    perFrameUBOs[currentFrame].transferData(&ubo, sizeof(ubo));
    
    /*VulkanUniformInfos::GlobalUniformBufferObject gubo[4];
    gubo[0].light = glm::vec3(1.f, 0.f, 0.f);
    gubo[1].light = glm::vec3(0.5f, 0.5f, 0.f);
    gubo[2].light = glm::vec3(0.f, 0.5f, 0.5f);
    gubo[3].light = glm::vec3(0.f, 0.f, 1.f);
    gubo[0].lightPosition = glm::vec3(-10.f, 10.f, 10.f);
    gubo[1].lightPosition = glm::vec3(-10.f, -10.f, 10.f);
    gubo[2].lightPosition = glm::vec3(10.f, 10.f, 10.f);
    gubo[3].lightPosition = glm::vec3(10.f, -10.f, 10.f);

    int s = sizeof(VulkanUniformInfos::GlobalUniformBufferObject);
    globalUBO[0].transferData(gubo, sizeof(VulkanUniformInfos::GlobalUniformBufferObject));
    globalUBO[1].transferData(gubo + 1, sizeof(VulkanUniformInfos::GlobalUniformBufferObject));
    globalUBO[2].transferData(gubo + 2, sizeof(VulkanUniformInfos::GlobalUniformBufferObject));
    globalUBO[3].transferData(gubo + 3, sizeof(VulkanUniformInfos::GlobalUniformBufferObject));*/

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
    vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.graphicsPipeline);

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
                                                               rManager->descSets[VulkanRendererInfos::perFrameUniform].setOffset};
    vkCmdSetDescriptorBufferOffsetsEXT(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipeline.layout, 0, VulkanRendererInfos::numDescRoles, bufferIndices, offsets);
}

void VulkanRenderer::draw(VkBuffer indexBuffer, VkBuffer* vertexBuffers, uint32_t numIndices,  uint32_t indexOffset,
                                                                          glm::mat4& model, glm::ivec4& pbrMat) {
    VulkanUniformInfos::PushConstant pConst;
    pConst.frameIndex = currentFrame;
    pConst.model = model;
    pConst.maps = pbrMat;
    vkCmdPushConstants(commandBuffers[currentFrame], pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(VulkanUniformInfos::PushConstant), &pConst);

    VkDeviceSize offsets[5] = {0, 0, 0, 0, 0};
    vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 5, vertexBuffers, offsets);
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

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    //we cannot output color until image becomes availble
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
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

void VulkanRenderer::createDepthMap() {
    depthFormat = VulkanImageUtils::findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                          VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL,
                                                       VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, context->physicalDevice);
    VulkanImageUtils::createImage2D(depthImage, swapchain.extent.width, swapchain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context);
}

void VulkanRenderer::createUniformBuffers() {
    /*globalUBO.resize(descriptorSetLayoutInfos[VulkanUniformInfos::uniformDescSet][1].descriptorCount);
    for (uint32_t i = 0; i < descriptorSetLayoutInfos[VulkanUniformInfos::uniformDescSet][1].descriptorCount; i++) {
        globalUBO[i] = VulkanBuffer(sizeof(VulkanUniformInfos::GlobalUniformBufferObject),
                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context);
        globalUBO[i].map();
    }*/
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


    /*test
    texture = ResourceManager::loadImage("./assets/starfish.jpg", EngineFormats::RGBA);
    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = sampler2D;
    imageInfo.imageView = texture.data.vkImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorDataEXT descData{};
    descData.pCombinedImageSampler = &imageInfo;

    texDescSet.updateDescriptor(0, 0, descData); */
}

void VulkanRenderer::createPipelines() {
    pipeline = VulkanGraphicsPipeline("./src/shaders/pbr_vert.spv", "./src/shaders/pbr_frag.spv", 
                                      VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, 
                                      swapchain.extent, swapchain.format, depthFormat, 
                                      rManager->descriptorSetLayouts, context->logicalDevice);
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
}

void VulkanRenderer::cleanUp() {
    descAllocator.cleanUp();

    for (VulkanBuffer& pfUbo : perFrameUBOs) {
        pfUbo.cleanUp();
    }

    pipeline.cleanUp();

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