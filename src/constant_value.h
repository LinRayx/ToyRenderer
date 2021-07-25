#ifndef CONSTANT_VALUE
#define CONSTANT_VALUE
#include <glm/glm.hpp>
#include "vulkan_basics.h"
#include "camera.h"

namespace ConstantValue {
	static int width = 800;
	static int height = 600;
	static glm::vec4 clearColor = glm::vec4(0, 0, 0, 0);
	static float angle = 45.f;
	static const float SPEED = 2.5f;
	static const float SENSITIVITY = 0.1f;

	struct constant_param_t {
		glm::vec3 viewPos;
	};

	struct constant_param_buffer_t {
		buffers_s buffer;
		void* data;
	};

	struct constant_set_param {
		first_person_camera_t* camera;
	};

	int create_constant_param_buffer(constant_param_buffer_t* buffer, const device_t* device, const swapchain_t* swapchain);
	int update_constant_param(constant_param_t* constant, constant_param_buffer_t* buffer, constant_set_param* param, const swapchain_t* swapchain);
}

#endif // !CONSTANT_VALUE
