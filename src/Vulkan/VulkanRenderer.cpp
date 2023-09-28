#include "VulkanRenderer.h"
#include "VulkanBufferUtils.h"
#include "VulkanContext.h"

#include <stdexcept>
#include <iostream>

VulkanRenderer::VulkanRenderer(){}

VulkanRenderer::VulkanRenderer(VulkanContext& context) {
    this->instance = context.vulkanInstance;
    this->logicalDevice = context.logicalDevice;
    this->physicalDevice = context.physicalDevice;
    this->graphicsQueue = context.graphicsQueue;
    this->presentQueue = context.presentQueue;
	swapchain = VulkanSwapchain(context);
    commandPool = VulkanCommandPool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                    context.queueFamilyIndices.graphicsFamily.value(),
                                    context.logicalDevice);
    createDescriptorPool();
    createDescriptorSetLayouts();
    createUniformBuffers();
    createDepthResources();
    createDescriptorSets();
    createPipelines();
    createSyncObjects();
    createCommandBuffers();
}

void VulkanRenderer::beginDrawCalls(const glm::mat4& projView) {
    vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

    UniformBufferObject ubo{};
    ubo.projView = projView;
    perFrameUBOs[currentFrame].transferData(&ubo, sizeof(ubo));

    GlobalUniformBufferObject gubo{};
    gubo.light = glm::vec3(0.3f, 0.3f, 0.3f);
    VulkanBuffer globalDescriptorSetBuffer;
    gubo.lightPosition = glm::vec3(0.f, 0.f, 3.f);
    globalUBO.transferData(&gubo, sizeof(gubo));

    //imageIndex returned by vkAcquireNextImageKHR is only guranteed to be availble next, but it may not 
    //be available immediately, so a semaphore is needed to synchronize vkcommands that depend on the image.
    VkResult result = vkAcquireNextImageKHR(logicalDevice, swapchain.swapchain, UINT64_MAX, 
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
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

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    VkImageMemoryBarrier imageToRenderBarrier{};
    imageToRenderBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageToRenderBarrier.srcAccessMask = 0;
    imageToRenderBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageToRenderBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageToRenderBarrier.image = swapchain.images[imageIndex];
    imageToRenderBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(
        commandBuffers[currentFrame],
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // srcStageMask
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
        0,
        0,
        nullptr,
        0,
        nullptr,
        1, // imageMemoryBarrierCount
        &imageToRenderBarrier // pImageMemoryBarriers
    );

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

    /***dynamic states*********/
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

    VkDescriptorBufferBindingInfoEXT bindingInfo[2]{};
    bindingInfo[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
    bindingInfo[0].address = globalDescriptorSetBufferDeviceAddr;
    bindingInfo[0].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    // Binding 1 = Image
    bindingInfo[1].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
    bindingInfo[1].address = perFrameDescriptorSetBuffersDeviceAddr;
    bindingInfo[1].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    VulkanCommon::vkCmdBindDescriptorBuffersEXT(commandBuffers[currentFrame], 2, bindingInfo);

    uint32_t globalBufferIndex = DescriptorSetLayoutIndex::global;
    uint32_t perFrameBufferIndex = DescriptorSetLayoutIndex::perFrame;
    VkDeviceSize bufferOffset = 0;

    VulkanCommon::vkCmdSetDescriptorBufferOffsetsEXT(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, 
                                       pipeline.pipelineLayout, 0, 1, &(globalBufferIndex), &bufferOffset);
    VulkanCommon::vkCmdSetDescriptorBufferOffsetsEXT(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                       pipeline.pipelineLayout, 1, 1, &(perFrameBufferIndex), &bufferOffset);
}

void VulkanRenderer::draw(uint32_t numIndices, VkBuffer indexBuffer, VkBuffer* vertexBuffers) {
    VkDeviceSize offsets[] = { 0,0 };
    vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 2, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffers[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffers[currentFrame], numIndices, 1, 0, 0, 0);
}

void VulkanRenderer::submitDrawCalls() {
    vkCmdEndRendering(commandBuffers[currentFrame]);

    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;

    VkImageMemoryBarrier imageToPresentBarrier{};
    imageToPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageToPresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    imageToPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    imageToPresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    imageToPresentBarrier.image = swapchain.images[imageIndex];
    imageToPresentBarrier.subresourceRange = subresourceRange;

    vkCmdPipelineBarrier(
        commandBuffers[currentFrame],
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
        0,
        0,
        nullptr,
        0,
        nullptr,
        1, // imageMemoryBarrierCount
        &imageToPresentBarrier // pImageMemoryBarriers
    );

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

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
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

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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
        if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void VulkanRenderer::createCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
    allocInfo.commandPool = commandPool.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void VulkanRenderer::createDescriptorPool() {
    //create a size info struct for each type of descriptor, like uniform buffer, or image samplers...
    std::vector<VkDescriptorPoolSize> poolSizes(descriptorTypes.size());
    for (int i = 0; i < poolSizes.size(); i++) {
        poolSizes[i].type = descriptorTypes[i]; 
        poolSizes[i].descriptorCount = 100; //how many descriptor of this type
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT + 1);

    if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void VulkanRenderer::createDescriptorSetLayouts() {
    descriptorSetLayouts.reserve(descriptorSetLayoutInfos.size());
    descriptorSetLayoutSizes.reserve(descriptorSetLayoutInfos.size());

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
    for(const std::vector<DescriptorSetBindingInfo>& layoutInfo: descriptorSetLayoutInfos) {
        std::vector<VkDescriptorSetLayoutBinding> perFrameDescriptorSetBindings(layoutInfo.size());
        int i = 0;
        for (size_t i = 0; i < layoutInfo.size(); i++) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = (uint32_t)i;
            layoutBinding.descriptorCount = layoutInfo[i].numDescriptor;
            layoutBinding.descriptorType = layoutInfo[i].type;
            layoutBinding.stageFlags = layoutInfo[i].stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            perFrameDescriptorSetBindings[i] = layoutBinding;
        }

        layoutCreateInfo.bindingCount = (uint32_t)layoutInfo.size();
        layoutCreateInfo.pBindings = perFrameDescriptorSetBindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(logicalDevice, &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        descriptorSetLayouts.push_back(layout);
        VkDeviceSize layoutSize;
        VulkanCommon::vkGetDescriptorSetLayoutSizeEXT(logicalDevice, layout, &layoutSize);
        descriptorSetLayoutSizes.push_back(layoutSize);
    }
}

void VulkanRenderer::createUniformBuffers() {
    globalUBO = VulkanBuffer(sizeof(GlobalUniformBufferObject), 
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                             logicalDevice, physicalDevice);
    globalUBO.map();

    perFrameUBOs.reserve(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        perFrameUBOs.push_back(VulkanBuffer(sizeof(UniformBufferObject),
                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                logicalDevice, physicalDevice));
        perFrameUBOs.back().map();
    }
}

void VulkanRenderer::createDepthResources() {
    depthFormat = VulkanImageUtils::findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, 
                                                                  VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL,
                                                                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, physicalDevice);
    VulkanImageUtils::createImage2D(swapchain.extent.width, swapchain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
                                        depthImage, logicalDevice);
}

void VulkanRenderer::createDescriptorSets() {
    globalDescriptorSetBuffer = VulkanBuffer(descriptorSetLayoutSizes[DescriptorSetLayoutIndex::global],
                                    VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    logicalDevice, physicalDevice);
    globalDescriptorSetBuffer.map();
    globalDescriptorSetBufferDeviceAddr = VulkanBufferUtils::getBufferDeviceAddress(globalDescriptorSetBuffer.vkBuffer);
    
    perFrameDescriptorSetBuffers = VulkanBuffer(MAX_FRAMES_IN_FLIGHT * descriptorSetLayoutSizes[DescriptorSetLayoutIndex::perFrame],
                                VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                logicalDevice, physicalDevice);
    perFrameDescriptorSetBuffers.map();
    perFrameDescriptorSetBuffersDeviceAddr = VulkanBufferUtils::getBufferDeviceAddress(perFrameDescriptorSetBuffers.vkBuffer);
    
    //need to buffer alignment sizes for each descriptor
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties{};
    descriptorBufferProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
    VkPhysicalDeviceProperties2 deviceProperties{};
    deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    deviceProperties.pNext = &descriptorBufferProperties;
    VulkanCommon::vkGetPhysicalDeviceProperties2KHR(physicalDevice, &deviceProperties);
    
    uniformDescriptorOffset = VulkanBufferUtils::getAlignedBufferSize(descriptorBufferProperties.uniformBufferDescriptorSize,
                                                                     descriptorBufferProperties.descriptorBufferOffsetAlignment);

    VkDescriptorAddressInfoEXT addressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
    addressInfo.address =  VulkanBufferUtils::getBufferDeviceAddress(globalUBO.vkBuffer);
    addressInfo.range = globalUBO.size;
    addressInfo.format = VK_FORMAT_UNDEFINED;

    VkDescriptorGetInfoEXT bufferDescriptorInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
    bufferDescriptorInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bufferDescriptorInfo.data.pUniformBuffer = &addressInfo;
    VulkanCommon::vkGetDescriptorEXT(logicalDevice, &bufferDescriptorInfo, descriptorBufferProperties.uniformBufferDescriptorSize,
                                     globalDescriptorSetBuffer.data);
    
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorAddressInfoEXT perFrameAddressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
        perFrameAddressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(perFrameUBOs[i].vkBuffer);
        perFrameAddressInfo.range = perFrameUBOs[i].size;
        perFrameAddressInfo.format = VK_FORMAT_UNDEFINED;
        
        bufferDescriptorInfo.data.pUniformBuffer = &perFrameAddressInfo;
        VulkanCommon::vkGetDescriptorEXT(logicalDevice, &bufferDescriptorInfo, descriptorBufferProperties.uniformBufferDescriptorSize, 
                      (char*)perFrameDescriptorSetBuffers.data + i * descriptorSetLayoutSizes[DescriptorSetLayoutIndex::perFrame]);
    }
}

void VulkanRenderer::createPipelines() {
    pipeline = VulkanGraphicsPipeline("./src/shaders/vert.spv", "./src/shaders/frag.spv", 
                                      VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, 
                                      logicalDevice, swapchain.extent, swapchain.format, depthFormat, descriptorSetLayouts);
}

VulkanRenderer::~VulkanRenderer() {
	cleanup();
}

void VulkanRenderer::cleanup() {
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
    }
    commandPool.cleanup();
	swapchain.cleanup();
}

