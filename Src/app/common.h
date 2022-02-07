#pragma once

#define IMAGEFORMAT VK_FORMAT_R8G8B8A8_SRGB
#define FONTFORMAT VK_FORMAT_R8G8B8A8_UNORM
#define SWAPCHAINCOLORFORMAT VK_FORMAT_B8G8R8A8_SRGB
#define VERTEXFORMAT VK_FORMAT_R32G32B32_SFLOAT

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
		int count;
		void* data;

		void flush(VkDevice& device, size_t uploadSize)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = memory;
			mappedRange.offset = 0;
			mappedRange.size = uploadSize;
			vkFlushMappedMemoryRanges(device, 1, &mappedRange);
		}
	};
	
	struct Image {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkSampler sampler;
	};


}