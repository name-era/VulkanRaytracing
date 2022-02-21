#pragma once

#define IMAGEFORMAT VK_FORMAT_R8G8B8A8_SRGB
#define FONTFORMAT VK_FORMAT_R8G8B8A8_UNORM
#define SWAPCHAINCOLORFORMAT VK_FORMAT_B8G8R8A8_UNORM
#define VERTEXFORMAT VK_FORMAT_R32G32B32_SFLOAT

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <algorithm>

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
		void* mapped;

		void Flush(VkDevice& device, size_t uploadSize)
		{
			VkMappedMemoryRange mappedRange = {};
			mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedRange.memory = memory;
			mappedRange.offset = 0;
			mappedRange.size = uploadSize;
			vkFlushMappedMemoryRanges(device, 1, &mappedRange);
		}

		void Destroy(VkDevice& device) {
			vkDestroyBuffer(device, buffer, nullptr);
			vkFreeMemory(device, memory, nullptr);
		}

		uint64_t GetBufferDeviceAddress(VkDevice device) {
			VkBufferDeviceAddressInfo bufferDeviceInfo{};
			bufferDeviceInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			bufferDeviceInfo.buffer = buffer;
			return vkGetBufferDeviceAddressKHR(device, &bufferDeviceInfo);
		}

	};

	struct DynamicBuffer {
		Buffer dynamicBuffer;
		uint64_t blockSize;
	};
	
	struct Image {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkSampler sampler;
	};

}

namespace PrimitiveMesh {

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 color;
	};

	inline void GetCeiling(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
		Vertex srcVertices[] = {
			Vertex{ {-1.0f, 0.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f}, color },
			Vertex{ {-1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f}, color },
			Vertex{ { 1.0f, 0.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f}, color },
			Vertex{ { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f}, color },
		};
		vertices.resize(4);

		std::transform(
			std::begin(srcVertices), std::end(srcVertices), vertices.begin(),
			[=](auto v) {
				v.pos.x *= 10.0f;
				v.pos.z *= 10.0f;
				return v;
			}
		);

		indices = { 0, 1, 2, 2, 1, 3 };
	}

}