#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>


#include <vulkan/vulkan.h>
#include <spirv_reflect.h>


//���j�t�H�[���ϐ��̏��
struct UniformBinding {
    VkDescriptorType type;
    std::string name;
    uint32_t set;
    uint32_t binding;
    uint32_t count;
};

struct ShaderModuleInfo {
    VkShaderModule handle;
    VkShaderStageFlagBits stage;
    std::vector<std::vector<UniformBinding>> uniforms;
};

struct DescriptorSetLayouts {
    VkDescriptorSetLayout matrices;
    VkDescriptorSetLayout textures;
};

struct BindingInfo {
    uint32_t binding;
    VkDescriptorType type;
};

struct Pipeline {
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    DescriptorSetLayouts descriptorSetLayouts;
    VkDescriptorPool descriptorPool;

    std::vector<gltf::glTFImage> gltfImages;
    VkDescriptorSet descriptorSet;

    std::vector<BindingInfo> bindingInfo;
};


class Shader {
public:
    //�V�F�[�_�t�@�C����ǂݍ���
    static std::vector<char> ReadFile(const std::string& filename);

    //���j�t�H�[���ϐ��̏����擾����
    std::vector<std::vector<UniformBinding>> GetUniformDescriptions(SpvReflectShaderModule* module);

    //�V�F�[�_���W���[�����쐬����
    ShaderModuleInfo CreateShaderModule(const std::vector<char>& code, VkShaderStageFlagBits stage);

    //�V�F�[�_�[�t�@�C����ǂݍ���ŃV�F�[�_�[���W���[�����쐬����
    void LoadShaderPrograms(std::string vertFileName, std::string fragFileName, uint32_t shaderIndex);

    //�f�B�X�N���v�^���C�A�E�g�̍쐬�Aspirv-reflect����쐬
    void CreateDescriptorSetLayout(ShaderModuleInfo vertexModuleInfo, ShaderModuleInfo fragmentModuleInfo, uint32_t shaderIndex);

    //�O���t�B�b�N�X�p�C�v���C�����쐬����
    void CreateGraphicsPipeline(ShaderModuleInfo vertexShaderSet, ShaderModuleInfo fragmentShaderSet, uint32_t shaderIndex);

    //�L�q�q�v�[�����쐬����
    void CreateDescriptorPool(ShaderModuleInfo vertexShaderSet, ShaderModuleInfo fragmentShaderSet, uint32_t shaderIndex);

    //�L�q�q�Z�b�g���쐬����
    void CreateDescriptorSets(uint32_t shaderIndex);




private:

    UniformBufferObject _ubo;

    //�f�B�X�N���v�^
    std::vector<Pipeline*> _pipelines;
};