#include "shader.h"

std::vector<char> Shader::ReadFile(const std::string& filename) {

    //最後の位置を読み取る
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

    for (auto& s : sets) {
        for (uint32_t i = 0; i < s->binding_count; i++) {
            auto b = s->bindings[i];
            UniformBinding u;
            if (b->name != nullptr) {
                u.name = b->name;
            }

            u.set = b->set;
            u.binding = b->binding;
            u.count = b->count;
            u.type = (VkDescriptorType)b->descriptor_type;

            descriptions[b->set].push_back(u);
        }
    }
    return descriptions;
}

Shader::ShaderModuleInfo Shader::CreateShaderModule(const std::vector<char>& code, VkShaderStageFlagBits stage) {

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    //ポインターをchar型からuint32_t型へキャストする
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_vulkanDevice->_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    ShaderModuleInfo module;
    module.handle = shaderModule;
    module.stage = stage;

    SpvReflectShaderModule ref;
    SpvReflectResult result = spvReflectCreateShaderModule(code.size(), code.data(), &ref);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    module.uniforms = GetUniformDescriptions(&ref);

    return module;
}

void Shader::LoadShaderPrograms(std::string vertFileName, std::string fragFileName, uint32_t shaderIndex, VulkanDevice* device) {

    _vulkanDevice = device;

    auto vertShaderCode = ReadFile(vertFileName);
    auto fragShaderCode = ReadFile(fragFileName);

    ShaderModuleInfo vertShaderModule = CreateShaderModule(vertShaderCode, VK_SHADER_STAGE_VERTEX_BIT);
    ShaderModuleInfo fragShaderModule = CreateShaderModule(fragShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT);
}