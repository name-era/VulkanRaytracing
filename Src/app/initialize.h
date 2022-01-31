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
        //Vulkan Raytracing API �ŕK�v.
        VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
        //VK_KHR_acceleration_structure�ŕK�v
        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
        //descriptor indexing �ɕK�v
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    };

    /**
    * @brief    �R���X�g���N�^
    */
    VulkanDevice();

    /**
    * @brief    �f�X�g���N�^
    */
    ~VulkanDevice();



    /**
    * @brief    �L���[�t�@�~����������
    */
    QueueFamilyIndices FindQueueFamilyIndex(VkPhysicalDevice device);

    /**
    * @brief    �f�o�C�X�̊g�����`�F�b�N����
    */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    /**
    * @brief    �f�o�C�X���g���邩�m�F����
    */
    bool IsDeviceSuitable(VkPhysicalDevice device);

    /**
    * @brief    �_���f�o�C�X���擾����
    */
    void CreateLogicalDevice();

    /**
    * @brief    �������^�C�v��T��
    */
    uint32_t FindMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    /**
    * @brief    �o�b�t�@���쐬����
    */
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    
    /**
    * @brief    �C���[�W�̍쐬
    */
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    /**
    * @brief    �C���[�W���C�A�E�g�̍쐬
    */
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    /**
    * @brief    �o�b�t�@���摜�ɃR�s�[����
    */
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    /**
    * @brief    �R�}���h�o�b�t�@�̋L�^�J�n
    */
    VkCommandBuffer BeginSingleTimeCommands();

    /**
    * @brief    �R�}���h�o�b�t�@�̋L�^�I��
    */
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    /**
    * @brief    �C���[�W�r���[���쐬����
    */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    /**
    * @brief    �o�b�t�@���R�s�[����
    */
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);



    VkQueue _graphicsQueue;
    VkQueue _presentQueue;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;

};