#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>


#include <vulkan/vulkan.h>
#include <spirv_reflect.h>


//ユニフォーム変数の情報
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
    //シェーダファイルを読み込む
    static std::vector<char> ReadFile(const std::string& filename);

    //ユニフォーム変数の情報を取得する
    std::vector<std::vector<UniformBinding>> GetUniformDescriptions(SpvReflectShaderModule* module);

    //シェーダモジュールを作成する
    ShaderModuleInfo CreateShaderModule(const std::vector<char>& code, VkShaderStageFlagBits stage);

    //シェーダーファイルを読み込んでシェーダーモジュールを作成する
    void LoadShaderPrograms(std::string vertFileName, std::string fragFileName, uint32_t shaderIndex);

    //ディスクリプタレイアウトの作成、spirv-reflectから作成
    void CreateDescriptorSetLayout(ShaderModuleInfo vertexModuleInfo, ShaderModuleInfo fragmentModuleInfo, uint32_t shaderIndex);

    //グラフィックスパイプラインを作成する
    void CreateGraphicsPipeline(ShaderModuleInfo vertexShaderSet, ShaderModuleInfo fragmentShaderSet, uint32_t shaderIndex);

    //記述子プールを作成する
    void CreateDescriptorPool(ShaderModuleInfo vertexShaderSet, ShaderModuleInfo fragmentShaderSet, uint32_t shaderIndex);

    //記述子セットを作成する
    void CreateDescriptorSets(uint32_t shaderIndex);




private:

    UniformBufferObject _ubo;

    //ディスクリプタ
    std::vector<Pipeline*> _pipelines;
};