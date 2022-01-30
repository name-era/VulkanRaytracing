#include "gui.h"


void Gui::CreateRenderPassForImGui() {

    //最小限に作成
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_physicalDevice);

    VkAttachmentDescription attachment = {};
    attachment.format = _swapChainImageFormat;
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
    if (vkCreateRenderPass(_device, &info, nullptr, &i_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Could not create Dear ImGui's render pass");
    }
}

void Gui::CreateCommandPoolForImGui(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags) {

    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(_physicalDevice);
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    commandPoolCreateInfo.flags = flags;

    if (vkCreateCommandPool(_device, &commandPoolCreateInfo, nullptr, commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Could not create graphics command pool");
    }
}

void Gui::CreateFrameBuffersForImGui() {

    i_frameBuffers.resize(_swapChainResources.size());

    for (size_t i = 0; i < _swapChainResources.size(); i++) {

        VkImageView attachment[1];
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = i_renderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment;
        info.width = _swapChainExtent.width;
        info.height = _swapChainExtent.height;
        info.layers = 1;

        attachment[0] = _swapChainResources[i]->swapChainImageView;

        vkCreateFramebuffer(_device, &info, nullptr, &i_frameBuffers[i]);

    }
}

void Gui::CreateCommandBuffersForImGui(VkCommandBuffer* commandBuffer, uint32_t commandBufferCount, VkCommandPool& commandPool) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.commandBufferCount = commandBufferCount;
    vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, commandBuffer);
}

void Gui::PrepareImGui() {

    //imgui用のディスクリプタプールを作成
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

    vkCreateDescriptorPool(_device, &pool_info, nullptr, &i_descriptorPool);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForVulkan(_window, true);

    QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);

    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = _instance;
    info.PhysicalDevice = _physicalDevice;
    info.Device = _device;
    info.QueueFamily = indices.graphicsFamily.value();
    info.Queue = _graphicsQueue;
    info.PipelineCache = VK_NULL_HANDLE;
    info.DescriptorPool = i_descriptorPool;
    info.Allocator = nullptr;

    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_physicalDevice);

    info.MinImageCount = swapChainSupport.capabilities.minImageCount + 1;
    info.ImageCount = swapChainSupport.capabilities.minImageCount + 1;
    info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&info, i_renderPass);


    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    EndSingleTimeCommands(commandBuffer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Gui::PrepareRenderingImGui() {

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    // ImGui ウィジェットを描画する
    ImGui::Begin("Info");
    ImGui::Text("Framerate(avg) %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    if (ImGui::Button("Button"))
    {

    }

    ImGui::SliderFloat("Rotate", &_angle, 0.0f, 360.0f);
    ImGui::SliderFloat("PosX", &_cmeraPosX, 2.0f, 10.0f);
    ImGui::SliderFloat("PosY", &_cameraPosY, 2.0f, 10.0f);
    ImGui::SliderFloat("PosZ", &_cameraPsZ, 2.0f, 10.0f);

    ImGui::ColorPicker4("Color", _color);
    ImGui::End();
    ImGui::Render();
}

void Gui::RenderImGui(uint32_t imageIndex) {

    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(i_commandBuffers[imageIndex], &info);


    VkRenderPassBeginInfo RenderPassinfo = {};

    VkClearValue clearValue{};
    clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

    RenderPassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassinfo.renderPass = i_renderPass;
    RenderPassinfo.framebuffer = i_frameBuffers[imageIndex];
    RenderPassinfo.renderArea.extent.width = _swapChainExtent.width;
    RenderPassinfo.renderArea.extent.height = _swapChainExtent.height;
    RenderPassinfo.clearValueCount = 1;
    RenderPassinfo.pClearValues = &clearValue;
    vkCmdBeginRenderPass(i_commandBuffers[imageIndex], &RenderPassinfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), i_commandBuffers[imageIndex]);


    vkCmdEndRenderPass(i_commandBuffers[imageIndex]);
    vkEndCommandBuffer(i_commandBuffers[imageIndex]);
}

void Gui::CleanupImGui() {
    for (auto framebuffer : i_frameBuffers) {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(_device, i_renderPass, nullptr);

    vkFreeCommandBuffers(_device, i_commandPool, static_cast<uint32_t>(i_commandBuffers.size()), i_commandBuffers.data());
    vkDestroyCommandPool(_device, i_commandPool, nullptr);
    vkDestroyDescriptorPool(_device, i_descriptorPool, nullptr);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}