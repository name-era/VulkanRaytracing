#include "utils.h"

namespace utils
{
	uint32_t GetAlinedSize(uint32_t value, uint32_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	VkTransformMatrixKHR ConvertFrom4x4To3x4(const glm::mat4x3& m) {
		VkTransformMatrixKHR mtx{};
		auto mT = glm::transpose(m);
		memcpy(&mtx.matrix[0], &mT[0], sizeof(float) * 4);
		memcpy(&mtx.matrix[1], &mT[1], sizeof(float) * 4);
		memcpy(&mtx.matrix[2], &mT[2], sizeof(float) * 4);
		return mtx;
	}
}