#pragma once

#define COLORFORMAT VK_FORMAT_R8G8B8A8_UNORM
#define MAX_FRAMES_IN_FLIGHT 2

#include <optional>
#include <stdexcept>
#include <vector>
#include <set>

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
        //Vulkan Raytracing API �ŕK�v.
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        //VK_KHR_acceleration_structure�ŕK�v
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        //descriptor indexing �ɕK�v
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    };

    /**
    * @brief    �������^�C�v��T��
    */
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /**
    * @brief    �L���[�t�@�~���C���f�b�N�X��������
    */
    uint32_t FindQueueFamilyIndex(VkQueueFlagBits queueFlags);

    /**
    * @brief    �_���f�o�C�X���擾����
    */
    void CreateLogicalDevice();

    /**
    * @brief    �f�o�C�X�̊g�����`�F�b�N����
    */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    /**
    * @brief    �f�o�C�X���g���邩�m�F����
    */
    bool IsDeviceSuitable(VkPhysicalDevice device);

    /**
    * @brief    �R�}���h�v�[�����쐬����
    */
    void CreateCommandPool();

    /**
    * @brief    �o�b�t�@���쐬����
    */
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    /**
    * @brief    �o�b�t�@���R�s�[����
    */
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkQueue queue, VkDeviceSize size);

    /**
    * @brief    �R�}���h�o�b�t�@�̋L�^�J�n
    */
    VkCommandBuffer BeginSingleTimeCommands();

    /**
    * @brief    �R�}���h�o�b�t�@�̋L�^�I��
    */
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue);

    /**
    * @brief    �T�|�[�g���Ă���t�H�[�}�b�g��������
    */
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    /**
    * @brief    �[�x�t�H�[�}�b�g��������
    */
    VkFormat FindDepthFormat();



    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VkPhysicalDeviceMemoryProperties _memProperties;
    std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
    VkCommandPool _commandPool;
    VkQueue _graphicsQueue;
    VkQueue _presentQueue;

    float _angle = 0.f;
    float _cmeraPosX = 2.0f;
    float _cameraPosY = 2.0f;
    float _cameraPsZ = 2.0f;
    float _color[4];
};