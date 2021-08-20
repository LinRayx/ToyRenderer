#pragma once
#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

#include "Bindable.h"
#include "Buffer.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "DynamicVertex.h"

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
		VertexBuffer(VkVertexInputBindingDescription _bindingDescription, std::vector<VkVertexInputAttributeDescription> _attributeDescriptions) 
			: bindingDescription(_bindingDescription), attributeDescriptions(_attributeDescriptions)
		{}
		VertexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, size_t size, void* data);
		VertexBuffer(std::shared_ptr<Graphics::Vulkan> _vulkan_ptr, const Dcb::VertexBuffer& vbuf, bool onlyLayout = false);
		~VertexBuffer();
		VkBuffer Get();
		std::shared_ptr<Graphics::Buffer> buffer_ptr;
	private:
		int location = 0;
	public:
		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	};



}

#endif // !VERTEX_BUFFER_H
