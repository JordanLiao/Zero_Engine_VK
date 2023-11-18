#include "VulkanRenderer.h"
#include "VulkanBufferUtils.h"
#include "VulkanContext.h"

#include <stdexcept>
#include <iostream>

VulkanRenderer::VulkanRenderer(){}

VulkanRenderer::VulkanRenderer(VulkanContext* context) {
    this->context = context;
	swapchain = VulkanSwapchain(context);
    graphicsCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                    context->queueFamilyIndices.graphicsFamily.value(), context->logicalDevice);
    transferCmdPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                                    context->queueFamilyIndices.transferFamily.value(), context->logicalDevice);


    vkCmdBindDescriptorBuffersEXT = reinterpret_cast<PFN_vkCmdBindDescriptorBuffersEXT>(
                            vkGetDeviceProcAddr(context->logicalDevice, "vkCmdBindDescriptorBuffersEXT"));
    vkCmdSetDescriptorBufferOffsetsEXT = reinterpret_cast<PFN_vkCmdSetDescriptorBufferOffsetsEXT>(
                            vkGetDeviceProcAddr(context->logicalDevice, "vkCmdSetDescriptorBufferOffsetsEXT"));

    VulkanRendererUtils::createDescriptorSetLayouts(descriptorSetLayoutInfos, descriptorSetLayouts, 
                                                    descriptorSetLayoutSizes, context);
    uniDescAllocator = VulkanDescriptorAllocator(DESCRIPTOR_ALLOCATOR_BUFFER_SIZE, 
                                              VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT, context);
    texDescAllocator = VulkanDescriptorAllocator(DESCRIPTOR_ALLOCATOR_BUFFER_SIZE,
                                                 VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT, context);

    sampler2D = VulkanImageUtils::createSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, context);
    createDepthResources();
    createUniformBuffers();
    createDescriptorSets();
    createPipelines();
    createSyncObjects();
    createCommandBuffers();
}

void VulkanRenderer::beginDrawCalls(const glm::mat4& projView) {
    vkWaitForFences(context->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(context->logicalDevice, 1, &inFlightFences[currentFrame]);

    UniformBufferObject ubo{};
    ubo.projView = projView;
    ubo.viewPos = glm::vec3(0.f, 0.f, 5.f);
    perFrameUBOs[currentFrame].transferData(&ubo, sizeof(ubo));
    
    GlobalUniformBufferObject gubo[4];
    gubo[0].light = glm::vec3(1.f, 0.f, 0.f);
    gubo[1].light = glm::vec3(0.5f, 0.5f, 0.f);
    gubo[2].light = glm::vec3(0.f, 0.5f, 0.5f);
    gubo[3].light = glm::vec3(0.f, 0.f, 1.f);
    gubo[0].lightPosition = glm::vec3(-10.f, 10.f, 10.f);
    gubo[1].lightPosition = glm::vec3(-10.f, -10.f, 10.f);
    gubo[2].lightPosition = glm::vec3(10.f, 10.f, 10.f);
    gubo[3].lightPosition = glm::vec3(10.f, -10.f, 10.f);

    int s = sizeof(GlobalUniformBufferObject);
    globalUBO[0].transferData(gubo, sizeof(GlobalUniformBufferObject));
    globalUBO[1].transferData(gubo + 1, sizeof(GlobalUniformBufferObject));
    globalUBO[2].transferData(gubo + 2, sizeof(GlobalUniformBufferObject));
    globalUBO[3].transferData(gubo + 3, sizeof(GlobalUniformBufferObject));

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
    //viewport.y = 0.f;
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

    uint32_t uniformDescBufferIndex = 0, samplerDescBufferIndex = 1;
    VkDescriptorBufferBindingInfoEXT bindingInfo[2]{};
    bindingInfo[uniformDescBufferIndex].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
    bindingInfo[uniformDescBufferIndex].address = uniDescAllocator.deviceAddress;
    bindingInfo[uniformDescBufferIndex].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    bindingInfo[samplerDescBufferIndex].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
    bindingInfo[samplerDescBufferIndex].address = texDescAllocator.deviceAddress;
    bindingInfo[samplerDescBufferIndex].usage = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

    //vkCmdBindDescriptorBuffersEXT(commandBuffers[currentFrame], 2, bindingInfo);
    vkCmdBindDescriptorBuffersEXT(commandBuffers[currentFrame], VulkanDescriptorAllocator::allocBindingInfos.size(), 
                                                                VulkanDescriptorAllocator::allocBindingInfos.data());

    uint32_t bufferIndices[2] = { uniformDescBufferIndex, uniformDescBufferIndex };
    VkDeviceSize offsets[2] = { globalDescOffset , perFrameDescOffsets[currentFrame] };
    vkCmdSetDescriptorBufferOffsetsEXT(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipeline.pipelineLayout, 0, 2, bufferIndices, offsets);
}

void VulkanRenderer::draw(VkBuffer indexBuffer, VkBuffer* vertexBuffers, uint32_t numIndices,  uint32_t indexOffset) {
    VkDeviceSize offsets[] = { 0,0 };
    vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 2, vertexBuffers, offsets);
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

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::createDepthResources() {
    depthFormat = VulkanImageUtils::findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                                          VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL,
                                                       VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, context->physicalDevice);
    VulkanImageUtils::createImage2D(depthImage, swapchain.extent.width, swapchain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, context);
}

void VulkanRenderer::createUniformBuffers() {
    globalUBO.resize(descriptorSetLayoutInfos[DescriptorSetLayoutIndex::global][0].descriptorCount);
    for (uint32_t i = 0; i < descriptorSetLayoutInfos[DescriptorSetLayoutIndex::global][0].descriptorCount; i++) {
        globalUBO[i] = VulkanBuffer(sizeof(GlobalUniformBufferObject),
                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context);
        globalUBO[i].map();
    }

    perFrameUBOs.reserve(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        perFrameUBOs.push_back(VulkanBuffer(sizeof(UniformBufferObject),
                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, context));
        perFrameUBOs.back().map();
    }
}

void VulkanRenderer::createDescriptorSets() {
    std::vector<std::vector<std::vector<VulkanBuffer*>>> globalUBOBufferPtrs(1, std::vector<std::vector<VulkanBuffer*>>(1));
    globalUBOBufferPtrs[0][0].reserve(globalUBO.size());
    for (VulkanBuffer& gubo : globalUBO) {
        globalUBOBufferPtrs[0][0].push_back(&gubo);
    }

    globalDescOffset = uniDescAllocator.getDescriptor(globalUBOBufferPtrs[0], descriptorSetLayoutInfos[DescriptorSetLayoutIndex::global],
                                                        descriptorSetLayouts[DescriptorSetLayoutIndex::global]);

    perFrameDescOffsets.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::vector<std::vector<VulkanBuffer*>> resourcePtr = { {&(perFrameUBOs[i])} };
        perFrameDescOffsets[i] = uniDescAllocator.getDescriptor(resourcePtr, descriptorSetLayoutInfos[DescriptorSetLayoutIndex::perFrame],
                                                                descriptorSetLayouts[DescriptorSetLayoutIndex::perFrame]);
    }
}

void VulkanRenderer::createPipelines() {
    pipeline = VulkanGraphicsPipeline("./src/shaders/vert.spv", "./src/shaders/frag.spv", 
                                      VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, 
                                      swapchain.extent, swapchain.format, depthFormat, 
                                      descriptorSetLayouts, context->logicalDevice);
}

void VulkanRenderer::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
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
    createDepthResources();

    context->resized = false;
}

void VulkanRenderer::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

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
    for (VulkanBuffer& gubo : globalUBO) {
        gubo.cleanUp();
    }

    for (VulkanBuffer& pfUbo : perFrameUBOs) {
        pfUbo.cleanUp();
    }

    uniDescAllocator.cleanUp();
    texDescAllocator.cleanUp();

    vkDestroySampler(context->logicalDevice, sampler2D, nullptr);
    for (size_t i = 0; i < descriptorSetLayouts.size(); i++) {
        vkDestroyDescriptorSetLayout(context->logicalDevice, descriptorSetLayouts[i], nullptr);
    }
    pipeline.cleanUp();

    vkDestroyImageView(context->logicalDevice, depthImage.vkImageView, nullptr);
    vkDestroyImage(context->logicalDevice, depthImage.vkImage, nullptr);
    vkFreeMemory(context->logicalDevice, depthImage.vkDeviceMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context->logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(context->logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(context->logicalDevice, inFlightFences[i], nullptr);
    }
    graphicsCmdPool.cleanUp();
    transferCmdPool.cleanUp();
	swapchain.cleanUp();
}