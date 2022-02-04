#pragma once


#include <vector>
#include <array>

#include <vulkan/vulkan.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <glfw3.h>
#include <glm/glm.hpp>

#include "device.h"
#include "shader.h"

class Gui {

public:

    void SetMousePos(float x, float y);

    /**
    * @brief    イメージを作成する
    */
    void CreateImage(uint32_t width, uint32_t height, VkImage& image, VkDeviceMemory& imageMemory);

    /**
    * @brief    イメージレイアウトの指定
    */
    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    
    /**
    * @brief    バッファをイメージにコピーする
    */
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    /**
    * @brief    イメージの準備
    */
    void PrepareImage();

    /**
    * @brief    イメージビューを作成する
    */
    VkImageView CreateImageView(VkImage& image, VkImageView& imageView);
    
    /**
    * @brief    サンプラーを作成する
    */
    void CreateSampler();

    /**
    * @brief    ディスクリプタセットの作成
    */
    void CreateDescriptorSet();

    /**
    * @brief    パイプラインの作成
    */
    void PreparePipeline();




    /**
    * @brief    フレームバッファを作成する
    */
    void CreateFrameBuffers(VkImageView imageView);
    /**
    * @brief    レンダーパスを作成する
    */
    void CreateRenderPass();

    
    void PrepareImGui(VkInstance& instance, VkCommandPool& commandBuffers);
    void Draw(VkCommandBuffer commandBuffer);
    void Cleanup();
    void Recreate();
    void Destroy();
    void Connect(VulkanDevice* device, VkQueue& queue);

private:

    glm::vec2 mousePos;


    VulkanDevice* _vulkanDevice;
    Shader* _shader;
    VkQueue _queue;


    //ImGui
    VkRenderPass _renderPass;
    std::vector<VkFramebuffer> _frameBuffers;

    //new
    VkPipeline _pipeline;
    VkPipelineLayout _pipelineLayout;

    std::array<Shader::ShaderModuleInfo, 2> _shaderModules;

    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock;

    common::Buffer _vertexBuffer;
    common::Buffer _indexBuffer;
    common::Image _fontImage;
    VkCommandBuffer _commandBuffer;
    VkDescriptorPool _descriptorPool;
    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorSet _descriptorSet;


    const uint32_t _width = 200;
    const uint32_t _height = 200;

};