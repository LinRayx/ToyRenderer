#pragma once

#include "vulkan_basics.h"

namespace Mesh {
	
	struct mesh_buffer_t {
		buffers_s vertices;
		void* data;
	};

	int create_mesh_buffer(mesh_buffer_t* buffer, uint32_t size, void* data, const device_t* device);
}