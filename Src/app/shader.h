#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include <array>

#include <vulkan/vulkan.h>
#include <spirv_reflect.h>

#include "device.h"



class Shader {

public:

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

    struct BindingInfo {
        uint32_t binding;
        VkDescriptorType type;
    };

    struct Pipeline {
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;
        
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;
        std::vector<BindingInfo> bindingInfo;
    };

    struct ShaderModules {
        Shader::ShaderModuleInfo vert;
        Shader::ShaderModuleInfo frag;
    };


    //�V�F�[�_�t�@�C����ǂݍ���
    static std::vector<char> ReadFile(const std::string& filename);

    //���j�t�H�[���ϐ��̏����擾����
    std::vector<std::vector<UniformBinding>> GetUniformDescriptions(SpvReflectShaderModule* module);

    //�V�F�[�_���W���[�����쐬����
    ShaderModuleInfo CreateShaderModule(const std::vector<char>& code, VkShaderStageFlagBits stage);

    //�V�F�[�_�[�t�@�C����ǂݍ���ŃV�F�[�_�[���W���[�����쐬����
    ShaderModules LoadShaderPrograms(std::string vertFileName, std::string fragFileName);
  

    void Connect(VulkanDevice* device);


    uint32_t uboCount = 0;
    uint32_t imageCount = 0;

private:
    VulkanDevice* _vulkanDevice;

};