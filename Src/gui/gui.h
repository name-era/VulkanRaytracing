#pragma once

#include <vector>
#include <array>

#include <vulkan/vulkan.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <glfw3.h>

#include "device.h"
#include "swapchain.h"

class Gui {

public:
    /**
    * @brief    レンダーパスを作成する
    */
    void CreateRenderPass();

    /**
    * @brief    イメージを作成する
    */
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    /**
    * @brief    イメージビューを作成する
    */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    /**
    * @brief    デプスリソースを作成する
    */
    void CreateDepthResources();

    /**
    * @brief    フレームバッファを作成する
    */
    void CreateFrameBuffers(VkImageView imageView);
    void CreateCommandPool();
    void CreateSemaphore();
    void CreateDescriptorPool();
    void PrepareImGui(GLFWwindow* window, VkInstance& instance);
    void Present(uint32_t index);
    void Render(uint32_t imageIndex);
    void Draw();
    void Cleanup();
    void Recreate();
    void Destroy();
    void Connect(VulkanDevice* device);

private:

    VulkanDevice* _vulkanDevice;

    //ImGui
    Swapchain* _swapchain;
    VkDescriptorPool _descriptorPool;
    VkRenderPass _renderPass;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
    std::vector<VkFramebuffer> _frameBuffers;

    //depth
    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;

    VkSemaphore _presentCompleteSemaphore;
    VkSemaphore _renderCompleteSemaphore;
    VkFence _renderFences;
    uint32_t _index;
};