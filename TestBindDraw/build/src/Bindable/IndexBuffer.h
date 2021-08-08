#pragma once
#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "Bindable.h"
#include "Buffer.h"

namespace Bind
{
	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, size_t size, void* data);
		~IndexBuffer();
		Bindable* Bind() noexcept;
	private:
		std::shared_ptr<Graphics::Buffer> buffer_ptr;
	};
}

#endif // !INDEX_BUFFER_H
