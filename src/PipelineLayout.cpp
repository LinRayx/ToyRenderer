#include "PipelineLayout.h"

namespace Bind
{
	void PipelineLayout::AddLayout(DESCRIPTOR_TYPE type, uint32_t offset, Graphics::Buffer* buffer)
	{
		VkDescriptorSetLayoutBinding layout_binding = {};
		switch (type)
		{
		case DESCRIPTOR_TYPE::UNFIORM:
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		default:
			break;
		}
		VkDescriptorBufferInfo buffer_info = {};
		buffer_info.offset = offset;
		VkWriteDescriptorSet write_set = {};
		write_set.dstBinding = ++dstBinding;

		layout_bindinds.emplace_back(std::move(layout_binding));
		buffer_infos.emplace_back(std::move(buffer_info));
		write_sets.emplace_back(std::move(write_set));
		buffers.emplace_back(buffer);
	}

	void Update()
	{

	}
}