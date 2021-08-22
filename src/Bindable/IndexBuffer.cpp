#pragma once
#include "IndexBuffer.h"

namespace Bind
{
	IndexBuffer::IndexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, size_t size, void* data)
	{
		// buffer_ptr = std::make_shared<Graphics::Buffer>(_vulkan_ptr, size, data);
	}
	IndexBuffer::IndexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, const std::vector<unsigned short>& vbuf)
	{
		this->elem_count = vbuf.size();
		buffer_ptr = std::make_shared<Graphics::Buffer>(_vulkan_ptr, Graphics::BufferUsage::INDEX_BUFFER, vbuf.size() * sizeof(vbuf[0]), (void*)vbuf.data(), vbuf.size());
	}
	IndexBuffer::~IndexBuffer()
	{
	}
	size_t IndexBuffer::GetCount()
	{
		return elem_count;
	}
}