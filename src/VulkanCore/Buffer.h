#pragma once
#ifndef BUFFER_H
#define BUFFER_H
#include "Vulkan.h"
#include <memory>
using namespace std;

namespace Graphics {

	class Buffer
	{

	public:

		Buffer(shared_ptr<Vulkan> _vulkan_ptr, size_t size);
		Buffer(shared_ptr<Vulkan> _vulkan_ptr, size_t size, void* data);
		void CopyData(void* data);
		~Buffer();


	private:
		shared_ptr<Vulkan> vulkan_ptr;
		Vulkan::buffers_t buffer;
		void* data;
	};

}
#endif // !BUFFER_H