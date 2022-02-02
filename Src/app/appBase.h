#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>
#include <vector>
#include <iostream>
#include <array>
#include <chrono>
#include <unordered_map>

#include "camera.h"
#include "swapchain.h"
#include "gui.h"
#include "shader.h"

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;


class AppBase
{
public:

    struct {
        bool left = false;
        bool right = false;
        bool middle = false;
    } mouseButtons;


    bool _framebufferResized;

public:

    /**
    * @brief    コンストラクタ
    */
    AppBase();

    /**
    * @brief    デストラクタ
    */
    ~AppBase() {};

    /**
    * @brief    マウス移動
    */
    void MouseMove(double x, double y);


    /*******************************************************************************************************************
    *                                             初期化
    ********************************************************************************************************************/

    /**
    * @brief    ウィンドウの初期化
    */
    void InitializeWindow();

    /**
    * @brief    コールバックの初期化
    */
    void SetupGlfwCallbacks();

    /**
    * @brief    検証レイヤーのサポートを確認する
    */
    bool CheckValidationLayerSupport();

    /**
    * @brief    GLFWが必要としている拡張機能を取得する
    */
    std::vector<const char*> getRequiredExtensions();

    /**
    * @brief    デバッグメッセージを有効にする
    */
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /**
    * @brief    インスタンスを作成する
    */
    void CreateInstance();

    /**
    * @brief    デバッグメッセージを有効にする
    */
    void SetupDebugMessenger();

    /**
    * @brief    物理デバイスを取得する
    */
    void PickupPhysicalDevice();

    /**
    * @brief    レンダーパスを作成する
    */
    void CreateRenderPass();

    /**
    * @brief    グラフィックスパイプラインを作成する
    */
    void CreateGraphicsPipeline(Shader::ShaderModuleInfo vertModule, Shader::ShaderModuleInfo fragModule);

    /**
    * @brief    イメージの作成
    */
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    /**
    * @brief    イメージビューを作成する
    */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    /**
    * @brief    深度リソースを作成する
    */
    void CreateDepthResources();

    /**
    * @brief    フレームバッファを作成する
    */
    void CreateFramebuffers();

    /**
    * @brief    コマンドバッファを作成する
    */
    void CreateCommandBuffers();

    /**
    * @brief    同期オブジェクトを作成する
    */
    void CreateSyncObjects();

    /**
    * @brief    コールバックの初期化
    */
    void Initialize();

    /*******************************************************************************************************************
    *                                             ループ内
    ********************************************************************************************************************/

    /**
    * @brief   スワップチェーンを再構成する
    */
    void RecreateSwapChain();

    /**
    * @brief    描画する
    */
    void drawFrame();

    /**
    * @brief    ループ
    */
    void Run();

    /*******************************************************************************************************************
    *                                             終了時
    ********************************************************************************************************************/
    /**
    * @brief    ウィンドウの破棄
    */
    void CleanupWindow();

    /**
    * @brief    スワップチェーンのリソースを開放する
    */
    void CleanupSwapchain();

    /**
    * @brief    リソースを破棄する
    */
    void Destroy();


#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    VulkanDevice* _vulkanDevice;
    Swapchain* _swapchain;
    Shader* _shader;
    Gui* _gui;
    Camera* _camera;


    GLFWwindow* _window;
    VkInstance _instance;
    VkPhysicalDevice _physicalDevice;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkRenderPass _renderPass;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _pipeline;
    std::vector<VkFramebuffer> _frameBuffers;

    uint32_t _mipLevels;


    //depth
    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;



    std::vector<VkCommandBuffer> _commandBuffers;
    VkSemaphore _presentCompleteSemaphore;
    VkSemaphore _renderCompleteSemaphore;
    //Fence to wait for all command buffers to finish
    VkFence _renderFences;


    glm::vec2 mousePos;
    bool viewUpdated = false;
    float frameTimer = 1.0f;

    std::array<Shader::ShaderModuleInfo, 2> _shaderModules;

};