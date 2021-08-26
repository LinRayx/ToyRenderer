#include "Buffer.h"
#include <stdexcept>

namespace Graphics {


	Buffer::Buffer(BufferUsage type, size_t size) : size(size)
	{
		buffers.resize(Vulkan::getInstance()->swapchain.image_count);
		buffersMemorys.resize(Vulkan::getInstance()->swapchain.image_count);
		for (size_t i = 0; i < buffers.size(); ++i)
		{
			createBuffer(size, getUsage(type), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffers[i], buffersMemorys[i]);
		}
	}


	// Vertex Buffer
	Buffer::Buffer(BufferUsage type, size_t size, void* newData, size_t elem_count) :  elem_count(elem_count), size(size)
	{
		buffers.resize(1);
		buffersMemorys.resize(1);
		for (size_t i = 0; i < buffers.size(); ++i)
		{
			void* data;
			createBuffer(size, getUsage(type), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffers[i], buffersMemorys[i]);
			if (vkMapMemory(Vulkan::getInstance()->device.device, buffersMemorys[i], 0, size, 0, &data)) {
				exit(1);
			}

			memcpy(data, newData, size);

			vkUnmapMemory(Vulkan::getInstance()->device.device, buffersMemorys[i]);
		}
	}

	Buffer::~Buffer()
	{
		for (size_t i = 0; i < buffers.size(); i++) {
			vkDestroyBuffer(Vulkan::getInstance()->device.device, buffers[i], Vulkan::getInstance()->device.allocator);
			vkFreeMemory(Vulkan::getInstance()->device.device, buffersMemorys[i], Vulkan::getInstance()->device.allocator);
		}
	}

	void Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		if (vkCreateBuffer(Vulkan::getInstance()->device.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Vulkan::getInstance()->device.device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Vulkan::getInstance()->findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(Vulkan::getInstance()->device.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(Vulkan::getInstance()->device.device, buffer, bufferMemory, 0);
	}

	void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(Vulkan::getInstance()->device.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(Vulkan::getInstance()->device.device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Vulkan::getInstance()->findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(Vulkan::getInstance()->device.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(Vulkan::getInstance()->device.device, buffer, bufferMemory, 0);
	}

	void Buffer::UpdateData(uint32_t currentImage, size_t size, const char* newData)
	{
		update = true;
		void* data;
		vkMapMemory(Vulkan::getInstance()->device.device, buffersMemorys[currentImage], 0, size, 0, &data);
		memcpy(data, newData, size);
		vkUnmapMemory(Vulkan::getInstance()->device.device, buffersMemorys[currentImage]);
	}

	VkBufferUsageFlags Buffer::getUsage(BufferUsage type)
	{
		switch (type)
		{
		case BufferUsage::VERTEX_BUFFER:
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		case BufferUsage::UNIFORM:
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		case BufferUsage::INDEX_BUFFER:
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		default:
			break;
		}
		return VkBufferUsageFlags();
	}



}
