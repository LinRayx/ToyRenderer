#pragma once
#ifndef BUFFER_H
#define BUFFER_H
#include "Vulkan.h"
#include <memory>
using namespace std;

namespace Graphics {

	enum class BufferUsage
	{
		VERTEX_BUFFER
	};

	class Buffer
	{
		friend class CommandBuffer;
	public:

		Buffer(shared_ptr<Vulkan> _vulkan_ptr, size_t size);
		Buffer(shared_ptr<Vulkan> _vulkan_ptr, size_t size, void* data);
		Buffer(shared_ptr<Vulkan> _vulkan_ptr, size_t size, void* data, size_t elem_count);
		Buffer(shared_ptr<Vulkan> _vulkan_ptr, BufferUsage type, size_t size, void* data, size_t elem_count);
		void CopyData(void* data);
		~Buffer();


	private:
		VkBufferUsageFlags getUsage(BufferUsage type);

		shared_ptr<Vulkan> vulkan_ptr;
		Vulkan::buffers_t buffer;
		size_t elem_count;
		void* data;
	};

}
#endif // !BUFFER_H