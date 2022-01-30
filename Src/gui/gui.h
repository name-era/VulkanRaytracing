#pragma once

#include <vulkan/vulkan.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

class Gui {

public:
    //imgui
    void CreateRenderPassForImGui();
    void CreateCommandPoolForImGui(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags);
    void CreateFrameBuffersForImGui();
    void CreateCommandBuffersForImGui(VkCommandBuffer* commandBuffer, uint32_t commandBufferCount, VkCommandPool& commandPool);
    void PrepareImGui();
    void PrepareRenderingImGui();
    void RenderImGui(uint32_t imageIndex);
    void CleanupImGui();

private:
    //ImGui
    VkDescriptorPool i_descriptorPool;
    VkRenderPass i_renderPass;
    VkCommandPool i_commandPool;
    std::vector<VkCommandBuffer> i_commandBuffers;
    std::vector<VkFramebuffer> i_frameBuffers;
};