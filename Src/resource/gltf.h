#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include "dataStructure.h"
#include "device.h"

class glTF {
public:

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
        void LoadglTFImages(tinygltf::Image& gltfImage, VulkanDevice* device);
    };

    struct glTFMaterial {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        uint32_t baseColorTexture;
        Texture* baseColorTexture = nullptr;
    };

    struct Primitive {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t materialIndex;
    };

    struct Node {
        Node* parent;
        std::vector<Node> children;
        std::vector<Primitive> mesh;
        glm::mat4 matrix;
    };


    glTF(VulkanDevice& vulkanDevice);
    ~glTF();

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

    


private:

    VkCommandPool g_commandPool = VK_NULL_HANDLE;


    std::vector<Texture> _textures;
    std::vector<glTFMaterial> _gltfMaterials;

    std::vector<Node> _gltfNodes;

    Vertices _gltfVertices;
    Indices _gltfIndices;

    VulkanDevice* _vulkanDevice;
    uint32_t _mipLevel;



};