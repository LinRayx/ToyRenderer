#include "RenderSystem/RenderLoop.h"

namespace RenderSystem
{

	RenderLoop::RenderLoop(shared_ptr<Graphics::Vulkan> vulkan_ptr,
		shared_ptr<Graphics::CommandBuffer> cmdBuf_ptr, shared_ptr<Graphics::CommandQueue> cmdQue_ptr)
		: vulkan_ptr(vulkan_ptr), cmdBuf_ptr(cmdBuf_ptr), cmdQue_ptr(cmdQue_ptr)
	{
		frameT_ptr = std::make_shared<FrameTimer>();
	}
	void RenderLoop::AddPSO(PipelineStateObject& pso)
	{
		pso_vecs.emplace_back(std::move(pso));
	}
}