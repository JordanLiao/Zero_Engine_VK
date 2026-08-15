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
    this->vkRourceManager = rManager;

	swapchain = VulkanSwapchain(context);
    graphicsCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                    context->getQueueFamilyIndices().graphicsFamily.value(), context->getLogicalDevice());
    transferCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                    context->getQueueFamilyIndices().transferFamily.value(), context->getLogicalDevice());
    //computeCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                    //context->getQueueFamilyIndices().computeFamily.value(), context->getLogicalDevice());


    createDepthMap();
    createUniformBuffers();
    createPipelines();
    createSyncObjects();
    createCommandBuffers();
}

VulkanRenderer::~VulkanRenderer() {
    cleanUp();
}

void VulkanRenderer::beginRendering(const glm::vec3& viewPos, const glm::vec3& viewDir, const glm::mat4& projView, float deltaT) {
    vkWaitForFences(context->getLogicalDevice(), 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->getLogicalDevice(), 1, &inFlightFences[currentFrame]);

    VulkanUniformInfos::PerFrameUBO ubo{};
    ubo.projView = projView;
    ubo.viewDir = viewDir;
    ubo.viewPos = viewPos;
    ubo.deltaT = deltaT;
    perFrameUBOs[currentFrame].transferData(&ubo, sizeof(ubo));

    //imageIndex returned by vkAcquireNextImageKHR is only guranteed to be availble next, but it may not 
    //be available immediately, so a semaphore is needed to synchronize vkcommands that depend on the image.
    VkResult result = vkAcquireNextImageKHR(context->getLogicalDevice(), swapchain.getSwapchain(), UINT64_MAX,
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    } 

    vkResetCommandBuffer(graphicsCmdBuffers[currentFrame], 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(graphicsCmdBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VulkanImageUtils::transitionImageLayout(swapchain.images[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, 
                                            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, graphicsCmdBuffers[currentFrame]);

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

    vkCmdBeginRendering(graphicsCmdBuffers[currentFrame], &renderingInfo);
}

void VulkanRenderer::beginDrawCalls() {
    vkCmdBindPipeline(graphicsCmdBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pbrPipeline.pipeline);

    /***dynamic states********************************************/
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (float)(swapchain.extent.height);
    viewport.width = (float)(swapchain.extent.width);
    viewport.height = -(float)(swapchain.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(graphicsCmdBuffers[currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain.extent;
    vkCmdSetScissor(graphicsCmdBuffers[currentFrame], 0, 1, &scissor);
    /*************************************************************/

    auto descriptorSet = vkRourceManager->getDescriptorSets();

    //bind descriptor sets here
    vkCmdBindDescriptorSets(
        graphicsCmdBuffers[currentFrame],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pbrPipeline.layout,
        0,                  // firstSet: set = 0
        1,                  // descriptorSetCount
        &descriptorSet,
        0,
        nullptr
    );
}

void VulkanRenderer::drawPBR(VkBuffer indexBuffer, VkBuffer* vertexBuffers, uint32_t numIndices, uint32_t indexOffset, 
                             glm::mat4& model, glm::ivec4& pbrMat) {
    VulkanUniformInfos::PBRConstant pConst;
    //TODO: populate pConst here
    pConst.frameIdx = currentFrame;

    vkCmdPushConstants(graphicsCmdBuffers[currentFrame], pbrPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(VulkanUniformInfos::PBRConstant), &pConst);

    VkDeviceSize offsets[5] = {0, 0, 0, 0, 0};
    vkCmdBindVertexBuffers(graphicsCmdBuffers[currentFrame], 0, 5, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(graphicsCmdBuffers[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(graphicsCmdBuffers[currentFrame], numIndices, 1, indexOffset, 0, 0);
}

void VulkanRenderer::drawPhong(VkBuffer indexBuffer, VkBuffer* vertexBuffers, uint32_t numIndices, uint32_t indexOffset,
                                glm::mat4& model) {
    VulkanUniformInfos::PhongConstant pConst;
    pConst.frameIndex = currentFrame;
    pConst.model = model;
    vkCmdPushConstants(graphicsCmdBuffers[currentFrame], simplePipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0, sizeof(VulkanUniformInfos::PhongConstant), &pConst);

    VkDeviceSize offsets[3] = { 0, 0, 0};
    vkCmdBindVertexBuffers(graphicsCmdBuffers[currentFrame], 0, 3, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(graphicsCmdBuffers[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(graphicsCmdBuffers[currentFrame], numIndices, 1, indexOffset, 0, 0);
}

void VulkanRenderer::submitDrawCalls() {
    vkCmdEndRendering(graphicsCmdBuffers[currentFrame]);

    VulkanImageUtils::transitionImageLayout(swapchain.images[imageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, graphicsCmdBuffers[currentFrame]);

    if (vkEndCommandBuffer(graphicsCmdBuffers[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    //std::vector<VkSemaphore> waitSemaphores = { computeFinishedSemaphores[currentFrame], imageAvailableSemaphores[currentFrame] };
    std::vector<VkSemaphore> waitSemaphores = {imageAvailableSemaphores[currentFrame] };
    //we cannot output color until image becomes availble
    std::vector<VkPipelineStageFlags> waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = waitSemaphores.size();
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = waitStages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &graphicsCmdBuffers[currentFrame];

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(context->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapchain.getSwapchain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    VkResult result = vkQueuePresentKHR(context->getPresentQueue(), &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || context->getWindowResized()) {
        recreateSwapchain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    //update frame idx
    currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
}

VkCommandBuffer VulkanRenderer::getDrawCmdBuf() const {
    return graphicsCmdBuffers[currentFrame];
}

void VulkanRenderer::createDepthMap() {
    depthFormat = VulkanImageUtils::findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                          VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL,
                                                       VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, context->getPhysicalDevice());
    depthImage = VulkanImageUtils::createImage2D(swapchain.extent.width, swapchain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context);
}

void VulkanRenderer::createUniformBuffers() {
    perFrameUBOs.reserve(FRAMES_IN_FLIGHT);
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        perFrameUBOs.push_back(VulkanBuffer(sizeof(VulkanUniformInfos::PerFrameUBO),
                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context));
        perFrameUBOs.back().map();

        vkRourceManager->addDescriptor(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &perFrameUBOs.back());
    }
}

void VulkanRenderer::createPipelines() {
    std::vector<VkDescriptorSetLayout> layouts = { vkRourceManager->getDescriptorSetLayouts() };
    pbrPipeline = VulkanPipeline("./src/shaders/pbr_vert.spv", "./src/shaders/pbr_frag.spv", 
                                      VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, 
                                      swapchain.extent, swapchain.format, depthFormat, sizeof(VulkanUniformInfos::PBRConstant),
                                      layouts, context);
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
        if (vkCreateSemaphore(context->getLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->getLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->getLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    /*computeInFlightFences.resize(FRAMES_IN_FLIGHT);
    computeFinishedSemaphores.resize(FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context->getLogicalDevice(), &semaphoreInfo, nullptr, &computeFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->getLogicalDevice(), &fenceInfo, nullptr, &computeInFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute synchronization objects for a frame!");
        }
    }*/
}

void VulkanRenderer::recreateSwapchain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(context->getWindow(), &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(context->getWindow(), &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(context->getLogicalDevice());
    swapchain = VulkanSwapchain::recreateSwapchain(context, swapchain);

    //vkDestroyImageView(context->getLogicalDevice(), depthImage.vkImageView, nullptr);
    createDepthMap();

    //reset windowResided when the new swapchain is ready
    context->setWindowResized(false);
}

void VulkanRenderer::createCommandBuffers() {
    graphicsCmdBuffers.resize(FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = (uint32_t)graphicsCmdBuffers.size();
    allocInfo.commandPool = graphicsCmdPool.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(context->getLogicalDevice(), &allocInfo, graphicsCmdBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    /*
    computeCmdBuffers.resize(FRAMES_IN_FLIGHT);

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = (uint32_t)computeCmdBuffers.size();
    allocInfo.commandPool = computeCmdPool.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(context->getLogicalDevice(), &allocInfo, computeCmdBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }*/
}

void VulkanRenderer::cleanUp() {
    for (VulkanBuffer& pfUbo : perFrameUBOs) {
        pfUbo.cleanUp();
    }

    pbrPipeline.cleanUp();

    for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context->getLogicalDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context->getLogicalDevice(), imageAvailableSemaphores[i], nullptr);
        //vkDestroySemaphore(context->getLogicalDevice(), computeFinishedSemaphores[i], nullptr);
        vkDestroyFence(context->getLogicalDevice(), inFlightFences[i], nullptr);
    }

    graphicsCmdPool.freeCommandBuffers(graphicsCmdBuffers.data(), graphicsCmdBuffers.size());
    //computeCmdPool.freeCommandBuffers(computeCmdBuffers.data(), computeCmdBuffers.size());

    graphicsCmdPool.cleanUp();
    transferCmdPool.cleanUp();
}

VkFormat VulkanRenderer::getSwapChainColorFormat() const {
    return swapchain.format;
}

VkFormat VulkanRenderer::getDepthFormat() const {
    return depthFormat;
}

/*void VulkanRenderer::beginCompute() {
    vkWaitForFences(context->getLogicalDevice(), 1, &computeInFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->getLogicalDevice(), 1, &computeInFlightFences[currentFrame]);

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
            //VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
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
}*/