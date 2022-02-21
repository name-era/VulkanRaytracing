#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <glfw3.h>

#include "common.h"

class Swapchain {
	
public:

    struct SwapChainSupportDetails {
        //��{�I�ȕ\�ʋ@�\�i�X���b�v�`�F�[�����̉摜�̍ŏ��ő吔�A�摜�̍ŏ��ő啝�ƍ����j
        VkSurfaceCapabilitiesKHR capabilities = {};
        //�\�ʃt�H�[�}�b�g�i�s�N�Z���t�H�[�}�b�g�A�F��ԁj
        std::vector<VkSurfaceFormatKHR> formats;
        //���p�\�ȃv���[���e�[�V�������[�h
        std::vector<VkPresentModeKHR> presentModes;
    };

    /**
    * @brief    ������
    */
    void Connect(GLFWwindow* _window, VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& device);

    /**
    * @brief    �T�[�t�F�X�����
    */
    void CreateSurface();

    /**
    * @brief    �X���b�v�`�F�[���̃T�|�[�g���m�F����
    */
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, uint32_t queueFalimyIndex);

    /**
    * @brief    �X���b�v�`�F�[���̃T�[�t�F�X�t�H�[�}�b�g��I������
    */
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /**
    * @brief    �X���b�v�`�F�[���̕\�����[�h��I������
    */
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    /**
    * @brief    �X���b�v�`�F�[���͈̔͂�I������
    */
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    /**
    * @brief    �X���b�v�`�F�[�����쐬����
    */
    void CreateSwapChain(uint32_t queueFamilyIndex);

    /**
    * @brief    �\���p
    */
    VkResult QueuePresent(VkQueue& queue, uint32_t imageIndex, VkSemaphore& waitSemaphore);

    /**
    * @brief    �X���b�v�`�F�[���č쐬�̂Ƃ��ɃN���[������
    */
    void Cleanup();

    /**
    * @brief    �I����
    */
    void Destroy();


    VkExtent2D _extent = { 0,0 };
    uint32_t _imageCount = 0;
    uint32_t _minImageCount = 0;
    std::vector<vk::Image> _swapchainImages;
    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

private:
    GLFWwindow* _window = nullptr;
    VkInstance _instance = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
};