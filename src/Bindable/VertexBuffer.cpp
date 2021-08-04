#pragma once
#include "VertexBuffer.h"

namespace Bind
{
	VertexBuffer::VertexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, size_t size, void* data)
	{
		buffer_ptr = std::make_shared<Graphics::Buffer>(_vulkan_ptr, size, data);
	}

	VertexBuffer::VertexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, std::shared_ptr<Draw::Model> model_ptr)
	{
		buffer_ptr = std::make_shared<Graphics::Buffer>(_vulkan_ptr, Graphics::BufferUsage::VERTEX_BUFFER, model_ptr->getSize(), model_ptr->getData(), model_ptr->getCount());
		bindingDescription = model_ptr->getBindingDescription();
		attributeDescriptions = model_ptr->getAttributeDescriptions();
	}

	VertexBuffer::~VertexBuffer()
	{
	}

}