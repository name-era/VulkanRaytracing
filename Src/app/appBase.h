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
    * @brief    �R�[���o�b�N�̏�����
    */
    void Initialize();


































    //�[�x���\�[�X���쐬����
    void CreateDepthResources();

    //�t���[���o�b�t�@���쐬����
    void CreateFramebuffers();

    //���j�t�H�[���o�b�t�@���쐬����
    void CreateUniformBuffers();


    //�m�[�h�̕`��
    void DrawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Node node);

    //�R�}���h�o�b�t�@���쐬����
    void CreateCommandBuffers();

    //�����I�u�W�F�N�g���쐬����
    void CreateSyncObjects();


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


    //�J�����̓���
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