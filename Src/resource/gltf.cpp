#include "gltf.h"

void gltf::CreateglTFImage(glTFImage* image, void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight) {

    _mipLevels = 1;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    //�X�e�[�W�o�b�t�@�Ƀe�N�X�`���̃f�[�^�𑗂�
    void* data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, buffer, bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    CreateImage(texWidth, texHeight, _mipLevels, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image->textureImage, image->textureImageMemory);
    TransitionImageLayout(image->textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _mipLevels);
    CopyBufferToImage(stagingBuffer, image->textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(image->textureImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, _mipLevels);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

}

void gltf::CreateglTFImageView(glTFImage* image, VkFormat format) {
    image->textureImageView = CreateImageView(image->textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels);
}

void gltf::CreateglTFSampler(glTFImage* image) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    //�摜�̃T�C�Y�𒴂���ꍇ�́A�e�N�X�`�����J��Ԃ�
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
    samplerInfo.maxLod = static_cast<float>(_mipLevels);
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(_device, &samplerInfo, nullptr, &image->textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void gltf::LoadglTFImages(tinygltf::Model& input) {

    _gltfImages.resize(input.textures.size());

    for (size_t i = 0; i < input.images.size(); i++) {
        tinygltf::Image& glTFImage = input.images[i];
        unsigned char* buffer = nullptr;
        VkDeviceSize bufferSize = 0;
        bool deleteBuffer = false;

        //RGB�̏ꍇ�ARGBA�ɂ��Ă���
        if (glTFImage.component == 3) {
            bufferSize = glTFImage.width * glTFImage.height * 4;
            buffer = new unsigned char[bufferSize];
            unsigned char* rgba = buffer;
            unsigned char* rgb = &glTFImage.image[0];
            for (size_t i = 0; i < glTFImage.width * glTFImage.height; i++) {
                memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                rgba += 4;
                rgb += 3;
            }
            deleteBuffer = true;
        }
        else {
            buffer = &glTFImage.image[0];
            bufferSize = glTFImage.image.size();
        }

        //�C���[�W�̐������쐬
        CreateglTFImage(&_gltfImages[i], buffer, bufferSize, VK_FORMAT_R8G8B8A8_UNORM, glTFImage.width, glTFImage.height);
        CreateglTFImageView(&_gltfImages[i], VK_FORMAT_R8G8B8A8_UNORM);
        CreateglTFSampler(&_gltfImages[i]);

        if (deleteBuffer) {
            delete[] buffer;
        }

    }
}

void gltf::LoadglTFMaterials(tinygltf::Model& input) {

    _gltfMaterials.resize(input.materials.size());

    for (size_t i = 0; i < input.materials.size(); i++) {
        tinygltf::Material glTFMaterial = input.materials[i];
        //�x�[�X�J���[
        if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
            _gltfMaterials[i].baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
        }
        //�x�[�X�J���[�̃e�N�X�`���C���f�b�N�X
        if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
            _gltfMaterials[i]._gltfBaseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
        }

    }
}

void gltf::LoadglTFTextures(tinygltf::Model& input) {
    _gltfTextures.resize(input.textures.size());
    for (size_t i = 0; i < input.textures.size(); i++) {
        _gltfTextures[i].imageIndex = input.textures[i].source;
    }
}

void gltf::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer) {
    Node node{};
    node.matrix = glm::mat4(1.0f);

    //���[�J���̃m�[�h�s��
    //4�~4�s��ɏC������
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

    //�q�m�[�h�����߂�
    if (inputNode.children.size() > 0) {
        for (size_t i = 0; i < inputNode.children.size(); i++) {
            LoadNode(input.nodes[inputNode.children[i]], input, &node, indexBuffer, vertexBuffer);
        }
    }

    //�m�[�h�����b�V���f�[�^�������Ă���ꍇ�A���_�ƃC���f�b�N�X�����[�h����
    if (inputNode.mesh > -1) {
        const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
        //���̃m�[�h�̃��b�V���ɑ΂���v���~�e�B�u����
        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            uint32_t firstIndex = static_cast<uint32_t> (indexBuffer.size());
            uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexCount = 0;


            //���_
            const float* positionBuffer = nullptr;
            const float* normalsBuffer = nullptr;
            const float* texCoordsBuffer = nullptr;
            size_t vertexCount = 0;

            //���_�f�[�^�̃o�b�t�@�f�[�^
            if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                positionBuffer = reinterpret_cast<const float*> (&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                vertexCount = accessor.count;
            }
            //���_�m�[�}���̃o�b�t�@�f�[�^
            if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }
            //���_�e�N�X�`�����W�̃o�b�t�@�f�[�^
            if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                texCoordsBuffer = reinterpret_cast<const float*> (&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }
            //�f�[�^�����f���ɓK�p
            for (size_t v = 0; v < vertexCount; v++) {
                Vertex vert{};
                vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                vert.color = glm::vec3(1.0f);
                vertexBuffer.push_back(vert);
            }

            //�C���f�b�N�X
            const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
            const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

            indexCount += static_cast<uint32_t>(accessor.count);

            //�F�X�ȃC���f�b�N�X�̃^�C�v������
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
            node.mesh.primitives.push_back(primitive);
        }
    }
    if (parent) {
        parent->children.push_back(node);
    }
    else {
        _gltfNodes.push_back(node);
    }

}

void gltf::LoadglTF(std::string filename) {

    tinygltf::Model glTFInput;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex> vertexBuffer;


    if (fileLoaded) {
        LoadglTFImages(glTFInput);
        LoadglTFMaterials(glTFInput);
        LoadglTFTextures(glTFInput);
        const tinygltf::Scene& scene = glTFInput.scenes[0];
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
            LoadNode(node, glTFInput, nullptr, indexBuffer, vertexBuffer);
        }
    }
    else {
        throw std::runtime_error("failed to find glTF file!");
    }

    //���_�o�b�t�@�̍쐬
    VkDeviceSize vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);


    Vertices vertexStaging;


    CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexStaging.buffer, vertexStaging.memory);

    void* data;
    vkMapMemory(_device, vertexStaging.memory, 0, vertexBufferSize, 0, &data);
    memcpy(data, vertexBuffer.data(), (size_t)vertexBufferSize);
    vkUnmapMemory(_device, vertexStaging.memory);

    CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _gltfVertices.buffer, _gltfVertices.memory);

    CopyBuffer(vertexStaging.buffer, _gltfVertices.buffer, vertexBufferSize);

    vkDestroyBuffer(_device, vertexStaging.buffer, nullptr);
    vkFreeMemory(_device, vertexStaging.memory, nullptr);



    //�C���f�b�N�X�o�b�t�@�̍쐬
    VkDeviceSize indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    _gltfIndices.count = static_cast<uint32_t>(indexBuffer.size());

    Vertices indexStaging;

    CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexStaging.buffer, indexStaging.memory);

    vkMapMemory(_device, indexStaging.memory, 0, indexBufferSize, 0, &data);
    memcpy(data, indexBuffer.data(), (size_t)indexBufferSize);
    vkUnmapMemory(_device, indexStaging.memory);

    CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _gltfIndices.buffer, _gltfIndices.memory);

    CopyBuffer(indexStaging.buffer, _gltfIndices.buffer, indexBufferSize);

    vkDestroyBuffer(_device, indexStaging.buffer, nullptr);
    vkFreeMemory(_device, indexStaging.memory, nullptr);


}

uint32_t gltf::getImageNum() {
    return _gltfImages.size();
}
