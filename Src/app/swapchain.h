#pragma once

#include <vector>
#include <algorithm>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <glfw3.h>

class Swapchain {
	
public:

    struct SwapChainSupportDetails {
        //��{�I�ȕ\�ʋ@�\�i�X���b�v�`�F�[�����̉摜�̍ŏ��ő吔�A�摜�̍ŏ��ő啝�ƍ����j
        VkSurfaceCapabilitiesKHR capabilities;
        //�\�ʃt�H�[�}�b�g�i�s�N�Z���t�H�[�}�b�g�A�F��ԁj
        std::vector<VkSurfaceFormatKHR> formats;
        //���p�\�ȃv���[���e�[�V�������[�h
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct SwapchianBuffer {
        VkImage image;
        VkImageView imageview;
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
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

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
    void CreateSwapChain();

    /**
    * @brief    �����[�X
    */
    void Cleanup();

    VkExtent2D _extent;
    uint32_t _imageCount;
    uint32_t _minImageCount;
    VkFormat _colorFormat;
    std::vector<SwapchianBuffer> _swapchainBuffers;
    VkSwapchainKHR _swapchain;

private:
    GLFWwindow* _window;
    VkInstance _instance;
    VkDevice _device;
    VkPhysicalDevice _physicalDevice;
    VkSurfaceKHR _surface;

    

};