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

    vkGetDescriptorSetLayoutSizeEXT = reinterpret_cast<PFN_vkGetDescriptorSetLayoutSizeEXT>(
                            vkGetDeviceProcAddr(logicalDevice, "vkGetDescriptorSetLayoutSizeEXT"));
    vkGetDescriptorSetLayoutBindingOffsetEXT = reinterpret_cast<PFN_vkGetDescriptorSetLayoutBindingOffsetEXT>(
                            vkGetDeviceProcAddr(logicalDevice, "vkGetDescriptorSetLayoutBindingOffsetEXT"));
    vkCmdBindDescriptorBuffersEXT = reinterpret_cast<PFN_vkCmdBindDescriptorBuffersEXT>(
                            vkGetDeviceProcAddr(logicalDevice, "vkCmdBindDescriptorBuffersEXT"));
    vkCmdSetDescriptorBufferOffsetsEXT = reinterpret_cast<PFN_vkCmdSetDescriptorBufferOffsetsEXT>(
                            vkGetDeviceProcAddr(logicalDevice, "vkCmdSetDescriptorBufferOffsetsEXT"));
    vkGetDescriptorEXT = reinterpret_cast<PFN_vkGetDescriptorEXT>(
                            vkGetDeviceProcAddr(logicalDevice, "vkGetDescriptorEXT"));

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
    ubo.viewPos = glm::vec3(0.f, 0.f, 5.f);
    perFrameUBOs[currentFrame].transferData(&ubo, sizeof(ubo));
    
    GlobalUniformBufferObject gubo[4];
    gubo[0].light = glm::vec3(1.f, 0.f, 0.f);
    gubo[1].light = glm::vec3(0.5f, 0.5f, 0.f);
    gubo[2].light = glm::vec3(0.f, 0.5f, 0.5f);
    gubo[3].light = glm::vec3(0.f, 0.f, 1.f);
    gubo[0].lightPosition = glm::vec3(-15.f, 5.f, 0.f);
    gubo[1].lightPosition = glm::vec3(-3.33f, 10.f, 0.f);
    gubo[2].lightPosition = glm::vec3(3.33f, 10.f, 0.f);
    gubo[3].lightPosition = glm::vec3(10.f, 5.f, 0.f);

    int s = sizeof(GlobalUniformBufferObject);
    globalUBO[0].transferData(gubo, sizeof(GlobalUniformBufferObject));
    globalUBO[1].transferData(gubo + 1, sizeof(GlobalUniformBufferObject));
    globalUBO[2].transferData(gubo + 2, sizeof(GlobalUniformBufferObject));
    globalUBO[3].transferData(gubo + 3, sizeof(GlobalUniformBufferObject));


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

    uint32_t globalIndex = DescriptorSetLayoutIndex::global;
    uint32_t perFrameIndex = DescriptorSetLayoutIndex::perFrame;
    VkDescriptorBufferBindingInfoEXT bindingInfo[2]{};
    bindingInfo[globalIndex].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
    bindingInfo[globalIndex].address = globalDescriptorSetBufferDeviceAddr;
    bindingInfo[globalIndex].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;

    bindingInfo[perFrameIndex].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
    bindingInfo[perFrameIndex].address = perFrameDescriptorSetBuffersDeviceAddr;
    bindingInfo[perFrameIndex].usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
    vkCmdBindDescriptorBuffersEXT(commandBuffers[currentFrame], 2, bindingInfo);

    VkDeviceSize bufferOffset = 0;
    vkCmdSetDescriptorBufferOffsetsEXT(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, 
                                       pipeline.pipelineLayout, 0, 1, &globalIndex, &bufferOffset);
    vkCmdSetDescriptorBufferOffsetsEXT(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                       pipeline.pipelineLayout, 1, 1, &perFrameIndex, &bufferOffset);
}

void VulkanRenderer::draw(uint32_t numIndices, VkBuffer indexBuffer, VkBuffer* vertexBuffers) {
    VkDeviceSize offsets[] = { 0,0 };
    vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 2, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffers[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffers[currentFrame], numIndices, 1, 0, 0, 0);
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
        std::vector<VkDescriptorSetLayoutBinding> bindings(layoutInfo.size());
        int i = 0;
        for (size_t i = 0; i < layoutInfo.size(); i++) {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = (uint32_t)i;
            layoutBinding.descriptorCount = layoutInfo[i].numDescriptor;
            layoutBinding.descriptorType = layoutInfo[i].type;
            layoutBinding.stageFlags = layoutInfo[i].stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            bindings[i] = layoutBinding;
        }

        layoutCreateInfo.bindingCount = (uint32_t)layoutInfo.size();
        layoutCreateInfo.pBindings = bindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(logicalDevice, &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        descriptorSetLayouts.push_back(layout);
        VkDeviceSize layoutSize;
        vkGetDescriptorSetLayoutSizeEXT(logicalDevice, layout, &layoutSize);
        descriptorSetLayoutSizes.push_back(layoutSize);
    }
}

void VulkanRenderer::createUniformBuffers() {
    globalUBO.resize(descriptorSetLayoutInfos[DescriptorSetLayoutIndex::global][0].numDescriptor);
    for (int i = 0; i < descriptorSetLayoutInfos[DescriptorSetLayoutIndex::global][0].numDescriptor; i++) {
        globalUBO[i] = VulkanBuffer(sizeof(GlobalUniformBufferObject),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            logicalDevice, physicalDevice);
        globalUBO[i].map();
    }

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
    
    //need buffer alignment sizes for each descriptor
    VkPhysicalDeviceDescriptorBufferPropertiesEXT descriptorBufferProperties{};
    descriptorBufferProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT;
    VkPhysicalDeviceProperties2 deviceProperties{};
    deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    deviceProperties.pNext = &descriptorBufferProperties;
    VulkanCommon::vkGetPhysicalDeviceProperties2KHR(physicalDevice, &deviceProperties);
    
    //uniformDescriptorOffset = VulkanBufferUtils::getAlignedBufferSize(descriptorBufferProperties.uniformBufferDescriptorSize,
    //                                                                 descriptorBufferProperties.descriptorBufferOffsetAlignment);

    VkDescriptorAddressInfoEXT globalAddressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
    globalAddressInfo.range = sizeof(GlobalUniformBufferObject);
    globalAddressInfo.format = VK_FORMAT_UNDEFINED;

    //fill the descriptor set buffers with actual descriptor sets
    VkDescriptorGetInfoEXT globalGetInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
    globalGetInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    globalGetInfo.data.pUniformBuffer = &globalAddressInfo;

    //actually must get the descriptor for 
    for (int i = 0; i < descriptorSetLayoutInfos[DescriptorSetLayoutIndex::global][0].numDescriptor; i++) {
        globalAddressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(globalUBO[i].vkBuffer);
        
        vkGetDescriptorEXT(logicalDevice, &globalGetInfo, descriptorBufferProperties.uniformBufferDescriptorSize,
            (char*)globalDescriptorSetBuffer.data + i * descriptorBufferProperties.uniformBufferDescriptorSize);
    }
    
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorAddressInfoEXT perFrameAddressInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT };
        perFrameAddressInfo.address = VulkanBufferUtils::getBufferDeviceAddress(perFrameUBOs[i].vkBuffer);
        perFrameAddressInfo.range = perFrameUBOs[i].size;
        perFrameAddressInfo.format = VK_FORMAT_UNDEFINED;
        
        VkDescriptorGetInfoEXT perFrameGetInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT };
        perFrameGetInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        perFrameGetInfo.data.pUniformBuffer = &perFrameAddressInfo;
        //offset by the size of a one descriptor set.
        vkGetDescriptorEXT(logicalDevice, &perFrameGetInfo, descriptorBufferProperties.uniformBufferDescriptorSize,
                      (char*)perFrameDescriptorSetBuffers.data + i * descriptorSetLayoutSizes[DescriptorSetLayoutIndex::perFrame]);
    }
}

void VulkanRenderer::createPipelines() {
    pipeline = VulkanGraphicsPipeline("./src/shaders/vert.spv", "./src/shaders/frag.spv", 
                                      VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT, 
                                      logicalDevice, swapchain.extent, swapchain.format, depthFormat, descriptorSetLayouts);
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

