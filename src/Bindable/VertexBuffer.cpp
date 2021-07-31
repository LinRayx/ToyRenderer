#pragma once
#include "VertexBuffer.h"

namespace Bind
{
	VertexBuffer::VertexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, size_t size, void* data)
	{
		buffer_ptr = std::make_shared<Graphics::Buffer>(_vulkan_ptr, size, data);
	}

	VertexBuffer::~VertexBuffer()
	{
	}

}