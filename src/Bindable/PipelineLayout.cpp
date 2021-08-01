#include "PipelineLayout.h"

namespace Bind
{
	void PipelineLayout::AddLayout(DESCRIPTOR_TYPE type, uint32_t offset)
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
		layout_binding.binding = dstBinding++;
		layout_bindinds.emplace_back(std::move(layout_binding));

	}

}