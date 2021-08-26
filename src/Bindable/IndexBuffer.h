#pragma once
#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "Bindable.h"
#include "VulkanCore/vulkan_core_headers.h"

#include <vector>

namespace Bind
{
	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer(const std::vector<unsigned short>& vbuf);
		~IndexBuffer();
		std::shared_ptr<Graphics::Buffer> buffer_ptr;
		size_t GetCount();
	private:
		size_t elem_count;
	};
}

#endif // !INDEX_BUFFER_H
