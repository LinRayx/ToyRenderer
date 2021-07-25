#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan_basics.h"
#include "camera.h"

namespace Transform {
	struct mvp_matrix_t {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	struct mvp_matrix_buffer_t {
		buffers_s buffer;
		void* data;
	};

	struct mvp_set_param {
		int width;
		int height;
		float angle;
		glm::mat4 model;
		first_person_camera_t* camera;
	};

	int create_mvp_matrix_buffer(mvp_matrix_buffer_t* buffer, const device_t* device, const swapchain_t* swapchain);
	int update_mvp_matrix(mvp_matrix_buffer_t* buffer, mvp_matrix_t* mvp_matrix, mvp_set_param* param, const swapchain_t* swapchain);
}

#endif // !TRANSFORM_H
