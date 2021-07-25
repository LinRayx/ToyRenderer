#include "constant_value.h"

namespace ConstantValue {
	int create_constant_param_buffer(constant_param_buffer_t* buffer, const device_t* device, const swapchain_t* swapchain) {
		memset(buffer, 0, sizeof(*buffer));
		VkBufferCreateInfo bufferInfo = {};

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(constant_param_t);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBufferCreateInfo* buffer_infos = (VkBufferCreateInfo*)malloc(sizeof(VkBufferCreateInfo) * swapchain->image_count);
		for (uint32_t i = 0; i != swapchain->image_count; ++i)
			buffer_infos[i] = bufferInfo;

		if (create_buffers(&buffer->buffer, device, buffer_infos, swapchain->image_count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			return 1;
		}

		if (vkMapMemory(device->device, buffer->buffer.memory, 0, buffer->buffer.size, 0, &buffer->data)) {
			return 1;
		}

		free(buffer_infos);
		return 0;
	}

	int update_constant_param(constant_param_t* constant, constant_param_buffer_t* buffer, constant_set_param* param, const swapchain_t* swapchain) {
		constant->viewPos = param->camera->Position;
		for (int i = 0; i < swapchain->image_count; ++i) {
			memcpy(((char*)buffer->data) + buffer->buffer.buffers[i].offset, constant, sizeof(constant_param_t));
		}
		return 0;
	}
}