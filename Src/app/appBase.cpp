#include "appBase.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include <stb_image.h>

/*******************************************************************************************************************
*                                             glTF
********************************************************************************************************************/

namespace glTF {

    enum RenderFlags {
        BindImages = 0x00000001,
        RenderOpaqueNodes = 0x00000002,
        RenderAlphaMaskedNodes = 0x00000004,
        RenderAlphaBlendedNodes = 0x00000008
    };

    enum DescriptorBindingFlags {
        ImageBaseColor = 0x00000001,
        ImageNormalMap = 0x00000002
    };
    uint32_t descriptorBidningFlags = DescriptorBindingFlags::ImageBaseColor;

    struct Texture {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
        VkSampler sampler;
        VulkanDevice* vulkanDevice;
        uint32_t mipLevel;
        VkQueue queue;

        /**
        * @brief    イメージの作成
        */
        void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

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
        void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);



        /**
        * @brief    イメージの読み込み
        */
        void LoadglTFImages(tinygltf::Image& gltfImage);

        /**
        * @brief    初期化
        */
        void Connect(VulkanDevice* device, VkQueue transQueue);

        /**
        * @brief    リソースの破棄
        */
        void Destroy();
    };

    struct Vertex {

        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec4 color;

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
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, uv);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };

    enum FileLoadingFlags {
        None = 0x00000000,
        PreTransformVertices = 0x00000001,
        PreMultiplyVertexColors = 0x00000002,
        FlipY = 0x00000004,
        DontLoadImages = 0x00000008
    };

    struct Material {
        glTF::Texture* baseColorTexture = nullptr;
        glTF::Texture* metallicRoughnessTexture = nullptr;
        glTF::Texture* normalTexture = nullptr;
        glTF::Texture* occlusionTexture = nullptr;
        glTF::Texture* emissiveTexture = nullptr;
        //glTF::Texture* specularGlossinessTexture;
        //glTF::Texture* diffuseTexture;

        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        float roughnessFactor = 1.0f;
        float metallicFactor = 1.0f;

        enum AlphaMode { ALPHA_OPAQUE, ALPHA_MASK, ALPHA_BLEND };
        AlphaMode alphaMode = ALPHA_OPAQUE;
        float alphaCutoff = 1.0f;

        VkDescriptorSet descriptorSet;
        void createDescriptorSet(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout);
    };

    struct Primitive {
        uint32_t firstIndex;
        uint32_t indexCount;
        uint32_t firstVertex;
        uint32_t vertexCount;
        Material& material;

        Primitive(uint32_t firstIndex, uint32_t indexCount, Material& material);
    };

    struct Mesh {
        std::vector<Primitive*> primitives;
        std::string name;

        struct UniformBuffer {
            VkBuffer buffer;
            VkDeviceMemory memory;
            VkDescriptorSet descriptorSet;
            void* mapped;
        }uniformBuffer;

        struct UniformBlock {
            glm::mat4 matrix;
            glm::mat4 jointMatrix[64]{};
            float jointcount{ 0 };
        }uniformBlock;

        VulkanDevice* vulkanDevice;
        Mesh(VulkanDevice* device, glm::mat4 matrix);
        ~Mesh();
    };

    struct Node {
        Node* parent;
        std::vector<Node*> children;
        Mesh* mesh;
        glm::mat4 matrix;
        std::string name;
        uint32_t index;
        glm::vec3 translation{};
        glm::vec3 scale{ 1.0f };
        glm::quat rotation{};
        
        /**
        * @brief   ローカル座標の取得
        */
        glm::mat4 localMatrix();

        /**
        * @brief   座標の取得
        */
        glm::mat4 getMatrix();
        
        /**
        * @brief    ユニフォームバッファの更新
        */
        void UpdateUniformBuffer(glm::mat4 projection, glm::mat4 view);

        ~Node();
    };

    struct DescriptorLayouts {
        VkDescriptorSetLayout ubo;
        VkDescriptorSetLayout image;
    }_descriptorSetLayout;

    class Model {
    public:

        Model();
        ~Model() {};

        /**
        * @brief    コピーコンストラクタの禁止
        */
        Model(const Model&);
        Model& operator=(const Model&);


        void LoadImages(tinygltf::Model& gltfModel);

        /**
        * @brief    テクスチャを取得する
        */
        Texture* GetTexture(uint32_t index);
        
        /**
        * @brief    マテリアルをロードする
        */
        void LoadMaterials(tinygltf::Model& input);


        void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer);
        
        /**
        * @brief    バッファ作成時のメモリフラグを指定する
        */
        void SetMemoryPropertyFlags(VkMemoryPropertyFlags memoryFlags);

        /**
        * @brief    ディスクリプタプールの作成
        */
        void CreateDescriptorPool();

        /**
        * @brief    ディスクリプタレイアウトの作成
        */
        void CreateDescriptorSetLayout();

        /**
        * @brief    ノードのディスクリプタレイアウトの作成
        */
        void CreateNodeDescriptorSets(Node* node);

        /**
        * @brief    ディスクリプタセットを作成する
        */
        void CreateDescriptorSets();

        /**
        * @brief    ディスクリプタ関連の作成
        */
        void CreateDescriptors();

        /**
        * @brief    glTFファイルを読み込む
        */
        void LoadFromFile(std::string filename, uint32_t fileLoadingFlags);

        /**
        * @brief    初期化
        */
        void Connect(VulkanDevice* device);

        /**
        * @brief    ノード描画
        */
        void DrawNode(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, Node* node, uint32_t renderFlags, uint32_t bindImageSet = 1);

        /**
        * @brief    モデル描画
        */
        void Draw(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, uint32_t renderFlags, uint32_t bindImageSet);

        /**
        * @brief    再構成
        */
        void Recreate();

        /**
        * @brief    スワップチェーン再作成時
        */
        void Cleanup();

        /**
        * @brief    破棄
        */
        void Destroy();

        /**
        * @brief    自インスタンスの取得
        */
        static Model* GetglTF();


        VkMemoryPropertyFlags memoryPropertyFlags;
        Initializers::Buffer _vertices;
        Initializers::Buffer _indices;

    private:
        std::vector<Node*> _nodes;
        std::vector<Node*> _linearNodes;
        std::vector<Texture> _textures;
        std::vector<Material> _materials;


        VulkanDevice* _vulkanDevice;
        uint32_t _mipLevel;
        VkDescriptorPool _descriptorPool;

    };

}

namespace {
    //シングルトン
    glTF::Model* s_model = nullptr;

}

/*******************************************************************************************************************
*                                             glTF Texture
********************************************************************************************************************/
void glTF::Texture::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (!(imageInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (vkCreateImage(vulkanDevice->_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(vulkanDevice->_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vulkanDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(vulkanDevice->_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(vulkanDevice->_device, image, imageMemory, 0);
}

void glTF::Texture::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = vulkanDevice->BeginCommand();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


    vulkanDevice->EndCommand(commandBuffer, queue);
}

void glTF::Texture::PrepareImage(void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight) {

    mipLevel = 1;

    Initializers::Buffer stagingBuffer;
    stagingBuffer = vulkanDevice->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(vulkanDevice->_device, stagingBuffer.memory, 0, VK_WHOLE_SIZE, 0, &data);
    memcpy(data, buffer, bufferSize);
    vkUnmapMemory(vulkanDevice->_device, stagingBuffer.memory);

    CreateImage(texWidth, texHeight, mipLevel, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);
    vulkanDevice->TransitionImageLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevel);
    CopyBufferToImage(stagingBuffer.buffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    vulkanDevice->TransitionImageLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevel);

    vkDestroyBuffer(vulkanDevice->_device, stagingBuffer.buffer, nullptr);
    vkFreeMemory(vulkanDevice->_device, stagingBuffer.memory, nullptr);
}

void glTF::Texture::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(vulkanDevice->_device, &viewInfo, nullptr, &view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void glTF::Texture::LoadglTFImages(tinygltf::Image& gltfImage) {

    unsigned char* buffer = nullptr;
    VkDeviceSize bufferSize = 0;
    bool deleteBuffer = false;

    //RGBの場合、RGBAにしておく
    if (gltfImage.component == 3) {
        bufferSize = gltfImage.width * gltfImage.height * 4;
        buffer = new unsigned char[bufferSize];
        unsigned char* rgba = buffer;
        unsigned char* rgb = &gltfImage.image[0];
        for (size_t i = 0; i < gltfImage.width * gltfImage.height; i++) {
            memcpy(rgba, rgb, sizeof(unsigned char) * 3);
            rgba += 4;
            rgb += 3;
        }
        deleteBuffer = true;
    }
    else {
        buffer = &gltfImage.image[0];
        bufferSize = gltfImage.image.size();
    }


    PrepareImage(buffer, bufferSize, IMAGEFORMAT, gltfImage.width, gltfImage.height);
    CreateImageView(image, IMAGEFORMAT, VK_IMAGE_ASPECT_COLOR_BIT, mipLevel);
    sampler = vulkanDevice->CreateSampler();

    if (deleteBuffer) {
        delete[] buffer;
    }
}

void glTF::Texture::Connect(VulkanDevice* device, VkQueue transQueue) {

    vulkanDevice = device;
    queue = transQueue;
}

void glTF::Texture::Destroy() {
    vkDestroySampler(vulkanDevice->_device, sampler, nullptr);
    vkDestroyImageView(vulkanDevice->_device, view, nullptr);
    vkDestroyImage(vulkanDevice->_device, image, nullptr);
    vkFreeMemory(vulkanDevice->_device, memory, nullptr);
}

/*******************************************************************************************************************
*                                             glTF Material
********************************************************************************************************************/
void glTF::Material::createDescriptorSet(VulkanDevice* vulkanDevice, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout) {

    VkDescriptorSetAllocateInfo allocInfoMaterial{};
    allocInfoMaterial.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfoMaterial.descriptorPool = descriptorPool;
    allocInfoMaterial.descriptorSetCount = 1;
    allocInfoMaterial.pSetLayouts = &descriptorSetLayout;

    if (vkAllocateDescriptorSets(vulkanDevice->_device, &allocInfoMaterial, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    
    std::vector<VkDescriptorImageInfo> imageInfos{};
    std::vector<VkWriteDescriptorSet> writeDescriptorSets{};
    
    if (descriptorBidningFlags & DescriptorBindingFlags::ImageBaseColor) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = baseColorTexture->view;
        imageInfo.sampler = baseColorTexture->sampler;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.pImageInfo = &imageInfo;
        writeDescriptorSet.descriptorCount = 1;

        imageInfos.push_back(imageInfo);
        writeDescriptorSets.push_back(writeDescriptorSet);
    }
    if (normalTexture && descriptorBidningFlags & DescriptorBindingFlags::ImageNormalMap) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = normalTexture->view;
        imageInfo.sampler = normalTexture->sampler;

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.pImageInfo = &imageInfo;
        writeDescriptorSet.descriptorCount = 1;

        imageInfos.push_back(imageInfo);
        writeDescriptorSets.push_back(writeDescriptorSet);
    }
    
    vkUpdateDescriptorSets(vulkanDevice->_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

/*******************************************************************************************************************
*                                             glTF Primitive
********************************************************************************************************************/
glTF::Primitive::Primitive(uint32_t firstIndex, uint32_t indexCount, Material& material):
    firstIndex(firstIndex),
    indexCount(indexCount),
    material(material)
{
}

/*******************************************************************************************************************
*                                             glTF Mesh
********************************************************************************************************************/
glTF::Mesh::Mesh(VulkanDevice* device, glm::mat4 matrix) {
    vulkanDevice = device;
    uniformBlock.matrix = matrix;
    vulkanDevice->CreateBuffer(
        sizeof(uniformBlock),
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uniformBuffer.buffer, uniformBuffer.memory
    );
    vkMapMemory(vulkanDevice->_device, uniformBuffer.memory, 0, sizeof(uniformBlock), 0, &uniformBuffer.mapped);
}

glTF::Mesh::~Mesh() {
    vkDestroyBuffer(vulkanDevice->_device, uniformBuffer.buffer, nullptr);
    vkFreeMemory(vulkanDevice->_device, uniformBuffer.memory, nullptr);
}

/*******************************************************************************************************************
*                                             glTF Node
********************************************************************************************************************/

glm::mat4 glTF::Node::localMatrix()
{
    //multiply SRT order
    return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
}

glm::mat4 glTF::Node::getMatrix() {
    glm::mat4 m = localMatrix();
    Node* p = parent;
    while (p) {
        m = p->localMatrix() * m;
        p = p->parent;
    }
    return m;
}

void glTF::Node::UpdateUniformBuffer(glm::mat4 projection, glm::mat4 view) {

    if (mesh) {
        glm::mat4 m = getMatrix();
        //update skinning
        memcpy(mesh->uniformBuffer.mapped, &m, VK_WHOLE_SIZE);
    }
}

glTF::Node::~Node() {
    if (mesh) {
        delete mesh;
    }
    for (auto& child : children) {
        delete child;
    }
}

/*******************************************************************************************************************
*                                             glTF Model
********************************************************************************************************************/
glTF::Model::Model() {

}

void glTF::Model::LoadImages(tinygltf::Model& input) {
    for (tinygltf::Image& image : input.images) {
        Texture texture;
        texture.Connect(_vulkanDevice, _vulkanDevice->_queue);
        texture.LoadglTFImages(image);
        _textures.push_back(texture);
    }
}

glTF::Texture* glTF::Model::GetTexture(uint32_t index) {
    if (index < _textures.size()) {
        return &_textures[index];
    }
    return nullptr;
}

void glTF::Model::LoadMaterials(tinygltf::Model& input) {

    for (tinygltf::Material& mat : input.materials) {
        Material material;
        if (mat.values.find("baseColorTexture") != mat.values.end()) {
            material.baseColorTexture = GetTexture(input.textures[mat.values["baseColorTexture"].TextureIndex()].source);
        }
        if (mat.values.find("metallicRoughnessTexture") != mat.values.end()) {
            material.metallicRoughnessTexture = GetTexture(input.textures[mat.values["metallicRoughnessTexture"].TextureIndex()].source);
        }
        if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
            material.normalTexture = GetTexture(input.textures[mat.additionalValues["normalTexture"].TextureIndex()].source);
        }
        if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
            material.occlusionTexture = GetTexture(input.textures[mat.additionalValues["occlusionTexture"].TextureIndex()].source);
        }
        if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
            material.emissiveTexture = GetTexture(input.textures[mat.additionalValues["emissiveTexture"].TextureIndex()].source);
        }

        if (mat.values.find("baseColorFactor") != mat.values.end()) {
            material.baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
        }
        if (mat.values.find("roughnessFactor") != mat.values.end()) {
            material.roughnessFactor = static_cast<float>(mat.values["roughnessFactor"].Factor());
        }
        if (mat.values.find("metallicFactor") != mat.values.end()) {
            material.metallicFactor = static_cast<float>(mat.values["metallicFactor"].Factor());
        }
        if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
            tinygltf::Parameter param = mat.additionalValues["alphaMode"];
            if (param.string_value == "BLEND") {
                material.alphaMode = Material::ALPHA_BLEND;
            }
            if (param.string_value == "MASK") {
                material.alphaMode = Material::ALPHA_MASK;
            }
        }
        if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
            material.alphaCutoff = static_cast<float>(mat.additionalValues["alphaCotoff"].Factor());
        }

        _materials.push_back(material);
    }

}

void glTF::Model::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer) {
    Node* newNode = new Node{};
    newNode->parent = parent;
    newNode->matrix = glm::mat4(1.0f);
    newNode->name = newNode->name;

    //4×4 matrix
    if (inputNode.matrix.size() == 16) {
        newNode->matrix = glm::make_mat4x4(inputNode.matrix.data());
    }
    if (inputNode.scale.size() == 3) {
        newNode->scale = glm::make_vec3(inputNode.scale.data());
    }
    if (inputNode.rotation.size() == 4) {
        newNode->rotation = glm::make_quat(inputNode.rotation.data());
    }
    if (inputNode.translation.size() == 3) {
        newNode->translation = glm::make_vec3(inputNode.translation.data());
    }

    if (inputNode.children.size() > 0) {
        for (size_t i = 0; i < inputNode.children.size(); i++) {
            LoadNode(input.nodes[inputNode.children[i]], input, newNode, indexBuffer, vertexBuffer);
        }
    }

    if (inputNode.mesh > -1) {
        const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
        Mesh* newMesh = new Mesh(_vulkanDevice, newNode->matrix);
        newMesh->name = mesh.name;

        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            const tinygltf::Primitive& primitive = mesh.primitives[i];

            if (primitive.indices < 0) {
                continue;
            }
            uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexStart = static_cast<uint32_t> (indexBuffer.size());
            uint32_t indexCount = 0;
            uint32_t vertexCount = 0;
            
            //vertices
            {    
                const float* positionBuffer = nullptr;
                const float* normalsBuffer = nullptr;
                const float* texCoordsBuffer = nullptr;
                const float* colorBuffer = nullptr;
                uint32_t numColorComponent;

                if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[primitive.attributes.find("POSITION")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    positionBuffer = reinterpret_cast<const float*> (&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    vertexCount = static_cast<uint32_t>(accessor.count);
                }

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[primitive.attributes.find("NORMAL")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    texCoordsBuffer = reinterpret_cast<const float*> (&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                }

                if (primitive.attributes.find("COLOR_0") != primitive.attributes.end()) {
                    const tinygltf::Accessor& accessor = input.accessors[primitive.attributes.find("COLOR_0")->second];
                    const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                    colorBuffer = reinterpret_cast<const float*> (&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                    numColorComponent = accessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
                }

                for (size_t v = 0; v < vertexCount; v++) {
                    Vertex vert{};
                    vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                    vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                    vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                    if (colorBuffer) {
                        switch (numColorComponent) {
                        case 3:
                            vert.color = glm::vec4(glm::make_vec3(&colorBuffer[v * 3]), 1.0f);
                        case 4:
                            vert.color = glm::make_vec4(&colorBuffer[v * 4]);
                        }
                    }
                    else {
                        vert.color = glm::vec4(1.0f);
                    }
                    vertexBuffer.push_back(vert);
                }
            }
            

            //indices
            {
                const tinygltf::Accessor& accessor = input.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

                indexCount = static_cast<uint32_t>(accessor.count);

                switch (accessor.componentType) {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    uint32_t* buf = new uint32_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    uint16_t* buf = new uint16_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    uint8_t* buf = new uint8_t[accessor.count];
                    memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
                    for (size_t index = 0; index < accessor.count; index++) {
                        indexBuffer.push_back(buf[index] + vertexStart);
                    }
                    break;
                }
                default:
                    std::cerr << "Index conponent type" << accessor.componentType << "not supported!" << std::endl;
                    return;
                }
            }
            
            
            Primitive* newPrimitive = new Primitive(indexStart, indexCount, primitive.material > -1 ? _materials[primitive.material] : _materials.back());
            newPrimitive->firstVertex = vertexStart;
            newPrimitive->vertexCount = vertexCount;
            newPrimitive->firstIndex = indexStart;
            newPrimitive->indexCount = indexCount;
            newMesh->primitives.push_back(newPrimitive);
        }
        newNode->mesh = newMesh;
    }
    if (parent) {
        parent->children.push_back(newNode);
    }
    else {
        _nodes.push_back(newNode);
    }
    _linearNodes.push_back(newNode);
}

void glTF::Model::SetMemoryPropertyFlags(VkMemoryPropertyFlags memoryFlags) {
    memoryPropertyFlags = memoryFlags;
}

void glTF::Model::CreateDescriptorPool() {

    uint32_t imageCount = 0;
    uint32_t uboCount = 0;

    for (auto node : _linearNodes) {
        if (node->mesh) {
            uboCount++;
        }
    }

    for (auto material : _materials) {
        if (material.baseColorTexture != nullptr) {
            imageCount++;
        }
    }

    std::vector<VkDescriptorPoolSize> poolSizes{};
    poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , uboCount });

    if (imageCount > 0) {
        if (descriptorBidningFlags & DescriptorBindingFlags::ImageBaseColor) {
            poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , imageCount });
        }
        if (descriptorBidningFlags & DescriptorBindingFlags::ImageNormalMap) {
            poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , imageCount });
        }
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = imageCount + uboCount;

    if (vkCreateDescriptorPool(_vulkanDevice->_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void glTF::Model::CreateDescriptorSetLayout() {

    //node uniform buffer
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo uboLayoutInfo{};
    uboLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    uboLayoutInfo.bindingCount = 1;
    uboLayoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(_vulkanDevice->_device, &uboLayoutInfo, nullptr, &_descriptorSetLayout.ubo) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    //material
    std::vector<VkDescriptorSetLayoutBinding>samplerBindings{};
    if (descriptorBidningFlags & DescriptorBindingFlags::ImageBaseColor) {
        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding.binding = static_cast<uint32_t>(samplerBindings.size());
        samplerBinding.descriptorCount = 1;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.pImmutableSamplers = nullptr;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerBindings.push_back(samplerBinding);
    }
    if (descriptorBidningFlags & DescriptorBindingFlags::ImageNormalMap) {
        VkDescriptorSetLayoutBinding samplerBinding{};
        samplerBinding.binding = static_cast<uint32_t>(samplerBindings.size());
        samplerBinding.descriptorCount = 1;
        samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerBinding.pImmutableSamplers = nullptr;
        samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerBindings.push_back(samplerBinding);
    }

    VkDescriptorSetLayoutCreateInfo imageLayoutInfo{};
    imageLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    imageLayoutInfo.bindingCount = static_cast<uint32_t>(samplerBindings.size());
    imageLayoutInfo.pBindings = samplerBindings.data();

    if (vkCreateDescriptorSetLayout(_vulkanDevice->_device, &imageLayoutInfo, nullptr, &_descriptorSetLayout.image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void glTF::Model::CreateNodeDescriptorSets(glTF::Node* node) {

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_descriptorSetLayout.ubo;
    if (vkAllocateDescriptorSets(_vulkanDevice->_device, &allocInfo, &node->mesh->uniformBuffer.descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkWriteDescriptorSet descriptorWrite{};
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = node->mesh->uniformBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = node->mesh->uniformBuffer.descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(_vulkanDevice->_device, 1, &descriptorWrite, 0, nullptr);
    
    for (auto& child : node->children) {
        CreateNodeDescriptorSets(child);
    }
}

void glTF::Model::CreateDescriptorSets() {
    
    for (auto node : _nodes) {
        CreateNodeDescriptorSets(node);
    }
    
    //material
    for (auto& material : _materials) {
        if (material.baseColorTexture != nullptr) {
            material.createDescriptorSet(_vulkanDevice, _descriptorPool, _descriptorSetLayout.image);
        }
    }
}

void glTF::Model::CreateDescriptors() {
    CreateDescriptorPool();
    CreateDescriptorSetLayout();
    CreateDescriptorSets();
}

void glTF::Model::LoadFromFile(std::string filename, uint32_t fileLoadingFlags) {

    tinygltf::Model glTFInput;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex> vertexBuffer;

    if (fileLoaded) {
        LoadImages(glTFInput);
        LoadMaterials(glTFInput);

        const tinygltf::Scene& scene = glTFInput.scenes[0];
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
            LoadNode(node, glTFInput, nullptr, indexBuffer, vertexBuffer);
        }
        if (glTFInput.animations.size() > 0) {
            //ここでアニメーションを読み込む
        }
    }
    else {
        throw std::runtime_error("failed to find glTF file!");
    }

    
    if (fileLoadingFlags & FileLoadingFlags::PreTransformVertices || fileLoadingFlags & FileLoadingFlags::PreMultiplyVertexColors || fileLoadingFlags & FileLoadingFlags::FlipY) {
        for (Node* node : _linearNodes) {
            if (node->mesh) {
                const glm::mat4 localMatrix = node->getMatrix();
                for (Primitive* primitive : node->mesh->primitives) {
                    for (uint32_t i = 0; i < primitive->vertexCount; i++) {
                        Vertex& vertex = vertexBuffer[primitive->firstVertex + i];

                        if (fileLoadingFlags & FileLoadingFlags::PreTransformVertices) {
                            vertex.pos = glm::vec3(localMatrix * glm::vec4(vertex.pos, 1.0f));
                            vertex.normal = glm::normalize(glm::mat3(localMatrix) * vertex.normal);
                        }
                        if (fileLoadingFlags & FileLoadingFlags::FlipY) {
                            vertex.pos.y *= -1.0f;
                            vertex.normal.y *= -1.0f;
                        }
                        if (fileLoadingFlags & FileLoadingFlags::PreMultiplyVertexColors) {
                            vertex.color = primitive->material.baseColorFactor * vertex.color;
                        }
                    }
                }
            }
        }
    }



    //vertex buffer
    VkDeviceSize vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);
    _vertices.count = static_cast<uint32_t>(vertexBuffer.size());

    Initializers::Buffer vertexStaging;
    vertexStaging = _vulkanDevice->CreateBuffer(
        vertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    void* data;
    vkMapMemory(_vulkanDevice->_device, vertexStaging.memory, 0, VK_WHOLE_SIZE, 0, &data);
    memcpy(data, vertexBuffer.data(), (size_t)vertexBufferSize);
    vkUnmapMemory(_vulkanDevice->_device, vertexStaging.memory);

    _vertices = _vulkanDevice->CreateBuffer(
        vertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | memoryPropertyFlags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    _vulkanDevice->CopyBuffer(vertexStaging.buffer, _vertices.buffer, vertexBufferSize);

    vkDestroyBuffer(_vulkanDevice->_device, vertexStaging.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, vertexStaging.memory, nullptr);

    //index buffer
    VkDeviceSize indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    _indices.count = static_cast<uint32_t>(indexBuffer.size());

    Initializers::Buffer indexStaging;
    indexStaging = _vulkanDevice->CreateBuffer(
        indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    vkMapMemory(_vulkanDevice->_device, indexStaging.memory, 0, VK_WHOLE_SIZE, 0, &data);
    memcpy(data, indexBuffer.data(), (size_t)indexBufferSize);
    vkUnmapMemory(_vulkanDevice->_device, indexStaging.memory);

    _indices = _vulkanDevice->CreateBuffer(
        indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | memoryPropertyFlags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    _vulkanDevice->CopyBuffer(indexStaging.buffer, _indices.buffer, indexBufferSize);

    vkDestroyBuffer(_vulkanDevice->_device, indexStaging.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, indexStaging.memory, nullptr);

    CreateDescriptors();
}

void glTF::Model::Connect(VulkanDevice* device) {
    _vulkanDevice = device;
}

void glTF::Model::DrawNode(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, Node* node, uint32_t renderFlags, uint32_t bindImageSet)
{
    if (node->mesh) {
        for (Primitive* primitive : node->mesh->primitives) {
            const glTF::Material& material = primitive->material;
            bool skip = false;
            if (renderFlags & RenderFlags::RenderOpaqueNodes) {
                skip = (material.alphaMode != Material::ALPHA_OPAQUE);
            }
            if (renderFlags & RenderFlags::RenderAlphaMaskedNodes) {
                skip = (material.alphaMode != Material::ALPHA_MASK);
            }
            if (renderFlags & RenderFlags::RenderAlphaBlendedNodes) {
                skip = (material.alphaMode != Material::ALPHA_BLEND);
            }
            if (!skip) {
                if (renderFlags & RenderFlags::BindImages) {
                    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, bindImageSet, 1, &material.descriptorSet, 0, nullptr);
                }
                vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
            }
        }
    }

    for (auto& child : node->children) {
        DrawNode(commandBuffer, pipelineLayout, child, renderFlags, bindImageSet);
    }
}

void glTF::Model::Draw(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, uint32_t renderFlags, uint32_t bindImageSet) {
    //モデル描画
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_vertices.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, _indices.buffer, 0, VK_INDEX_TYPE_UINT32);

    for (auto& node : _nodes) {
        DrawNode(commandBuffer, pipelineLayout, node, renderFlags, bindImageSet);
    }
}

void glTF::Model::Recreate() {
}

void glTF::Model::Cleanup() {

}

void glTF::Model::Destroy() {
    
    vkDestroyBuffer(_vulkanDevice->_device, _vertices.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, _vertices.memory, nullptr);
    vkDestroyBuffer(_vulkanDevice->_device, _indices.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, _indices.memory, nullptr);

    for (auto texture : _textures) {
        texture.Destroy();
    }
    for (auto node : _nodes) {
        delete node;
    }
    vkDestroyDescriptorSetLayout(_vulkanDevice->_device, _descriptorSetLayout.ubo, nullptr);
    vkDestroyDescriptorSetLayout(_vulkanDevice->_device, _descriptorSetLayout.image, nullptr);

    vkDestroyDescriptorPool(_vulkanDevice->_device, _descriptorPool, nullptr);

}

glTF::Model* glTF::Model::GetglTF() {
    if (s_model == nullptr) {
        s_model = new Model();
    }
    return s_model;
}

/*******************************************************************************************************************
*                                             AppBase
********************************************************************************************************************/
AppBase::AppBase() :
    _framebufferResized(false)
{

}

/*******************************************************************************************************************
*                                             コールバック
********************************************************************************************************************/

//フレームバッファのサイズ変更時
static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {

    auto app = reinterpret_cast<AppBase*>(glfwGetWindowUserPointer(window));
    app->_framebufferResized = true;
}

//マウス入力時の処理
static void mouseButton(GLFWwindow* window, int button, int action, int modsy) {

    AppBase* instance = static_cast<AppBase*>(glfwGetWindowUserPointer(window));

    if (instance != NULL) {

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            //とりあえず左だけ
            instance->_mouseButtons.left = true;
        }
        else if (action == GLFW_RELEASE) {
            instance->_mouseButtons.left = false;
            instance->_mouseButtons.middle = false;
            instance->_mouseButtons.right = false;
        }
    }
}

void AppBase::MouseMove(double x, double y) {
    double dx = _mousePos.x - x;
    double dy = _mousePos.y - y;
    bool handled = false;

    if (_mouseButtons.left) {
        _camera->rotate(glm::vec3(dy * _camera->rotationSpeed, -dx * _camera->rotationSpeed, 0.0f));
        viewUpdated = true;
    }
    if (_mouseButtons.right) {
        _camera->translate(glm::vec3(-0.0f, 0.0f, dy * 0.05f));
        viewUpdated = true;
    }
    if (_mouseButtons.middle) {
        _camera->translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
        viewUpdated = true;
    }
    //マウス位置保存
    _mousePos = glm::vec2((float)x, (float)y);
}

//カーソル入力
static void cursor(GLFWwindow* window, double xpos, double ypos) {

    AppBase* const instance(static_cast<AppBase*>(glfwGetWindowUserPointer(window)));
    if (instance != NULL) {
        //ワールド座標系に対するデバイス座標系の拡大率を更新する
        instance->MouseMove(xpos, ypos);
    }
}

//マウスホイール操作時の処理
static void wheel(GLFWwindow* window, double x, double y) {

    AppBase* instance = static_cast<AppBase*>(glfwGetWindowUserPointer(window));

    if (instance != NULL) {
        //ワールド座標系に対するデバイス座標系の拡大率を更新する
        instance->_camera->translate(glm::vec3(0.0f, 0.0f, (float)y * 0.05f));
    }
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

    std::string prefix("");

    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        prefix = "VERBOSE";
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        prefix = "INFO";
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        prefix = "WARNING";
    }
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        prefix = "ERROR";
    }

    std::cerr << "validation layer: " << prefix << "[" << pCallbackData->messageIdNumber << "][" << pCallbackData->pMessageIdName << "]:" << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}


/*******************************************************************************************************************
*                                             初期化
********************************************************************************************************************/

void AppBase::InitializeWindow() {
    if (glfwInit() == GL_FALSE) {
        throw std::runtime_error("failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _window = glfwCreateWindow(WIDTH, HEIGHT, "VulkanRaytracing", NULL, NULL);
    
}

void AppBase::SetupGlfwCallbacks() {

    //コールバック関数の登録
    glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);

    glfwSetMouseButtonCallback(_window, mouseButton);

    //マウス入力
    glfwSetCursorPosCallback(_window, cursor);

    //マウスホイール操作時に呼び出す処理の登録
    glfwSetScrollCallback(_window, wheel);

    glfwSetWindowUserPointer(_window, this);

}

bool AppBase::CheckValidationLayerSupport() {

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> AppBase::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void AppBase::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void AppBase::CreateInstance() {

    //検証レイヤーが有効のときに使えるか確認
    if (enableValidationLayers && !CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.pEngineName = "Vulkan";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void AppBase::SetupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void AppBase::PickupPhysicalDevice() {

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    for (VkPhysicalDevice device : devices) {
        
        if (_vulkanDevice->IsDeviceSuitable(device)) {
            _physicalDevice = device;
            break;
        }
    }

    if (_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void AppBase::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = SWAPCHAINCOLORFORMAT;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = _vulkanDevice->FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();
    
    if (vkCreateRenderPass(_vulkanDevice->_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void AppBase::CreateGraphicsPipeline() {
      
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.push_back(_shader->LoadShaderProgram("Shaders/mesh.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
    shaderStages.push_back(_shader->LoadShaderProgram("Shaders/mesh.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = glTF::Vertex::getBindingDescription();
    auto attributeDescriptions = glTF::Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_swapchain->_extent.width;
    viewport.height = (float)_swapchain->_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = _swapchain->_extent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.size = sizeof(glm::mat4);
    pushConstantRange.offset = 0;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    
    std::array<VkDescriptorSetLayout, 2> bindings = { glTF::_descriptorSetLayout.ubo, glTF::_descriptorSetLayout.image };

    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t> (bindings.size());
    pipelineLayoutInfo.pSetLayouts = bindings.data();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(_vulkanDevice->_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = r_pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(_vulkanDevice->_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

void AppBase::InitializeGUI() {
    
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(_vulkanDevice->_device, &pool_info, nullptr, &_descriptorPool);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(_window, true);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = _instance;
    initInfo.PhysicalDevice = _physicalDevice;
    initInfo.Device = _vulkanDevice->_device;
    initInfo.QueueFamily = _vulkanDevice->FindQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
    initInfo.Queue = _vulkanDevice->_queue;
    initInfo.DescriptorPool = _descriptorPool;
    initInfo.MinImageCount = _swapchain->_imageCount;
    initInfo.ImageCount = _swapchain->_imageCount;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo, _renderPass);

    //upload GUI font texture
    VkCommandBuffer commandBuffer = _vulkanDevice->BeginCommand();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    _vulkanDevice->EndCommand(commandBuffer, _vulkanDevice->_queue);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void AppBase::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (!(imageInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (vkCreateImage(_vulkanDevice->_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_vulkanDevice->_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _vulkanDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_vulkanDevice->_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_vulkanDevice->_device, image, imageMemory, 0);
}

VkImageView AppBase::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(_vulkanDevice->_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void AppBase::CreateDepthResources() {

    VkFormat depthFormat = _vulkanDevice->FindDepthFormat();
    CreateImage(_swapchain->_extent.width, _swapchain->_extent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
    if (depthFormat > VK_FORMAT_D16_UNORM_S8_UINT) {
        _depthImageView = CreateImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_STENCIL_BIT, 1);
    }
    else {
        _depthImageView = CreateImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }

}

void AppBase::CreateFramebuffers() {
    
    _frameBuffers.resize(_swapchain->_imageCount);

    for (size_t i = 0; i < _swapchain->_imageCount; i++) {
        
        std::array<VkImageView, 2> attachments = {
            _swapchain->_swapchainBuffers[i].imageview,
            _depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _swapchain->_extent.width;
        framebufferInfo.height = _swapchain->_extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_vulkanDevice->_device, &framebufferInfo, nullptr, &_frameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void AppBase::CreateCommandBuffers() {
    _commandBuffers.resize(_swapchain->_imageCount);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _vulkanDevice->_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

    if (vkAllocateCommandBuffers(_vulkanDevice->_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void AppBase::BuildCommandBuffers(bool renderImgui) {

    //コマンドバッファへの記録の開始
    for (size_t i = 0; i < _commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        //レイトレーシングを行う
        vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, r_pipeline);
        vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, r_pipelineLayout, 0, 1, &r_descriptorSet, 0, nullptr);
        VkStridedDeviceAddressRegionKHR callableSbtEntry{};
        vkCmdTraceRaysKHR(
            _commandBuffers[i],
            &raygenRegion, &missRegion, &hitRegion, &callableSbtEntry,
            WIDTH, HEIGHT, 1
        );

        //レイトレーシングの結果をバックバッファにコピーする

        //スワップチェーンイメージを転送dstに設定
        _vulkanDevice->SetImageRayout(
            _commandBuffers[i],
            _swapchain->_swapchainBuffers[i].image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1
        );

        //レイトレーシング結果イメージを転送srcに設定
        _vulkanDevice->SetImageRayout(
            _commandBuffers[i],
            r_strageImage.image,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            1
        );

        VkImageCopy copyRegion{};
        copyRegion.extent = { WIDTH,HEIGHT,1 };
        copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT,0,0,1 };
        copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT,0,0,1 };

        vkCmdCopyImage(_commandBuffers[i], r_strageImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _swapchain->_swapchainBuffers[i].image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

        //レイアウトを元に戻す

        _vulkanDevice->SetImageRayout(
            _commandBuffers[i],
            _swapchain->_swapchainBuffers[i].image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            1
        );

        _vulkanDevice->SetImageRayout(
            _commandBuffers[i],
            r_strageImage.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            1
        );

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _renderPass;
        renderPassInfo.framebuffer = _frameBuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = _swapchain->_extent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        if (renderImgui) {
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _commandBuffers[i]);
        }
        
        //s_model->Draw(_commandBuffers[i], _pipelineLayout);

        vkCmdEndRenderPass(_commandBuffers[i]);

        if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void AppBase::CreateSyncObjects() {

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(_vulkanDevice->_device, &semaphoreInfo, nullptr, &_presentCompleteSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(_vulkanDevice->_device, &semaphoreInfo, nullptr, &_renderCompleteSemaphore) != VK_SUCCESS ||
        vkCreateFence(_vulkanDevice->_device, &fenceInfo, nullptr, &_renderFences) != VK_SUCCESS) {
        throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
    
}

void AppBase::Initialize() {

    InitializeWindow();
    SetupGlfwCallbacks();
    CreateInstance();
    SetupDebugMessenger();
    PickupPhysicalDevice();

    _vulkanDevice = new VulkanDevice();
    _vulkanDevice->Connect(_physicalDevice);
    _vulkanDevice->CreateLogicalDevice();

    load_VK_EXTENSIONS(
        _instance,
        vkGetInstanceProcAddr,
        _vulkanDevice->_device,
        vkGetDeviceProcAddr
    );

    _swapchain = new Swapchain();
    _swapchain->Connect(_window, _instance, _physicalDevice, _vulkanDevice->_device);
    _swapchain->CreateSurface();
    _swapchain->CreateSwapChain(_vulkanDevice->FindQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT));

    CreateRenderPass();
    _vulkanDevice->CreateCommandPool();
    
    _camera = new Camera();
    _camera->type = Camera::CameraType::firstperson;
    _camera->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
    _camera->setPerspective(60.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 512.0f);
    _camera->setTranslation(glm::vec3(0.0f, 0.5f, -2.0f));
    


    InitRayTracing();

    InitializeGUI();
    CreateDepthResources();
    CreateFramebuffers();
    CreateCommandBuffers();
    BuildCommandBuffers(false);
    CreateSyncObjects();
}

/*******************************************************************************************************************
*                                             レイトレーシング
********************************************************************************************************************/

void AccelerationStructure::CreateAccelerationStructureBuffer(VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo) {
    
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkCreateBuffer(vulkanDevice->_device, &bufferCreateInfo, nullptr, &buffer);

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(vulkanDevice->_device, buffer, &memoryRequirements);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;

    vkAllocateMemory(vulkanDevice->_device, &memoryAllocateInfo, nullptr, &memory);
    vkBindBufferMemory(vulkanDevice->_device, buffer, memory, 0);

    VkAccelerationStructureCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = buffer;
    createInfo.size = buildSizeInfo.accelerationStructureSize;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    vkCreateAccelerationStructureKHR(vulkanDevice->_device, &createInfo, nullptr, &handle);
}

void AppBase::PolygonMesh::Connect(Initializers::Buffer& vertBuffer, Initializers::Buffer& idxBuffer) {
    vertexBuffer = vertBuffer;
    indexBuffer = idxBuffer;
}

void AppBase::PolygonMesh::BuildBLAS(VulkanDevice* vulkanDevice) {

    VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
    VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
    vertexBufferDeviceAddress.deviceAddress = vertexBuffer.GetBufferDeviceAddress(vulkanDevice->_device);
    indexBufferDeviceAddress.deviceAddress = indexBuffer.GetBufferDeviceAddress(vulkanDevice->_device);

    uint32_t numTriangles = static_cast<uint32_t>(indexBuffer.count) / 3;
    uint32_t maxVertex = vertexBuffer.count;

    VkAccelerationStructureGeometryKHR geometryInfo{};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometryInfo.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    geometryInfo.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometryInfo.geometry.triangles.vertexData = vertexBufferDeviceAddress;
    geometryInfo.geometry.triangles.maxVertex = maxVertex;
    geometryInfo.geometry.triangles.vertexStride = sizeof(glTF::Vertex);
    geometryInfo.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    geometryInfo.geometry.triangles.indexData = indexBufferDeviceAddress;
    geometryInfo.geometry.triangles.transformData.deviceAddress = 0;
    geometryInfo.geometry.triangles.transformData.hostAddress = nullptr;

    //get sizeInfo
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &geometryInfo;

    VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo;
    buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    vkGetAccelerationStructureBuildSizesKHR(
        vulkanDevice->_device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildGeometryInfo,
        &numTriangles,
        &buildSizesInfo
    );

    blas.CreateAccelerationStructureBuffer(buildSizesInfo);

    //create scratch buffer
    Initializers::Buffer scratchBuffer = vulkanDevice->CreateBuffer(
        buildSizesInfo.buildScratchSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    //VkAccelerationStructureBuildGeometryInfoKHR other parameter
    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.dstAccelerationStructure = blas.handle;
    buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.GetBufferDeviceAddress(vulkanDevice->_device);

    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo.primitiveCount = numTriangles;
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.transformOffset = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos = { &buildRangeInfo };
    
    //build
    VkCommandBuffer commandBuffer = vulkanDevice->BeginCommand();
    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, buildRangeInfos.data());

    //memory barrier
    VkBufferMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    memoryBarrier.buffer = blas.buffer;
    memoryBarrier.size = VK_WHOLE_SIZE;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        0, 0, nullptr, 1, &memoryBarrier, 0, nullptr
    );

    vulkanDevice->EndCommandAndWait(commandBuffer, vulkanDevice->_queue);

    VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    deviceAddressInfo.accelerationStructure = blas.handle;
    blas.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(vulkanDevice->_device, &deviceAddressInfo);

    vkDestroyBuffer(vulkanDevice->_device, scratchBuffer.buffer, nullptr);
    vkFreeMemory(vulkanDevice->_device, scratchBuffer.memory, nullptr);
}

AppBase::Image AppBase::CreateTextureCube(const wchar_t* imageFiles[6], VkImageUsageFlags usage, VkMemoryPropertyFlags memProps) {

    int width, height;
    stbi_uc* images[6] = { 0 };
    std::vector<char> binImages[6];

    for (uint32_t i = 0; i < 6; i++) {
        std::ifstream infile(imageFiles[i], std::ios::binary);
        if (infile) {
            throw std::runtime_error("failed to find images!");
        }
        binImages[i].resize(infile.seekg(0, std::ifstream::end).tellg());
        infile.seekg(0, std::ifstream::beg).read(binImages[i].data(), binImages[i].size());

        images[i] = stbi_load_from_memory(
            reinterpret_cast<uint8_t*>(binImages->data()),
            int(binImages->size()),
            &width,
            &height,
            nullptr,
            4
        );
    }

    //create image
    usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (!(imageInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    Image cubeMap;
    vkCreateImage(_vulkanDevice->_device, &imageInfo, nullptr, &cubeMap.image);

    //create imageview
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = cubeMap.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    viewInfo.format = imageInfo.format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(_vulkanDevice->_device, &viewInfo, nullptr, &cubeMap.view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    //staging buffer
    Initializers::Buffer buffers[6];
    auto imageSize = width * height * sizeof(uint32_t);
    for (uint32_t i = 0; i < 6; i++) {
        buffers[i] = _vulkanDevice->CreateBuffer(
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        void* data;
        vkMapMemory(_vulkanDevice->_device, buffers[i].memory, 0, VK_WHOLE_SIZE, 0, &data);
        memcpy(data, buffers[i].buffer, imageSize);
        vkUnmapMemory(_vulkanDevice->_device, buffers[i].memory);

    }

    _vulkanDevice->TransitionImageLayout(cubeMap.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1);
    
    //copy to image
    VkCommandBuffer commandBuffer = _vulkanDevice->BeginCommand();
    for (uint32_t i = 0; i < 6; i++) {

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            uint32_t(width),
            uint32_t(height),
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffers[i].buffer, cubeMap.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    }
    _vulkanDevice->EndCommand(commandBuffer, _vulkanDevice->_queue);
    _vulkanDevice->TransitionImageLayout(cubeMap.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

    for (auto buffer : buffers) {
        vkDestroyBuffer(_vulkanDevice->_device, buffer.buffer, nullptr);
        vkFreeMemory(_vulkanDevice->_device, buffer.memory, nullptr);
    }

    cubeMap.sampler = _vulkanDevice->CreateSampler();

    return cubeMap;
}

void AppBase::CreateBLAS() {

    //load gltf model
    glTF::Model::GetglTF();
    s_model->Connect(_vulkanDevice);
    s_model->SetMemoryPropertyFlags(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
    const uint32_t glTFLoadingFlags = glTF::FileLoadingFlags::PreTransformVertices | glTF::FileLoadingFlags::PreMultiplyVertexColors | glTF::FileLoadingFlags::FlipY;
    s_model->LoadFromFile("Assets/reflectionScene/reflectionScene.gltf", glTFLoadingFlags);
    
    r_gltfModel.vertexBuffer = s_model->_vertices;
    r_gltfModel.indexBuffer = s_model->_indices;
    r_gltfModel.BuildBLAS(_vulkanDevice);

    //ceiling
    std::vector <primitive::Vertex> vertices;
    std::vector<uint32_t> indices;
    primitive::GetCeiling(vertices, indices);
    uint32_t vertexSize = static_cast<uint32_t>(vertices.size() * sizeof(primitive::Vertex));
    uint32_t indexSize = static_cast<uint32_t>(indices.size() * sizeof(uint32_t));

    //vertex
    Initializers::Buffer vertexStaging;
    vertexStaging = _vulkanDevice->CreateBuffer(
        vertexSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    void* data;
    vkMapMemory(_vulkanDevice->_device, vertexStaging.memory, 0, VK_WHOLE_SIZE, 0, &data);
    memcpy(data, vertices.data(), vertexSize);
    vkUnmapMemory(_vulkanDevice->_device, vertexStaging.memory);

    r_ceiling.vertexBuffer = _vulkanDevice->CreateBuffer(
        vertexSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    _vulkanDevice->CopyBuffer(vertexStaging.buffer, r_ceiling.vertexBuffer.buffer, vertexSize);

    vkDestroyBuffer(_vulkanDevice->_device, vertexStaging.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, vertexStaging.memory, nullptr);

    //index
    Initializers::Buffer indexStaging;
    indexStaging = _vulkanDevice->CreateBuffer(
        indexSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    vkMapMemory(_vulkanDevice->_device, indexStaging.memory, 0, VK_WHOLE_SIZE, 0, &data);
    memcpy(data, indices.data(), indexSize);
    vkUnmapMemory(_vulkanDevice->_device, indexStaging.memory);

    r_ceiling.indexBuffer = _vulkanDevice->CreateBuffer(
        indexSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    _vulkanDevice->CopyBuffer(indexStaging.buffer, r_ceiling.indexBuffer.buffer, indexSize);

    vkDestroyBuffer(_vulkanDevice->_device, indexStaging.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, indexStaging.memory, nullptr);

    r_ceiling.BuildBLAS(_vulkanDevice);


    //background texture
    const wchar_t* textures[6] = {
        L"textures/posx.jpg", L"textures/negx.jpg",
        L"textures/posy.jpg", L"textures/negy.jpg",
        L"textures/posz.jpg", L"textures/negz.jpg"
    };

    r_cubeMap = CreateTextureCube(textures, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    


}

void AppBase::CreateTLAS() {
    VkTransformMatrixKHR transformMatrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f 
    };

    VkAccelerationStructureInstanceKHR instance{};
    instance.transform = transformMatrix;
    instance.instanceCustomIndex = 0;
    instance.mask = 0xFF;
    instance.instanceShaderBindingTableRecordOffset = 0;
    instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    instance.accelerationStructureReference = maingltf.deviceAddress;
    
    _vulkanDevice->CreateBuffer(
        sizeof(VkAccelerationStructureInstanceKHR),
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        r_instanceBuffer.buffer,
        r_instanceBuffer.memory
    );

    vkMapMemory(_vulkanDevice->_device, r_instanceBuffer.memory, 0, VK_WHOLE_SIZE, 0, &r_instanceBuffer.mapped);
    memcpy(r_instanceBuffer.mapped, &instance, sizeof(VkAccelerationStructureInstanceKHR));
    vkUnmapMemory(_vulkanDevice->_device, r_instanceBuffer.memory);

    //デバイスアドレスを取得
    VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
    instanceDataDeviceAddress.deviceAddress = GetBufferDeviceAddress(r_instanceBuffer.buffer);

    VkAccelerationStructureGeometryKHR geometryInfo{};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    geometryInfo.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometryInfo.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    geometryInfo.geometry.instances.arrayOfPointers = VK_FALSE;
    geometryInfo.geometry.instances.data = instanceDataDeviceAddress;

    //サイズ情報を取得する
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &geometryInfo;

    uint32_t primiticeCount = 1;
    VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo;
    buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    vkGetAccelerationStructureBuildSizesKHR(
        _vulkanDevice->_device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildGeometryInfo,
        &primiticeCount,
        &buildSizesInfo
    );

    CreateAccelerationStructureBuffer(r_topLevelAS, buildSizesInfo);

    //TRAS生成
    VkAccelerationStructureCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = r_topLevelAS.buffer;
    createInfo.size = buildSizesInfo.accelerationStructureSize;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    vkCreateAccelerationStructureKHR(_vulkanDevice->_device, &createInfo, nullptr, &r_topLevelAS.handle);
    
    //TLASの構築に必要なスクラッチ（作業）バッファの作成
    RayTracingScratchBuffer scratchBuffer = CreateScratchBuffer(buildSizesInfo.buildScratchSize);

    //VkAccelerationStructureBuildGeometryInfoKHRの他のパラメータ
    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.dstAccelerationStructure = r_topLevelAS.handle;
    buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

    //コマンドバッファにTLASの構築を登録する
    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo.primitiveCount = primiticeCount;
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.transformOffset = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos = { &buildRangeInfo };

    VkCommandBuffer commandBuffer = _vulkanDevice->BeginCommand();
    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, buildRangeInfos.data());

    //メモリバリア
    VkBufferMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    memoryBarrier.buffer = r_topLevelAS.buffer;
    memoryBarrier.size = VK_WHOLE_SIZE;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        0, 0, nullptr, 1, &memoryBarrier, 0, nullptr
    );

    _vulkanDevice->EndCommandAndWait(commandBuffer, _vulkanDevice->_queue);

    //Acceleration Structureのデバイスアドレスを取得
    VkAccelerationStructureDeviceAddressInfoKHR deviceAddressInfo{};
    deviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    deviceAddressInfo.accelerationStructure = r_topLevelAS.handle;
    r_topLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(_vulkanDevice->_device, &deviceAddressInfo);

    vkDestroyBuffer(_vulkanDevice->_device, scratchBuffer.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, scratchBuffer.memory, nullptr);
}

void AppBase::CreateStrageImage() {
    
    CreateImage(
        WIDTH, HEIGHT, 1, SWAPCHAINCOLORFORMAT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, r_strageImage.image, r_strageImage.memory
    );
    r_strageImage.view = CreateImageView(r_strageImage.image, SWAPCHAINCOLORFORMAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    
    _vulkanDevice->TransitionImageLayout(r_strageImage.image, VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL,1);

}

void AppBase::UpdateUniformBuffer() {
    _uniformData.projInverse = glm::inverse(_camera->matrix.perspective);
    _uniformData.viewInverse = glm::inverse(_camera->matrix.view);
    _uniformData.lightPos = glm::vec4(cos(glm::radians(timer * 360.0f)) * 40.0f, -50.0f + sin(glm::radians(timer * 360.0f)) * 20.0f, 25.0f + sin(glm::radians(timer * 360.0f)) * 5.0f, 0.0f);
    _uniformData.vertexSize = sizeof(glTF::Vertex);
    memcpy(r_ubo.mapped, &_uniformData, sizeof(_uniformData));
}

void AppBase::CreateUniformBuffer() {
    VkDeviceSize bufferSize = sizeof(_uniformData);
    _vulkanDevice->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, r_ubo.buffer, r_ubo.memory);

    vkMapMemory(_vulkanDevice->_device, r_ubo.memory, 0, VK_WHOLE_SIZE, 0, &r_ubo.mapped);
    memcpy(r_ubo.mapped, &r_ubo, sizeof(_uniformData));

    UpdateUniformBuffer();
}

void AppBase::CreateRaytracingLayout() {

    VkDescriptorSetLayoutBinding accelerationStructurelayoutBinding{};
    accelerationStructurelayoutBinding.binding = 0;
    accelerationStructurelayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    accelerationStructurelayoutBinding.descriptorCount = 1;
    accelerationStructurelayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

    VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
    resultImageLayoutBinding.binding = 1;
    resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    resultImageLayoutBinding.descriptorCount = 1;
    resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    //uniform buffer
    VkDescriptorSetLayoutBinding uniformBufferBinding{};
    uniformBufferBinding.binding = 2;
    uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferBinding.descriptorCount = 1;
    uniformBufferBinding.stageFlags = VK_SHADER_STAGE_ALL;

    //background
    VkDescriptorSetLayoutBinding backgroundBinding{};
    backgroundBinding.binding = 3;
    backgroundBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    backgroundBinding.descriptorCount = 1;
    backgroundBinding.stageFlags = VK_SHADER_STAGE_ALL;

    //primitiveMesh
    VkDescriptorSetLayoutBinding primitiveMeshBinding{};
    primitiveMeshBinding.binding = 4;
    primitiveMeshBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    primitiveMeshBinding.descriptorCount = 1;
    primitiveMeshBinding.stageFlags = VK_SHADER_STAGE_ALL;

    //material
    VkDescriptorSetLayoutBinding materialBinding{};
    materialBinding.binding = 5;
    materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    materialBinding.descriptorCount = 1;
    materialBinding.stageFlags = VK_SHADER_STAGE_ALL;

    //texture
    VkDescriptorSetLayoutBinding textureBinding{};
    textureBinding.binding = 6;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    textureBinding.descriptorCount = 1;
    textureBinding.stageFlags = VK_SHADER_STAGE_ALL;

    std::vector<VkDescriptorSetLayoutBinding> bindings({
        accelerationStructurelayoutBinding,
        resultImageLayoutBinding,
        uniformBufferBinding,
        backgroundBinding,
        primitiveMeshBinding,
        materialBinding,
        textureBinding
    });
    
    VkDescriptorSetLayoutCreateInfo descriptorSetCreateInfo{};
    descriptorSetCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetCreateInfo.pBindings = bindings.data();
    vkCreateDescriptorSetLayout(_vulkanDevice->_device, &descriptorSetCreateInfo, nullptr, &r_descriptorSetLayout);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &r_descriptorSetLayout;
    vkCreatePipelineLayout(_vulkanDevice->_device, &pipelineLayoutCreateInfo, nullptr, &r_pipelineLayout);
}

void AppBase::CreateRaytracingPipeline() {

    std::vector<VkPipelineShaderStageCreateInfo> stages;

    //raygen
    auto rgStage = _shader->LoadShaderProgram("Shaders/raytracingReflections/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    VkRayTracingShaderGroupCreateInfoKHR raygenShaderGroup{};
    raygenShaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    raygenShaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    raygenShaderGroup.generalShader = static_cast<uint32_t>(stages.size());
    raygenShaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    raygenShaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    raygenShaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    r_shaderGroups.push_back(raygenShaderGroup);
    stages.push_back(rgStage);
    
    VkSpecializationMapEntry specializationMapEntry{};
    specializationMapEntry.constantID = 0;
    specializationMapEntry.offset = 0;
    specializationMapEntry.size = sizeof(uint32_t);

    uint32_t maxRecursion = 4;
    VkSpecializationInfo specializationInfo{};
    specializationInfo.mapEntryCount = 1;
    specializationInfo.pMapEntries = &specializationMapEntry;
    specializationInfo.dataSize = sizeof(maxRecursion);
    specializationInfo.pData = &maxRecursion;

    stages.back().pSpecializationInfo = &specializationInfo;

    //miss
    auto missStage = _shader->LoadShaderProgram("Shaders/raytracingReflections/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR);
    VkRayTracingShaderGroupCreateInfoKHR missShaderGroup{};
    missShaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    missShaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    missShaderGroup.generalShader = static_cast<uint32_t>(stages.size());
    missShaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
    missShaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    missShaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    r_shaderGroups.push_back(missShaderGroup);
    stages.push_back(missStage);
    

    //closest hit
    auto chStage = _shader->LoadShaderProgram("Shaders/raytracingReflections/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    VkRayTracingShaderGroupCreateInfoKHR closesthitShaderGroup{};
    closesthitShaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    closesthitShaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    closesthitShaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
    closesthitShaderGroup.closestHitShader = static_cast<uint32_t>(stages.size());
    closesthitShaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
    closesthitShaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
    r_shaderGroups.push_back(closesthitShaderGroup);
    stages.push_back(chStage);
    
    //create raytracing pipeline
    VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(stages.size());
    pipelineCreateInfo.pStages = stages.data();
    pipelineCreateInfo.groupCount = static_cast<uint32_t>(r_shaderGroups.size());
    pipelineCreateInfo.pGroups = r_shaderGroups.data();
    pipelineCreateInfo.maxPipelineRayRecursionDepth = 1;
    pipelineCreateInfo.layout = r_pipelineLayout;

    vkCreateRayTracingPipelinesKHR(_vulkanDevice->_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &r_pipeline);
    for (auto stage : stages) {
        vkDestroyShaderModule(_vulkanDevice->_device, stage.module, nullptr);
    }
}

VkPhysicalDeviceRayTracingPipelinePropertiesKHR AppBase::GetRayTracingPipelineProperties() {

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR pipelineProperties{};
    pipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 deviceProperties{};
    deviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties.pNext = &pipelineProperties;

    vkGetPhysicalDeviceProperties2(_physicalDevice, &deviceProperties);

    return pipelineProperties;
}

void AppBase::CreateShaderBindingTable() {
    const auto raytracingPipelineProperties = GetRayTracingPipelineProperties();
    
    //各エントリのサイズを求める
    const uint32_t handleSize = raytracingPipelineProperties.shaderGroupHandleSize;
    const uint32_t handleAligned = raytracingPipelineProperties.shaderGroupHandleAlignment;
    const uint32_t handleSizeAligned = tools::GetAlinedSize(handleSize, handleAligned);
    const uint32_t shaderGroupSize = static_cast<uint32_t>(r_shaderGroups.size()) * handleAligned;

    //シェーダーグループのハンドルを取得する
    std::vector<uint8_t> shaderHandleStorage(shaderGroupSize);
    vkGetRayTracingShaderGroupHandlesKHR(_vulkanDevice->_device, r_pipeline, 0, static_cast<uint32_t>(r_shaderGroups.size()), shaderGroupSize, shaderHandleStorage.data());

    const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const VkMemoryPropertyFlags propertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    _vulkanDevice->CreateBuffer(handleSize, bufferUsageFlags, propertyFlags, r_raygenShaderBindingTable.buffer, r_raygenShaderBindingTable.memory);
    _vulkanDevice->CreateBuffer(handleSize, bufferUsageFlags, propertyFlags, r_missShaderBindingTable.buffer, r_missShaderBindingTable.memory);
    _vulkanDevice->CreateBuffer(handleSize, bufferUsageFlags, propertyFlags, r_hitShaderBindingTable.buffer, r_hitShaderBindingTable.memory);


    vkMapMemory(_vulkanDevice->_device, r_raygenShaderBindingTable.memory, 0, VK_WHOLE_SIZE, 0, &r_raygenShaderBindingTable.mapped);
    vkMapMemory(_vulkanDevice->_device, r_missShaderBindingTable.memory, 0, VK_WHOLE_SIZE, 0, &r_missShaderBindingTable.mapped);
    vkMapMemory(_vulkanDevice->_device, r_hitShaderBindingTable.memory, 0, VK_WHOLE_SIZE, 0, &r_hitShaderBindingTable.mapped);
    
    memcpy(r_raygenShaderBindingTable.mapped, shaderHandleStorage.data() + handleSizeAligned * raygenShaderIndex, handleSize);
    memcpy(r_missShaderBindingTable.mapped, shaderHandleStorage.data() + handleSizeAligned * missShaderIndex, handleSize);
    memcpy(r_hitShaderBindingTable.mapped, shaderHandleStorage.data() + handleSizeAligned * hitShaderIndex, handleSize);

    raygenRegion.deviceAddress = GetBufferDeviceAddress(r_raygenShaderBindingTable.buffer);
    raygenRegion.stride = handleSizeAligned;
    raygenRegion.size = handleSizeAligned;

    missRegion.deviceAddress = GetBufferDeviceAddress(r_missShaderBindingTable.buffer);
    missRegion.stride = handleSizeAligned;
    missRegion.size = handleSizeAligned;

    hitRegion.deviceAddress = GetBufferDeviceAddress(r_hitShaderBindingTable.buffer);
    hitRegion.stride = handleAligned;
    hitRegion.size = handleAligned;
}

void AppBase::CreateDescriptorSets() {
    
    //create descriptorPool
    std::vector<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR , 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE , 1 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , 1 },
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(_vulkanDevice->_device, &poolInfo, nullptr, &r_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    //create descriptorSet
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = r_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &r_descriptorSetLayout;

    if (vkAllocateDescriptorSets(_vulkanDevice->_device, &allocInfo, &r_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    //update descriptorSet
    VkWriteDescriptorSetAccelerationStructureKHR accelerationInfo{};
    accelerationInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    accelerationInfo.accelerationStructureCount = 1;
    accelerationInfo.pAccelerationStructures = &r_topLevelAS.handle;
    
    std::array<VkWriteDescriptorSet, 5>writeDescriptorSet{};
    writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[0].pNext = &accelerationInfo;
    writeDescriptorSet[0].dstSet = r_descriptorSet;
    writeDescriptorSet[0].dstBinding = 0;
    writeDescriptorSet[0].descriptorCount = 1;
    writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = r_strageImage.view;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[1].dstSet = r_descriptorSet;
    writeDescriptorSet[1].dstBinding = 1;
    writeDescriptorSet[1].descriptorCount = 1;
    writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    writeDescriptorSet[1].pImageInfo = &imageInfo;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = r_ubo.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    writeDescriptorSet[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[2].dstSet = r_descriptorSet;
    writeDescriptorSet[2].dstBinding = 2;
    writeDescriptorSet[2].descriptorCount = 1;
    writeDescriptorSet[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet[2].pBufferInfo = &bufferInfo;

    //todo:イメージの作成
    VkDescriptorImageInfo bgImageInfo{};
    bgImageInfo.imageView = s_model->_vertices.buffer;
    bgImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    writeDescriptorSet[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[3].dstSet = r_descriptorSet;
    writeDescriptorSet[3].dstBinding = 3;
    writeDescriptorSet[3].descriptorCount = 1;
    writeDescriptorSet[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet[3].pImageInfo = &bgImageInfo;

    VkDescriptorBufferInfo primMeshInfo{};
    primMeshInfo.buffer = s_model->_indices.buffer;
    primMeshInfo.offset = 0;
    primMeshInfo.range = VK_WHOLE_SIZE;

    writeDescriptorSet[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[4].dstSet = r_descriptorSet;
    writeDescriptorSet[4].dstBinding = 4;
    writeDescriptorSet[4].descriptorCount = 1;
    writeDescriptorSet[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet[4].pBufferInfo = &primMeshInfo;

    vkUpdateDescriptorSets(_vulkanDevice->_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, VK_NULL_HANDLE);

}

void AppBase::InitRayTracing() {

    CreateBLAS();
    CreateTLAS();
    CreateStrageImage();
    CreateUniformBuffer();
    CreateRaytracingLayout();
    _shader = new Shader();
    _shader->Connect(_vulkanDevice);
    CreateRaytracingPipeline();
    CreateShaderBindingTable();
    CreateDescriptorSets();
}

/*******************************************************************************************************************
*                                             描画
********************************************************************************************************************/

void AppBase::RecreateSwapChain() {

    //フレームバッファを作成し直す
    //ビューポートサイズはグラフィックスパイプラインの作成時に指定されるので、パイプラインを再構築する。
    int width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }

    //コマンドが終了するまで待つ
    vkDeviceWaitIdle(_vulkanDevice->_device);

    CleanupSwapchain();
    _swapchain->CreateSwapChain(_vulkanDevice->FindQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT));

    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateDepthResources();
    CreateFramebuffers();
    s_model->Recreate();
    CreateCommandBuffers();
    BuildCommandBuffers(true);

    _camera->UpdateAspectRatio((float)width / (float)height);
    vkQueueWaitIdle(_vulkanDevice->_queue);
}

void AppBase::drawFrame() {

    VkResult result;

    //終わっていないコマンドを待つ
    do {
        result = vkWaitForFences(_vulkanDevice->_device, 1, &_renderFences, VK_TRUE, 100000000);
    } while (result == VK_TIMEOUT);

    vkResetFences(_vulkanDevice->_device, 1, &_renderFences);

    uint32_t imageIndex;
    result = vkAcquireNextImageKHR(_vulkanDevice->_device, _swapchain->_swapchain, UINT64_MAX, _presentCompleteSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    VkCommandBuffer submitCommandBuffers = _commandBuffers[imageIndex];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &_presentCompleteSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &submitCommandBuffers;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &_renderCompleteSemaphore;

    if (vkQueueSubmit(_vulkanDevice->_queue, 1, &submitInfo, _renderFences) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    result = _swapchain->QueuePresent(_vulkanDevice->_queue, imageIndex, _renderCompleteSemaphore);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebufferResized) {
        _framebufferResized = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    vkQueueWaitIdle(_vulkanDevice->_queue);
}

void AppBase::ShowMenuFile() {
    ImGui::MenuItem("(demo menu)", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor");
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), true);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        IM_ASSERT(0);
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}
    
void AppBase::SetGUIWindow() {
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::ShowDemoWindow();
}

void AppBase::UpdateGUI() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    SetGUIWindow();
    ImGui::Render();
}

void AppBase::Run() {
    auto tStart = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(_window)) {

        glfwPollEvents();
        drawFrame();

        UpdateGUI();

        auto tEnd = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        timer = (float)elapsedTime / 1000.0f;
        UpdateUniformBuffer();

        BuildCommandBuffers(true);
    }
    vkDeviceWaitIdle(_vulkanDevice->_device);
}

/*******************************************************************************************************************
*                                             終了時
********************************************************************************************************************/

void AppBase::CleanupWindow() {
    glfwDestroyWindow(_window);
    glfwTerminate();
}

void AppBase::CleanupSwapchain() {

    vkDestroyImageView(_vulkanDevice->_device, _depthImageView, nullptr);
    vkDestroyImage(_vulkanDevice->_device, _depthImage, nullptr);
    vkFreeMemory(_vulkanDevice->_device, _depthImageMemory, nullptr);

    _swapchain->Cleanup();
    for (auto frameBuffer : _frameBuffers) {
        vkDestroyFramebuffer(_vulkanDevice->_device, frameBuffer, nullptr);
    }

    vkFreeCommandBuffers(_vulkanDevice->_device, _vulkanDevice->_commandPool, (uint32_t)_commandBuffers.size(), _commandBuffers.data());

    //vkDestroyPipeline(_vulkanDevice->_device, _pipeline, nullptr);
    //vkDestroyPipelineLayout(_vulkanDevice->_device, r_pipelineLayout, nullptr);
    vkDestroyRenderPass(_vulkanDevice->_device, _renderPass, nullptr);
    s_model->Cleanup();
}

void AppBase::Destroy() {

    CleanupSwapchain();
   
    //raytracing
    s_model->Destroy();
    r_instanceBuffer.Destroy(_vulkanDevice->_device);
    r_ubo.Destroy(_vulkanDevice->_device);
    r_raygenShaderBindingTable.Destroy(_vulkanDevice->_device);
    r_missShaderBindingTable.Destroy(_vulkanDevice->_device);
    r_hitShaderBindingTable.Destroy(_vulkanDevice->_device);

    vkDestroyBuffer(_vulkanDevice->_device, maingltf.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, maingltf.memory, nullptr);
    vkDestroyAccelerationStructureKHR(_vulkanDevice->_device, maingltf.handle, nullptr);
    vkDestroyBuffer(_vulkanDevice->_device, r_topLevelAS.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, r_topLevelAS.memory, nullptr);
    vkDestroyAccelerationStructureKHR(_vulkanDevice->_device, r_topLevelAS.handle, nullptr);

    r_strageImage.Destroy(_vulkanDevice->_device);

    vkDestroyDescriptorSetLayout(_vulkanDevice->_device, r_descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(_vulkanDevice->_device, r_descriptorPool, nullptr);

    vkDestroyPipelineLayout(_vulkanDevice->_device, r_pipelineLayout, nullptr);
    vkDestroyPipeline(_vulkanDevice->_device, r_pipeline, nullptr);

    
    vkDestroyDescriptorPool(_vulkanDevice->_device, _descriptorPool, nullptr);
    ImGui_ImplVulkan_Shutdown();

    vkDestroySemaphore(_vulkanDevice->_device, _renderCompleteSemaphore, nullptr);
    vkDestroySemaphore(_vulkanDevice->_device, _presentCompleteSemaphore, nullptr);
    vkDestroyFence(_vulkanDevice->_device, _renderFences, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    }    
    
    _vulkanDevice->Destroy();

    _swapchain->Destroy();
    vkDestroyInstance(_instance, nullptr);

    delete _vulkanDevice;
    delete _swapchain;
    delete _shader;
    delete s_model;
    delete _camera;
}