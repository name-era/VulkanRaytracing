#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cassert>

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


    //シェーダファイルを読み込む
    static std::vector<char> ReadFile(const std::string& filename);

    //ユニフォーム変数の情報を取得する
    std::vector<std::vector<UniformBinding>> GetUniformDescriptions(SpvReflectShaderModule* module);

    //シェーダモジュールを作成する
    ShaderModuleInfo CreateShaderModule(const std::vector<char>& code, VkShaderStageFlagBits stage);

    //シェーダーファイルを読み込んでシェーダーモジュールを作成する
    void LoadShaderPrograms(std::string vertFileName, std::string fragFileName, uint32_t shaderIndex, VulkanDevice* device);

    //グラフィックスパイプラインを作成する
    void CreateGraphicsPipeline(ShaderModuleInfo vertexShaderSet, ShaderModuleInfo fragmentShaderSet, uint32_t shaderIndex);



    uint32_t uboCount = 0;
    uint32_t imageCount = 0;

private:
    VulkanDevice* _vulkanDevice;

};