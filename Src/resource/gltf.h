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

class gltf {
public:

    struct glTFImage {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        VkDescriptorSet descriptorSet;
    };

    struct glTFMaterial {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        uint32_t _gltfBaseColorTextureIndex;
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


    gltf(VulkanDevice& vulkanDevice);
    ~gltf();

public:
    /**
    * @brief    コピーコンストラクタの禁止
    */
    gltf(const gltf&);
    gltf& operator=(const gltf&);


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


    std::vector<glTFImage> _gltfImages;
    std::vector<glTFMaterial> _gltfMaterials;
    std::vector<uint32_t> _gltfTextures;

    std::vector<Node> _gltfNodes;

    Vertices _gltfVertices;
    Indices _gltfIndices;
    
    VulkanDevice* _vulkanFunc;
    VkDevice _device;
    VkPhysicalDevice _physicalDevice;
    uint32_t _mipLevel;


};