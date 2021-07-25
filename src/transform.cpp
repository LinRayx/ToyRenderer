
#include "transform.h"

namespace Transform {
	int create_mvp_matrix_buffer(mvp_matrix_buffer_t* buffer, const device_t* device, const swapchain_t* swapchain) {
		memset(buffer, 0, sizeof(*buffer));
		VkBufferCreateInfo bufferInfo = {};

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(mvp_matrix_t);
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
		return 0;
	}

	int update_mvp_matrix(mvp_matrix_buffer_t* buffer, mvp_matrix_t* mvp_matrix, mvp_set_param* param, const swapchain_t* swapchain) {
		mvp_matrix->view = GetViewMatrix(param->camera);
		mvp_matrix->proj = GetProjectMatrix(param->camera, param->width, param->height);
		mvp_matrix->model = param->model;
		mvp_matrix->model = glm::rotate(mvp_matrix->model, glm::radians(param->angle), glm::vec3(1.0f, 0.3f, 0.5f));
		for (int i = 0; i < swapchain->image_count; ++i) {
			memcpy(((char*)buffer->data) + buffer->buffer.buffers[i].offset, mvp_matrix, sizeof(mvp_matrix_t));
		}
		// memcpy((char*)cube_transform->data + sizeof(ubo), &ubo, sizeof(ubo));
		return 0;
	}
}