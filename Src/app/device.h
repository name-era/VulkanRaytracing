#pragma once

#include <optional>
#include <stdexcept>
#include <vector>

#include <vulkan/vulkan.h>

struct VulkanDevice {

    struct QueueFamily {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct QueueFamilyIndices
    {
        uint32_t graphics;
        uint32_t compute;
        uint32_t transfer;
    }_queueFamilyIndices;


    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        //Vulkan Raytracing API で必要.
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        //VK_KHR_acceleration_structureで必要
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        //descriptor indexing に必要
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    };

    /**
    * @brief    メモリタイプを探す
    */
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /**
    * @brief    キューファミリインデックスを見つける
    */
    uint32_t FindQueueFamilyIndex(VkQueueFlagBits queueFlags);

    /**
    * @brief    論理デバイスを取得する
    */
    void CreateLogicalDevice();

    /**
    * @brief    コマンドプールを作成する
    */
    void CreateCommandPool();

    /**
    * @brief    バッファを作成する
    */
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    /**
    * @brief    バッファをコピーする
    */
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkQueue queue, VkDeviceSize size);

    /**
    * @brief    コマンドバッファの記録開始
    */
    VkCommandBuffer BeginSingleTimeCommands();

    /**
    * @brief    コマンドバッファの記録終了
    */
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue);

    /**
    * @brief    サポートしているフォーマットを見つける
    */
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    /**
    * @brief    深度フォーマットを見つける
    */
    VkFormat FindDepthFormat();



    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VkPhysicalDeviceMemoryProperties _memProperties;
    std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
    VkCommandPool _commandPool;
};