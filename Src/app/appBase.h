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
#include <optional>
#include <set>
#include <algorithm>
#include <array>
#include <chrono>
#include <unordered_map>


#include "camera.h"
#include "gltf.h"
#include "shader.h"
#include "gui.h"

class AppBase
{
public:

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }


        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {

            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, normal);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, uv);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };



    struct Vertices {
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

    struct Indices {
        int count;
        VkBuffer buffer;
        VkDeviceMemory memory;
    };

    struct Primitive {
        uint32_t firstIndex;
        uint32_t indexCount;
        int32_t materialIndex;
    };

    struct Mesh {
        std::vector<Primitive> primitives;
    };

    struct Node {
        Node* parent;
        std::vector<Node> children;
        Mesh mesh;
        glm::mat4 matrix;
    };


    struct UniformBufferObject {
        VkBuffer buffer;
        VkDeviceMemory memory;
        struct Values {
            glm::mat4 projection;
            glm::mat4 model;
            glm::vec4 lightPos = glm::vec4(5.0f, 5.0f, -5.0f, 1.0f);
        } values;
    };



    struct swapChainResource
    {
        VkImage swapChainImage;
        VkImageView swapChainImageView;
        VkFramebuffer swapChainFrameBuffer;
    };

    struct {
        bool left = false;
        bool right = false;
        bool middle = false;
    } mouseButtons;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };


    struct SwapChainSupportDetails {
        //基本的な表面機能（スワップチェーン内の画像の最小最大数、画像の最小最大幅と高さ）
        VkSurfaceCapabilitiesKHR capabilities;
        //表面フォーマット（ピクセルフォーマット、色空間）
        std::vector<VkSurfaceFormatKHR> formats;
        //利用可能なプレゼンテーションモード
        std::vector<VkPresentModeKHR> presentModes;
    };

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
    * @brief    サーフェスを作る
    */
    void CreateSurface();

    /**
    * @brief    キューファミリを見つける
    */
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    /**
    * @brief    デバイスの拡張をチェックする
    */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    /**
    * @brief    スワップチェインをサポートしているか確認する
    */
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    /**
    * @brief    デバイスが使えるか確認する
    */
    bool IsDeviceSuitable(VkPhysicalDevice device);

    /**
    * @brief    物理デバイスを取得する
    */
    void PickupPhysicalDevice();

    /**
    * @brief    論理デバイスを取得する
    */
    void CreateLogicalDevice();

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
    void CreateSwapChain();

    /**
    * @brief    イメージビューを作成する
    */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    /**
    * @brief    イメージビューを作成する
    */
    void CreateSwapChainImageViews();

    /**
    * @brief    サポートしているフォーマットを見つける
    */
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    /**
    * @brief    深度フォーマットを見つける
    */
    VkFormat FindDepthFormat();

    /**
    * @brief    レンダーパスを作成する
    */
    void CreateRenderPass();















    /**
    * @brief    コールバックの初期化
    */
    void Initialize();

































    //コマンドプールを作成する
    void CreateCommandPool();

    //画像作成
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    //深度リソースを作成する
    void CreateDepthResources();

    //フレームバッファを作成する
    void CreateFramebuffers();

    //メモリタイプを探す
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    //抽象バッファを作成する
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    //ユニフォームバッファを作成する
    void CreateUniformBuffers();


    //ノードの描画
    void DrawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Node node);

    //コマンドバッファを作成する
    void CreateCommandBuffers();

    //同期オブジェクトを作成する
    void CreateSyncObjects();

    //コマンドバッファの記録開始
    VkCommandBuffer BeginSingleTimeCommands();

    //コマンドバッファの記録終了
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    //テクスチャ画像を作成する
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    //バッファを画像にコピーする
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    //バッファをコピーする
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


    //毎フレーム

    //ユニフォームバッファを更新する
    void UpdateUniformBuffer();

    //描画する
    void drawFrame();

    //メインループ
    void Run();

    //スワップチェーンを再構成する
    void RecreateSwapChain();



    //終了時

    //スワップチェーンのリソースを開放する
    void CleanupSwapChain();

    //リソースを破棄する
    void Cleanup();

    //Mipmapg画像を作成する
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    //ステンシルコンポーネントが含まれているか
    bool HasStencilComponent(VkFormat format);



    /**
    * @brief    ウィンドウの破棄
    */
    void CleanupWindow();

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };



    GLFWwindow* _window;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
    VkSurfaceKHR _surface;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;





    VkQueue _graphicsQueue;
    VkQueue _presentQueue;


    //スワップチェーン関連
    VkSwapchainKHR _swapChain;
    VkFormat _swapChainImageFormat;
    VkExtent2D _swapChainExtent;
    std::vector<swapChainResource*> _swapChainResources;



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



    gltf* _gltf;
    Shader* _shader;
    Gui* _gui;

};