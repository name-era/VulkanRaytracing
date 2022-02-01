#pragma once
#include <iostream>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

#include "device.h"
#include "shader.h"

class glTF {
public:

    struct Vertices {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

    struct Indices {
        int count;
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

    struct Vertex {

        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }


        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {

            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, normal);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, uv);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };

    struct Texture {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        VkDescriptorSet descriptorSet;

        VulkanDevice* vulkanDevice;
        uint32_t mipLevel;
        VkQueue queue;

        /**
        * @brief    イメージの作成
        */
        void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        /**
        * @brief    イメージレイアウトの作成
        */
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

        /**
        * @brief    バッファをイメージにコピーする
        */
        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        /**
        * @brief    イメージの準備
        */
        void PrepareImage(void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight);

        /**
        * @brief    イメージビューを作成する
        */
        VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

        /**
        * @brief    サンプラーを作成する
        */
        void CreateSampler();

        /**
        * @brief    イメージの読み込み
        */
        void LoadglTFImages(tinygltf::Image& gltfImage, VulkanDevice* device, VkQueue transQueue);
    };

    struct glTFMaterial {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        Texture* baseColorTexture = nullptr;
        VkDescriptorSet descriptorSet;
    };

    struct Primitive {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t materialIndex;
    };

    struct UniformBuffer {
        VkBuffer buffer;
        VkDeviceMemory memory;
    }_uniformBuffer;

    struct UniformBlock {
        glm::mat4 projection;
        glm::mat4 model;
        glm::vec4 lightPos = glm::vec4(5.0f, 5.0f, -5.0f, 1.0f);
    }_ubo;

    struct Node {
        Node* parent;
        std::vector<Node> children;
        std::vector<Primitive> primitive;
        glm::mat4 matrix;
    };

    struct DescriptorLayouts {
        VkDescriptorSetLayout matrix;
        VkDescriptorSetLayout texture;
    };


    glTF();
    ~glTF() {};

public:

    /**
    * @brief    コピーコンストラクタの禁止
    */
    glTF(const glTF&);
    glTF& operator=(const glTF&);


    void LoadImages(tinygltf::Model& gltfModel);
    Texture* GetTexture(uint32_t index);
    void LoadglTFMaterials(tinygltf::Model& input);
    void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer);
    void LoadFromFile(std::string filename, VulkanDevice* device);


    /**
    * @brief    ディスクリプタプールの作成
    */
    void CreateDescriptorPool();

    /**
    * @brief    ディスクリプタレイアウトの作成
    */
    void CreateDescriptorSetLayout();

    /**
    * @brief    ディスクリプタセットを作成する
    */
    void CreateDescriptorSets();
    
    /**
    * @brief    ディスクリプタ関連の作成
    */
    void CreateDescriptors();

    /**
    * @brief    初期化
    */
    void Connect(VulkanDevice* device);

    /**
    * @brief    ノード描画
    */
    void DrawNode(VkCommandBuffer& commandBuffer, VkPipelineLayout pipelineLayout, Node node);

    /**
    * @brief    モデル描画
    */
    void Draw(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout);

    /**
    * @brief    ユニフォームバッファの更新
    */
    void UpdateUniformBuffer(glm::mat4 projection, glm::mat4 view);

    /**
    * @brief    破棄
    */
    void Cleanup();

    DescriptorLayouts _descriptorSetLayout;

private:
    std::vector<Texture> _textures;
    std::vector<glTFMaterial> _materials;
    std::vector<Node> _nodes;

    Vertices _vertices;
    Indices _indices;

    VulkanDevice* _vulkanDevice;

    uint32_t _mipLevel;
    VkQueue _queue;

    VkDescriptorPool _descriptorPool;
    VkDescriptorSet _uniformDescriptorSet;
};