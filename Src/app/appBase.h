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
        //��{�I�ȕ\�ʋ@�\�i�X���b�v�`�F�[�����̉摜�̍ŏ��ő吔�A�摜�̍ŏ��ő啝�ƍ����j
        VkSurfaceCapabilitiesKHR capabilities;
        //�\�ʃt�H�[�}�b�g�i�s�N�Z���t�H�[�}�b�g�A�F��ԁj
        std::vector<VkSurfaceFormatKHR> formats;
        //���p�\�ȃv���[���e�[�V�������[�h
        std::vector<VkPresentModeKHR> presentModes;
    };

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
    * @brief    �T�[�t�F�X�����
    */
    void CreateSurface();

    /**
    * @brief    �L���[�t�@�~����������
    */
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    /**
    * @brief    �f�o�C�X�̊g�����`�F�b�N����
    */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    /**
    * @brief    �X���b�v�`�F�C�����T�|�[�g���Ă��邩�m�F����
    */
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    /**
    * @brief    �f�o�C�X���g���邩�m�F����
    */
    bool IsDeviceSuitable(VkPhysicalDevice device);

    /**
    * @brief    �����f�o�C�X���擾����
    */
    void PickupPhysicalDevice();

    /**
    * @brief    �_���f�o�C�X���擾����
    */
    void CreateLogicalDevice();

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
    * @brief    �C���[�W�r���[���쐬����
    */
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    /**
    * @brief    �C���[�W�r���[���쐬����
    */
    void CreateSwapChainImageViews();

    /**
    * @brief    �T�|�[�g���Ă���t�H�[�}�b�g��������
    */
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    /**
    * @brief    �[�x�t�H�[�}�b�g��������
    */
    VkFormat FindDepthFormat();

    /**
    * @brief    �����_�[�p�X���쐬����
    */
    void CreateRenderPass();















    /**
    * @brief    �R�[���o�b�N�̏�����
    */
    void Initialize();

































    //�R�}���h�v�[�����쐬����
    void CreateCommandPool();

    //�摜�쐬
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    //�[�x���\�[�X���쐬����
    void CreateDepthResources();

    //�t���[���o�b�t�@���쐬����
    void CreateFramebuffers();

    //�������^�C�v��T��
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    //���ۃo�b�t�@���쐬����
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    //���j�t�H�[���o�b�t�@���쐬����
    void CreateUniformBuffers();


    //�m�[�h�̕`��
    void DrawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Node node);

    //�R�}���h�o�b�t�@���쐬����
    void CreateCommandBuffers();

    //�����I�u�W�F�N�g���쐬����
    void CreateSyncObjects();

    //�R�}���h�o�b�t�@�̋L�^�J�n
    VkCommandBuffer BeginSingleTimeCommands();

    //�R�}���h�o�b�t�@�̋L�^�I��
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    //�e�N�X�`���摜���쐬����
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

    //�o�b�t�@���摜�ɃR�s�[����
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    //�o�b�t�@���R�s�[����
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);


    //���t���[��

    //���j�t�H�[���o�b�t�@���X�V����
    void UpdateUniformBuffer();

    //�`�悷��
    void drawFrame();

    //���C�����[�v
    void Run();

    //�X���b�v�`�F�[�����č\������
    void RecreateSwapChain();



    //�I����

    //�X���b�v�`�F�[���̃��\�[�X���J������
    void CleanupSwapChain();

    //���\�[�X��j������
    void Cleanup();

    //Mipmapg�摜���쐬����
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    //�X�e���V���R���|�[�l���g���܂܂�Ă��邩
    bool HasStencilComponent(VkFormat format);



    /**
    * @brief    �E�B���h�E�̔j��
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


    //�X���b�v�`�F�[���֘A
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


    //�J�����̓���
    Camera camera;
    glm::vec2 mousePos;
    bool viewUpdated = false;
    float frameTimer = 1.0f;



    gltf* _gltf;
    Shader* _shader;
    Gui* _gui;

};