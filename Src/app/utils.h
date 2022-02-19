#pragma once
#include <glm/glm.hpp>
#include <fcntl.h>
#include <vulkan/vulkan.h>

namespace utils
{
	uint32_t GetAlinedSize(uint32_t value, uint32_t alignment);

	VkTransformMatrixKHR ConvertFrom4x4To3x4(const glm::mat4x3& m);
}