#pragma once
#include "VertexBuffer.h"

namespace Bind
{

	VertexBuffer::VertexBuffer(const Dcb::VertexBuffer& vbuf, bool onlyLayout)
	{
		for (size_t i = 0; i < vbuf.GetLayout().GetElementCount(); ++i)
		{
			auto& elem = vbuf.GetLayout().ResolveByIndex(i);
			VkVertexInputAttributeDescription attr = {};
			attr.binding = 0;
			attr.location = location++;
			attr.format = elem.GetFormat();
			attr.offset = elem.GetOffset();

			attributeDescriptions.emplace_back(std::move(attr));
		}

		bindingDescription.binding = 0;
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescription.stride = vbuf.GetLayout().Size();
		if (onlyLayout) return;
		buffer_ptr = std::make_shared<Graphics::Buffer>(Graphics::BufferUsage::VERTEX_BUFFER, vbuf.SizeBytes(), (void*)vbuf.GetData(), vbuf.Size());
	}

	VertexBuffer::~VertexBuffer()
	{
	}
	VkBuffer VertexBuffer::Get()
	{
		return buffer_ptr->buffers[0];
	}
}