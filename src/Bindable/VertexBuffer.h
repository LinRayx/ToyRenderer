#pragma once
#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "Bindable.h"
#include "Buffer.h"
#include <memory>
#include <vector>

namespace Draw
{
	class Drawable;
}

namespace Bind
{
	class VertexBuffer : public Bindable
	{
		friend class Draw::Drawable;
	public:
		VertexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, size_t size, void* data);
		~VertexBuffer();

	private:
		std::shared_ptr<Graphics::Buffer> buffer_ptr;
		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};
}

#endif // !VERTEX_BUFFER_H
