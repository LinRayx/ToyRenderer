#pragma once
#include "IndexBuffer.h"

namespace Bind
{

	IndexBuffer::IndexBuffer(const std::vector<unsigned short>& vbuf)
	{
		this->elem_count = vbuf.size();
		buffer_ptr = std::make_shared<Graphics::Buffer>(Graphics::BufferUsage::INDEX_BUFFER, vbuf.size() * sizeof(vbuf[0]), (void*)vbuf.data(), vbuf.size());
	}
	IndexBuffer::~IndexBuffer()
	{
	}
	size_t IndexBuffer::GetCount()
	{
		return elem_count;
	}
	VkBuffer IndexBuffer::Get()
	{
		return buffer_ptr->buffers[0];
	}
}