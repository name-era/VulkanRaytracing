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
    * @brief    �����_�[�p�X���쐬����
    */
    void CreateRenderPass();

    /**
    * @brief    �C���[�W���쐬����
    */
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    /**
    * @brief    �C���[�W�r���[���쐬����
    */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    /**
    * @brief    �f�v�X���\�[�X���쐬����
    */
    void CreateDepthResources();

    /**
    * @brief    �t���[���o�b�t�@���쐬����
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