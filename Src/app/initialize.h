#pragma once
#include <optional>
#include <vector>
#include <set>
#include <iostream>

#include <vulkan/vulkan.h>

struct Initialize {
public:




    /**
    * @brief    �R���X�g���N�^
    */
    Initialize();

    /**
    * @brief    �f�X�g���N�^
    */
    ~Initialize();





    /**
    * @brief    �f�o�C�X�̊g�����`�F�b�N����
    */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    /**
    * @brief    �f�o�C�X���g���邩�m�F����
    */
    bool IsDeviceSuitable(VkPhysicalDevice device);





















};