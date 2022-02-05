#pragma once

#define IMAGEFORMAT VK_FORMAT_R8G8B8A8_SRGB
#define SRGBCOLORFORMAT VK_FORMAT_B8G8R8A8_SRGB

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

#include <vulkan/vulkan.h>

namespace Initializers {
	
	struct MouseButtons {
		bool left = false;
		bool right = false;
		bool middle = false;
	};

	struct Buffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
	};
	
	struct Image {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkSampler sampler;
	};


}