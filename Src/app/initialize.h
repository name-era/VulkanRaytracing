#pragma once
#include <optional>
#include <vector>
#include <set>
#include <iostream>

#include <vulkan/vulkan.h>

struct VulkanDevice {
public:

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };


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
    * @brief    コンストラクタ
    */
    VulkanDevice();

    /**
    * @brief    デストラクタ
    */
    ~VulkanDevice();



    /**
    * @brief    キューファミリを見つける
    */
    QueueFamilyIndices FindQueueFamilyIndex(VkPhysicalDevice device);

    /**
    * @brief    デバイスの拡張をチェックする
    */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    /**
    * @brief    デバイスが使えるか確認する
    */
    bool IsDeviceSuitable(VkPhysicalDevice device);

    /**
    * @brief    論理デバイスを取得する
    */
    void CreateLogicalDevice();

    /**
    * @brief    メモリタイプを探す
    */
    uint32_t FindMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /**
    * @brief    バッファを作成する
    */
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    
    /**
    * @brief    イメージの作成
    */
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    /**
    * @brief    イメージレイアウトの作成
    */
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    /**
    * @brief    バッファを画像にコピーする
    */
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    /**
    * @brief    コマンドバッファの記録開始
    */
    VkCommandBuffer BeginSingleTimeCommands();

    /**
    * @brief    コマンドバッファの記録終了
    */
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    /**
    * @brief    イメージビューを作成する
    */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    /**
    * @brief    バッファをコピーする
    */
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);



    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;

};