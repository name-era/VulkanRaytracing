#pragma once

#define MAX_FRAMES_IN_FLIGHT 2

#include <optional>
#include <stdexcept>
#include <vector>
#include <set>

#include <vulkan/vulkan.h>
#include <extensions_vk.hpp>

#include "common.h"


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
        uint32_t graphics = 0;
        uint32_t compute = 0;
        uint32_t transfer = 0;
    }_queueFamilyIndices;

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
    bool IsDeviceSuitable(VkPhysicalDevice& device);

    /**
    * @brief    �R�}���h�v�[�����쐬����
    */
    void CreateCommandPool();

    /**
    * @brief    �o�b�t�@���쐬����
    */
    vk::Buffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    /**
    * @brief    �o�b�t�@���R�s�[����
    */
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    /**
    * @brief    �R�}���h�o�b�t�@�̋L�^�J�n
    */
    VkCommandBuffer BeginCommand();

    /**
    * @brief    �R�}���h�o�b�t�@�̋L�^�I�����ҋ@�i�t���[���Ɗ֘A�t���Ȃ��R�}���h�o�b�t�@�j
    */
    void FlushCommandBuffer(VkCommandBuffer& commandBuffer, VkQueue& queue, VkFenceCreateFlags flags = 0);

    /**
    * @brief    �T�|�[�g���Ă���t�H�[�}�b�g��������
    */
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    /**
    * @brief    �[�x�t�H�[�}�b�g��������
    */
    VkFormat FindDepthFormat();

    /**
    * @brief    �T���v���[���쐬����
    */
    VkSampler CreateSampler();

    /**
    * @brief    ������
    */
    void Connect(VkPhysicalDevice physicalDevice);

    /**
    * @brief    �j��
    */
    void Destroy();

    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    VkPhysicalDeviceMemoryProperties _memProperties;
    std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
    VkCommandPool _commandPool;
    VkQueue _queue;

    float _angle = 0.f;
    float _cmeraPosX = 2.0f;
    float _cameraPosY = 2.0f;
    float _cameraPsZ = 2.0f;
    float _color[4];
    VkDeviceSize minUniformBufferOffsetAlignment = 0;
    VkDeviceSize minStorageBufferOffsetAlignment = 0;
};