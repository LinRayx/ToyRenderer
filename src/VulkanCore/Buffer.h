
#pragma once
#ifndef BUFFER_H
#define BUFFER_H
#include "Vulkan.h"
#include <vector>
#include <memory>
using namespace std;

namespace Graphics {


	class Buffer
	{
		friend class CommandBuffer;
		friend class DescriptorSetCore;
	public:
		Buffer(shared_ptr<Vulkan> _vulkan_ptr, BufferUsage type, size_t size);
		Buffer(shared_ptr<Vulkan> _vulkan_ptr, BufferUsage type, size_t size, void* data, size_t elem_count);
		~Buffer();
		void UpdateData(uint32_t currentImage, size_t size, const char* newData);
	private:
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		

		std::vector<VkDeviceMemory> buffersMemorys;
		VkBufferUsageFlags getUsage(BufferUsage type);
		shared_ptr<Vulkan> vulkan_ptr;
		size_t size;
		
		bool update;
	public:
		size_t elem_count;
		vector<VkBuffer> buffers;
	};

}
#endif // !BUFFER_H