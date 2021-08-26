#ifndef OUTLINE_PSO_H
#define OUTLINE_PSO_H

#include "RenderSystem/PipelineStateObject.h"
#include "VulkanCore/VulkanInitalizers.hpp"

namespace RenderSystem
{
	// 1. ��Ⱦ���壬ͬʱд��ģ�建�壬stencil pass ͨ����д�룬����������Ȳ���
	// 2. ��ֹд��ģ�建�壬������Ȳ��ԣ��Ŵ�һ�����壬��Ⱦ�߿�
	// 3. ��������ģ�����Ȼ���
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