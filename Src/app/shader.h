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


//���spirv-reflect���g�����߂ɃN���X�ɂ��Ă�������
class Shader {

public:

    struct UniformBinding {
        VkDescriptorType type;
        std::string name;
        uint32_t set;
        uint32_t binding;
        uint32_t count;
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

    /**
    * @brief    �V�F�[�_�t�@�C����ǂݍ���
    */
    static std::vector<char> ReadFile(const std::string& filename);

    /**
    * @brief    ���j�t�H�[���ϐ��̏����擾����
    */
    std::vector<std::vector<UniformBinding>> GetUniformDescriptions(SpvReflectShaderModule* module);
    
    /**
    * @brief    �V�F�[�_���W���[�����쐬����
    */
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    
    /**
    * @brief    �V�F�[�_�[��ǂݍ���
    */
    VkPipelineShaderStageCreateInfo LoadShaderProgram(std::string shaderFileName, VkShaderStageFlagBits stage);

    /**
    * @brief    ������
    */
    void Connect(VulkanDevice* device);


    uint32_t uboCount = 0;
    uint32_t imageCount = 0;

private:
    VulkanDevice* _vulkanDevice;

};