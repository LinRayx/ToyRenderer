#include "Buffer.h"

namespace Graphics {
	Buffer::Buffer(shared_ptr<Vulkan> _vulkan_ptr, size_t size) : vulkan_ptr(_vulkan_ptr)
	{
		//memset(lights, 0, sizeof(*lights));
		//VkBufferCreateInfo bufferInfo = {};

		//bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		//bufferInfo.size = sizeof(point_light_attri_t);
		//bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		//bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//VkBufferCreateInfo* buffer_infos = (VkBufferCreateInfo*)malloc(sizeof(VkBufferCreateInfo) * swapchain->image_count);
		//for (uint32_t i = 0; i != swapchain->image_count; ++i)
		//	buffer_infos[i] = bufferInfo;

		//if (create_buffers(&lights->buffer, device, buffer_infos, swapchain->image_count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
		//	return 1;
		//}

		//if (vkMapMemory(device->device, lights->buffer.memory, 0, lights->buffer.size, 0, &lights->data)) {
		//	return 1;
		//}

		//return 0;
		memset(&buffer, 0, sizeof(buffer));
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBufferCreateInfo* buffer_infos = (VkBufferCreateInfo*)malloc(sizeof(VkBufferCreateInfo) * _vulkan_ptr->swapchain.image_count);
		for (uint32_t i = 0; i != _vulkan_ptr->swapchain.image_count; ++i) {
			buffer_infos[i] = buffer_info;
		}
		if (_vulkan_ptr->create_buffers(&buffer, buffer_infos, _vulkan_ptr->swapchain.image_count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			exit(1);
		}

		free(buffer_infos);

		if (vkMapMemory(_vulkan_ptr->device.device, buffer.memory, 0, buffer.size, 0, &data)) {
			exit(1);
		}
	}
	Buffer::~Buffer()
	{
	}

	

}