#include "Bindable/DepthStencilState.h"

namespace Bind
{
	DepthStencilState::DepthStencilState()
	{
		VkPipelineDepthStencilStateCreateInfo depthStencilState = Graphics::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
		depthStencilStates.emplace_back(depthStencilState);
		depthStencilState.stencilTestEnable = VK_TRUE;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;	// always pass stencil test
		depthStencilState.back.failOp = VK_STENCIL_OP_REPLACE;		// failed stencil test
		depthStencilState.back.depthFailOp = VK_STENCIL_OP_REPLACE; // pass stencil test but failed depth test
		depthStencilState.back.passOp = VK_STENCIL_OP_REPLACE;		// pass both the depth and stencil tests.
		depthStencilState.back.compareMask = 0xff;
		depthStencilState.back.writeMask = 0xff;
		depthStencilState.back.reference = 1;
		depthStencilState.front = depthStencilState.back;
		depthStencilStates.emplace_back(depthStencilState);
		depthStencilState.back.compareOp = VK_COMPARE_OP_NOT_EQUAL;	// only pass stencil test when A != B
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.depthFailOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_REPLACE;
		depthStencilState.front = depthStencilState.back;
		depthStencilState.depthTestEnable = VK_FALSE;
		depthStencilStates.emplace_back(depthStencilState);
	}

	VkPipelineDepthStencilStateCreateInfo DepthStencilState::GetDepthStencilState(DepthStencilStateType type)
	{ 
		return depthStencilStates[static_cast<size_t>(type)];
	}
	std::unique_ptr<DepthStencilState> depthStencilState_ptr = std::make_unique<DepthStencilState>();


}

