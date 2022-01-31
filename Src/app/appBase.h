#pragma once

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

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
#include "initialize.h"
#include "swapchain.h"
#include "gltf.h"

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
    * @brief    コールバックの初期化
    */
    void Initialize();


































    //深度リソースを作成する
    void CreateDepthResources();

    //フレームバッファを作成する
    void CreateFramebuffers();

    //ユニフォームバッファを作成する
    void CreateUniformBuffers();


    //ノードの描画
    void DrawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Node node);

    //コマンドバッファを作成する
    void CreateCommandBuffers();

    //同期オブジェクトを作成する
    void CreateSyncObjects();


    /*******************************************************************************************************************
    *                                             ループ内
    ********************************************************************************************************************/

    //ユニフォームバッファを更新する
    void UpdateUniformBuffer();

    //描画する
    void drawFrame();

    //メインループ
    void Run();

    //スワップチェーンを再構成する
    void RecreateSwapChain();

    //Mipmapg画像を作成する
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    //ステンシルコンポーネントが含まれているか
    bool HasStencilComponent(VkFormat format);






    /**
    * @brief    ウィンドウの破棄
    */
    void CleanupWindow();

    /**
    * @brief    スワップチェーンのリソースを開放する
    */
    void CleanupSwapChain();

    /**
    * @brief    リソースを破棄する
    */
    void Cleanup();

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };


    GLFWwindow* _window;
    VkInstance _instance;






    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;


    const int MAX_FRAMES_IN_FLIGHT;
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;
    size_t _currentFrame;


    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;


    uint32_t _mipLevels;

    float _angle = 0.f;
    float _cmeraPosX = 2.0f;
    float _cameraPosY = 2.0f;
    float _cameraPsZ = 2.0f;
    float _color[4];


    //カメラの動き
    Camera camera;
    glm::vec2 mousePos;
    bool viewUpdated = false;
    float frameTimer = 1.0f;


    Swapchain* _swapchain;
    glTF* _gltf;


    VkDevice _device;
    VkPhysicalDevice _physicalDevice;
    VkRenderPass _renderPass;
};