#include "device.h"


uint32_t VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_memProperties);

    for (uint32_t i = 0; i < _memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (_memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

uint32_t VulkanDevice::FindQueueFamilyIndex(VkQueueFlagBits queueFlags) {


    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);

    _queueFamilyProperties.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, _queueFamilyProperties.data());

    int i = 0;
    if (queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        for (const auto& queueFamily : _queueFamilyProperties) {

            if (queueFamily.queueFlags & queueFlags &&(queueFamily.queueFlags&& VK_QUEUE_GRAPHICS_BIT)==0) {
                return i;
            }
        }
    }

    if (queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        for (const auto& queueFamily : _queueFamilyProperties) {

            if ((queueFamily.queueFlags & queueFlags) && ((queueFamily.queueFlags && VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)==0)) {
                return i;
            }
        }
    }

    if (queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        for (const auto& queueFamily : _queueFamilyProperties) {

            if (queueFamily.queueFlags & queueFlags) {
                return i;
            }
        }
    }
}

void VulkanDevice::CreateLogicalDevice() {

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

    {
        _queueFamilyIndices.graphics = FindQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = _queueFamilyIndices.graphics;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    //PhysicalDeviceが備える各種機能を使うための準備
    //バッファのデバイスアドレスを有効にする
    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressF;
    bufferDeviceAddressF.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    bufferDeviceAddressF.bufferDeviceAddress = VK_TRUE;
    bufferDeviceAddressF.pNext = nullptr;

    //レイトレーシングパイプラインを使えるようにする
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineF;
    rayTracingPipelineF.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rayTracingPipelineF.rayTracingPipeline = VK_TRUE;
    rayTracingPipelineF.pNext = &bufferDeviceAddressF;

    //Accelerationによるレイトレーシングを有効にする
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureF;
    accelerationStructureF.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelerationStructureF.accelerationStructure = VK_TRUE;
    accelerationStructureF.pNext = &rayTracingPipelineF;

    //ディスクリプタ配列を使えるようにする
    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingF;
    descriptorIndexingF.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    descriptorIndexingF.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
    descriptorIndexingF.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    descriptorIndexingF.runtimeDescriptorArray = VK_TRUE;
    descriptorIndexingF.descriptorBindingVariableDescriptorCount = VK_TRUE;
    descriptorIndexingF.descriptorBindingPartiallyBound = VK_TRUE;
    descriptorIndexingF.pNext = &accelerationStructureF;

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(_physicalDevice, &features);
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
    physicalDeviceFeatures2.features = features;
    physicalDeviceFeatures2.pNext = &descriptorIndexingF;

    createInfo.pNext = &physicalDeviceFeatures2;
    createInfo.pEnabledFeatures = nullptr;

    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
}

void VulkanDevice::CreateCommandPool() {

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = _queueFamilyIndices.graphics;

    if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void VulkanDevice::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(_device, buffer, bufferMemory, 0);
}

VkCommandBuffer VulkanDevice::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanDevice::EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

void VulkanDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkQueue queue, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    //転送を完了する
    EndSingleTimeCommands(commandBuffer);
}

VkFormat VulkanDevice::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanDevice::FindDepthFormat() {
    //深度フォーマット候補
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}
