#include "Buffer.h"
#include <stdexcept>

namespace Graphics {


	Buffer::Buffer(shared_ptr<Vulkan> _vulkan_ptr, BufferUsage type, size_t size) : vulkan_ptr(_vulkan_ptr), size(size)
	{
		buffers.resize(vulkan_ptr->swapchain.image_count);
		buffersMemorys.resize(vulkan_ptr->swapchain.image_count);
		for (size_t i = 0; i < buffers.size(); ++i)
		{
			createBuffer(size, getUsage(type), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffers[i], buffersMemorys[i]);
		}
	}


	// Vertex Buffer
	Buffer::Buffer(shared_ptr<Vulkan> _vulkan_ptr, BufferUsage type, size_t size, void* newData, size_t elem_count) : vulkan_ptr(_vulkan_ptr), elem_count(elem_count), size(size)
	{
		buffers.resize(1);
		buffersMemorys.resize(1);
		for (size_t i = 0; i < buffers.size(); ++i)
		{
			void* data;
			createBuffer(size, getUsage(type), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffers[i], buffersMemorys[i]);
			if (vkMapMemory(vulkan_ptr->device.device, buffersMemorys[i], 0, size, 0, &data)) {
				exit(1);
			}

			memcpy(data, newData, size);

			vkUnmapMemory(vulkan_ptr->device.device, buffersMemorys[i]);
		}
	}

	Buffer::~Buffer()
	{
		for (size_t i = 0; i < buffers.size(); i++) {
			vkDestroyBuffer(vulkan_ptr->device.device, buffers[i], vulkan_ptr->device.allocator);
			vkFreeMemory(vulkan_ptr->device.device, buffersMemorys[i], vulkan_ptr->device.allocator);
		}
	}

	void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(vulkan_ptr->device.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(vulkan_ptr->device.device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = vulkan_ptr->findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(vulkan_ptr->device.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(vulkan_ptr->device.device, buffer, bufferMemory, 0);
	}

	void Buffer::UpdateData(uint32_t currentImage, size_t size, void* newData)
	{
		update = true;
		void* data;
		vkMapMemory(vulkan_ptr->device.device, buffersMemorys[currentImage], 0, size, 0, &data);
		memcpy(data, &newData, size);
		vkUnmapMemory(vulkan_ptr->device.device, buffersMemorys[currentImage]);
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
