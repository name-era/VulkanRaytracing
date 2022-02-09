#include "appBase.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>


/*******************************************************************************************************************
*                                             glTF
********************************************************************************************************************/

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
        * @brief    サンプラーを作成する
        */
        void CreateSampler();

        /**
        * @brief    イメージの読み込み
        */
        void LoadglTFImages(tinygltf::Image& gltfImage);

        /**
        * @brief    初期化
        */
        void Connect(VulkanDevice* device, VkQueue transQueue);
    };

    struct glTFMaterial {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        uint32_t textureIndex;
    };

    struct Primitive {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t materialIndex;
    };

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
    void LoadglTFMaterials(tinygltf::Model& input);
    void LoadTextureIndices(tinygltf::Model& input);
    void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer);
    void LoadFromFile(std::string filename);


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
    void DrawNode(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, Node node);

    /**
    * @brief    モデル描画
    */
    void Draw(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout);

    /**
    * @brief    ユニフォームバッファの更新
    */
    void UpdateUniformBuffer(glm::mat4 projection, glm::mat4 view);

    /**
    * @brief    再構成
    */
    void Recreate();

    /**
    * @brief    クリーン
    */
    void Cleanup();

    /**
    * @brief    破棄
    */
    void Destroy();

    /**
    * @brief    自インスタンスの取得
    */
    static glTF* GetglTF();

    DescriptorLayouts _descriptorSetLayout;
    VkDescriptorSet _uniformDescriptorSet;

private:
    std::vector<Texture> _textures;
    std::vector<uint32_t> _textureIndices;
    std::vector<glTFMaterial> _materials;
    std::vector<Node> _nodes;

    Vertices _vertices;
    Indices _indices;
    Initializers::Buffer _uniformBuffer;

    VulkanDevice* _vulkanDevice;

    uint32_t _mipLevel;

    VkDescriptorPool _descriptorPool;

};

namespace {
    //シングルトン
    glTF* s_gltf = nullptr;

    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        //Vulkan Raytracing API で必要
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        //VK_KHR_acceleration_structureで必要
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        //descriptor indexing に必要
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
    };
}

glTF::glTF() {

}

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

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkanDevice->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(vulkanDevice->_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, buffer, bufferSize);
    vkUnmapMemory(vulkanDevice->_device, stagingBufferMemory);

    CreateImage(texWidth, texHeight, mipLevel, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
    vulkanDevice->TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevel);
    CopyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    vulkanDevice->TransitionImageLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevel);

    vkDestroyBuffer(vulkanDevice->_device, stagingBuffer, nullptr);
    vkFreeMemory(vulkanDevice->_device, stagingBufferMemory, nullptr);
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

    if (vkCreateImageView(vulkanDevice->_device, &viewInfo, nullptr, &textureImageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

void glTF::Texture::CreateSampler() {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(vulkanDevice->_physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipLevel);
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(vulkanDevice->_device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
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
    CreateImageView(textureImage, IMAGEFORMAT, VK_IMAGE_ASPECT_COLOR_BIT, mipLevel);
    CreateSampler();

    if (deleteBuffer) {
        delete[] buffer;
    }
}

void glTF::LoadImages(tinygltf::Model& input) {
    for (tinygltf::Image& image : input.images) {
        Texture texture;
        texture.Connect(_vulkanDevice, _vulkanDevice->_queue);
        texture.LoadglTFImages(image);
        _textures.push_back(texture);
    }
}

void glTF::Texture::Connect(VulkanDevice* device, VkQueue transQueue) {

    vulkanDevice = device;
    queue = transQueue;
}

void glTF::LoadglTFMaterials(tinygltf::Model& input) {

    _materials.resize(input.materials.size());

    for (size_t i = 0; i < input.materials.size(); i++) {
        tinygltf::Material material = input.materials[i];
        //baseColor
        if (material.values.find("baseColorFactor") != material.values.end()) {
            _materials[i].baseColorFactor = glm::make_vec4(material.values["baseColorFactor"].ColorFactor().data());

        }
        //texture
        if (material.values.find("baseColorTexture") != material.values.end()) {
            _materials[i].textureIndex = material.values["baseColorTexture"].TextureIndex();
        }
    }
}

void glTF::LoadTextureIndices(tinygltf::Model& input) {
    _textureIndices.resize(input.textures.size());
    for (uint32_t i = 0; i < input.textures.size(); i++) {
        _textureIndices[i] = input.textures[i].source;
    }
}

void glTF::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer) {
    Node node{};
    node.matrix = glm::mat4(1.0f);

    //4×4行列に修正する
    if (inputNode.scale.size() == 3) {
        node.matrix = glm::scale(node.matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
    }

    if (inputNode.translation.size() == 3) {
        node.matrix = glm::translate(node.matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
    }
    if (inputNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(inputNode.rotation.data());
        node.matrix *= glm::mat4(q);
    }

    if (inputNode.matrix.size() == 16) {
        node.matrix = glm::make_mat4x4(inputNode.matrix.data());
    }

    if (inputNode.children.size() > 0) {
        for (size_t i = 0; i < inputNode.children.size(); i++) {
            LoadNode(input.nodes[inputNode.children[i]], input, &node, indexBuffer, vertexBuffer);
        }
    }

    //ノードがメッシュデータを持っている場合、頂点とインデックスをロードする
    if (inputNode.mesh > -1) {
        const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];

        //このノードのメッシュに対するプリミティブ分回す
        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            uint32_t firstIndex = static_cast<uint32_t> (indexBuffer.size());
            uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexCount = 0;


            //頂点
            const float* positionBuffer = nullptr;
            const float* normalsBuffer = nullptr;
            const float* texCoordsBuffer = nullptr;
            size_t vertexCount = 0;

            //頂点
            if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                positionBuffer = reinterpret_cast<const float*> (&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                vertexCount = accessor.count;
            }
            //ノーマル
            if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }
            //テクスチャ座標
            if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                texCoordsBuffer = reinterpret_cast<const float*> (&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }
            //データをモデルに適用
            for (size_t v = 0; v < vertexCount; v++) {
                Vertex vert{};
                vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                vert.color = glm::vec3(1.0f);
                vertexBuffer.push_back(vert);
            }

            //インデックス
            const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
            const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

            indexCount += static_cast<uint32_t>(accessor.count);

            switch (accessor.componentType) {
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                const uint32_t* buf = reinterpret_cast<const uint32_t*> (&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t index = 0; index < accessor.count; index++) {
                    indexBuffer.push_back(buf[index] + vertexStart);
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t index = 0; index < accessor.count; index++) {
                    indexBuffer.push_back(buf[index] + vertexStart);
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t index = 0; index < accessor.count; index++) {
                    indexBuffer.push_back(buf[index] + vertexStart);
                }
                break;
            }
            default:
                std::cerr << "Index conponent type" << accessor.componentType << "not supported!" << std::endl;
                return;
            }

            Primitive primitive{};
            primitive.firstIndex = firstIndex;
            primitive.indexCount = indexCount;
            primitive.materialIndex = glTFPrimitive.material;
            node.primitive.push_back(primitive);
        }
    }
    if (parent) {
        parent->children.push_back(node);
    }
    else {
        _nodes.push_back(node);
    }
}

void glTF::LoadFromFile(std::string filename) {

    tinygltf::Model glTFInput;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex> vertexBuffer;

    if (fileLoaded) {
        LoadImages(glTFInput);
        LoadglTFMaterials(glTFInput);
        LoadTextureIndices(glTFInput);
        const tinygltf::Scene& scene = glTFInput.scenes[0];
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
            LoadNode(node, glTFInput, nullptr, indexBuffer, vertexBuffer);
        }
    }
    else {
        throw std::runtime_error("failed to find glTF file!");
    }

    //vertex buffer
    VkDeviceSize vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);
    Vertices vertexStaging;
    _vulkanDevice->CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexStaging.buffer, vertexStaging.memory);

    void* data;
    vkMapMemory(_vulkanDevice->_device, vertexStaging.memory, 0, vertexBufferSize, 0, &data);
    memcpy(data, vertexBuffer.data(), (size_t)vertexBufferSize);
    vkUnmapMemory(_vulkanDevice->_device, vertexStaging.memory);

    _vulkanDevice->CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _vertices.buffer, _vertices.memory);

    _vulkanDevice->CopyBuffer(vertexStaging.buffer, _vertices.buffer, vertexBufferSize);

    vkDestroyBuffer(_vulkanDevice->_device, vertexStaging.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, vertexStaging.memory, nullptr);

    //index buffer
    VkDeviceSize indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    _indices.count = static_cast<uint32_t>(indexBuffer.size());

    Vertices indexStaging;

    _vulkanDevice->CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexStaging.buffer, indexStaging.memory);

    vkMapMemory(_vulkanDevice->_device, indexStaging.memory, 0, indexBufferSize, 0, &data);
    memcpy(data, indexBuffer.data(), (size_t)indexBufferSize);
    vkUnmapMemory(_vulkanDevice->_device, indexStaging.memory);

    _vulkanDevice->CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _indices.buffer, _indices.memory);

    _vulkanDevice->CopyBuffer(indexStaging.buffer, _indices.buffer, indexBufferSize);

    vkDestroyBuffer(_vulkanDevice->_device, indexStaging.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, indexStaging.memory, nullptr);

    //uniform buffer
    VkDeviceSize bufferSize = sizeof(UniformBlock);
    _vulkanDevice->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffer.buffer, _uniformBuffer.memory);

    vkMapMemory(_vulkanDevice->_device, _uniformBuffer.memory, 0, sizeof(_ubo), 0, &_uniformBuffer.mapped);
    memcpy(_uniformBuffer.mapped, &_ubo, sizeof(_ubo));
    vkUnmapMemory(_vulkanDevice->_device, _uniformBuffer.memory);
}

void glTF::CreateDescriptorPool() {

    uint32_t textureCount = 0;
    uint32_t uboCount = 1;

    std::vector<VkDescriptorPoolSize> poolSizes{};
    poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , uboCount });
    
    for (auto texture : _textures) {
        textureCount++;
    }

    if (textureCount > 0) {
        poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , textureCount });
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = textureCount + uboCount;

    if (vkCreateDescriptorPool(_vulkanDevice->_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void glTF::CreateDescriptorSetLayout() {

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

    if (vkCreateDescriptorSetLayout(_vulkanDevice->_device, &uboLayoutInfo, nullptr, &_descriptorSetLayout.matrix) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.pImmutableSamplers = nullptr;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo imageLayoutInfo{};
    imageLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    imageLayoutInfo.bindingCount = 1;
    imageLayoutInfo.pBindings = &samplerBinding;

    if (vkCreateDescriptorSetLayout(_vulkanDevice->_device, &imageLayoutInfo, nullptr, &_descriptorSetLayout.texture) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void glTF::CreateDescriptorSets() {

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_descriptorSetLayout.matrix;

    if (vkAllocateDescriptorSets(_vulkanDevice->_device, &allocInfo, &_uniformDescriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkWriteDescriptorSet descriptorWrite{};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = _uniformBuffer.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBlock);

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = _uniformDescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(_vulkanDevice->_device, 1, &descriptorWrite, 0, nullptr);
    

    //テクスチャ
    for (auto& texture : _textures) {

        VkDescriptorSetAllocateInfo allocInfoMaterial{};
        allocInfoMaterial.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfoMaterial.descriptorPool = _descriptorPool;
        allocInfoMaterial.descriptorSetCount = 1;
        allocInfoMaterial.pSetLayouts = &_descriptorSetLayout.texture;

        if (vkAllocateDescriptorSets(_vulkanDevice->_device, &allocInfoMaterial, &texture.descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        VkDescriptorImageInfo imageInfo{};

        imageInfo.imageView = texture.textureImageView;
        imageInfo.sampler = texture.textureSampler;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet descriptorWriteMaterial{};
        descriptorWriteMaterial.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWriteMaterial.dstSet = texture.descriptorSet;
        descriptorWriteMaterial.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWriteMaterial.dstBinding = 0;
        descriptorWriteMaterial.pImageInfo = &imageInfo;
        descriptorWriteMaterial.descriptorCount = 1;

        vkUpdateDescriptorSets(_vulkanDevice->_device, 1, &descriptorWriteMaterial, 0, nullptr);
    }
}

void glTF::CreateDescriptors() {
    CreateDescriptorPool();
    CreateDescriptorSetLayout();
    CreateDescriptorSets();
}

void glTF::Connect(VulkanDevice* device) {
    _vulkanDevice = device;
}

void glTF::DrawNode(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout, Node node)
{

    if (node.primitive.size() > 0) {

        glm::mat4 nodeMatrix = node.matrix;
        Node* currentParent = node.parent;
        while (currentParent) {
            nodeMatrix = currentParent->matrix * nodeMatrix;
            currentParent = currentParent->parent;
        }

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);

        for (Primitive& primitive : node.primitive) {
            if (primitive.indexCount > 0) {
                uint32_t index = _textureIndices[_materials[primitive.materialIndex].textureIndex];
                //現在のプリミティブのテクスチャにディスクリプタをバインドする
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &_textures[index].descriptorSet, 0, nullptr);
                vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
            }
        }
    }

    for (auto& child : node.children) {
        DrawNode(commandBuffer, pipelineLayout, child);
    }
}

void glTF::Draw(VkCommandBuffer& commandBuffer, VkPipelineLayout& pipelineLayout) {
    //モデル描画
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_vertices.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, _indices.buffer, 0, VK_INDEX_TYPE_UINT32);

    for (auto& node : _nodes) {
        DrawNode(commandBuffer, pipelineLayout, node);
    }
}

void glTF::UpdateUniformBuffer(glm::mat4 projection, glm::mat4 view) {

    _ubo.projection = projection;
    _ubo.model = view;

    memcpy(_uniformBuffer.mapped, &_ubo, sizeof(_ubo));
}

void glTF::Recreate() {
    //uniform buffer
    VkDeviceSize bufferSize = sizeof(UniformBlock);
    _vulkanDevice->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffer.buffer, _uniformBuffer.memory);
    CreateDescriptorPool();
    CreateDescriptorSets();
}

void glTF::Cleanup() {
    vkDestroyBuffer(_vulkanDevice->_device, _uniformBuffer.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, _uniformBuffer.memory, nullptr);
    vkDestroyDescriptorPool(_vulkanDevice->_device, _descriptorPool, nullptr);
}

void glTF::Destroy() {
    
    for (auto texture : _textures) {
        vkDestroySampler(_vulkanDevice->_device, texture.textureSampler, nullptr);
        vkDestroyImageView(_vulkanDevice->_device, texture.textureImageView, nullptr);
        vkDestroyImage(_vulkanDevice->_device, texture.textureImage, nullptr);
        vkFreeMemory(_vulkanDevice->_device, texture.textureImageMemory, nullptr);
    }

    vkDestroyDescriptorSetLayout(_vulkanDevice->_device, _descriptorSetLayout.matrix, nullptr);
    vkDestroyDescriptorSetLayout(_vulkanDevice->_device, _descriptorSetLayout.texture, nullptr);

    vkDestroyBuffer(_vulkanDevice->_device, _vertices.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, _vertices.memory, nullptr);

    vkDestroyBuffer(_vulkanDevice->_device, _indices.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, _indices.memory, nullptr);
}

glTF* glTF::GetglTF() {
    if (s_gltf == nullptr) {
        s_gltf = new glTF();
    }
    return s_gltf;
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
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

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

    if (enableValidationLayers) {
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
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = _vulkanDevice->FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
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

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
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
    
    std::array<VkDescriptorSetLayout, 2> bindings = { glTF::GetglTF()->_descriptorSetLayout.matrix, glTF::GetglTF()->_descriptorSetLayout.texture };

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

void AppBase::PrepareGUI() {
    
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

    _commandBuffers.resize(_swapchain->_imageCount);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _vulkanDevice->_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

    if (vkAllocateCommandBuffers(_vulkanDevice->_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    //コマンドバッファへの記録の開始
    for (size_t i = 0; i < _commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _renderPass;
        renderPassInfo.framebuffer = _frameBuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = _swapchain->_extent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.f, 0.f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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

        //glTF::GetglTF()->Draw(_commandBuffers[i], _pipelineLayout);
        if (renderImgui) {
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _commandBuffers[i]);
        }
        //_gui->DrawUI(_commandBuffers[i]);
        
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

    _swapchain = new Swapchain();
    _swapchain->Connect(_window, _instance, _physicalDevice, _vulkanDevice->_device);
    _swapchain->CreateSurface();
    _swapchain->CreateSwapChain();

    CreateRenderPass();
    _vulkanDevice->CreateCommandPool();
    
    //raytracing用に変更する
    //load gltf model
    //glTF::GetglTF()->Connect(_vulkanDevice);
    //glTF::GetglTF()->LoadFromFile("Assets/flightHelmet/FlightHelmet.gltf");
    //glTF::GetglTF()->CreateDescriptors();
    //_shader = new Shader();
    //_shader->Connect(_vulkanDevice);
    //CreateGraphicsPipeline();

    InitRayTracing();

    PrepareGUI();

    CreateDepthResources();
    CreateFramebuffers();
    CreateCommandBuffers();
    BuildCommandBuffers(false);
    CreateSyncObjects();

    _camera = new Camera();
    _camera->type = Camera::CameraType::lookat;
    _camera->flipY = true;
    _camera->setPosition(glm::vec3(0.0f, -0.1f, -1.0f));
    _camera->setRotation(glm::vec3(0.0f, -135.0f, 0.0f));
    _camera->setPerspective(60.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 256.0f);

    load_VK_EXTENSIONS(
        _instance,
        vkGetInstanceProcAddr,
        _vulkanDevice->_device,
        vkGetDeviceProcAddr
    );
}

/*******************************************************************************************************************
*                                             レイトレーシング
********************************************************************************************************************/

uint64_t AppBase::GetBufferDeviceAddress(VkBuffer buffer) {
    VkBufferDeviceAddressInfo bufferDeviceInfo{};
    bufferDeviceInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceInfo.buffer = buffer;
    return vkGetBufferDeviceAddress(_vulkanDevice->_device, &bufferDeviceInfo);
}

void AppBase::CreateAccelerationStructureBuffer(AccelerationStructure& accelerationStructure, VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkCreateBuffer(_vulkanDevice->_device, &bufferCreateInfo, nullptr, &accelerationStructure.buffer);

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(_vulkanDevice->_device, accelerationStructure.buffer, &memoryRequirements);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;

    vkAllocateMemory(_vulkanDevice->_device, &memoryAllocateInfo, nullptr, &accelerationStructure.memory);
    vkBindBufferMemory(_vulkanDevice->_device, accelerationStructure.buffer, accelerationStructure.memory, 0);
}

AppBase::RayTracingScratchBuffer AppBase::CreateScrachBuffer(VkDeviceSize size) {
    
    RayTracingScratchBuffer scratchBuffer{};

    VkBufferCreateInfo bufferCI{};
    bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCI.size = size;
    bufferCI.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    vkCreateBuffer(_vulkanDevice->_device, &bufferCI, nullptr, &scratchBuffer.buffer);

    VkMemoryRequirements memoryRequirements{};
    vkGetBufferMemoryRequirements(_vulkanDevice->_device, scratchBuffer.buffer, &memoryRequirements);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
    memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;

    vkAllocateMemory(_vulkanDevice->_device, &memoryAllocateInfo, nullptr, &scratchBuffer.memory);
    vkBindBufferMemory(_vulkanDevice->_device, scratchBuffer.buffer, scratchBuffer.memory, 0);

    VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
    bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddressInfo.buffer = scratchBuffer.buffer;

    scratchBuffer.deviceAddress = GetBufferDeviceAddress(scratchBuffer.buffer);
    return scratchBuffer;
}

void AppBase::CreateBLAS() {
    struct Vertex
    {
        float pos[3];
    };

    std::vector<Vertex> tri = {
        {1.0f, 1.0f, 0.0f},
        {-1.0f, 1.0f, 0.0f},
        {0.0f, -1.0f, 0.0f}
    };

    VkTransformMatrixKHR transformMatrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f
    };

    std::vector<uint32_t> indices = { 0,1,2 };

    _vulkanDevice->CreateBuffer(
        tri.size() * sizeof(Vertex),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        r_vertexBufferBLAS.buffer,
        r_vertexBufferBLAS.memory
    );

    vkMapMemory(_vulkanDevice->_device, r_vertexBufferBLAS.memory, 0, tri.size() * sizeof(Vertex), 0, &r_vertexBufferBLAS.mapped);
    memcpy(r_vertexBufferBLAS.mapped, r_vertexBufferBLAS.buffer, tri.size() * sizeof(Vertex));
    vkUnmapMemory(_vulkanDevice->_device, r_vertexBufferBLAS.memory);

    _vulkanDevice->CreateBuffer(
        indices.size() * sizeof(uint32_t),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        r_indexBufferBLAS.buffer,
        r_indexBufferBLAS.memory
    );

    vkMapMemory(_vulkanDevice->_device, r_indexBufferBLAS.memory, 0, indices.size() * sizeof(uint32_t), 0, &r_indexBufferBLAS.mapped);
    memcpy(r_indexBufferBLAS.mapped, r_indexBufferBLAS.buffer, indices.size() * sizeof(uint32_t));
    vkUnmapMemory(_vulkanDevice->_device, r_indexBufferBLAS.memory);

    _vulkanDevice->CreateBuffer(
        sizeof(VkTransformMatrixKHR),
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        r_transformBufferBLAS.buffer,
        r_transformBufferBLAS.memory
    );

    vkMapMemory(_vulkanDevice->_device, r_transformBufferBLAS.memory, 0, sizeof(VkTransformMatrixKHR), 0, &r_transformBufferBLAS.mapped);
    memcpy(r_transformBufferBLAS.mapped, r_transformBufferBLAS.buffer, sizeof(VkTransformMatrixKHR));
    vkUnmapMemory(_vulkanDevice->_device, r_transformBufferBLAS.memory);

    VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
    VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
    VkDeviceOrHostAddressConstKHR transformBufferDeviceAddress{};

    //デバイスアドレスを取得
    vertexBufferDeviceAddress.deviceAddress = GetBufferDeviceAddress(r_vertexBufferBLAS.buffer);
    indexBufferDeviceAddress.deviceAddress = GetBufferDeviceAddress(r_indexBufferBLAS.buffer);
    transformBufferDeviceAddress.deviceAddress = GetBufferDeviceAddress(r_transformBufferBLAS.buffer);

    VkAccelerationStructureGeometryKHR geometryInfo{};
    geometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometryInfo.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    geometryInfo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometryInfo.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    geometryInfo.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
    geometryInfo.geometry.triangles.vertexData = vertexBufferDeviceAddress;
    geometryInfo.geometry.triangles.maxVertex = static_cast<uint32_t>(tri.size());
    geometryInfo.geometry.triangles.vertexStride = sizeof(Vertex);
    geometryInfo.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
    geometryInfo.geometry.triangles.indexData = indexBufferDeviceAddress;
    geometryInfo.geometry.triangles.transformData.deviceAddress = 0;
    geometryInfo.geometry.triangles.transformData.hostAddress = nullptr;
    geometryInfo.geometry.triangles.transformData = transformBufferDeviceAddress;

    //サイズ情報を取得する
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &geometryInfo;

    const uint32_t tirangleCount = 1;
    VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo;
    buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    vkGetAccelerationStructureBuildSizesKHR(
        _vulkanDevice->_device,
        VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
        &buildGeometryInfo,
        &tirangleCount,
        &buildSizesInfo
    );

    CreateAccelerationStructureBuffer(r_bottomLevelAS, buildSizesInfo);

    //BLAS生成
    VkAccelerationStructureCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer = r_bottomLevelAS.buffer;
    createInfo.size = buildSizesInfo.accelerationStructureSize;
    createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    vkCreateAccelerationStructureKHR(_vulkanDevice->_device, &createInfo, nullptr, &r_bottomLevelAS.handle);

    //BLASの構築に必要なスクラッチ（作業）バッファの作成
    RayTracingScratchBuffer scratchBuffer = CreateScrachBuffer(buildSizesInfo.buildScratchSize);

    //VkAccelerationStructureBuildGeometryInfoKHRの他のパラメータ
    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.dstAccelerationStructure = r_bottomLevelAS.handle;
    buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

    //コマンドバッファにBLASの構築を登録する
    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo.primitiveCount = tirangleCount;
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.transformOffset = 0;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos = { &buildRangeInfo };

    VkCommandBuffer commandBuffer = _vulkanDevice->BeginCommand();
    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, buildRangeInfos.data());

    //メモリバリア
    VkBufferMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    memoryBarrier.buffer = r_bottomLevelAS.buffer;
    memoryBarrier.size = VK_WHOLE_SIZE;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.dstAccessMask = VK_QUEUE_FAMILY_IGNORED;
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
    deviceAddressInfo.accelerationStructure = r_bottomLevelAS.handle;
    r_bottomLevelAS.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(_vulkanDevice->_device, &deviceAddressInfo);

    vkDestroyBuffer(_vulkanDevice->_device, scratchBuffer.buffer, nullptr);
    vkFreeMemory(_vulkanDevice->_device, scratchBuffer.memory, nullptr);
}

void AppBase::CreateTRAS() {
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
    instance.accelerationStructureReference = r_topLevelAS.deviceAddress;
    
    _vulkanDevice->CreateBuffer(
        sizeof(VkAccelerationStructureInstanceKHR),
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        r_instanceBuffer.buffer,
        r_instanceBuffer.memory
    );

    vkMapMemory(_vulkanDevice->_device, r_instanceBuffer.memory, 0, sizeof(VkAccelerationStructureInstanceKHR), 0, &r_instanceBuffer.mapped);
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
    RayTracingScratchBuffer scratchBuffer = CreateScrachBuffer(buildSizesInfo.buildScratchSize);

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
    memoryBarrier.dstAccessMask = VK_QUEUE_FAMILY_IGNORED;
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
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, r_strageImage.image, r_strageImage.memory
    );

    CreateImageView(r_strageImage.image, SWAPCHAINCOLORFORMAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    
    _vulkanDevice->TransitionImageLayout(r_strageImage.image, VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_GENERAL,1);

}

void AppBase::UpdateUniformBuffer() {
    _uniformData.projInverse = glm::inverse(_camera->matrix.perspective);
    _uniformData.viewInverse = glm::inverse(_camera->matrix.view);
    memcpy(r_ubo.mapped, &_uniformData, sizeof(_uniformData));
}

void AppBase::CreateUniformBuffer() {
    VkDeviceSize bufferSize = sizeof(UniformBlock);
    _vulkanDevice->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, r_ubo.buffer, r_ubo.memory);

    vkMapMemory(_vulkanDevice->_device, r_ubo.memory, 0, sizeof(_uniformData), 0, &r_ubo.mapped);
    memcpy(r_ubo.mapped, &r_ubo, sizeof(_uniformData));
    vkUnmapMemory(_vulkanDevice->_device, r_ubo.memory);

    UpdateUniformBuffer();
}

void AppBase::CreateRaytracingLayout() {

    VkDescriptorSetLayoutBinding accelerationStructurelayoutBinding{};
    accelerationStructurelayoutBinding.binding = 0;
    accelerationStructurelayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    accelerationStructurelayoutBinding.descriptorCount = 1;
    accelerationStructurelayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    VkDescriptorSetLayoutBinding resultImageLayoutBinding{};
    resultImageLayoutBinding.binding = 1;
    resultImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    resultImageLayoutBinding.descriptorCount = 1;
    resultImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    VkDescriptorSetLayoutBinding uniformBufferBinding{};
    uniformBufferBinding.binding = 2;
    uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBufferBinding.descriptorCount = 1;
    uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

    std::vector<VkDescriptorSetLayoutBinding> bindings({
        accelerationStructurelayoutBinding,
        resultImageLayoutBinding,
        uniformBufferBinding
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

    //シェーダーグループの作成
    {
        auto rgStage = _shader->LoadShaderProgram("Shaders/raytracing/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_KHR);
        const int indexRaygen = 0;
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader = indexRaygen;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        r_shaderGroups.push_back(shaderGroup);
        stages.push_back(rgStage);
    }

    {
        auto missStage = _shader->LoadShaderProgram("Shaders/raytracing/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_KHR);
        const int indexRaygen = 1;
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader = indexRaygen;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        r_shaderGroups.push_back(shaderGroup);
        stages.push_back(missStage);
    }

    {
        auto chStage = _shader->LoadShaderProgram("Shaders/raytracing/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
        const int indexRaygen = 2;
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup{};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        shaderGroup.generalShader = indexRaygen;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        r_shaderGroups.push_back(shaderGroup);
        stages.push_back(chStage);
    }

    //パイプラインの生成
    VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineCreateInfo.stageCount = static_cast<uint32_t>(stages.size());
    pipelineCreateInfo.pStages = stages.data();
    pipelineCreateInfo.groupCount = static_cast<uint32_t>(r_shaderGroups.size());
    pipelineCreateInfo.pGroups = r_shaderGroups.data();
    pipelineCreateInfo.maxPipelineRayRecursionDepth = 1;
    pipelineCreateInfo.layout = r_pipelineLayout;
    vkCreateRayTracingPipelinesKHR(_vulkanDevice->_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &r_pipeline);
}

VkPhysicalDeviceRayTracingPipelinePropertiesKHR AppBase::GetRayTracingPipelineProperties() {

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR pipelineProperties;
    pipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2 deviceProperties;
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
    
    //create descriptorSet
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &r_descriptorSetLayout;

    if (vkAllocateDescriptorSets(_vulkanDevice->_device, &allocInfo, &r_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    //create descriptorPool
    std::vector<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR , 1 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE , 1 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , 1 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(_vulkanDevice->_device, &poolInfo, nullptr, &r_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    //update descriptorSet
    VkWriteDescriptorSetAccelerationStructureKHR accelerationInfo{};
    accelerationInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    accelerationInfo.accelerationStructureCount = 1;
    accelerationInfo.pAccelerationStructures = &r_topLevelAS.handle;
    
    std::array<VkWriteDescriptorSet, 3>writeDescriptorSet{};
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
    bufferInfo.range = sizeof(UniformBlock);

    writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet[1].dstSet = r_descriptorSet;
    writeDescriptorSet[1].dstBinding = 2;
    writeDescriptorSet[1].descriptorCount = 1;
    writeDescriptorSet[1].pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(_vulkanDevice->_device, static_cast<uint32_t>(writeDescriptorSet.size()), writeDescriptorSet.data(), 0, VK_NULL_HANDLE);

}

void AppBase::InitRayTracing() {
    CreateBLAS();
    CreateTRAS();
    CreateStrageImage();
    CreateUniformBuffer();
    CreateRaytracingLayout();
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
    _swapchain->CreateSwapChain();

    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateDepthResources();
    CreateFramebuffers();
    glTF::GetglTF()->Recreate();
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
    
void AppBase::SetImGuiWindow() {
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

void AppBase::Run() {
    while (!glfwWindowShouldClose(_window)) {
        
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        SetImGuiWindow();
        ImGui::Render();

        auto tStart = std::chrono::high_resolution_clock::now();

        glfwPollEvents();
        drawFrame();
        glTF::GetglTF()->UpdateUniformBuffer(_camera->matrix.perspective, _camera->matrix.view);
        
        auto tEnd = std::chrono::high_resolution_clock::now();
        auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        frameTimer = (float)tDiff / 1000.0f;
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

    vkDestroyPipeline(_vulkanDevice->_device, _pipeline, nullptr);
    vkDestroyPipelineLayout(_vulkanDevice->_device, r_pipelineLayout, nullptr);
    vkDestroyRenderPass(_vulkanDevice->_device, _renderPass, nullptr);
    glTF::GetglTF()->Cleanup();
}

void AppBase::Destroy() {

    CleanupSwapchain();
    
    glTF::GetglTF()->Destroy();
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
    delete glTF::GetglTF();
    delete _camera;
}