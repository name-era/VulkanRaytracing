#include "shader.h"

std::vector<char> Shader::ReadFile(const std::string& filename) {

    //ç≈å„ÇÃà íuÇì«Ç›éÊÇÈ
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

std::vector<std::vector<Shader::UniformBinding>> Shader::GetUniformDescriptions(SpvReflectShaderModule* module) {

    uint32_t set_count = 0;
    SpvReflectResult result = spvReflectEnumerateDescriptorSets(module, &set_count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectDescriptorSet*> sets(set_count);
    result = spvReflectEnumerateDescriptorSets(module, &set_count, sets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<std::vector<UniformBinding>> descriptions(sets.size());

    for (uint32_t i = 0; i < sets.size(); i++) {
        for (uint32_t j = 0; j < sets[i]->binding_count; j++) {
            auto b = sets[i]->bindings[j];
            UniformBinding u;
            if (b->name != nullptr) {
                u.name = b->name;
            }

            u.set = b->set;
            u.binding = b->binding;
            u.count = b->count;
            u.type = (VkDescriptorType)b->descriptor_type;

            descriptions[i].push_back(u);
        }
    }
    return descriptions;
}

VkShaderModule Shader::CreateShaderModule(const std::vector<char>& code) {

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_vulkanDevice->_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }


    //SpvReflectShaderModule ref;
    //SpvReflectResult result = spvReflectCreateShaderModule(code.size(), code.data(), &ref);
    //assert(result == SPV_REFLECT_RESULT_SUCCESS);
    //GetUniformDescriptions(&ref);

    return shaderModule;
}

VkPipelineShaderStageCreateInfo Shader::LoadShaderProgram(std::string shaderFileName, VkShaderStageFlagBits stage) {
    
    auto shaderCode = ReadFile(shaderFileName);
    VkShaderModule shaderModule = CreateShaderModule(shaderCode);

    VkPipelineShaderStageCreateInfo shaderStage = {};
    shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage.stage = stage;
    shaderStage.module = shaderModule;
    shaderStage.pName = "main";

    return shaderStage;
}

void Shader::Connect(VulkanDevice* device) {
    _vulkanDevice = device;
}