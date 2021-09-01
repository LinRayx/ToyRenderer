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

	/**
	* Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
	*
	* @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the buffer mapping call
	*/
	VkResult BufferV2::map(VkDeviceSize size, VkDeviceSize offset)
	{
		return vkMapMemory(device, memory, offset, size, 0, &mapped);
	}

	/**
	* Unmap a mapped memory range
	*
	* @note Does not return a result as vkUnmapMemory can't fail
	*/
	void BufferV2::unmap()
	{
		if (mapped)
		{
			vkUnmapMemory(device, memory);
			mapped = nullptr;
		}
	}

	/**
	* Attach the allocated memory block to the buffer
	*
	* @param offset (Optional) Byte offset (from the beginning) for the memory region to bind
	*
	* @return VkResult of the bindBufferMemory call
	*/
	VkResult BufferV2::bind(VkDeviceSize offset)
	{
		return vkBindBufferMemory(device, buffer, memory, offset);
	}

	/**
	* Setup the default descriptor for this buffer
	*
	* @param size (Optional) Size of the memory range of the descriptor
	* @param offset (Optional) Byte offset from beginning
	*
	*/
	void BufferV2::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
	{
		descriptor.offset = offset;
		descriptor.buffer = buffer;
		descriptor.range = size;
	}

	/**
	* Copies the specified data to the mapped buffer
	*
	* @param data Pointer to the data to copy
	* @param size Size of the data to copy in machine units
	*
	*/
	void BufferV2::copyTo(void* data, VkDeviceSize size)
	{
		assert(mapped);
		memcpy(mapped, data, size);
	}

	/**
	* Flush a memory range of the buffer to make it visible to the device
	*
	* @note Only required for non-coherent memory
	*
	* @param size (Optional) Size of the memory range to flush. Pass VK_WHOLE_SIZE to flush the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the flush call
	*/
	VkResult BufferV2::flush(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(device, 1, &mappedRange);
	}

	/**
	* Invalidate a memory range of the buffer to make it visible to the host
	*
	* @note Only required for non-coherent memory
	*
	* @param size (Optional) Size of the memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete buffer range.
	* @param offset (Optional) Byte offset from beginning
	*
	* @return VkResult of the invalidate call
	*/
	VkResult BufferV2::invalidate(VkDeviceSize size, VkDeviceSize offset)
	{
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(device, 1, &mappedRange);
	}

	/**
	* Release all Vulkan resources held by this buffer
	*/
	void BufferV2::destroy()
	{
		if (buffer)
		{
			vkDestroyBuffer(device, buffer, nullptr);
		}
		if (memory)
		{
			vkFreeMemory(device, memory, nullptr);
		}
	}

	VkResult BufferV2::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, BufferV2* buffer, VkDeviceSize size, void* data)
	{
		buffer->device = Vulkan::getInstance()->GetDevice().device;

		// Create the buffer handle
		VkBufferCreateInfo bufferCreateInfo = initializers::bufferCreateInfo(usageFlags, size);
		vkCreateBuffer(Vulkan::getInstance()->GetDevice().device,  &bufferCreateInfo, nullptr, &buffer->buffer);

		// Create the memory backing up the buffer handle
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc = initializers::memoryAllocateInfo();
		vkGetBufferMemoryRequirements(Vulkan::getInstance()->GetDevice().device, buffer->buffer, &memReqs);
		memAlloc.allocationSize = memReqs.size;
		// Find a memory type index that fits the properties of the buffer
		memAlloc.memoryTypeIndex = Vulkan::getInstance()->findMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
		// If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
		VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
		if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
			memAlloc.pNext = &allocFlagsInfo;
		}
		vkAllocateMemory(Vulkan::getInstance()->GetDevice().device, &memAlloc, nullptr, &buffer->memory);

		buffer->alignment = memReqs.alignment;
		buffer->size = size;
		buffer->usageFlags = usageFlags;
		buffer->memoryPropertyFlags = memoryPropertyFlags;

		// If a pointer to the buffer data has been passed, map the buffer and copy over the data
		if (data != nullptr)
		{
			buffer->map();
			memcpy(buffer->mapped, data, size);
			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
				buffer->flush();

			buffer->unmap();
		}

		// Initialize a default descriptor that covers the whole buffer size
		buffer->setupDescriptor();

		// Attach the memory to the buffer object
		return buffer->bind();
	}

}
