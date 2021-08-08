#pragma once
#include "IndexBuffer.h"

namespace Bind
{
	IndexBuffer::IndexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, size_t size, void* data)
	{
		// buffer_ptr = std::make_shared<Graphics::Buffer>(_vulkan_ptr, size, data);
	}
	IndexBuffer::~IndexBuffer()
	{
	}
}