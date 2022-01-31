#pragma once
#include <optional>
#include <vector>
#include <set>
#include <iostream>

#include <vulkan/vulkan.h>

struct Initialize {
public:




    /**
    * @brief    コンストラクタ
    */
    Initialize();

    /**
    * @brief    デストラクタ
    */
    ~Initialize();





    /**
    * @brief    デバイスの拡張をチェックする
    */
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    /**
    * @brief    デバイスが使えるか確認する
    */
    bool IsDeviceSuitable(VkPhysicalDevice device);





















};