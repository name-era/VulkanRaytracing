#include "gui.h"

void Gui::CreateRenderPass() {

    VkAttachmentDescription attachment = {};
    attachment.format = COLORFORMAT;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    if (vkCreateRenderPass(_vulkanDevice->_device, &info, nullptr, &_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Could not create Dear ImGui's render pass");
    }
}

void Gui::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (!(imageInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (vkCreateImage(_vulkanDevice->_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_vulkanDevice->_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _vulkanDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_vulkanDevice->_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_vulkanDevice->_device, image, imageMemory, 0);
}

VkImageView Gui::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(_vulkanDevice->_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Gui::CreateDepthResources() {

    VkFormat depthFormat = _vulkanDevice->FindDepthFormat();
    CreateImage(_swapchain->_extent.width, _swapchain->_extent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
    _depthImageView = CreateImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void Gui::CreateFrameBuffers(VkImageView imageView) {
    
    std::array<VkImageView, 2> attachments = {
    imageView,
    _depthImageView
    };

    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = _renderPass;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.data();
    info.width = _swapchain->_extent.width;
    info.height = _swapchain->_extent.height;
    info.layers = 1;

    VkFramebuffer frameBuffer;
    vkCreateFramebuffer(_vulkanDevice->_device, &info, nullptr, &frameBuffer);
    _frameBuffers.push_back(frameBuffer);
}

void Gui::CreateCommandPool() {

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = _vulkanDevice->_queueFamilyIndices.graphics;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(_vulkanDevice->_device, &commandPoolCreateInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Could not create graphics command pool");
    }
}

void Gui::CreateSemaphore() {
    _imageSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _fences.resize(MAX_FRAMES_IN_FLIGHT);
    _imagesInFlight.resize(_swapchain->_imageCount, VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(_vulkanDevice->_device, &semaphoreInfo, nullptr, &_imageSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(_vulkanDevice->_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(_vulkanDevice->_device, &fenceInfo, nullptr, &_fences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Gui::CreateDescriptorPool() {
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(_vulkanDevice->_device, &pool_info, nullptr, &_descriptorPool);
}

void Gui::PrepareImGui(GLFWwindow* window, VkInstance& instance) {

    _swapchain = new Swapchain();
    _swapchain->CreateSurface();
    _swapchain->CreateSwapChain();
    
    CreateCommandPool();
    CreateRenderPass();
    CreateDepthResources();

    for (uint32_t i = 0; i < _swapchain->_imageCount; i++){
        CreateFrameBuffers(_swapchain->_swapchainBuffers[i].imageview);
    }

    CreateDescriptorPool();
    CreateSemaphore();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForVulkan(window, true);


    ImGui_ImplVulkan_InitInfo info{};
    
    info.Instance = instance;
    info.PhysicalDevice = _vulkanDevice->_physicalDevice;
    info.Device = _vulkanDevice->_device;
    info.QueueFamily = _vulkanDevice->_queueFamilyIndices.graphics;
    info.Queue = _vulkanDevice->_graphicsQueue;
    info.PipelineCache = VK_NULL_HANDLE;
    info.DescriptorPool = _descriptorPool;
    info.Allocator = nullptr;
    info.MinImageCount = _swapchain->_minImageCount + 1;
    info.ImageCount = _swapchain->_minImageCount + 1;
    info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&info, _renderPass);

    //upload fonts
    VkCommandBuffer commandBuffer = _vulkanDevice->BeginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    _vulkanDevice->EndSingleTimeCommands(commandBuffer, _vulkanDevice->_graphicsQueue);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Gui::Present(uint32_t index) {
    
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(_vulkanDevice->_device, _swapchain->_swapchain, UINT64_MAX, _imageSemaphores[index], VK_NULL_HANDLE, &imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { _imageSemaphores[index] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;

    VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[index] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(_vulkanDevice->_device, 1, &_fences[index]);

    if (vkQueueSubmit(_vulkanDevice->_graphicsQueue, 1, &submitInfo, _fences[index]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { _swapchain->_swapchain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;
}

void Gui::Render(uint32_t imageIndex) {

    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();

    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(_commandBuffer, &info);


    VkRenderPassBeginInfo RenderPassinfo = {};
    VkClearValue clearValue{};
    clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

    RenderPassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassinfo.renderPass = _renderPass;
    RenderPassinfo.framebuffer = _frameBuffers[imageIndex];
    RenderPassinfo.renderArea.extent.width = _swapchain->_extent.width;
    RenderPassinfo.renderArea.extent.height = _swapchain->_extent.height;
    RenderPassinfo.clearValueCount = 1;
    RenderPassinfo.pClearValues = &clearValue;
    vkCmdBeginRenderPass(_commandBuffer, &RenderPassinfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(_commandBuffer);

    vkEndCommandBuffer(_commandBuffer);
}

void Gui::Draw() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ImGui ウィジェット
    ImGui::Begin("Info");
    ImGui::Text("Framerate(avg) %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    if (ImGui::Button("Button"))
    {

    }
    
    ImGui::SliderFloat("Rotate", &_vulkanDevice->_angle, 0.0f, 360.0f);
    ImGui::SliderFloat("PosX", &_vulkanDevice->_cmeraPosX, 2.0f, 10.0f);
    ImGui::SliderFloat("PosY", &_vulkanDevice->_cameraPosY, 2.0f, 10.0f);
    ImGui::SliderFloat("PosZ", &_vulkanDevice->_cameraPsZ, 2.0f, 10.0f);

    ImGui::ColorPicker4("Color", _vulkanDevice->_color);
    ImGui::End();
    Render(_index);
    Present(_index);
    _index = (_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Gui::CleanupImGui() {
    
    for (auto framebuffer : _frameBuffers) {
        vkDestroyFramebuffer(_vulkanDevice->_device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(_vulkanDevice->_device, _commandPool, 1, &_commandBuffer);
    vkDestroyCommandPool(_vulkanDevice->_device, _commandPool, nullptr);
    vkDestroyRenderPass(_vulkanDevice->_device, _renderPass, nullptr);
    vkDestroyDescriptorPool(_vulkanDevice->_device, _descriptorPool, nullptr);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Gui::Connect(VulkanDevice* device) {
    _vulkanDevice = device;
}

