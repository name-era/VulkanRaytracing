#include <stdexcept>

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

#include <vector>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>


#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <spirv_reflect.h>


#include "camera.h"

#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

//VulkanBase
class VulkanBase
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

    struct glTFImage {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        VkDescriptorSet descriptorSet;
    };

    struct glTFMaterial {
        glm::vec4 baseColorFactor = glm::vec4(1.0f);
        uint32_t _gltfBaseColorTextureIndex;
    };

    struct Texture {
        int32_t imageIndex;
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

    struct DescriptorSetLayouts {
        VkDescriptorSetLayout matrices;
        VkDescriptorSetLayout textures;
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

    struct UniformBinding {
        VkDescriptorType type;
        std::string name;
        uint32_t set;
        uint32_t binding;
        uint32_t count;
    };

    struct ShaderModuleInfo {
        VkShaderModule handle;
        VkShaderStageFlagBits stage;
        std::vector<std::vector<VulkanBase::UniformBinding>> uniforms;
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


    //�R���X�g���N�^
    VulkanBase();

    //�f�X�g���N�^
    ~VulkanBase() {};

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanBase*>(glfwGetWindowUserPointer(window));
        app->_framebufferResized = true;
    }

    //�g���@�\
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }


    //������
    void InitializeVulkanBase(GLFWwindow* window);

    //�}�E�X�ړ�
    void MouseMove(double x, double y);

    //Callback
    void SetupGlfwCallbacks();

    //���؃��C���[�̃T�|�[�g���m�F����
    bool CheckValidationLayerSupport();

    //GLFW���K�v�Ƃ��Ă���g���@�\���擾����
    std::vector<const char*> getRequiredExtensions();

    //�f�o�b�O���b�Z�[�W��L���ɂ���
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    //�C���X�^���X���쐬����
    void CreateInstance();

    //�f�o�b�O���b�Z�[�W��L���ɂ���
    void SetupDebugMessenger();

    //�T�[�t�F�X�����
    void CreateSurface();

    //�f�o�C�X�̊g�����`�F�b�N����
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    //�X���b�v�`�F�C�����T�|�[�g���Ă��邩�m�F����
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

    //�f�o�C�X���g���邩�m�F����
    bool IsDeviceSuitable(VkPhysicalDevice device);

    //�����f�o�C�X���擾����
    void PickupPhysicalDevice();

    //�L���[�t�@�~����������
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    //�_���f�o�C�X���擾����
    void CreateLogicalDevice();

    //�X���b�v�`�F�[���̃T�[�t�F�X�t�H�[�}�b�g��I������
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    //�X���b�v�`�F�[���̕\�����[�h��I������
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    //�X���b�v�`�F�[���͈̔͂�I������
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    //�X���b�v�`�F�[�����쐬����
    void CreateSwapChain();

    //�C���[�W�r���[�̍쐬
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    //�C���[�W�r���[���쐬����
    void CreateSwapChainImageViews();

    //�T�|�[�g���Ă���t�H�[�}�b�g��T��
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    //�[�x�t�H�[�}�b�g��T��
    VkFormat FindDepthFormat();

    //�����_�[�p�X���쐬����
    void CreateRenderPass();

    //�V�F�[�_�t�@�C����ǂݍ���
    static std::vector<char> ReadFile(const std::string& filename);

    //���j�t�H�[���ϐ��̏����擾����
    std::vector<std::vector<UniformBinding>> GetUniformDescriptions(SpvReflectShaderModule* module);

    //�V�F�[�_���W���[�����쐬����
    ShaderModuleInfo CreateShaderModule(const std::vector<char>& code, VkShaderStageFlagBits stage);

    //�V�F�[�_�[�t�@�C����ǂݍ���ŃV�F�[�_�[���W���[�����쐬����
    void CreateShaderModules(std::string vertFileName, std::string fragFileName);

    //�f�B�X�N���v�^���C�A�E�g�̍쐬�Aspirv-reflect����쐬
    void CreateDescriptorSetLayout();

    //�O���t�B�b�N�X�p�C�v���C�����쐬����
    void CreateGraphicsPipeline();

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

    //�L�q�q�v�[�����쐬����
    void CreateDescriptorPool();

    //�L�q�q�Z�b�g���쐬����
    void CreateDescriptorSets();

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

    //glTF�֘A
    void CreateglTFImage(glTFImage* image, void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight);
    void CreateglTFImageView(glTFImage* image, VkFormat format);
    void CreateglTFSampler(glTFImage* image);
    void LoadglTFImages(tinygltf::Model& input);
    void LoadglTFMaterials(tinygltf::Model& input);
    void LoadglTFTextures(tinygltf::Model& input);
    void LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer);
    void LoadglTF(std::string filename);

    //imgui
    void CreateRenderPassForImGui();
    void CreateCommandPoolForImGui(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags);
    void CreateFrameBuffersForImGui();
    void CreateCommandBuffersForImGui(VkCommandBuffer* commandBuffer, uint32_t commandBufferCount, VkCommandPool& commandPool);
    void PrepareImGui();
    void PrepareRenderingImGui();
    void RenderImGui(uint32_t imageIndex);
    void CleanupImGui();


    //Mipmapg�摜���쐬����
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    //�X�e���V���R���|�[�l���g���܂܂�Ă��邩
    bool HasStencilComponent(VkFormat format);



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


    VkRenderPass _renderPass;
    VkPipelineLayout _pipelineLayout;
    VkPipeline _graphicsPipeline;
    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _commandBuffers;


    const int MAX_FRAMES_IN_FLIGHT;
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;
    std::vector<VkFence> _imagesInFlight;
    size_t _currentFrame;

    //�f�B�X�N���v�^
    DescriptorSetLayouts _descriptorSetLayout;
    VkDescriptorPool _descriptorPool;

    //�s��Ɏg�p����f�B�X�N���v�^
    VkDescriptorSet _descriptorSet;


    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;


    uint32_t _mipLevels;


    //ImGui
    VkDescriptorPool i_descriptorPool;
    VkRenderPass i_renderPass;
    VkCommandPool i_commandPool;
    std::vector<VkCommandBuffer> i_commandBuffers;
    std::vector<VkFramebuffer> i_frameBuffers;

    float _angle = 0.f;
    float _cmeraPosX = 2.0f;
    float _cameraPosY = 2.0f;
    float _cameraPsZ = 2.0f;
    float _color[4];


    //glTF
    VkCommandPool g_commandPool = VK_NULL_HANDLE;

    //glTF Images
    std::vector<glTFImage> _gltfImages;
    //glTF Materials
    std::vector<glTFMaterial> _gltfMaterials;
    //glTF Textures
    std::vector<Texture> _gltfTextures;

    std::vector<Node> _gltfNodes;

    Vertices _gltfVertices;
    Indices _gltfIndices;

    UniformBufferObject _gltfShaderData;

    //�J�����̓���
    Camera camera;
    glm::vec2 mousePos;
    bool viewUpdated = false;
    float frameTimer = 1.0f;

    //�V�F�[�_�[���W���[�����܂ރV�F�[�_�[�̗l�X�ȏ��
    ShaderModuleInfo _vertShaderModule;
    ShaderModuleInfo _fragShaderModule;

};

VulkanBase::VulkanBase() :
    _swapChainImageFormat(VK_FORMAT_B8G8R8A8_SRGB),
    _swapChainExtent({ 0,0 }),
    _currentFrame(0),
    _framebufferResized(false),
    MAX_FRAMES_IN_FLIGHT(2)
{
    camera.type = Camera::CameraType::lookat;
    camera.flipY = true;
    camera.setPosition(glm::vec3(0.0f, -0.1f, -1.0f));
    camera.setRotation(glm::vec3(0.0f, -135.0f, 0.0f));
    camera.setPerspective(60.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 256.0f);
}

//�}�E�X���͎��̏���
static void mouseButton(GLFWwindow* window, int button, int action, int modsy) {

    VulkanBase* instance = static_cast<VulkanBase*>(glfwGetWindowUserPointer(window));

    if (instance != NULL) {

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            //�Ƃ肠����������
            instance->mouseButtons.left = true;
        }
        else if (action == GLFW_RELEASE) {
            instance->mouseButtons.left = false;
            instance->mouseButtons.middle = false;
            instance->mouseButtons.right = false;
        }
    }
}

void VulkanBase::MouseMove(double x, double y) {
    double dx = mousePos.x - x;
    double dy = mousePos.y - y;
    bool handled = false;

    if (mouseButtons.left) {
        camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
        viewUpdated = true;
    }
    if (mouseButtons.right) {
        camera.translate(glm::vec3(-0.0f, 0.0f, dy * 0.05f));
        viewUpdated = true;
    }
    if (mouseButtons.middle) {
        camera.translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
        viewUpdated = true;
    }
    //�}�E�X�ʒu�ۑ�
    mousePos = glm::vec2((float)x, (float)y);
}

//�J�[�\������
static void cursor(GLFWwindow* window, double xpos, double ypos) {
    //���̃C���X�^���X��this�|�C���^�𓾂�
    VulkanBase* const instance(static_cast<VulkanBase*>(glfwGetWindowUserPointer(window)));
    if (instance != NULL) {
        //���[���h���W�n�ɑ΂���f�o�C�X���W�n�̊g�嗦���X�V����
        instance->MouseMove(xpos, ypos);
    }
}

//�}�E�X�z�C�[�����쎞�̏���
static void wheel(GLFWwindow* window, double x, double y) {

    VulkanBase* instance = static_cast<VulkanBase*>(glfwGetWindowUserPointer(window));

    if (instance != NULL) {
        //���[���h���W�n�ɑ΂���f�o�C�X���W�n�̊g�嗦���X�V����
        instance->camera.translate(glm::vec3(0.0f, 0.0f, (float)y * 0.05f));
    }
}

void VulkanBase::SetupGlfwCallbacks() {

    //�R�[���o�b�N�֐��̓o�^
    glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);

    glfwSetMouseButtonCallback(_window, mouseButton);

    //�}�E�X����
    glfwSetCursorPosCallback(_window, cursor);

    //�}�E�X�z�C�[�����쎞�ɌĂяo�������̓o�^
    glfwSetScrollCallback(_window, wheel);

    glfwSetWindowUserPointer(_window, this);

}

bool VulkanBase::CheckValidationLayerSupport() {

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> VulkanBase::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void VulkanBase::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

void VulkanBase::CreateInstance() {

    //���؃��C���[���L���̂Ƃ��Ɏg���邩�m�F
    if (enableValidationLayers && !CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan";
    appInfo.pEngineName = "Vulkan";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void VulkanBase::SetupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void VulkanBase::CreateSurface() {
    if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

bool VulkanBase::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VulkanBase::SwapChainSupportDetails VulkanBase::QuerySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
        //for (const auto& f : details.formats) {
        //    if (f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        //        printf("VK_COLOR_SPACE_SRGB_NONLINEAR_KHR is supported.\n");
        //    }
        //}
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool VulkanBase::IsDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = FindQueueFamilies(device);

    bool extensionsSupported = CheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    //�f�o�C�X�̊g���@�\���T�|�[�g����Ă�����A�X���b�v�`�F�C�����T�|�[�g���Ă��邩�m�F����
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && supportedFeatures.samplerAnisotropy;
}

void VulkanBase::PickupPhysicalDevice() {

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (IsDeviceSuitable(device)) {
            _physicalDevice = device;
            break;
        }
    }

    if (_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

VulkanBase::QueueFamilyIndices VulkanBase::FindQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

void VulkanBase::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    //�O���t�B�b�N�L���[�ƃv���[���e�[�V�����L���[�̗������쐬����
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    //VK_KHR_SWAPCHAIN_EXTENSION_NAME���̃X���b�v�`�F�C���ɕK�v�Ȋg���@�\��L���ɂ���
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    //�Â��o�[�W������Vulkan�ł̓C���X�^���X�ƃf�o�C�X�̌��؃��C���[����ʂ���Ă������ߔO�̂��ߐݒ肵�Ă���
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    //�L���[�n���h�����擾
    vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
}

VkSurfaceFormatKHR VulkanBase::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
            return availableFormat;
        }
    }
    //�Ή����ĂȂ�������ŏ��̃t�H�[�}�b�g�ɂ���
    return availableFormats[0];
}

VkPresentModeKHR VulkanBase::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanBase::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {

        int width, height;
        glfwGetFramebufferSize(_window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        //Retina�f�B�X�v���C��
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void VulkanBase::CreateSwapChain() {

    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = _surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);

    std::vector<VkImage> swapchainImages;
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, swapchainImages.data());

    for (uint32_t i = 0; i < imageCount; i++) {
        _swapChainResources.push_back(new swapChainResource);
        _swapChainResources[i]->swapChainImage = swapchainImages[i];
    }

    _swapChainImageFormat = surfaceFormat.format;
    _swapChainExtent = extent;
}

VkImageView VulkanBase::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(_device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void VulkanBase::CreateSwapChainImageViews() {

    for (size_t i = 0; i < _swapChainResources.size(); i++) {
        _swapChainResources[i]->swapChainImageView = CreateImageView(_swapChainResources[i]->swapChainImage, _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

VkFormat VulkanBase::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanBase::FindDepthFormat() {
    //�[�x�t�H�[�}�b�g���
    return FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void VulkanBase::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = FindDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

std::vector<char> VulkanBase::ReadFile(const std::string& filename) {

    //�Ō�̈ʒu��ǂݎ��
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

std::vector<std::vector<VulkanBase::UniformBinding>> VulkanBase::GetUniformDescriptions(SpvReflectShaderModule* module) {

    uint32_t set_count = 0;
    SpvReflectResult result = spvReflectEnumerateDescriptorSets(module, &set_count, nullptr);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);
    std::vector<SpvReflectDescriptorSet*> sets(set_count);
    result = spvReflectEnumerateDescriptorSets(module, &set_count, sets.data());
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    std::vector<std::vector<UniformBinding>> descriptions(sets.size());

    for (auto& s : sets) {
        for (uint32_t i = 0; i < s->binding_count; i++) {
            auto b = s->bindings[i];
            UniformBinding u;
            if (b->name != nullptr) {
                u.name = b->name;
            }

            u.set = b->set;
            u.binding = b->binding;
            u.count = b->count;
            u.type = (VkDescriptorType)b->descriptor_type;

            descriptions[i].push_back(u);
        }
    }
    return descriptions;
}

VulkanBase::ShaderModuleInfo VulkanBase::CreateShaderModule(const std::vector<char>& code, VkShaderStageFlagBits stage) {

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    //�|�C���^�[��char�^����uint32_t�^�փL���X�g����
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    ShaderModuleInfo module;
    module.handle = shaderModule;
    module.stage = stage;

    SpvReflectShaderModule ref;
    SpvReflectResult result = spvReflectCreateShaderModule(code.size(), code.data(), &ref);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    module.uniforms = GetUniformDescriptions(&ref);

    return module;
}

void VulkanBase::CreateShaderModules(std::string vertFileName, std::string fragFileName) {

    auto vertShaderCode = ReadFile(vertFileName);
    auto fragShaderCode = ReadFile(fragFileName);

    _vertShaderModule = CreateShaderModule(vertShaderCode, VK_SHADER_STAGE_VERTEX_BIT);
    _fragShaderModule = CreateShaderModule(fragShaderCode, VK_SHADER_STAGE_FRAGMENT_BIT);
}

void VulkanBase::CreateDescriptorSetLayout() {

    //set���ƂɃf�B�X�N���v�^���쐬
    for (auto& set : _vertShaderModule.uniforms) {

        std::vector<VkDescriptorSetLayoutBinding*> bindings;

        for (auto& bind : set) {
            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = bind.binding;
            uboLayoutBinding.descriptorCount = bind.count;
            uboLayoutBinding.descriptorType = bind.type;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = _vertShaderModule.stage;
            bindings.push_back(&uboLayoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = *bindings.data();


        if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout.matrices) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    for (auto& set : _fragShaderModule.uniforms) {

        std::vector<VkDescriptorSetLayoutBinding*> bindings;

        for (auto& bind : set) {

            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = bind.binding;
            uboLayoutBinding.descriptorCount = bind.count;
            uboLayoutBinding.descriptorType = bind.type;
            uboLayoutBinding.pImmutableSamplers = nullptr;
            uboLayoutBinding.stageFlags = _fragShaderModule.stage;
            bindings.push_back(&uboLayoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = *bindings.data();

        if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout.textures) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }
}

void VulkanBase::CreateGraphicsPipeline() {

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = _vertShaderModule.stage;
    vertShaderStageInfo.module = _vertShaderModule.handle;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = _fragShaderModule.stage;
    fragShaderStageInfo.module = _fragShaderModule.handle;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)_swapChainExtent.width;
    viewport.height = (float)_swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = _swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.size = sizeof(glm::mat4);
    pushConstantRange.offset = 0;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

    std::array<VkDescriptorSetLayout, 2> bindings = { _descriptorSetLayout.matrices, _descriptorSetLayout.textures };

    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t> (bindings.size());
    pipelineLayoutInfo.pSetLayouts = bindings.data();
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    //vkDestroyShaderModule(_device, _vertShaderModule.handle, nullptr);
    //vkDestroyShaderModule(_device, _fragShaderModule.handle, nullptr);
}

void VulkanBase::CreateCommandPool() {

    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(_physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void VulkanBase::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (!(imageInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
        imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_device, image, imageMemory, 0);
}

void VulkanBase::CreateDepthResources() {

    VkFormat depthFormat = FindDepthFormat();

    CreateImage(_swapChainExtent.width, _swapChainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depthImage, _depthImageMemory);
    _depthImageView = CreateImageView(_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

void VulkanBase::CreateFramebuffers() {

    for (size_t i = 0; i < _swapChainResources.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            _swapChainResources[i]->swapChainImageView,
            _depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = _swapChainExtent.width;
        framebufferInfo.height = _swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainResources[i]->swapChainFrameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

}

uint32_t VulkanBase::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanBase::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(_device, buffer, bufferMemory, 0);
}

void VulkanBase::CreateUniformBuffers() {

    VkDeviceSize bufferSize = sizeof(_gltfShaderData.values);
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _gltfShaderData.buffer, _gltfShaderData.memory);
}

void VulkanBase::CreateDescriptorPool() {

    //���ڃf�B�X�N���v�^�Z�b�g���쐬���邱�Ƃ͂ł��Ȃ��A�R�}���h�o�b�t�@�̂悤�Ƀv�[�����犄�蓖�Ă�K�v������
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(_gltfImages.size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    //���̃f�B�X�N���v�^���l������1�𑫂�
    poolInfo.maxSets = static_cast<uint32_t>(_gltfImages.size()) + 1;

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

}

void VulkanBase::CreateDescriptorSets() {

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_descriptorSetLayout.matrices;


    if (vkAllocateDescriptorSets(_device, &allocInfo, &_descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkWriteDescriptorSet descriptorWrite{};

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = _gltfShaderData.buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject::Values);

    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = _descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);


    for (auto& image : _gltfImages) {

        VkDescriptorSetAllocateInfo allocInfoMaterial{};
        allocInfoMaterial.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfoMaterial.descriptorPool = _descriptorPool;
        allocInfoMaterial.descriptorSetCount = 1;
        allocInfoMaterial.pSetLayouts = &_descriptorSetLayout.textures;

        if (vkAllocateDescriptorSets(_device, &allocInfoMaterial, &image.descriptorSet) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        //�}�e���A���p
        VkDescriptorImageInfo imageInfo{};

        imageInfo.imageView = image.textureImageView;
        imageInfo.sampler = image.textureSampler;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


        VkWriteDescriptorSet descriptorWriteMaterial{};
        descriptorWriteMaterial.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWriteMaterial.dstSet = image.descriptorSet;
        descriptorWriteMaterial.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWriteMaterial.dstBinding = 0;
        descriptorWriteMaterial.pImageInfo = &imageInfo;
        descriptorWriteMaterial.descriptorCount = 1;

        vkUpdateDescriptorSets(_device, 1, &descriptorWriteMaterial, 0, nullptr);
    }
}

void VulkanBase::DrawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, Node node)
{
    if (node.mesh.primitives.size() > 0) {

        glm::mat4 nodeMatrix = node.matrix;
        Node* currentParent = node.parent;
        while (currentParent) {
            nodeMatrix = currentParent->matrix * nodeMatrix;
            currentParent = currentParent->parent;
        }

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);

        for (Primitive& primitive : node.mesh.primitives) {
            if (primitive.indexCount > 0) {
                //�e�N�X�`���C���f�b�N�X�̎擾
                Texture texture = _gltfTextures[_gltfMaterials[primitive.materialIndex]._gltfBaseColorTextureIndex];

                //���݂̃v���~�e�B�u�̃e�N�X�`���Ƀf�B�X�N���v�^���o�C���h����
                vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &_gltfImages[texture.imageIndex].descriptorSet, 0, nullptr);
                vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
            }
        }
    }

    for (auto& child : node.children) {
        DrawNode(commandBuffer, pipelineLayout, child);
    }
}

void VulkanBase::CreateCommandBuffers() {

    //�R�}���h�o�b�t�@�̊��蓖��
    _commandBuffers.resize(_swapChainResources.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

    if (vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    //�R�}���h�o�b�t�@�ւ̋L�^�̊J�n
    for (size_t i = 0; i < _commandBuffers.size(); i++) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = _renderPass;
        renderPassInfo.framebuffer = _swapChainResources[i]->swapChainFrameBuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = _swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.f, 0.f, 0.f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

        vkCmdBindDescriptorSets(_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);


        //���f���`��
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(_commandBuffers[i], 0, 1, &_gltfVertices.buffer, offsets);

        //���_����2^16������������UINT32
        vkCmdBindIndexBuffer(_commandBuffers[i], _gltfIndices.buffer, 0, VK_INDEX_TYPE_UINT32);

        for (auto& node : _gltfNodes) {
            DrawNode(_commandBuffers[i], _pipelineLayout, node);
        }

        vkCmdEndRenderPass(_commandBuffers[i]);

        if (vkEndCommandBuffer(_commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void VulkanBase::CreateSyncObjects() {
    //�Z�}�t�H�F
    //�摜���擾����ă����_�����O�̏������ł������Ƃ�ʒm����
    //�����_�����O���I�����ăv���[���e�[�V�������s����\�������邱�Ƃ�ʒm����
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    _imagesInFlight.resize(_swapChainResources.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

VkCommandBuffer VulkanBase::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void VulkanBase::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

void VulkanBase::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    //barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeCommands(commandBuffer);
}

void VulkanBase::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


    EndSingleTimeCommands(commandBuffer);
}

void VulkanBase::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    //�]������������
    EndSingleTimeCommands(commandBuffer);
}


//�`��
void VulkanBase::UpdateUniformBuffer() {
    _gltfShaderData.values.projection = camera.matrices.perspective;
    _gltfShaderData.values.model = camera.matrices.view;

    void* data;
    vkMapMemory(_device, _gltfShaderData.memory, 0, sizeof(_gltfShaderData.values), 0, &data);
    memcpy(data, &_gltfShaderData.values, sizeof(_gltfShaderData.values));
    vkUnmapMemory(_device, _gltfShaderData.memory);
}

void VulkanBase::drawFrame() {

    PrepareRenderingImGui();

    //�摜���擾����ă����_�����O�̏������ł����Z�}�t�H
    vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

    //�X���b�v�`�F�[�����T�[�t�F�X�ƌ݊������Ȃ��Ȃ����Ƃ�
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }


    if (_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(_device, 1, &_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }

    RenderImGui(imageIndex);

    _imagesInFlight[imageIndex] = _inFlightFences[_currentFrame];

    std::array<VkCommandBuffer, 2> submitCommandBuffers = { _commandBuffers[imageIndex], i_commandBuffers[imageIndex] };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    //���M����R�}���h�o�b�t�@�̎w��
    submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommandBuffers.size());
    submitInfo.pCommandBuffers = submitCommandBuffers.data();

    //�����_�����O���I�����ĕ\�����鏀�����ł����i�R�}���h�o�b�t�@���I�������j�Z�}�t�H
    VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);

    if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { _swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(_presentQueue, &presentInfo);

    //�E�B���h�E�T�C�Y���ύX����Ă�����
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || _framebufferResized) {
        _framebufferResized = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    _currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanBase::Run() {
    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();

        //auto tStart = std::chrono::high_resolution_clock::now();

        if (viewUpdated)
        {
            viewUpdated = false;
        }

        drawFrame();

        //auto tEnd = std::chrono::high_resolution_clock::now();
        //auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
        //frameTimer = (float)tDiff / 1000.0f;
        //camera.update(frameTimer);
        //if (camera.moving()) {
        //    //�L�[�{�[�h
        //    viewUpdated = true;
        //}

        UpdateUniformBuffer();
    }
    vkDeviceWaitIdle(_device);
}


//�E�B���h�E�T�C�Y�ύX��
void VulkanBase::RecreateSwapChain() {

    //�t���[���o�b�t�@���쐬������
    //�r���[�|�[�g�T�C�Y�̓O���t�B�b�N�X�p�C�v���C���̍쐬���Ɏw�肳���̂ŁA�p�C�v���C�����č\�z����B
    int width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }



    //�R�}���h���I������܂ő҂�
    vkDeviceWaitIdle(_device);

    CleanupSwapChain();

    CreateSwapChain();
    CreateSwapChainImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateDepthResources();
    CreateFramebuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();

    _imagesInFlight.resize(_swapChainResources.size(), VK_NULL_HANDLE);

    camera.updateAspectRatio((float)width / (float)height);

    //ImGui
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_physicalDevice);
    ImGui_ImplVulkan_SetMinImageCount(swapChainSupport.capabilities.minImageCount + 1);

    for (auto framebuffer : i_frameBuffers) {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(_device, i_renderPass, nullptr);

    CreateRenderPassForImGui();
    CreateFrameBuffersForImGui();
    i_commandBuffers.resize(_swapChainResources.size());
    CreateCommandBuffersForImGui(i_commandBuffers.data(), static_cast<uint32_t>(i_commandBuffers.size()), i_commandPool);
}


//�I����
void VulkanBase::CleanupSwapChain() {

    vkDestroyImageView(_device, _depthImageView, nullptr);
    vkDestroyImage(_device, _depthImage, nullptr);
    vkFreeMemory(_device, _depthImageMemory, nullptr);

    for (auto s : _swapChainResources) {
        vkDestroyFramebuffer(_device, s->swapChainFrameBuffer, nullptr);
        vkDestroyImageView(_device, s->swapChainImageView, nullptr);
        _swapChainResources.resize(0);
    }

    //�R�}���h�o�b�t�@���N���A����
    vkFreeCommandBuffers(_device, _commandPool, static_cast<uint32_t> (_commandBuffers.size()), _commandBuffers.data());


    vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
    vkDestroyRenderPass(_device, _renderPass, nullptr);


    vkDestroySwapchainKHR(_device, _swapChain, nullptr);


    vkDestroyBuffer(_device, _gltfShaderData.buffer, nullptr);
    vkFreeMemory(_device, _gltfShaderData.memory, nullptr);


    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
}

void VulkanBase::Cleanup() {

    CleanupSwapChain();

    //�V�F�[�_�[���W���[���̍폜
    vkDestroyShaderModule(_device, _vertShaderModule.handle, nullptr);
    vkDestroyShaderModule(_device, _fragShaderModule.handle, nullptr);

    //ImGui
    CleanupImGui();

    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout.matrices, nullptr);
    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout.textures, nullptr);

    vkDestroyBuffer(_device, _gltfVertices.buffer, nullptr);
    vkFreeMemory(_device, _gltfVertices.memory, nullptr);

    vkDestroyBuffer(_device, _gltfIndices.buffer, nullptr);
    vkFreeMemory(_device, _gltfIndices.memory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(_device, _inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(_device, _commandPool, nullptr);
    vkDestroyDevice(_device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyInstance(_instance, nullptr);
}


//glTF�֘A
void VulkanBase::CreateglTFImage(glTFImage* image, void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight) {

    _mipLevels = 1;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    //�X�e�[�W�o�b�t�@�Ƀe�N�X�`���̃f�[�^�𑗂�
    void* data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, buffer, bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    CreateImage(texWidth, texHeight, _mipLevels, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image->textureImage, image->textureImageMemory);
    TransitionImageLayout(image->textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _mipLevels);
    CopyBufferToImage(stagingBuffer, image->textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    TransitionImageLayout(image->textureImage, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, _mipLevels);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

}

void VulkanBase::CreateglTFImageView(glTFImage* image, VkFormat format) {
    image->textureImageView = CreateImageView(image->textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels);
}

void VulkanBase::CreateglTFSampler(glTFImage* image) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    //�摜�̃T�C�Y�𒴂���ꍇ�́A�e�N�X�`�����J��Ԃ�
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(_mipLevels);
    samplerInfo.mipLodBias = 0.0f;

    if (vkCreateSampler(_device, &samplerInfo, nullptr, &image->textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void VulkanBase::LoadglTFImages(tinygltf::Model& input) {

    _gltfImages.resize(input.textures.size());

    for (size_t i = 0; i < input.images.size(); i++) {
        tinygltf::Image& glTFImage = input.images[i];
        unsigned char* buffer = nullptr;
        VkDeviceSize bufferSize = 0;
        bool deleteBuffer = false;

        //RGB�̏ꍇ�ARGBA�ɂ��Ă���
        if (glTFImage.component == 3) {
            bufferSize = glTFImage.width * glTFImage.height * 4;
            buffer = new unsigned char[bufferSize];
            unsigned char* rgba = buffer;
            unsigned char* rgb = &glTFImage.image[0];
            for (size_t i = 0; i < glTFImage.width * glTFImage.height; i++) {
                memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                rgba += 4;
                rgb += 3;
            }
            deleteBuffer = true;
        }
        else {
            buffer = &glTFImage.image[0];
            bufferSize = glTFImage.image.size();
        }

        //�C���[�W�̐������쐬
        CreateglTFImage(&_gltfImages[i], buffer, bufferSize, VK_FORMAT_R8G8B8A8_UNORM, glTFImage.width, glTFImage.height);
        CreateglTFImageView(&_gltfImages[i], VK_FORMAT_R8G8B8A8_UNORM);
        CreateglTFSampler(&_gltfImages[i]);

        if (deleteBuffer) {
            delete[] buffer;
        }

    }
}

void VulkanBase::LoadglTFMaterials(tinygltf::Model& input) {

    _gltfMaterials.resize(input.materials.size());

    for (size_t i = 0; i < input.materials.size(); i++) {
        tinygltf::Material glTFMaterial = input.materials[i];
        //�x�[�X�J���[
        if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
            _gltfMaterials[i].baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
        }
        //�x�[�X�J���[�̃e�N�X�`���C���f�b�N�X
        if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
            _gltfMaterials[i]._gltfBaseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
        }

    }
}

void VulkanBase::LoadglTFTextures(tinygltf::Model& input) {
    _gltfTextures.resize(input.textures.size());
    for (size_t i = 0; i < input.textures.size(); i++) {
        _gltfTextures[i].imageIndex = input.textures[i].source;
    }
}

void VulkanBase::LoadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<Vertex>& vertexBuffer) {
    Node node{};
    node.matrix = glm::mat4(1.0f);

    //���[�J���̃m�[�h�s��
    //4�~4�s��ɏC������
    if (inputNode.scale.size() == 3) {
        node.matrix = glm::scale(node.matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
    }

    if (inputNode.translation.size() == 3) {
        node.matrix = glm::translate(node.matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
    }
    if (inputNode.rotation.size() == 4) {
        glm::quat q = glm::make_quat(inputNode.rotation.data());
        node.matrix *= glm::mat4(q);
    }

    if (inputNode.matrix.size() == 16) {
        node.matrix = glm::make_mat4x4(inputNode.matrix.data());
    }

    //�q�m�[�h�����߂�
    if (inputNode.children.size() > 0) {
        for (size_t i = 0; i < inputNode.children.size(); i++) {
            LoadNode(input.nodes[inputNode.children[i]], input, &node, indexBuffer, vertexBuffer);
        }
    }

    //�m�[�h�����b�V���f�[�^�������Ă���ꍇ�A���_�ƃC���f�b�N�X�����[�h����
    if (inputNode.mesh > -1) {
        const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
        //���̃m�[�h�̃��b�V���ɑ΂���v���~�e�B�u����
        for (size_t i = 0; i < mesh.primitives.size(); i++) {
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            uint32_t firstIndex = static_cast<uint32_t> (indexBuffer.size());
            uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
            uint32_t indexCount = 0;


            //���_
            const float* positionBuffer = nullptr;
            const float* normalsBuffer = nullptr;
            const float* texCoordsBuffer = nullptr;
            size_t vertexCount = 0;

            //���_�f�[�^�̃o�b�t�@�f�[�^
            if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                positionBuffer = reinterpret_cast<const float*> (&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                vertexCount = accessor.count;
            }
            //���_�m�[�}���̃o�b�t�@�f�[�^
            if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }
            //���_�e�N�X�`�����W�̃o�b�t�@�f�[�^
            if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                texCoordsBuffer = reinterpret_cast<const float*> (&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
            }
            //�f�[�^�����f���ɓK�p
            for (size_t v = 0; v < vertexCount; v++) {
                Vertex vert{};
                vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
                vert.color = glm::vec3(1.0f);
                vertexBuffer.push_back(vert);
            }

            //�C���f�b�N�X
            const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
            const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

            indexCount += static_cast<uint32_t>(accessor.count);

            //�F�X�ȃC���f�b�N�X�̃^�C�v������
            switch (accessor.componentType) {
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                const uint32_t* buf = reinterpret_cast<const uint32_t*> (&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t index = 0; index < accessor.count; index++) {
                    indexBuffer.push_back(buf[index] + vertexStart);
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t index = 0; index < accessor.count; index++) {
                    indexBuffer.push_back(buf[index] + vertexStart);
                }
                break;
            }
            case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t index = 0; index < accessor.count; index++) {
                    indexBuffer.push_back(buf[index] + vertexStart);
                }
                break;
            }
            default:
                std::cerr << "Index conponent type" << accessor.componentType << "not supported!" << std::endl;
                return;
            }

            Primitive primitive{};
            primitive.firstIndex = firstIndex;
            primitive.indexCount = indexCount;
            primitive.materialIndex = glTFPrimitive.material;
            node.mesh.primitives.push_back(primitive);
        }
    }
    if (parent) {
        parent->children.push_back(node);
    }
    else {
        _gltfNodes.push_back(node);
    }

}

void VulkanBase::LoadglTF(std::string filename) {

    tinygltf::Model glTFInput;
    tinygltf::TinyGLTF gltfContext;
    std::string error, warning;

    bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

    std::vector<uint32_t> indexBuffer;
    std::vector<Vertex> vertexBuffer;


    if (fileLoaded) {
        LoadglTFImages(glTFInput);
        LoadglTFMaterials(glTFInput);
        LoadglTFTextures(glTFInput);
        const tinygltf::Scene& scene = glTFInput.scenes[0];
        for (size_t i = 0; i < scene.nodes.size(); i++) {
            const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
            LoadNode(node, glTFInput, nullptr, indexBuffer, vertexBuffer);
        }
    }
    else {
        throw std::runtime_error("failed to find glTF file!");
    }

    //���_�o�b�t�@�̍쐬
    VkDeviceSize vertexBufferSize = vertexBuffer.size() * sizeof(Vertex);


    Vertices vertexStaging;


    CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexStaging.buffer, vertexStaging.memory);

    void* data;
    vkMapMemory(_device, vertexStaging.memory, 0, vertexBufferSize, 0, &data);
    memcpy(data, vertexBuffer.data(), (size_t)vertexBufferSize);
    vkUnmapMemory(_device, vertexStaging.memory);

    CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _gltfVertices.buffer, _gltfVertices.memory);

    CopyBuffer(vertexStaging.buffer, _gltfVertices.buffer, vertexBufferSize);

    vkDestroyBuffer(_device, vertexStaging.buffer, nullptr);
    vkFreeMemory(_device, vertexStaging.memory, nullptr);



    //�C���f�b�N�X�o�b�t�@�̍쐬
    VkDeviceSize indexBufferSize = indexBuffer.size() * sizeof(uint32_t);
    _gltfIndices.count = static_cast<uint32_t>(indexBuffer.size());

    Vertices indexStaging;

    CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, indexStaging.buffer, indexStaging.memory);

    vkMapMemory(_device, indexStaging.memory, 0, indexBufferSize, 0, &data);
    memcpy(data, indexBuffer.data(), (size_t)indexBufferSize);
    vkUnmapMemory(_device, indexStaging.memory);

    CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _gltfIndices.buffer, _gltfIndices.memory);

    CopyBuffer(indexStaging.buffer, _gltfIndices.buffer, indexBufferSize);

    vkDestroyBuffer(_device, indexStaging.buffer, nullptr);
    vkFreeMemory(_device, indexStaging.memory, nullptr);


}



//imGUI�֘A
//imGUI������
void VulkanBase::CreateRenderPassForImGui() {

    //�ŏ����ɍ쐬
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_physicalDevice);

    VkAttachmentDescription attachment = {};
    attachment.format = _swapChainImageFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;
    if (vkCreateRenderPass(_device, &info, nullptr, &i_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Could not create Dear ImGui's render pass");
    }
}

void VulkanBase::CreateCommandPoolForImGui(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags) {

    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(_physicalDevice);
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    commandPoolCreateInfo.flags = flags;

    if (vkCreateCommandPool(_device, &commandPoolCreateInfo, nullptr, commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Could not create graphics command pool");
    }
}

void VulkanBase::CreateFrameBuffersForImGui() {

    i_frameBuffers.resize(_swapChainResources.size());

    for (size_t i = 0; i < _swapChainResources.size(); i++) {

        VkImageView attachment[1];
        VkFramebufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.renderPass = i_renderPass;
        info.attachmentCount = 1;
        info.pAttachments = attachment;
        info.width = _swapChainExtent.width;
        info.height = _swapChainExtent.height;
        info.layers = 1;

        attachment[0] = _swapChainResources[i]->swapChainImageView;

        vkCreateFramebuffer(_device, &info, nullptr, &i_frameBuffers[i]);

    }
}

void VulkanBase::CreateCommandBuffersForImGui(VkCommandBuffer* commandBuffer, uint32_t commandBufferCount, VkCommandPool& commandPool) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.commandBufferCount = commandBufferCount;
    vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, commandBuffer);
}

void VulkanBase::PrepareImGui() {

    //imgui�p�̃f�B�X�N���v�^�v�[�����쐬
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
    pool_info.pPoolSizes = pool_sizes;

    vkCreateDescriptorPool(_device, &pool_info, nullptr, &i_descriptorPool);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForVulkan(_window, true);

    QueueFamilyIndices indices = FindQueueFamilies(_physicalDevice);

    ImGui_ImplVulkan_InitInfo info{};
    info.Instance = _instance;
    info.PhysicalDevice = _physicalDevice;
    info.Device = _device;
    info.QueueFamily = indices.graphicsFamily.value();
    info.Queue = _graphicsQueue;
    info.PipelineCache = VK_NULL_HANDLE;
    info.DescriptorPool = i_descriptorPool;
    info.Allocator = nullptr;

    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(_physicalDevice);

    info.MinImageCount = swapChainSupport.capabilities.minImageCount + 1;
    info.ImageCount = swapChainSupport.capabilities.minImageCount + 1;
    info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&info, i_renderPass);


    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
    EndSingleTimeCommands(commandBuffer);
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void VulkanBase::PrepareRenderingImGui() {

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    // ImGui �E�B�W�F�b�g��`�悷��
    ImGui::Begin("Info");
    ImGui::Text("Framerate(avg) %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    if (ImGui::Button("Button"))
    {

    }

    ImGui::SliderFloat("Rotate", &_angle, 0.0f, 360.0f);
    ImGui::SliderFloat("PosX", &_cmeraPosX, 2.0f, 10.0f);
    ImGui::SliderFloat("PosY", &_cameraPosY, 2.0f, 10.0f);
    ImGui::SliderFloat("PosZ", &_cameraPsZ, 2.0f, 10.0f);

    ImGui::ColorPicker4("Color", _color);
    ImGui::End();
    ImGui::Render();
}

void VulkanBase::RenderImGui(uint32_t imageIndex) {

    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(i_commandBuffers[imageIndex], &info);


    VkRenderPassBeginInfo RenderPassinfo = {};

    VkClearValue clearValue{};
    clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };

    RenderPassinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassinfo.renderPass = i_renderPass;
    RenderPassinfo.framebuffer = i_frameBuffers[imageIndex];
    RenderPassinfo.renderArea.extent.width = _swapChainExtent.width;
    RenderPassinfo.renderArea.extent.height = _swapChainExtent.height;
    RenderPassinfo.clearValueCount = 1;
    RenderPassinfo.pClearValues = &clearValue;
    vkCmdBeginRenderPass(i_commandBuffers[imageIndex], &RenderPassinfo, VK_SUBPASS_CONTENTS_INLINE);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), i_commandBuffers[imageIndex]);


    vkCmdEndRenderPass(i_commandBuffers[imageIndex]);
    vkEndCommandBuffer(i_commandBuffers[imageIndex]);
}

void VulkanBase::CleanupImGui() {
    for (auto framebuffer : i_frameBuffers) {
        vkDestroyFramebuffer(_device, framebuffer, nullptr);
    }
    vkDestroyRenderPass(_device, i_renderPass, nullptr);

    vkFreeCommandBuffers(_device, i_commandPool, static_cast<uint32_t>(i_commandBuffers.size()), i_commandBuffers.data());
    vkDestroyCommandPool(_device, i_commandPool, nullptr);
    vkDestroyDescriptorPool(_device, i_descriptorPool, nullptr);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}

void VulkanBase::InitializeVulkanBase(GLFWwindow* window) {

    _window = window;

    SetupGlfwCallbacks();
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickupPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateSwapChainImageViews();
    CreateRenderPass();

    //�V�F�[�_�[�ǂݍ���
    CreateShaderModules("shaders/mesh.vert.spv", "shaders/mesh.frag.spv");
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();


    CreateCommandPool();
    CreateDepthResources();
    CreateFramebuffers();


    LoadglTF("./models/flightHelmet/FlightHelmet.gltf");



    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();


    //ImGui
    CreateRenderPassForImGui();
    //���CreateCommandPool()�Ƃ܂Ƃ߂�
    CreateCommandPoolForImGui(&i_commandPool, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    CreateFrameBuffersForImGui();
    i_commandBuffers.resize(_swapChainResources.size());
    CreateCommandBuffersForImGui(i_commandBuffers.data(), static_cast<uint32_t>(i_commandBuffers.size()), i_commandPool);
    PrepareImGui();
}

void VulkanBase::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {

    //Linear filtering�̃T�|�[�g���m�F����
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(_physicalDevice, imageFormat, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (uint32_t i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = { 0, 0, 0 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = { 0, 0, 0 };
        blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    EndSingleTimeCommands(commandBuffer);
}

bool VulkanBase::HasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}


int main() {

    if (glfwInit() == GL_FALSE) {
        throw std::runtime_error("failed to initialize GLFW");
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window;

    window = glfwCreateWindow(WIDTH, HEIGHT, "LoadglTFModel", NULL, NULL);

    VulkanBase* _vulkan = new VulkanBase;

    _vulkan->InitializeVulkanBase(window);

    _vulkan->Run();

    /**
     * �I����
     */
    glfwDestroyWindow(window);
    glfwTerminate();

    return 1;
}