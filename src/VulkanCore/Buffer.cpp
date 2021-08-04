#include "Buffer.h"
namespace Graphics {
	Buffer::Buffer(shared_ptr<Vulkan> _vulkan_ptr, size_t size) : vulkan_ptr(_vulkan_ptr)
	{

		memset(&buffer, 0, sizeof(buffer));
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		uint32_t count = 1;

		VkBufferCreateInfo* buffer_infos = (VkBufferCreateInfo*)malloc(sizeof(VkBufferCreateInfo) * count);
		for (uint32_t i = 0; i != count; ++i) {
			buffer_infos[i] = buffer_info;
		}
		if (_vulkan_ptr->create_buffers(&buffer, buffer_infos, count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			exit(1);
		}

		free(buffer_infos);
		if (vkMapMemory(vulkan_ptr->device.device, buffer.memory, 0, buffer.size, 0, &data)) {
			exit(1);
		}
	}

	Buffer::Buffer(shared_ptr<Vulkan> _vulkan_ptr, size_t size, void* data) : vulkan_ptr(_vulkan_ptr)
	{

		memset(&buffer, 0, sizeof(buffer));
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		uint32_t count = 1;

		VkBufferCreateInfo* buffer_infos = (VkBufferCreateInfo*)malloc(sizeof(VkBufferCreateInfo) * count);
		for (uint32_t i = 0; i != count; ++i) {
			buffer_infos[i] = buffer_info;
		}
		if (_vulkan_ptr->create_buffers(&buffer, buffer_infos, count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			exit(1);
		}

		free(buffer_infos);
		if (vkMapMemory(vulkan_ptr->device.device, buffer.memory, 0, buffer.size, 0, &data)) {
			exit(1);
		}

		memcpy(this->data, data, buffer.size);

		vkUnmapMemory(vulkan_ptr->device.device, buffer.memory);
	}

	void Buffer::CopyData(void* data)
	{
		memcpy(this->data, data, buffer.size);
	}

	Buffer::Buffer(shared_ptr<Vulkan> _vulkan_ptr, BufferUsage type, size_t size, void* data, size_t elem_count) : vulkan_ptr(_vulkan_ptr), elem_count(elem_count)
	{
		memset(&buffer, 0, sizeof(buffer));
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = getUsage(type);
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		uint32_t count = 1;

		VkBufferCreateInfo* buffer_infos = (VkBufferCreateInfo*)malloc(sizeof(VkBufferCreateInfo) * count);
		for (uint32_t i = 0; i != count; ++i) {
			buffer_infos[i] = buffer_info;
		}
		if (_vulkan_ptr->create_buffers(&buffer, buffer_infos, count, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
			exit(1);
		}

		free(buffer_infos);
		if (vkMapMemory(vulkan_ptr->device.device, buffer.memory, 0, buffer.size, 0, &this->data)) {
			exit(1);
		}

		memcpy(this->data, data, size);

		vkUnmapMemory(vulkan_ptr->device.device, buffer.memory);
	}

	Buffer::~Buffer()
	{
		vulkan_ptr->destroy_buffers(&buffer);
	}

	VkBufferUsageFlags Buffer::getUsage(BufferUsage type)
	{
		switch (type)
		{
		case BufferUsage::VERTEX_BUFFER:
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		default:
			break;
		}
		return VkBufferUsageFlags();
	}



}
