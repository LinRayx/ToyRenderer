#include "mesh.h"
#include <string.h>

namespace Mesh {
	int create_mesh_buffer(mesh_buffer_t* buffer, uint32_t size, void* data, const device_t* device) {
		memset(buffer, 0, sizeof(*buffer));
		VkBufferCreateInfo bufferInfo = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};

		if (create_buffers(&buffer->vertices, device, &bufferInfo, 1, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			return 1;
		}
		if (vkMapMemory(device->device, buffer->vertices.memory, 0, buffer->vertices.size, 0, &buffer->data)) {
			return 1;
		}
		memcpy(buffer->data, data, (size_t)buffer->vertices.size);
	}
}