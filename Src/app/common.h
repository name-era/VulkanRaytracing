#pragma once

#define IMAGE_FORMAT VK_FORMAT_R8G8B8A8_SRGB
#define FONT_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define SWAPCHAIN_COLOR_FORMAT VK_FORMAT_B8G8R8A8_UNORM
#define VERTEX_FORMAT VK_FORMAT_R32G32B32_SFLOAT
#define DEFAULT_FENCE_TIMEOUT 100000000000

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <algorithm>
#include <stdexcept>
#include <vector>

const uint32_t WIDTH = 1280;
const uint32_t HEIGHT = 720;

namespace vk {
	
	struct MouseButtons {
		bool left = false;
		bool right = false;
		bool middle = false;
	};

	struct Buffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		int count = 0;
		void* mapped;
		VkDeviceAddress address = 0;

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
			address = vkGetBufferDeviceAddressKHR(device, &bufferDeviceInfo);
			return address;
		}

	};
	
	struct Image {
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;
		VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		uint32_t layerCount = 1;
		void Destroy(VkDevice device) {
			if (image != VK_NULL_HANDLE) {
				vkDestroyImage(device, image, nullptr);
			}
			if (memory != VK_NULL_HANDLE) {
				vkDestroyImageView(device, view, nullptr);
			}
			if (view != VK_NULL_HANDLE) {
				vkFreeMemory(device, memory, nullptr);
			}
			if (sampler != VK_NULL_HANDLE) {
				vkDestroySampler(device, sampler, nullptr);
			}
		}
		
		void SetImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, uint32_t mipLevels);

	};
}

namespace PrimitiveMesh {

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
		glm::vec4 color;
	};

	inline void GetPlane(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
	{
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
		Vertex srcVertices[] = {
			Vertex{ {-1.0f, -1.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f}, color },
			Vertex{ {-1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f}, color },
			Vertex{ { 1.0f, -1.0f,-1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f}, color },
			Vertex{ { 1.0f, -1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f}, color },
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


	inline void GetSphere(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, float radius, uint32_t slices, uint32_t stacks) {
		//vertices
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
		const float SLICES = (float)slices;
		const float STACKS = (float)stacks;
		for (uint32_t stack = 0; stack <= stacks; stack++) {
			for (uint32_t slice = 0; slice <= slices; slice++) {
				glm::vec3 p;
				//-1`1
				p.y = 2.0f * stack / STACKS - 1.0f;
				//circle radius correspond to y
				float r = std::sqrtf(1 - p.y * p.y);
				//rotation x and z axis
				float theta = 2.0f * glm::pi<float>() * slice / SLICES;
				p.x = r * std::sinf(theta);
				p.z = r * std::cosf(theta);
				glm::vec3 v = p * radius;
				glm::vec3 n = glm::normalize(v);
				glm::vec2 uv = { float(slice) / SLICES,1.0 - float(stack) / STACKS };

				Vertex vert;
				vert.pos = v;
				vert.normal = n;
				vert.uv = uv;
				vert.color = color;
				vertices.push_back(vert);
			}
		}

		//indices
		uint32_t sliceNum = slices + 1;
		for (uint32_t stack = 0; stack < STACKS; stack++) {
			for (uint32_t slice = 0; slice < slices; slice++) {
				uint32_t index = stack * sliceNum;
				uint32_t i0 = index + slice;
				uint32_t i1 = index + slice + 1;
				uint32_t i2 = i0 + sliceNum;
				uint32_t i3 = i1 + sliceNum;
				indices.push_back(i0);
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i2);
				indices.push_back(i1);
				indices.push_back(i3);
			}
		}
	}
}