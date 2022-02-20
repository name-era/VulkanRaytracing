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

    VkAccelerationStructureKHR handle;
    uint64_t deviceAddress = 0;
    VkDeviceMemory memory;
    VkBuffer buffer;


    /**
    * @brief    AccelerationStructure�o�b�t�@�̍쐬
    */
    void CreateAccelerationStructureBuffer(VkAccelerationStructureGeometryKHR geometryInfo, uint32_t primitiveCount);

private:
    VulkanDevice* vulkanDevice;
};

class AppBase
{

public:
    
    struct Image {
        VkImage image;
        VkDeviceMemory memory;
        VkImageView view;
        VkSampler sampler;
        void Destroy(VkDevice device) {
            vkDestroyImage(device, image, nullptr);
            vkDestroyImageView(device, view, nullptr);
            vkFreeMemory(device, memory, nullptr);
        }
    };

    struct PolygonMesh {
        Initializers::Buffer vertexBuffer;
        Initializers::Buffer indexBuffer;

        uint32_t vertexStride = 0;

        AccelerationStructure blas;
        void Connect(Initializers::Buffer& vertBuffer, Initializers::Buffer& idxBuffer);
        void BuildBLAS(VulkanDevice* vulkanDevice);

    };

    enum MaterialType {
        LAMBERT, METAL, GLASS
    };

    enum TextureID {
        TexID_Floor, TexID_Sphere
    };

    struct Material {
        glm::vec4 diffuse = glm::vec4(1.0f);
        glm::vec4 specular = glm::vec4(1.0f, 1.0f, 1.0f, 20.0f);
        uint32_t materialType = LAMBERT;
        uint32_t textureIndex = -1;
    };

    struct SceneObject {
        PolygonMesh* mesh = nullptr;
        glm::mat4 transform = glm::mat4(1.0f);
        Material material;

        uint32_t offset = 0;
        uint32_t index = 0;
    };

    struct PrimParam
    {
        uint64_t indexBufferAddress;
        uint64_t vertexBufferAddress;
        uint32_t materialIndex;
    };

    struct UniformBlock {
        glm::mat4 viewInverse;
        glm::mat4 projInverse;
        glm::vec4 lightDirection;
        glm::vec4 lightColor;
        glm::vec4 ambientColor;
        glm::vec3 cameraPosition;
    }_uniformData;

    enum ShaderGroups {
        raygenShaderIndex = 0,
        missShaderIndex = 1,
        hitShaderIndex = 2
    };

    /**
    * @brief    �R���X�g���N�^
    */
    AppBase();

    /**
    * @brief    �f�X�g���N�^
    */
    ~AppBase() {};

    /**
    * @brief    �}�E�X�ړ�
    */
    void MouseMove(double x, double y);


    /*******************************************************************************************************************
    *                                             ������
    ********************************************************************************************************************/

    /**
    * @brief    �E�B���h�E�̏�����
    */
    void InitializeWindow();

    /**
    * @brief    �R�[���o�b�N�̏�����
    */
    void SetupGlfwCallbacks();

    /**
    * @brief    ���؃��C���[�̃T�|�[�g���m�F����
    */
    bool CheckValidationLayerSupport();

    /**
    * @brief    GLFW���K�v�Ƃ��Ă���g���@�\���擾����
    */
    std::vector<const char*> getRequiredExtensions();

    /**
    * @brief    �f�o�b�O���b�Z�[�W��L���ɂ���
    */
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /**
    * @brief    �C���X�^���X���쐬����
    */
    void CreateInstance();

    /**
    * @brief    �f�o�b�O���b�Z�[�W��L���ɂ���
    */
    void SetupDebugMessenger();

    /**
    * @brief    �����f�o�C�X���擾����
    */
    void PickupPhysicalDevice();

    /**
    * @brief    �����_�[�p�X���쐬����
    */
    void CreateRenderPass();

    /**
    * @brief    �ʏ�̃O���t�B�b�N�X�p�C�v���C�����쐬����
    */
    void CreateGraphicsPipeline();
    
    /**
    * @brief    example���g����ImGUI�̕\��
    */
    void InitializeGUI();

    /**
    * @brief    �[�x���\�[�X���쐬����
    */
    void CreateDepthResources();

    /**
    * @brief    �t���[���o�b�t�@���쐬����
    */
    void CreateFramebuffers();

    /**
    * @brief    �R�}���h�o�b�t�@���쐬����
    */
    void CreateCommandBuffers();

    /**
    * @brief    �R�}���h�o�b�t�@���X�V����
    */
    void BuildCommandBuffers(bool renderImgui);

    /**
    * @brief    �����I�u�W�F�N�g���쐬����
    */
    void CreateSyncObjects();

    /**
    * @brief    ������
    */
    void Initialize();


    /*******************************************************************************************************************
    *                                             ���C�g���[�V���O
    ********************************************************************************************************************/

    //���Ƃ�device�N���X�Ɉړ�
    Image CreateTextureCube(const wchar_t* fileNames[6], VkImageUsageFlags usage, VkMemoryPropertyFlags memProps);

    Image CreateTextureImageAndView(uint32_t width, uint32_t height, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps);

    Image Create2DTexture(const wchar_t* fileNames, VkImageUsageFlags usage, VkMemoryPropertyFlags memProps);

    /**
    * @brief    ���b�V�����\�z����
    */
    void PrepareMesh();

    /**
    * @brief    �g�p����e�N�X�`�����\�z����
    */
    void PrepareTexture();

    /**
    * @brief    BLAS���\�z����
    */
    void CreateBLAS();

    /**
    * @brief    �V�[���I�u�W�F�N�g���쐬����
    */
    void CreateSceneObject();

    /**
    * @brief    �V�[���̃I�u�W�F�N�g���̃o�b�t�@���쐬����
    */
    void CreateSceneBuffers();

    /**
    * @brief    TRAS���\�z����
    */
    void CreateTLAS();

    /**
    * @brief    Strage Image���\�z����
    */
    void CreateStrageImage();

    /**
    * @brief    ���j�t�H�[���o�b�t�@���X�V����
    */
    void UpdateUniformBuffer();
    
    /**
    * @brief    ���j�t�H�[���o�b�t�@���쐬����
    */
    void CreateUniformBuffer();

    /**
    * @brief    ���C�g���[�V���O�p�f�B�X�N���v�^�Z�b�g���C�A�E�g�A�p�C�v���C�����C�A�E�g���쐬����
    */
    void CreateRaytracingLayout();

    /**
    * @brief    ���C�g���[�V���O�p�p�C�v���C�����쐬����
    */
    void CreateRaytracingPipeline();

    /**
    * @brief    ���C�g���[�V���O�p�p�C�v���C���v���p�e�B���擾����
    */
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR GetRayTracingPipelineProperties();

    /**
    * @brief    �V�F�[�_�[�̃o�C���f�B���O�e�[�u�����쐬����
    */
    void CreateShaderBindingTable();

    /**
    * @brief    �f�B�X�N���v�^�Z�b�g���쐬����
    */
    void CreateDescriptorSets();

    /**
    * @brief    ���C�g���[�V���O������
    */
    void InitRayTracing();

    /*******************************************************************************************************************
    *                                             ���[�v��
    ********************************************************************************************************************/

    /**
    * @brief   �X���b�v�`�F�[�����č\������
    */
    void RecreateSwapChain();

    /**
    * @brief    �`�悷��
    */
    void drawFrame();

    void ShowMenuFile();

    /**
    * @brief    GUI�E�B���h�E�̍쐬
    */
    void SetGUIWindow();

    /**
    * @brief    GUI�̍X�V
    */
    void UpdateGUI();

    /**
    * @brief    ���[�v
    */
    void Run();

    /*******************************************************************************************************************
    *                                             �I����
    ********************************************************************************************************************/
    /**
    * @brief    �E�B���h�E�̔j��
    */
    void CleanupWindow();

    /**
    * @brief    �X���b�v�`�F�[���̃��\�[�X���J������
    */
    void CleanupSwapchain();

    /**
    * @brief    ���\�[�X��j������
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
    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;

    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;
    VkSemaphore _presentCompleteSemaphore;
    VkSemaphore _renderCompleteSemaphore;
    //Fence to wait for all command buffers to finish
    VkFence _renderFences;


    glm::vec2 _mousePos;
    bool viewUpdated = false;
    float timer = 0.0f;
    Initializers::MouseButtons _mouseButtons;
    bool _framebufferResized;
    
    //UI�p
    VkDescriptorPool _descriptorPool;

    //���C�g���[�V���O�i���ƂŕʃN���X�ɂ���j
    Initializers::Buffer r_instanceBuffer;
    Initializers::Buffer r_ubo;
    Initializers::Buffer r_raygenShaderBindingTable;
    Initializers::Buffer r_missShaderBindingTable;
    Initializers::Buffer r_hitShaderBindingTable;


    PolygonMesh r_meshGlTF;
    PolygonMesh r_meshPlane;
    SceneObject r_gltfModel;
    SceneObject r_ceiling;
    std::vector<SceneObject> r_sceneObjects;
    Initializers::Buffer r_materialStorageBuffer;
    Initializers::Buffer r_objectStorageBuffer;
    std::vector<Image> r_textures;
    Image r_cubeMap;

    //TLAS
    AccelerationStructure r_topLevelAS;
    
    Image r_strageImage;

    VkDescriptorSetLayout r_descriptorSetLayout;
    VkDescriptorPool r_descriptorPool;
    VkDescriptorSet r_descriptorSet;

    VkPipelineLayout r_pipelineLayout;
    VkPipeline r_pipeline;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> r_shaderGroups;

    VkStridedDeviceAddressRegionKHR raygenRegion;
    VkStridedDeviceAddressRegionKHR missRegion;
    VkStridedDeviceAddressRegionKHR hitRegion;

};