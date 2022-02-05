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


    /**
    * @brief    �C���[�W���쐬����
    */
    void CreateImage(uint32_t width, uint32_t height, VkImage& image, VkDeviceMemory& imageMemory);

    /**
    * @brief    �C���[�W���C�A�E�g�̎w��
    */
    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    
    /**
    * @brief    �o�b�t�@���C���[�W�ɃR�s�[����
    */
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    /**
    * @brief    �C���[�W�̏���
    */
    void PrepareImage();

    /**
    * @brief    �C���[�W�r���[���쐬����
    */
    void CreateImageView();
    
    /**
    * @brief    �T���v���[���쐬����
    */
    void CreateSampler();

    /**
    * @brief    �f�B�X�N���v�^�Z�b�g�̍쐬
    */
    void CreateDescriptorSet();

    /**
    * @brief    �p�C�v���C���̍쐬
    */
    void CreateGraphicsPipeline();

    /**
    * @brief    UI�̏���
    */
    void PrepareUI(VkInstance& instance, VkCommandPool& commandBuffers);

    /**
    * @brief    UI�̍X�V
    */
    void UpdateUI(float frameTimer, Initializers::MouseButtons mouseButtons, glm::vec2 mousePos);

    /**
    * @brief    �`��
    */
    void Draw(VkCommandBuffer commandBuffer);

    void Cleanup();
    void Recreate();

    /**
    * @brief    �j��
    */
    void Destroy();
    void Connect(VulkanDevice* device, VkQueue& queue);

private:

    VulkanDevice* _vulkanDevice;
    Shader* _shader;
    VkQueue _queue;

    Initializers::Buffer _vertexBuffer;
    Initializers::Buffer _indexBuffer;
    Initializers::Image _fontImage;

    //pipeline
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;

    //descriptor
    VkDescriptorPool _descriptorPool;
    VkDescriptorSetLayout _descriptorSetLayout;
    VkDescriptorSet _descriptorSet;
    
    Shader::ShaderModules _shaderModules;

    const uint32_t _width = 200;
    const uint32_t _height = 200;

};