#include "Graphics.h"

namespace Graphics
{

	VkDescriptorType Graphics::GetDescriptorType(DescriptorType type)
	{
		switch (type)
		{
		case DescriptorType::UNIFORM:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			break;
		default:
			break;
		}
	}

	VkShaderStageFlags Graphics::GetStageFlag(StageFlag flag)
	{
		switch (flag)
		{
		case StageFlag::VERTEX:
			return VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case StageFlag::FRAGMENT:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case StageFlag::ALL:
			return VK_SHADER_STAGE_ALL;
			break;
		default:
			break;
		}
	}
}