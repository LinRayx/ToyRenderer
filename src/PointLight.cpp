#include "PointLight.h"

namespace PointLight {


	int create_light_attri_buffer(point_light_attri_buffer_t* lights, const device_t* device, const swapchain_t* swapchain) {
		memset(lights, 0, sizeof(*lights));
		VkBufferCreateInfo bufferInfo = {};

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(point_light_attri_t);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBufferCreateInfo* buffer_infos = (VkBufferCreateInfo*)malloc(sizeof(VkBufferCreateInfo) * swapchain->image_count);
		for (uint32_t i = 0; i != swapchain->image_count; ++i)
			buffer_infos[i] = bufferInfo;

		if (create_buffers(&lights->buffer, device, buffer_infos, swapchain->image_count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			return 1;
		}

		if (vkMapMemory(device->device, lights->buffer.memory, 0, lights->buffer.size, 0, &lights->data)) {
			return 1;
		}

		return 0;
	}

	int create_light_mvp_buffer(point_light_mvp_buffer_t* lights, const device_t* device, const swapchain_t* swapchain) {
		memset(lights, 0, sizeof(*lights));
		VkBufferCreateInfo bufferInfo = {};

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(point_light_mvp_t);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBufferCreateInfo* buffer_infos = (VkBufferCreateInfo*)malloc(sizeof(VkBufferCreateInfo) * swapchain->image_count);
		for (uint32_t i = 0; i != swapchain->image_count; ++i)
			buffer_infos[i] = bufferInfo;

		if (create_buffers(&lights->buffer, device, buffer_infos, swapchain->image_count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			return 1;
		}

		if (vkMapMemory(device->device, lights->buffer.memory, 0, lights->buffer.size, 0, &lights->data)) {
			return 1;
		}

		return 0;
	}

	int update_light_attri(point_light_attri_buffer_t* lights, point_light_attri_t* attri, const swapchain_t* swapchain) {
		attri->Color = glm::vec3(1, 0, 0);
		attri->density = 1;
		attri->Position = glm::vec3(0, 0, 5);

		for (int i = 0; i < swapchain->image_count; ++i) {
			memcpy(((char*)lights->data) + lights->buffer.buffers[i].offset, attri, sizeof(*attri));
		}
		return 0;
	}

	int update_light_mvp(point_light_mvp_buffer_t* lights, const swapchain_t* swapchain) {
		return 0;
	}
}