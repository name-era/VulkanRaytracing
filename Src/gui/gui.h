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

    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate;
    } _pushConstBlock;

    struct UIVert
    {
        ImVec2  pos;
        ImVec2  uv;
        ImU32   color;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(ImDrawVert);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {

            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(UIVert, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(UIVert, uv);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(UIVert, color);

            return attributeDescriptions;
        }
    };

    struct ShaderModule {
        Shader::ShaderModuleInfo vert;
        Shader::ShaderModuleInfo frag;
    }_shaderModules;

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
    void CreateGraphicsPipeline();




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


    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate;
    } pushConstBlock;

    Initializers::Buffer _vertexBuffer;
    Initializers::Buffer _indexBuffer;
    Initializers::Image _fontImage;
    VkCommandBuffer _commandBuffer;
    VkDescriptorPool _descriptorPool;
    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorSet _descriptorSet;


    const uint32_t _width = 200;
    const uint32_t _height = 200;

};