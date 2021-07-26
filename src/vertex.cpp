#include "vertex.h"

namespace Vertex
{
	// VertexLayout
	const VertexLayout::Element& VertexLayout::ResolveByIndex(size_t i) const 
	{
		return elements[i];
	}
	VertexLayout& VertexLayout::Append(ElementType type) 
	{
		elements.emplace_back(type, Size());
		return *this;
	}
	size_t VertexLayout::Size() const 
	{
		return elements.empty() ? 0u : elements.back().GetOffsetAfter();
	}
	size_t VertexLayout::GetElementCount() const noexcept
	{
		return elements.size();
	}



	// VertexLayout::Element
	VertexLayout::Element::Element(ElementType type, size_t offset)
		:
		type(type),
		offset(offset)
	{}
	size_t VertexLayout::Element::GetOffsetAfter() const 
	{
		return offset + Size();
	}
	size_t VertexLayout::Element::GetOffset() const
	{
		return offset;
	}
	size_t VertexLayout::Element::Size() const 
	{
		return SizeOf(type);
	}
	constexpr size_t VertexLayout::Element::SizeOf(ElementType type) 
	{
		switch (type)
		{
		case Position2D:
			return sizeof(Map<Position2D>::SysType);
		case Position3D:
			return sizeof(Map<Position3D>::SysType);
		case Texture2D:
			return sizeof(Map<Texture2D>::SysType);
		case Normal:
			return sizeof(Map<Normal>::SysType);
		case Float3Color:
			return sizeof(Map<Float3Color>::SysType);
		case Float4Color:
			return sizeof(Map<Float4Color>::SysType);
		}
		assert("Invalid element type" && false);
		return 0u;
	}

	VkFormat VertexLayout::Element::GetVkFormat(ElementType type)
	{
		switch (type)
		{
		case Position2D:
			return Map<Position2D>::vkFormat;
		case Position3D:
			return Map<Position3D>::vkFormat;
		case Texture2D:
			return Map<Texture2D>::vkFormat;
		case Normal:
			return Map<Normal>::vkFormat;
		case Float3Color:
			return Map<Float3Color>::vkFormat;
		case Float4Color:
			return Map<Float4Color>::vkFormat;
		}
		assert("Invalid element type" && false);
		return VK_FORMAT_R8_SINT;
	}

	VertexLayout::ElementType VertexLayout::Element::GetType() const noexcept
	{
		return type;
	}

	VkVertexInputBindingDescription VertexLayout::GetBindingDescription() const 
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = Size();
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	std::vector<VkVertexInputAttributeDescription> VertexLayout::GetAttributeDescriptions() const 
	{
		std::vector<VkVertexInputAttributeDescription> vec(GetElementCount());
		for (size_t i = 0; i < GetElementCount(); ++i) {
			vec[i].binding = 0;
			vec[i].location = i;
			
			VertexLayout::Map<Position2D>::vkFormat;

			vec[i].format = VertexLayout::Element::GetVkFormat(elements[i].GetType());
			vec[i].offset = elements[i].GetOffset();
		}
		return std::vector<VkVertexInputAttributeDescription>();
	}


	// Vertex
	Vertex::Vertex(char* pData, const VertexLayout& layout) 
		:
	pData(pData),
		layout(layout)
	{
		assert(pData != nullptr);
	}
	ConstVertex::ConstVertex(const Vertex& v) 
		:
	vertex(v)
	{}


	// VertexBuffer
	VertexBuffer::VertexBuffer(VertexLayout layout) 
		:
	layout(std::move(layout))
	{}
	const char* VertexBuffer::GetData() const 
	{
		return buffer.data();
	}
	const VertexLayout& VertexBuffer::GetLayout() const noexcept
	{
		return layout;
	}
	size_t VertexBuffer::Size() const 
	{
		return buffer.size() / layout.Size();
	}
	size_t VertexBuffer::SizeBytes() const 
	{
		return buffer.size();
	}
	Vertex VertexBuffer::Back() 
	{
		assert(buffer.size() != 0u);
		return Vertex{ buffer.data() + buffer.size() - layout.Size(),layout };
	}
	Vertex VertexBuffer::Front() 
	{
		assert(buffer.size() != 0u);
		return Vertex{ buffer.data(),layout };
	}
	Vertex VertexBuffer::operator[](size_t i) 
	{
		assert(i < Size());
		return Vertex{ buffer.data() + layout.Size() * i,layout };
	}
	ConstVertex VertexBuffer::Back() const 
	{
		return const_cast<VertexBuffer*>(this)->Back();
	}
	ConstVertex VertexBuffer::Front() const 
	{
		return const_cast<VertexBuffer*>(this)->Front();
	}
	ConstVertex VertexBuffer::operator[](size_t i) const 
	{
		return const_cast<VertexBuffer&>(*this)[i];
	}
}