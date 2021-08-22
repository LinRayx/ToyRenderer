#pragma once
#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "Bindable.h"
#include "Buffer.h"
#include <vector>

namespace Bind
{
	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, size_t size, void* data);
		IndexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, const std::vector<unsigned short>& vbuf);
		~IndexBuffer();
		std::shared_ptr<Graphics::Buffer> buffer_ptr;
		size_t GetCount();
	private:
		size_t elem_count;
	};
}

#endif // !INDEX_BUFFER_H
