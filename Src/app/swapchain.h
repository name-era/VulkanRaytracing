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
        //基本的な表面機能（スワップチェーン内の画像の最小最大数、画像の最小最大幅と高さ）
        VkSurfaceCapabilitiesKHR capabilities;
        //表面フォーマット（ピクセルフォーマット、色空間）
        std::vector<VkSurfaceFormatKHR> formats;
        //利用可能なプレゼンテーションモード
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct SwapchianBuffer {
        VkImage image;
        VkImageView imageview;
    };


    /**
    * @brief    初期化
    */
    void Connect(GLFWwindow* _window, VkInstance& instance, VkPhysicalDevice& physicalDevice, VkDevice& device);

    /**
    * @brief    サーフェスを作る
    */
    void CreateSurface();

    /**
    * @brief    スワップチェーンのサポートを確認する
    */
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, uint32_t queueFalimyIndex);

    /**
    * @brief    スワップチェーンのサーフェスフォーマットを選択する
    */
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    /**
    * @brief    スワップチェーンの表示モードを選択する
    */
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    /**
    * @brief    スワップチェーンの範囲を選択する
    */
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    /**
    * @brief    スワップチェーンを作成する
    */
    void CreateSwapChain(uint32_t queueFamilyIndex);

    /**
    * @brief    表示用
    */
    VkResult QueuePresent(VkQueue& queue, uint32_t imageIndex, VkSemaphore& waitSemaphore);

    /**
    * @brief    スワップチェーン再作成のときにクリーンする
    */
    void Cleanup();

    /**
    * @brief    終了時
    */
    void Destroy();


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