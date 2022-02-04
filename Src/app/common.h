#define IMAGEFORMAT VK_FORMAT_R8G8B8A8_SRGB
#define SRGBCOLORFORMAT VK_FORMAT_B8G8R8A8_SRGB

#include <vulkan/vulkan.h>

namespace Initializers {
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