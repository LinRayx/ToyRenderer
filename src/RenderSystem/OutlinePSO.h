#ifndef OUTLINE_PSO_H
#define OUTLINE_PSO_H

#include "RenderSystem/PipelineStateObject.h"
#include "VulkanCore/VulkanInitalizers.hpp"

namespace RenderSystem
{
	// 1. 渲染物体，同时写入模板缓冲，stencil pass 通过就写入，不用在意深度测试
	// 2. 禁止写入模板缓冲，禁用深度测试，放大一个物体，渲染边框
	// 3. 重新启用模板和深度缓冲
	class OutlinePSO : public PipelineStateObject
	{
	public:
		OutlinePSO();
		~OutlinePSO();

		virtual void BuildPipeline() override;
		virtual void BuildCommandBuffer(shared_ptr<Graphics::CommandBuffer> cmd) override;

		virtual void Update(int cur) override;
		virtual void Add(Draw::Model* model) override;
	private:

		shared_ptr<Bind::VertexShader> v_outlineShader_ptr;
		shared_ptr<Bind::PixelShader> p_outlineShader_ptr;
		

		VkPipeline pipelineStencil;
		VkPipeline pipelineOutline;
	};
}

#endif