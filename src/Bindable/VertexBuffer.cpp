#pragma once
#include "VertexBuffer.h"

namespace Bind
{
	VertexBuffer::VertexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, size_t size, void* data)
	{
		// buffer_ptr = std::make_shared<Graphics::Buffer>(_vulkan_ptr, size, data);
	}

	VertexBuffer::VertexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, const Dcb::VertexBuffer& vbuf, bool onlyLayout)
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
		bindingDescription.stride = vbuf.SizeBytes();
		if (onlyLayout) return;
		buffer_ptr = std::make_shared<Graphics::Buffer>(_vulkan_ptr, Graphics::BufferUsage::VERTEX_BUFFER, vbuf.SizeBytes(), (void*)vbuf.GetData(), vbuf.Size());
	}

	VertexBuffer::~VertexBuffer()
	{
	}
}