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
    * @brief    �O���t�B�b�N�X�p�C�v���C�����쐬����
    */
    void CreateGraphicsPipeline(Shader::ShaderModuleInfo vertModule, Shader::ShaderModuleInfo fragModule);

    /**
    * @brief    �C���[�W�̍쐬
    */
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    /**
    * @brief    �C���[�W�r���[���쐬����
    */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    /**
    * @brief    �f�v�X���\�[�X���쐬����
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
    * @brief    �����I�u�W�F�N�g���쐬����
    */
    void CreateSyncObjects();

    /**
    * @brief    �R�[���o�b�N�̏�����
    */
    void Initialize();

    /*******************************************************************************************************************
    *                                             ���[�v��
    ********************************************************************************************************************/

    //���j�t�H�[���o�b�t�@���X�V����
    void UpdateUniformBuffer();

    //�`�悷��
    void drawFrame();

    //���C�����[�v
    void Run();

    //�X���b�v�`�F�[�����č\������
    void RecreateSwapChain();

    //Mipmapg�摜���쐬����
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    //�X�e���V���R���|�[�l���g���܂܂�Ă��邩
    bool HasStencilComponent(VkFormat format);


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
    void CleanupSwapChain();

    /**
    * @brief    ���\�[�X��j������
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

    VulkanDevice* _vulkanDevice;
    Swapchain* _swapchain;
    Shader* _shader;
    glTF* _gltf;

    GLFWwindow* _window;
    VkInstance _instance;
    VkDevice _device;
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
    const int MAX_FRAMES_IN_FLIGHT;
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;
    size_t _currentFrame;





    

    float _angle = 0.f;
    float _cmeraPosX = 2.0f;
    float _cameraPosY = 2.0f;
    float _cameraPsZ = 2.0f;
    float _color[4];


    //�J�����̓���
    Camera camera;
    glm::vec2 mousePos;
    bool viewUpdated = false;
    float frameTimer = 1.0f;





};