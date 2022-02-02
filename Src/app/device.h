#pragma once

#define COLORFORMAT VK_FORMAT_R8G8B8A8_UNORM
#define MAX_FRAMES_IN_FLIGHT 2

#include <optional>
#include <stdexcept>
#include <vector>
#include <set>

#include <vulkan/vulkan.h>
#include <extensions_vk.hpp>

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
    * @brief    デバイスの拡張をチェックする
    */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    /**
    * @brief    デバイスが使えるか確認する
    */
    bool IsDeviceSuitable(VkPhysicalDevice& device);

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
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

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

    /**
    * @brief    初期化
    */
    void Connect(VkPhysicalDevice physicalDevice);

    /**
    * @brief    破棄
    */
    void Destroy();

    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VkPhysicalDeviceMemoryProperties _memProperties;
    std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
    VkCommandPool _commandPool;
    VkQueue _graphicsQueue;

    float _angle = 0.f;
    float _cmeraPosX = 2.0f;
    float _cameraPosY = 2.0f;
    float _cameraPsZ = 2.0f;
    float _color[4];
};