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

std::vector<std::vector<UniformBinding>> Shader::GetUniformDescriptions(SpvReflectShaderModule* module) {

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
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
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

void Shader::CreateDescriptorSetLayout(ShaderModuleInfo vertexModuleInfo, ShaderModuleInfo fragmentModuleInfo, uint32_t shaderIndex) {

    std::vector<VkDescriptorSetLayoutBinding> bindings;

    for (auto& set : vertexModuleInfo.uniforms) {

        std::vector<VkDescriptorSetLayoutBinding*> bindings;

        for (auto& bind : set) {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = bind.binding;
            uboLayoutBinding.descriptorCount = bind.count;
            uboLayoutBinding.descriptorType = bind.type;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = vertexModuleInfo.stage;
            bindings.push_back(&uboLayoutBinding);
        }

    }

    for (auto& set : fragmentModuleInfo.uniforms) {

        std::vector<VkDescriptorSetLayoutBinding*> bindings;

        for (auto& bind : set) {

            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = bind.binding;
            uboLayoutBinding.descriptorCount = bind.count;
            uboLayoutBinding.descriptorType = bind.type;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = fragmentModuleInfo.stage;
            bindings.push_back(&uboLayoutBinding);
        }
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_pipelines[shaderIndex]->descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void Shader::CreateGraphicsPipeline(ShaderModuleInfo vertexShaderSet, ShaderModuleInfo fragmentShaderSet, uint32_t shaderIndex) {

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertexShaderSet.handle;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragmentShaderSet.handle;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

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
    viewport.width = (float)_swapChainExtent.width;
    viewport.height = (float)_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = _swapChainExtent;

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
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
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

    std::array<VkDescriptorSetLayout, 2> bindings = { _pipelines[shaderIndex]->descriptorSetLayouts.matrices, _pipelines[shaderIndex]->descriptorSetLayouts.textures };

    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t> (bindings.size());
    pipelineLayoutInfo.pSetLayouts = bindings.data();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelines[shaderIndex]->pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipelines[shaderIndex]->graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(_device, vertexShaderSet.handle, nullptr);
    vkDestroyShaderModule(_device, fragmentShaderSet.handle, nullptr);
}

void Shader::CreateDescriptorPool(ShaderModuleInfo vertexShaderSet, ShaderModuleInfo fragmentShaderSet, uint32_t shaderIndex) {

    //uniform情報のうち、bindingとtypeだけ保存しておく
    uint32_t uniformCount = 0;
    uint32_t samplerCount = 0;

    std::vector<BindingInfo> bindingInfo;

    for (auto& set : vertexShaderSet.uniforms) {
        for (auto& bind : set) {
            if (bind.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                uniformCount++;
            }
            if (bind.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                samplerCount++;
            }

            BindingInfo bInfo;
            bInfo.binding = bind.binding;
            bInfo.type = (VkDescriptorType)bind.type;
            bindingInfo.push_back(bInfo);
        }
    }

    for (auto& set : fragmentShaderSet.uniforms) {
        for (auto& bind : set) {
            if (bind.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                uniformCount++;
            }
            if (bind.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                samplerCount++;
            }
            BindingInfo bInfo;
            bInfo.binding = bind.binding;
            bInfo.type = (VkDescriptorType)bind.type;
            bindingInfo.push_back(bInfo);
        }
    }

    _pipelines[shaderIndex]->bindingInfo = bindingInfo;

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = uniformCount;
    //マテリアルの数だけ作成する
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(_gltf->getImageNum());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(_gltf->getImageNum()) + 1;

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_pipelines[shaderIndex]->descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Shader::CreateDescriptorSets(uint32_t shaderIndex) {

    //ubo用
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _pipelines[shaderIndex]->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_pipelines[shaderIndex]->descriptorSetLayouts.matrices;


    if (vkAllocateDescriptorSets(_device, &allocInfo, &_pipelines[shaderIndex]->descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkWriteDescriptorSet descriptorWrite{};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = _ubo.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject::Values);

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = _pipelines[shaderIndex]->descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);

    //イメージサンプラー用
    for (auto& image : _pipelines[shaderIndex]->gltfImages) {

        VkDescriptorSetAllocateInfo allocInfoMaterial{};
        allocInfoMaterial.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfoMaterial.descriptorPool = _pipelines[shaderIndex]->descriptorPool;
        allocInfoMaterial.descriptorSetCount = 1;
        allocInfoMaterial.pSetLayouts = &_pipelines[shaderIndex]->descriptorSetLayouts.textures;

        if (vkAllocateDescriptorSets(_device, &allocInfoMaterial, &image.descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }


        VkDescriptorImageInfo imageInfo{};

        imageInfo.imageView = image.textureImageView;
        imageInfo.sampler = image.textureSampler;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


        VkWriteDescriptorSet descriptorWriteMaterial{};
        descriptorWriteMaterial.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWriteMaterial.dstSet = image.descriptorSet;
        descriptorWriteMaterial.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWriteMaterial.dstBinding = 0;
        descriptorWriteMaterial.pImageInfo = &imageInfo;
        descriptorWriteMaterial.descriptorCount = 1;

        vkUpdateDescriptorSets(_device, 1, &descriptorWriteMaterial, 0, nullptr);
    }
}

void Shader::LoadShaderPrograms(std::string vertFileName, std::string fragFileName, uint32_t shaderIndex) {

    auto vertShaderCode = ReadFile(vertFileName);
    auto fragShaderCode = ReadFile(fragFileName);

    ShaderModuleInfo vertShaderModule = CreateShaderModule(vertShaderCode, VK_SHADER_STAGE_VERTEX_BIT);
    ShaderModuleInfo fragShaderModule = CreateShaderModule(fragShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT);

    CreateDescriptorSetLayout(vertShaderModule, fragShaderModule, shaderIndex);

    //シェーダーごとにグラフィックスパイプラインを作成
    CreateGraphicsPipeline(vertShaderModule, fragShaderModule, shaderIndex);
    CreateDescriptorPool(vertShaderModule, fragShaderModule, shaderIndex);
    CreateDescriptorSets(shaderIndex);
}