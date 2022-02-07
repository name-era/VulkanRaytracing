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


//後でspirv-reflectを使うためにクラスにしておきたい
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
    * @brief    シェーダファイルを読み込む
    */
    static std::vector<char> ReadFile(const std::string& filename);

    /**
    * @brief    ユニフォーム変数の情報を取得する
    */
    std::vector<std::vector<UniformBinding>> GetUniformDescriptions(SpvReflectShaderModule* module);
    
    /**
    * @brief    シェーダモジュールを作成する
    */
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    
    /**
    * @brief    シェーダーを読み込む
    */
    VkPipelineShaderStageCreateInfo LoadShaderProgram(std::string shaderFileName, VkShaderStageFlagBits stage);

    /**
    * @brief    初期化
    */
    void Connect(VulkanDevice* device);


    uint32_t uboCount = 0;
    uint32_t imageCount = 0;

private:
    VulkanDevice* _vulkanDevice;

};