#ifndef STENCIL_STATE_H
#define STENCIL_STATE_H

#include "Bindable/Bindable.h"
#include "VulkanCore/Vulkan.h"
#include "VulkanCore/VulkanInitalizers.hpp"
#include <memory>

namespace Bind
{
	enum class DepthStencilStateType
	{
		Default = 0,
		WriteStencil,
		DrawOutline,
	};

	class DepthStencilState : public Bindable
	{
	public:
		DepthStencilState();

		VkPipelineDepthStencilStateCreateInfo GetDepthStencilState(DepthStencilStateType type);
	private:
		std::vector<VkPipelineDepthStencilStateCreateInfo> depthStencilStates;
	};
	// һ��Ҫ�ǵõ�cpp�ж����±���
	extern std::unique_ptr<DepthStencilState> depthStencilState_ptr;


}

#endif // !STENCIL_STATE_H
