
#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "vulkan_basics.h"

namespace PointLight {
	struct point_light_attri_t {
		glm::vec3 Position;
		glm::vec3 Color;
		float density;
	};

	struct point_light_attri_buffer_t {
		buffers_t buffer;
		void* data;
	};

	struct point_light_mvp_t {
		glm::vec3 model;
		glm::vec3 view;
		glm::vec3 proj;
		glm::vec3 Color;
	};

	struct point_light_mvp_buffer_t {
		buffers_t buffer;
		void* data;
	};

	int create_light_attri_buffer(point_light_attri_buffer_t* lights, const device_t* device, const swapchain_t* swapchain);
	int create_light_mvp_buffer(point_light_mvp_buffer_t* lights, const device_t* device, const swapchain_t* swapchain);

	int update_light_attri(point_light_attri_buffer_t* lights, point_light_attri_t* attri, const swapchain_t* swapchain);
	int update_light_mvp(point_light_mvp_buffer_t* lights, const swapchain_t* swapchain);

	int destroy(buffers_t* buffer, const device_t* device);
}
#endif // !POINT_LIGHT_H
