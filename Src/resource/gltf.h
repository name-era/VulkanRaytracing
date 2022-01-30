#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

class gltf {
public:

    struct glTFImage {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        VkDescriptorSet descriptorSet;
    };

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    VkDescriptorSet descriptorSet;

    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    uint32_t _gltfBaseColorTextureIndex;

    //glTFŠÖ˜A
    gltf();
    ~gltf();

    void CreateglTFImage(glTFImage* image, void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight);
    void CreateglTFImageView(glTFImage* image, VkFormat format);
    void CreateglTFSampler(glTFImage* image);
    void LoadglTFImages(tinygltf::Model& input);
    void LoadglTFMaterials(tinygltf::Model& input);
    void LoadglTFTextures(tinygltf::Model& input);
    void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer);
    void LoadglTF(std::string filename);

    uint32_t getImageNum();


private:

    VkCommandPool g_commandPool = VK_NULL_HANDLE;

    //glTF Images
    std::vector<glTFImage> _gltfImages;
    //glTF Materials
    std::vector<glTFMaterial> _gltfMaterials;
    //glTF Textures
    std::vector<Texture> _gltfTextures;

    std::vector<Node> _gltfNodes;

    Vertices _gltfVertices;
    Indices _gltfIndices;


};