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
    void Update(VkCommandBuffer commandBuffer, VkAccelerationStructureTypeKHR type, VkAccelerationStructureGeometryKHR geometryInfo, uint32_t primitiveCount, VkBuildAccelerationStructureFlagsKHR flags = 0);
    void CreateAccelerationStructureBuffer(VkAccelerationStructureTypeKHR type, VkAccelerationStructureGeometryKHR geometryInfo, uint32_t primitiveCount, VkBuildAccelerationStructureFlagsKHR flags = 0);
    void Destroy();
    
    VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
    uint64_t deviceAddress = 0;
    vk::Buffer asBuffer;
    vk::Buffer updateBuffer;

private:
    VulkanDevice* vulkanDevice = VK_NULL_HANDLE;
};

class AppBase
{

public:

    struct PolygonMesh {

        PolygonMesh(VulkanDevice* vulkandevice, vk::Buffer& vertBuffer, vk::Buffer& idxBuffer, uint32_t stride);
        void BuildBLAS(VulkanDevice* vulkanDevice, VkBuildAccelerationStructureFlagsKHR flags = 0);
        
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
        uint32_t useShadow = 0;

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
        uint32_t useShadow = 0;
        uint64_t padding1 = 0;
    };

    struct UniformBlock {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
        glm::vec4 lightDirection = glm::vec4(-0.2f, -1.0f, -1.0f, 0.0f);
        glm::vec4 lightColor = glm::vec4(1.0f);
        glm::vec4 ambientColor = glm::vec4(0.5f);
        glm::vec4 cameraPosition;
        glm::vec4 pointLightPosition = glm::vec4(5.0, 10.0, 5.0, 0.0);
        int shaderFlags = 0;
    }_uniformData;

    enum ShaderGroups {
        raygenShaderIndex = 0,
        missShaderIndex = 1,
        shadowMissShaderIndex = 2,
        hitShaderIndex = 3
    };

    /**
    * @brief    ?R???X?g???N?^
    */
    AppBase();

    /**
    * @brief    ?f?X?g???N?^
    */
    ~AppBase() {};

    /**
    * @brief    ?}?E?X????
    */
    void MouseMove(double x, double y);


    /*******************************************************************************************************************
    *                                             ??????
    ********************************************************************************************************************/

    /**
    * @brief    ?E?B???h?E????????
    */
    void InitializeWindow();

    /**
    * @brief    ?R?[???o?b?N????????
    */
    void SetupGlfwCallbacks();

    /**
    * @brief    ???????C???[???T?|?[?g???m?F????
    */
    bool CheckValidationLayerSupport();

    /**
    * @brief    GLFW???K?v???????????g???@?\??????????
    */
    std::vector<const char*> getRequiredExtensions();

    /**
    * @brief    ?f?o?b?O???b?Z?[?W???L????????
    */
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /**
    * @brief    ?C???X?^???X??????????
    */
    void CreateInstance();

    /**
    * @brief    ?f?o?b?O???b?Z?[?W???L????????
    */
    void SetupDebugMessenger();

    /**
    * @brief    ?????f?o?C?X??????????
    */
    void PickupPhysicalDevice();

    /**
    * @brief    ?????_?[?p?X??????????
    */
    void CreateRenderPass();

    /**
    * @brief    ???????O???t?B?b?N?X?p?C?v???C????????????
    */
    void CreateGraphicsPipeline();
    
    /**
    * @brief    example???g????ImGUI???\??
    */
    void InitGUI();

    /**
    * @brief    ?[?x???\?[?X??????????
    */
    void CreateDepthResources();

    /**
    * @brief    ?t???[???o?b?t?@??????????
    */
    void CreateFramebuffers();

    /**
    * @brief    ?R?}???h?o?b?t?@??????????
    */
    void CreateCommandBuffers();

    /**
    * @brief    ?R?}???h?o?b?t?@???X?V????
    */
    void BuildCommandBuffers(uint32_t index, bool renderImgui);

    /**
    * @brief    ?????I?u?W?F?N?g??????????
    */
    void CreateSyncObjects();

    /**
    * @brief    ??????
    */
    void Initialize();


    /*******************************************************************************************************************
    *                                             ???C?g???[?V???O
    ********************************************************************************************************************/

    //??????device?N???X??????
    vk::Image CreateTextureCube(const wchar_t* fileNames[6], VkImageUsageFlags usage, VkMemoryPropertyFlags memProps);

    vk::Image CreateTextureImageAndView(uint32_t width, uint32_t height, VkFormat format, VkImageAspectFlags aspectFlags, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps);

    vk::Image Create2DTexture(const wchar_t* fileNames, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps);


    void PrepareMesh();
    void PrepareTexture();
    void CreateBLAS();
    void CreateSceneObject();
    void UpdateMaterialsBuffer();
    void CreateSceneBuffers();

    void UpdateTLAS(VkCommandBuffer commandBuffer);
    void CreateTLAS();

    void CreateStrageImage();

    void UpdateUniformBuffer();
    void CreateUniformBuffer();

    void CreateRaytracingLayout();
    void CreateRaytracingPipeline();

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetRayTracingPipelineProperties();

    void CreateShaderBindingTable();

    void CreateDescriptorSets();

    void InitRayTracing();

    /*******************************************************************************************************************
    *                                             ???[?v??
    ********************************************************************************************************************/

    /**
    * @brief   ?X???b?v?`?F?[???????\??????
    */
    void RecreateSwapChain();

    /**
    * @brief    ?`??????
    */
    void drawFrame();

    void ShowMenuFile();

    /**
    * @brief    GUI?E?B???h?E??????
    */
    void SetGUIWindow();

    /**
    * @brief    GUI???X?V
    */
    void UpdateGUI();

    /**
    * @brief    ???[?v
    */
    void Run();


    /*******************************************************************************************************************
    *                                             ?I????
    ********************************************************************************************************************/
    /**
    * @brief    ?E?B???h?E???j??
    */
    void CleanupWindow();

    /**
    * @brief    ?X???b?v?`?F?[???????\?[?X???J??????
    */
    void CleanupSwapchain();

    /**
    * @brief    ???\?[?X???j??????
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
    
    //UI?p
    VkDescriptorPool _descriptorPool;

    //???C?g???[?V???O?i?????????N???X???????j
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