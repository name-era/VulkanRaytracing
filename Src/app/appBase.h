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
#include "common.h"
#include "utils.h"

class AccelerationStructure {

public:

    AccelerationStructure(){};
    AccelerationStructure(VulkanDevice* device);
    void CreateAccelerationStructureBuffer(VkAccelerationStructureTypeKHR type, VkAccelerationStructureGeometryKHR geometryInfo, uint32_t primitiveCount);
    void Destroy();
    
    VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
    uint64_t deviceAddress = 0;
    vk::Buffer asBuffer;

private:
    VulkanDevice* vulkanDevice = VK_NULL_HANDLE;
};

class AppBase
{

public:

    struct PolygonMesh {

        PolygonMesh(VulkanDevice* vulkandevice, vk::Buffer& vertBuffer, vk::Buffer& idxBuffer, uint32_t stride);
        void BuildBLAS(VulkanDevice* vulkanDevice);
        
        AccelerationStructure* blas;
        vk::Buffer vertexBuffer;
        vk::Buffer indexBuffer;
        uint32_t vertexStride = 0;
    };

    enum MaterialType {
        LAMBERT, METAL, GLASS
    };

    enum TextureID {
        TexID_Floor, TexID_Sphere
    };

    struct Material {
        glm::vec4 diffuse = glm::vec4(0.6f);
        glm::vec4 specular = glm::vec4(1.0f, 1.0f, 1.0f, 20.0f);
        uint32_t materialType = LAMBERT;
        uint32_t textureIndex = -1;
        //memory alignment:4*2byte
        uint64_t padding0 = 0;
    };

    struct SceneObject {
        PolygonMesh* mesh = nullptr;
        glm::mat4 transform = glm::mat4(1.0f);
        Material material;

        uint32_t shaderOffset = 0;
        uint32_t index = 0;
        void Destroy(VkDevice device) {
            mesh->vertexBuffer.Destroy(device);
            mesh->indexBuffer.Destroy(device);
            mesh->blas->Destroy();
        }
    };

    struct PrimParam
    {
        uint64_t indexBufferAddress;
        uint64_t vertexBufferAddress;
        uint32_t materialIndex;
        //memory alignment:4+8byte
        uint32_t padding0 = 0;
        uint64_t padding1 = 0;
    };

    struct UniformBlock {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
        glm::vec4 lightDirection;
        glm::vec4 lightColor;
        glm::vec4 ambientColor;
        glm::vec4 cameraPosition;
        glm::vec4 pointLightPosition;
        int shaderFlags = 0;
    }_uniformData;

    enum ShaderGroups {
        raygenShaderIndex = 0,
        missShaderIndex = 1,
        shadowMissShaderIndex = 2,
        hitShaderIndex = 3
    };

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
    * @brief    通常のグラフィックスパイプラインを作成する
    */
    void CreateGraphicsPipeline();
    
    /**
    * @brief    exampleを使ったImGUIの表示
    */
    void InitializeGUI();

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
    * @brief    コマンドバッファを更新する
    */
    void BuildCommandBuffers(uint32_t index, bool renderImgui);

    /**
    * @brief    同期オブジェクトを作成する
    */
    void CreateSyncObjects();

    /**
    * @brief    初期化
    */
    void Initialize();


    /*******************************************************************************************************************
    *                                             レイトレーシング
    ********************************************************************************************************************/

    //あとでdeviceクラスに移動
    vk::Image CreateTextureCube(const wchar_t* fileNames[6], VkImageUsageFlags usage, VkMemoryPropertyFlags memProps);

    vk::Image CreateTextureImageAndView(uint32_t width, uint32_t height, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps);

    vk::Image Create2DTexture(const wchar_t* fileNames, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps);

    /**
    * @brief    メッシュを構築する
    */
    void PrepareMesh();

    /**
    * @brief    使用するテクスチャを構築する
    */
    void PrepareTexture();

    /**
    * @brief    BLASを構築する
    */
    void CreateBLAS();

    /**
    * @brief    シーンオブジェクトを作成する
    */
    void CreateSceneObject();
    void UpdateMaterialsBuffer();
    /**
    * @brief    シーンのオブジェクト情報のバッファを作成する
    */
    void CreateSceneBuffers();

    /**
    * @brief    TRASを構築する
    */
    void CreateTLAS();

    /**
    * @brief    Strage Imageを構築する
    */
    void CreateStrageImage();

    /**
    * @brief    ユニフォームバッファを更新する
    */
    void UpdateUniformBuffer();
    
    /**
    * @brief    動的ユニフォームバッファを作成する（あとでストレージバッファにも対応）
    */
    void CreateUniformBuffer();

    /**
    * @brief    レイトレーシング用ディスクリプタセットレイアウト、パイプラインレイアウトを作成する
    */
    void CreateRaytracingLayout();

    /**
    * @brief    レイトレーシング用パイプラインを作成する
    */
    void CreateRaytracingPipeline();

    /**
    * @brief    レイトレーシング用パイプラインプロパティを取得する
    */
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetRayTracingPipelineProperties();

    /**
    * @brief    シェーダーのバインディングテーブルを作成する
    */
    void CreateShaderBindingTable();

    /**
    * @brief    ディスクリプタセットを作成する
    */
    void CreateDescriptorSets();

    /**
    * @brief    レイトレーシング初期化
    */
    void InitRayTracing();

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

    void ShowMenuFile();

    /**
    * @brief    GUIウィンドウの作成
    */
    void SetGUIWindow();

    /**
    * @brief    GUIの更新
    */
    void UpdateGUI();

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
    vk::Image _depthImage;

    VkCommandPool _commandPool;
    struct FrameCommandBuffer {
        VkCommandBuffer commandBuffer;
        VkFence fence;
    };
    std::vector<FrameCommandBuffer> _commandBuffers;

    VkSemaphore _presentCompleteSemaphore;
    VkSemaphore _renderCompleteSemaphore;

    glm::vec2 _mousePos;
    bool viewUpdated = false;
    float timer = 0.0f;
    vk::MouseButtons _mouseButtons;
    bool _framebufferResized;
    
    //UI用
    VkDescriptorPool _descriptorPool;

    //レイトレーシング（あとで別クラスにする）
    vk::Buffer r_instanceBuffer;
    vk::Buffer r_uniformBuffer;
    vk::Buffer r_raygenShaderBindingTable;
    vk::Buffer r_missShaderBindingTable;
    vk::Buffer r_hitShaderBindingTable;


    PolygonMesh* r_meshGlTF;
    PolygonMesh* r_meshPlane;
    PolygonMesh* r_meshSphere;
    SceneObject r_gltfModel;
    SceneObject r_ceiling;
    SceneObject r_sphere;

    std::vector<SceneObject> r_sceneObjects;
    vk::Buffer r_materialStorageBuffer;
    vk::Buffer r_objectStorageBuffer;

    std::vector<vk::Image> r_textures;
    vk::Image r_cubeMap;

    //TLAS
    AccelerationStructure* r_topLevelAS;
    vk::Image r_strageImage;

    VkDescriptorSetLayout r_descriptorSetLayout;
    VkDescriptorPool r_descriptorPool;
    VkDescriptorSet r_descriptorSet;

    VkPipelineLayout r_pipelineLayout;
    VkPipeline r_pipeline;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> r_shaderGroups;

    VkStridedDeviceAddressRegionKHR raygenRegion;
    VkStridedDeviceAddressRegionKHR missRegion;
    VkStridedDeviceAddressRegionKHR hitRegion;

    uint32_t _frameIndex = 0;

};